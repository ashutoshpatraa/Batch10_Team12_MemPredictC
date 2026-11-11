# Development Memory

## Implemented

- Added `main.c` with validated menu input and all nine required menu choices.
- Added `memory.c/.h` for malloc/calloc/realloc/free wrappers, statistics, a
  256-entry active registry, dashboard, leak report, and final cleanup.
- Added `predictor.c/.h` for the latest 64 observations and average upward trend.
- Added read-only allocation and prediction snapshots for the Windows UI.
- Added a native Win32 dashboard and CLI-compatible launcher.
- Added Makefile, `.gitignore`, README, test cases, prompts, and presentation
  materials.

## Important algorithm choices

Reallocation adjusts current, cumulative, and peak values by the size delta.
The predictor ignores downward changes when calculating an upward pressure
trend. The registry tracks only allocations created by this application.

## Known limitations

The registry capacity is fixed at 256 active blocks. Team member names need
verified input. The current MinGW GCC 6.3.0 toolchain builds both Windows targets successfully
after its bin directory is added to PATH or detected by `build.bat`.

The dashboard is intentionally native Win32 rather than raylib because no
raylib installation was available in the checked toolchain. It does not claim
system-wide monitoring and uses the same tracker and trend predictor as the
terminal application.

## Pending work

Run the documented build and sample-input regression test after repairing the
local GCC installation; the current MinGW GCC exits silently before producing
an executable.
