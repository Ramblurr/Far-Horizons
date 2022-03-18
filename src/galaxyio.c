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
#include "galaxy.h"
#include "galaxyio.h"
#include "json.h"


struct galaxy_data galaxy;


typedef struct {
    int32_t d_num_species;  /* Design number of species in galaxy. */
    int32_t num_species;    /* Actual number of species allocated. */
    int32_t radius;         /* Galactic radius in parsecs. */
    int32_t turn_number;    /* Current turn number. */
} binary_data_t;


static binary_data_t galaxyData;


void galaxyDataAsJson(FILE *fp) {
    fprintf(fp, "{\n  \"turn\": %d,\n  \"num_species\": %d,\n  \"d_num_species\": %d,\n  \"radius\": %d\n}\n",
            galaxy.turn_number, galaxy.num_species, galaxy.d_num_species, galaxy.radius);

//    json_value_t *j = json_new_object();
    json_write(json_box_bool(0), stdout);
    json_write(json_box_integer(1), stdout);
    json_write(json_box_null(), stdout);
    json_write(json_box_number(0.9), stdout);
    json_write(json_box_string("stringer"), stdout);
    json_write(json_box_undefined(), stdout);
    json_write(json_new_array(), stdout);
    json_write(json_append(NULL, json_box_string("whammo 42")), stdout);
    json_write(json_append(json_append(NULL, json_box_bool(1)), json_append(NULL, json_box_string("whammo 42"))), stdout);
    json_write(json_append(json_append(NULL, json_box_bool(1)), json_box_integer(42)), stdout);
    json_write(json_new_object(), stdout);
}


void galaxyDataAsSexpr(FILE *fp) {
    fprintf(fp, "(galaxy (turn %13d)\n        (num_species %6d)\n        (d_num_species %4d)\n        (radius %11d))\n",
            galaxy.turn_number, galaxy.num_species, galaxy.d_num_species, galaxy.radius);
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


void save_galaxy_data(void) {
    FILE *fp = fopen("galaxy.dat", "wb");
    if (fp == NULL) {
        perror("save_galaxy_data");
        fprintf(stderr, "\n\tCannot create new version of file 'galaxy.dat'!\n");
        exit(-1);
    }
    galaxyData.turn_number = galaxy.turn_number;
    galaxyData.num_species = galaxy.num_species;
    galaxyData.d_num_species = galaxy.d_num_species;
    galaxyData.radius = galaxy.radius;
    if (fwrite(&galaxyData, sizeof(galaxyData), 1, fp) != 1) {
        perror("save_galaxy_data");
        fprintf(stderr, "\n\tCannot write data to file 'galaxy.dat'!\n\n");
        exit(-1);
    }
    fclose(fp);
}
