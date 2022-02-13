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

#ifndef FAR_HORIZONS_PLANET_H
#define FAR_HORIZONS_PLANET_H

#include <stdint.h>
#include <stdio.h>

/* In case gamemaster creates new star systems with Edit program. */
#define NUM_EXTRA_PLANETS    100

struct planet_data {
    uint8_t temperature_class;  /* Temperature class, 1-30. */
    uint8_t pressure_class;     /* Pressure class, 0-29. */
    uint8_t special;            /* 0 = not special, 1 = ideal home planet, 2 = ideal colony planet, 3 = radioactive hellhole. */
    uint8_t reserved1;          /* Reserved for future use. Zero for now. */
    uint8_t gas[4];             /* Gas in atmosphere. Zero if none. */
    uint8_t gas_percent[4];     /* Percentage of gas in atmosphere. */
    int16_t reserved2;          /* Reserved for future use. Zero for now. */
    int16_t diameter;           /* Diameter in thousands of kilometers. */
    int16_t gravity;            /* Surface gravity. Multiple of Earth gravity times 100. */
    int16_t mining_difficulty;  /* Mining difficulty times 100. */
    int16_t econ_efficiency;    /* Economic efficiency. Always 100 for a home planet. */
    int16_t md_increase;        /* Increase in mining difficulty. */
    int32_t message;            /* Message associated with this planet, if any. */
    int32_t reserved3;          /* Reserved for future use. Zero for now. */
    int32_t reserved4;          /* Reserved for future use. Zero for now. */
    int32_t reserved5;          /* Reserved for future use. Zero for now. */
};

typedef struct planet_data planet_data_t;

void get_planet_data(void);

void planetDataAsSExpr(FILE *fp);

void save_planet_data(void);

// globals. ugh.

extern char gas_string[14][4];
extern struct planet_data *planet;
extern struct planet_data *planet_base;
extern int planet_data_modified;

#endif //FAR_HORIZONS_PLANET_H
