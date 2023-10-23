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
#include "galaxy.h"
#include "galaxyio.h"
#include "location.h"
#include "locationio.h"
#include "nampla.h"
#include "namplavars.h"
#include "planetio.h"
#include "planetvars.h"
#include "ship.h"
#include "shipvars.h"
#include "species.h"
#include "speciesio.h"
#include "speciesvars.h"


void add_location(int x, int y, int z) {
    for (int i = 0; i < num_locs; i++) {
        if (loc[i].x == x && loc[i].y == y && loc[i].z == z && loc[i].s == species_number) {
            return; /* This location is already in list for this species. */
        }
    }

    /* Add new location to the list. */
    loc[num_locs].x = x;
    loc[num_locs].y = y;
    loc[num_locs].z = z;
    loc[num_locs].s = species_number;
    num_locs++;
    if (num_locs < MAX_LOCATIONS) {
        return;
    }
    fprintf(stderr, "\n\n\tInternal error. Overflow of 'loc' arrays!\n\n");
    exit(-1);
}


/* This routine will create the "loc" array based on current species' data. */
void do_locations(void) {
    num_locs = 0;
    for (species_number = 1; species_number <= MAX_SPECIES; species_number++) {
        int spidx = species_number - 1;
        if (data_in_memory[spidx] == FALSE) {
            continue;
        }

        species = &spec_data[spidx];
        nampla_base = namp_data[spidx];
        ship_base = ship_data[spidx];

        nampla = nampla_base - 1;
        for (int i = 0; i < species->num_namplas; i++) {
            nampla++;
            if (nampla->pn == 99) {
                continue;
            }
            if (nampla->status & POPULATED) {
                add_location(nampla->x, nampla->y, nampla->z);
            }
        }

        ship = ship_base - 1;
        for (int i = 0; i < species->num_ships; i++) {
            ship++;
            if (ship->pn == 99) {
                continue;
            } else if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
                continue;
            }
            add_location(ship->x, ship->y, ship->z);
        }
    }
}


// locationCommand creates the location data file from the current data.
// also updates the economic efficiency field in the planet data file.
int locationCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    // load data used to derive locations
    printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    printf("fh: %s: loading   planet   data...\n", cmdName);
    get_planet_data();
    printf("fh: %s: loading   species  data...\n", cmdName);
    get_species_data();

    // allocate memory for array "total_econ_base"
    long *total_econ_base = (long *) ncalloc(__FUNCTION__, __LINE__, num_planets, sizeof(long));
    if (total_econ_base == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for total_econ_base!\n\n");
        exit(-1);
    }

    // initialize total econ base for each planet
    planet = planet_base;
    for (int i = 0; i < num_planets; i++) {
        total_econ_base[i] = 0;
        planet++;
    }

    // get total economic base for each planet from nampla data.
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        struct species_data *sp;
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

    // update economic efficiencies of all planets.
    planet = planet_base;
    for (int i = 0; i < num_planets; i++) {
        long diff = total_econ_base[i] - 2000;
        if (diff <= 0) {
            planet->econ_efficiency = 100;
        } else {
            planet->econ_efficiency = (100 * (diff / 20 + 2000)) / total_econ_base[i];
        }
        planet++;
    }

    // create new locations data
    do_locations();

    // save the results
    printf("fh: %s: saving    planet   data...\n", cmdName);
    save_planet_data(planet_base, num_planets);
    printf("fh: %s: saving    location data...\n", cmdName);
    save_location_data(loc, num_locs);

    // clean up
    free_species_data();
    free(planet_base);

    return 0;
}
