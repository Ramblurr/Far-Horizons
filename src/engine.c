// Far Horizons Game Engine
// Copyright (C) 2022 Michael D Henderson
// Copyright (C) 2021 Raven Zachary
// Copyright (C) 2019 Casey Link, Adam Piggott
// Copyright (C) 1999 Richard A. Morneau
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "engine.h"

// allow callers to set the seed. ugh.
unsigned long last_random = 1924085713L;    /* Random seed. */

/* This routine is intended to take a long argument and return a pointer to a string that has embedded commas to make the string more readable. */
char *commas(long value) {
    static char result_plus_commas[33];
    int i, j, n, length, negative;
    char temp[32];
    long abs_value;

    if (value < 0) {
        abs_value = -value;
        negative = TRUE;
    } else {
        abs_value = value;
        negative = FALSE;
    }
    sprintf(temp, "%ld", abs_value);
    length = strlen(temp);
    i = length - 1;
    j = 31;
    result_plus_commas[32] = '\0';
    for (n = 0; n < length; n++) {
        result_plus_commas[j--] = temp[i--];
        if (j % 4 == 0) {
            result_plus_commas[j--] = ',';
        }
    }

    j++;
    if (result_plus_commas[j] == ',') {
        j++;
    }
    if (negative) {
        result_plus_commas[--j] = '-';
    }

    return &result_plus_commas[j];
}


// rnd returns a random int between 1 and max, inclusive.
// It uses the so-called "Algorithm M" method, which is a combination
// of the congruential and shift-register methods.
int rnd(unsigned int max) {
    static unsigned long _lastRandom; // random seed
    // seedState 0 == uninitialized
    // seedState 1 == initializing
    // seedState 2 == initialized
    static int seedState = 0;
    unsigned long a;
    unsigned long b;
    unsigned long c;
    unsigned long cong_result;
    unsigned long shift_result;

    if (seedState == 0) {
        char *envSeed = getenv("FH_SEED");
        seedState = 1;
        if (envSeed != NULL) {
            for (; *envSeed != 0; envSeed++) {
                if (isdigit(*envSeed)) {
                    _lastRandom = _lastRandom * 10 + *envSeed - '0';
                }
            }
            if (_lastRandom == 0) {
                _lastRandom = 1924085713L;
            }
            seedState = 2;
        }
    }
    if (seedState == 2) {
        last_random = _lastRandom;
    }

    /* For congruential method, multiply previous value by the prime number 16417. */
    a = last_random;
    b = last_random << 5;
    c = last_random << 14;
    cong_result = a + b + c;    /* Effectively multiply by 16417. */

    /* For shift-register method, use shift-right 15 and shift-left 17 with no-carry addition (i.e., exclusive-or). */
    a = last_random >> 15;
    shift_result = a ^ last_random;
    a = shift_result << 17;
    shift_result ^= a;

    last_random = cong_result ^ shift_result;

    a = last_random & 0x0000FFFF;

    return (int) ((a * (long) max) >> 16) + 1L;
}
