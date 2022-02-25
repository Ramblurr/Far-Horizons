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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "galaxy.h"
#include "galaxyio.h"
#include "logvars.h"
#include "namplavars.h"
#include "planetio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "stario.h"
#include "scan.h"


typedef struct {
    star_data_t *star;
    double distance;
} scan_system_t;

// scanCommand performs a scan on a system for a species.
// It is for use by the game master only.
int scanCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    if (argc != 5) {
        fprintf(stderr, "usage: fh scan speciesNumber x y z\n");
        return 2;
    }
    int spno = atoi(argv[1]);
    int spidx = spno - 1;
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);
    int z = atoi(argv[4]);

    printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    if (spno < 1 || spno > galaxy.num_species) {
        fprintf(stderr, "error: invalid species number\n");
    } else if (x < 0 || x > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid x coordinate\n");
    } else if (y < 0 || y > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid y coordinate\n");
    } else if (z < 0 || z > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid z coordinate\n");
    }
    printf("fh: %s: loading   star     data...\n", cmdName);
    get_star_data();
    printf("fh: %s: loading   planet   data...\n", cmdName);
    get_planet_data();
    printf("fh: %s: loading   species  data...\n", cmdName);
    get_species_data();

    printf("Scan for SP %s:\n", spec_data[spidx].name);

    // set external globals for the scan command
    ignore_field_distorters = TRUE;
    log_file = stdout;
    species_number = spno;
    species_index = spidx;
    species = &spec_data[species_index];
    nampla_base = namp_data[species_index];

    // display scan for the location
    scan(x, y, z);

    return 0;
}


// scanNearCommand scans all systems near a location.
// It is for use by the game master only.
int scanNearCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    int x = 0;
    int y = 0;
    int z = 0;
    int radius = 0;
    int systems = FALSE;

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
            fprintf(stderr,
                    "fh: usage: %s --x=integer --y=integer --z=integer --radius=integer\n", cmdName);
            return 2;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            radius = atoi(val);
        } else if (strcmp(opt, "--x") == 0 && val != NULL) {
            x = atoi(val);
        } else if (strcmp(opt, "--y") == 0 && val != NULL) {
            y = atoi(val);
        } else if (strcmp(opt, "--z") == 0 && val != NULL) {
            z = atoi(val);
        } else if (strcmp(opt, "--systems") == 0) {
            systems = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    int radiusSquared = radius * radius;

    // external globals?
    ignore_field_distorters = TRUE;
    log_file = stdout;

    //printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    if (x < 0 || x > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid x coordinate\n");
    } else if (y < 0 || y > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid y coordinate\n");
    } else if (z < 0 || z > 2 * galaxy.radius) {
        fprintf(stderr, "error: invalid z coordinate\n");
    } else if (radius < 0 || radius > galaxy.radius) {
        fprintf(stderr, "error: invalid radius\n");
    }
    //printf("fh: %s: loading   star     data...\n", cmdName);
    get_star_data();
    //printf("fh: %s: loading   planet   data...\n", cmdName);
    get_planet_data();
    //printf("fh: %s: loading   species  data...\n", cmdName);
    get_species_data();

    /* Display scan. */
    if (systems == FALSE) {
        printf("Ships and populated planets within %d parsecs of %d %d %d:\n", radius, x, y, z);
        for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
            if (!data_in_memory[spidx]) {
                continue;
            }
            int species_printed = FALSE;
            species_number = spidx + 1;
            species = &spec_data[spidx];

            /* Set dest_x for all ships to zero.
             * We will use this to prevent multiple listings of a ship. */
            for (int ship_index = 0; ship_index < species->num_ships; ship_index++) {
                ship_data_t *sd = ship_data[spidx] + ship_index;
                sd[ship_index].dest_x = 0;
            }

            /* Check all namplas for this species. */
            for (int namplaIndex = 0; namplaIndex < species->num_namplas; namplaIndex++) {
                nampla_data_t *nd = namp_data[spidx] + namplaIndex;
                if ((nd->status & POPULATED) == 0) {
                    continue;
                }
                int delta_x = x - nd->x;
                int delta_y = y - nd->y;
                int delta_z = z - nd->z;
                int distance_squared = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);
                if (distance_squared > radiusSquared) {
                    continue;
                }
                if (species_printed == FALSE) {
                    printf("  Species #%d, SP %s:\n", species_number, species->name);
                    species_printed = TRUE;
                }
                printf("    %2d %2d %2d #%d", nd->x, nd->y, nd->z, nd->pn);
                if ((nd->status & HOME_PLANET) != 0) {
                    printf("  Home planet");
                } else if ((nd->status & MINING_COLONY) != 0) {
                    printf("  Mining colony");
                } else if ((nd->status & RESORT_COLONY) != 0) {
                    printf("  Resort colony");
                } else {
                    printf("  Normal colony");
                }
                printf(" PL %s, EB = %d.%d, %d Yrds",
                       nd->name,
                       (nd->mi_base + nd->ma_base) / 10, (nd->mi_base + nd->ma_base) % 10,
                       nd->shipyards);
                for (int i = 0; i < MAX_ITEMS; i++) {
                    if (nd->item_quantity[i] > 0) {
                        printf(", %d %s", nd->item_quantity[i], item_abbr[i]);
                    }
                }
                if (nd->hidden != FALSE) {
                    printf(", HIDING!");
                }
                printf("\n");

                /* List ships at this colony. */
                for (int ship_index = 0; ship_index < species->num_ships; ship_index++) {
                    ship_data_t *sd = ship_data[spidx] + ship_index;
                    if (sd->dest_x != 0) {
                        /* Already listed. */
                        continue;
                    } else if (sd->x != nd->x || sd->y != nd->y || sd->z != nd->z ||
                               sd->pn != nd->pn) {
                        // not at this colony
                        continue;
                    }
                    printf("                 %s", ship_name(sd));
                    for (int i = 0; i < MAX_ITEMS; i++) {
                        if (sd->item_quantity[i] > 0) {
                            printf(", %d %s", sd->item_quantity[i], item_abbr[i]);
                        }
                    }
                    printf("\n");
                    sd->dest_x = 99;  /* Do not list this ship again. */
                }
            }

            for (int shipIndex = 0; shipIndex < species->num_ships; shipIndex++) {
                ship_data_t *sd = ship_data[spidx] + shipIndex;
                if (sd->pn == 99) {
                    // sometimes 99 means that the ship's slot is not used?
                    continue;
                } else if (sd->dest_x != 0) {
                    /* Already listed above. */
                    continue;
                }
                int delta_x = x - sd->x;
                int delta_y = y - sd->y;
                int delta_z = z - sd->z;
                int distance_squared = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);
                if (distance_squared > radiusSquared) {
                    continue;
                }
                if (!species_printed) {
                    printf("  Species #%d, SP %s:\n", species_number, species->name);
                    species_printed = TRUE;
                }
                printf("    %2d %2d %2d", sd->x, sd->y, sd->z);
                printf("     %s", ship_name(sd));
                for (int i = 0; i < MAX_ITEMS; i++) {
                    if (sd->item_quantity[i] > 0) {
                        printf(", %d %s", sd->item_quantity[i], item_abbr[i]);
                    }
                }
                printf("\n");
                sd->dest_x = 99;  /* Do not list this ship again. */
            }
        }
        return 0;
    }

    printf("Systems within %d parsecs of %d %d %d:\n", radius, x, y, z);
    scan_system_t **ss = calloc(num_stars, sizeof(scan_system_t *));
    if (ss == NULL) {
        perror("scan_system_t *");
        return 2;
    }
    for (int i = 0; i < num_stars; i++) {
        ss[i] = calloc(1, sizeof(scan_system_t));
        if (ss[i] == NULL) {
            perror("scan_system_t");
            return 2;
        }
        ss[i]->star = star_base + i;
        int delta_x = x - ss[i]->star->x;
        int delta_y = y - ss[i]->star->y;
        int delta_z = z - ss[i]->star->z;
        ss[i]->distance = sqrt((double) ((delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z)));
    }
    // bubbly and proud
    for (int i = 0; i < num_stars; i++) {
        for (int j = i + 1; j < num_stars; j++) {
            if (ss[i]->distance > ss[j]->distance) {
                scan_system_t *tmp = ss[i];
                ss[i] = ss[j];
                ss[j] = tmp;
            }
        }
    }
    for (int i = 0; i < num_stars; i++) {
        if (ss[i]->distance < (double) radius) {
            star_data_t *star = ss[i]->star;
            printf("System  x = %3d  y = %3d  z = %3d  %c%c%c %8.2f parsecs\n",
                   star->x, star->y, star->z,
                   type_char[star->type], color_char[star->color], size_char[star->size],
                   ss[i]->distance);
            if (star->worm_here) {
                printf("\t*** terminus of a natural wormhole.\n");
            }
            if (star->num_planets == 0) {
                printf("\t*** nova remnant, no planets.\n\n");
                continue;
            }
            printf("\t#  Dia  Grav  TempClass  PressClass  MiningDiff\n");
            printf("\t-----------------------------------------------\n");
            for (int pn = 0; pn < star->num_planets; pn++) {
                planet_data_t *planet = planet_base + star->planet_index + pn - 1;
                int orbit = pn + 1;
                printf("\t%d  %3d  %d.%02d  %9d  %10d  %7d.%02d\n",
                       orbit,
                       planet->diameter,
                       planet->gravity / 100, planet->gravity % 100,
                       planet->temperature_class,
                       planet->pressure_class,
                       planet->mining_difficulty / 100, planet->mining_difficulty % 100);
            }
        }
    }

    return 0;
}


