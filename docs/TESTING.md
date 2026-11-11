# Testing

## Strategy

Compile with C11 and strict warnings, run the automatic demonstration, then
exercise scripted menu inputs. Confirm dashboard counters, invalid input
messages, prediction sufficiency, and leak reporting.

## Test results

The source was reviewed for the cases below. A genuine compiler/runtime run
must be performed on the target machine before final submission; this document
does not invent output. Record the command, compiler version, observed output,
and PASS/FAIL after that run.

| ID | Scenario | Expected |
|---|---|---|
| T01 | malloc 64 then dashboard | 64 current bytes and one active allocation |
| T02 | calloc 32 | Successful allocation with a new ID |
| T03 | free valid ID | Current bytes and active count decrease |
| T04 | two allocations | Both IDs appear in the leak report |
| T05 | realloc 20 to 100 | Current bytes increase by 80 |
| T06 | realloc 100 to 10 | Current bytes decrease by 90 |
| T07 | size zero | Validation error; no allocation |
| T08 | nonexistent ID | Invalid-ID message; counters unchanged |
| T09 | demo | Allocation, deallocation, realloc, prediction, and leak report |
| T10 | prediction before data | Insufficient-observations message |
| T11 | repeated allocation/free | No active allocation after final free |
| T12 | EOF/invalid menu text | Input error handled without crash |

## Regression command

```text
make
mempredictc.exe < tests/sample_inputs.txt
```

For Windows GUI packaging:

```text
build.bat
run.bat
```

The GUI checklist includes launching the dashboard, resizing the window,
malloc/calloc/realloc/free controls, prediction updates, graph observations,
bounded activity log behavior, leak status, and exit cleanup. The same
`MemoryStats` and `PredictionStats` values drive both the dashboard and the
core modules.

## Verified results

| Check | Command/action | Result |
|---|---|---|
| Compiler diagnosis | `C:\MinGW\bin\gcc.exe -v` | GCC 6.3.0 present; shell PATH did not include `C:\MinGW\bin` |
| Compiler isolation | Compile and run `hello.c` | PASS; executable linked and ran successfully |
| CLI build | Strict C11 build to `build\MemPredictC_CLI.exe` | PASS |
| GUI build | Strict C11 build with `-luser32 -lgdi32` | PASS |
| Makefile build | `C:\MinGW\bin\mingw32-make.exe -B` | PASS |
| CLI regression | `build\MemPredictC_CLI.exe < tests\sample_inputs.txt` | PASS; demo, prediction, leak report, and cleanup observed |
| GUI startup | Launch `build\MemPredictC.exe` | PASS; window remained running during startup check |
| Screenshot | Capture running dashboard | PASS; [MemPredictC-dashboard.png](../screenshots/MemPredictC-dashboard.png) |

Both executables were copied to `release\`. The current toolchain is
32-bit MinGW GCC 6.3.0; the dashboard uses only system Win32 libraries.

## Compiler warnings and known issues

The initial failures occurred because `gcc` was not on the shell `PATH`, and
the first GUI build also exposed source compatibility issues (`_strtoui64`,
missing `ULONG_MAX`, and `%zu` diagnostics in the legacy MinGW runtime). The
build script now detects `C:\MinGW\bin` automatically, and the GUI uses
standard `strtoull`, `<limits.h>`, and portable explicit formatting.
