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
#include "galaxyio.h"
#include "planetio.h"
#include "show.h"
#include "stario.h"
#include "enginevars.h"


int showCommand(int argc, char *argv[]) {
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
            fprintf(stderr, "usage: show (galaxy | help)\n");
            return 2;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "galaxy") == 0 && val == NULL) {
            return showGalaxyCommand(argc - 1, argv + 1);
        } else if (strcmp(opt, "help") == 0 && val == NULL) {
            return showHelp();
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return 0;
}


int showGalaxyCommand(int argc, char *argv[]) {
    static int star_here[MAX_DIAMETER][MAX_DIAMETER];

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
            fprintf(stderr, "usage: show galaxy [-v]\n");
            fprintf(stderr, "       display a crude ASCII map of the galaxy  with the relative positions\n");
            fprintf(stderr, "       of home planets, ideal colonies, and other star systems. the GM may\n");
            fprintf(stderr, "       run this program after creating a new galaxy to visually confirm the\n");
            fprintf(stderr, "       distribution is not too lopsided.\n");
            fprintf(stderr, "       in the map, 'H' indicates ideal home planet, 'C' ideal colony, and '*'\n");
            fprintf(stderr, "       is used for all other stars.\n");
            return 2;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    /* Get all the raw data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();

    int galactic_diameter = 2 * galaxy.radius;

    /* For each star, set corresponding element of star_here[] to index into star array. */
    for (int x = 0; x < galactic_diameter; x++) {
        /* Initialize array. */
        for (int y = 0; y < galactic_diameter; y++) {
            star_here[x][y] = -1;
        }
    }

    // bug: if multiple systems have the same x and y, only one system's index gets saved
    star_data_t *star = star_base;
    for (int star_index = 0; star_index < num_stars; star_index++) {
        star_here[star->x][star->y] = star_index;
        star++;
    }

    printf("+");
    for (int i = 0; i < galactic_diameter; i++) {
        printf("-");
    }
    printf("+\n");

    /* Outermost loop will control y-coordinates. */
    for (int y = galactic_diameter - 1; y >= 0; y--) {
        /* Innermost loop will control x-coordinate. */
        for (int x = 0; x <= galactic_diameter; x++) {
            if (x == 0) {
                printf("|");
            } else if (x == galactic_diameter) {
                printf("|\n");
                continue;
            }

            int star_index = star_here[x][y];
            if (star_index == -1) {
                printf(" ");
            } else {
                star = star_base + star_index;
                planet_data_t *planet = planet_base + star->planet_index;
                int special = 0;
                for (int i = 0; special == 0 && i < star->num_planets; i++) {
                    if (planet->special != 0) {
                        special = planet->special;
                    }
                    planet++;
                }

                switch (special) {
                    case 0:
                        printf(".");
                        break;
                    case 1:
                        printf("H");
                        break;
                    case 2:
                        printf("C");
                        break;
                    case 3:
                        printf("R");
                        break;
                    default:
                        printf("%d", planet->special);
                }
            }
        }
    }

    printf("+");
    for (int i = 0; i < galactic_diameter; i++) {
        printf("-");
    }
    printf("+\n");

    printf("    H - ideal home planet\n");
    printf("    C - ideal colony\n");
    printf("    . - all other star systems\n");

    return 0;
}


int showHelp(void) {
    printf("usage: fh [option...] command [argument...]\n");
    printf("  opt: --help     show this helpful text and exit\n");
    printf("  opt: --version  display version and exit\n");
    printf("  opt: -t         enable test mode\n");
    printf("  opt: -v         enable verbose mode\n");
    printf("  cmd: turn       display the current turn number\n");
    printf("  cmd: locations  create locations data file and update\n");
    printf("                  economic efficiency in planets data file\n");
    printf("  cmd: combat     run combat commands\n");
    printf("            opt:  --combat   run combat    (default)\n");
    printf("               :  --strike   run strikes\n");
    printf("  cmd: pre-departure\n");
    printf("                  run pre-departure commands\n");
    printf("  cmd: jump       run jump commands\n");
    printf("  cmd: production\n");
    printf("                  run production commands\n");
    printf("  cmd: post-arrival\n");
    printf("                  run post-arrival commands\n");
    printf("  cmd: finish     run end of turn logic\n");
    printf("  cmd: report     create end of turn reports\n");
    printf("  cmd: stats      display statistics\n");
    printf("  cmd: create     create a new galaxy, home system templates\n");
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
}