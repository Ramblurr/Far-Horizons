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

#ifndef FAR_HORIZONS_STAR_H
#define FAR_HORIZONS_STAR_H

#include <stdio.h>
#include "engine.h"
#include "ship.h"

/* In case gamemaster creates new star systems with Edit program. */
#define NUM_EXTRA_STARS    20

/* Star types. */
#define DWARF         1
#define DEGENERATE    2
#define MAIN_SEQUENCE 3
#define GIANT         4

/* Star Colors. */
#define BLUE         1
#define BLUE_WHITE   2
#define WHITE        3
#define YELLOW_WHITE 4
#define YELLOW       5
#define ORANGE       6
#define RED          7


int changeSystemToHomeSystem(star_data_t *star);

int chToStarColor(char ch);

int chToStarType(char ch);

void closest_unvisited_star(struct ship_data *ship);

void closest_unvisited_star_report(struct ship_data *ship, FILE *fp);

double distanceBetween(star_data_t *s1, star_data_t *s2);

star_data_t *findHomeSystemCandidate(int radius);

// hasHomeSystemNeighbor returns TRUE if the star has a neighbor within the given radius that is a home system.
int hasHomeSystemNeighbor(star_data_t *star, int radius);

void scan(int x, int y, int z, int printLSN);

char star_color(int c);

char star_size(int c);

char star_type(int c);

int star_visited(int x, int y, int z);


// globals. ugh.

extern char color_char[];

extern char size_char[];

extern char type_char[];

#endif //FAR_HORIZONS_STAR_H
