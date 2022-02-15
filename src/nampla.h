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

#ifndef FAR_HORIZONS_NAMPLA_H
#define FAR_HORIZONS_NAMPLA_H

#include "item.h"

// Additional memory must be allocated for routines that name planets.
// This is the default 'extras', which may be changed, if necessary, by the main program.
#define NUM_EXTRA_NAMPLAS 50

/* Status codes for named planets. These are logically ORed together. */
#define HOME_PLANET        1
#define COLONY             2
#define POPULATED          8
#define MINING_COLONY      16
#define RESORT_COLONY      32
#define DISBANDED_COLONY   64

struct nampla_data {
    char name[32];                 /* Name of planet. */
    int x, y, z, pn;               /* Coordinates. */
    int status;                    /* Status of planet. */
    int hiding;                    /* HIDE order given. */
    int hidden;                    /* Colony is hidden. */
    int planet_index;              /* Index (starting at zero) into the file "planets.dat" of this planet. */
    int siege_eff;                 /* Siege effectiveness - a percentage between 0 and 99. */
    int shipyards;                 /* Number of shipyards on planet. */
    int IUs_needed;                /* Incoming ship with only CUs on board. */
    int AUs_needed;                /* Incoming ship with only CUs on board. */
    int auto_IUs;                  /* Number of IUs to be automatically installed. */
    int auto_AUs;                  /* Number of AUs to be automatically installed. */
    int IUs_to_install;            /* Colonial mining units to be installed. */
    int AUs_to_install;            /* Colonial manufacturing units to be installed. */
    int mi_base;                   /* Mining base times 10. */
    int ma_base;                   /* Manufacturing base times 10. */
    int pop_units;                 /* Number of available population units. */
    int item_quantity[MAX_ITEMS];  /* Quantity of each item available. */
    int use_on_ambush;             /* Amount to use on ambush. */
    int message;                   /* Message associated with this planet, if any. */
    int special;                   /* Different for each application. */
};
typedef struct nampla_data nampla_data_t;

int check_population(struct nampla_data *nampla);

void delete_nampla(struct nampla_data *nampla);

#endif //FAR_HORIZONS_NAMPLA_H
