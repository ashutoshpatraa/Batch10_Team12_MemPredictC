#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static AllocationRecord records[MAX_ALLOCATIONS];
static MemoryStats stats;
static unsigned long next_id;

void memory_init(void)
{
    memset(records, 0, sizeof(records));
    memset(&stats, 0, sizeof(stats));
    next_id = 1;
}

static AllocationRecord *find_record(unsigned long id)
{
    size_t i;

    for (i = 0; i < MAX_ALLOCATIONS; ++i) {
        if (records[i].active && records[i].id == id) {
            return &records[i];
        }
    }
    return NULL;
}

static AllocationRecord *free_record_slot(void)
{
    size_t i;

    for (i = 0; i < MAX_ALLOCATIONS; ++i) {
        if (!records[i].active) {
            return &records[i];
        }
    }
    return NULL;
}

int memory_allocate(size_t size, AllocationKind kind, unsigned long *id_out)
{
    AllocationRecord *record;
    void *address;

    if (size == 0 || id_out == NULL ||
        size > SIZE_MAX - stats.current_bytes ||
        size > SIZE_MAX - stats.total_allocated) {
        return 0;
    }

    record = free_record_slot();
    if (record == NULL) {
        return 0;
    }

    if (kind == ALLOCATION_CALLOC) {
        address = calloc(1, size);
    } else {
        address = malloc(size);
    }
    if (address == NULL) {
        return 0;
    }

    record->id = next_id++;
    record->address = address;
    record->size = size;
    record->active = 1;
    record->kind = kind;

    stats.total_allocated += size;
    stats.current_bytes += size;
    if (stats.current_bytes > stats.peak_bytes) {
        stats.peak_bytes = stats.current_bytes;
    }
    stats.allocation_operations++;
    stats.active_allocations++;
    *id_out = record->id;
    return 1;
}

int memory_reallocate(unsigned long id, size_t new_size)
{
    AllocationRecord *record;
    void *new_address;
    size_t old_size;

    if (new_size == 0) {
        return 0;
    }
    record = find_record(id);
    if (record == NULL) {
        return 0;
    }

    if (new_size >= record->size) {
        size_t increase = new_size - record->size;
        if (increase > SIZE_MAX - stats.current_bytes ||
            increase > SIZE_MAX - stats.total_allocated) {
            return 0;
        }
    } else {
        size_t decrease = record->size - new_size;
        if (decrease > SIZE_MAX - stats.total_freed) {
            return 0;
        }
    }

    new_address = realloc(record->address, new_size);
    if (new_address == NULL) {
        return 0;
    }

    old_size = record->size;
    record->address = new_address;
    record->size = new_size;
    if (new_size >= old_size) {
        stats.total_allocated += new_size - old_size;
        stats.current_bytes += new_size - old_size;
    } else {
        stats.total_freed += old_size - new_size;
        stats.current_bytes -= old_size - new_size;
    }
    if (stats.current_bytes > stats.peak_bytes) {
        stats.peak_bytes = stats.current_bytes;
    }
    stats.allocation_operations++;
    return 1;
}

int memory_free(unsigned long id)
{
    AllocationRecord *record = find_record(id);

    if (record == NULL) {
        return 0;
    }

    free(record->address);
    stats.total_freed += record->size;
    stats.current_bytes -= record->size;
    stats.deallocation_operations++;
    stats.active_allocations--;
    memset(record, 0, sizeof(*record));
    return 1;
}

const AllocationRecord *memory_find(unsigned long id)
{
    return find_record(id);
}

MemoryStats memory_get_stats(void)
{
    return stats;
}

void memory_print_dashboard(void)
{
    printf("\n===== MEMORY DASHBOARD =====\n");
    printf("Current memory       : %zu bytes\n", stats.current_bytes);
    printf("Peak memory          : %zu bytes\n", stats.peak_bytes);
    printf("Total allocated      : %zu bytes\n", stats.total_allocated);
    printf("Total freed          : %zu bytes\n", stats.total_freed);
    printf("Allocation operations: %lu\n", stats.allocation_operations);
    printf("Deallocation ops     : %lu\n", stats.deallocation_operations);
    printf("Active allocations   : %zu\n", stats.active_allocations);
    printf("============================\n");
}

void memory_print_leaks(void)
{
    size_t i;
    size_t outstanding = 0;
    size_t bytes = 0;

    printf("\n===== LEAK REPORT =====\n");
    for (i = 0; i < MAX_ALLOCATIONS; ++i) {
        if (records[i].active) {
            printf("Allocation ID %lu: %zu bytes unreleased\n",
                   records[i].id, records[i].size);
            outstanding++;
            bytes += records[i].size;
        }
    }
    if (outstanding == 0) {
        printf("No outstanding tracked allocations.\n");
    }
    printf("Outstanding allocations: %zu\n", outstanding);
    printf("Outstanding memory     : %zu bytes\n", bytes);
    printf("This report covers only allocations made through MemPredictC.\n");
    printf("=======================\n");
}

void memory_release_all(void)
{
    size_t i;

    for (i = 0; i < MAX_ALLOCATIONS; ++i) {
        if (records[i].active) {
            free(records[i].address);
            if (records[i].size <= SIZE_MAX - stats.total_freed) {
                stats.total_freed += records[i].size;
            } else {
                stats.total_freed = SIZE_MAX;
            }
            if (stats.deallocation_operations < ULONG_MAX) {
                stats.deallocation_operations++;
            }
            memset(&records[i], 0, sizeof(records[i]));
        }
    }
    stats.current_bytes = 0;
    stats.active_allocations = 0;
}
