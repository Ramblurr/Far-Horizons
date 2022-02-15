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

// This program will generate default orders for a species if no explicit orders have been provided.

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "engine.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "stario.h"
#include "planet.h"
#include "planetio.h"
#include "species.h"
#include "speciesio.h"
#include "locationio.h"
#include "no_orders.h"

int main(int argc, char *argv[]) {
    // from engine.c
    extern unsigned long last_random;
    // from galaxy.c
    extern struct galaxy_data galaxy;
    // from species.c
    extern int species_number;
    extern int species_index;
    extern int data_in_memory[MAX_SPECIES];
    // from ship.c
    extern int truncate_name;

    /* Check for valid command line. */
    if (argc != 1) {
        fprintf(stderr, "\n\tUsage: NoOrders\n\n");
        exit(0);
    }

    /* Seed random number generator. */
    last_random = time(NULL);
    for (int i = 0; i < 907; i++) {
        rnd(100);
    }

    /* Get all necessary data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();
    get_location_data();

    truncate_name = TRUE;

    /* Major loop. Check each species in the game. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        char filename[32];
        struct stat sb;
        species_index = species_number - 1;
        /* Check if this species is still in the game. */
        if (!data_in_memory[species_index]) {
            fprintf(stderr, "[fhorders] species %2d is not in memory\n", species_number);
            continue;
        }
        /* Check if we have orders. */
        sprintf(filename, "sp%02d.ord", species_number);
        if (stat(filename, &sb) == 0) {
            // file exists
            continue;
        }
        // no file, so do our thing, whatever that is
        NoOrdersForSpecies();
    }

    /* Clean up and exit. */
    free_species_data();
    return 0;
}
