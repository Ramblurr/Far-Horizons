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
#include "export.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "locationio.h"
#include "namplaio.h"
#include "namplavars.h"
#include "planetio.h"
#include "shipio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "stario.h"
#include "transactionio.h"
#include "json.h"
#include "data.h"


int exportToSExpr(int argc, char *argv[]);

int exportToJson(int argc, char *argv[]);


int exportCommand(int argc, char *argv[]) {
    // const char *cmdName = argv[0];
    // printf("fh: export: %s: loading   galaxy  data...\n", cmdName);
    // get_galaxy_data();
    // printf("fh: export: %s: loading   star    data...\n", cmdName);
    // get_star_data();
    // printf("fh: export: %s: loading   planet  data...\n", cmdName);
    // get_planet_data();
    // printf("fh: export: %s: loading   species data...\n", cmdName);
    // get_species_data();

    for (int i = 1; i < argc; i++) {
        // fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
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
            fprintf(stderr, "usage: export (json | sexpr) options...\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "json") == 0 && val == NULL) {
            return exportToJson(argc - i, argv + i);
        } else if (strcmp(opt, "sexpr") == 0 && val == NULL) {
            return exportToSExpr(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: export: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return 2;
}


int exportToJson(int argc, char *argv[]) {
    const char *cmdName = argv[0];

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
            fprintf(stderr, "usage: export json\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else {
            fprintf(stderr, "fh: export: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }

    // load game data
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    // build location data
    do_locations();

    // transaction data is kind of iffy
    get_transaction_data();

    printf("fh: export: %s: exporting galaxy       data...\n", cmdName);
    cJSON *root = galaxyDataToJson(&galaxy);
    if (root == 0) {
        fprintf(stderr, "json: there was an error converting galaxy data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "galaxy", "galaxy.json");
    cJSON_Delete(root);

    printf("fh: export: %s: exporting star         data...\n", cmdName);
    root = starsDataToJson(star_base, num_stars);
    if (root == 0) {
        fprintf(stderr, "json: there was an error converting star data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "stars", "stars.json");
    cJSON_Delete(root);

    printf("fh: export: %s: exporting planet       data...\n", cmdName);
    root = planetsDataToJson(planet_base, num_planets);
    if (root == 0) {
        fprintf(stderr, "json: there was an error converting planet data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "planets", "planets.json");
    cJSON_Delete(root);

    printf("fh: export: %s: exporting species      data...\n", cmdName);
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        int spNo = spidx + 1;
        if (!data_in_memory[spidx]) {
            continue;
        }
        struct species_data *sp = &spec_data[spidx];
        if (spNo != sp->id) {
            fprintf(stderr, "internal error: spNo %d != sp->id %d\n", spNo, sp->id);
            exit(2);
        }

        // convert species data to json
        cJSON *species = speciesToJson(sp);
        if (species == 0) {
            fprintf(stderr, "json: there was an error converting species %02d data to json\n", spNo);
            exit(2);
        }
        cJSON *namplas = namedPlanetsToJson(namp_data[spidx], sp->num_namplas);
        if (namplas == 0) {
            fprintf(stderr, "json: there was an error converting species %02d named planets data to json\n",
                    sp->id);
            exit(2);
        }
        cJSON *ships = shipsToJson(ship_data[spidx], sp->num_ships);
        if (ships == 0) {
            fprintf(stderr, "json: there was an error converting species %02d ships data to json\n", sp->id);
            exit(2);
        }

        // save the json data to files
        char filename[128];
        sprintf(filename, "species.%03d.json", spNo);
        jsonWriteFile(species, "species", filename);
        sprintf(filename, "species.%03d.planets.json", spNo);
        jsonWriteFile(namplas, "species: named planets", filename);
        sprintf(filename, "species.%03d.ships.json", spNo);
        jsonWriteFile(ships, "species: ships", filename);
    }

    printf("fh: export: %s: exporting locations    data...\n", cmdName);
    root = locationsToJson(loc, num_locs);
    if (root == 0) {
        fprintf(stderr, "json: there was an error converting locations data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "locations", "locations.json");
    cJSON_Delete(root);

    printf("fh: export: %s: exporting transactions data...\n", cmdName);
    root = transactionsToJson(transaction, num_transactions);
    if (root == 0) {
        fprintf(stderr, "json: there was an error converting transactions data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "transactions", "transactions.json");
    cJSON_Delete(root);

    return 0;
}


int exportToSExpr(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: export: %s: loading   galaxy data...\n", cmdName);
    get_galaxy_data();
    printf("fh: export: %s: loading   star   data...\n", cmdName);
    get_star_data();
    printf("fh: export: %s: loading   planet data...\n", cmdName);
    get_planet_data();

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
            fprintf(stderr, "usage: export (json | sexpr) options...\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.sexpr", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.sexpr'!\n");
                return 2;
            }
            galaxyDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.sexpr", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.sexpr'!\n");
                return 2;
            }
            locationDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.sexpr", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.sexpr'!\n");
                return 2;
            }
            planetDataAsSExpr(planet_base, num_planets, fp);
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

                    sprintf(filename, "species.%02d.sexpr", spNo);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    speciesDataAsSExpr(sp, fp);
                    fclose(fp);

                    sprintf(filename, "species.%02d.planets.sexpr", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    namplaDataAsSExpr(spNo, namp_data[spidx], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "species.%02d.ships.sexpr", spNo);
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
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.sexpr", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.sexpr'!\n");
                return 2;
            }
            starDataAsSExpr(star_base, num_stars, fp);
            fclose(fp);
        } else if (strcmp(argv[i], "transactions") == 0) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, argv[i]);
            get_transaction_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("transactions.sexpr", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'transactions.sexpr'!\n");
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
