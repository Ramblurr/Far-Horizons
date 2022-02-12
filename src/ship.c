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
#include "ship.h"
#include "species.h"

struct ship_data *ship_data[MAX_SPECIES];
struct ship_data *ship_base;
struct ship_data *ship;

// Additional memory must be allocated for routines that build ships.
// This is the default 'extras', which may be changed, if necessary.
int extra_ships = NUM_EXTRA_SHIPS;
int num_new_ships[MAX_SPECIES];

char ship_abbr[NUM_SHIP_CLASSES][4] = {
        "PB", "CT", "ES", "FF", "DD", "CL", "CS",
        "CA", "CC", "BC", "BS", "DN", "SD", "BM",
        "BW", "BR", "BA", "TR"
};
char ship_type[3][2] = {"", "S", "S"};

int truncate_name = FALSE;
int ignore_field_distorters = FALSE;
char full_ship_id[64];

/* This routine will return a pointer to a string containing a complete
   ship name, including its orbital/landed status and age. If global
   variable "truncate_name" is TRUE, then orbital/landed status and age
   will not be included. */
char *ship_name(struct ship_data *ship) {
    int effective_age;
    int status;
    int ship_is_distorted;
    char temp[16];
    if (ship->item_quantity[FD] == ship->tonnage) {
        ship_is_distorted = TRUE;
    } else {
        ship_is_distorted = FALSE;
    }
    if (ship->status == ON_SURFACE) {
        ship_is_distorted = FALSE;
    }
    if (ignore_field_distorters) {
        ship_is_distorted = FALSE;
    }
    if (ship_is_distorted) {
        if (ship->class == TR) {
            sprintf(full_ship_id, "%s%d ???", ship_abbr[ship->class],
                    ship->tonnage);
        } else if (ship->class == BA) {
            sprintf(full_ship_id, "BAS ???");
        } else {
            sprintf(full_ship_id, "%s ???", ship_abbr[ship->class]);
        }
    } else if (ship->class == TR) {
        sprintf(full_ship_id, "%s%d%s %s",
                ship_abbr[ship->class], ship->tonnage, ship_type[ship->type],
                ship->name);
    } else {
        sprintf(full_ship_id, "%s%s %s",
                ship_abbr[ship->class], ship_type[ship->type], ship->name);
    }
    if (truncate_name) {
        return &full_ship_id[0];
    }
    strcat(full_ship_id, " (");
    effective_age = ship->age;
    if (effective_age < 0) {
        effective_age = 0;
    }
    if (!ship_is_distorted) {
        if (ship->status != UNDER_CONSTRUCTION) {
            /* Do age. */
            sprintf(temp, "A%d,", effective_age);
            strcat(full_ship_id, temp);
        }
    }
    status = ship->status;
    switch (status) {
        case UNDER_CONSTRUCTION:
            sprintf(temp, "C");
            break;
        case IN_ORBIT:
            sprintf(temp, "O%d", ship->pn);
            break;
        case ON_SURFACE:
            sprintf(temp, "L%d", ship->pn);
            break;
        case IN_DEEP_SPACE:
            sprintf(temp, "D");
            break;
        case FORCED_JUMP:
            sprintf(temp, "FJ");
            break;
        case JUMPED_IN_COMBAT:
            sprintf(temp, "WD");
            break;
        default:
            sprintf(temp, "***???***");
            fprintf(stderr, "\n\tWARNING!!!  Internal error in subroutine 'ship_name'\n\n");
    }
    strcat(full_ship_id, temp);
    if (ship->type == STARBASE) {
        sprintf(temp, ",%ld tons", 10000L * (long) ship->tonnage);
        strcat(full_ship_id, temp);
    }
    strcat(full_ship_id, ")");
    return &full_ship_id[0];
}

