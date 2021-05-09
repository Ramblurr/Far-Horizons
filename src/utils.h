#ifndef FH_UTILS_H
#define FH_UTILS_H

int rnd(unsigned int max);

// most of the executables set last_random directly.
// that still works unless FH_SEED is exported.
unsigned long last_random;

#endif
