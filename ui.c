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
#define SIDEBAR_WIDTH 224
#define CONTROL_SIZE 3001
#define CONTROL_HORIZON 3002
#define CONTROL_ALLOCATE 3003
#define CONTROL_CALLOC 3004
#define CONTROL_RESIZE 3005
#define CONTROL_FREE 3006
#define CONTROL_PREDICT 3007
#define CONTROL_DEMO 3008
#define CONTROL_LEAKS 3009
#define NAV_DASHBOARD 3101
#define NAV_ALLOCATIONS 3102
#define NAV_PREDICTION 3103
#define NAV_LEAKS 3104
#define NAV_ACTIVITY 3105
#define NAV_SETTINGS 3106
#define COLOR_BG RGB(13, 17, 23)
#define COLOR_SIDEBAR RGB(16, 22, 30)
#define COLOR_PANEL RGB(22, 27, 34)
#define COLOR_PANEL_ALT RGB(28, 35, 44)
#define COLOR_BORDER RGB(48, 57, 69)
#define COLOR_TEXT RGB(230, 237, 243)
#define COLOR_MUTED RGB(139, 148, 158)
#define COLOR_CYAN RGB(34, 211, 238)
#define COLOR_GREEN RGB(63, 185, 80)
#define COLOR_YELLOW RGB(210, 153, 34)
#define COLOR_RED RGB(248, 81, 73)
#define COLOR_BLUE RGB(88, 166, 255)
#define MAX_LOG_ENTRIES 24
#define MAX_TEXT 160

typedef enum {
    PAGE_DASHBOARD,
    PAGE_ALLOCATIONS,
    PAGE_PREDICTION,
    PAGE_LEAKS,
    PAGE_ACTIVITY,
    PAGE_SETTINGS
} Page;

typedef struct {
    char text[MAX_TEXT];
    COLORREF color;
} ActivityEntry;

typedef struct {
    HWND window;
    HWND size_edit;
    HWND horizon_edit;
    HWND allocate_button;
    HWND calloc_button;
    HWND resize_button;
    HWND free_button;
    HWND predict_button;
    HWND demo_button;
    HWND leaks_button;
    HWND nav[6];
    HFONT title_font;
    HFONT heading_font;
    HFONT body_font;
    HFONT small_font;
    ActivityEntry log[MAX_LOG_ENTRIES];
    size_t log_count;
    Page page;
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

static void record_current_memory(void)
{
    predictor_record(memory_get_stats().current_bytes);
}

static int read_value(HWND edit, size_t *value)
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

static void set_status(const char *message, COLORREF color)
{
    add_log(message, color);
    InvalidateRect(state.window, NULL, FALSE);
}

static void perform_allocate(AllocationKind kind)
{
    size_t size;
    unsigned long id;
    char message[MAX_TEXT];

    if (!read_value(state.size_edit, &size)) {
        set_status("Invalid size. Enter a positive whole number.", COLOR_RED);
        return;
    }
    if (!memory_allocate(size, kind, &id)) {
        set_status("Allocation failed. Check the size or registry capacity.",
                   COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "%s allocation created: ID %lu, %lu bytes",
             kind == ALLOCATION_CALLOC ? "calloc" : "malloc", id,
             (unsigned long) size);
    set_status(message, COLOR_GREEN);
}

static void perform_resize(void)
{
    size_t id;
    size_t new_size;
    char message[MAX_TEXT];

    if (!read_value(state.size_edit, &id) || id > (size_t) ULONG_MAX ||
        !read_value(state.horizon_edit, &new_size)) {
        set_status("Resize requires an allocation ID and a new size.", COLOR_RED);
        return;
    }
    if (!memory_reallocate((unsigned long) id, new_size)) {
        set_status("Resize failed. Check that the ID is active.", COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "allocation %lu resized to %lu bytes",
             (unsigned long) id, (unsigned long) new_size);
    set_status(message, COLOR_CYAN);
}

static void perform_free(void)
{
    size_t id;
    char message[MAX_TEXT];

    if (!read_value(state.size_edit, &id) || id > (size_t) ULONG_MAX ||
        !memory_free((unsigned long) id)) {
        set_status("Free failed. Enter an active allocation ID.", COLOR_RED);
        return;
    }
    record_current_memory();
    snprintf(message, sizeof(message), "allocation %lu released",
             (unsigned long) id);
    set_status(message, COLOR_GREEN);
}

static void perform_prediction(void)
{
    size_t steps;
    PredictionStats prediction;
    char message[MAX_TEXT];

    if (!read_value(state.horizon_edit, &steps)) {
        set_status("Invalid horizon. Enter a positive number.", COLOR_RED);
        return;
    }
    prediction = predictor_get_stats(steps);
    if (!prediction.has_prediction) {
        set_status("Prediction needs at least two observations.", COLOR_YELLOW);
        return;
    }
    snprintf(message, sizeof(message), "prediction calculated: %.0f bytes in %lu steps",
             prediction.predicted_memory, (unsigned long) steps);
    set_status(message, prediction.warning ? COLOR_YELLOW : COLOR_CYAN);
}

static void perform_demo(void)
{
    unsigned long first;
    unsigned long second;
    unsigned long leaked;

    if (memory_allocate(120, ALLOCATION_MALLOC, &first)) {
        record_current_memory();
    }
    if (memory_allocate(80, ALLOCATION_CALLOC, &second)) {
        record_current_memory();
    }
    if (memory_reallocate(first, 200)) {
        record_current_memory();
    }
    if (memory_free(second)) {
        record_current_memory();
    }
    if (memory_allocate(900, ALLOCATION_MALLOC, &leaked)) {
        record_current_memory();
        (void) leaked;
    }
    set_status("demo completed: active allocation retained for leak review",
               COLOR_YELLOW);
}

static void perform_leak_check(void)
{
    MemoryStats stats = memory_get_stats();

    if (stats.active_allocations == 0) {
        set_status("leak check passed: no outstanding allocations", COLOR_GREEN);
    } else {
        set_status("potential leak detected: review active allocations", COLOR_RED);
    }
}

static void show_control(HWND control, int visible)
{
    ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
}

static void draw_text(HDC dc, const char *text, int x, int y, COLORREF color,
                      HFONT font)
{
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, font);
    TextOutA(dc, x, y, text, (int) strlen(text));
}

static void draw_wrapped(HDC dc, const char *text, RECT rect, COLORREF color,
                         HFONT font)
{
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    SelectObject(dc, font);
    DrawTextA(dc, text, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);
}

static void fill_round_panel(HDC dc, RECT rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HGDIOBJ old_brush = SelectObject(dc, brush);
    HGDIOBJ old_pen = SelectObject(dc, pen);

    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, 10, 10);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
}

static void draw_card(HDC dc, RECT rect, const char *label, size_t value,
                      COLORREF accent)
{
    char value_text[64];

    fill_round_panel(dc, rect, COLOR_PANEL);
    draw_text(dc, label, rect.left + 16, rect.top + 13, COLOR_MUTED,
              state.small_font);
    snprintf(value_text, sizeof(value_text), "%lu", (unsigned long) value);
    draw_text(dc, value_text, rect.left + 16, rect.top + 35, accent,
              state.heading_font);
}

static void draw_graph(HDC dc, RECT rect)
{
    size_t observations[MAX_OBSERVATIONS];
    size_t count = predictor_copy_observations(observations, MAX_OBSERVATIONS);
    size_t i;
    size_t max_value = 1;
    int left = rect.left + 42;
    int right = rect.right - 20;
    int top = rect.top + 42;
    int bottom = rect.bottom - 30;
    HPEN grid = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HPEN line = CreatePen(PS_SOLID, 2, COLOR_CYAN);
    HGDIOBJ old;
    char label[64];

    fill_round_panel(dc, rect, COLOR_PANEL);
    draw_text(dc, "Memory usage", rect.left + 18, rect.top + 16, COLOR_TEXT,
              state.body_font);
    if (count < 2) {
        draw_text(dc, "No memory activity yet", rect.left + 18, rect.top + 80,
                  COLOR_CYAN, state.heading_font);
        draw_wrapped(dc, "Allocate memory or run the demo to see live statistics.",
                     (RECT) {rect.left + 18, rect.top + 116, rect.right - 18,
                             rect.bottom - 18}, COLOR_MUTED, state.body_font);
        DeleteObject(grid);
        DeleteObject(line);
        return;
    }
    for (i = 0; i < count; ++i) {
        if (observations[i] > max_value) {
            max_value = observations[i];
        }
    }
    old = SelectObject(dc, grid);
    for (i = 1; i < 4; ++i) {
        int y = top + (bottom - top) * (int) i / 4;
        MoveToEx(dc, left, y, NULL);
        LineTo(dc, right, y);
    }
    MoveToEx(dc, left, top, NULL);
    LineTo(dc, left, bottom);
    SelectObject(dc, line);
    for (i = 0; i < count; ++i) {
        int x = left + (int) ((right - left) * (double) i /
                              (double) (count - 1));
        int y = bottom - (int) ((bottom - top) * (double) observations[i] /
                                (double) max_value);
        if (i == 0) {
            MoveToEx(dc, x, y, NULL);
        } else {
            LineTo(dc, x, y);
        }
    }
    SelectObject(dc, old);
    snprintf(label, sizeof(label), "%lu bytes", (unsigned long) max_value);
    draw_text(dc, label, left + 6, top - 18, COLOR_MUTED, state.small_font);
    DeleteObject(grid);
    DeleteObject(line);
}

static void draw_header(HDC dc, RECT client, const char *title,
                        const char *description)
{
    draw_text(dc, title, SIDEBAR_WIDTH + 32, 28, COLOR_TEXT, state.title_font);
    draw_text(dc, description, SIDEBAR_WIDTH + 34, 61, COLOR_MUTED,
              state.body_font);
    draw_text(dc, "LIVE SESSION", client.right - 122, 34, COLOR_GREEN,
              state.small_font);
}

static void draw_sidebar(HDC dc, RECT client)
{
    RECT side = {0, 0, SIDEBAR_WIDTH, client.bottom};
    RECT footer = {18, client.bottom - 72, SIDEBAR_WIDTH - 18, client.bottom - 18};
    HBRUSH brush = CreateSolidBrush(COLOR_SIDEBAR);
    FillRect(dc, &side, brush);
    DeleteObject(brush);
    draw_text(dc, "MemPredictC", 24, 28, COLOR_CYAN, state.title_font);
    draw_text(dc, "MEMORY OBSERVABILITY", 26, 62, COLOR_MUTED, state.small_font);
    draw_text(dc, "v1.0.0", footer.left, footer.top, COLOR_MUTED, state.small_font);
    draw_text(dc, "C11 ENGINE", footer.left, footer.top + 22, COLOR_GREEN,
              state.small_font);
}

static void draw_prediction_card(HDC dc, RECT rect, size_t steps)
{
    PredictionStats prediction = predictor_get_stats(steps);
    char text[96];

    fill_round_panel(dc, rect, COLOR_PANEL);
    draw_text(dc, "Trend prediction", rect.left + 18, rect.top + 16, COLOR_TEXT,
              state.body_font);
    snprintf(text, sizeof(text), "Current  %lu bytes",
             (unsigned long) prediction.current_memory);
    draw_text(dc, text, rect.left + 18, rect.top + 54, COLOR_MUTED,
              state.small_font);
    snprintf(text, sizeof(text), "Average growth  %.2f bytes/op",
             prediction.average_growth);
    draw_text(dc, text, rect.left + 18, rect.top + 76, COLOR_MUTED,
              state.small_font);
    if (!prediction.has_prediction) {
        draw_text(dc, "Waiting for observations", rect.left + 18, rect.top + 108,
                  COLOR_YELLOW, state.body_font);
    } else {
        snprintf(text, sizeof(text), "Next %lu  %.0f bytes",
                 (unsigned long) steps, prediction.predicted_memory);
        draw_text(dc, text, rect.left + 18, rect.top + 108, COLOR_CYAN,
                  state.body_font);
        draw_text(dc, prediction.warning ? "Threshold warning" : "Below threshold",
                  rect.left + 18, rect.top + 134,
                  prediction.warning ? COLOR_YELLOW : COLOR_GREEN,
                  state.small_font);
    }
}

static void draw_activity_panel(HDC dc, RECT rect, size_t limit)
{
    size_t i;

    fill_round_panel(dc, rect, COLOR_PANEL);
    draw_text(dc, "Recent activity", rect.left + 18, rect.top + 16, COLOR_TEXT,
              state.body_font);
    if (state.log_count == 0) {
        draw_text(dc, "No activity recorded", rect.left + 18, rect.top + 54,
                  COLOR_MUTED, state.body_font);
        return;
    }
    for (i = 0; i < state.log_count && i < limit; ++i) {
        draw_text(dc, state.log[i].text, rect.left + 18,
                  rect.top + 52 + (int) i * 25, state.log[i].color,
                  state.small_font);
    }
}

static void draw_dashboard(HDC dc, RECT client)
{
    MemoryStats stats = memory_get_stats();
    int width = client.right - SIDEBAR_WIDTH - 64;
    int card_width = (width - 20) / 3;
    RECT card;
    RECT graph;
    RECT prediction;
    RECT activity;
    size_t i;
    size_t values[6];

    draw_header(dc, client, "Dashboard", "Live overview of tracked memory activity.");
    values[0] = stats.current_bytes;
    values[1] = stats.peak_bytes;
    values[2] = stats.total_allocated;
    values[3] = stats.total_freed;
    values[4] = stats.active_allocations;
    values[5] = stats.allocation_operations;
    for (i = 0; i < 6; ++i) {
        card.left = SIDEBAR_WIDTH + 32 + (int) (i % 3) * (card_width + 10);
        card.top = 102 + (int) (i / 3) * 78;
        card.right = card.left + card_width;
        card.bottom = card.top + 64;
        draw_card(dc, card, (const char *[]) {"CURRENT MEMORY", "PEAK MEMORY",
                  "TOTAL ALLOCATED", "TOTAL FREED", "ACTIVE ALLOCATIONS",
                  "ALLOCATION OPERATIONS"}[i], values[i],
                  i == 0 ? COLOR_CYAN : i == 4 && values[i] > 0 ?
                  COLOR_YELLOW : COLOR_GREEN);
    }
    graph.left = SIDEBAR_WIDTH + 32;
    graph.top = 270;
    graph.right = SIDEBAR_WIDTH + 32 + (width * 2 / 3);
    graph.bottom = client.bottom - 32;
    draw_graph(dc, graph);
    prediction.left = graph.right + 14;
    prediction.right = client.right - 32;
    prediction.top = graph.top;
    prediction.bottom = graph.top + 178;
    draw_prediction_card(dc, prediction, 3);
    activity.left = prediction.left;
    activity.right = prediction.right;
    activity.top = prediction.bottom + 14;
    activity.bottom = client.bottom - 32;
    draw_activity_panel(dc, activity, 8);
}

static void draw_allocation_page(HDC dc, RECT client)
{
    MemoryStats stats = memory_get_stats();
    AllocationRecord records[MAX_ALLOCATIONS];
    size_t count = memory_get_active_allocations(records, MAX_ALLOCATIONS);
    RECT panel;
    char text[128];
    size_t i;

    draw_header(dc, client, "Allocations", "Create, resize, and release tracked blocks.");
    panel.left = SIDEBAR_WIDTH + 32;
    panel.right = client.right - 32;
    panel.top = 102;
    panel.bottom = 178;
    fill_round_panel(dc, panel, COLOR_PANEL);
    draw_text(dc, "Allocation controls", panel.left + 18, panel.top + 14,
              COLOR_TEXT, state.body_font);
    draw_text(dc, "Size / ID", panel.left + 18, panel.top + 43, COLOR_MUTED,
              state.small_font);
    draw_text(dc, "New size", panel.left + 222, panel.top + 43, COLOR_MUTED,
              state.small_font);
    panel.top = 198;
    panel.bottom = client.bottom - 32;
    fill_round_panel(dc, panel, COLOR_PANEL);
    draw_text(dc, "Active allocation table", panel.left + 18, panel.top + 16,
              COLOR_TEXT, state.body_font);
    draw_text(dc, "ID", panel.left + 20, panel.top + 54, COLOR_MUTED, state.small_font);
    draw_text(dc, "TYPE", panel.left + 110, panel.top + 54, COLOR_MUTED, state.small_font);
    draw_text(dc, "SIZE", panel.left + 250, panel.top + 54, COLOR_MUTED, state.small_font);
    draw_text(dc, "STATUS", panel.left + 390, panel.top + 54, COLOR_MUTED, state.small_font);
    for (i = 0; i < count && i < 18; ++i) {
        snprintf(text, sizeof(text), "%lu", records[i].id);
        draw_text(dc, text, panel.left + 20, panel.top + 84 + (int) i * 27,
                  COLOR_CYAN, state.body_font);
        draw_text(dc, records[i].kind == ALLOCATION_CALLOC ? "calloc" : "malloc",
                  panel.left + 110, panel.top + 84 + (int) i * 27,
                  COLOR_TEXT, state.body_font);
        snprintf(text, sizeof(text), "%lu bytes", (unsigned long) records[i].size);
        draw_text(dc, text, panel.left + 250, panel.top + 84 + (int) i * 27,
                  COLOR_TEXT, state.body_font);
        draw_text(dc, "active", panel.left + 390, panel.top + 84 + (int) i * 27,
                  COLOR_GREEN, state.body_font);
    }
    if (count == 0) {
        draw_text(dc, "No active allocations. Use the controls above or run the demo.",
                  panel.left + 20, panel.top + 88, COLOR_MUTED, state.body_font);
    }
    (void) stats;
}

static void draw_prediction_page(HDC dc, RECT client)
{
    PredictionStats prediction = predictor_get_stats(3);
    RECT graph;
    RECT card;
    char text[128];

    draw_header(dc, client, "Memory Prediction",
                "A transparent trend estimate based on recent observations.");
    card.left = SIDEBAR_WIDTH + 32;
    card.top = 102;
    card.right = client.right - 32;
    card.bottom = 188;
    fill_round_panel(dc, card, COLOR_PANEL);
    snprintf(text, sizeof(text), "Current: %lu bytes    Average growth: %.2f bytes/op",
             (unsigned long) prediction.current_memory, prediction.average_growth);
    draw_text(dc, text, card.left + 20, card.top + 20, COLOR_TEXT, state.body_font);
    snprintf(text, sizeof(text), "Next 3 operations: %s",
             prediction.has_prediction ? "projection available" : "waiting for data");
    draw_text(dc, text, card.left + 20, card.top + 52,
              prediction.warning ? COLOR_YELLOW : COLOR_CYAN, state.body_font);
    graph.left = SIDEBAR_WIDTH + 32;
    graph.top = 210;
    graph.right = client.right - 32;
    graph.bottom = client.bottom - 32;
    draw_graph(dc, graph);
    draw_wrapped(dc, "This is a simple mathematical trend estimate, not trained AI "
                     "or machine learning. It only uses allocations tracked by "
                     "MemPredictC.",
                 (RECT) {graph.left + 20, graph.bottom - 64, graph.right - 20,
                         graph.bottom - 18}, COLOR_MUTED, state.small_font);
}

static void draw_leaks_page(HDC dc, RECT client)
{
    MemoryStats stats = memory_get_stats();
    AllocationRecord records[MAX_ALLOCATIONS];
    size_t count = memory_get_active_allocations(records, MAX_ALLOCATIONS);
    RECT panel;
    char text[128];
    size_t i;

    draw_header(dc, client, "Leak Detection",
                "Review active allocations tracked by the C11 engine.");
    panel.left = SIDEBAR_WIDTH + 32;
    panel.right = client.right - 32;
    panel.top = 102;
    panel.bottom = 188;
    fill_round_panel(dc, panel, COLOR_PANEL);
    draw_text(dc, count == 0 ? "HEALTHY" : "POTENTIAL LEAKS",
              panel.left + 20, panel.top + 20,
              count == 0 ? COLOR_GREEN : COLOR_RED, state.heading_font);
    snprintf(text, sizeof(text), "%lu active allocations   |   %lu outstanding bytes",
             (unsigned long) stats.active_allocations,
             (unsigned long) stats.current_bytes);
    draw_text(dc, text, panel.left + 20, panel.top + 55, COLOR_MUTED, state.body_font);
    panel.top = 210;
    panel.bottom = client.bottom - 32;
    fill_round_panel(dc, panel, COLOR_PANEL);
    draw_text(dc, "Outstanding allocations", panel.left + 18, panel.top + 16,
              COLOR_TEXT, state.body_font);
    for (i = 0; i < count && i < 20; ++i) {
        snprintf(text, sizeof(text), "ID %lu        %lu bytes        unreleased",
                 records[i].id, (unsigned long) records[i].size);
        draw_text(dc, text, panel.left + 20, panel.top + 58 + (int) i * 28,
                  COLOR_RED, state.body_font);
    }
    if (count == 0) {
        draw_text(dc, "No unreleased allocations", panel.left + 20, panel.top + 60,
                  COLOR_GREEN, state.body_font);
        draw_text(dc, "Your tracked memory is clear.", panel.left + 20, panel.top + 88,
                  COLOR_MUTED, state.body_font);
    }
}

static void draw_activity_page(HDC dc, RECT client)
{
    RECT panel;
    size_t i;

    draw_header(dc, client, "Activity Log", "Chronological events from this session.");
    panel.left = SIDEBAR_WIDTH + 32;
    panel.right = client.right - 32;
    panel.top = 102;
    panel.bottom = client.bottom - 32;
    fill_round_panel(dc, panel, COLOR_PANEL);
    if (state.log_count == 0) {
        draw_text(dc, "No activity recorded yet.", panel.left + 20, panel.top + 24,
                  COLOR_MUTED, state.body_font);
        return;
    }
    for (i = 0; i < state.log_count; ++i) {
        draw_text(dc, state.log[i].text, panel.left + 24,
                  panel.top + 24 + (int) i * 27, state.log[i].color,
                  state.body_font);
    }
}

static void draw_settings_page(HDC dc, RECT client)
{
    RECT panel;

    draw_header(dc, client, "Settings", "Runtime information and project boundaries.");
    panel.left = SIDEBAR_WIDTH + 32;
    panel.right = client.right - 32;
    panel.top = 102;
    panel.bottom = 300;
    fill_round_panel(dc, panel, COLOR_PANEL);
    draw_text(dc, "Engine status", panel.left + 20, panel.top + 20, COLOR_TEXT,
              state.body_font);
    draw_text(dc, "C11 memory engine: online", panel.left + 20, panel.top + 58,
              COLOR_GREEN, state.body_font);
    draw_text(dc, "Registry capacity: 256 active allocations", panel.left + 20,
              panel.top + 88, COLOR_MUTED, state.body_font);
    draw_text(dc, "Warning threshold: 1024 bytes", panel.left + 20,
              panel.top + 118, COLOR_MUTED, state.body_font);
    draw_wrapped(dc, "MemPredictC tracks only allocations created through its own "
                     "wrapper functions. It does not monitor other processes or "
                     "all operating-system memory.",
                 (RECT) {panel.left + 20, panel.top + 148, panel.right - 20,
                         panel.bottom - 20}, COLOR_MUTED, state.body_font);
}

static void draw_ui(HDC dc, RECT client)
{
    HBRUSH background = CreateSolidBrush(COLOR_BG);

    FillRect(dc, &client, background);
    DeleteObject(background);
    draw_sidebar(dc, client);
    switch (state.page) {
    case PAGE_DASHBOARD: draw_dashboard(dc, client); break;
    case PAGE_ALLOCATIONS: draw_allocation_page(dc, client); break;
    case PAGE_PREDICTION: draw_prediction_page(dc, client); break;
    case PAGE_LEAKS: draw_leaks_page(dc, client); break;
    case PAGE_ACTIVITY: draw_activity_page(dc, client); break;
    case PAGE_SETTINGS: draw_settings_page(dc, client); break;
    }
}

static void draw_button(const DRAWITEMSTRUCT *item)
{
    char text[64];
    COLORREF fill = COLOR_PANEL;
    COLORREF foreground = COLOR_TEXT;
    RECT rect = item->rcItem;
    HBRUSH brush;
    HPEN pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;

    GetWindowTextA(item->hwndItem, text, sizeof(text));
    if (item->CtlID >= NAV_DASHBOARD && item->CtlID <= NAV_SETTINGS) {
        fill = (int) item->CtlID - NAV_DASHBOARD == (int) state.page ?
            RGB(24, 56, 67) : COLOR_SIDEBAR;
        foreground = (int) item->CtlID - NAV_DASHBOARD == (int) state.page ?
            COLOR_CYAN : COLOR_MUTED;
    } else if (item->CtlID == CONTROL_DEMO) {
        fill = RGB(24, 56, 67);
        foreground = COLOR_CYAN;
    } else if (item->CtlID == CONTROL_LEAKS) {
        fill = RGB(60, 38, 38);
        foreground = COLOR_RED;
    }
    brush = CreateSolidBrush(fill);
    pen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    old_brush = SelectObject(item->hDC, brush);
    old_pen = SelectObject(item->hDC, pen);
    RoundRect(item->hDC, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
    SelectObject(item->hDC, old_brush);
    SelectObject(item->hDC, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
    draw_text(item->hDC, text, rect.left + 12,
              rect.top + ((rect.bottom - rect.top - 14) / 2),
              foreground, state.small_font);
}

static void position_controls(HWND window)
{
    RECT rect;
    int x;
    int y;

    GetClientRect(window, &rect);
    x = SIDEBAR_WIDTH + 50;
    y = 142;
    if (state.page == PAGE_PREDICTION) {
        MoveWindow(state.horizon_edit, rect.right - 310, 68, 150, 28, TRUE);
    } else {
        MoveWindow(state.horizon_edit, x + 205, y, 150, 28, TRUE);
    }
    MoveWindow(state.size_edit, x, y, 150, 28, TRUE);
    MoveWindow(state.allocate_button, x + 375, y, 90, 28, TRUE);
    MoveWindow(state.calloc_button, x + 470, y, 90, 28, TRUE);
    MoveWindow(state.resize_button, x + 565, y, 90, 28, TRUE);
    MoveWindow(state.free_button, x + 660, y, 90, 28, TRUE);
    MoveWindow(state.predict_button, rect.right - 140, 68, 100, 28, TRUE);
    MoveWindow(state.demo_button, rect.right - 245, 68, 100, 28, TRUE);
    MoveWindow(state.leaks_button, rect.right - 350, 68, 100, 28, TRUE);
}

static void update_controls(void)
{
    int allocation_page = state.page == PAGE_ALLOCATIONS;
    int prediction_page = state.page == PAGE_PREDICTION;
    int dashboard_page = state.page == PAGE_DASHBOARD;

    show_control(state.size_edit, allocation_page);
    show_control(state.horizon_edit, allocation_page || prediction_page);
    show_control(state.allocate_button, allocation_page);
    show_control(state.calloc_button, allocation_page);
    show_control(state.resize_button, allocation_page);
    show_control(state.free_button, allocation_page);
    show_control(state.predict_button, prediction_page);
    show_control(state.demo_button, dashboard_page);
    show_control(state.leaks_button, state.page == PAGE_LEAKS);
    InvalidateRect(state.window, NULL, FALSE);
}

static HWND make_control(const char *class_name, const char *text, DWORD style,
                         int id, HWND window)
{
    if (strcmp(class_name, "BUTTON") == 0) {
        style = BS_OWNERDRAW;
    }
    HWND control = CreateWindowA(class_name, text, WS_CHILD | WS_VISIBLE | style,
                                 0, 0, 0, 0, window, (HMENU) id,
                                 GetModuleHandleA(NULL), NULL);
    set_font(control, state.body_font);
    return control;
}

static void create_controls(HWND window)
{
    int i;

    state.size_edit = make_control("EDIT", "64", WS_BORDER | ES_AUTOHSCROLL,
                                   CONTROL_SIZE, window);
    state.horizon_edit = make_control("EDIT", "3", WS_BORDER | ES_AUTOHSCROLL,
                                      CONTROL_HORIZON, window);
    state.allocate_button = make_control("BUTTON", "malloc", BS_PUSHBUTTON,
                                         CONTROL_ALLOCATE, window);
    state.calloc_button = make_control("BUTTON", "calloc", BS_PUSHBUTTON,
                                       CONTROL_CALLOC, window);
    state.resize_button = make_control("BUTTON", "realloc", BS_PUSHBUTTON,
                                       CONTROL_RESIZE, window);
    state.free_button = make_control("BUTTON", "free", BS_PUSHBUTTON,
                                     CONTROL_FREE, window);
    state.predict_button = make_control("BUTTON", "Predict", BS_PUSHBUTTON,
                                        CONTROL_PREDICT, window);
    state.demo_button = make_control("BUTTON", "Run Demo", BS_PUSHBUTTON,
                                     CONTROL_DEMO, window);
    state.leaks_button = make_control("BUTTON", "Check Leaks", BS_PUSHBUTTON,
                                      CONTROL_LEAKS, window);
    for (i = 0; i < 6; ++i) {
        state.nav[i] = make_control("BUTTON",
            (const char *[]) {"Dashboard", "Allocations", "Prediction",
                              "Leak Detection", "Activity Log", "Settings"}[i],
            BS_PUSHBUTTON, NAV_DASHBOARD + i, window);
        set_font(state.nav[i], state.body_font);
    }
}

static void position_nav(void)
{
    int i;

    for (i = 0; i < 6; ++i) {
        MoveWindow(state.nav[i], 18, 92 + i * 44, SIDEBAR_WIDTH - 36, 34, TRUE);
    }
}

static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                    WPARAM wparam, LPARAM lparam)
{
    PAINTSTRUCT paint;
    RECT client;
    int command = LOWORD(wparam);

    switch (message) {
    case WM_CREATE:
        state.window = window;
        memory_init();
        predictor_init(1024);
        predictor_record(0);
        state.page = PAGE_DASHBOARD;
        create_controls(window);
        add_log("session started", COLOR_GREEN);
        update_controls();
        return 0;
    case WM_SIZE:
        position_controls(window);
        position_nav();
        InvalidateRect(window, NULL, FALSE);
        return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *) lparam)->ptMinTrackSize.x = 1040;
        ((MINMAXINFO *) lparam)->ptMinTrackSize.y = 680;
        return 0;
    case WM_COMMAND:
        if (command >= NAV_DASHBOARD && command <= NAV_SETTINGS) {
            state.page = (Page) (command - NAV_DASHBOARD);
            update_controls();
            return 0;
        }
        switch (command) {
        case CONTROL_ALLOCATE: perform_allocate(ALLOCATION_MALLOC); break;
        case CONTROL_CALLOC: perform_allocate(ALLOCATION_CALLOC); break;
        case CONTROL_RESIZE: perform_resize(); break;
        case CONTROL_FREE: perform_free(); break;
        case CONTROL_PREDICT: perform_prediction(); break;
        case CONTROL_DEMO: perform_demo(); break;
        case CONTROL_LEAKS: perform_leak_check(); break;
        default: break;
        }
        InvalidateRect(window, NULL, FALSE);
        return 0;
    case WM_DRAWITEM:
        draw_button((const DRAWITEMSTRUCT *) lparam);
        return TRUE;
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
        draw_ui(paint.hdc, client);
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

    {
        typedef BOOL (WINAPI *DpiAwareFunction)(void);
        HMODULE user32 = LoadLibraryA("user32.dll");
        DpiAwareFunction set_dpi_aware = user32 == NULL ? NULL :
            (DpiAwareFunction) GetProcAddress(user32, "SetProcessDPIAware");
        if (set_dpi_aware != NULL) {
            set_dpi_aware();
        }
        if (user32 != NULL) {
            FreeLibrary(user32);
        }
    }
    memset(&state, 0, sizeof(state));
    state.title_font = CreateFontA(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                   DEFAULT_PITCH, "Segoe UI");
    state.heading_font = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH, "Segoe UI");
    state.body_font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
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
    window = CreateWindowExA(0, APP_CLASS, APP_TITLE,
                             WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1366, 820,
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
    DeleteObject(state.heading_font);
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
