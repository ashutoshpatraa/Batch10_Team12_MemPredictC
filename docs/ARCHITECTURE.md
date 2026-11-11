# Architecture

## Modules

- `main.c`: CLI menu, safe numeric input, demo orchestration, and user messages.
- `cli.h`: public entry point used by both the CLI and GUI fallback.
- `memory.h/.c`: `AllocationRecord`, `MemoryStats`, allocation wrappers,
  registry lookup, dashboard, leak report, and cleanup.
- `predictor.h/.c`: bounded observation history and trend projection.
- `ui.h/.c`: native Win32 dashboard, controls, graph, activity log, and
  read-only presentation of engine snapshots.
- `gui_main.c`: GUI launcher with terminal fallback.

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

The UI uses `memory_get_active_allocations`, `predictor_get_stats`, and
`predictor_copy_observations`. These APIs copy bounded data into caller-owned
buffers and do not expose writable tracker internals.
