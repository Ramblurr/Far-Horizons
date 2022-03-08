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
#include "star.h"

/* In case gamemaster creates new star systems with Edit program. */
#define NUM_EXTRA_PLANETS    100

/* Gases in planetary atmospheres. */
#define    H2     1    /* Hydrogen */
#define    CH4    2    /* Methane */
#define    HE     3    /* Helium */
#define    NH3    4    /* Ammonia */
#define    N2     5    /* Nitrogen */
#define    CO2    6    /* Carbon Dioxide */
#define    O2     7    /* Oxygen */
#define    HCL    8    /* Hydrogen Chloride */
#define    CL2    9    /* Chlorine */
#define    F2    10    /* Fluorine */
#define    H2O   11    /* Steam */
#define    SO2   12    /* Sulfur Dioxide */
#define    H2S   13    /* Hydrogen Sulfide */


int createHomeSystemTemplates();

void fix_gases(struct planet_data *pl);

// generate_planets creates planets and inserts them into the planet_data array.
void generate_planets(struct planet_data *first_planet, int num_planets, int earth_like, int makeMiningEasier);

#endif //FAR_HORIZONS_PLANET_H
