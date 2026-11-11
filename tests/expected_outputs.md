# Expected Output Checks

The scripted input should include these observable strings:

- `AUTOMATIC DEMONSTRATION`
- `reallocated from 120 to 200 bytes`
- `MEMORY DASHBOARD`
- `LEAK REPORT`
- `No active allocation has that ID.`
- `Next 3 operation(s)`
- `All tracked allocations released. Goodbye.`

Exact byte totals may differ if the input sequence is changed. Copy actual
results into `docs/TESTING.md` after running the executable.
