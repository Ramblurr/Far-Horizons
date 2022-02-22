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
#include "commandvars.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "locationio.h"
#include "logvars.h"
#include "planetio.h"
#include "planetvars.h"
#include "namplaio.h"
#include "namplavars.h"
#include "scan.h"
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

int setCommand(int argc, char *argv[]);

int setPlanet(int argc, char *argv[]);

int setSpecies(int argc, char *argv[]);

int setStar(int argc, char *argv[]);

int statsCommand(int argc, char *argv[]);

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
            printf("  cmd: stats      display statistics\n");
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
        } else if (strcmp(argv[i], "stats") == 0) {
            return statsCommand(argc - i, argv + i);
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


int statsCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];

    int i, j, m, n, nampla_index, ship_index, num_ships, avg_tech_level;
    int all_tech_level[6], n_species, n_warships, n_starbases;
    int n_transports, avg_pop_pl, n_pop_pl, ls_needed, num_yards;
    int production_penalty, fleet_percent_cost, num_pop_planets;
    int min_starbases, max_starbases, min_warships, max_warships;
    int min_transports, max_transports, min_tech_level[6];
    int max_tech_level[6], min_pop_pl, max_pop_pl, ntr, nba, nwa;
    int n_yards, min_yards, max_yards, avg_yards;

    long total_production, raw_material_units, production_capacity;
    long total_tonnage, total_offensive_power, total_defensive_power;
    long avg_production, all_production, avg_warship_tons;
    long all_warship_tons, avg_starbase_tons, all_starbase_tons;
    long avg_transport_tons, all_transport_tons, n1, n2, n3;
    long min_production, max_production;

    short tons;


    /* Check for valid command line. */
    if (argc != 1) {
        fprintf(stderr, "\n\tUsage: Stats\n\n");
        exit(0);
    }

    printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    printf("fh: %s: loading   planet   data...\n", cmdName);
    get_planet_data();
    printf("fh: %s: loading   species  data...\n", cmdName);
    get_species_data();

    /* Initialize data. */
    n_species = 0;
    all_production = 0;
    min_production = 1000000000;
    max_production = 0;
    all_warship_tons = 0;
    all_starbase_tons = 0;
    all_transport_tons = 0;
    n_warships = 0;
    min_warships = 32000;
    max_warships = 0;
    n_starbases = 0;
    min_starbases = 32000;
    max_starbases = 0;
    n_transports = 0;
    min_transports = 32000;
    max_transports = 0;
    n_pop_pl = 0;
    min_pop_pl = 32000;
    max_pop_pl = 0;
    n_yards = 0;
    min_yards = 32000;
    max_yards = 0;

    for (int i = 0; i < 6; i++) {
        all_tech_level[i] = 0;
        min_tech_level[i] = 32000;
        max_tech_level[i] = 0;
    }

    /* Print header. */
    printf("SP Species               Tech Levels        Total  Num Num  Num  Offen.  Defen.\n");
    printf(" # Name             MI  MA  ML  GV  LS  BI  Prod.  Pls Shps Yrds  Power   Power\n");
    printf("-------------------------------------------------------------------------------\n");

    /* Main loop. For each species, take appropriate action. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        if (!data_in_memory[species_number - 1]) {
            continue;
        }

        ++n_species;

        species = &spec_data[species_number - 1];
        nampla_base = namp_data[species_number - 1];
        ship_base = ship_data[species_number - 1];

        /* Get fleet maintenance cost. */
        fleet_percent_cost = species->fleet_percent_cost;

        if (fleet_percent_cost > 10000) {
            fleet_percent_cost = 10000;
        }

        /* Print species data. */
        printf("%2d", species_number);
        printf(" %-15.15s", species->name);

        for (int i = 0; i < 6; i++) {
            printf("%4d", species->tech_level[i]);
            all_tech_level[i] += (int) species->tech_level[i];
            if (species->tech_level[i] < min_tech_level[i]) {
                min_tech_level[i] = species->tech_level[i];
            }
            if (species->tech_level[i] > max_tech_level[i]) {
                max_tech_level[i] = species->tech_level[i];
            }
        }

        /* Get stats for namplas. */
        total_production = 0;
        total_defensive_power = 0;
        num_yards = 0;
        num_pop_planets = 0;
        home_planet = planet_base + (int) nampla_base->planet_index;
        for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
            nampla = nampla_base + nampla_index;
            if (nampla->pn == 99) {
                continue;
            }

            num_yards += nampla->shipyards;
            n_yards += nampla->shipyards;

            planet = planet_base + (int) nampla->planet_index;

            raw_material_units =
                    (10L * (long) species->tech_level[MI] * (long) nampla->mi_base) / (long) planet->mining_difficulty;

            production_capacity = ((long) species->tech_level[MA] * (long) nampla->ma_base) / 10L;

            ls_needed = life_support_needed(species, home_planet, planet);

            if (ls_needed == 0) {
                production_penalty = 0;
            } else {
                production_penalty = (100 * ls_needed) / species->tech_level[LS];
            }

            raw_material_units -= (production_penalty * raw_material_units) / 100;

            raw_material_units = (((long) planet->econ_efficiency * raw_material_units) + 50) / 100;

            production_capacity -= (production_penalty * production_capacity) / 100;

            production_capacity = (((long) planet->econ_efficiency * production_capacity) + 50) / 100;

            if (nampla->status & MINING_COLONY) {
                n1 = (2 * raw_material_units) / 3;
            } else if (nampla->status & RESORT_COLONY) {
                n1 = (2 * production_capacity) / 3;
            } else {
                n1 = (production_capacity > raw_material_units) ? raw_material_units : production_capacity;
            }

            n2 = ((fleet_percent_cost * n1) + 5000) / 10000;
            n3 = n1 - n2;
            total_production += n3;

            tons = nampla->item_quantity[PD] / 200;
            if (tons < 1 && nampla->item_quantity[PD] > 0) {
                tons = 1;
            }
            total_defensive_power += power(tons);

            if (nampla->status & POPULATED) {
                ++n_pop_pl;
                ++num_pop_planets;
            }
        }

        printf("%7ld%4d", total_production, num_pop_planets);

        if (total_production < min_production) {
            min_production = total_production;
        }
        if (total_production > max_production) {
            max_production = total_production;
        }

        if (num_pop_planets < min_pop_pl) {
            min_pop_pl = num_pop_planets;
        }
        if (num_pop_planets > max_pop_pl) {
            max_pop_pl = num_pop_planets;
        }

        if (num_yards < min_yards) {
            min_yards = num_yards;
        }
        if (num_yards > max_yards) {
            max_yards = num_yards;
        }

        all_production += total_production;

        /* Get stats for ships. */
        num_ships = 0;
        ntr = 0;
        nba = 0;
        nwa = 0;
        total_tonnage = 0;
        total_offensive_power = 0;
        for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
            ship = ship_base + ship_index;
            if (ship->pn == 99) {
                continue;
            } else if (ship->status == UNDER_CONSTRUCTION) {
                continue;
            }

            ++num_ships;
            total_tonnage += (long) ship->tonnage;

            if (ship->type == STARBASE) {
                total_defensive_power += power(ship->tonnage);
                all_starbase_tons += (long) ship->tonnage;
                ++n_starbases;
                ++nba;
            } else if (ship->class == TR) {
                all_transport_tons += (long) ship->tonnage;
                ++n_transports;
                ++ntr;
            } else {
                if (ship->type == SUB_LIGHT) {
                    total_defensive_power += power(ship->tonnage);
                } else {
                    total_offensive_power += power(ship->tonnage);
                }
                all_warship_tons += (long) ship->tonnage;
                ++n_warships;
                ++nwa;
            }
        }

        if (nwa < min_warships) {
            min_warships = nwa;
        }
        if (nwa > max_warships) {
            max_warships = nwa;
        }

        if (nba < min_starbases) {
            min_starbases = nba;
        }
        if (nba > max_starbases) {
            max_starbases = nba;
        }

        if (ntr < min_transports) {
            min_transports = ntr;
        }
        if (ntr > max_transports) {
            max_transports = ntr;
        }

        total_offensive_power +=
                ((long) species->tech_level[ML] * total_offensive_power) / 50;

        total_defensive_power +=
                ((long) species->tech_level[ML] * total_defensive_power) / 50;

        if (species->tech_level[ML] == 0) {
            total_defensive_power = 0;
            total_offensive_power = 0;
        }

        total_offensive_power /= 10;
        total_defensive_power /= 10;

        printf("%5d", num_ships);
        printf("%5d", num_yards);
        printf("%8ld%8ld\n", total_offensive_power, total_defensive_power);
    }

    m = n_species / 2;
    printf("\n");
    for (int i = 0; i < 6; i++) {
        avg_tech_level = (all_tech_level[i] + m) / n_species;
        printf("Average %s tech level = %d (min = %d, max = %d)\n",
               tech_name[i], avg_tech_level, min_tech_level[i], max_tech_level[i]);
    }

    i = ((10 * n_warships) + m) / n_species;
    printf("\nAverage number of warships per species = %d.%d (min = %d, max = %d)\n",
           i / 10, i % 10, min_warships, max_warships);

    if (n_warships == 0) {
        n_warships = 1;
    }
    avg_warship_tons = (10000L * all_warship_tons) / n_warships;
    avg_warship_tons = 1000L * ((avg_warship_tons + 500L) / 1000L);
    printf("Average warship size = %s tons\n", commas(avg_warship_tons));

    avg_warship_tons = (10000L * all_warship_tons) / n_species;
    avg_warship_tons = 1000L * ((avg_warship_tons + 500L) / 1000L);
    printf("Average total warship tonnage per species = %s tons\n", commas(avg_warship_tons));

    i = ((10 * n_starbases) + m) / n_species;
    printf("\nAverage number of starbases per species = %d.%d (min = %d, max = %d)\n",
           i / 10, i % 10, min_starbases, max_starbases);

    if (n_starbases == 0) {
        n_starbases = 1;
    }
    avg_starbase_tons = (10000L * all_starbase_tons) / n_starbases;
    avg_starbase_tons = 1000L * ((avg_starbase_tons + 500L) / 1000L);
    printf("Average starbase size = %s tons\n", commas(avg_starbase_tons));

    avg_starbase_tons = (10000L * all_starbase_tons) / n_species;
    avg_starbase_tons = 1000L * ((avg_starbase_tons + 500L) / 1000L);
    printf("Average total starbase tonnage per species = %s tons\n", commas(avg_starbase_tons));

    i = ((10 * n_transports) + m) / n_species;
    printf("\nAverage number of transports per species = %d.%d (min = %d, max = %d)\n",
           i / 10, i % 10, min_transports, max_transports);

    if (n_transports == 0) {
        n_transports = 1;
    }
    avg_transport_tons = (10000L * all_transport_tons) / n_transports;
    avg_transport_tons = 1000L * ((avg_transport_tons + 500L) / 1000L);
    printf("Average transport size = %s tons\n", commas(avg_transport_tons));

    avg_transport_tons = (10000L * all_transport_tons) / n_species;
    avg_transport_tons = 1000L * ((avg_transport_tons + 500L) / 1000L);
    printf("Average total transport tonnage per species = %s tons\n", commas(avg_transport_tons));

    avg_yards = ((10 * n_yards) + m) / n_species;
    printf("\nAverage number of shipyards per species = %d.%d (min = %d, max = %d)\n",
           avg_yards / 10, avg_yards % 10, min_yards, max_yards);

    avg_pop_pl = ((10 * n_pop_pl) + m) / n_species;
    printf("\nAverage number of populated planets per species = %d.%d (min = %d, max = %d)\n",
           avg_pop_pl / 10, avg_pop_pl % 10, min_pop_pl, max_pop_pl);

    avg_production = (all_production + m) / n_species;
    printf("Average total production per species = %ld (min = %ld, max = %ld)\n",
           avg_production, min_production, max_production);

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