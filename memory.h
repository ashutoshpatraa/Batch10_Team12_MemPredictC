#ifndef MEMPREDICTC_MEMORY_H
#define MEMPREDICTC_MEMORY_H

#include <stddef.h>

#define MAX_ALLOCATIONS 256

typedef enum {
    ALLOCATION_MALLOC,
    ALLOCATION_CALLOC
} AllocationKind;

typedef struct {
    unsigned long id;
    void *address;
    size_t size;
    int active;
    AllocationKind kind;
} AllocationRecord;

typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t current_bytes;
    size_t peak_bytes;
    unsigned long allocation_operations;
    unsigned long deallocation_operations;
    size_t active_allocations;
} MemoryStats;

void memory_init(void);
int memory_allocate(size_t size, AllocationKind kind, unsigned long *id_out);
int memory_reallocate(unsigned long id, size_t new_size);
int memory_free(unsigned long id);
const AllocationRecord *memory_find(unsigned long id);
MemoryStats memory_get_stats(void);
/* Copies up to capacity active records into caller-owned storage. */
size_t memory_get_active_allocations(AllocationRecord *records_out,
                                     size_t capacity);
void memory_print_dashboard(void);
void memory_print_leaks(void);
void memory_release_all(void);

#endif
