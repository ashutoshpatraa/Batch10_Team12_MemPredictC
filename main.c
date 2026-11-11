#include "memory.h"
#include "predictor.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DEFAULT_THRESHOLD 1024

static int read_unsigned(const char *prompt, size_t *value)
{
    char input[64];
    char *end;
    unsigned long long parsed;

    printf("%s", prompt);
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 0;
    }
    input[strcspn(input, "\n")] = '\0';
    if (input[0] == '\0' || input[0] == '-') {
        return 0;
    }
    errno = 0;
    parsed = strtoull(input, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed > (unsigned long long) SIZE_MAX) {
        return 0;
    }
    *value = (size_t) parsed;
    return 1;
}

static void record_current_memory(void)
{
    predictor_record(memory_get_stats().current_bytes);
}

static void allocate_interactively(AllocationKind kind)
{
    size_t size;
    unsigned long id;

    if (!read_unsigned("Enter number of bytes: ", &size) || size == 0) {
        printf("Invalid size. Enter a positive whole number.\n");
        return;
    }
    if (memory_allocate(size, kind, &id)) {
        printf("Allocation successful. ID: %lu, size: %zu bytes\n", id, size);
        record_current_memory();
    } else {
        printf("Allocation failed: invalid request, registry full, or insufficient memory.\n");
    }
}

static void reallocate_interactively(void)
{
    size_t id;
    size_t size;

    if (!read_unsigned("Enter allocation ID: ", &id) ||
        id > (size_t) ULONG_MAX ||
        !read_unsigned("Enter new size in bytes: ", &size) || size == 0) {
        printf("Invalid ID or size.\n");
        return;
    }
    if (memory_reallocate((unsigned long) id, size)) {
        printf("Reallocation successful.\n");
        record_current_memory();
    } else {
        printf("Reallocation failed: check the ID, size, or available memory.\n");
    }
}

static void free_interactively(void)
{
    size_t id;

    if (!read_unsigned("Enter allocation ID: ", &id) ||
        id > (size_t) ULONG_MAX) {
        printf("Invalid allocation ID.\n");
        return;
    }
    if (memory_free((unsigned long) id)) {
        printf("Allocation %zu released.\n", id);
        record_current_memory();
    } else {
        printf("No active allocation has that ID.\n");
    }
}

static void run_demo(void)
{
    unsigned long first;
    unsigned long second;
    unsigned long leaked;

    printf("\n===== AUTOMATIC DEMONSTRATION =====\n");
    if (memory_allocate(120, ALLOCATION_MALLOC, &first)) {
        printf("1. malloc allocation created (ID %lu, 120 bytes).\n", first);
        record_current_memory();
    }
    if (memory_allocate(80, ALLOCATION_CALLOC, &second)) {
        printf("2. calloc allocation created (ID %lu, 80 bytes).\n", second);
        record_current_memory();
    }
    if (memory_reallocate(first, 200)) {
        printf("3. ID %lu reallocated from 120 to 200 bytes.\n", first);
        record_current_memory();
    }
    if (memory_free(second)) {
        printf("4. ID %lu freed successfully.\n", second);
        record_current_memory();
    }
    if (memory_allocate(900, ALLOCATION_MALLOC, &leaked)) {
        printf("5. Intentional unreleased allocation created (ID %lu).\n", leaked);
        record_current_memory();
    }
    predictor_print(3);
    memory_print_dashboard();
    memory_print_leaks();
    printf("===== END DEMONSTRATION =====\n");
}

static void print_menu(void)
{
    printf("\n===== MemPredictC =====\n");
    printf("1. Allocate Memory\n");
    printf("2. Allocate Using Calloc\n");
    printf("3. Reallocate Memory\n");
    printf("4. Free Memory\n");
    printf("5. View Memory Dashboard\n");
    printf("6. Predict Future Memory Usage\n");
    printf("7. Run Automatic Demonstration\n");
    printf("8. Check Memory Leaks\n");
    printf("9. Exit\n");
}

int main(void)
{
    size_t choice;
    size_t steps;

    memory_init();
    predictor_init(DEFAULT_THRESHOLD);
    predictor_record(0);
    printf("MemPredictC: Predict Memory. Detect Leaks. Optimize C Applications.\n");

    for (;;) {
        print_menu();
        if (!read_unsigned("Choose an option: ", &choice)) {
            printf("Please enter a menu number from 1 to 9.\n");
            continue;
        }
        switch (choice) {
        case 1:
            allocate_interactively(ALLOCATION_MALLOC);
            break;
        case 2:
            allocate_interactively(ALLOCATION_CALLOC);
            break;
        case 3:
            reallocate_interactively();
            break;
        case 4:
            free_interactively();
            break;
        case 5:
            memory_print_dashboard();
            break;
        case 6:
            if (read_unsigned("Predict how many operations ahead: ", &steps)) {
                predictor_print(steps);
            } else {
                printf("Invalid prediction horizon.\n");
            }
            break;
        case 7:
            run_demo();
            break;
        case 8:
            memory_print_leaks();
            break;
        case 9:
            memory_release_all();
            printf("All tracked allocations released. Goodbye.\n");
            return 0;
        default:
            printf("Invalid menu choice.\n");
            break;
        }
    }
}
