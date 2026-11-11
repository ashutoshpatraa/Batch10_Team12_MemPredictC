# Project Context

## Identity

MemPredictC is the Batch 10, Team 12 First-Year BTech Mini Hackathon project:
**AI-Guided Dynamic Memory Allocation and Memory Leak Prediction for Long-Running
C Applications**. It is written in standard C11.

## Objective and scope

The program demonstrates dynamic allocation, records allocations made through
its own wrapper functions, detects unreleased tracked allocations, and predicts
near-future usage from a recent trend. It is not a system-wide monitor and does
not claim to find every memory error.

## Current status

The core application, interactive menu, dashboard, automatic demonstration,
leak report, prediction module, Makefile, README, tests, and handover
documentation are implemented. Team names remain to be verified.

## Restrictions and decisions

The code stays understandable to first-year students, uses only the C
standard library, avoids external ML frameworks, validates numeric input, and
uses a fixed 256-entry registry. Prediction is explicitly a mathematical trend
estimate.

## Build

`make` builds `mempredictc`; on Windows use `mempredictc.exe`. Direct GCC
compilation is documented in the root README.
