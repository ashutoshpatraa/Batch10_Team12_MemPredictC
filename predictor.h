#ifndef MEMPREDICTC_PREDICTOR_H
#define MEMPREDICTC_PREDICTOR_H

#include <stddef.h>

#define MAX_OBSERVATIONS 64

void predictor_init(size_t warning_threshold);
void predictor_record(size_t current_memory);
void predictor_print(size_t steps);
size_t predictor_get_threshold(void);

#endif
