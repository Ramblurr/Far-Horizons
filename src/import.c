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


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "import.h"
#include "unmarshal.h"
#include "galaxyio.h"
#include "stario.h"
#include "planetio.h"
#include "speciesio.h"
#include "cjson/helpers.h"
#include "namplavars.h"
#include "shipvars.h"


int importFromJson(int doTest);


int importCommand(int argc, char *argv[]) {
    int doImportJson = FALSE;
    int doTest = FALSE;

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
            doTest = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            doTest = TRUE;
        } else if (strcmp(opt, "json") == 0 && val == NULL) {
            doImportJson = TRUE;
        } else {
            fprintf(stderr, "import: unknown option '%s%s%s'\n", opt, val ? "=" : "", val);
            return 2;
        }
    }

    if (doImportJson) {
        return importFromJson(doTest);
    }

    return 0;
}

int importFromJson(int doTest) {
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    cJSON *root = jsonParseFile("galaxy.json");
    unmarshalGalaxyFile(root, &galaxy);
    cJSON_Delete(root);

    root = jsonParseFile("systems.json");
    unmarshalSystemsFile(root, star_base, planet_base);
    cJSON_Delete(root);

    for (int i = 0; i < MAX_SPECIES; i++) {
        if (data_in_memory[i]) {
            char filename[128];
            sprintf(filename, "species.%03d.json", i + 1);
            struct stat sb;
            if (stat(filename, &sb) != 0) {
                // assume that file is missing
                printf(" warn: missing species file '%s'\n", filename);
                continue;
            }
            root = jsonParseFile(filename);
            unmarshalSpeciesFile(root, &spec_data[i], namp_data[i], ship_data[i]);
        }
    }

    if (doTest) {
        printf(" test: changes not saved\n");
        return 0;
    }

    save_galaxy_data();
    save_star_data();
    save_planet_data();
    save_species_data();

    return 2;
}
