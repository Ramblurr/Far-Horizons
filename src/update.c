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
#include "species.h"
#include "speciesio.h"
#include "update.h"
#include "ship.h"
#include "shipvars.h"


int updatePlanet(int argc, char *argv[]);

int updateShip(int argc, char *argv[]);

int updateSpecies(int argc, char *argv[]);

int updateStar(int argc, char *argv[]);


int updateCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    printf("fh: %s: loading   galaxy   data...\n", cmdName);
    get_galaxy_data();
    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "planet") == 0) {
            return updatePlanet(argc - i, argv + i);
        } else if (strcmp(argv[i], "ship") == 0) {
            return updateShip(argc - i, argv + i);
        } else if (strcmp(argv[i], "species") == 0) {
            return updateSpecies(argc - i, argv + i);
        } else if (strcmp(argv[i], "star") == 0) {
            return updateStar(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, argv[i]);
            return 2;
        }
    }
    return 0;
}


int updatePlanet(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    return 0;
}


int updateShip(int argc, char *argv[]) {
    species_data_t *sp = NULL;
    int spno = 0;
    int spidx = -1;
    ship_data_t *ship = NULL;
    const char *shipName = NULL;

    printf("fh: update: loading   species  data...\n");
    get_species_data();

    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: update ship: argc %2d argv '%s'\n", i, argv[i]);
        const char *opt = argv[i];
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr, "fh: usage: update ship spNo shipName [field value]\n");
            fprintf(stderr, "    where: spNo is a valid species number (no leading zeroes)\n");
            fprintf(stderr, "    where: shipName is a valid ship name (case sensitive, no type)\n");
            fprintf(stderr, "    where: field is age\n");
            fprintf(stderr, "      and: value is an integer between 1 and 50\n");
            return 2;
        } else if (spno == 0) {
            spno = atoi(opt);
            spidx = spno - 1;
            if (!(1 <= spno && spno <= galaxy.num_species)) {
                fprintf(stderr, "error: invalid species number '%s'\n", opt);
                return 2;
            } else if (data_in_memory[spidx] == FALSE) {
                fprintf(stderr, "error: species %d is not loaded\n", spno);
                return 2;
            }
            printf("fh: update ship: species number is %3d\n", spno);
            sp = spec_data + spidx;
            ship_base = ship_data[spidx];
        } else if (ship == NULL) {
            for (int j = 0; j < sp->num_ships; j++) {
                if (strcmp(ship_base[j].name, opt) == 0) {
                    ship = ship_base + j;
                    break;
                }
            }
            if (ship == NULL) {
                fprintf(stderr, "error: species %d has no ship named '%s'\n", opt);
                return 2;
            }
        } else if (strcmp(opt, "--class=BC") == 0) {
            printf("fh: update ship: species %d name '%s' class to BC\n", spno, ship->name);
            ship->class = BC;
            ship->tonnage = ship_tonnage[BC];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=BM") == 0) {
            printf("fh: update ship: species %d name '%s' class to BM\n", spno, ship->name);
            ship->class = BM;
            ship->tonnage = ship_tonnage[BM];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=BR") == 0) {
            printf("fh: update ship: species %d name '%s' class to BR\n", spno, ship->name);
            ship->class = BR;
            ship->tonnage = ship_tonnage[BR];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=BS") == 0) {
            printf("fh: update ship: species %d name '%s' class to BS\n", spno, ship->name);
            ship->class = BS;
            ship->tonnage = ship_tonnage[BS];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=BW") == 0) {
            printf("fh: update ship: species %d name '%s' class to BW\n", spno, ship->name);
            ship->class = BW;
            ship->tonnage = ship_tonnage[BW];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=CA") == 0) {
            printf("fh: update ship: species %d name '%s' class to CA\n", spno, ship->name);
            ship->class = CA;
            ship->tonnage = ship_tonnage[CA];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=CC") == 0) {
            printf("fh: update ship: species %d name '%s' class to CC\n", spno, ship->name);
            ship->class = CC;
            ship->tonnage = ship_tonnage[CC];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=CL") == 0) {
            printf("fh: update ship: species %d name '%s' class to CL\n", spno, ship->name);
            ship->class = CL;
            ship->tonnage = ship_tonnage[CL];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=CT") == 0) {
            printf("fh: update ship: species %d name '%s' class to CT\n", spno, ship->name);
            ship->class = CT;
            ship->tonnage = ship_tonnage[CT];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=DD") == 0) {
            printf("fh: update ship: species %d name '%s' class to DD\n", spno, ship->name);
            ship->class = DD;
            ship->tonnage = ship_tonnage[DD];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=DN") == 0) {
            printf("fh: update ship: species %d name '%s' class to DN\n", spno, ship->name);
            ship->class = DN;
            ship->tonnage = ship_tonnage[DN];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=ES") == 0) {
            printf("fh: update ship: species %d name '%s' class to ES\n", spno, ship->name);
            ship->class = ES;
            ship->tonnage = ship_tonnage[ES];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=FF") == 0) {
            printf("fh: update ship: species %d name '%s' class to FF\n", spno, ship->name);
            ship->class = FF;
            ship->tonnage = ship_tonnage[FF];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=PB") == 0) {
            printf("fh: update ship: species %d name '%s' class to PB\n", spno, ship->name);
            ship->class = PB;
            ship->tonnage = ship_tonnage[PB];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=SD") == 0) {
            printf("fh: update ship: species %d name '%s' class to SD\n", spno, ship->name);
            ship->class = SD;
            ship->tonnage = ship_tonnage[SD];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class=TR") == 0) {
            printf("fh: update ship: species %d name '%s' class to TR\n", spno, ship->name);
            ship->class = TR;
            ship->tonnage = ship_tonnage[TR];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--ftl") == 0) {
            printf("fh: update ship: species %d name '%s' force ftl\n", spno, ship->name);
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--sub-light") == 0) {
            printf("fh: update ship: species %d name '%s' force sub-light\n", spno, ship->name);
            ship->type = SUB_LIGHT;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--tonnage") == 0) {
            if (ship->class != TR) {
                fprintf(stderr, "error: tonnage is valid only for transports\n");
                return 2;
            }
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing tonnage value\n");
                return 2;
            }
            i++;
            int value = atoi(argv[i]);
            if (value < 1) {
                fprintf(stderr, "error: invalid tonnage value\n");
                return 2;
            } else if (value > 5 * sp->tech_level[MA]) {
                fprintf(stderr, "error: invalid tonnage value (exceeds MA)\n");
                return 2;
            }
            printf("fh: update ship: species %d name '%s' tonnage from %d to %d\n",
                   spno, ship->name, ship->tonnage, value);
            ship->tonnage = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "age") == 0) {
            if (i + 1 == argc || argv[i + 1] == NULL || strlen(argv[i + 1]) == 0) {
                fprintf(stderr, "error: missing age value\n");
                return 2;
            }
            i++;
            int value = atoi(argv[i]);
            if (value < 0 || value > 50) {
                fprintf(stderr, "error: invalid age value\n");
                return 2;
            }
            printf("fh: update ship: species %d name '%s' age from %4d to %4d\n", spno, ship->name, ship->age, value);
            ship->age = value;
            data_modified[spidx] = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (sp == NULL || data_modified[spidx] == FALSE) {
        printf("fh: update ship: no changes to save\n");
    } else {
        printf("fh: update: saving    species  data...\n");
        save_species_data();
    }
    return 0;
}


int updateSpecies(int argc, char *argv[]) {
    species_data_t *sp = NULL;
    int spno = 0;
    int spidx = -1;

    printf("fh: update: loading   species  data...\n");
    get_species_data();

    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "fh: update species: argc %2d argv '%s'\n", i, argv[i]);
        const char *opt = argv[i];
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr, "fh: usage: update species spNo [field value]\n");
            fprintf(stderr, "    where: spNo is a valid species number (no leading zeroes)\n");
            fprintf(stderr, "    where: field is govt-type\n");
            fprintf(stderr, "      and: value is between 1 and 31 characters\n");
            return 2;
        } else if (spno == 0) {
            spno = atoi(opt);
            spidx = spno - 1;
            if (!(1 <= spno && spno <= galaxy.num_species)) {
                fprintf(stderr, "error: invalid species number '%s'\n", opt);
                return 2;
            } else if (data_in_memory[spidx] == FALSE) {
                fprintf(stderr, "error: species %d is not loaded\n", spno);
                return 2;
            }
            printf("fh: update species: species number is %3d\n", spno);
            sp = spec_data + spidx;
        } else if (strcmp(opt, "bi") == 0 || strcmp(opt, "gv") == 0 || strcmp(opt, "ls") == 0 ||
                   strcmp(opt, "ma") == 0 || strcmp(opt, "mi") == 0 || strcmp(opt, "ml") == 0) {
            const char *tech = opt;
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
            printf("fh: update species: %s from %4d to %4d\n", tech, sp->tech_level[code], value);
            sp->tech_level[code] = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "eu") == 0) {
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
            printf("fh: update species: eu from %4d to %4d\n", sp->econ_units, value);
            sp->econ_units = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "govt-type") == 0) {
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
            printf("fh: update species: govt-type from \"%s\" to \"%s\"\n", sp->govt_type, value);
            memset(sp->govt_type, 0, 32);
            strcpy(sp->govt_type, value);
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "hp") == 0) {
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
            printf("fh: update species: hp from %4d to %4d\n", sp->hp_original_base, value);
            sp->hp_original_base = value;
            data_modified[spidx] = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }
    if (sp == NULL || data_modified[spidx] == FALSE) {
        printf("fh: update species: no changes to save\n");
    } else {
        printf("fh: update: saving    species  data...\n");
        save_species_data();
    }
    return 0;
}


int updateStar(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    return 0;
}


