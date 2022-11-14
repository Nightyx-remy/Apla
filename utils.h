#ifndef APLA_COMPILER_UTILS_H
#define APLA_COMPILER_UTILS_H

#include <stdint.h>
#include <math.h>

#define MIN(x, y) x > y ? y : x
#define MAX(x, y) x < y ? y : x

uintptr_t next_pow2(uintptr_t x);

#endif //APLA_COMPILER_UTILS_H
