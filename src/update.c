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
#include <ctype.h>
#include "galaxy.h"
#include "galaxyio.h"
#include "planetio.h"
#include "ship.h"
#include "shipvars.h"
#include "species.h"
#include "speciesio.h"
#include "stario.h"
#include "update.h"


int updateHomeSystem(int argc, char *argv[]);

int updatePlanet(int argc, char *argv[]);

int updateShip(int argc, char *argv[]);

int updateSpecies(int argc, char *argv[]);

int updateStar(int argc, char *argv[]);


int updateCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    get_galaxy_data();
    get_star_data();
    get_planet_data();
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
            fprintf(stderr,
                    "fh: usage: update (home-system | planet | ship | species | star)\n");
            return 2;
        } else if (strcmp(opt, "home-system") == 0) {
            return updateHomeSystem(argc - i, argv + i);
        } else if (strcmp(opt, "planet") == 0) {
            return updatePlanet(argc - i, argv + i);
        } else if (strcmp(opt, "ship") == 0) {
            return updateShip(argc - i, argv + i);
        } else if (strcmp(opt, "species") == 0) {
            return updateSpecies(argc - i, argv + i);
        } else if (strcmp(opt, "star") == 0) {
            return updateStar(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, opt);
            return 2;
        }
    }
    return 0;
}


// updateHomeSystem changes the type of a planet in an existing star system to HOME_PLANET
// and then replaces the existing planets in the system with ones from the related homesystem
// template.
// by default, this function chooses a system at random from the set of systems that:
//    are at least some minimum distance from other home systems
//    do not already contain a home planet
// the user may specify the minimum distance and/or the locations of a system to use.
// if a system is specified, the use may pass in a flag to ignore the minimum distance
// check described above. this is useful if the gamemaster intends to force species to
// be close neighbors.
// there is no way to force multiple planets in a system to be home planets.
int updateHomeSystem(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    int chooseRandomly = TRUE;
    int force = FALSE;
    int radius = 10;
    int x = 0;
    int y = 0;
    int z = 0;

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
            fprintf(stderr, "fh: usage: update home-system options...\n");
            fprintf(stderr, "           applies a home system template to a system\n");
            fprintf(stderr, "      opt: --radius=n      minimum distance between home systems [default 10]\n");
            fprintf(stderr,
                    "           --system=x,y,z  coordinates of system to update       [defaults to random system]\n");
            fprintf(stderr, "      opt: --force         ignore radius and home system check\n");
            return 2;
        } else if (strcmp(opt, "--force") == 0 && val == NULL) {
            force = TRUE;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            radius = atoi(val);
            if (radius < 1 || radius > galaxy.radius) {
                fprintf(stderr, "error: invalid radius '%s'\n", val);
                return 2;
            }
        } else if (strcmp(opt, "--system") == 0 && val != NULL) {
            for (; *val != 0 && *val != ','; val++) {
                if (!isdigit(*val)) {
                    fprintf(stderr, "error: system coordinates must be numeric\n");
                    return 2;
                }
                x = x * 10 + *val - '0';
            }
            if (*val != ',') {
                fprintf(stderr, "error: system coordinates must be separated by commas\n");
                return 2;
            }
            val++;
            for (; *val != 0 && *val != ','; val++) {
                if (!isdigit(*val)) {
                    fprintf(stderr, "error: system coordinates must be numeric\n");
                    return 2;
                }
                y = y * 10 + *val - '0';
            }
            if (*val != ',') {
                fprintf(stderr, "error: system coordinates must be separated by commas\n");
                return 2;
            }
            val++;
            for (; *val != 0; val++) {
                if (!isdigit(*val)) {
                    fprintf(stderr, "error: system coordinates must be numeric\n");
                    return 2;
                }
                z = z * 10 + *val - '0';
            }
            chooseRandomly = FALSE;
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, opt);
            return 2;
        }
    }

    star_data_t *star = NULL;
    if (chooseRandomly == FALSE) {
        for (int i = 0; i < num_stars; i++) {
            star_data_t *star2 = star_base + i;
            if (star2->x == x && star2->y == y && star2->z == z) {
                star = star2;
                break;
            }
        }
        if (star == NULL) {
            fprintf(stderr, "error: could not find non-home system at %d %d %d\n", x, y, z);
            return 2;
        } else if (star->num_planets < 3) {
            fprintf(stderr, "error: system at %d %d %d has only %d planets!\n", x, y, z, star->num_planets);
        } else if (hasHomeSystemNeighbor(star, radius) != FALSE) {
            if (force == FALSE) {
                fprintf(stderr, "error: system at %d %d %d has home system neighbor within %d parsecs\n",
                        x, y, z, radius);
                return 2;
            }
            printf(" warn: system at %d %d %d has home system neighbor within %d parsecs\n", x, y, z, radius);
        }
        printf(" info: given  system %3d %3d %3d\n", star->x, star->y, star->z);
    } else {
        star = findHomeSystemCandidate(radius);
        if (star == NULL) {
            fprintf(stderr, "error: no systems meet the criteria for home systems!\n");
            return 2;
        }
        printf(" info: random system %3d %3d %3d\n", star->x, star->y, star->z);
    }

    if (changeSystemToHomeSystem(star) != 0) {
        fprintf(stderr, "error: updateHomeSystem: failed to update the system\n");
        return 2;
    }

    // save the updated data
    save_star_data();
    FILE *fp = fopen("stars.hs.txt", "wb");
    if (fp == NULL) {
        perror("changeSystemToHomeSystem:");
        exit(2);
    }
    starDataAsSExpr(star_base, num_stars, fp);
    fclose(fp);

    save_planet_data();
    fp = fopen("planets.hs.txt", "wb");
    if (fp == NULL) {
        perror("changeSystemToHomeSystem:");
        exit(2);
    }
    planetDataAsSExpr(planet_base, num_planets, fp);
    fclose(fp);

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
            fprintf(stderr, "fh: usage: update ship spNo shipName [field value]\n");
            fprintf(stderr, "    where: spNo is a valid species number (no leading zeroes)\n");
            fprintf(stderr, "    where: shipName is a valid ship name (case sensitive, no type)\n");
            fprintf(stderr, "    where: field is age\n");
            fprintf(stderr, "      and: value is an integer between 1 and 50\n");
            fprintf(stderr, "    where: opt is --class=ship_class\n");
            fprintf(stderr, "      and: ship_class is an valid ship class (PB, DD, etc)\n");
            fprintf(stderr, "    where: opt is --ftl\n");
            fprintf(stderr, "    where: opt is --name=new_name\n");
            fprintf(stderr, "    where: opt is --sub-light\n");
            fprintf(stderr, "    where: opt is --tonnage value\n");
            fprintf(stderr, "      and: value is an valid tonnage value\n");
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
                fprintf(stderr, "error: species %d has no ship named '%s'\n", spno, opt);
                return 2;
            }
        } else if (strcmp(opt, "--age") == 0 && val != NULL) {
            int value = atoi(val);
            if (value < 0 || value > 50) {
                fprintf(stderr, "error: invalid age value '%s'\n", val);
                return 2;
            }
            printf("fh: update ship: species %d name '%s' age from %4d to %4d\n", spno, ship->name, ship->age, value);
            ship->age = value;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--class") == 0 && val != NULL) {
            if (strcmp(val, "BC") == 0) {
                ship->class = BC;
            } else if (strcmp(val, "BM") == 0) {
                ship->class = BM;
            } else if (strcmp(val, "BR") == 0) {
                ship->class = BR;
            } else if (strcmp(val, "BS") == 0) {
                ship->class = BS;
            } else if (strcmp(val, "BW") == 0) {
                ship->class = BW;
            } else if (strcmp(val, "CA") == 0) {
                ship->class = CA;
            } else if (strcmp(val, "CC") == 0) {
                ship->class = CC;
            } else if (strcmp(val, "CL") == 0) {
                ship->class = CL;
            } else if (strcmp(val, "CT") == 0) {
                ship->class = CT;
            } else if (strcmp(val, "DD") == 0) {
                ship->class = DD;
            } else if (strcmp(val, "DN") == 0) {
                ship->class = DN;
            } else if (strcmp(val, "ES") == 0) {
                ship->class = ES;
            } else if (strcmp(val, "FF") == 0) {
                ship->class = FF;
            } else if (strcmp(val, "PB") == 0) {
                ship->class = PB;
            } else if (strcmp(val, "SD") == 0) {
                ship->class = SD;
            } else if (strcmp(val, "TR") == 0) {
                ship->class = TR;
            } else {
                fprintf(stderr, "error: unknown ship class '%s'\n", val);
                return 2;
            }
            printf("fh: update ship: species %d name '%s' class to %s\n", spno, ship->name, val);
            ship->tonnage = ship_tonnage[ship->class];
            ship->type = FTL;
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--ftl") == 0 && val != NULL) {
            if (strcmp(val, "yes") == 0) {
                printf("fh: update ship: species %d name '%s' to ftl\n", spno, ship->name);
                ship->type = FTL;
                data_modified[spidx] = TRUE;
            } else if (strcmp(val, "no") == 0) {
                printf("fh: update ship: species %d name '%s' to sub-light\n", spno, ship->name);
                ship->type = SUB_LIGHT;
                data_modified[spidx] = TRUE;
            } else {
                fprintf(stderr, "error: ftl value must be yes or no\n");
                return 2;
            }
        } else if (strcmp(opt, "--item") == 0 && val != NULL) {
            int newQuantity = 0;
            if (strcmp(val, "FS") == 0) {
                newQuantity = ++(ship->item_quantity[FS]);
            } else if (strcmp(val, "GW") == 0) {
                newQuantity = ++(ship->item_quantity[GW]);
            } else {
                fprintf(stderr, "error: unknown inventory item '%s'\n", val);
                return 2;
            }
            printf("fh: update ship: species %d name '%s' inventory %s to %d\n", spno, ship->name, val, newQuantity);
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--name") == 0 && val != NULL) {
            if (strlen(val) < 5) {
                fprintf(stderr, "error: ship name is too short\n");
                return 2;
            } else if (strlen(val) > 30) {
                fprintf(stderr, "error: ship name is too long\n");
                return 2;
            }
            printf("fh: update ship: species %d name '%s' to '%s'\n", spno, ship->name, val);
            memset(ship->name, 0, 32);
            strcpy(ship->name, val);
            data_modified[spidx] = TRUE;
        } else if (strcmp(opt, "--reset-inventory") == 0 && val != NULL) {
            for (int item = 0; item < MAX_ITEMS; item++) {
                ship->item_quantity[item] = 0;
            }
        } else if (strcmp(opt, "--tonnage") == 0 && val != NULL) {
            if (ship->class != TR) {
                fprintf(stderr, "error: tonnage is valid only for transports\n");
                return 2;
            }
            int value = atoi(val);
            if (value < 1) {
                fprintf(stderr, "error: invalid tonnage value '%s'\n", val);
                return 2;
            } else if (value > 5 * sp->tech_level[MA]) {
                fprintf(stderr, "error: invalid tonnage value '%s' (exceeds MA)\n", val);
                return 2;
            }
            printf("fh: update ship: species %d name '%s' tonnage from %d to %d\n",
                   spno, ship->name, ship->tonnage, value);
            ship->tonnage = value;
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


