# Testing

## Strategy

Compile with C11 and strict warnings, run the automatic demonstration, then
exercise scripted menu inputs. Confirm dashboard counters, invalid input
messages, prediction sufficiency, and leak reporting.

## Test results

The cases below were exercised through the CLI regression input and build
commands. The GUI startup check verifies window initialization; interactive
control behavior shares the same memory and predictor modules as the CLI.

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
| Compiler diagnosis | `C:\MinGW\bin\gcc.exe -v` | PASS; GCC 6.3.0 present |
| Compiler isolation | Compile and run temporary `hello.c` | PASS; `MemPredictC compiler test successful!` |
| CLI build | `build.bat` strict C11 CLI target | PASS; `build\MemPredictC_CLI.exe` generated |
| GUI build | `build.bat` with `-mwindows -luser32 -lgdi32` | PASS; `build\MemPredictC.exe` generated |
| Makefile build | `C:\MinGW\bin\mingw32-make.exe -B` | PASS; both targets rebuilt |
| CLI regression | `build\MemPredictC_CLI.exe < tests\sample_inputs.txt` | PASS; exit code 0; demo, prediction, leak report, invalid ID, and cleanup observed |
| GUI startup | Launch `build\MemPredictC.exe` | PASS; window handle present and title `MemPredictC | Memory Monitoring & Prediction` |
| GUI subsystem | `objdump -p build\MemPredictC.exe` | PASS; `Subsystem 00000002 (Windows GUI)` |
| Screenshot | Existing dashboard capture | PASS; [MemPredictC-dashboard.png](../screenshots/MemPredictC-dashboard.png) |

Both executables were copied to `release\`. The current toolchain is
32-bit MinGW GCC 6.3.0; the dashboard uses only system Win32 libraries.

## Compiler warnings and known issues

The initial failure was caused by `gcc` not being on the shell `PATH`, even
though `C:\MinGW\bin\gcc.exe` was installed. `build.bat` now checks that
absolute path first and adds its directory to the child build environment. The
GUI target uses the native Windows subsystem and links `USER32` and `GDI32`.

## UI redesign validation (2026-11-17)

The Win32 presentation layer was redesigned without changing `memory.c`,
`memory.h`, `predictor.c`, or `predictor.h`.

| UI check | Result |
|---|---|
| Empty dashboard state | PASS; displays “No memory activity yet” and guidance |
| Dashboard statistics and graph | PASS; values updated from the engine after demo |
| Allocations page | PASS; malloc allocation appeared in the active table |
| Prediction page | PASS; trend graph, average growth, projection, and explanation visible |
| Leak Detection page | PASS; active IDs and outstanding bytes visible |
| Activity Log page | PASS; chronological engine events visible |
| Settings page | PASS; C11 engine status and tracking boundary visible |
| Navigation buttons | PASS; all six pages opened |
| Automatic demonstration | PASS; dashboard updated to 1164 current bytes and 3 active allocations |
| Resize check | PASS; tested at 1270px width and maximized 2582x1550 client size without clipping |
| GUI startup | PASS; native window initialized with expected title |
| CLI fallback | PASS; scripted regression exit code 0 after UI rebuild |

Screenshots were captured during the live UI validation session. The checked-in
image at [screenshots/MemPredictC-dashboard.png](../screenshots/MemPredictC-dashboard.png)
is retained as the repository screenshot reference; the live validation also
confirmed the redesigned dashboard and page states at runtime.
