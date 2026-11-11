# Development Memory

## Implemented

- Added `main.c` with validated menu input and all nine required menu choices.
- Added `memory.c/.h` for malloc/calloc/realloc/free wrappers, statistics, a
  256-entry active registry, dashboard, leak report, and final cleanup.
- Added `predictor.c/.h` for the latest 64 observations and average upward trend.
- Added Makefile, `.gitignore`, README, test cases, prompts, and presentation
  materials.

## Important algorithm choices

Reallocation adjusts current, cumulative, and peak values by the size delta.
The predictor ignores downward changes when calculating an upward pressure
trend. The registry tracks only allocations created by this application.

## Known limitations

The registry capacity is fixed at 256 active blocks. Team member names need
verified input. Compiler-specific test output must be refreshed on the target
machine if the toolchain differs.

## Pending work

Run the documented build and sample-input regression test with an installed
C11 compiler, then replace any toolchain-specific placeholder in `docs/TESTING.md`.
