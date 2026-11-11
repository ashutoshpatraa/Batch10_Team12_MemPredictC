#include "predictor.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

PredictionStats predictor_get_stats(size_t steps)
{
    PredictionStats result;
    size_t i;
    size_t total_change = 0;

    result.current_memory = observation_count == 0
                                ? 0
                                : observations[observation_count - 1];
    result.average_growth = 0.0;
    result.predicted_memory = 0.0;
    result.steps = steps;
    result.observation_count = observation_count;
    result.has_prediction = observation_count >= 2 && steps > 0;
    result.warning = 0;

    if (!result.has_prediction) {
        return result;
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
    result.average_growth = (double) total_change /
                            (double) (observation_count - 1);
    result.predicted_memory = (double) result.current_memory +
                              result.average_growth * (double) steps;
    result.warning = result.predicted_memory >= (double) threshold;
    return result;
}

size_t predictor_copy_observations(size_t *observations_out, size_t capacity)
{
    size_t count;

    if (observations_out == NULL || capacity == 0) {
        return 0;
    }
    count = observation_count < capacity ? observation_count : capacity;
    memcpy(observations_out, observations, count * sizeof(*observations_out));
    return count;
}

void predictor_print(size_t steps)
{
    PredictionStats result = predictor_get_stats(steps);

    if (!result.has_prediction && result.observation_count < 2) {
        printf("\nPrediction needs at least 2 memory observations; currently %zu.\n",
               observation_count);
        return;
    }
    if (!result.has_prediction) {
        printf("\nPrediction steps must be greater than zero.\n");
        return;
    }

    printf("\n===== MEMORY PREDICTION =====\n");
    printf("Observations used : %zu\n", result.observation_count);
    printf("Average upward trend: %.2f bytes/operation\n", result.average_growth);
    printf("Next %zu operation(s): %.0f bytes projected\n",
           steps, result.predicted_memory);
    if (result.warning) {
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
