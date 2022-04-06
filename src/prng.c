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
#include <ctype.h>
#include <stdint.h>
#include "prng.h"


static uint64_t prngSeed = 0;


// prng returns a random int between 1 and max, inclusive.
// It uses the so-called "Algorithm M" method, which is a combination of the congruential and shift-register methods.
int prng(unsigned int max) {
    if (prngSeed == 0) {
        char *envSeed = getenv("FH_SEED");
        if (envSeed) {
            for (; *envSeed != 0; envSeed++) {
                if (isdigit(*envSeed)) {
                    prngSeed = prngSeed * 10 + *envSeed - '0';
                }
            }
        }
        if (prngSeed == 0) {
            prngSeed = 1924085713L;
        }
    }

    /* For congruential method, multiply previous value by the prime number 16417. */
    uint64_t cong_result = prngSeed + (prngSeed << 5) + (prngSeed << 14);    /* Effectively multiply by 16417. */

    /* For shift-register method, use shift-right 15 and shift-left 17 with no-carry addition (i.e., exclusive-or). */
    uint64_t shift_result = (prngSeed >> 15) ^ prngSeed;
    shift_result ^= (shift_result << 17);

    prngSeed = cong_result ^ shift_result;

    return (int) (((prngSeed & 0x0000FFFF) * (uint64_t) max) >> 16) + 1L;
}


uint64_t prngGetSeed(void) {
    return prngSeed;
}


int prngSetSeed(uint64_t seed) {
    prngSeed = seed;
}


