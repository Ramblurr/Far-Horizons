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
    char name[32];                  /* Name of planet. */
    char x, y, z, pn;               /* Coordinates. */
    char status;                    /* Status of planet. */
    char reserved1;                 /* Zero for now. */
    char hiding;                    /* HIDE order given. */
    char hidden;                    /* Colony is hidden. */
    short reserved2;                /* Zero for now. */
    short planet_index;             /* Index (starting at zero) into the file "planets.dat" of this planet. */
    short siege_eff;                /* Siege effectiveness - a percentage between 0 and 99. */
    short shipyards;                /* Number of shipyards on planet. */
    int reserved4;                  /* Zero for now. */
    int IUs_needed;                 /* Incoming ship with only CUs on board. */
    int AUs_needed;                 /* Incoming ship with only CUs on board. */
    int auto_IUs;                   /* Number of IUs to be automatically installed. */
    int auto_AUs;                   /* Number of AUs to be automatically installed. */
    int reserved5;                  /* Zero for now. */
    int IUs_to_install;             /* Colonial mining units to be installed. */
    int AUs_to_install;             /* Colonial manufacturing units to be installed. */
    long mi_base;                   /* Mining base times 10. */
    long ma_base;                   /* Manufacturing base times 10. */
    long pop_units;                 /* Number of available population units. */
    long item_quantity[MAX_ITEMS];  /* Quantity of each item available. */
    long reserved6;                 /* Zero for now. */
    long use_on_ambush;             /* Amount to use on ambush. */
    long message;                   /* Message associated with this planet, if any. */
    long special;                   /* Different for each application. */
    char padding[28];               /* Use for expansion. Initialized to all zeroes. */
};
typedef struct nampla_data nampla_data_t;

int check_population(struct nampla_data *nampla);

void delete_nampla(struct nampla_data *nampla);

// globals. ugh.

extern struct nampla_data *nampla;
extern struct nampla_data *nampla_base;
extern struct nampla_data *namp_data[MAX_SPECIES];
extern int nampla_index;
extern int num_new_namplas[MAX_SPECIES];

#endif //FAR_HORIZONS_NAMPLA_H
