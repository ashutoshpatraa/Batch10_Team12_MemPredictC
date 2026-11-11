# Decision Log

## Fixed registry (2026-11-11)

**Decision:** Use a 256-entry static registry.  
**Reason:** It is deterministic and easy for first-year students to explain.  
**Alternatives:** Linked list or dynamically growing registry.  
**Consequence:** The program reports allocation failure when all slots are used.

## Trend prediction (2026-11-11)

**Decision:** Use the average of non-negative successive memory changes.  
**Reason:** The calculation is transparent and honest about not being AI/ML.  
**Alternatives:** Linear regression or an external model.  
**Consequence:** It is simple but cannot model sudden workload changes.

## Input handling (2026-11-11)

**Decision:** Read lines with `fgets` and parse with `strtoull`.  
**Reason:** This avoids leftover input and unchecked `%d` conversion.  
**Alternatives:** `scanf`.  
**Consequence:** Invalid lines are rejected consistently.
