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
#include <string.h>
#include "data.h"
#include "ship.h"
#include "shipio.h"
#include "item.h"


/* load ship data from file and create empty slots for future use */
struct ship_data *get_ship_data(int numShips, int extraShips, FILE *fp) {
    /* Allocate enough memory for all ships. */
    binary_ship_data_t *binData = (binary_ship_data_t *) ncalloc(__FUNCTION__, __LINE__, numShips + extraShips,
                                                                 sizeof(binary_ship_data_t));
    if (binData == NULL) {
        perror("get_ship_data");
        fprintf(stderr, "\nCannot allocate enough memory for ship data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d ship entries\n\n", numShips, extraShips);
        exit(-1);
    }
    /* Read it all into memory. */
    if (numShips > 0 && fread(binData, sizeof(binary_ship_data_t), numShips, fp) != numShips) {
        perror("get_ship_data");
        fprintf(stderr, "\nCannot read ship data into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* translate between the structures */
    struct ship_data *shipData = (struct ship_data *) ncalloc(__FUNCTION__, __LINE__, numShips + extraShips,
                                                              sizeof(struct ship_data));
    for (int i = 0; i < numShips; i++) {
        struct ship_data *s = &shipData[i];
        binary_ship_data_t *sd = &binData[i];
        memcpy(s->name, sd->name, 32);
        s->x = (char) (sd->x);
        s->y = (char) (sd->y);
        s->z = (char) (sd->z);
        s->pn = (char) (sd->pn);
        s->status = (char) (sd->status);
        s->type = (char) (sd->type);
        s->dest_x = (char) (sd->dest_x);
        s->dest_y = (char) (sd->dest_y);
        s->dest_z = (char) (sd->dest_z);
        s->just_jumped = (char) (sd->just_jumped);
        s->arrived_via_wormhole = (char) (sd->arrived_via_wormhole);
        s->class = sd->class;
        s->tonnage = sd->tonnage;
        for (int j = 0; j < MAX_ITEMS; j++) {
            s->item_quantity[j] = sd->item_quantity[j];
        }
        s->age = sd->age;
        s->remaining_cost = sd->remaining_cost;
        s->loading_point = sd->loading_point;
        s->unloading_point = sd->unloading_point;
        s->special = sd->special;
    }
    /* release the binary data memory we allocated */
    free(binData);

    return shipData;
}


void save_ship_data(struct ship_data *shipData, int numShips, FILE *fp) {
    /* Allocate enough memory for all ships. */
    binary_ship_data_t *binData = (binary_ship_data_t *) ncalloc(__FUNCTION__, __LINE__,
                                                                 numShips, sizeof(binary_ship_data_t));
    if (binData == NULL) {
        perror("save_ship_data");
        fprintf(stderr, "\nCannot allocate enough memory for ship data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* translate between the structures */
    for (int i = 0; i < numShips; i++) {
        struct ship_data *s = &shipData[i];
        binary_ship_data_t *sd = &binData[i];
        strncpy((char *) (sd->name), s->name, 32);
        sd->x = s->x;
        sd->y = s->y;
        sd->z = s->z;
        sd->pn = s->pn;
        sd->status = s->status;
        sd->type = s->type;
        sd->dest_x = s->dest_x;
        sd->dest_y = s->dest_y;
        sd->dest_z = s->dest_z;
        sd->just_jumped = s->just_jumped;
        sd->arrived_via_wormhole = s->arrived_via_wormhole;
        sd->class = s->class;
        sd->tonnage = s->tonnage;
        for (int j = 0; j < MAX_ITEMS; j++) {
            sd->item_quantity[j] = s->item_quantity[j];
        }
        sd->age = s->age;
        sd->remaining_cost = s->remaining_cost;
        sd->loading_point = s->loading_point;
        sd->unloading_point = s->unloading_point;
        sd->special = s->special;
    }
    /* Write ship data. */
    if (numShips > 0 && fwrite(binData, sizeof(binary_ship_data_t), numShips, fp) != numShips) {
        perror("save_ship_data");
        fprintf(stderr, "\nCannot write ship data to file!\n");
        fprintf(stderr, "\n\tattempted to write %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* release the binary data memory we allocated */
    free(binData);
}


void shipDataAsJson(int spNo, struct ship_data *shipData, int num_ships, FILE *fp) {
    fprintf(fp, "{\n");
    fprintf(fp, "  \"species_no\": %d,\n", spNo);
    fprintf(fp, "  \"ships\": [");
    for (int i = 0; i < num_ships; i++) {
        struct ship_data *sd = &shipData[i];
        fprintf(fp, "{\n");
        fprintf(fp, "      \"id\": %d,\n", i + 1);
        fprintf(fp, "      \"name\": \"%s\",\n", sd->name);
        fprintf(fp, "      \"location\": {\"x\": %d, \"y\": %d, \"z\": %d, \"orbit\": %d, \"status\": %d},\n",
                sd->x, sd->y, sd->z, sd->pn, sd->status);
        fprintf(fp, "      \"destination\": {\"x\": %d, \"y\": %d, \"z\": %d},\n",
                sd->dest_x, sd->dest_y, sd->dest_z);
        fprintf(fp, "      \"age\": %d,\n", sd->age);
        fprintf(fp, "      \"arrived_via_wormhole\": %s,\n", sd->arrived_via_wormhole ? "true" : "false");
        fprintf(fp, "      \"class\": %d,\n", sd->class);
        fprintf(fp, "      \"just_jumped\": %s,\n", sd->just_jumped ? "true" : "false");
        fprintf(fp, "      \"just_jumped_val\": %d,\n", sd->just_jumped);
        fprintf(fp, "      \"loading_point\": %d,\n", sd->loading_point);
        fprintf(fp, "      \"remaining_cost\": %d,\n", sd->remaining_cost);
        fprintf(fp, "      \"tonnage\": %d,\n", sd->tonnage);
        fprintf(fp, "      \"type\": %d,\n", sd->type);
        fprintf(fp, "      \"unloading_point\": %d,\n", sd->unloading_point);
        const char *sep = "\n";
        fprintf(fp, "      \"cargo\": [");
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (sd->item_quantity[j] > 0) {
                fprintf(fp, "%s        {\"code\": %2d, \"qty\": %6d}", sep, j, sd->item_quantity[j]);
                sep = ",\n";
            }
        }
        if (*sep == ',') {
            // not an empty list so put closing bracket on new line
            fprintf(fp, "\n      ");
        }
        fprintf(fp, "]\n");
        fprintf(fp, "    }");
        if (i + 1 < num_ships) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "]\n");
    fprintf(fp, "}\n");
}


void shipDataAsSExpr(int spNo, struct ship_data *shipData, int num_ships, FILE *fp) {
    fprintf(fp, "(ships (species_no %3d)", spNo);
    for (int i = 0; i < num_ships; i++) {
        struct ship_data *sd = &shipData[i];
        fprintf(fp, "\n  (ship (id %6d) (name \"%s\")", i + 1, sd->name);
        fprintf(fp, "\n        (location    (x %3d) (y %3d) (z %3d) (orbit %d) (status %3d))",
                sd->x, sd->y, sd->z, sd->pn, sd->status);
        fprintf(fp, "\n        (destination (x %3d) (y %3d) (z %3d))",
                sd->dest_x, sd->dest_y, sd->dest_z);
        fprintf(fp, "\n        (age                  %9d)", sd->age);
        fprintf(fp, "\n        (arrived_via_wormhole %9s)", sd->arrived_via_wormhole ? "true" : "false");
        fprintf(fp, "\n        (class                %9d)", sd->class);
        fprintf(fp, "\n        (just_jumped          %9s)", sd->just_jumped ? "true" : "false");
        fprintf(fp, "\n        (just_jumped_val      %9d)", sd->just_jumped);
        fprintf(fp, "\n        (loading_point        %9d)", sd->loading_point);
        fprintf(fp, "\n        (remaining_cost       %9d)", sd->remaining_cost);
        fprintf(fp, "\n        (tonnage              %9d)", sd->tonnage);
        fprintf(fp, "\n        (type                 %9d)", sd->type);
        fprintf(fp, "\n        (unloading_point      %9d)", sd->unloading_point);
        fprintf(fp, "\n        (cargo");
        const char *sep = "";
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (sd->item_quantity[j] > 0) {
                fprintf(fp, "%s (item (code %2d) (qty %6d))", sep, j, sd->item_quantity[j]);
                sep = "\n              ";
            }
        }
        fprintf(fp, "))");
    }
    fprintf(fp, ")\n");
}
