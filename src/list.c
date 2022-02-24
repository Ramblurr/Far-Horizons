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
#include <string.h>
#include "galaxy.h"
#include "galaxyio.h"
#include "planetio.h"
#include "speciesio.h"
#include "stario.h"
#include "starvars.h"
#include "list.h"
#include "nampla.h"
#include "namplavars.h"
#include "speciesvars.h"
#include "shipvars.h"

int listCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    // load data used to derive locations
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    int spno = 0;
    int spidx = -1;
    nampla_data_t *home_nampla = NULL;
    planet_data_t *home_planet = NULL;

    for (int i = 1; i < argc; i++) {
        const char *opt = argv[i];
        if (spno == 0) {
            spno = atoi(opt);
            spidx = spno - 1;
            if (!(1 <= spno && spno <= galaxy.num_species)) {
                fprintf(stderr, "error: invalid species number '%s'\n", opt);
                return 2;
            } else if (data_in_memory[spidx] == FALSE) {
                fprintf(stderr, "error: species %d is not loaded\n", spno);
                return 2;
            }
            species = spec_data + spidx;
            species_number = spno;
            nampla_base = namp_data[spidx];
            ship_base = ship_data[spidx];
            home_nampla = namp_data[spidx];
            home_planet = planet_base + (long) home_nampla->planet_index;
        } else if (strcmp(opt, "scanned") == 0) {
            for (int j = 0; j < num_stars; j++) {
                star_data_t *star = star_base + j;
                // list only if the species has visited this system
                if ((star->visited_by[spidx / 32] & (1 << (spidx % 32))) == 0) {
                    continue;
                }
                printf("System  x = %d  y = %d  z = %d\n", star->x, star->y, star->z);
                if (star->worm_here == TRUE) {
                    printf("\tThis star system is the terminus of a natural wormhole.\n");
                }
                /* Check for nova. */
                if (star->num_planets == 0) {
                    printf("\tThis star is a nova remnant.\n");
                    printf("\t Any planets it may have once had have been blown away.\n\n");
                } else {
                    // list all the planets and colonies in the system
                    printf("\t#  Grav  MIDiff  LSN   Details\n");
                    printf("\t-----------------------------------------------------\n");
                    for (int pidx = 0; pidx < star->num_planets; pidx++) {
                        planet_data_t *planet = planet_base + star->planet_index + pidx;
                        int pn = pidx + 1;
                        char lsFlag = ' ';
                        int lsNeeded = life_support_needed(species, home_planet, planet);
                        if (lsNeeded <= species->tech_level[LS]) {
                            lsFlag = '*';
                        }
                        printf("\t%d  %d.%02d  %3d.%02d %4d%c  ",
                               pn,
                               planet->gravity / 100, planet->gravity % 100,
                               planet->mining_difficulty / 100, planet->mining_difficulty % 100,
                               lsNeeded, lsFlag);
                        int printLead = FALSE;
                        // is there a colony here? print the colony and inventory.
                        for (int npidx = 0; npidx < species->num_namplas; npidx++) {
                            nampla_data_t *colony = nampla_base + npidx;
                            if (star->x != colony->x || star->y != colony->y || star->z != colony->z ||
                                pn != colony->pn) {
                                continue;
                            }
                            printf("PL %s", colony->name);
                            printLead = TRUE;
                            for (int item = 0; item < MAX_ITEMS; item++) {
                                if (colony->item_quantity[item] > 0) {
                                    printf("\n\t                       %s %s %d",
                                           item_abbr[item], item_name[item], colony->item_quantity[item]);
                                }
                            }
                        }
                        // are there any ships here? print the ship and inventory.
                        for (int shidx = 0; shidx < species->num_ships; shidx++) {
                            ship_data_t *ship = ship_base + shidx;
                            if (star->x != ship->x || star->y != ship->y || star->z != ship->z || pn != ship->pn) {
                                continue;
                            }
                            printf("\n\t                    -- %s", ship_name(ship));
                            for (int item = 0; item < MAX_ITEMS; item++) {
                                if (ship->item_quantity[item] > 0) {
                                    printf("\n\t                       %s %s %d",
                                           item_abbr[item], item_name[item], ship->item_quantity[item]);
                                }
                            }
                        }
                        printf("\n");
                    }
                    // are there any ships in deep orbit? print the ship and inventory.
                    int foundDeepOrbiters = FALSE;
                    for (int shidx = 0; shidx < species->num_ships; shidx++) {
                        ship_data_t *ship = ship_base + shidx;
                        if (star->x != ship->x || star->y != ship->y || star->z != ship->z || ship->pn != 0) {
                            continue;
                        }
                        if (foundDeepOrbiters == FALSE) {
                            printf("\n\tShips in deep orbit -------------------------------");
                            foundDeepOrbiters = TRUE;
                        }
                        printf("\n\t                    -- %s", ship_name(ship));
                        for (int item = 0; item < MAX_ITEMS; item++) {
                            if (ship->item_quantity[item] > 0) {
                                printf("\n\t                       %s %s %d", item_abbr[item], item_name[item],
                                       ship->item_quantity[item]);
                            }
                        }
                    }
                    // are there any aliens here? print something?
                }
            }
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }
    return 0;
}