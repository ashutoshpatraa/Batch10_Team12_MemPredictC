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

## Compiler warnings and known issues

Not yet recorded in this fresh repository. Run with `-Wall -Wextra -Wpedantic`
and record any warning rather than suppressing it.
