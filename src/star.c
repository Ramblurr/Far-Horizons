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
#include "species.h"
#include "ship.h"
#include "star.h"

// from no_orders.c
extern FILE *orders_file;

// from species.c
extern int species_number;
extern struct species_data *species;

int num_stars;
struct star_data *star_base;
int star_data_modified;

// global coordinates argh
int x;
int y;
int z;

void closest_unvisited_star(struct ship_data *ship) {
    int found = FALSE;
    long shx;
    long shy;
    long shz;
    long stx;
    long sty;
    long stz;
    long closest_distance = 999999;
    long temp_distance;
    struct star_data *star;
    struct star_data *closest_star;

/* Get array index and bit mask. */
    int species_array_index = (species_number - 1) / 32;
    int species_bit_number = (species_number - 1) % 32;
    long species_bit_mask = 1 << species_bit_number;

    shx = ship->x;
    shy = ship->y;
    shz = ship->z;

    x = -1;

    for (int i = 0; i < num_stars; i++) {
        star = star_base + i;

/* Check if bit is already set. */
        if (star->visited_by[species_array_index] & species_bit_mask) {
            continue;
        }

        stx = star->x;
        sty = star->y;
        stz = star->z;
        temp_distance = ((shx - stx) * (shx - stx)) + ((shy - sty) * (shy - sty)) + ((shz - stz) * (shz - stz));

        if (temp_distance < closest_distance) {
            x = stx;
            y = sty;
            z = stz;
            closest_distance = temp_distance;
            closest_star = star;
            found = TRUE;
        }
    }

    if (found) {
        fprintf(orders_file, "%d %d %d", x, y, z);
        /* So that we don't send more than one ship to the same place. */
        closest_star->visited_by[species_array_index] |= species_bit_mask;
    } else {
        fprintf(orders_file, "???");
    }
}

void get_star_data (void) {
    /* Open star file. */
    FILE *fp = fopen("stars.dat", "rb");
    if (fp == NULL) {
        fprintf(stderr, "\n\tCannot open file stars.dat!\n");
        exit(-1);
    }
    /* Read header data. */
    if (fread(&num_stars, sizeof(num_stars), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read num_stars in file 'stars.dat'!\n\n");
        exit(-1);
    }
    /* Allocate enough memory for all stars. */
    star_base = (struct star_data *) calloc(num_stars + NUM_EXTRA_STARS, sizeof(struct star_data));
    if (star_base == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for star file!\n\n");
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(star_base, sizeof(struct star_data), num_stars, fp) != num_stars) {
        fprintf(stderr, "\nCannot read star file into memory!\n\n");
        exit(-1);
    }
    fclose(fp);
    star_data_modified = FALSE;
}
