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
#include "location.h"
#include "no_orders.h"
#include "ordervars.h"

void NoOrdersForSpecies(void) {
    // from engine.c
    extern unsigned long last_random;

    // from galaxy.c
    extern struct galaxy_data galaxy;

    // from star.c
    extern int x;
    extern int y;
    extern int z;
    extern int num_stars;
    extern struct star_data *star_base;

    // from planet.c
    extern struct planet_data *planet_base;
    extern struct planet_data *planet;
    extern struct planet_data *home_planet;

    // from species.c
    extern int species_number;
    extern int species_index;
    extern int data_in_memory[MAX_SPECIES];
    extern struct species_data spec_data[MAX_SPECIES];
    extern struct species_data *species;

    // from nampla.c
    extern struct nampla_data *namp_data[MAX_SPECIES];
    extern struct nampla_data *nampla_base;

    // from ship.c
    extern struct ship_data *ship_data[MAX_SPECIES];
    extern struct ship_data *ship_base;
    extern char ship_type[3][2];
    extern int truncate_name;

    // from location.c
    extern int num_locs;
    extern struct sp_loc_data loc[MAX_LOCATIONS];

    int i;
    int j;
    int k;
    int ship_index;
    int locations_fd;
    int my_loc_index;
    int nampla_index;
    int its_loc_index;
    int tonnage;
    int found;
    int alien_number;
    int alien_index;
    int array_index;
    int bit_number;
    int ls_needed;
    int production_penalty;
    char filename[32];
    char *random_name();
    char message_line[132];
    long n;
    long nn;
    long raw_material_units;
    long production_capacity;
    long balance;
    long current_base;
    long CUs_needed;
    long IUs_needed;
    long AUs_needed;
    long EUs;
    long bit_mask;
    FILE *message_file;
    FILE *log_file;
    struct species_data *alien;
    struct nampla_data *nampla;
    struct nampla_data *home_nampla;
    struct nampla_data *temp_nampla;
    struct ship_data *ship;
    struct sp_loc_data *locations_base;
    struct sp_loc_data *my_loc;
    struct sp_loc_data *its_loc;

    species = &spec_data[species_index];
    nampla_base = namp_data[species_index];
    ship_base = ship_data[species_index];
    home_nampla = nampla_base;
    home_planet = planet_base + (int) home_nampla->planet_index;

    for (i = 0; i < species->num_ships; i++) {
        ship = ship_base + i;
        ship->special = 0;
    }

    /* Print message for gamemaster. */
    printf("Generating orders for species #%02d, SP %s...\n", species_number, species->name);

    /* Open message file. */
    sprintf(filename, "noorders.txt");
    message_file = fopen(filename, "r");
    if (message_file == NULL) {
        fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
        exit(-1);
    }

    /* Open log file. */
    sprintf(filename, "sp%02d.log", species_number);
    log_file = fopen(filename, "a");
    if (log_file == NULL) {
        fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
        exit(-1);
    }

    /* Copy message to log file. */
    while (readln(message_line, 131, message_file) != NULL) {
        fputs(message_line, log_file);
    }

    fclose(message_file);
    fclose(log_file);

    /* Open orders file for writing. */
    sprintf(filename, "sp%02d.ord", species_number);
    orders_file = fopen(filename, "w");
    if (orders_file == NULL) {
        fprintf(stderr, "\n\tCannot open '%s' for writing!\n\n", filename);
        exit(-1);
    }

    /* Issue PRE-DEPARTURE orders. */
    fprintf(orders_file, "START PRE-DEPARTURE\n");
    fprintf(orders_file, "; Place pre-departure orders here.\n\n");

    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
        nampla = nampla_base + nampla_index;
        if (nampla->pn == 99) {
            continue;
        }

        /* Generate auto-installs for colonies that were loaded via the DEVELOP command. */
        if (nampla->auto_IUs) {
            fprintf(orders_file, "\tInstall\t%d IU\tPL %s\n", nampla->auto_IUs, nampla->name);
        }
        if (nampla->auto_AUs) {
            fprintf(orders_file, "\tInstall\t%d AU\tPL %s\n", nampla->auto_AUs, nampla->name);
        }
        if (nampla->auto_IUs || nampla->auto_AUs) {
            fprintf(orders_file, "\n");
        }

        nampla->item_quantity[CU] -= nampla->auto_IUs + nampla->auto_AUs;

        /* Generate auto UNLOAD orders for transports at this nampla. */
        for (j = 0; j < species->num_ships; j++) {
            ship = ship_base + j;
            if (ship->pn == 99) {
                continue;
            } else if (ship->x != nampla->x) {
                continue;
            } else if (ship->y != nampla->y) {
                continue;
            } else if (ship->z != nampla->z) {
                continue;
            } else if (ship->pn != nampla->pn) {
                continue;
            } else if (ship->status == JUMPED_IN_COMBAT) {
                continue;
            } else if (ship->status == FORCED_JUMP) {
                continue;
            } else if (ship->class != TR) {
                continue;
            } else if (ship->item_quantity[CU] < 1) {
                continue;
            }

            /* New colonies will never be started automatically unless ship was loaded via a DEVELOP order. */
            if (ship->loading_point != 0) { /* Check if transport is at specified unloading point. */
                n = ship->unloading_point;
                if (n == nampla_index || (n == 9999 && nampla_index == 0)) {
                    goto unload_ship;
                }
            }

            if ((nampla->status & POPULATED) == 0) {
                continue;
            } else if ((nampla->mi_base + nampla->ma_base) >= 2000) {
                continue;
            } else if (nampla->x == nampla_base->x && nampla->y == nampla_base->y && nampla->z == nampla_base->z) {
                /* Home sector. */
                continue;
            }

            unload_ship:

            n = ship->loading_point;
            if (n == 9999) {
                /* Home planet. */
                n = 0;
            }
            if (n == nampla_index) {
                /* Ship was just loaded here. */
                continue;
            }

            fprintf(orders_file, "\tUnload\tTR%d%s %s\n\n", ship->tonnage, ship_type[ship->type], ship->name);

            nampla->item_quantity[CU] = 0;

            ship->special = ship->loading_point;
            n = nampla - nampla_base;
            if (n == 0) {
                n = 9999;
            }
            ship->unloading_point = n;
        }

        if (nampla->status & HOME_PLANET) {
            continue;
        } else if (nampla->item_quantity[CU] == 0) {
            continue;
        } else if (nampla->item_quantity[IU] == 0 && nampla->item_quantity[AU] == 0) {
            continue;
        }

        if (nampla->item_quantity[IU] > 0) {
            fprintf(orders_file, "\tInstall\t0 IU\tPL %s\n", nampla->name);
        }
        if (nampla->item_quantity[AU] > 0) {
            fprintf(orders_file, "\tInstall\t0 AU\tPL %s\n\n", nampla->name);
        }
    }

    fprintf(orders_file, "END\n\n");

    fprintf(orders_file, "START JUMPS\n");
    fprintf(orders_file, "; Place jump orders here.\n\n");

    /* Initialize to make sure ships are not given more than one JUMP order. */
    for (i = 0; i < species->num_ships; i++) {
        ship = ship_base + i;
        ship->just_jumped = FALSE;
    }

    /* Generate auto-jumps for ships that were loaded via the DEVELOP command or which were UNLOADed because of the AUTO command. */
    for (i = 0; i < species->num_ships; i++) {
        ship = ship_base + i;

        if (ship->status == JUMPED_IN_COMBAT) {
            continue;
        } else if (ship->status == FORCED_JUMP) {
            continue;
        } else if (ship->pn == 99) {
            continue;
        } else if (ship->just_jumped) {
            continue;
        }

        j = ship->special;
        if (j) {
            if (j == 9999) {
                /* Home planet. */
                j = 0;
            }
            temp_nampla = nampla_base + j;
            fprintf(orders_file, "\tJump\t%s, PL %s\t; ", ship_name(ship), temp_nampla->name);
            print_mishap_chance(ship, temp_nampla->x, temp_nampla->y, temp_nampla->z);
            fprintf(orders_file, "\n\n");
            ship->just_jumped = TRUE;
            continue;
        }

        n = ship->unloading_point;
        if (n != 0) {
            if (n == 9999) {
                /* Home planet. */
                n = 0;
            }
            temp_nampla = nampla_base + n;
            if (temp_nampla->x == ship->x && temp_nampla->y == ship->y && temp_nampla->z == ship->z) {
                continue;
            }
            fprintf(orders_file, "\tJump\t%s, PL %s\t; ", ship_name(ship), temp_nampla->name);
            print_mishap_chance(ship, temp_nampla->x, temp_nampla->y, temp_nampla->z);
            fprintf(orders_file, "\n\n");
            ship->just_jumped = TRUE;
        }
    }

    /* Generate JUMP orders for all TR1s. */
    for (i = 0; i < species->num_ships; i++) {
        ship = ship_base + i;
        if (ship->pn == 99) {
            continue;
        } else if (ship->status == UNDER_CONSTRUCTION) {
            continue;
        } else if (ship->status == JUMPED_IN_COMBAT) {
            continue;
        } else if (ship->status == FORCED_JUMP) {
            continue;
        } else if (ship->just_jumped) {
            continue;
        }

        if (ship->class == TR && ship->tonnage == 1 && ship->type == FTL) {
            fprintf(orders_file, "\tJump\tTR1 %s, ", ship->name);
            closest_unvisited_star(ship);
            fprintf(orders_file, "\n\t\t\t; Age %d, now at %d %d %d, ", ship->age, ship->x, ship->y, ship->z);
            print_mishap_chance(ship, x, y, z);
            ship->dest_x = x;
            ship->dest_y = y;
            ship->dest_z = z;
            fprintf(orders_file, "\n\n");
            ship->just_jumped = TRUE;
        }
    }

    fprintf(orders_file, "END\n\n");

    fprintf(orders_file, "START PRODUCTION\n");

    /* Generate a PRODUCTION order for each planet that can produce. */
    for (nampla_index = species->num_namplas - 1; nampla_index >= 0; nampla_index--) {
        nampla = nampla_base + nampla_index;
        if (nampla->pn == 99) {
            continue;
        } else if (nampla->mi_base == 0 && (nampla->status & RESORT_COLONY) == 0) {
            continue;
        } else if (nampla->ma_base == 0 && (nampla->status & MINING_COLONY) == 0) {
            continue;
        }
        fprintf(orders_file, "    PRODUCTION PL %s\n", nampla->name);
        if (nampla->status & MINING_COLONY) {
            fprintf(orders_file, "    ; The above PRODUCTION order is required for this mining colony, even\n");
            fprintf(orders_file, "    ;  if no other production orders are given for it.\n");
        } else if (nampla->status & RESORT_COLONY) {
            fprintf(orders_file, "    ; The above PRODUCTION order is required for this resort colony, even\n");
            fprintf(orders_file, "    ;  though no other production orders can be given for it.\n");
        } else {
            fprintf(orders_file, "    ; Place production orders here for planet %s.\n\n", nampla->name);
        }

        /* Build IUs and AUs for incoming ships with CUs. */
        if (nampla->IUs_needed) {
            fprintf(orders_file, "\tBuild\t%d IU\n", nampla->IUs_needed);
        }
        if (nampla->AUs_needed) {
            fprintf(orders_file, "\tBuild\t%d AU\n", nampla->AUs_needed);
        }
        if (nampla->IUs_needed || nampla->AUs_needed) {
            fprintf(orders_file, "\n");
        }
        if (nampla->status & MINING_COLONY) {
            continue;
        } else if (nampla->status & RESORT_COLONY) {
            continue;
        }

        /* See if there are any RMs to recycle. */
        n = nampla->special / 5;
        if (n > 0) {
            fprintf(orders_file, "\tRecycle\t%ld RM\n\n", 5 * n);
        }

        /* Generate DEVELOP commands for ships arriving here because of AUTO command. */
        for (i = 0; i < species->num_ships; i++) {
            ship = ship_base + i;
            if (ship->pn == 99) {
                continue;
            }
            k = ship->special;
            if (k == 0) {
                continue;
            }
            if (k == 9999) {
                /* Home planet. */
                k = 0;
            }
            if (nampla != nampla_base + k) {
                continue;
            }
            k = ship->unloading_point;
            if (k == 9999) {
                /* Home planet. */
                k = 0;
            }
            temp_nampla = nampla_base + k;
            fprintf(orders_file, "\tDevelop\tPL %s, TR%d%s %s\n\n", temp_nampla->name, ship->tonnage,
                    ship_type[ship->type], ship->name);
        }

        /* Give orders to continue construction of unfinished ships and starbases. */
        for (i = 0; i < species->num_ships; i++) {
            ship = ship_base + i;
            if (ship->pn == 99) {
                continue;
            } else if (ship->x != nampla->x) {
                continue;
            } else if (ship->y != nampla->y) {
                continue;
            } else if (ship->z != nampla->z) {
                continue;
            } else if (ship->pn != nampla->pn) {
                continue;
            } else if (ship->status == UNDER_CONSTRUCTION) {
                fprintf(orders_file, "\tContinue\t%s, %d\t; Left to pay = %d\n\n", ship_name(ship),
                        ship->remaining_cost, ship->remaining_cost);
                continue;
            } else if (ship->type != STARBASE) {
                continue;
            }
            j = (species->tech_level[MA] / 2) - ship->tonnage;
            if (j < 1) {
                continue;
            }
            fprintf(orders_file, "\tContinue\tBAS %s, %d\t; Current tonnage = %s\n\n", ship->name, 100 * j,
                    commas(10000 * (long) ship->tonnage));
        }

        /* Generate DEVELOP command if this is a colony with an economic base less than 200. */
        n = nampla->mi_base + nampla->ma_base + nampla->IUs_needed + nampla->AUs_needed;
        if ((nampla->status & COLONY) && n < 2000 && nampla->pop_units > 0) {
            if (nampla->pop_units > (2000L - n)) {
                nn = 2000L - n;
            } else {
                nn = nampla->pop_units;
            }
            fprintf(orders_file, "\tDevelop\t%ld\n\n", 2L * nn);
            nampla->IUs_needed += nn;
        }

        /* For home planets and any colonies that have an economic base of at least 200, check if there are other colonized planets in the same sector that are not self-sufficient.  If so, DEVELOP them. */
        if (n >= 2000 || (nampla->status & HOME_PLANET)) {
            for (i = 1; i < species->num_namplas; i++) {/* Skip HP. */
                if (i == nampla_index) {
                    continue;
                }
                temp_nampla = nampla_base + i;
                if (temp_nampla->pn == 99) {
                    continue;
                } else if (temp_nampla->x != nampla->x) {
                    continue;
                } else if (temp_nampla->y != nampla->y) {
                    continue;
                } else if (temp_nampla->z != nampla->z) {
                    continue;
                }
                n = temp_nampla->mi_base + temp_nampla->ma_base + temp_nampla->IUs_needed + temp_nampla->AUs_needed;
                if (n == 0) {
                    continue;
                }
                nn = temp_nampla->item_quantity[IU] + temp_nampla->item_quantity[AU];
                if (nn > temp_nampla->item_quantity[CU]) {
                    nn = temp_nampla->item_quantity[CU];
                }
                n += nn;
                if (n >= 2000L) {
                    continue;
                }
                nn = 2000L - n;
                if (nn > nampla->pop_units) {
                    nn = nampla->pop_units;
                }
                fprintf(orders_file, "\tDevelop\t%ld\tPL %s\n\n", 2L * nn, temp_nampla->name);
                temp_nampla->AUs_needed += nn;
            }
        }
    }

    fprintf(orders_file, "END\n\n");

    fprintf(orders_file, "START POST-ARRIVAL\n");
    fprintf(orders_file, "; Place post-arrival orders here.\n\n");

    /* Generate an AUTO command. */
    fprintf(orders_file, "\tAuto\n\n");

    /* Generate SCAN orders for all TR1s in sectors that current species does not inhabit. */
    for (i = 0; i < species->num_ships; i++) {
        ship = ship_base + i;
        if (ship->pn == 99) {
            continue;
        } else if (ship->status == UNDER_CONSTRUCTION) {
            continue;
        } else if (ship->class != TR) {
            continue;
        } else if (ship->tonnage != 1) {
            continue;
        } else if (ship->type != FTL) {
            continue;
        } else if (ship->dest_x == -1) {
            /* Not jumping anywhere. */
            continue;
        }
        found = FALSE;
        for (j = 1; j < species->num_namplas; j++) {
            /* Skip home sector. */
            nampla = nampla_base + j;
            if (nampla->pn == 99) {
                continue;
            } else if (nampla->x != ship->dest_x) {
                continue;
            } else if (nampla->y != ship->dest_y) {
                continue;
            } else if (nampla->z != ship->dest_z) {
                continue;
            } else if (nampla->status & POPULATED) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            fprintf(orders_file, "\tScan\tTR1 %s\n", ship->name);
        }
    }

    fprintf(orders_file, "END\n\n");

    /* Clean up for this species. */
    fclose(orders_file);

}

void print_mishap_chance(struct ship_data *ship, int destx, int desty, int destz) {
    // from species.c
    extern struct species_data *species;

    // from star.c
    extern int num_stars;
    extern struct star_data *star_base;
    extern int x;
    extern int y;
    extern int z;

    int mishap_GV;
    int mishap_age;
    long stx;
    long sty;
    long stz;
    long mishap_chance;
    long success_chance;
    long temp_distance;

    if (destx == -1) {
        fprintf(orders_file, "Mishap chance = ???");
        return;
    }

    stx = destx;
    sty = desty;
    stz = destz;
    temp_distance = ((stx - ship->x) * (stx - ship->x)) + ((sty - ship->y) * (sty - ship->y)) +
                    ((stz - ship->z) * (stz - ship->z));

    mishap_age = ship->age;
    mishap_GV = species->tech_level[GV];
    if (mishap_GV > 0) {
        mishap_chance = 100 * temp_distance / mishap_GV;
    } else {
        mishap_chance = 10000;
    }
    if (mishap_age > 0 && mishap_chance < 10000) {
        success_chance = 10000L - mishap_chance;
        success_chance -= (2L * (long) mishap_age * success_chance) / 100L;
        mishap_chance = 10000L - success_chance;
    }
    if (mishap_chance > 10000) {
        mishap_chance = 10000;
    }
    fprintf(orders_file, "mishap chance = %ld.%02ld%%", mishap_chance / 100L, mishap_chance % 100L);
}
