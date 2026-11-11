# MemPredictC Project Report

## 1. Project Title

**MemPredictC: AI-Guided Dynamic Memory Allocation and Memory Leak Prediction
for Long-Running C Applications**

**Tagline:** Predict Memory. Detect Leaks. Optimize C Applications.

## 2. Project Information

| Item | Details |
|---|---|
| Project type | First-Year BTech Mini Hackathon |
| Batch | 10 |
| Team | 12 |
| Language | Standard C11 |
| Repository | `Batch10_Team12_MemPredictC` |
| Team members | To be added after verification |

## 3. Abstract

MemPredictC is an educational C program that demonstrates dynamic memory
management and helps users understand memory usage over time. The program
provides wrappers for `malloc`, `calloc`, `realloc`, and `free`. It assigns an
ID to each tracked allocation, maintains memory statistics, reports active
unreleased allocations, and estimates future memory usage using a simple
trend-based calculation.

The project is intentionally transparent and beginner-friendly. It is not a
machine-learning system and does not monitor every process or all memory on the
computer. It tracks only allocations created through its own memory-management
functions.

## 4. Introduction

Dynamic memory is useful when a C program needs storage whose size or lifetime
is not known at compile time. However, allocated memory must eventually be
released. Forgetting to release memory can cause a memory leak, especially in
long-running applications.

Beginners often use allocation functions without seeing their cumulative
effect. MemPredictC makes this behavior visible through a dashboard, allocation
IDs, a leak report, and a future-usage estimate.

## 5. Problem Statement

Manual tracking of dynamic memory is difficult in a growing C application.
Unreleased blocks may remain unnoticed, and increasing memory usage may only
become visible after performance has already degraded. A simple educational
tool is needed to demonstrate allocation, release, tracking, leak reporting,
and basic prediction in one program.

## 6. Objectives

1. Demonstrate `malloc`, `calloc`, `realloc`, and `free`.
2. Track total allocated, total freed, current, and peak memory.
3. Count allocation, deallocation, and active allocation operations.
4. Detect unreleased allocations created through MemPredictC.
5. Estimate future usage from recent memory observations.
6. Warn when projected usage reaches a configurable threshold.
7. Provide a clear interactive menu and an automatic demonstration.
8. Keep the implementation understandable to first-year C students.

## 7. Proposed Solution

The program uses three modules:

- `main.c` handles the menu, input validation, and demonstration workflow.
- `memory.c` maintains the allocation registry and memory statistics.
- `predictor.c` stores observations and calculates a trend-based projection.

Every successful allocation receives a unique ID. The registry stores its
address, size, operation type, and active state. A successful free removes the
record from the active set. At any time, the dashboard can display current
usage and cumulative statistics. The leak report lists every active record.

## 8. System Features

### 8.1 Dynamic allocation

The application supports:

- Allocation with `malloc`
- Zero-initialized allocation with `calloc`
- Resizing with `realloc`
- Releasing memory with `free`

### 8.2 Memory tracking

The program reports:

- Total allocated memory
- Total freed memory
- Current memory
- Peak memory
- Allocation operations
- Deallocation operations
- Active allocations

### 8.3 Leak detection

The registry identifies active blocks that have not been released. The report
shows each allocation ID, its size, the number of outstanding allocations, and
the total outstanding bytes.

### 8.4 Prediction

The predictor stores recent current-memory observations. It calculates the
average non-negative change between observations and projects memory usage a
requested number of operations ahead. A warning is printed when the projection
reaches the 1024-byte threshold.

### 8.5 Automatic demonstration

Option 7 demonstrates successful allocation, calloc, reallocation,
deallocation, multiple active allocations, an intentional unreleased block,
prediction, dashboard output, and leak reporting.

## 9. Algorithm

For observations `u[0]` through `u[n-1]`:

```text
total_change = sum(max(0, u[i] - u[i-1]))
average_change = total_change / (n - 1)
projection = u[n-1] + average_change * requested_steps
```

If fewer than two observations exist, the program reports that prediction data
is insufficient. This method is a basic mathematical estimate, not artificial
intelligence or machine learning.

## 10. Program Flow

1. Initialize the memory registry and predictor.
2. Record an initial zero-memory observation.
3. Display the interactive menu.
4. Read and validate the user's choice.
5. Perform the selected memory operation.
6. Record current usage after successful changes.
7. Display dashboard, prediction, demonstration, or leak information.
8. On exit, release all remaining tracked allocations.

## 11. Data Structures

`AllocationRecord` contains:

- Allocation ID
- Pointer address
- Allocation size
- Active flag
- Allocation kind

`MemoryStats` contains cumulative and current memory counters. The registry
supports up to 256 active records, which keeps the design predictable and easy
to explain.

## 12. Technologies and C Concepts

- Standard C11
- C standard library
- Structures and enumerations
- Header and source-file modules
- Pointers and `void *`
- Arrays and loops
- `size_t` and overflow checks
- `fgets` and `strtoull` input parsing
- Dynamic memory functions

## 13. Testing

The planned test coverage includes normal malloc and calloc operations,
valid and invalid frees, multiple allocations, resizing up and down, invalid
sizes, invalid IDs, prediction with insufficient observations, prediction with
enough observations, repeated allocation/free cycles, and intentional leak
detection.

The project includes:

- [tests/test_cases.md](../tests/test_cases.md)
- [tests/sample_inputs.txt](../tests/sample_inputs.txt)
- [tests/expected_outputs.md](../tests/expected_outputs.md)
- [docs/TESTING.md](TESTING.md)

The local MinGW GCC executable was detected, but it exited with status 1
without diagnostics even during preprocessing. No executable or runtime result
was therefore claimed. A successful compiler run should be completed before
final submission.

## 14. Limitations

1. Only allocations made through MemPredictC are tracked.
2. The program does not monitor another process or system-wide memory.
3. The fixed registry supports 256 active allocations.
4. The trend estimate cannot model sudden workload changes.
5. It does not detect every dangling pointer, buffer overflow, or invalid
   memory access.
6. Team member names and IDs must be added only after verification.

## 15. Future Enhancements

- Allow the registry size to be configured.
- Export observations and reports to CSV.
- Add timestamps and operation history.
- Add richer graphical dashboard features beyond the current native Windows
  dashboard.
- Add configurable warning thresholds.
- Integrate with a platform-specific profiling interface.
- Add automated unit tests using a standard C test framework.

## 16. Conclusion

MemPredictC provides a practical and understandable demonstration of dynamic
memory management in C. It connects allocation operations with live memory
statistics, tracked leak detection, and a simple future-usage prediction. The
project emphasizes correct resource handling while clearly stating the limits
of a program-level educational tracker.
