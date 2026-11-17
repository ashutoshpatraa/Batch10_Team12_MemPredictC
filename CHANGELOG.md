# Changelog

## 2026-11-11

- Created the C11 memory tracking module and predictor module.
- Added the interactive dashboard, automatic demonstration, and leak report.
- Added Makefile, ignore rules, README, project documentation, test cases, and
  presentation outline.
- Added a native Win32 dashboard with live statistics, graph, allocation
  controls, prediction panel, leak status, activity log, and CLI fallback.
- Added read-only memory/predictor snapshot APIs and Windows build/run scripts.
- Diagnosed the MinGW PATH issue, added legacy MinGW compatibility fixes, and
  verified both Windows executables plus a dashboard screenshot.

## 2026-11-17

- Fixed `build.bat` GCC detection when `C:\MinGW\bin` is not on PATH.
- Set the GUI linker target to the native Windows subsystem with `-mwindows`.
- Rebuilt and verified `build\MemPredictC.exe` and
  `build\MemPredictC_CLI.exe`.
- Ran the scripted CLI regression and verified GUI startup and Win32
  dependencies.
- Redesigned the Win32 UI with responsive dark navigation and dedicated
  Dashboard, Allocations, Prediction, Leak Detection, Activity Log, and
  Settings pages.
- Added scalable cards, empty graph state, owner-drawn controls, responsive
  resizing, and live engine-backed tables and activity views.
