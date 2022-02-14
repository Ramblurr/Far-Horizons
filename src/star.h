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

struct star_data {
    char x;             /* Coordinates. */
    char y;
    char z;
    char type;          /* Dwarf, degenerate, main sequence or giant. */
    char color;         /* Star color. Blue, blue-white, etc. */
    char size;          /* Star size, from 0 thru 9 inclusive. */
    char num_planets;   /* Number of usable planets in star system. */
    char home_system;   /* TRUE if this is a good potential home system. */
    char worm_here;     /* TRUE if wormhole entry/exit. */
    char worm_x;        /* Coordinates of wormhole's exit. */
    char worm_y;
    char worm_z;
    short reserved1;    /* Reserved for future use. Zero for now. */
    short reserved2;    /* Reserved for future use. Zero for now. */
    short planet_index; /* Index (starting at zero) into the file "planets.dat" of the first planet in the star system. */
    long message;       /* Message associated with this star system, if any. */
    long visited_by[NUM_CONTACT_WORDS]; /* A bit is set if corresponding species has been here. */
    long reserved3;     /* Reserved for future use. Zero for now. */
    long reserved4;     /* Reserved for future use. Zero for now. */
    long reserved5;     /* Reserved for future use. Zero for now. */
};

void closest_unvisited_star(struct ship_data *ship);

void get_star_data(void);

void scan(int x, int y, int z);

void save_star_data(void);

int star_visited(int x, int y, int z);

#endif //FAR_HORIZONS_STAR_H
