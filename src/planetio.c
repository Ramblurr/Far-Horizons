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
#include "data.h"
#include "engine.h"
#include "planet.h"
#include "planetio.h"
#include "stario.h"

int num_planets;

struct planet_data *planet_base;

int planet_data_modified;


void get_planet_data(void) {
    int32_t numPlanets;
    binary_planet_data_t *planetData;

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
    planetData = (binary_planet_data_t *) ncalloc(__FUNCTION__, __LINE__, numPlanets + NUM_EXTRA_PLANETS, sizeof(binary_planet_data_t));
    if (planetData == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d planet entries\n\n", numPlanets, NUM_EXTRA_PLANETS);
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(planetData, sizeof(binary_planet_data_t), numPlanets, fp) != numPlanets) {
        fprintf(stderr, "\nCannot read planet file into memory!\n\n");
        exit(-1);
    }
    fclose(fp);

    num_planets = numPlanets;
    /* Allocate enough memory for all planets. */
    planet_base = (struct planet_data *) ncalloc(__FUNCTION__, __LINE__, num_planets + NUM_EXTRA_PLANETS, sizeof(struct planet_data));
    if (planet_base == NULL) {
        perror("get_planet_data");
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n\n");
        exit(-1);
    }

    for (int i = 0; i < num_planets; i++) {
        struct planet_data *p = &planet_base[i];
        binary_planet_data_t *pd = &planetData[i];
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

        // mdhender: added fields to help clean up code
        p->id = i + 1;
        p->index = i;
    }

    // mdhender: added fields to help clean up code
    for (int sn = 0; sn < num_stars; sn++) {
        star_data_t *star = star_base + sn;
        for (int pn = 0; pn < star->num_planets; pn++) {
            struct planet_data *p = &planet_base[star->planet_index + pn];
            p->star = star;
            p->orbit = pn + 1;
        }
    }

    planet_data_modified = FALSE;

    free(planetData);
}


// getPlanetData returns the planet data
planet_data_t *getPlanetData(int extraRecords, const char *filename) {
    // open binary input file
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("getPlanetData");
        fprintf(stderr, "\n\tCannot open file '%s'!\n", filename);
        exit(-1);
    }

    // read header data, which is just the number of records in the file
    int32_t numRecords;
    if (fread(&numRecords, sizeof(numRecords), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read num_planets in file '%s'!\n\n", filename);
        exit(-1);
    }

    // allocate enough memory for all records
    binary_planet_data_t *rawRecords = (binary_planet_data_t *) ncalloc(__FUNCTION__, __LINE__, numRecords, sizeof(binary_planet_data_t));
    if (rawRecords == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file '%s'!\n", filename);
        exit(-1);
    }

    // read all records into memory
    if (fread(rawRecords, sizeof(binary_planet_data_t), numRecords, fp) != numRecords) {
        fprintf(stderr, "\nCannot read planet file '%s' into memory!\n\n", filename);
        exit(-1);
    }
    fclose(fp);

    // allocate memory for the translated records plus extra records plus the sentinel record
    if (extraRecords < 0) {
        extraRecords = 0;
    }
    planet_data_t *planetBase = (planet_data_t *) ncalloc(__FUNCTION__, __LINE__, numRecords + extraRecords + 1, sizeof(planet_data_t));
    if (planet_base == NULL) {
        perror("get_planet_data");
        fprintf(stderr, "\nCannot allocate enough memory for planet file '%s'!\n\n", filename);
        fprintf(stderr, "\n\tattempted to allocate %d + %d planet entries\n\n", num_planets, extraRecords);
        exit(-1);
    }

    // translate from the raw input record into the application record
    for (int i = 0; i < numRecords; i++) {
        struct planet_data *p = &planetBase[i];
        binary_planet_data_t *pd = &rawRecords[i];
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
        p->isValid = FALSE;
    }

    free(rawRecords);

    return planetBase;
}


// planetDataAsJson writes the current planet_base array to a text file as JSON.
void planetDataAsJson(int numPlanets, planet_data_t *planetBase, FILE *fp) {
    const char *sep = "";
    fprintf(fp, "[");
    for (int i = 0; i < numPlanets; i++) {
        planet_data_t *p = &planetBase[i];
        fprintf(fp, "%s\n  {\"id\": %d,", sep, i + 1);
        fprintf(fp, "\n   \"diameter\": %d,", p->diameter);
        fprintf(fp, "\n   \"gravity\": %d,", p->gravity);
        fprintf(fp, "\n   \"temperature_class\": %d,", p->temperature_class);
        fprintf(fp, "\n   \"pressure_class\": %d,", p->pressure_class);
        fprintf(fp, "\n   \"special\": %d,", p->special);
        fprintf(fp, "\n   \"gases\": [");
        for (int j = 0; j < 4; j++) {
            if (j != 0) {
                fprintf(fp, ", ");
            }
            fprintf(fp, "{\":code\": %d, \":percent\": %d}", p->gas[j], p->gas_percent[j]);
        }
        fprintf(fp, "],");
        fprintf(fp, "\n   \"mining_difficulty\": {\"base\": %d, \"increase\": %d},",
                p->mining_difficulty, p->md_increase);
        fprintf(fp, "\n   \"econ_efficiency\": %d,", p->econ_efficiency);
        fprintf(fp, "\n   \"message\": %d}", p->message);
        sep = ",";
    }
    fprintf(fp, "\n]\n");
}


// planetDataAsSExpr writes the current planet_base array to a text file as an s-expression.
void planetDataAsSExpr(planet_data_t *planetBase, int numPlanets, FILE *fp) {
    fprintf(fp, "(planets");
    for (int i = 0; i < numPlanets; i++) {
        planet_data_t *p = &planetBase[i];
        fprintf(fp,
                "\n  (planet (id %5d) (diameter %3d) (gravity %2d.%02d) (temperature_class %3d) (pressure_class %3d) (special %2d) (gases (%2d %3d) (%2d %3d) (%2d %3d) (%2d %3d)) (mining_difficulty %3d.%02d %3d) (econ_efficiency %3d) (message %d))",
                i + 1,
                p->diameter,
                p->gravity / 100, p->gravity % 100,
                p->temperature_class, p->pressure_class, p->special,
                p->gas[0], p->gas_percent[0],
                p->gas[1], p->gas_percent[1],
                p->gas[2], p->gas_percent[2],
                p->gas[3], p->gas_percent[3],
                p->mining_difficulty / 100, p->mining_difficulty % 100,
                p->md_increase, p->econ_efficiency, p->message);
    }
    fprintf(fp, ")\n");
}


void save_planet_data(void) {
    FILE *fp;
    int32_t numPlanets = num_planets;
    binary_planet_data_t *planetData = (binary_planet_data_t *) ncalloc(__FUNCTION__, __LINE__, numPlanets, sizeof(binary_planet_data_t));
    if (planetData == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for planet file!\n");
        fprintf(stderr, "\n\tattempted to allocate %d planet entries\n\n", numPlanets);
        exit(-1);
    }
    for (int i = 0; i < num_planets; i++) {
        struct planet_data *p = &planet_base[i];
        binary_planet_data_t *pd = &planetData[i];
        pd->temperature_class = p->temperature_class;
        pd->pressure_class = p->pressure_class;
        pd->special = p->special;
        for (int g = 0; g < 4; g++) {
            pd->gas[g] = p->gas[g];
            pd->gas_percent[g] = p->gas_percent[g];
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
    if (fwrite(planetData, sizeof(binary_planet_data_t), numPlanets, fp) != numPlanets) {
        perror("save_planet_data");
        fprintf(stderr, "\nCannot write planet data to disk!\n\n");
        exit(-1);
    }
    fclose(fp);

    planet_data_modified = FALSE;

    free(planetData);
}


void savePlanetData(planet_data_t *planetBase, int numPlanets, const char *filename) {
    int32_t numRecords = numPlanets;
    binary_planet_data_t *planetData = (binary_planet_data_t *) ncalloc(__FUNCTION__, __LINE__, numRecords, sizeof(binary_planet_data_t));
    if (planetData == NULL) {
        fprintf(stderr, "error: cannot allocate enough memory for planet file '%s'!\n", filename);
        fprintf(stderr, "       attempted to allocate %d planet records\n", numRecords);
        exit(2);
    }
    for (int i = 0; i < numPlanets; i++) {
        struct planet_data *p = &planetBase[i];
        binary_planet_data_t *pd = &planetData[i];
        pd->temperature_class = p->temperature_class;
        pd->pressure_class = p->pressure_class;
        pd->special = p->special;
        for (int g = 0; g < 4; g++) {
            pd->gas[g] = p->gas[g];
            pd->gas_percent[g] = p->gas_percent[g];
        }
        pd->diameter = p->diameter;
        pd->gravity = p->gravity;
        pd->mining_difficulty = p->mining_difficulty;
        pd->econ_efficiency = p->econ_efficiency;
        pd->md_increase = p->md_increase;
        pd->message = p->message;
    }

    /* Open planet file for writing. */
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("savePlanetData");
        fprintf(stderr, "error: cannot create file '%s'!\n", filename);
        exit(2);
    }
    /* Write header data. */
    if (fwrite(&numRecords, sizeof(numRecords), 1, fp) != 1) {
        perror("savePlanetData");
        fprintf(stderr, "error: cannot write numPlanets to file '%s'!\n", filename);
        exit(2);
    }
    /* Write planet data to disk. */
    if (fwrite(planetData, sizeof(binary_planet_data_t), numRecords, fp) != numRecords) {
        perror("savePlanetData");
        fprintf(stderr, "error: cannot write planet data to file '%s'!\n", filename);
        exit(2);
    }
    fclose(fp);
    free(planetData);
}

