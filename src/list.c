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
#include "list.h"
#include "nampla.h"
#include "namplavars.h"
#include "speciesvars.h"
#include "shipvars.h"
#include "planetvars.h"


void print_LSN(struct planet_data *planet, struct planet_data *home_planet);


int listCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    // load data used to derive locations
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    int list_planets = TRUE;
    int list_wormholes = FALSE;
    int listGalaxy = FALSE;
    int listScanned = FALSE;

    int spno = 0;
    int spidx = -1;
    nampla_data_t *home_nampla = NULL;
    planet_data_t *home_planet = NULL;

    for (int i = 1; i < argc; i++) {
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }

        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr, "fh: usage: list galaxy options...\n");
            fprintf(stderr, "           list all systems in the galaxy\n");
            fprintf(stderr, "      opt: --planets=bool     list planets   [default is false]\n");
            fprintf(stderr, "           --wormholes=bool   list wormholes [default is false]\n");
            fprintf(stderr, "fh: usage: list scanned options...\n");
            fprintf(stderr, "           list all systems scanned by a species\n");
            fprintf(stderr, "      opt: --species=integer  species number to scan for [required]\n");
            return 2;
        } else if (strcmp(opt, "--planets") == 0 && val != NULL && listGalaxy != FALSE) {
            if (strcmp(val, "true") == 0) {
                list_planets = TRUE;
            } else if (strcmp(val, "false") == 0) {
                list_planets = FALSE;
            } else {
                fprintf(stderr, "error: --planets requires either 'true' or 'false'\n");
                return 2;
            }
        } else if (strcmp(opt, "--wormholes") == 0 && val != NULL && listGalaxy != FALSE) {
            if (strcmp(val, "true") == 0) {
                list_wormholes = TRUE;
            } else if (strcmp(val, "false") == 0) {
                list_wormholes = FALSE;
            } else {
                fprintf(stderr, "error: --wormholes requires either 'true' or 'false'\n");
                return 2;
            }
        } else if (strcmp(opt, "--species") == 0 && val != NULL) {
            spno = atoi(val);
            spidx = spno - 1;
            if (!(1 <= spno && spno <= galaxy.num_species)) {
                fprintf(stderr, "error: invalid species number '%s'\n", opt);
                return 2;
            } else if (data_in_memory[spidx] == FALSE) {
                fprintf(stderr, "error: species %d is not loaded\n", spno);
                return 2;
            }
        } else if (strcmp(opt, "galaxy") == 0 && val == NULL) {
            if (listScanned != FALSE) {
                fprintf(stderr, "error: you must not specify both galaxy and scanned\n");
                return 2;
            }
            listGalaxy = TRUE;
        } else if (strcmp(opt, "scanned") == 0 && val == NULL) {
            if (listGalaxy != FALSE) {
                fprintf(stderr, "error: you must not specify both galaxy and scanned\n");
                return 2;
            }
            listScanned = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (listGalaxy == TRUE) {
        /* Initialize counts. */
        int total_planets = 0;
        int total_wormstars = 0;
        int type_count[10];
        for (int i = DWARF; i <= GIANT; i++) {
            type_count[i] = 0;
        }

        /* For each star, list info. */
        planet_data_t *planet = planet_base;
        for (int star_index = 0; star_index < num_stars; star_index++) {
            star_data_t *star = star_base + star_index;
            if (list_wormholes == FALSE) {
                if (list_planets != FALSE) {
                    printf("System #%d:\t", star_index + 1);
                }
                printf("x = %d\ty = %d\tz = %d", star->x, star->y, star->z);
                printf("\tstellar type = %c%c%c",
                       type_char[star->type], color_char[star->color], size_char[star->size]);
                if (list_planets != FALSE) {
                    printf("\t%d planets.", star->num_planets);
                }
                printf("\n");
                if (star->num_planets == 0) {
                    printf("\tStar #%d went nova! All planets were blown away!\n", star_index + 1);
                }
            }

            total_planets += star->num_planets;
            type_count[star->type] += 1;

            if (star->worm_here != FALSE) {
                total_wormstars++;
                if (list_planets != FALSE) {
                    printf("!!! Natural wormhole from here to %d %d %d\n",
                           star->worm_x, star->worm_y, star->worm_z);
                } else if (list_wormholes != FALSE) {
                    printf("Wormhole #%d: from %d %d %d to %d %d %d\n",
                           total_wormstars, star->x, star->y, star->z,
                           star->worm_x, star->worm_y, star->worm_z);
                    for (int i = 0; i < num_stars; i++) {
                        star_data_t *worm_star = star_base + i;
                        if (star->worm_x == worm_star->x && star->worm_y == worm_star->y &&
                            star->worm_z == worm_star->z) {
                            worm_star->worm_here = FALSE;
                            break;
                        }
                    }
                }
            }

            int home_system = FALSE;
            planet_data_t *home_planet = planet;
            if (list_planets != FALSE) {
                /* Check if system has a home planet. */
                for (int i = 1; i <= star->num_planets; i++) {
                    home_planet = planet + i;
                    if (home_planet->special == 1 || home_planet->special == 2) {
                        home_system = TRUE;
                        break;
                    }
                }
            }

            if (list_planets != FALSE) {
                for (int i = 1; i <= star->num_planets; i++) {
                    switch (planet->special) {
                        case 0:
                            printf("     ");
                            break;
                        case 1:
                            printf(" HOM ");
                            break;  /* Ideal home planet. */
                        case 2:
                            printf(" COL ");
                            break;  /* Ideal colony planet. */
                    }
                    printf("#%d dia=%3d g=%d.%02d tc=%2d pc=%2d md=%d.%02d", i,
                           planet->diameter,
                           planet->gravity / 100, planet->gravity % 100,
                           planet->temperature_class,
                           planet->pressure_class,
                           planet->mining_difficulty / 100, planet->mining_difficulty % 100);

                    if (home_system == FALSE) {
                        printf("  ");
                    } else {
                        print_LSN(planet, home_planet);
                    }

                    int num_gases = 0;
                    for (int n = 0; n < 4; n++) {
                        if (planet->gas_percent[n] > 0) {
                            if (num_gases > 0) { printf(","); }
                            printf("%s(%d%%)", gas_string[planet->gas[n]], planet->gas_percent[n]);
                            num_gases++;
                        }
                    }
                    if (num_gases == 0) {
                        printf("No atmosphere");
                    }
                    printf("\n");
                    planet++;
                }
            }
            if (list_planets != FALSE) {
                printf("\n");
            }
        }
        if (list_wormholes) {
            return 0;
        }
        /* Print summary. */
        printf("\nThe galaxy has a radius of %d parsecs.\n", galaxy.radius);
        printf("It contains %d dwarf stars, %d degenerate stars, ",
               type_count[DWARF], type_count[DEGENERATE]);
        printf("%d main sequence stars,\n    and %d giant stars, ",
               type_count[MAIN_SEQUENCE], type_count[GIANT]);
        printf("for a total of %d stars.\n", num_stars);
        if (list_planets != FALSE) {
            printf("The total number of planets in the galaxy is %d.\n", total_planets);
            printf("The total number of natural wormholes in the galaxy is %d.\n",
                   total_wormstars / 2);
            printf("The galaxy was designed for %d species.\n", galaxy.d_num_species);
            printf("A total of %d species have been designated so far.\n\n",
                   galaxy.num_species);
        }
        return 0;
    }

    if (listScanned == TRUE) {
        if (spno == 0) {
            fprintf(stderr, "error: you must specify species to scan for\n");
            return 2;
        }
        species = spec_data + spidx;
        species_number = spno;
        nampla_base = namp_data[spidx];
        ship_base = ship_data[spidx];
        home_nampla = namp_data[spidx];
        home_planet = planet_base + (long) home_nampla->planet_index;
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
                        if (ship->loading_point != 0) {
                            nampla_data_t *colony = NULL;
                            if (ship->loading_point == 9999) {
                                // use homeworld
                                colony = nampla_base + 0;
                            } else if (0 < ship->loading_point && ship->loading_point < species->num_namplas) {
                                colony = nampla_base + ship->loading_point;
                            }
                            if (colony == NULL) {
                                printf("\n\t                       load   %5d  ***internal error***",
                                       ship->loading_point);
                            } else {
                                printf("\n\t                       load   PL %s", colony->name);
                            }
                        }
                        if (ship->unloading_point != 0) {
                            nampla_data_t *colony = NULL;
                            if (ship->unloading_point == 9999) {
                                // use homeworld
                                colony = nampla_base + 0;
                            } else if (0 < ship->unloading_point && ship->unloading_point < species->num_namplas) {
                                colony = nampla_base + ship->unloading_point;
                            }
                            if (colony == NULL) {
                                printf("\n\t                       unload %5d  ***internal error***",
                                       ship->unloading_point);
                            } else {
                                printf("\n\t                       unload PL %s", colony->name);
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
                            printf("\n\t                       %s %s %d",
                                   item_abbr[item], item_name[item], ship->item_quantity[item]);
                        }
                    }
                }
                // are there any aliens here? print something?
                printf("\n");
            }
        }
        return 0;
    }

    fprintf(stderr, "error: you must specify galaxy or scanned\n");
    return 2;
}


/* This routine provides an approximate LSN for a planet.
 * It assumes that oxygen is required and any gas that does not appear on the home planet is poisonous. */
void print_LSN(struct planet_data *planet, struct planet_data *home_planet) {
    int j = planet->temperature_class - home_planet->temperature_class;
    if (j < 0) {
        j = -j;
    }
    int ls_needed = 2 * j;        /* Temperature class. */

    j = planet->pressure_class - home_planet->pressure_class;
    if (j < 0) {
        j = -j;
    }
    ls_needed += 2 * j;        /* Pressure class. */

    /* Check gases. */
    ls_needed += 2; // assumes oxygen is not present
    for (j = 0; j < 4; j++) {
        if (planet->gas[j] != 0) {
            if (planet->gas[j] == O2) {
                ls_needed -= 2;
            }
            int poison = TRUE;
            for (int k = 0; k < 4; k++) {
                /* Compare with home planet. */
                if (planet->gas[j] == home_planet->gas[k]) {
                    poison = FALSE;
                    break;
                }
            }
            if (poison != FALSE) {
                ls_needed += 2;
            }
        }
    }
    printf("%4d ", ls_needed);
}
