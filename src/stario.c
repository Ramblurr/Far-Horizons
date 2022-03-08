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
#include "galaxy.h"
#include "galaxyio.h"
#include "star.h"
#include "stario.h"


int num_stars;

struct star_data *star_base;

int star_data_modified;


typedef struct {
    uint8_t x;              /* Coordinates. */
    uint8_t y;
    uint8_t z;
    uint8_t type;           /* Dwarf, degenerate, main sequence or giant. */
    uint8_t color;          /* Star color. Blue, blue-white, etc. */
    uint8_t size;           /* Star size, from 0 thru 9 inclusive. */
    uint8_t num_planets;    /* Number of usable planets in star system. */
    uint8_t home_system;    /* TRUE if this is a good potential home system. */
    uint8_t worm_here;      /* TRUE if wormhole entry/exit. */
    uint8_t worm_x;         /* Coordinates of wormhole exit (if there is one) */
    uint8_t worm_y;
    uint8_t worm_z;
    int16_t reserved1;      /* Reserved for future use. Zero for now. */
    int16_t reserved2;      /* Reserved for future use. Zero for now. */
    int16_t planet_index;   /* Index (starting at zero) into the file "planets.dat" of the first planet in the star system. */
    int32_t message;        /* Message associated with this star system, if any. */
    uint32_t visited_by[NUM_CONTACT_WORDS]; /* A bit is set if corresponding species has been here. */
    int32_t reserved3;      /* Reserved for future use. Zero for now. */
    int32_t reserved4;      /* Reserved for future use. Zero for now. */
    int32_t reserved5;      /* Reserved for future use. Zero for now. */
} binary_data_t;


void get_star_data(void) {
    int32_t numStars;
    binary_data_t *starData;

    /* Open star file. */
    FILE *fp = fopen("stars.dat", "rb");
    if (fp == NULL) {
        perror("get_star_data");
        fprintf(stderr, "\n\tCannot open file stars.dat!\n");
        exit(-1);
    }
    /* Read header data. */
    if (fread(&numStars, sizeof(numStars), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read num_stars in file 'stars.dat'!\n\n");
        exit(-1);
    }

    /* Allocate enough memory for all stars plus maybe an extra few. */
    starData = (binary_data_t *) calloc(numStars, sizeof(binary_data_t));
    if (starData == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for star file!\n");
        fprintf(stderr, "\n\tattempted to allocate %d star entries\n\n", numStars);
        exit(-1);
    }
    num_stars = numStars;
    star_base = (struct star_data *) calloc(num_stars + NUM_EXTRA_STARS, sizeof(struct star_data));
    if (star_base == NULL) {
        perror("get_star_data");
        fprintf(stderr, "\nCannot allocate enough memory for star file!\n\n");
        exit(-1);
    }

    /* Read it all into memory. */
    if (fread(starData, sizeof(binary_data_t), numStars, fp) != numStars) {
        fprintf(stderr, "\nCannot read star file into memory!\n\n");
        exit(-1);
    }
    fclose(fp);

    // translate the data
    for (int i = 0; i < num_stars; i++) {
        struct star_data *s = &star_base[i];
        binary_data_t *data = &starData[i];
        s->x = data->x;
        s->y = data->y;
        s->z = data->z;
        s->type = data->type;
        s->color = data->color;
        s->size = data->size;
        s->num_planets = data->num_planets;
        s->home_system = data->home_system;
        s->worm_here = data->worm_here;
        s->worm_x = data->worm_x;
        s->worm_y = data->worm_y;
        s->worm_z = data->worm_z;
        s->planet_index = data->planet_index;
        s->message = data->message;
        for (int j = 0; j < NUM_CONTACT_WORDS; j++) {
            s->visited_by[j] = data->visited_by[j];
        }
        // mdhender: added to help clean up code
        s->id = i + 1;
        s->index = i;
    }
    star_data_modified = FALSE;

    free(starData);
}


void save_star_data(void) {
    // open star file for writing
    FILE *fp = fopen("stars.dat", "wb");
    if (fp == NULL) {
        perror("save_star_data");
        fprintf(stderr, "\n\tCannot create file 'stars.dat'!\n");
        exit(-1);
    }
    saveStarData(star_base, num_stars, fp);
    fclose(fp);

    star_data_modified = FALSE;
}


//     star_data_t *starBase = star_base;
//     int32_t numStars = num_stars;
// caller should update `star_data_modified = FALSE` if they care to.
void saveStarData(star_data_t *starBase, int numStars, FILE *fp) {
    if (fp == NULL) {
        fprintf(stderr, "error: saveStarData: internal error: passed null file pointer\n");
        exit(2);
    }
    binary_data_t *starData = (binary_data_t *) calloc(numStars, sizeof(binary_data_t));
    if (starData == NULL) {
        perror("saveStarData:");
        fprintf(stderr, "error: cannot allocate enough memory to convert stars data\n");
        fprintf(stderr, "       attempted to allocate %d star entries\n", numStars);
        exit(2);
    }

    // translate the data
    for (int i = 0; i < numStars; i++) {
        struct star_data *s = &starBase[i];
        binary_data_t *data = &starData[i];
        data->x = s->x;
        data->y = s->y;
        data->z = s->z;
        data->type = s->type;
        data->color = s->color;
        data->size = s->size;
        data->num_planets = s->num_planets;
        data->home_system = s->home_system;
        data->worm_here = s->worm_here;
        data->worm_x = s->worm_x;
        data->worm_y = s->worm_y;
        data->worm_z = s->worm_z;
        data->planet_index = s->planet_index;
        data->message = s->message;
        for (int j = 0; j < NUM_CONTACT_WORDS; j++) {
            data->visited_by[j] = s->visited_by[j];
        }
    }

    // write header data
    int32_t numOfElements = numStars;
    if (fwrite(&numOfElements, sizeof(numOfElements), 1, fp) != 1) {
        perror("saveStarData:");
        fprintf(stderr, "error: cannot write stars header to file\n");
        exit(2);
    }
    // write records
    if (fwrite(starData, sizeof(binary_data_t), numOfElements, fp) != numOfElements) {
        perror("saveStarData:");
        fprintf(stderr, "error: cannot write stars data to file\n");
        exit(2);
    }

    // we no longer do this; the caller is responsible
    // star_data_modified = FALSE;

    free(starData);
}


void starDataAsSExpr(star_data_t *starBase, int numStars, FILE *fp) {
    fprintf(fp, "(stars");
    for (int i = 0; i < numStars; i++) {
        struct star_data *s = &starBase[i];
        fprintf(fp, "\n  (star (id %4d) (x %3d) (y %3d) (z %3d) (type '%c') (color '%c') (size '%c')",
                i + 1, s->x, s->y, s->z,
                star_type(s->type), star_color(s->color), star_size(s->size));
        fprintf(fp, "\n        (planets");
        for (int p = 0; p < s->num_planets; p++) {
            fprintf(fp, " %4d", s->planet_index + p + 1);
        }
        fprintf(fp, ") (home_system %s)", s->home_system ? "true" : "false");
        fprintf(fp, "\n        (wormhole (here %-5s) (exit_x %3d) (exit_y %3d) (exit_z %3d))",
                s->worm_here ? "true" : "false", s->worm_x, s->worm_y, s->worm_z);
        fprintf(fp, "\n        (visited_by");
        for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
            // write the species only if it has visited this system
            if ((s->visited_by[spidx / 32] & (1 << (spidx % 32))) != 0) {
                fprintf(fp, " %3d", spidx + 1);
            }
        }
        fprintf(fp, ")");
        fprintf(fp, "\n        (message %d))", s->message);
    }
    fprintf(fp, ")\n");
}

