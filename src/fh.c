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
#include <string.h>
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "locationio.h"
#include "logvars.h"
#include "planetio.h"
#include "planetvars.h"
#include "namplaio.h"
#include "namplavars.h"
#include "shipio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "stario.h"
#include "transactionio.h"


int exportCommand(int argc, char *argv[]);

int exportToSExpr(int argc, char *argv[]);

int exportToJson(int argc, char *argv[]);

int locationCommand(int argc, char *argv[]);

int logRandomCommand(int argc, char *argv[]);

int reportCommand(int argc, char *argv[]);

int scanCommand(int argc, char *argv[]);

int scanNearCommand(int argc, char *argv[]);

int setCommand(int argc, char *argv[]);

int setPlanet(int argc, char *argv[]);

int setSpecies(int argc, char *argv[]);

int setStar(int argc, char *argv[]);

int turnCommand(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    test_mode = FALSE;
    verbose_mode = FALSE;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "?") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("usage: fh [option...] command [argument...]\n");
            printf("  opt: --help     show this helpful text\n");
            printf("  opt: -t         enable test mode\n");
            printf("  opt: -v         enable verbose mode\n");
            printf("  cmd: turn       display the current turn number\n");
            printf("  cmd: locations  create locations data file and update economic efficiency\n");
            printf("                  in planets data file\n");
            printf("  cmd: report     create end of turn reports\n");
            printf("  cmd: export     convert binary .dat to json or s-expression\n");
            printf("           args:  (json | sexpr) galaxy | stars | planets | species | locations | transactions\n");
            printf("  cmd: logrnd     display a list of random values for testing the PRNG\n");
            printf("  cmd: scan       display a species-specific scan for a location\n");
            printf("           args:  _spNo_ _x_ _y_ _z_\n");
            printf("  cmd: scan-near  display ships and colonies near a location\n");
            printf("           args:  _x_ _y_ _z_ _radiusInParsecs_\n");
            printf("  cmd: set        update values for planet, species, or star\n");
            printf("         args:    (planet | species | star ) values\n");
            return 0;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else if (strcmp(argv[i], "export") == 0) {
            return exportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "locations") == 0) {
            return locationCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "logrnd") == 0) {
            return logRandomCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "report") == 0) {
            return reportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan") == 0) {
            return scanCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan-near") == 0) {
            return scanNearCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "set") == 0) {
            return setCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "turn") == 0) {
            return turnCommand(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }
    printf("fh: try `fh --help` for instructions\n");
    return 2;
}


int exportCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    for (int i = 1; i < argc; i++) {
        // fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "json") == 0) {
            return exportToJson(argc - i, argv + i);
        } else if (strcmp(argv[i], "sexpr") == 0) {
            return exportToSExpr(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
}


int exportToJson(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: export: %s: loading   galaxy data...\n", cmdName);
    get_galaxy_data();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.json'!\n");
                return 2;
            }
            galaxyDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.json'!\n");
                return 2;
            }
            locationDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_planet_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.json'!\n");
                return 2;
            }
            planetDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "species") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_species_data();
            printf("fh: export: %s: exporting %s files...\n", cmdName, argv[i]);
            for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
                int spNo = spidx + 1;
                if (data_in_memory[spidx]) {
                    struct species_data *sp = &spec_data[spidx];
                    char filename[32];

                    sprintf(filename, "sp%02d.species.json", spNo);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: json:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    speciesDataAsJson(spNo, sp, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.namplas.json", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: json:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    namplaDataAsJson(spNo, namp_data[spidx], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.ships.json", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: json:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    shipDataAsJson(spNo, ship_data[spidx], sp->num_ships, fp);
                    fclose(fp);
                }
            }
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_star_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.json'!\n");
                return 2;
            }
            fclose(fp);
        } else if (strcmp(argv[i], "transactions") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_transaction_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("interspecies.json", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'interspecies.json'!\n");
                return 2;
            }
            transactionDataAsJson(fp);
            fclose(fp);
        } else {
            fprintf(stderr, "fh: export: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
}


int exportToSExpr(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: export: %s: loading   galaxy data...\n", cmdName);
    get_galaxy_data();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.txt'!\n");
                return 2;
            }
            galaxyDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.txt'!\n");
                return 2;
            }
            locationDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_planet_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
                return 2;
            }
            planetDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "species") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_species_data();
            printf("fh: export: %s: exporting %s files...\n", cmdName, argv[i]);
            for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
                int spNo = spidx + 1;
                if (data_in_memory[spidx]) {
                    struct species_data *sp = &spec_data[spidx];
                    char filename[32];

                    sprintf(filename, "sp%02d.species.txt", spNo);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    speciesDataAsSExpr(spNo, sp, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.namplas.txt", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    namplaDataAsSExpr(spNo, namp_data[spidx], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.ships.txt", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    shipDataAsSExpr(spNo, ship_data[spidx], sp->num_ships, fp);
                    fclose(fp);
                }
            }
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_star_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.txt'!\n");
                return 2;
            }
            starDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "transactions") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_transaction_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("interspecies.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'interspecies.txt'!\n");
                return 2;
            }
            transactionDataAsSExpr(fp);
            fclose(fp);
        } else {
            fprintf(stderr, "fh: export: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
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
    long *total_econ_base = (long *) calloc(num_planets, sizeof(long));
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
    save_planet_data();
    printf("fh: %s: saving    location data...\n", cmdName);
    save_location_data();

    // clean up
    free_species_data();
    free(planet_base);

    return 0;
}


// logRandomCommand generates random numbers using the historical default seed value.
int logRandomCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    // delete any seed from the environment so that we're sure our value is used
    putenv("FH_SEED");
    // use the historical default seed value
    last_random = defaultHistoricalSeedValue;
    // then print out a nice set of random values
    for (int i = 0; i < 1000000; i++) {
        int r = rnd(1024 * 1024);
        if (i < 10) {
            printf("%9d %9d\n", i, r);
        } else if (1000 < i && i < 1010) {
            printf("%9d %9d\n", i, r);
        } else if ((i % 85713) == 0) {
            printf("%9d %9d\n", i, r);
        }
    }
    return 0;
}


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


int scanNearCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    if (argc != 5) {
        fprintf(stderr, "usage: fh scan-near x y z radiusInParsecs\n");
        return 2;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int z = atoi(argv[3]);
    int radius = atoi(argv[4]);
    int radiusSquared = radius * radius;

    // external globals?
    ignore_field_distorters = TRUE;
    log_file = stdout;

    printf("fh: %s: loading   galaxy   data...\n", cmdName);
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
    printf("fh: %s: loading   star     data...\n", cmdName);
    get_star_data();
    printf("fh: %s: loading   planet   data...\n", cmdName);
    get_planet_data();
    printf("fh: %s: loading   species  data...\n", cmdName);
    get_species_data();

    /* Display scan. */
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


int setCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "planet") == 0) {
            return setPlanet(argc - i, argv + i);
        } else if (strcmp(argv[i], "species") == 0) {
            return setSpecies(argc - i, argv + i);
        } else if (strcmp(argv[i], "star") == 0) {
            return setStar(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
}


int setPlanet(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    return 0;
}


int setSpecies(int argc, char *argv[]) {
    species_data_t *sp = NULL;
    int spno = 0;
    int spidx = -1;

    printf("fh: set: loading   species  data...\n");
    get_species_data();

    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: set species: argc %2d argv '%s'\n", i, argv[i]);
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
            fprintf(stderr, "fh: usage: set species spNo [field value]\n");
            fprintf(stderr, "    where: spNo is a valid species number (no leading zeroes)\n");
            fprintf(stderr, "    where: field is govt-type\n");
            fprintf(stderr, "      and: value is between 1 and 31 characters\n");
            return 2;
        } else if (spno == 0) {
            spno = atoi(argv[i]);
            if (spno < 1 || spno > galaxy.num_species) {
                fprintf(stderr, "error: invalid species number\n");
                return 2;
            } else if (!data_in_memory[spno]) {
                fprintf(stderr, "error: unable to load species %d into memory\n", spno);
                return 2;
            }
            printf("fh: set species: species number is %3d\n", spno);
            spidx = spno - 1;
            sp = spec_data + spidx;
        } else if (strcmp(argv[i], "bi") == 0 || strcmp(argv[i], "gv") == 0 || strcmp(argv[i], "ls") == 0 ||
                   strcmp(argv[i], "ma") == 0 || strcmp(argv[i], "mi") == 0 || strcmp(argv[i], "ml") == 0) {
            const char *tech = argv[i];
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing tech level value\n");
                return 2;
            }
            i++;
            int value = atoi(argv[i]);
            if (value < 0) {
                fprintf(stderr, "error: invalid tech level value\n");
                return 2;
            }
            int code;
            if (strcmp(tech, "bi") == 0) {
                code = BI;
            } else if (strcmp(tech, "gv") == 0) {
                code = GV;
            } else if (strcmp(tech, "ls") == 0) {
                code = LS;
            } else if (strcmp(tech, "ma") == 0) {
                code = MA;
            } else if (strcmp(tech, "mi") == 0) {
                code = MI;
            } else {
                code = ML;
            }
            printf("fh: set species: %s from %4d to %4d\n", tech, sp->tech_level[code], value);
            sp->tech_level[code] = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(argv[i], "eu") == 0) {
            const char *tech = argv[i];
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing economic units value\n");
                return 2;
            }
            i++;
            int value = atoi(argv[i]);
            if (value < 0) {
                fprintf(stderr, "error: invalid economic units value\n");
                return 2;
            }
            printf("fh: set species: %s from %4d to %4d\n", tech, sp->econ_units, value);
            sp->econ_units = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(argv[i], "govt-type") == 0) {
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing government type value\n");
                return 2;
            }
            i++;
            const char *value = argv[i];
            if (!(strlen(value) < 32)) {
                fprintf(stderr, "error: invalid government type\n");
                return 2;
            }
            printf("fh: set species: govt-type from \"%s\" to \"%s\"\n", sp->govt_type, value);
            memset(sp->govt_type, 0, 32);
            strcpy(sp->govt_type, value);
            data_modified[spidx] = TRUE;
        } else if (strcmp(argv[i], "hp") == 0) {
            const char *tech = argv[i];
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing hp economic base value\n");
                return 2;
            }
            i++;
            int value = atoi(argv[i]);
            if (value < 0) {
                fprintf(stderr, "error: invalid hp economic base value\n");
                return 2;
            }
            printf("fh: set species: %s from %4d to %4d\n", tech, sp->hp_original_base, value);
            sp->hp_original_base = value;
            data_modified[spidx] = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }
    if (sp == NULL || data_modified[spidx] == FALSE) {
        printf("fh: set species: no changes to save\n");
    } else {
        printf("fh: set: saving    species  data...\n");
        save_species_data();
    }
    return 0;
}


int setStar(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    return 0;
}


// turnCommand displays the current turn number
int turnCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    // check for valid command line
    if (argc != 1) {
        fprintf(stderr, "fh: %s: invalid option '%s'\n", cmdName, argv[1]);
        return 2;
    }

    // load the galaxy data and then print the current turn number
    get_galaxy_data();
    printf("%d\n", galaxy.turn_number);

    return 0;
}