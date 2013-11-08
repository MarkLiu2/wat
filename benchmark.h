#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

int64_t wat_gettime();

int statistics(uint32_t *samples, int size, char *name);

#endif
