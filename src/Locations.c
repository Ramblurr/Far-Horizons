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

// This program will create the file locations.dat and will update the
// economic efficiencies of all planets.
// These functions are also performed by Finish.c.
// This program should be run before the strike phase or whenever manual
// changes are made to the species data files that resulted in something
// not being where it was or something being where it was not.
// It should also be run if you run Finish on fewer than all species and
// decide to keep the resulting planets.dat file.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "engine.h"
#include "galaxy.h"
#include "planet.h"
#include "species.h"
#include "nampla.h"
#include "ship.h"
#include "location.h"

// from galaxy.c
extern struct galaxy_data galaxy;

// from planet.c
extern int num_planets;
extern struct planet_data *planet;
extern struct planet_data *planet_base;

// from species.c
extern int data_in_memory[];
extern int data_modified[];
extern struct species_data *spec_data;
extern struct species_data *species;
extern int species_number;

// from nampla.c
extern struct nampla_data *namp_data[MAX_SPECIES];
extern struct nampla_data *nampla_base;
extern struct nampla_data *nampla;

// from ship.c
extern struct ship_data *ship_base;
extern struct ship_data *ship;

// from location.c
extern int num_locs;
extern struct sp_loc_data loc[MAX_LOCATIONS];

int species_index;
int test_mode;
int verbose_mode;

int main(int argc, char *argv[]) {
    int i;
    int nampla_index;
    long diff;
    long total;
    long *total_econ_base;
    FILE *fp;

    /* Check for options, if any. */
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        }
    }

/* Get commonly used data. */
    get_galaxy_data();
    get_planet_data();
    get_species_data();

/* Allocate memory for array "total_econ_base". */
    total_econ_base = (long *) calloc(num_planets, sizeof(long));
    if (total_econ_base == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for total_econ_base!\n\n");
        exit(-1);
    }

/* Initialize total econ base for each planet. */
    planet = planet_base;
    for (i = 0; i < num_planets; i++) {
        total_econ_base[i] = 0;
        planet++;
    }

/* Get total economic base for each planet from nampla data. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        if (data_in_memory[species_number - 1] == FALSE) {
            continue;
        }
        data_modified[species_number - 1] = TRUE;

        species = &spec_data[species_number - 1];
        nampla_base = namp_data[species_number - 1];

        for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
            nampla = nampla_base + nampla_index;
            if (nampla->pn == 99) {
                continue;
            }
            if ((nampla->status & HOME_PLANET) == 0) {
                total_econ_base[nampla->planet_index] += nampla->mi_base + nampla->ma_base;
            }
        }
    }

/* Update economic efficiencies of all planets. */
    planet = planet_base;
    for (i = 0; i < num_planets; i++) {
        total = total_econ_base[i];
        diff = total - 2000;
        if (diff <= 0) {
            planet->econ_efficiency = 100;
        } else {
            planet->econ_efficiency = (100 * (diff / 20 + 2000)) / total;
        }
        planet++;
    }

/* Create new locations array. */
    do_locations();

/* Clean up and exit. */
    save_location_data();

    fp = fopen("locations.txt", "wb");
    if (fp == NULL) {
        perror("locations: main: unable to create 'locations.txt'\n");
        exit(2);
    }
    locationDataAsSExpr(fp);
    fclose(fp);

    save_planet_data();

    fp = fopen("planets.txt", "wb");
    if (fp == NULL) {
        perror("locations: main: unable to create 'planets.txt'\n");
        exit(2);
    }
    planetDataAsSExpr(fp);
    fclose(fp);

    free_species_data();
    free(planet_base);

    exit(0);
}
