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
#include "galaxyio.h"
#include "stario.h"
#include "planetio.h"
#include "locationio.h"
#include "speciesio.h"


void dumpCommand(int argc, char *argv[]);


int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "dump") == 0) {
            dumpCommand(argc - i, argv + i);
        }
    }
}


void dumpCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    for (int i = 1; i < argc; i++) {
        fprintf(stderr, "[fh] %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        if (strcmp(argv[i], "galaxy") == 0) {
            printf("[fh] %s: loading %s file...\n", cmdName, argv[i]);
            get_galaxy_data();
            printf("[fh] %s: dumping %s file...\n", cmdName, argv[i]);
            save_galaxy_data();
        } else if (strcmp(argv[i], "locations") == 0) {
            printf("[fh] %s: loading %s file...\n", cmdName, argv[i]);
            get_location_data();
            printf("[fh] %s: dumping %s file...\n", cmdName, argv[i]);
            save_location_data();
        } else if (strcmp(argv[i], "planets") == 0) {
            printf("[fh] %s: loading %s file...\n", cmdName, argv[i]);
            get_planet_data();
            printf("[fh] %s: dumping %s file...\n", cmdName, argv[i]);
            save_planet_data();
        } else if (strcmp(argv[i], "species") == 0) {
            printf("[fh] %s: loading %s file...\n", cmdName, argv[i]);
            get_species_data();
            printf("[fh] %s: dumping %s file...\n", cmdName, argv[i]);
            save_species_data();
        } else if (strcmp(argv[i], "stars") == 0) {
            printf("[fh] %s: loading %s file...\n", cmdName, argv[i]);
            get_star_data();
            printf("[fh] %s: dumping %s file...\n", cmdName, argv[i]);
            save_star_data();
        } else {
            fprintf(stderr, "[fh] %s: unknown option '%s'\n", argv[i]);
            exit(2);
        }
    }
}

