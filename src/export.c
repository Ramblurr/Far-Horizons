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

    return 0;
}


int exportToJson(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: export: %s: loading   galaxy  data...\n", cmdName);
    get_galaxy_data();
    printf("fh: export: %s: loading   star    data...\n", cmdName);
    get_star_data();
    printf("fh: export: %s: loading   planet  data...\n", cmdName);
    get_planet_data();
    printf("fh: export: %s: loading   species data...\n", cmdName);
    get_species_data();

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
            fprintf(stderr, "usage: export json globals\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "galaxy") == 0 && val == NULL) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, opt);
            FILE *fp = fopen("galaxy.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.json'!\n");
                return 2;
            }
            galaxyDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(opt, "globals") == 0 && val == NULL) {
            FILE *fp = fopen("export.json", "w");
            if (fp == NULL) {
                perror("fh: export: globals");
                fprintf(stderr, "\n\tCannot create new version of file 'export.json'!\n");
                return 2;
            }
            exportData(fp);
            fclose(fp);
        } else if (strcmp(opt, "locations") == 0 && val == NULL) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, opt);
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
        } else if (strcmp(opt, "planets") == 0 && val == NULL) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, opt);
            FILE *fp = fopen("planets.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.json'!\n");
                return 2;
            }
            planetDataAsJson(num_planets, planet_base, fp);
            fclose(fp);
        } else if (strcmp(opt, "species") == 0 && val == NULL) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, opt);
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
                    speciesDataAsJson(sp, fp);
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
        } else if (strcmp(opt, "stars") == 0 && val == NULL) {
            printf("fh: export: %s: exporting %s data...\n", cmdName, opt);
            FILE *fp = fopen("stars.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.json'!\n");
                return 2;
            }
            fclose(fp);
        } else if (strcmp(opt, "transactions") == 0 && val == NULL) {
            printf("fh: export: %s: loading   %s data...\n", cmdName, opt);
            get_transaction_data();
            printf("fh: export: %s: exporting %s data...\n", cmdName, opt);
            FILE *fp = fopen("interspecies.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
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

//    if (globalMode) {
//        printf("fh: export: exporting global data...\n");
//
////        printf("export: json --global\n");
////        FILE *fp = fopen("game.json", "r");
////        if (fp == NULL) {
////            perror("exportToJson:");
////            exit(2);
////        }
////        json_value_t *j = json_unmarshal(fp);
////        if (j == NULL) {
////            fprintf(stderr, "exportToJson: j is NULL\n");
////            exit(2);
////        }
////        fclose(fp);
//
//        FILE *fp = fopen("export.json", "w");
////        json_marshal(j, fp);
////        fclose(fp);
////
////        FILE *fp = fopen("galaxy.json", "wb");
//        if (fp == NULL) {
//            perror("fh: export: global:");
//            fprintf(stderr, "\n\tCannot create new version of file 'export.json'!\n");
//            return 2;
//        }
//        galaxyDataAsJson(fp);
//        fclose(fp);
//    }

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
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
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

                    sprintf(filename, "sp%02d.species.txt", spNo);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    speciesDataAsSExpr(sp, fp);
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
            printf("fh: export: %s: exporting %s data...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.txt'!\n");
                return 2;
            }
            starDataAsSExpr(star_base, num_stars, fp);
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
