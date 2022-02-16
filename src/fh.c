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
#include "galaxy.h"
#include "galaxyio.h"
#include "stario.h"
#include "planetio.h"
#include "locationio.h"
#include "speciesio.h"
#include "namplavars.h"
#include "namplaio.h"
#include "shipio.h"
#include "shipvars.h"


int dumpCommand(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "?") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("usage: fh [option...] command [argument...]\n");
            printf("  opt: --help  show this helpful text\n");
            printf("  cmd: dump    convert binary .dat to s-expression\n");
            printf("       args:   galaxy | stars | planets | species | locations | transactions\n");
            return 0;
        } else if (strcmp(argv[i], "dump") == 0) {
            return dumpCommand(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }
    printf("fh: try `fh --help` for instructions\n");
    return 2;
}


int dumpCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: %s: loading galaxy file...\n", cmdName);
    get_galaxy_data();

    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: %s: dumping %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.txt", "wb");
            if (fp == NULL) {
                perror("fh: dump:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.txt'!\n");
                return 2;
            }
            galaxyDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: %s: loading %s file...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: %s: dumping %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.txt", "wb");
            if (fp == NULL) {
                perror("fh: dump:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.txt'!\n");
                return 2;
            }
            locationDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: %s: loading %s file...\n", cmdName, argv[i]);
            get_planet_data();
            printf("fh: %s: dumping %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.txt", "wb");
            if (fp == NULL) {
                perror("fh: dump:");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
                return 2;
            }
            planetDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "species") == 0) {
            printf("fh: %s: loading %s files...\n", cmdName, argv[i]);
            get_species_data();
            printf("fh: %s: dumping %s files...\n", cmdName, argv[i]);
            for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
                if (data_in_memory[species_index]) {
                    struct species_data *sp = &spec_data[species_index];
                    char filename[32];

                    sprintf(filename, "sp%02d.species.txt", species_index + 1);
                    FILE *fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: dump:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    speciesDataAsSExpr(sp, species_index + 1, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.namplas.txt", species_index + 1);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: dump:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    namplaDataAsSExpr(namp_data[species_index], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.ships.txt", species_index + 1);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: dump:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    shipDataAsSExpr(ship_data[species_index], sp->num_ships, fp);
                    fclose(fp);
                }
            }
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("fh: %s: loading %s file...\n", cmdName, argv[i]);
            get_star_data();
            printf("fh: %s: dumping %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.txt", "wb");
            if (fp == NULL) {
                perror("fh: dump:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.txt'!\n");
                return 2;
            }
            starDataAsSexpr(fp);
            fclose(fp);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }
    return 0;
}
