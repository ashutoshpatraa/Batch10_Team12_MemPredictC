# Presentation Outline (8-10 slides)

1. **Title:** MemPredictC, Batch 10 Team 12, tagline, verified team members.
2. **Problem:** Unreleased dynamic memory can grow during long-running programs.
3. **Why it matters:** C gives control and responsibility for allocation lifetime.
4. **Solution:** A wrapper registry, dashboard, leak report, and trend estimate.
5. **Architecture:** `main.c`, `memory.c/.h`, and `predictor.c/.h`.
6. **Algorithm:** IDs and counters; average upward change for projection.
7. **Live demo:** Run option 7 and explain the intentional unreleased block.
8. **Testing:** Show strict compilation and the documented manual cases.
9. **Limits/future:** Only wrapped allocations; fixed capacity; CSV/export ideas.
10. **Conclusion:** Predict memory, detect tracked leaks, and write safer C.
