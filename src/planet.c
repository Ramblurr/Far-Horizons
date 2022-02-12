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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "engine.h"
#include "planet.h"

int32_t num_planets;
int planet_data_modified;

struct planet_data *planet_base;
struct planet_data *planet;

void get_planet_data(void) {
    /* Open planet file. */
    FILE *fp = fopen("planets.dat", "rb");
    if (fp == NULL) {
        fprintf(stderr, "\n\tCannot open file planets.dat!\n");
        exit(-1);
    }
    /* Read header data. */
    if (fread(&num_planets, sizeof(num_planets), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read num_planets in file 'planets.dat'!\n\n");
        exit(-1);
    }
    /* Allocate enough memory for all planets. */
    planet_base = (struct planet_data *) calloc(num_planets + NUM_EXTRA_PLANETS, sizeof(struct planet_data));
    if (planet_base == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n\n");
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(planet_base, sizeof(struct planet_data), num_planets, fp) != num_planets) {
        fprintf(stderr, "\nCannot read planet file into memory!\n\n");
        exit(-1);
    }
    fclose(fp);
    planet_data_modified = FALSE;
}

// planetDataAsSExpr writes the current planet_base array to a text file as an s-expression.
void planetDataAsSExpr(FILE *fp) {
    fprintf(fp, "(planets");
    for (int i = 0; i < num_planets; i++) {
        planet_data_t *p = &planet_base[i];
        fprintf(fp,
                "\n  (planet (id %5d) (diameter %3d) (gravity %3d) (temperature_class %3d) (pressure_class %3d) (special %2d) (gases (%2d %3d) (%2d %3d) (%2d %3d) (%2d %3d)) (mining_difficulty %3d %3d) (econ_efficiency %3d) (message %d))",
                i + 1, p->diameter, p->gravity, p->temperature_class, p->pressure_class, p->special, p->gas[0],
                p->gas_percent[0], p->gas[1], p->gas_percent[1], p->gas[2], p->gas_percent[2], p->gas[3],
                p->gas_percent[3], p->mining_difficulty, p->md_increase, p->econ_efficiency, p->message);
    }
    fprintf(fp, ")\n");
}

void save_planet_data(void) {
    /* Open planet file for writing. */
    FILE *fp = fopen("planets.dat", "wb");
    if (fp == NULL) {
        perror("save_planet_data");
        fprintf(stderr, "\n\tCannot create file 'planets.dat'!\n");
        exit(-1);
    }
    /* Write header data. */
    if (fwrite(&num_planets, sizeof(num_planets), 1, fp) != 1) {
        perror("save_planet_data");
        fprintf(stderr, "\n\tCannot write num_planets to file 'planets.dat'!\n\n");
        exit(-1);
    }
    /* Write planet data to disk. */
    if (fwrite(planet_base, sizeof(struct planet_data), num_planets, fp) != num_planets) {
        perror("save_planet_data");
        fprintf(stderr, "\nCannot write planet data to disk!\n\n");
        exit(-1);
    }
    fclose(fp);
}
