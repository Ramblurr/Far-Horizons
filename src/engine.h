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

#ifndef FAR_HORIZONS_ENGINE_H
#define FAR_HORIZONS_ENGINE_H

// declare constants
#define TRUE  1
#define FALSE 0

#define STANDARD_NUMBER_OF_SPECIES      15 /* A standard game has 15 species. */
#define STANDARD_NUMBER_OF_STAR_SYSTEMS 90 /* A standard game has 90 star systems. */
#define STANDARD_GALACTIC_RADIUS        20 /* A standard game has a galaxy with a radius of 20 parsecs. */

/* Minimum and maximum values for a galaxy. */
#define MIN_SPECIES  1
#define MAX_SPECIES  100
#define MIN_STARS    12
#define MAX_STARS    1000
#define MIN_RADIUS   6
#define MAX_RADIUS   50
#define MAX_DIAMETER (2*MAX_RADIUS)
#define MAX_PLANETS  (9*MAX_STARS)

#define HP_AVAILABLE_POP 1500

/* Assume at least 32 bits per long word. */
#define NUM_CONTACT_WORDS	((MAX_SPECIES - 1) / 32) + 1

int agrep_score(char *correct_string, char *unknown_string);
char *commas(long value);
int rnd(unsigned int max);

#endif //FAR_HORIZONS_ENGINE_H
