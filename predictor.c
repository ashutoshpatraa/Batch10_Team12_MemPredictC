#include "predictor.h"

#include <stdio.h>
#include <stdint.h>

static size_t observations[MAX_OBSERVATIONS];
static size_t observation_count;
static size_t threshold;

void predictor_init(size_t warning_threshold)
{
    observation_count = 0;
    threshold = warning_threshold;
}

void predictor_record(size_t current_memory)
{
    if (observation_count < MAX_OBSERVATIONS) {
        observations[observation_count++] = current_memory;
    } else {
        size_t i;
        for (i = 1; i < MAX_OBSERVATIONS; ++i) {
            observations[i - 1] = observations[i];
        }
        observations[MAX_OBSERVATIONS - 1] = current_memory;
    }
}

void predictor_print(size_t steps)
{
    size_t i;
    size_t total_change = 0;
    double average_change;
    double prediction;

    if (observation_count < 2) {
        printf("\nPrediction needs at least 2 memory observations; currently %zu.\n",
               observation_count);
        return;
    }
    if (steps == 0) {
        printf("\nPrediction steps must be greater than zero.\n");
        return;
    }

    for (i = 1; i < observation_count; ++i) {
        if (observations[i] >= observations[i - 1]) {
            size_t change = observations[i] - observations[i - 1];
            if (change > SIZE_MAX - total_change) {
                total_change = SIZE_MAX;
            } else {
                total_change += change;
            }
        }
    }
    average_change = (double) total_change / (double) (observation_count - 1);
    prediction = (double) observations[observation_count - 1] +
                 average_change * (double) steps;

    printf("\n===== MEMORY PREDICTION =====\n");
    printf("Observations used : %zu\n", observation_count);
    printf("Average upward trend: %.2f bytes/operation\n", average_change);
    printf("Next %zu operation(s): %.0f bytes projected\n", steps, prediction);
    if (prediction >= (double) threshold) {
        printf("WARNING: projected usage reaches the %zu-byte threshold.\n", threshold);
    } else {
        printf("Projected usage is below the %zu-byte threshold.\n", threshold);
    }
    printf("This is a simple trend estimate, not machine learning.\n");
    printf("=============================\n");
}

size_t predictor_get_threshold(void)
{
    return threshold;
}
