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
#include "star.h"
#include "planet.h"
#include "species.h"
#include "nampla.h"
#include "ship.h"
#include "log.h"
#include "no_orders.h"

char color_char[] = " OBAFGKM";
int print_LSN = TRUE;
int num_stars;
char size_char[] = "0123456789";
struct star_data *star;
struct star_data *star_base;
int star_data_modified;
char type_char[] = " dD g";
int x;
int y;
int z;
int pn;

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

void get_star_data(void) {
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


void save_star_data(void) {
    /* Open star file for writing. */
    FILE *fp = fopen("stars.dat", "wb");
    if (fp == NULL) {
        perror("save_star_data");
        fprintf(stderr, "\n\tCannot create file 'stars.dat'!\n");
        exit(-1);
    }
    /* Write header data. */
    if (fwrite(&num_stars, sizeof(num_stars), 1, fp) != 1) {
        perror("save_star_data");
        fprintf(stderr, "\n\tCannot write num_stars to file 'stars.dat'!\n\n");
        exit(-1);
    }
    /* Write star data to disk. */
    if (fwrite(star_base, sizeof(struct star_data), num_stars, fp) != num_stars) {
        perror("save_star_data");
        fprintf(stderr, "\nCannot write star data to disk!\n\n");
        exit(-1);
    }
    fclose(fp);
}


void scan(int x, int y, int z) {
    int i, j, k, n, found, num_gases, ls_needed;
    char filename[32];
    struct star_data *star;
    struct planet_data *planet, *home_planet;
    struct nampla_data *home_nampla;

    /* Find star. */
    star = star_base;
    found = FALSE;
    for (i = 0; i < num_stars; i++) {
        if (star->x == x && star->y == y && star->z == z) {
            found = TRUE;
            break;
        }
        ++star;
    }

    if (!found) {
        fprintf(log_file,
                "Scan Report: There is no star system at x = %d, y = %d, z = %d.\n",
                x, y, z);
        return;
    }

    /* Print data for star, */
    fprintf(log_file, "Coordinates:\tx = %d\ty = %d\tz = %d", x, y, z);
    fprintf(log_file, "\tstellar type = %c%c%c", type_char[star->type], color_char[star->color], size_char[star->size]);

    fprintf(log_file, "   %d planets.\n\n", star->num_planets);

    if (star->worm_here) {
        fprintf(log_file,
                "This star system is the terminus of a natural wormhole.\n\n");
    }

    /* Print header. */
    fprintf(log_file, "               Temp  Press Mining\n");
    fprintf(log_file, "  #  Dia  Grav Class Class  Diff  LSN  Atmosphere\n");
    fprintf(log_file, " ---------------------------------------------------------------------\n");

    /* Check for nova. */
    if (star->num_planets == 0) {
        fprintf(log_file, "\n\tThis star is a nova remnant. Any planets it may have once\n");
        fprintf(log_file, "\thad have been blown away.\n\n");
        return;
    }

    /* Print data for each planet. */
    planet = planet_base + (long) star->planet_index;
    if (print_LSN) {
        home_nampla = nampla_base;
        home_planet = planet_base + (long) home_nampla->planet_index;
    }

    for (i = 1; i <= star->num_planets; i++) {
        /* Get life support tech level needed. */
        if (print_LSN) {
            ls_needed = life_support_needed(species, home_planet, planet);
        } else {
            ls_needed = 99;
        }

        fprintf(log_file, "  %d  %3d  %d.%02d  %2d    %2d    %d.%02d %4d  ",
                i,
                planet->diameter,
                planet->gravity / 100,
                planet->gravity % 100,
                planet->temperature_class,
                planet->pressure_class,
                planet->mining_difficulty / 100,
                planet->mining_difficulty % 100,
                ls_needed);

        num_gases = 0;
        for (n = 0; n < 4; n++) {
            if (planet->gas_percent[n] > 0) {
                if (num_gases > 0) { fprintf(log_file, ","); }
                fprintf(log_file, "%s(%d%%)", gas_string[planet->gas[n]], planet->gas_percent[n]);
                ++num_gases;
            }
        }

        if (num_gases == 0) { fprintf(log_file, "No atmosphere"); }

        fprintf(log_file, "\n");
        ++planet;
    }

    if (star->message) {
        /* There is a message that must be logged whenever this star
            system is scanned. */
        sprintf(filename, "message%ld.txt\0", star->message);
        log_message(filename);
    }

    return;
}


/* The following routine will check if coordinates x-y-z contain a star and,
 * if so, will set the appropriate bit in the "visited_by" variable for the star.
 * If the star exists, TRUE will be returned; otherwise, FALSE will be returned. */
int star_visited(int x, int y, int z) {
    int found = FALSE;

    /* Get array index and bit mask. */
    int species_array_index = (species_number - 1) / 32;
    int species_bit_number = (species_number - 1) % 32;
    long species_bit_mask = 1 << species_bit_number;

    for (int i = 0; i < num_stars; i++) {
        struct star_data *star = star_base + i;
        if (x != star->x) { continue; }
        if (y != star->y) { continue; }
        if (z != star->z) { continue; }
        found = TRUE;
        /* Check if bit is already set. */
        if (star->visited_by[species_array_index] & species_bit_mask) { break; }
        /* Set the appropriate bit. */
        star->visited_by[species_array_index] |= species_bit_mask;
        star_data_modified = TRUE;
        break;
    }

    return found;
}
