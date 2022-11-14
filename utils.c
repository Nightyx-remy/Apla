#include "utils.h"

uintptr_t next_pow2(uintptr_t x) {
    return (uintptr_t) pow(2, ceil(log((double) x) / log(2)));
}
