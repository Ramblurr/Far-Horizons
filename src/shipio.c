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
#include "ship.h"
#include "shipio.h"
#include "item.h"

typedef struct {
    uint8_t name[32];                   /* Name of ship. */
    uint8_t x, y, z, pn;                /* Current coordinates. */
    uint8_t status;                     /* Current status of ship. */
    uint8_t type;                       /* Ship type. */
    uint8_t dest_x, dest_y;             /* Destination if ship was forced to jump from combat. */
    uint8_t dest_z;                     /* Ditto. Also used by TELESCOPE command. */
    uint8_t just_jumped;                /* Set if ship jumped this turn. */
    uint8_t arrived_via_wormhole;       /* Ship arrived via wormhole in the PREVIOUS turn. */
    uint8_t reserved1;                  /* Unused. Zero for now. */
    int16_t reserved2;                  /* Unused. Zero for now. */
    int16_t reserved3;                  /* Unused. Zero for now. */
    int16_t class;                      /* Ship class. */
    int16_t tonnage;                    /* Ship tonnage divided by 10,000. */
    int16_t item_quantity[MAX_ITEMS];   /* Quantity of each item carried. */
    int16_t age;                        /* Ship age. */
    int16_t remaining_cost;             /* The cost needed to complete the ship if still under construction. */
    int16_t reserved4;                  /* Unused. Zero for now. */
    int16_t loading_point;              /* Nampla index for planet where ship was last loaded with CUs. Zero = none. Use 9999 for home planet. */
    int16_t unloading_point;            /* Nampla index for planet that ship should be given orders to jump to where it will unload. Zero = none. Use 9999 for home planet. */
    int32_t special;                    /* Different for each application. */
    uint8_t padding[28];                /* Use for expansion. Initialized to all zeroes. */
} binary_data_t;


/* load ship data from file and create empty slots for future use */
struct ship_data *get_ship_data(int numShips, int extraShips, FILE *fp) {
    /* Allocate enough memory for all ships. */
    binary_data_t *binData = (binary_data_t *) calloc(numShips + extraShips, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("get_ship_data");
        fprintf(stderr, "\nCannot allocate enough memory for ship data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d ship entries\n\n", numShips, extraShips);
        exit(-1);
    }
    /* Read it all into memory. */
    if (numShips > 0 && fread(binData, sizeof(binary_data_t), numShips, fp) != numShips) {
        perror("get_ship_data");
        fprintf(stderr, "\nCannot read ship data into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* translate between the structures */
    struct ship_data *shipData = (struct ship_data *) calloc(numShips + extraShips, sizeof(struct ship_data));
    for (int i = 0; i < numShips; i++) {
        struct ship_data *s = &shipData[i];
        binary_data_t *sd = &binData[i];
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
    binary_data_t *binData = (binary_data_t *) calloc(numShips, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("save_ship_data");
        fprintf(stderr, "\nCannot allocate enough memory for ship data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* translate between the structures */
    for (int i = 0; i < numShips; i++) {
        struct ship_data *s = &shipData[i];
        binary_data_t *sd = &binData[i];
        memcpy(sd->name, s->name, 32);
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
    if (numShips > 0 && fwrite(binData, sizeof(struct ship_data), numShips, fp) != numShips) {
        perror("save_ship_data");
        fprintf(stderr, "\nCannot write ship data to file!\n");
        fprintf(stderr, "\n\tattempted to write %d ship entries\n\n", numShips);
        exit(-1);
    }
    /* release the binary data memory we allocated */
    free(binData);
}