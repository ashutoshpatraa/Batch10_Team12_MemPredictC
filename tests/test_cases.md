# Manual Test Cases

Use the menu numbers shown in `main.c`. IDs are assigned in order during a
fresh process, so the first successful allocation is ID 1.

| ID | Input sequence | Expected result |
|---|---|---|
| T01 | `1`, `64`, `5` | Dashboard shows 64 current bytes |
| T02 | `2`, `32`, `8` | Leak report shows one 32-byte active block |
| T03 | `4`, `1`, `5` | ID 1 is released and counters decrease |
| T04 | `1`, `20`, `3`, `1`, `100`, `5` | Reallocation increases current usage to 100 |
| T05 | `3`, `99`, `10` | Invalid ID message |
| T06 | `1`, `0` | Invalid size message |
| T07 | `6`, `3` immediately after start | Insufficient observation message |
| T08 | `7` | Complete demonstration and intentional leak report |
| T09 | `x`, `9` | Invalid menu input, then clean exit |
| T10 | `1`, `16`, `4`, `1`, `8`, `9` | Allocation can be freed and program exits cleanly |
