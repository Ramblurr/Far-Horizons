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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "galaxy.h"
#include "engine.h"
#include "enginevars.h"
#include "galaxyio.h"
#include "planetio.h"
#include "stario.h"

int createGalaxyCommand(int argc, char *argv[]) {
    int lessCrowded = FALSE;
    int suggestValues = FALSE;

    long d;
    long d_num_species = 0;
    long desired_num_stars = 0;
    long galactic_radius = 0;
    long i;
    long max_planets = 0;
    long n;
    //long num_stars;
    //long num_planets;
    long num_wormholes = 0;
    struct planet_data *planet;
    //struct planet_data *planet_base;
    struct star_data *star;
    //struct star_data *star_base;
    long star_color;
    long star_here[MAX_DIAMETER][MAX_DIAMETER];
    long star_num_planets;
    long star_size;
    long star_type;
    struct star_data *worm_star;

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
                    "fh: usage: create-galaxy --species=integer [--stars=integer] [--radius=integer] [--suggest-values]\n");
        } else if (strcmp(opt, "--less-crowded") == 0) {
            lessCrowded = TRUE;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            galactic_radius = atoi(val);
        } else if (strcmp(opt, "--species") == 0 && val != NULL) {
            d_num_species = atoi(val);
        } else if (strcmp(opt, "--stars") == 0 && val != NULL) {
            d_num_species = atoi(val);
        } else if (strcmp(opt, "--stars") == 0 && val != NULL) {
            desired_num_stars = atoi(val);
        } else if (strcmp(opt, "--suggest-values") == 0) {
            suggestValues = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    int adjustedNumSpecies = d_num_species;
    if (lessCrowded != FALSE) {
        adjustedNumSpecies = (d_num_species * 3) / 2;
    }
    int numStars = (adjustedNumSpecies * STANDARD_NUMBER_OF_STAR_SYSTEMS) / STANDARD_NUMBER_OF_SPECIES;
    if (desired_num_stars == 0) {
        desired_num_stars = numStars;
    }

    long density = STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS /
                   STANDARD_NUMBER_OF_STAR_SYSTEMS;
    long radius;
    for (radius = MIN_RADIUS; radius * radius * radius < (desired_num_stars * density);) {
        radius++;
    }
    if (galactic_radius == 0) {
        galactic_radius = radius;
    }
    /* Get the number of cubic parsecs within a sphere with a radius of galactic_radius parsecs.
     * Again, use long values to prevent loss of data by compilers that use 16-bit ints. */
    long galactic_diameter = 2 * galactic_radius;
    long galactic_volume = (4 * 314 * galactic_radius * galactic_radius * galactic_radius) / 300;

    if (suggestValues == TRUE) {
        printf(" info: for %d species, a %sgalaxy needs about %d star systems.\n",
               d_num_species, lessCrowded == FALSE ? "" : "less crowded ", numStars);
        printf(" info: for %d stars, the galaxy should have a radius of about %d parsecs.\n",
               desired_num_stars, radius);
        return 0;
    }

    printf(" info: radius      %6d\n", galactic_radius);
    printf(" info: species     %6d\n", d_num_species);
    printf(" info: stars       %6d\n", desired_num_stars);
    printf(" info: lessCrowded %6s\n", lessCrowded == FALSE ? "false" : "true");

    if (d_num_species < MIN_SPECIES || d_num_species > MAX_SPECIES) {
        fprintf(stderr, "error: galaxy must have between %d and %d species\n", MIN_SPECIES, MAX_SPECIES);
        return 2;
    }
    if (desired_num_stars < MIN_STARS || desired_num_stars > MAX_STARS) {
        fprintf(stderr, "error: galaxy must have between %d and %d star systems\n", MIN_STARS, MAX_STARS);
        return 2;
    }
    if (galactic_radius < MIN_RADIUS || galactic_radius > MAX_RADIUS) {
        fprintf(stderr, "error: radius must be between %d and %d parsecs\n", MIN_RADIUS, MAX_RADIUS);
        return 2;
    }

    /* The probability of a star system existing at any particular set of x,y,z coordinates is one in chance_of_star. */
    long chance_of_star = galactic_volume / desired_num_stars;
    if (chance_of_star < 50) {
        fprintf(stderr, "error: galactic radius is too small for %d stars\n", desired_num_stars);
        fprintf(stderr, "       galactic_volume   == %6d\n", galactic_volume);
        fprintf(stderr, "       desired_num_stars == %6d\n", desired_num_stars);
        fprintf(stderr, "       chance_of_star    == %6d\n", chance_of_star);
        return 2;
    } else if (chance_of_star > 3200) {
        fprintf(stderr, "error: galactic radius is too large for %d stars\n", desired_num_stars);
        fprintf(stderr, "       galactic_volume   == %6d\n", galactic_volume);
        fprintf(stderr, "       desired_num_stars == %6d\n", desired_num_stars);
        fprintf(stderr, "       chance_of_star    == %6d\n", chance_of_star);
        return 2;
    }

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) {
        rnd(10);
    }

    /* Initialize star location data.
     * Set the z-coordinate to -1 as a flag that there's no star at the location. */
    for (int x = 0; x < galactic_diameter; x++) {
        for (int y = 0; y < galactic_diameter; y++) {
            star_here[x][y] = -1; /* z-coordinate */
        }
    }

    // randomly assign stars to locations within the galactic cluster
    for (num_stars = 0; num_stars < desired_num_stars;) {
        int x = rnd(galactic_diameter) - 1;
        int y = rnd(galactic_diameter) - 1;
        int z = rnd(galactic_diameter) - 1;

        int real_x = x - galactic_radius;
        int real_y = y - galactic_radius;
        int real_z = z - galactic_radius;

        // check that the coordinate is within the galactic boundary.
        int sq_distance_from_center = (real_x * real_x) + (real_y * real_y) + (real_z * real_z);
        if (sq_distance_from_center >= galactic_radius * galactic_radius) {
            continue;
        }

        // create a new star here if there's not already a star here.
        if (star_here[x][y] == -1) {
            star_here[x][y] = z;    /* z-coordinate. */
            num_stars++;
            if (num_stars == MAX_STARS) {
                break;
            }
        }
    }

    galaxy.d_num_species = d_num_species;
    galaxy.num_species = 0;
    galaxy.radius = galactic_radius;
    galaxy.turn_number = 0;

    /* Allocate enough memory for star and planet data. */
    star_base = (struct star_data *) calloc(num_stars, sizeof(struct star_data));
    if (star_base == NULL) {
        fprintf(stderr, "error: cannot allocate enough memory for star data!\n");
        return 2;
    }

    max_planets = 9 * num_stars;    /* Maximum number possible. */
    planet_base = (struct planet_data *) calloc(max_planets, sizeof(struct planet_data));
    if (planet_base == NULL) {
        fprintf(stderr, "error: cannot allocate enough memory for planet data!\n");
        exit(-1);
    }

    // printing to stdout for backspace trick later...
    fprintf(stdout, "\nGenerating star number     ");
    fflush(stdout);

    int pl_index = 0;
    int st_index = 0;
    planet = planet_base;
    for (int x = 0; x < galactic_diameter; x++) {
        for (int y = 0; y < galactic_diameter; y++) {
            if (star_here[x][y] == -1) {
                // no star at this location
                continue;
            }

            star = star_base + st_index;
            memset(star, 0, sizeof(struct star_data));

            /* Set coordinates. */
            star->x = x;
            star->y = y;
            star->z = star_here[x][y];

            /* Determine type of star. Make MAIN_SEQUENCE the most common star type. */
            star_type = rnd(GIANT + 6);
            if (star_type > GIANT) {
                star_type = MAIN_SEQUENCE;
            }
            star->type = star_type;

            /* Color and size of star are totally random. */
            star_color = rnd(RED);
            star->color = star_color;
            star_size = rnd(10) - 1;
            star->size = star_size;

            /* Determine the number of planets in orbit around the star.
             * The algorithm is something I tweaked until I liked it.
             * It's weird, but it works. */
            d = RED + 2 - star_color;
            /* Size of die.
             * Big stars (blue, blue-white) roll bigger dice. Smaller stars (orange, red) roll smaller dice. */
            n = star_type;
            if (n > 2) {
                n -= 1;
            }
            /* Number of rolls:
             *  - dwarves have 1 roll
             *  - degenerates and main sequence stars have 2 rolls
             *  - giants have 3 rolls. */
            star_num_planets = -2;
            for (i = 1; i <= n; i++) {
                star_num_planets += rnd(d);
            }
            /* Trim down if too many. */
            for (; star_num_planets > 9;) {
                star_num_planets -= rnd(3);
            }
            if (star_num_planets < 1) {
                star_num_planets = 1;
            }
            star->num_planets = star_num_planets;

            /* Determine pl_index of first planet in file "planets.dat". */
            star->planet_index = pl_index;

            /* Generate planets and write to file "planets.dat". */
            star_data_t *current_star = star;
            generate_planets(planet, star_num_planets);

            star->home_system = FALSE;
            star->worm_here = FALSE;

            /* Update pointers and indices. */
            pl_index += star_num_planets;
            planet += star_num_planets;

            st_index++;
            if (st_index % 10 == 0) {
                fprintf(stdout, "\b\b\b\b%4d", st_index);
            }
            fflush(stdout);
        }
    }

    fprintf(stdout, "\b\b\b\b%4d\n", st_index);
    fflush(stdout);

    /* Allocate natural wormholes. */
    num_wormholes = 0;
    for (i = 0; i < num_stars; i++) {
        star = star_base + i;
        if (star->home_system) {
            continue;
        } else if (star->worm_here) {
            continue;
        } else if (rnd(100) < 92) {
            continue;
        }

        /* There is a wormhole here. Get coordinates of other end. */
        for (;;) {
            n = rnd(num_stars);
            worm_star = star_base + n - 1;
            if (worm_star == star) {
                continue;
            } else if (worm_star->home_system) {
                continue;
            } else if (worm_star->worm_here) {
                continue;
            }
            break;
        }

        /* Eliminate wormholes less than 20 parsecs in length. */
        long dx = star->x - worm_star->x;
        long dy = star->y - worm_star->y;
        long dz = star->z - worm_star->z;
        if ((dx * dx) + (dy * dy) + (dz * dz) < 400) {
            continue;
        }

        star->worm_here = TRUE;
        star->worm_x = worm_star->x;
        star->worm_y = worm_star->y;
        star->worm_z = worm_star->z;

        worm_star->worm_here = TRUE;
        worm_star->worm_x = star->x;
        worm_star->worm_y = star->y;
        worm_star->worm_z = star->z;

        num_wormholes++;
    }

    if (st_index != num_stars) {
        fprintf(stderr, "error: internal consistency check #1 failed!!!\n");
        return 2;
    }

    num_planets = pl_index;
    printf(" info: this galaxy contains a total of %d stars and %d planets.\n", num_stars, num_planets);
    printf("       the galaxy contains %d natural wormholes.\n", num_wormholes);

    // save data
    save_galaxy_data();
    FILE *fp = fopen("galaxy.txt", "wb");
    if (fp == NULL) {
        perror("fh: export: sexpr:");
        fprintf(stderr, "\n\tCannot create new version of file 'galaxy.txt'!\n");
        return 2;
    }
    galaxyDataAsSexpr(fp);
    fclose(fp);

    save_star_data();
    fp = fopen("stars.txt", "wb");
    if (fp == NULL) {
        perror("fh: export: sexpr:");
        fprintf(stderr, "\n\tCannot create new version of file 'stars.txt'!\n");
        return 2;
    }
    starDataAsSexpr(fp);
    fclose(fp);

    save_planet_data();
    fp = fopen("planets.txt", "wb");
    if (fp == NULL) {
        perror("fh: export: sexpr:");
        fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
        return 2;
    }
    planetDataAsSExpr(fp);
    fclose(fp);

    free(star_base);
    free(planet_base);

    return 0;
}