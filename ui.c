#include "ui.h"

#ifdef _WIN32

#include "memory.h"
#include "predictor.h"

#include <windows.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_CLASS "MemPredictCDashboard"
#define APP_TITLE "MemPredictC | Memory Monitoring & Prediction"
#define COLOR_BG RGB(13, 20, 32)
#define COLOR_PANEL RGB(24, 34, 50)
#define COLOR_PANEL_ALT RGB(30, 43, 62)
#define COLOR_TEXT RGB(229, 238, 248)
#define COLOR_MUTED RGB(147, 166, 188)
#define COLOR_CYAN RGB(52, 211, 235)
#define COLOR_BLUE RGB(71, 133, 255)
#define COLOR_GREEN RGB(64, 203, 123)
#define COLOR_YELLOW RGB(246, 190, 65)
#define COLOR_RED RGB(240, 91, 91)
#define MAX_LOG_ENTRIES 12
#define CONTROL_ID_SIZE 1001
#define CONTROL_ID_HORIZON 1002
#define CONTROL_ID_ALLOCATE 1003
#define CONTROL_ID_CALLOC 1004
#define CONTROL_ID_REALLOCATE 1005
#define CONTROL_ID_FREE 1006
#define CONTROL_ID_PREDICT 1007
#define CONTROL_ID_DEMO 1008
#define CONTROL_ID_LEAKS 1009

typedef struct {
    char text[128];
    COLORREF color;
} ActivityEntry;

typedef struct {
    HWND window;
    HWND size_edit;
    HWND horizon_edit;
    HWND allocate_button;
    HWND calloc_button;
    HWND realloc_button;
    HWND free_button;
    HWND predict_button;
    HWND demo_button;
    HWND leaks_button;
    HFONT title_font;
    HFONT body_font;
    HFONT small_font;
    ActivityEntry log[MAX_LOG_ENTRIES];
    size_t log_count;
    unsigned long selected_id;
    int confirmation_pending;
} UiState;

static UiState state;

static void set_font(HWND control, HFONT font)
{
    SendMessageA(control, WM_SETFONT, (WPARAM) font, TRUE);
}

static void add_log(const char *text, COLORREF color)
{
    size_t i;

    if (state.log_count < MAX_LOG_ENTRIES) {
        state.log_count++;
    }
    for (i = state.log_count; i > 1; --i) {
        state.log[i - 1] = state.log[i - 2];
    }
    strncpy(state.log[0].text, text, sizeof(state.log[0].text) - 1);
    state.log[0].text[sizeof(state.log[0].text) - 1] = '\0';
    state.log[0].color = color;
}

static int read_size(HWND edit, size_t *value)
{
    char buffer[64];
    char *end;
    unsigned long long parsed;

    if (GetWindowTextA(edit, buffer, sizeof(buffer)) <= 0 ||
        buffer[0] == '-') {
        return 0;
    }
    parsed = strtoull(buffer, &end, 10);
    if (end == buffer || *end != '\0' || parsed == 0 ||
        parsed > (unsigned long long) SIZE_MAX) {
        return 0;
    }
    *value = (size_t) parsed;
    return 1;
}

static int read_id(size_t *value)
{
    return read_size(state.size_edit, value);
}

static void set_status(const char *message, COLORREF color)
{
    add_log(message, color);
    InvalidateRect(state.window, NULL, FALSE);
}

static void record_current_memory(void)
{
    predictor_record(memory_get_stats().current_bytes);
}

static void perform_allocate(AllocationKind kind)
{
    size_t size;
    unsigned long id;
    char message[128];

    if (!read_size(state.size_edit, &size)) {
        set_status("Invalid size: enter a positive whole number.", COLOR_RED);
        return;
    }
    if (!memory_allocate(size, kind, &id)) {
        set_status("Allocation failed: request could not be completed.", COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "%s %lu bytes (ID %lu)",
             kind == ALLOCATION_CALLOC ? "calloc:" : "malloc:",
             (unsigned long) size, id);
    set_status(message, COLOR_GREEN);
}

static void perform_reallocate(void)
{
    size_t id;
    size_t new_size;
    char message[128];

    if (!read_id(&id) || id > (size_t) ULONG_MAX ||
        !read_size(state.horizon_edit, &new_size)) {
        set_status("Reallocate: enter ID in Size and new bytes in Horizon.",
                   COLOR_RED);
        return;
    }
    if (!memory_reallocate((unsigned long) id, new_size)) {
        set_status("Reallocate failed: check the active ID and size.", COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "resized ID %lu to %lu bytes",
             (unsigned long) id, (unsigned long) new_size);
    set_status(message, COLOR_CYAN);
}

static void perform_free(void)
{
    size_t id;
    char message[128];

    if (!read_id(&id) || id > (size_t) ULONG_MAX ||
        !memory_free((unsigned long) id)) {
        set_status("Free failed: enter an active allocation ID in Size.", COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "released allocation ID %lu",
             (unsigned long) id);
    set_status(message, COLOR_GREEN);
}

static void perform_prediction(void)
{
    size_t steps;
    PredictionStats prediction;
    char message[128];

    if (!read_size(state.horizon_edit, &steps)) {
        set_status("Invalid horizon: enter a positive number.", COLOR_RED);
        return;
    }
    prediction = predictor_get_stats(steps);
    if (!prediction.has_prediction) {
        set_status("Prediction needs at least two observations.", COLOR_YELLOW);
        return;
    }
    snprintf(message, sizeof(message), "prediction generated: %.0f bytes in %lu steps",
             prediction.predicted_memory, (unsigned long) steps);
    set_status(message, prediction.warning ? COLOR_YELLOW : COLOR_CYAN);
}

static void perform_demo(void)
{
    unsigned long first;
    unsigned long second;
    unsigned long leaked;
    int first_ready = 0;
    int second_ready = 0;

    if (memory_allocate(120, ALLOCATION_MALLOC, &first)) {
        first_ready = 1;
        record_current_memory();
    }
    if (memory_allocate(80, ALLOCATION_CALLOC, &second)) {
        second_ready = 1;
        record_current_memory();
    }
    if (first_ready && memory_reallocate(first, 200)) {
        record_current_memory();
    }
    if (second_ready && memory_free(second)) {
        record_current_memory();
    }
    if (memory_allocate(900, ALLOCATION_MALLOC, &leaked)) {
        record_current_memory();
        (void) leaked;
    }
    set_status("automatic demonstration completed; one allocation is intentionally outstanding.",
               COLOR_YELLOW);
}

static void show_leaks(void)
{
    MemoryStats stats = memory_get_stats();
    char message[128];

    if (stats.active_allocations == 0) {
        set_status("Leak check: no outstanding tracked allocations.", COLOR_GREEN);
        return;
    }
    snprintf(message, sizeof(message), "leak detected: %lu allocations, %lu bytes",
             (unsigned long) stats.active_allocations,
             (unsigned long) stats.current_bytes);
    set_status(message, COLOR_RED);
}

static void draw_text(HDC dc, const char *text, int x, int y, COLORREF color,
                      HFONT font)
{
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, font);
    TextOutA(dc, x, y, text, (int) strlen(text));
}

static void draw_panel(HDC dc, RECT rect, const char *title)
{
    HBRUSH brush = CreateSolidBrush(COLOR_PANEL);
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
    draw_text(dc, title, rect.left + 18, rect.top + 14, COLOR_CYAN,
              state.body_font);
}

static void draw_stat(HDC dc, RECT rect, const char *label, size_t value,
                      COLORREF accent)
{
    char value_text[64];
    HBRUSH brush = CreateSolidBrush(COLOR_PANEL);

    FillRect(dc, &rect, brush);
    DeleteObject(brush);
    draw_text(dc, label, rect.left + 14, rect.top + 12, COLOR_MUTED,
              state.small_font);
    snprintf(value_text, sizeof(value_text), "%lu", (unsigned long) value);
    draw_text(dc, value_text, rect.left + 14, rect.top + 35, accent,
              state.title_font);
}

static void draw_graph(HDC dc, RECT rect)
{
    size_t observations[MAX_OBSERVATIONS];
    size_t count = predictor_copy_observations(observations, MAX_OBSERVATIONS);
    size_t i;
    int graph_left = rect.left + 42;
    int graph_right = rect.right - 18;
    int graph_top = rect.top + 48;
    int graph_bottom = rect.bottom - 24;
    size_t max_value = 1;
    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(46, 63, 84));
    HPEN line_pen = CreatePen(PS_SOLID, 2, COLOR_CYAN);
    HGDIOBJ old_pen;

    for (i = 0; i < count; ++i) {
        if (observations[i] > max_value) {
            max_value = observations[i];
        }
    }
    draw_text(dc, "Memory usage over observations", rect.left + 18,
              rect.top + 14, COLOR_CYAN, state.body_font);
    old_pen = SelectObject(dc, grid_pen);
    MoveToEx(dc, graph_left, graph_bottom, NULL);
    LineTo(dc, graph_right, graph_bottom);
    MoveToEx(dc, graph_left, graph_top, NULL);
    LineTo(dc, graph_left, graph_bottom);
    SelectObject(dc, line_pen);
    if (count > 0) {
        for (i = 0; i < count; ++i) {
            int x = graph_left + (int) ((graph_right - graph_left) *
                                        (double) i / (count > 1 ? count - 1 : 1));
            int y = graph_bottom - (int) ((graph_bottom - graph_top) *
                                          (double) observations[i] / max_value);
            if (i == 0) {
                MoveToEx(dc, x, y, NULL);
            } else {
                LineTo(dc, x, y);
            }
        }
    }
    SelectObject(dc, old_pen);
    DeleteObject(grid_pen);
    DeleteObject(line_pen);
}

static void draw_dashboard(HDC dc, RECT client)
{
    MemoryStats stats = memory_get_stats();
    PredictionStats prediction = predictor_get_stats(3);
    AllocationRecord records[MAX_ALLOCATIONS];
    size_t count = memory_get_active_allocations(records, MAX_ALLOCATIONS);
    RECT rect;
    char text[160];
    size_t i;
    int panel_width = (client.right - 48) / 3;

    draw_text(dc, "MemPredictC", 24, 20, COLOR_CYAN, state.title_font);
    draw_text(dc, "Memory Monitoring & Prediction", 190, 28, COLOR_MUTED,
              state.body_font);
    draw_text(dc, "● LIVE SESSION", client.right - 150, 28, COLOR_GREEN,
              state.small_font);

    rect.top = 70;
    rect.bottom = 128;
    for (i = 0; i < 6; ++i) {
        rect.left = 24 + (int) (i % 3) * (panel_width + 12);
        rect.right = rect.left + panel_width;
        if (i >= 3) {
            rect.top = 136;
            rect.bottom = 194;
        }
        switch (i) {
        case 0: draw_stat(dc, rect, "CURRENT MEMORY (bytes)",
                          stats.current_bytes, COLOR_CYAN); break;
        case 1: draw_stat(dc, rect, "PEAK MEMORY (bytes)",
                          stats.peak_bytes, COLOR_BLUE); break;
        case 2: draw_stat(dc, rect, "TOTAL ALLOCATED (bytes)",
                          stats.total_allocated, COLOR_GREEN); break;
        case 3: draw_stat(dc, rect, "TOTAL FREED (bytes)",
                          stats.total_freed, COLOR_GREEN); break;
        case 4: draw_stat(dc, rect, "ACTIVE ALLOCATIONS",
                          stats.active_allocations,
                          stats.active_allocations ? COLOR_YELLOW : COLOR_GREEN); break;
        default: draw_stat(dc, rect, "ALLOCATION OPERATIONS",
                           stats.allocation_operations, COLOR_BLUE); break;
        }
    }

    rect.left = 24;
    rect.right = client.right - 300;
    rect.top = 212;
    rect.bottom = client.bottom - 24;
    draw_panel(dc, rect, "MEMORY GRAPH");
    draw_graph(dc, rect);

    rect.left = client.right - 288;
    rect.right = client.right - 24;
    rect.top = 212;
    rect.bottom = 352;
    draw_panel(dc, rect, "TREND PREDICTION");
    snprintf(text, sizeof(text), "Current: %lu bytes",
             (unsigned long) prediction.current_memory);
    draw_text(dc, text, rect.left + 18, rect.top + 48, COLOR_TEXT, state.small_font);
    snprintf(text, sizeof(text), "Avg growth: %.2f bytes/op", prediction.average_growth);
    draw_text(dc, text, rect.left + 18, rect.top + 70, COLOR_TEXT, state.small_font);
    snprintf(text, sizeof(text), "Next 3: %.0f bytes", prediction.predicted_memory);
    draw_text(dc, text, rect.left + 18, rect.top + 92, COLOR_CYAN, state.body_font);
    draw_text(dc, prediction.warning ? "Warning threshold reached" : "Below warning threshold",
              rect.left + 18, rect.top + 116,
              prediction.warning ? COLOR_YELLOW : COLOR_GREEN, state.small_font);

    rect.top = 364;
    rect.bottom = client.bottom - 24;
    draw_panel(dc, rect, "ACTIVE ALLOCATIONS");
    for (i = 0; i < count && i < 7; ++i) {
        snprintf(text, sizeof(text), "ID %-4lu  %6lu bytes  %s",
                 records[i].id, (unsigned long) records[i].size,
                 records[i].kind == ALLOCATION_CALLOC ? "calloc" : "malloc");
        draw_text(dc, text, rect.left + 18, rect.top + 46 + (int) i * 19,
                  COLOR_TEXT, state.small_font);
    }
    if (count == 0) {
        draw_text(dc, "No outstanding allocations", rect.left + 18, rect.top + 48,
                  COLOR_GREEN, state.small_font);
    }

    rect.left = client.right - 288;
    rect.right = client.right - 24;
    rect.top = 364;
    rect.bottom = client.bottom - 24;
    draw_panel(dc, rect, "ACTIVITY LOG");
    for (i = 0; i < state.log_count && i < 7; ++i) {
        draw_text(dc, state.log[i].text, rect.left + 14, rect.top + 46 + (int) i * 21,
                  state.log[i].color, state.small_font);
    }
    draw_text(dc, "Size / ID", 24, client.bottom - 62, COLOR_MUTED,
              state.small_font);
    draw_text(dc, "New size / steps", 130, client.bottom - 62, COLOR_MUTED,
              state.small_font);
}

static void create_controls(HWND window)
{
    state.size_edit = CreateWindowA("EDIT", "64", WS_CHILD | WS_VISIBLE |
                                    WS_BORDER | ES_AUTOHSCROLL, 24, 0, 100, 26,
                                    window, (HMENU) CONTROL_ID_SIZE,
                                    GetModuleHandleA(NULL), NULL);
    state.horizon_edit = CreateWindowA("EDIT", "3", WS_CHILD | WS_VISIBLE |
                                       WS_BORDER | ES_AUTOHSCROLL, 130, 0, 100, 26,
                                       window, (HMENU) CONTROL_ID_HORIZON,
                                       GetModuleHandleA(NULL), NULL);
    state.allocate_button = CreateWindowA("BUTTON", "Allocate", WS_CHILD | WS_VISIBLE,
                                          240, 0, 90, 26, window,
                                          (HMENU) CONTROL_ID_ALLOCATE,
                                          GetModuleHandleA(NULL), NULL);
    state.calloc_button = CreateWindowA("BUTTON", "Calloc", WS_CHILD | WS_VISIBLE,
                                        336, 0, 80, 26, window,
                                        (HMENU) CONTROL_ID_CALLOC,
                                        GetModuleHandleA(NULL), NULL);
    state.realloc_button = CreateWindowA("BUTTON", "Resize", WS_CHILD | WS_VISIBLE,
                                         422, 0, 80, 26, window,
                                         (HMENU) CONTROL_ID_REALLOCATE,
                                         GetModuleHandleA(NULL), NULL);
    state.free_button = CreateWindowA("BUTTON", "Free ID", WS_CHILD | WS_VISIBLE,
                                      508, 0, 80, 26, window,
                                      (HMENU) CONTROL_ID_FREE,
                                      GetModuleHandleA(NULL), NULL);
    state.predict_button = CreateWindowA("BUTTON", "Predict", WS_CHILD | WS_VISIBLE,
                                         594, 0, 80, 26, window,
                                         (HMENU) CONTROL_ID_PREDICT,
                                         GetModuleHandleA(NULL), NULL);
    state.demo_button = CreateWindowA("BUTTON", "Demo", WS_CHILD | WS_VISIBLE,
                                      680, 0, 70, 26, window,
                                      (HMENU) CONTROL_ID_DEMO,
                                      GetModuleHandleA(NULL), NULL);
    state.leaks_button = CreateWindowA("BUTTON", "Check leaks", WS_CHILD | WS_VISIBLE,
                                       756, 0, 100, 26, window,
                                       (HMENU) CONTROL_ID_LEAKS,
                                       GetModuleHandleA(NULL), NULL);
    set_font(state.size_edit, state.body_font);
    set_font(state.horizon_edit, state.body_font);
    set_font(state.allocate_button, state.small_font);
    set_font(state.calloc_button, state.small_font);
    set_font(state.realloc_button, state.small_font);
    set_font(state.free_button, state.small_font);
    set_font(state.predict_button, state.small_font);
    set_font(state.demo_button, state.small_font);
    set_font(state.leaks_button, state.small_font);
}

static void position_controls(HWND window)
{
    RECT rect;
    int y;

    GetClientRect(window, &rect);
    y = rect.bottom - 44;
    MoveWindow(state.size_edit, 24, y, 100, 26, TRUE);
    MoveWindow(state.horizon_edit, 130, y, 100, 26, TRUE);
    MoveWindow(state.allocate_button, 240, y, 90, 26, TRUE);
    MoveWindow(state.calloc_button, 336, y, 80, 26, TRUE);
    MoveWindow(state.realloc_button, 422, y, 80, 26, TRUE);
    MoveWindow(state.free_button, 508, y, 80, 26, TRUE);
    MoveWindow(state.predict_button, 594, y, 80, 26, TRUE);
    MoveWindow(state.demo_button, 680, y, 70, 26, TRUE);
    MoveWindow(state.leaks_button, 756, y, 100, 26, TRUE);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                    WPARAM wparam, LPARAM lparam)
{
    PAINTSTRUCT paint;
    RECT client;
    HBRUSH background;

    switch (message) {
    case WM_CREATE:
        state.window = window;
        memory_init();
        predictor_init(1024);
        predictor_record(0);
        create_controls(window);
        add_log("session started", COLOR_GREEN);
        return 0;
    case WM_SIZE:
        position_controls(window);
        InvalidateRect(window, NULL, FALSE);
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *) lparam)->ptMinTrackSize.x = 1120;
        ((MINMAXINFO *) lparam)->ptMinTrackSize.y = 760;
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case CONTROL_ID_ALLOCATE: perform_allocate(ALLOCATION_MALLOC); break;
        case CONTROL_ID_CALLOC: perform_allocate(ALLOCATION_CALLOC); break;
        case CONTROL_ID_REALLOCATE: perform_reallocate(); break;
        case CONTROL_ID_FREE: perform_free(); break;
        case CONTROL_ID_PREDICT: perform_prediction(); break;
        case CONTROL_ID_DEMO: perform_demo(); break;
        case CONTROL_ID_LEAKS: show_leaks(); break;
        default: break;
        }
        return 0;
    case WM_CLOSE:
        if (MessageBoxA(window, "Release tracked allocations and exit?",
                        "Exit MemPredictC", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            memory_release_all();
            DestroyWindow(window);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        GetClientRect(window, &client);
        BeginPaint(window, &paint);
        background = CreateSolidBrush(COLOR_BG);
        FillRect(paint.hdc, &client, background);
        DeleteObject(background);
        draw_dashboard(paint.hdc, client);
        EndPaint(window, &paint);
        return 0;
    default:
        return DefWindowProcA(window, message, wparam, lparam);
    }
}

int ui_run(void)
{
    WNDCLASSA window_class;
    HWND window;
    MSG message;
    HINSTANCE instance = GetModuleHandleA(NULL);

    memset(&state, 0, sizeof(state));
    state.title_font = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH, "Segoe UI");
    state.body_font = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH, "Segoe UI");
    state.small_font = CreateFontA(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH, "Consolas");
    memset(&window_class, 0, sizeof(window_class));
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    window_class.lpszClassName = APP_CLASS;
    if (!RegisterClassA(&window_class) &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return 0;
    }
    window = CreateWindowExA(0, APP_CLASS, APP_TITLE, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1120, 760,
                             NULL, NULL, instance, NULL);
    if (window == NULL) {
        return 0;
    }
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    while (GetMessageA(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    DeleteObject(state.title_font);
    DeleteObject(state.body_font);
    DeleteObject(state.small_font);
    return 1;
}

#else

int ui_run(void)
{
    return 0;
}

#endif
