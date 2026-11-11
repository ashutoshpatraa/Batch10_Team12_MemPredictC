# Architecture

## Modules

- `main.c`: menu, safe numeric input, demo orchestration, and user messages.
- `memory.h/.c`: `AllocationRecord`, `MemoryStats`, allocation wrappers,
  registry lookup, dashboard, leak report, and cleanup.
- `predictor.h/.c`: bounded observation history and trend projection.

## Registry design

The memory module owns a static array of 256 records. Each active record has a
unique ID, address, size, operation kind, and active flag. The public API never
exposes writable registry state.

## Statistics

Current bytes and active count describe live records. Total allocated and total
freed are cumulative counters; a realloc size increase counts as allocated
delta, while a decrease counts as freed delta. Peak is the largest current
value observed.

## Communication

`main.c` calls the memory API, reads `MemoryStats`, and sends current bytes to
the predictor. The predictor has no pointer ownership and never allocates
application memory.
