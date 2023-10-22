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
#include "galaxy.h"
#include "galaxyio.h"
#include "cJSON.h"
#include "json.h"


struct galaxy_data galaxy;


static binary_galaxy_data_t galaxyData;

void galaxyDataAsSexpr(FILE *fp) {
    fprintf(fp, "(galaxy (turn %13d)\n        (num_species %6d)\n        (d_num_species %4d)\n        (radius %11d))\n",
            galaxy.turn_number, galaxy.num_species, galaxy.d_num_species, galaxy.radius);
}

struct galaxy_data *galaxyDataFromJson(cJSON *json) {
    if (json == 0) {
        fprintf(stderr, "galaxy: missing json data\n");
        return 0;
    }
    struct galaxy_data *g = malloc(sizeof(struct galaxy_data));
    if (g == 0) {
        perror("galaxyDataFromJson");
        exit(2);
    }
    g->turn_number = galaxy.turn_number;
    g->num_species = galaxy.num_species;
    g->d_num_species = galaxy.num_species;
    g->radius = galaxy.radius;

    cJSON *turn = cJSON_GetObjectItem(json, "turn");
    if (turn == 0 || !cJSON_IsNumber(turn)) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "galaxyDataFromJson: turn: error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "galaxyDataFromJson: turn: expected number\n");
        }
        return 0;
    } else {
        g->turn_number = turn->valueint;
    }
    cJSON *numSpecies = cJSON_GetObjectItem(json, "num_species");
    if (numSpecies == 0 || !cJSON_IsNumber(numSpecies)) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "galaxyDataFromJson: num_species: error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "galaxyDataFromJson: num_species: expected number\n");
        }
        return 0;
    } else {
        g->num_species = numSpecies->valueint;
    }
    cJSON *dNumSpecies = cJSON_GetObjectItem(json, "d_num_species");
    if (dNumSpecies == 0 || !cJSON_IsNumber(dNumSpecies)) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "galaxyDataFromJson: d_num_species: error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "galaxyDataFromJson: d_num_species: expected number\n");
        }
        return 0;
    } else {
        g->d_num_species = dNumSpecies->valueint;
    }
    cJSON *radius = cJSON_GetObjectItem(json, "radius");
    if (radius == 0 || !cJSON_IsNumber(radius)) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "galaxyDataFromJson: radius: error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "galaxyDataFromJson: radius: expected number\n");
        }
        return 0;
    } else {
        g->radius = radius->valueint;
    }

    return g;
}

void get_galaxy_data(void) {
    FILE *fp = fopen("galaxy.dat", "rb");
    if (fp == NULL) {
        fprintf(stderr, "\n\tCannot open file galaxy.dat!\n");
        exit(-1);
    }
    if (fread(&galaxyData, sizeof(galaxyData), 1, fp) != 1) {
        fprintf(stderr, "\n\tCannot read data in file 'galaxy.dat'!\n\n");
        exit(-1);
    }
    galaxy.turn_number = galaxyData.turn_number;
    galaxy.num_species = galaxyData.num_species;
    galaxy.d_num_species = galaxyData.d_num_species;
    galaxy.radius = galaxyData.radius;
    fclose(fp);
}

void save_galaxy_data(galaxy_data_t *gd) {
    FILE *fp = fopen("galaxy.dat", "wb");
    if (fp == NULL) {
        perror("save_galaxy_data");
        fprintf(stderr, "\n\tCannot create new version of file 'galaxy.dat'!\n");
        exit(-1);
    }
    // galaxyData.turn_number = galaxy.turn_number;
    // galaxyData.num_species = galaxy.num_species;
    // galaxyData.d_num_species = galaxy.d_num_species;
    // galaxyData.radius = galaxy.radius;
    if (fwrite(gd, sizeof(galaxy_data_t), 1, fp) != 1) {
        perror("save_galaxy_data");
        fprintf(stderr, "\n\tCannot write data to file 'galaxy.dat'!\n\n");
        exit(-1);
    }
    fclose(fp);
}

cJSON *galaxyDataToJson(galaxy_data_t *gd) {
    char *objName = "galaxy";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "turn_number", gd->turn_number);
    jsonAddIntToObj(obj, objName, "num_species", gd->num_species);
    jsonAddIntToObj(obj, objName, "d_num_species", gd->d_num_species);
    jsonAddIntToObj(obj, objName, "radius", gd->radius);
    return obj;
}

