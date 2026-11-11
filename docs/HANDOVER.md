# Handover

## Project overview

MemPredictC is a small C11 educational allocation tracker and trend predictor
for Batch 10, Team 12.

## Current development status

The core implementation and required repository structure are present. Build
and interactive regression testing should be run with the available C compiler.

## Working

Malloc, calloc, realloc, free, statistics, dashboard, leak report, prediction,
automatic demonstration, invalid-input handling, cleanup, Makefile, and docs
are implemented.

## Incomplete and known issues

Team member names are not present because they were not supplied. The local
environment may not have GCC or Make; use a C11 compiler on the target machine.

## Important files

Read `README.md`, `main.c`, `memory.c`, `predictor.c`, and `docs/TESTING.md`.

## Compile and test

Run `make`, then `mempredictc` (or `mempredictc.exe`) and choose option 7.
For scripted coverage, redirect `tests/sample_inputs.txt` to the executable.

## NEXT AGENT: START HERE

- Read `docs/context.md`, `docs/memory.md`, `docs/plan.md`, and this file.
- Current state: implementation is complete; validation is the next task.
- Exact next task: compile with GCC/Clang, run the sample input, and record
  genuine output in `docs/TESTING.md`.
- Known problems: verify `SIZE_MAX` support with the selected C11 toolchain.
- Avoid claiming system-wide monitoring or inventing team member details.
