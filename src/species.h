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

#ifndef FAR_HORIZONS_SPECIES_H
#define FAR_HORIZONS_SPECIES_H

#include <stdint.h>
#include <stdio.h>
#include "engine.h"


/* Tech level ids. */
#define MI 0 /* Mining tech level. */
#define MA 1 /* Manufacturing tech level. */
#define ML 2 /* Military tech level. */
#define GV 3 /* Gravitics tech level. */
#define LS 4 /* Life Support tech level. */
#define BI 5 /* Biology tech level. */


int alien_is_visible(int x, int y, int z, int species_number, int alien_number);

int distorted(int species_number);

void free_species_data(void);

int life_support_needed(struct species_data *species, struct planet_data *home, struct planet_data *colony);

int undistorted(int distorted_species_number);

#endif //FAR_HORIZONS_SPECIES_H
