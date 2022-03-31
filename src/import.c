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


static int importData(FILE *fp);


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
            fprintf(stderr, "  opt: --file=string   name of file to import\n");
            return 2;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "--file") == 0 && val && *val) {
            importFileName = val;
        } else {
            fprintf(stderr, "import: unknown option '%s%s%s'\n", opt, val ? "=" : "", val);
            return 2;
        }
    }

    if (importFileName == NULL || *importFileName == 0) {
        fprintf(stderr, "error: you must supply the file name to import\n");
        return 2;
    }
    if (verbose_mode) {
        printf(" info: importing '%s'\n", importFileName);
    }

    FILE *fp = fopen(importFileName, "rb");
    if (fp == NULL) {
        perror("importCommand: ");
        exit(2);
    }

    int rs = importData(fp);
    fclose(fp);
    return rs;
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


