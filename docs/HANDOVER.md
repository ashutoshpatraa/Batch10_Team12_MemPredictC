# Handover

## Project overview

MemPredictC is a small C11 educational allocation tracker and trend predictor
for Batch 10, Team 12.

## Current development status

The core implementation, redesigned native Win32 dashboard, required repository
structure, and release executables are present. Build and regression testing
are verified with MinGW GCC 6.3.0.

## Working

Malloc, calloc, realloc, free, statistics, dashboard, leak report, prediction,
automatic demonstration, invalid-input handling, cleanup, Makefile, responsive
Win32 navigation shell, page-specific views, CLI fallback, build scripts, and
docs are implemented. The engine modules were not rewritten.

## Incomplete and known issues

Team member names are not present because they were not supplied. The installed
`C:\MinGW\bin\gcc.exe` is MinGW GCC 6.3.0. The original silent failure was
caused by the missing PATH entry; `build.bat` now detects the absolute path.
The checked-in dashboard screenshot predates the latest UI redesign; live
screenshots were captured during validation but were not written through the
repository file tools.

## Important files

Read `README.md`, `main.c`, `memory.c`, `predictor.c`, and `docs/TESTING.md`.

## Compile and test

Run `build.bat`. It creates `build\MemPredictC.exe` and
`build\MemPredictC_CLI.exe`, copies both into `release\`, and `run.bat`
launches the dashboard.
For scripted coverage, redirect `tests/sample_inputs.txt` to the executable.

## NEXT AGENT: START HERE

- Read `docs/context.md`, `docs/memory.md`, `docs/plan.md`, and this file.
- Current state: redesigned GUI pages, responsive resizing, GUI startup, CLI
  regression, and packaged binaries are verified.
- Exact next task: optionally replace the checked-in screenshot with a saved
  capture of the redesigned dashboard and add verified team names.
- Known problems: verify `SIZE_MAX` support with the selected C11 toolchain.
- Avoid claiming system-wide monitoring or inventing team member details.
