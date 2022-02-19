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
#include "enginevars.h"
#include "transactionio.h"
#include "planetvars.h"
#include "speciesvars.h"


int exportCommand(int argc, char *argv[]);

int exportToSExpr(int argc, char *argv[]);

int exportToJson(int argc, char *argv[]);

int locationCommand(int argc, char *argv[]);

int logRandomCommand(int argc, char *argv[]);

int reportCommand(int argc, char *argv[]);

int setCommand(int argc, char *argv[]);

int setPlanet(int argc, char *argv[]);

int setSpecies(int argc, char *argv[]);

int setSpeciesGovtType(int argc, char *argv[]);

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
            printf("         args:    (json | sexpr) galaxy | stars | planets | species | locations | transactions\n");
            printf("  cmd: logrnd     display a list of random values for testing the PRNG\n");
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
    printf("fh: export: %s: loading   galaxy file...\n", cmdName);
    get_galaxy_data();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.json'!\n");
                return 2;
            }
            galaxyDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.json'!\n");
                return 2;
            }
            locationDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_planet_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.json'!\n");
                return 2;
            }
            planetDataAsJson(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "species") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_species_data();
            printf("fh: export: %s: exporting %s files...\n", cmdName, argv[i]);
            for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
                int spNo = species_index + 1;
                if (data_in_memory[species_index]) {
                    struct species_data *sp = &spec_data[species_index];
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
                    namplaDataAsJson(spNo, namp_data[species_index], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.ships.json", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: json:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    shipDataAsJson(spNo, ship_data[species_index], sp->num_ships, fp);
                    fclose(fp);
                }
            }
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_star_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.json", "wb");
            if (fp == NULL) {
                perror("fh: export: json:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.json'!\n");
                return 2;
            }
            fclose(fp);
        } else if (strcmp(argv[i], "transactions") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_transaction_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
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
    printf("fh: export: %s: loading   galaxy file...\n", cmdName);
    get_galaxy_data();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("galaxy.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'galaxy.txt'!\n");
                return 2;
            }
            galaxyDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_location_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("locations.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'locations.txt'!\n");
                return 2;
            }
            locationDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_planet_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("planets.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
                return 2;
            }
            planetDataAsSExpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "species") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_species_data();
            printf("fh: export: %s: exporting %s files...\n", cmdName, argv[i]);
            for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
                int spNo = species_index + 1;
                if (data_in_memory[species_index]) {
                    struct species_data *sp = &spec_data[species_index];
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
                    namplaDataAsSExpr(spNo, namp_data[species_index], sp->num_namplas, fp);
                    fclose(fp);

                    sprintf(filename, "sp%02d.ships.txt", spNo);
                    fp = fopen(filename, "wb");
                    if (fp == NULL) {
                        perror("fh: export: sexpr:");
                        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                        return 2;
                    }
                    shipDataAsSExpr(spNo, ship_data[species_index], sp->num_ships, fp);
                    fclose(fp);
                }
            }
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_star_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
            FILE *fp = fopen("stars.txt", "wb");
            if (fp == NULL) {
                perror("fh: export: sexpr:");
                fprintf(stderr, "\n\tCannot create new version of file 'stars.txt'!\n");
                return 2;
            }
            starDataAsSexpr(fp);
            fclose(fp);
        } else if (strcmp(argv[i], "transactions") == 0) {
            printf("fh: export: %s: loading   %s file...\n", cmdName, argv[i]);
            get_transaction_data();
            printf("fh: export: %s: exporting %s file...\n", cmdName, argv[i]);
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
    printf("fh: %s: loading   galaxy   file...\n", cmdName);
    get_galaxy_data();
    printf("fh: %s: loading   planet   file...\n", cmdName);
    get_planet_data();
    printf("fh: %s: loading   species  file...\n", cmdName);
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
    printf("fh: %s: saving    planet   file...\n", cmdName);
    save_planet_data();
    printf("fh: %s: saving    location file...\n", cmdName);
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


int setCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
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
    const char *cmdName = argv[0];
    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "govt-type") == 0) {
            return setSpeciesGovtType(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: set: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
}


int setSpeciesGovtType(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    const char *name = 0;
    for (int i = 1; i < argc; i++) {
        if (name == 0) {
            name = argv[i];
            continue;
        }
        fprintf(stderr, "fh: set: %s: unknown option '%s'\n", cmdName, argv[i]);
        return 2;
    }
    if (name == 0 || !(0 < strlen(name) && strlen(name) < 32)) {
        fprintf(stderr, "fh: usage: set species %s _string_value_\n", cmdName);
        fprintf(stderr, "    where: _string_value_ is between 1 and 31 characters\n");
        return 2;
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