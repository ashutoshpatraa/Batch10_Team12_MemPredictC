# Development Memory

## Implemented

- Added `main.c` with validated menu input and all nine required menu choices.
- Added `memory.c/.h` for malloc/calloc/realloc/free wrappers, statistics, a
  256-entry active registry, dashboard, leak report, and final cleanup.
- Added `predictor.c/.h` for the latest 64 observations and average upward trend.
- Added read-only allocation and prediction snapshots for the Windows UI.
- Added a native Win32 dashboard and CLI-compatible launcher.
- Replaced the fixed single-screen dashboard with responsive Dashboard,
  Allocations, Prediction, Leak Detection, Activity Log, and Settings pages.
- Added empty graph state, owner-drawn dark navigation/actions, responsive
  positioning, minimum window size, and DPI-awareness fallback.
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
terminal application. The current UI validation covered the default window,
1270px width resizing, and maximized 2582x1550 client size.

## Validation completed

The original MinGW PATH issue was fixed in `build.bat`. MinGW GCC 6.3.0 now
builds both targets, the CLI sample-input regression exits successfully, and
the GUI starts with the expected native Win32 window. Generated executables are
copied to both `build/` and `release/`.

## Pending work

Optionally replace the older checked-in screenshot with a saved capture of the
redesigned dashboard, then add verified team member names.
