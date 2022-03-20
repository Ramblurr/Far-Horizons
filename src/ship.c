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
#include <string.h>
#include "nampla.h"
#include "namplavars.h"
#include "ordersvars.h"
#include "ship.h"
#include "shipvars.h"
#include "species.h"
#include "speciesvars.h"


/* Look-up table for ship defensive/offensive power uses ship->tonnage as an index.
 * Each value is equal to 100 * (ship->tonnage)^1.2.
 * The 'power' subroutine uses recursion to calculate values for tonnages over 100. */
static short ship_power[101] = {
        0,    /* Zeroth element not used. */
        100, 230, 374, 528, 690, 859, 1033, 1213, 1397, 1585,
        1777, 1973, 2171, 2373, 2578, 2786, 2996, 3209, 3424, 3641,
        3861, 4082, 4306, 4532, 4759, 4988, 5220, 5452, 5687, 5923,
        6161, 6400, 6641, 6883, 7127, 7372, 7618, 7866, 8115, 8365,
        8617, 8870, 9124, 9379, 9635, 9893, 10151, 10411, 10672, 10934,
        11197, 11461, 11725, 11991, 12258, 12526, 12795, 13065, 13336, 13608,
        13880, 14154, 14428, 14703, 14979, 15256, 15534, 15813, 16092, 16373,
        16654, 16936, 17218, 17502, 17786, 18071, 18356, 18643, 18930, 19218,
        19507, 19796, 20086, 20377, 20668, 20960, 21253, 21547, 21841, 22136,
        22431, 22727, 23024, 23321, 23619, 23918, 24217, 24517, 24818, 25119
};


void delete_ship(struct ship_data *ship) {
    /* Set all bytes of record to zero. */
    memset(ship, 0, sizeof(struct ship_data));
    ship->pn = 99;
    strcpy(ship->name, "Unused");
}


int disbanded_ship(struct ship_data *ship) {
    struct nampla_data *nampla = nampla_base - 1;
    for (int nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
        nampla++;
        if (nampla->x != ship->x) { continue; }
        if (nampla->y != ship->y) { continue; }
        if (nampla->z != ship->z) { continue; }
        if (nampla->pn != ship->pn) { continue; }
        if ((nampla->status & DISBANDED_COLONY) == 0) { continue; }
        if (ship->type != STARBASE && ship->status == IN_ORBIT) { continue; }
        /* This ship is either on the surface of a disbanded colony or is a starbase orbiting a disbanded colony. */
        return TRUE;
    }
    return FALSE;
}


long power(short tonnage) {
    long result;
    short t1, t2;
    if (tonnage > 4068) {
        fprintf(stderr, "\n\n\tLong integer overflow will occur in call to 'power(tonnage)'!\n");
        fprintf(stderr, "\t\tActual call is power(%d).\n\n", tonnage);
        exit(-1);
    }
    if (tonnage <= 100) {
        return ship_power[tonnage];
    }
    /* Tonnage is not in table.
     * Break it up into two halves and get approximate result = 1.149 * (x1 + x2), using recursion. */
    t1 = tonnage / 2;
    t2 = tonnage - t1;
    return result = 1149L * (power(t1) + power(t2)) / 1000L;
}


void printMishapChanceToOrders(struct ship_data *ship, int destx, int desty, int destz) {
    int mishap_GV;
    int mishap_age;
    long stx;
    long sty;
    long stz;
    long mishap_chance;
    long success_chance;
    long temp_distance;

    if (destx == -1) {
        fprintf(orders_file, "Mishap chance = ???");
        return;
    }

    stx = destx;
    sty = desty;
    stz = destz;
    temp_distance = ((stx - ship->x) * (stx - ship->x))
                    + ((sty - ship->y) * (sty - ship->y))
                    + ((stz - ship->z) * (stz - ship->z));

    mishap_age = ship->age;
    mishap_GV = species->tech_level[GV];
    if (mishap_GV > 0) {
        mishap_chance = 100 * temp_distance / mishap_GV;
    } else {
        mishap_chance = 10000;
    }
    if (mishap_age > 0 && mishap_chance < 10000) {
        success_chance = 10000L - mishap_chance;
        success_chance -= (2L * (long) mishap_age * success_chance) / 100L;
        mishap_chance = 10000L - success_chance;
    }
    if (mishap_chance > 10000) {
        mishap_chance = 10000;
    }
    fprintf(orders_file, "mishap chance = %ld.%02ld%%", mishap_chance / 100L, mishap_chance % 100L);
}


char *shipDisplayName(ship_data_t *ship) {
    static char fullShipName[64];

    if (ship->class == TR) {
        sprintf(fullShipName, "%s%d%s %s",
                ship_abbr[ship->class], ship->tonnage, ship_type[ship->type], ship->name);
    } else {
        sprintf(fullShipName, "%s%s %s",
                ship_abbr[ship->class], ship_type[ship->type], ship->name);
    }
    return fullShipName;
}


/* This routine will return a pointer to a string containing a complete
 * ship name, including its orbital/landed status and age.
 * If global variable "truncate_name" is TRUE,
 * then orbital/landed status and age will not be included. */
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
            fprintf(stderr, "\ndebug: ship ptr %p name '%s' status %12d\n", ship, ship->name, ship->status);
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
