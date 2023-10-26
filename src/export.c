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
#include <stdlib.h>
#include "marshal.h"
#include "galaxyio.h"
#include "namplaio.h"
#include "planetio.h"
#include "shipio.h"
#include "speciesio.h"
#include "stario.h"
#include "namplavars.h"
#include "shipvars.h"

int exportToJson(int argc, char *argv[]);


int exportCommand(int argc, char *argv[]) {
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
            fprintf(stderr, "usage: export json\n");
            return 2;
        } else if (strcmp(opt, "json") == 0 && val == NULL) {
            return exportToJson(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: export: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return 0;
}


int exportToJson(int argc, char *argv[]) {
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    cJSON *root = marshalGalaxy();
    if (root == 0) {
        fprintf(stderr, "error: there was an error converting galaxy data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "galaxy", "galaxy.json");
    cJSON_Delete(root);

    root = marshalSystems();
    if (root == 0) {
        fprintf(stderr, "error: there was an error converting systems data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "systems", "systems.json");
    cJSON_Delete(root);

    for (int i = 0; i < MAX_SPECIES; i++) {
        if (data_in_memory[i]) {
            root = marshalSpecies(&spec_data[i], namp_data[i], ship_data[i]);
            if (root == 0) {
                fprintf(stderr, "error: there was an error converting species data to json\n");
                exit(2);
            }
            char filename[128];
            sprintf(filename, "species.%03d.json", i + 1);
            jsonWriteFile(root, "species", filename);
            cJSON_Delete(root);
        }
    }

    return 0;
}
