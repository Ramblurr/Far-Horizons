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
#include <string.h>
#include "cfgfile.h"
#include "commandvars.h"
#include "create.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "logvars.h"
#include "nampla.h"
#include "namplavars.h"
#include "orders.h"
#include "planetio.h"
#include "planetvars.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "stario.h"


int createOrdersCommand(int argc, char *argv[]);

int createGalaxyCommand(int argc, char *argv[]);

int createHomeSystemTemplatesCommand(int argc, char *argv[]);

int createSpeciesCommand(int argc, char *argv[]);


int createCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
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
            fprintf(stderr, "fh: usage: create (galaxy | home-system-templates | species)\n");
            return 2;
        } else if (strcmp(opt, "galaxy") == 0) {
            return createGalaxyCommand(argc - i, argv + i);
        } else if (strcmp(opt, "home-system-templates") == 0) {
            return createHomeSystemTemplatesCommand(argc - i, argv + i);
        } else if (strcmp(opt, "orders") == 0) {
            return createOrdersCommand(argc - i, argv + i);
        } else if (strcmp(opt, "species") == 0) {
            return createSpeciesCommand(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, opt);
            return 2;
        }
    }
    return 0;
}


int createGalaxyCommand(int argc, char *argv[]) {
    int desiredNumSpecies = 0;
    int desiredNumStars = 0;
    int galacticRadius = 0;
    int lessCrowded = FALSE;
    int suggestValues = FALSE;

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
                    "fh: usage: create galaxy --species=integer [--stars=integer] [--radius=integer] [--suggest-values]\n");
            return 2;
        } else if (strcmp(opt, "--less-crowded") == 0) {
            lessCrowded = TRUE;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            galacticRadius = atoi(val);
            if (galacticRadius < MIN_RADIUS || galacticRadius > MAX_RADIUS) {
                fprintf(stderr, "error: radius must be between %d and %d parsecs.\n", MIN_RADIUS, MAX_RADIUS);
                return 2;
            }
        } else if (strcmp(opt, "--species") == 0 && val != NULL) {
            desiredNumSpecies = atoi(val);
            if (desiredNumSpecies < MIN_SPECIES || desiredNumSpecies > MAX_SPECIES) {
                fprintf(stderr, "error: species must be between %d and %d.\n", MIN_SPECIES, MAX_SPECIES);
                return 2;
            }
        } else if (strcmp(opt, "--stars") == 0 && val != NULL) {
            desiredNumStars = atoi(val);
            if (desiredNumStars < MIN_STARS || desiredNumStars > MAX_STARS) {
                fprintf(stderr, "error: stars must be between %d and %d.\n", MIN_STARS, MAX_STARS);
                return 2;
            }
        } else if (strcmp(opt, "--suggest-values") == 0) {
            suggestValues = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (desiredNumSpecies == 0) {
        fprintf(stderr, "error: you must supply the desired number of species.\n");
        return 2;
    }

    if (desiredNumStars == 0 || suggestValues != FALSE) {
        if (lessCrowded == FALSE) {
            desiredNumStars = (desiredNumSpecies * STANDARD_NUMBER_OF_STAR_SYSTEMS) / STANDARD_NUMBER_OF_SPECIES;
        } else {
            // bump the number of stars by 50% to make it take longer to encounter other species.
            desiredNumStars =
                    (3 * desiredNumSpecies * STANDARD_NUMBER_OF_STAR_SYSTEMS) / (2 * STANDARD_NUMBER_OF_SPECIES);
        }
        if (desiredNumStars > MAX_STARS) {
            fprintf(stderr, "error: calculation results in a number greater than %d stars.\n", MAX_STARS);
            return 2;
        }
    }

    if (galacticRadius == 0 || suggestValues != FALSE) {
        long minVolume = desiredNumStars
                         * STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS
                         / STANDARD_NUMBER_OF_STAR_SYSTEMS;
        for (galacticRadius = MIN_RADIUS; galacticRadius * galacticRadius * galacticRadius < minVolume;) {
            galacticRadius++;
        }
        if (galacticRadius > MAX_RADIUS) {
            fprintf(stderr, "error: calculation results in a radius greater than %d parsecs.\n", MAX_RADIUS);
            return 2;
        }
    }

    if (suggestValues != FALSE) {
        printf(" info: for %d species, a %sgalaxy needs about %d star systems.\n",
               desiredNumSpecies, lessCrowded == FALSE ? "" : "less crowded ", desiredNumStars);
        printf(" info: for %d stars, the galaxy should have a radius of about %d parsecs.\n",
               desiredNumStars, galacticRadius);
        return 0;
    }

    return createGalaxy(galacticRadius, desiredNumStars, desiredNumSpecies);
}


int createHomeSystemTemplatesCommand(int argc, char *argv[]) {
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
                    "fh: usage: create home-system-templates...\n");
            return 2;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return createHomeSystemTemplates();
}


int createOrdersCommand(int argc, char *argv[]) {
    int advanced = FALSE;
    int reminder = FALSE;

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
            fprintf(stderr, "fh: usage: create orders opt\n");
            fprintf(stderr, "           creates orders files for species that have not yet submitted theirs\n");
            fprintf(stderr, "      opt: --add-reminder    insert a reminder into the orders\n");
            fprintf(stderr, "         : --auto            generate a better set of orders\n");
            fprintf(stderr, "         : --default         generate the minimal set of orders   [default]\n");
            return 2;
        } else if (strcmp(opt, "--add-reminder") == 0 && val == NULL) {
            reminder = TRUE;
        } else if (strcmp(opt, "--auto") == 0 && val == NULL) {
            advanced = TRUE;
        } else if (strcmp(opt, "--default") == 0 && val == NULL) {
            advanced = FALSE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return createOrders(advanced, reminder);
}


int createSpeciesCommand(int argc, char *argv[]) {
    const char *configFile = NULL;
    int radius = 10; // default minimum distance between home systems

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
            fprintf(stderr, "fh: usage: create species...\n");
            return 2;
        } else if (strcmp(opt, "--config") == 0 && val != NULL) {
            configFile = val;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            radius = atoi(val);
            if (radius < 1) {
                fprintf(stderr, "error: invalid radius '%s'\n", val);
                return 2;
            }
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (configFile == NULL || *configFile == 0) {
        fprintf(stderr, "error: you must supply a configuration file name\n");
        return 2;
    }
    species_cfg_t *cfg = CfgSpeciesFromFile(configFile);
    if (cfg == NULL) {
        fprintf(stderr, "error: found no sections in config file '%s'.\n", configFile);
        return 2;
    }

    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if (data_in_memory[spidx] == FALSE) {
            fprintf(stderr, "error: internal error: createSpeciesCommand: sp %d data not in memory\n", spidx + 1);
            exit(2);
        }
    }

    if (radius > galaxy.radius / 2) {
        fprintf(stderr, "error: radius must be between 1 and %d for this galaxy\n", galaxy.radius / 2);
        return 2;
    }

    for (species_cfg_t *c = cfg; c != NULL; c = c->next) {
        species_index = galaxy.num_species;
        species_number = species_index + 1;

        if (species_number > galaxy.d_num_species) {
            fprintf(stderr, "error: galaxy limit is %d species\n", galaxy.d_num_species);
            return 2;
        } else if (data_in_memory[species_index] != FALSE) {
            fprintf(stderr, "error: createSpeciesCommand: internal error: data_in_memory[%d] is TRUE\n", species_index);
            exit(2);
        } else if (data_modified[species_index] != FALSE) {
            fprintf(stderr, "error: createSpeciesCommand: internal error: data_modified[%d] is TRUE\n", species_index);
            exit(2);
        }

        if (c->name == NULL || *(c->name) == 0) {
            fprintf(stderr, "error: section missing species name\n");
            return 2;
        } else if (strlen(c->name) < 5) {
            fprintf(stderr, "error: species name '%s' must be at least 5 characters long.\n", c->name);
            return 2;
        } else if (strlen(c->name) > 31) {
            fprintf(stderr, "error: species name '%s' must be less than 32 characters long.\n", c->name);
            return 2;
        } else {
            for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
                if (strcasecmp(c->name, spec_data[spidx].name) == 0) {
                    fprintf(stderr, "error: species name '%s' is not unique\n", c->name);
                    return 2;
                }
            }
        }
        if (c->govtname == NULL || *(c->govtname) == 0) {
            fprintf(stderr, "error: section missing species govtname\n");
            return 2;
        } else if (strlen(c->govtname) < 5) {
            fprintf(stderr, "error: species govtname '%s' must be at least 5 characters long.\n", c->govtname);
            return 2;
        } else if (strlen(c->govtname) > 31) {
            fprintf(stderr, "error: species govtname '%s' must be less than 32 characters long.\n", c->govtname);
            return 2;
        }
        if (c->govttype == NULL || *(c->govttype) == 0) {
            fprintf(stderr, "error: section missing species govttype\n");
            return 2;
        } else if (strlen(c->govttype) < 5) {
            fprintf(stderr, "error: species govttype '%s' must be at least 5 characters long.\n", c->govttype);
            return 2;
        } else if (strlen(c->govttype) > 31) {
            fprintf(stderr, "error: species govttype '%s' must be less than 32 characters long.\n", c->govttype);
            return 2;
        }
        if (c->homeworld == NULL || *(c->homeworld) == 0) {
            fprintf(stderr, "error: section missing species homeworld\n");
            return 2;
        } else if (strlen(c->homeworld) < 5) {
            fprintf(stderr, "error: species homeworld '%s' must be at least 5 characters long.\n", c->homeworld);
            return 2;
        } else if (strlen(c->homeworld) > 31) {
            fprintf(stderr, "error: species homeworld '%s' must be less than 32 characters long.\n", c->homeworld);
            return 2;
        }
        if (c->bi + c->gv + c->ls + c->ml > 15) {
            fprintf(stderr, "error: species tech levels must sum to less than 16.\n");
            return 2;
        }

        // clear out the species data, just in case
        struct species_data *sp = &spec_data[species_index];
        memset(sp, 0, sizeof(struct species_data));

        sp->id = species_number;
        sp->index = species_index;
        strcpy(sp->name, c->name);
        strcpy(sp->govt_name, c->govtname);
        strcpy(sp->govt_type, c->govttype);
        sp->tech_level[MA] = 10;
        sp->tech_level[MI] = 10;
        sp->tech_level[ML] = c->ml;
        sp->tech_level[GV] = c->gv;
        sp->tech_level[LS] = c->ls;
        sp->tech_level[BI] = c->bi;
        // initialize other tech stuff
        for (int t = MI; t <= BI; t++) {
            sp->tech_knowledge[t] = sp->tech_level[t];
            sp->init_tech_level[t] = sp->tech_level[t];
            sp->tech_eps[t] = 0;
        }
        // make O2 a required gas for the species
        sp->required_gas = O2;

        // find candidate home systems
        star_data_t *candidateSystems[MAX_SPECIES + 1];
        int numCandidates = 0; // index into candidateSystems
        for (int s = 0; s < num_stars; s++) {
            star_data_t *star = star_base + s;
            for (int p = 0; p < star->num_planets; p++) {
                planet_data_t *planet = planet_base + star->planet_index + p;
                if (planet->special != HOME_PLANET) {
                    continue;
                }
                species_data_t *claimedBy = NULL;
                for (int spidx = 0; spidx < galaxy.num_species && claimedBy == NULL; spidx++) {
                    species_data_t *sp = &spec_data[spidx];
                    if (sp->x == star->x && sp->y == star->y && sp->z == star->z) {
                        // would be cruel to have two species share a system
                        claimedBy = sp;
                    }
                }
                if (claimedBy == NULL) {
                    // printf("%s:%d: unclaimed system: %d,%d,%d\n", __FUNCTION__, __LINE__, star->x, star->y, star->z);
                    candidateSystems[numCandidates++] = star;
                }
            }
        }
        if (numCandidates == 0) {
            // no candidates, so create one
            candidateSystems[0] = findHomeSystemCandidate(radius);
            if (candidateSystems[0] == NULL) {
                fprintf(stderr, "error: createSpeciesCommand: no systems meet the criteria for radius of %d!\n",
                        radius);
                return 2;
            }
            if (changeSystemToHomeSystem(candidateSystems[0]) != 0) {
                fprintf(stderr, "error: createSpeciesCommand: failed to change system to home system\n");
                return 2;
            }
            numCandidates++;
        }
        // randomly choose a home system from the list of candidates
        star_data_t *homeSystem = candidateSystems[rnd(numCandidates) - 1];
        // fetch the home planet in the home system
        planet_data_t *home_planet = NULL;
        for (int pn = 0; pn < homeSystem->num_planets; pn++) {
            planet_data_t *p = planet_base + homeSystem->planet_index + pn;
            if (p->special == HOME_PLANET) {
                home_planet = p;
                break;
            }
        }
        sp->x = homeSystem->x;
        sp->y = homeSystem->y;
        sp->z = homeSystem->z;
        sp->pn = home_planet->orbit;

        nampla_data_t *home_nampla = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(nampla_data_t));
        if (home_nampla == NULL) {
            perror("createSpeciesCommand:");
            fprintf(stderr, "error: createSpeciesCommand: unable to allocate memory\n");
            exit(2);
        }
        namp_data[species_index] = home_nampla;
        strcpy(home_nampla->name, c->homeworld);
        home_nampla->star = homeSystem;
        home_nampla->planet = home_planet;
        home_nampla->x = homeSystem->x;
        home_nampla->y = homeSystem->y;
        home_nampla->z = homeSystem->z;
        home_nampla->pn = home_planet->orbit;
        home_nampla->planet_index = home_planet->index;

        // verify that planet has oxygen and initialize the good gases
        int good_gas[14];
        for (int i = 0; i < 14; i++) {
            good_gas[i] = FALSE;
        }
        int foundOxygen = FALSE;
        int num_neutral = 0;
        for (int i = 0; i < 4; i++) {
            if (home_planet->gas[i] == O2) {
                foundOxygen = TRUE;
                sp->required_gas_min = home_planet->gas_percent[i] / 2;
                if (sp->required_gas_min < 1) {
                    sp->required_gas_min = 1;
                }
                sp->required_gas_max = 2 * home_planet->gas_percent[i];
                if (sp->required_gas_max < 20) {
                    sp->required_gas_max += 20;
                } else if (sp->required_gas_max > 100) {
                    sp->required_gas_max = 100;
                }
            }
            if (home_planet->gas[i] > 0) {
                // all home planet gases are either required or neutral
                good_gas[home_planet->gas[i]] = TRUE;
                num_neutral++;
            }
        }
        if (foundOxygen == FALSE) {
            fprintf(stderr, "error: createSpeciesCommand: internal error: planet id %4d does not have %s(%d)!\n",
                    home_planet->id, gas_string[O2], O2);
            exit(2);
        }

        // Helium must always be neutral since it is a noble gas
        if (good_gas[HE] == FALSE) {
            good_gas[HE] = TRUE;
            num_neutral++;
        }
        // this game is biased towards oxygen breathers, so make H2O neutral
        if (good_gas[H2O] == FALSE) {
            good_gas[H2O] = TRUE;
            num_neutral++;
        }
        // initialize neutral gases for the species.
        // start with the good_gas array and add neutral gases until there are exactly seven of them.
        // one of the seven gases will be the required gas (currently hard-coded to Oxygen).
        for (int roll = rnd(13); num_neutral < 7; roll = rnd(13)) {
            if (good_gas[roll] == FALSE) {
                good_gas[roll] = TRUE;
                num_neutral++;
            }
        }
        // add the list of neutral gases to the species data
        int g = 0; // index into neutral gases array
        for (int i = 1; i < 14; i++) { // start at one, ugh, first slot is ignored
            if (good_gas[i] != FALSE && i != O2) {
                sp->neutral_gas[g] = i;
                g++;
            }
        }
        // same for poison gases
        g = 0;
        for (int i = 1; i < 14; i++) { // start at one, ugh, first slot is ignored
            if (good_gas[i] == FALSE) {
                sp->poison_gas[g] = i;
                g++;
            }
        }

        /* Do mining and manufacturing bases of home planet.
         * Initial mining and production capacity will be 25 times sum of MI and MA plus a small random amount.
         * Mining and manufacturing base will be  reverse-calculated from the capacity. */
        int base = sp->tech_level[MI] + sp->tech_level[MA];
        base = (25 * base) + rnd(base) + rnd(base) + rnd(base);
        home_nampla->mi_base = home_planet->mining_difficulty * base / (10 * sp->tech_level[MI]);
        home_nampla->ma_base = 10 * base / sp->tech_level[MA];

        // fill out the rest
        sp->num_namplas = 1;    // just the home planet for now ("nampla" means "named planet")
        home_nampla->status = HOME_PLANET | POPULATED;
        home_nampla->pop_units = HP_AVAILABLE_POP;
        home_nampla->shipyards = 1;
        // everything else was initialized to zero in the earlier call to 'delete_nampla'

        /* Print summary. */
        printf("\n  Summary for species #%d:\n", species_number);

        printf("\tName of species: %s\n", sp->name);
        printf("\tName of home planet: %s\n", home_nampla->name);
        printf("\t\tCoordinates: %d %d %d #%d\n", sp->x, sp->y, sp->z, sp->pn);
        printf("\tName of government: %s\n", sp->govt_name);
        printf("\tType of government: %s\n\n", sp->govt_type);

        printf("\tTech levels: ");
        for (int i = 0; i < 6; i++) {
            printf("%s = %d", tech_name[i], sp->tech_level[i]);
            if (i == 2) {
                printf("\n\t             ");
            } else if (i < 5) {
                printf(",  ");
            }
        }

        printf("\n\n\tFor this species, the required gas is %s (%d%%-%d%%).\n",
               gas_string[sp->required_gas], sp->required_gas_min, sp->required_gas_max);

        printf("\tGases neutral to species:");
        for (int i = 0; i < 6; i++) {
            printf(" %s ", gas_string[sp->neutral_gas[i]]);
        }

        printf("\n\tGases poisonous to species:");
        for (int i = 0; i < 6; i++) {
            printf(" %s ", gas_string[sp->poison_gas[i]]);
        }

        printf("\n\n\tInitial mining base = %d.%d. Initial manufacturing base = %d.%d.\n",
               home_nampla->mi_base / 10, home_nampla->mi_base % 10,
               home_nampla->ma_base / 10, home_nampla->ma_base % 10);
        printf("\tIn the first turn, %d raw material units will be produced,\n",
               (10 * sp->tech_level[MI] * home_nampla->mi_base) / home_planet->mining_difficulty);
        printf("\tand the total production capacity will be %d.\n\n",
               (sp->tech_level[MA] * home_nampla->ma_base) / 10);

        // update galaxy
        galaxy.num_species++;

        // set visited_by bit in star data
        int species_array_index = (species_number - 1) / 32;
        int species_bit_number = (species_number - 1) % 32;
        int species_bit_mask = 1 << species_bit_number;
        homeSystem->visited_by[species_array_index] |= species_bit_mask;

        data_in_memory[species_index] = TRUE;
        data_modified[species_index] = TRUE;

        /* Create log file for first turn. Write home star system data to it. */
        char filename[128];
        sprintf(filename, "sp%02d.log", species_number);
        log_file = fopen(filename, "w");
        if (log_file == NULL) {
            perror("createSpeciesCommand:");
            fprintf(stderr, "error: cannot open '%s' for writing!\n\n", filename);
            exit(2);
        }

        fprintf(log_file, "\nScan of home star system for SP %s:\n\n", sp->name);
        species = sp; // species is required by the scan() function
        nampla_base = home_nampla; // nampla_base is required by the scan() function
        scan(home_nampla->x, home_nampla->y, home_nampla->z, TRUE);

        fclose(log_file);
    }

    // save the updated data
    save_galaxy_data(&galaxy);
    FILE *fp = fopen("galaxy.hs.txt", "wb");
    if (fp == NULL) {
        perror("changeSystemToHomeSystem:");
        exit(2);
    }
    galaxyDataAsSexpr(fp);
    fclose(fp);

    save_star_data();
    fp = fopen("stars.hs.txt", "wb");
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

    save_species_data();

    return 0;
}
