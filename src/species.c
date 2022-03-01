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
#include "galaxy.h"
#include "galaxyio.h"
#include "planet.h"
#include "species.h"
#include "speciesio.h"
#include "namplavars.h"
#include "shipvars.h"


int alien_is_visible(int x, int y, int z, int species_number, int alien_number) {
    int i, j;
    struct species_data *species, *alien;
    struct nampla_data *nampla, *alien_nampla;
    struct ship_data *alien_ship;

    /* Check if the alien has a ship or starbase here that is in orbit or in deep space. */
    alien = &spec_data[alien_number - 1];
    alien_ship = ship_data[alien_number - 1] - 1;
    for (i = 0; i < alien->num_ships; i++) {
        ++alien_ship;

        if (alien_ship->x != x) { continue; }
        if (alien_ship->y != y) { continue; }
        if (alien_ship->z != z) { continue; }
        if (alien_ship->item_quantity[FD] == alien_ship->tonnage) { continue; }

        if (alien_ship->status == IN_ORBIT || alien_ship->status == IN_DEEP_SPACE) {
            return TRUE;
        }
    }

    /* Check if alien has a planet that is not hidden. */
    alien_nampla = namp_data[alien_number - 1] - 1;
    for (i = 0; i < alien->num_namplas; i++) {
        ++alien_nampla;

        if (alien_nampla->x != x) { continue; }
        if (alien_nampla->y != y) { continue; }
        if (alien_nampla->z != z) { continue; }
        if ((alien_nampla->status & POPULATED) == 0) { continue; }

        if (!alien_nampla->hidden) { return TRUE; }

        /* The colony is hidden. See if we have population on the same planet. */
        species = &spec_data[species_number - 1];
        nampla = namp_data[species_number - 1] - 1;
        for (j = 0; j < species->num_namplas; j++) {
            ++nampla;

            if (nampla->x != x) { continue; }
            if (nampla->y != y) { continue; }
            if (nampla->z != z) { continue; }
            if (nampla->pn != alien_nampla->pn) { continue; }
            if ((nampla->status & POPULATED) == 0) { continue; }

            /* We have population on the same planet, so the alien
            cannot hide. */
            return TRUE;
        }
    }

    return FALSE;
}


/* The following routine provides the 'distorted' species number used to
	identify a species that uses field distortion units. The input
	variable 'species_number' is the same number used in filename
	creation for the species. */
int distorted(int species_number) {
    int i, j, n, ls;
    /* We must use the LS tech level at the start of the turn because
       the distorted species number must be the same throughout the
       turn, even if the tech level changes during production. */
    ls = spec_data[species_number - 1].init_tech_level[LS];
    i = species_number & 0x000F; /* Lower four bits. */
    j = (species_number >> 4) & 0x000F; /* Upper four bits. */
    n = (ls % 5 + 3) * (4 * i + j) + (ls % 11 + 7);
    return n;
}


// free_species_data will free memory used for all species data
void free_species_data(void) {
    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (namp_data[species_index] != NULL) {
            free(namp_data[species_index]);
            namp_data[species_index] = NULL;
        }
        if (ship_data[species_index] != NULL) {
            free(ship_data[species_index]);
            ship_data[species_index] = NULL;
        }
        data_in_memory[species_index] = FALSE;
        data_modified[species_index] = FALSE;
    }
}


/* Get life support tech level needed. */
int life_support_needed(struct species_data *species, struct planet_data *home, struct planet_data *colony) {
    // temperature class requires 3 points of LS per point of difference
    int tc = colony->temperature_class - home->temperature_class;
    if (tc < 0) {
        tc = -tc;
    }

    // pressure class requires 3 points of LS per point of difference
    int pc = colony->pressure_class - home->pressure_class;
    if (pc < 0) {
        pc = -pc;
    }

    /* Assuming required gas is NOT present. */
    int hasRequiredGas = FALSE;

    /* Check for poison gases on planet. */
    int poisonGases = 0;
    for (int j = 0; j < 4; j++) {
        // check if the slot has gas
        if (colony->gas_percent[j] != 0) {
            // check if required gas is present
            if (colony->gas[j] == species->required_gas) {
                // and in the right amount
                if (species->required_gas_min <= colony->gas_percent[j]
                    && colony->gas_percent[j] <= species->required_gas_max) {
                    hasRequiredGas = TRUE;
                }
            } else {
                // check if it is a poisonous gas
                for (int i = 0; i < 6; i++) {
                    if (colony->gas[j] == species->poison_gas[i]) {
                        poisonGases++;
                        break;
                    }
                }
            }
        }
    }

    // each point of difference and each poisonous gas requires 3 points of life support
    int ls_needed = 3 * (tc + pc + poisonGases);
    // add 3 more if the required gas is not present in the right amounts
    if (hasRequiredGas == FALSE) {
        ls_needed += 3;
    }

    return ls_needed;
}


int undistorted(int distorted_species_number) {
    int i, species_number;
    for (i = 0; i < MAX_SPECIES; i++) {
        species_number = i + 1;
        if (distorted(species_number) == distorted_species_number) {
            return species_number;
        }
    }
    return 0;    /* Not a legitimate species. */
}


