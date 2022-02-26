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

#include <string.h>
#include <stdio.h>
#include "export.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "locationio.h"
#include "planetio.h"
#include "speciesio.h"
#include "namplaio.h"
#include "namplavars.h"
#include "shipio.h"
#include "shipvars.h"
#include "stario.h"
#include "transactionio.h"


int exportToSExpr(int argc, char *argv[]);

int exportToJson(int argc, char *argv[]);


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
            planetDataAsJson(num_planets, planet_base, fp);
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
            planetDataAsSExpr(num_planets, planet_base, fp);
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
