#include "types.h"

static uint next_random = 1;

// Generates a pseudo-random number using a linear congruential generator.
uint random(void) {
    next_random = next_random * 1103515245 + 12345;
    return (uint)(next_random / 65536);
}

// Generates a pseudo-random number within a given range [0, max].
uint random_at_most(uint max) {
    if (max == 0) {
        return 0;
    }
    return random() % (max + 1);
}
