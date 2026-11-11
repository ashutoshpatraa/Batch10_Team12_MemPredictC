# Handover

## Project overview

MemPredictC is a small C11 educational allocation tracker and trend predictor
for Batch 10, Team 12.

## Current development status

The core implementation, native Win32 dashboard, and required repository
structure are present. Build and interactive regression testing should be run
with the available C compiler.

## Working

Malloc, calloc, realloc, free, statistics, dashboard, leak report, prediction,
automatic demonstration, invalid-input handling, cleanup, Makefile, native
Win32 dashboard, CLI fallback, build scripts, and docs are implemented.

## Incomplete and known issues

Team member names are not present because they were not supplied. The installed `C:\MinGW\bin\gcc.exe` is MinGW GCC 6.3.0. It works when invoked
by absolute path or after adding `C:\MinGW\bin` to `PATH`; the original silent
failure was caused by the missing PATH entry.

## Important files

Read `README.md`, `main.c`, `memory.c`, `predictor.c`, and `docs/TESTING.md`.

## Compile and test

Run `build.bat`. It creates `build\MemPredictC.exe` and
`build\MemPredictC_CLI.exe`, copies both into `release\`, and `run.bat`
launches the dashboard.
For scripted coverage, redirect `tests/sample_inputs.txt` to the executable.

## NEXT AGENT: START HERE

- Read `docs/context.md`, `docs/memory.md`, `docs/plan.md`, and this file.
- Current state: GUI and CLI builds are verified.
- Exact next task: perform any desired manual interaction testing on the
  packaged release binaries.
- Known problems: verify `SIZE_MAX` support with the selected C11 toolchain.
- Avoid claiming system-wide monitoring or inventing team member details.
