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
#include <stdint.h>
#include <stdio.h>
#include "engine.h"
#include "planetio.h"

int num_planets;

struct planet_data *planet_base;

int planet_data_modified;


typedef struct {
    uint8_t temperature_class;  /* Temperature class, 1-30. */
    uint8_t pressure_class;     /* Pressure class, 0-29. */
    uint8_t special;            /* 0 = not special, 1 = ideal home planet, 2 = ideal colony planet, 3 = radioactive hellhole. */
    uint8_t reserved1;          /* Reserved for future use. Zero for now. */
    uint8_t gas[4];             /* Gas in atmosphere. Zero if none. */
    uint8_t gas_percent[4];     /* Percentage of gas in atmosphere. */
    int16_t reserved2;          /* Reserved for future use. Zero for now. */
    int16_t diameter;           /* Diameter in thousands of kilometers. */
    int16_t gravity;            /* Surface gravity. Multiple of Earth gravity times 100. */
    int16_t mining_difficulty;  /* Mining difficulty times 100. */
    int16_t econ_efficiency;    /* Economic efficiency. Always 100 for a home planet. */
    int16_t md_increase;        /* Increase in mining difficulty. */
    int32_t message;            /* Message associated with this planet, if any. */
    int32_t reserved3;          /* Reserved for future use. Zero for now. */
    int32_t reserved4;          /* Reserved for future use. Zero for now. */
    int32_t reserved5;          /* Reserved for future use. Zero for now. */
} binary_data_t;


void get_planet_data(void) {
    int32_t numPlanets;
    binary_data_t *planetData;

    /* Open planet file. */
    FILE *fp = fopen("planets.dat", "rb");
    if (fp == NULL) {
        perror("get_planet_data");
        fprintf(stderr, "\n\tCannot open file planets.dat!\n");
        exit(-1);
    }
    /* Read header data. */
    if (fread(&numPlanets, sizeof(numPlanets), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read num_planets in file 'planets.dat'!\n\n");
        exit(-1);
    }
    /* Allocate enough memory for all planets. */
    planetData = (binary_data_t *) calloc(numPlanets + NUM_EXTRA_PLANETS, sizeof(binary_data_t));
    if (planetData == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d planet entries\n\n", numPlanets, NUM_EXTRA_PLANETS);
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(planetData, sizeof(binary_data_t), numPlanets, fp) != numPlanets) {
        fprintf(stderr, "\nCannot read planet file into memory!\n\n");
        exit(-1);
    }
    fclose(fp);

    num_planets = numPlanets;
    /* Allocate enough memory for all planets. */
    planet_base = (struct planet_data *) calloc(num_planets + NUM_EXTRA_PLANETS, sizeof(struct planet_data));
    if (planet_base == NULL) {
        perror("get_planet_data");
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n\n");
        exit(-1);
    }

    for (int i = 0; i < num_planets; i++) {
        struct planet_data *p = &planet_base[i];
        binary_data_t *pd = &planetData[i];
        p->temperature_class = pd->temperature_class;
        p->pressure_class = pd->pressure_class;
        p->special = pd->special;
        for (int g = 0; g < 4; g++) {
            p->gas[g] = pd->gas[g];
            p->gas_percent[g] = pd->gas_percent[g];
        }
        p->diameter = pd->diameter;
        p->gravity = pd->gravity;
        p->mining_difficulty = pd->mining_difficulty;
        p->econ_efficiency = pd->econ_efficiency;
        p->md_increase = pd->md_increase;
        p->message = pd->message;
    }
    planet_data_modified = FALSE;

    free(planetData);
}


// planetDataAsSExpr writes the current planet_base array to a text file as an s-expression.
void planetDataAsSExpr(FILE *fp) {
    fprintf(fp, "(planets");
    for (int i = 0; i < num_planets; i++) {
        planet_data_t *p = &planet_base[i];
        fprintf(fp,
                "\n  (planet (id %5d) (diameter %3d) (gravity %3d) (temperature_class %3d) (pressure_class %3d) (special %2d) (gases (%2d %3d) (%2d %3d) (%2d %3d) (%2d %3d)) (mining_difficulty %3d %3d) (econ_efficiency %3d) (message %d))",
                i + 1,
                p->diameter, p->gravity, p->temperature_class, p->pressure_class, p->special,
                p->gas[0], p->gas_percent[0], p->gas[1], p->gas_percent[1], p->gas[2], p->gas_percent[2],
                p->gas[3], p->gas_percent[3],
                p->mining_difficulty, p->md_increase, p->econ_efficiency, p->message);
    }
    fprintf(fp, ")\n");
}


void save_planet_data(void) {
    FILE *fp;
    int32_t numPlanets = num_planets;
    binary_data_t *planetData = (binary_data_t *) calloc(numPlanets, sizeof(binary_data_t));
    if (planetData == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n");
        fprintf(stderr, "\n\tattempted to allocate %d planet entries\n\n", numPlanets);
        exit(-1);
    }
    for (int i = 0; i < num_planets; i++) {
        struct planet_data *p = &planet_base[i];
        binary_data_t *pd = &planetData[i];
        pd->temperature_class = p->temperature_class;
        pd->pressure_class = p->pressure_class;
        pd->special = p->special;
        for (int g = 0; g < 4; g++) {
            p->gas[g] = p->gas[g];
            p->gas_percent[g] = p->gas_percent[g];
        }
        pd->diameter = p->diameter;
        pd->gravity = p->gravity;
        pd->mining_difficulty = p->mining_difficulty;
        pd->econ_efficiency = p->econ_efficiency;
        pd->md_increase = p->md_increase;
        pd->message = p->message;
    }

    /* Open planet file for writing. */
    fp = fopen("planets.dat", "wb");
    if (fp == NULL) {
        perror("save_planet_data");
        fprintf(stderr, "\n\tCannot create file 'planets.dat'!\n");
        exit(-1);
    }
    /* Write header data. */
    if (fwrite(&numPlanets, sizeof(numPlanets), 1, fp) != 1) {
        perror("save_planet_data");
        fprintf(stderr, "\n\tCannot write num_planets to file 'planets.dat'!\n\n");
        exit(-1);
    }
    /* Write planet data to disk. */
    if (fwrite(planetData, sizeof(binary_data_t), numPlanets, fp) != numPlanets) {
        perror("save_planet_data");
        fprintf(stderr, "\nCannot write planet data to disk!\n\n");
        exit(-1);
    }
    fclose(fp);

    planet_data_modified = FALSE;

    free(planetData);

    fp = fopen("planets.txt", "wb");
    if (fp == NULL) {
        perror("save_planets_data");
        fprintf(stderr, "\n\tCannot create new version of file 'planets.txt'!\n");
        exit(-1);
    }
    planetDataAsSExpr(fp);
    fclose(fp);
}
