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
#include <unistd.h>
#include "engine.h"
#include "data.h"
#include "enginevars.h"
#include "galaxyio.h"
#include "import.h"
#include "json.h"
#include "unmarshal.h"
#include "cjson/cJSON.h"
#include "planetio.h"
#include "speciesio.h"
#include "stario.h"
#include "locationio.h"
#include "transactionio.h"
#include "namplavars.h"
#include "shipvars.h"


static int importData(FILE *fp);

static int importFromJson(int argc, char *argv[]);

int importCommand(int argc, char *argv[]) {
    char *importFileName = NULL;

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
            fprintf(stderr, "usage: import options...\n");
            fprintf(stderr, "  opt: --galaxy=string   name of galaxy JSON file to import\n");
            return 2;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "json") == 0 && val == NULL) {
            return importFromJson(argc - i, argv + i);
        } else {
            fprintf(stderr, "import: unknown option '%s%s%s'\n", opt, val ? "=" : "", val);
            return 2;
        }
    }

    return 0;
}


int importData(FILE *fp) {
    json_value_t *j = json_unmarshal(fp);
    global_data_t *d = unmarshalData(j);

    galaxy.turn_number = d->turn;
    galaxy.radius = d->cluster->radius;
    galaxy.d_num_species = d->cluster->d_num_species;
    if (d->cluster != NULL) {
        for (global_species_t **species = d->species; *species != NULL; species++) {
            galaxy.num_species++;
        }
    }

    //fprintf(stderr, "%s: %s: %d\n", __FILE__, __FUNCTION__ , __LINE__);
    return 0;
}

int importFromJson(int argc, char *argv[]) {
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
            fprintf(stderr, "usage: import json\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else {
            fprintf(stderr, "fh: import: json: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }

    printf("fh: import: json: importing json data...\n");

    printf("fh: import: json: importing galaxy.json\n");
    galaxy_data_t *gd = 0;
    cJSON *root = jsonParseFile("galaxy.json");
    gd = galaxyDataFromJson(root);
    cJSON_Delete(root);

    printf("fh: import: json: importing stars.json\n");
    star_data_t **stars = 0;
    root = jsonParseFile("stars.json");
    stars = starsDataFromJson(root);
    cJSON_Delete(root);

    printf("fh: import: json: importing planets.json\n");
    planet_data_t **planets = 0;
    root = jsonParseFile("planets.json");
    planets = planetsDataFromJson(root);
    cJSON_Delete(root);

    printf("fh: import: json: importing species json files\n");
    species_data_t **species = ncalloc(__FUNCTION__ , __LINE__, MAX_SPECIES+1, sizeof(species_data_t *));
    for (int spNo = 1; spNo <= MAX_SPECIES; spNo++) {
        char filename[128];
        sprintf(filename, "species.%03d.json", spNo);
        if (access(filename, F_OK) != 0) {
            continue;
        }
        printf("fh: import: json: importing %s\n", filename);
        root = jsonParseFile(filename);
        species[spNo] = speciesFromJson(root);
        cJSON_Delete(root);
    }

    printf("fh: import: json: importing species named planet json files\n");
    nampla_data_t ***namedPlanets = ncalloc(__FUNCTION__ , __LINE__, MAX_SPECIES+1, sizeof(nampla_data_t **));
    for (int spNo = 1; spNo <= MAX_SPECIES; spNo++) {
        if (species[spNo] == 0) {
            continue;
        }
        char filename[128];
        sprintf(filename, "species.%03d.planets.json", spNo);
        printf("fh: import: json: importing %s\n", filename);
        if (access(filename, F_OK) != 0) {
            fprintf(stderr, "species %3d: missing %s file\n", spNo, filename);
            exit(2);
        }
        root = jsonParseFile(filename);
        namedPlanets[spNo] = namedPlanetsFromJson(root);
        cJSON_Delete(root);
    }

    printf("fh: import: json: importing species ships json files\n");
    ship_data_t ***ships = ncalloc(__FUNCTION__ , __LINE__, MAX_SPECIES+1, sizeof(ship_data_t **));
    for (int spNo = 1; spNo <= MAX_SPECIES; spNo++) {
        if (species[spNo] == 0) {
            continue;
        }
        char filename[128];
        sprintf(filename, "species.%03d.ships.json", spNo);
        printf("fh: import: json: importing %s\n", filename);
        if (access(filename, F_OK) != 0) {
            fprintf(stderr, "species %3d: missing %s file\n", spNo, filename);
            exit(2);
        }
        root = jsonParseFile(filename);
        ships[spNo] = shipsFromJson(root);
        cJSON_Delete(root);
    }

    printf("fh: import: json: importing locations.json\n");
    sp_loc_data_t **locations = 0;
    root = jsonParseFile("locations.json");
    locations = locationsFromJson(root);
    cJSON_Delete(root);

    printf("fh: import: json: done importing\n");

    // translate to old structure
    printf("fh: import: json: translating to binary structures...\n");
    printf("fh: import: json: translating stars to binary structures...\n");
    int numStars = 0;
    star_data_t *starBase = (star_data_t *) ncalloc(__FUNCTION__, __LINE__, MAX_STARS, sizeof(star_data_t));
    for (numStars = 0; stars[numStars] != 0; numStars++) {
        memcpy(&starBase[numStars], stars[numStars], sizeof(star_data_t));
    }
    printf("fh: import: json: translating planets to binary structures...\n");
    int numPlanets = 0;
    planet_data_t *planetBase = (planet_data_t *) ncalloc(__FUNCTION__, __LINE__, MAX_PLANETS,
                                                          sizeof(planet_data_t));
    for (numPlanets = 0; planets[numPlanets] != 0; numPlanets++) {
        memcpy(&planetBase[numPlanets], planets[numPlanets], sizeof(planet_data_t));
    }
    printf("fh: import: json: translating species to binary structures...\n");
    // clear out memory
    for (int i = 0; i < MAX_SPECIES; i++) {
        data_in_memory[i] = 0;
        data_modified[i] = 0;
        memset(&spec_data[i], 0, sizeof(species_data_t));
        namp_data[i] = 0;
        ship_data[i] = 0;
    }
    int numSpecies = 0;
    for (numSpecies = 0; species[numSpecies] != 0; numSpecies++) {
        species_data_t *sp = species[numSpecies];
        data_in_memory[sp->id - 1] = 1;
        data_modified[sp->id - 1] = 1;
        sp->econ_units = 555 * sp->id;
        memcpy(&spec_data[sp->id-1], sp, sizeof(species_data_t));
        namp_data[sp->id-1] = ncalloc(__FUNCTION__ , __LINE__, sp->num_namplas, sizeof(nampla_data_t));
        nampla_data_t **namedPlanetsBase = namedPlanets[sp->id];
        for (int j = 0; j < sp->num_namplas; j++) {
            memcpy(&namp_data[sp->id-1], namedPlanetsBase[j], sizeof(nampla_data_t));
        }
        ship_data[sp->id-1] = ncalloc(__FUNCTION__ , __LINE__, sp->num_ships, sizeof(ship_data_t));
        ship_data_t **shipsBase = ships[sp->id];
        for (int j = 0; j < sp->num_ships; j++) {
            memcpy(&ship_data[sp->id-1], shipsBase[j], sizeof(ship_data_t));
        }
    }
    printf("fh: import: json: translating locations to binary structures...\n");
    int numLocations = 0;
    sp_loc_data_t *allLocations = (sp_loc_data_t *) ncalloc(__FUNCTION__, __LINE__, MAX_LOCATIONS, sizeof(sp_loc_data_t));
    for (numLocations = 0; locations[numLocations] != 0; numLocations++) {
        memcpy(&allLocations[numLocations], locations[numLocations], sizeof(sp_loc_data_t));
    }
    printf("fh: import: json: done translating\n");

    // global sanity checks
    printf("fh: import: json: running data quality checks...\n");
    printf("fh: import: json: done checking\n");

    if (test_mode) {
        printf("fh: import: json: test mode, not saving changes\n");
        return 0;
    }

    printf("fh: import: json: saving changes...\n");

    save_galaxy_data(gd);
    printf("fh: import: json: saved galaxy          changes...\n");

    save_star_data(starBase, numStars);
    printf("fh: import: json: saved stars           changes...\n");

    save_planet_data(planetBase, numPlanets);
    printf("fh: import: json: saved planets         changes...\n");

    save_species_data();
    printf("fh: import: json: saved species         changes...\n");

    printf("fh: import: json: skipped named planets changes...\n");
    printf("fh: import: json: skipped ships         changes...\n");

    save_location_data(allLocations, numLocations);
    printf("fh: import: json: saved locations       changes...\n");

    save_transaction_data();
    printf("fh: import: json: saved transaction     changes...\n");

    printf("fh: import: json: done saving\n");

    return 0;
}
