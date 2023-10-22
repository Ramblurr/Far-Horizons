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
    int loadAnything = 0;
    int loadGalaxy = 0;
    int loadLocations = 0;
    int loadPlanets = 0;
    int loadSpecies = 0;
    int loadStars = 0;
    int loadTransactions = 0;

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
            fprintf(stderr, "usage: import json globals\n");
            return 2;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "galaxy") == 0 && val == NULL) {
            loadAnything = 1;
            loadGalaxy = 1;
        } else if (strcmp(opt, "locations") == 0 && val == NULL) {
            loadAnything = 1;
            loadLocations = 1;
        } else if (strcmp(opt, "planets") == 0 && val == NULL) {
            loadAnything = 1;
            loadPlanets = 1;
        } else if (strcmp(opt, "species") == 0 && val == NULL) {
            loadAnything = 1;
            loadSpecies = 1;
        } else if (strcmp(opt, "stars") == 0 && val == NULL) {
            loadAnything = 1;
            loadStars = 1;
        } else if (strcmp(opt, "transactions") == 0 && val == NULL) {
            loadAnything = 1;
            loadTransactions = 1;
        } else {
            fprintf(stderr, "fh: import: json: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }

    if (!loadAnything) {
        printf(" info: nothing to import\n");
        return 0;
    }

    printf("fh: import: json: loading   galaxy  data...\n");
    get_galaxy_data();
    printf("fh: import: json: loading   star    data...\n");
    get_star_data();
    printf("fh: import: json: loading   planet  data...\n");
    get_planet_data();
    printf("fh: import: json: loading   species data...\n");
    get_species_data();

    galaxy_data_t *gd = 0;
    if (loadGalaxy) {
        printf("fh: import: json: importing galaxy data...\n");
        cJSON *js = jsonParseFile("galaxy.json");
        if (js == 0) {
            fprintf(stderr, "fh: import: json: galaxy: error\n");
            return 2;
        }
        gd = galaxyDataFromJson(js);
        if (gd == 0) {
            fprintf(stderr, "fh: import: json: galaxy: error\n");
            return 2;
        }
    }

    if (verbose_mode) {
        printf(" info: done importing\n");
    }
    if (test_mode) {
        printf(" info: test mode, not saving changes\n");
        return 0;
    }

    if (verbose_mode) {
        printf(" info: saving changes...\n");
    }
    if (gd != 0) {
        save_galaxy_data(gd);
        printf(" info: saved galaxy changes...\n");
    }

    return 2;
}
