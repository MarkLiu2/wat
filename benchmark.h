#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

int64_t wat_gettime();

void statistics(uint32_t *samples, int size);

#endif
