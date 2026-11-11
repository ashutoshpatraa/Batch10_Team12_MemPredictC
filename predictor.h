#ifndef MEMPREDICTC_PREDICTOR_H
#define MEMPREDICTC_PREDICTOR_H

#include <stddef.h>

#define MAX_OBSERVATIONS 64

typedef struct {
    size_t current_memory;
    double average_growth;
    double predicted_memory;
    size_t steps;
    size_t observation_count;
    int has_prediction;
    int warning;
} PredictionStats;

void predictor_init(size_t warning_threshold);
void predictor_record(size_t current_memory);
/* Calculates a snapshot without printing or changing predictor state. */
PredictionStats predictor_get_stats(size_t steps);
/* Copies the bounded observation history into caller-owned storage. */
size_t predictor_copy_observations(size_t *observations_out, size_t capacity);
void predictor_print(size_t steps);
size_t predictor_get_threshold(void);

#endif
