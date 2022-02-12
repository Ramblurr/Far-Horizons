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

#ifndef FAR_HORIZONS_SHIP_H
#define FAR_HORIZONS_SHIP_H

#include "item.h"

// Additional memory must be allocated for routines that build ships.
// This is the default 'extras', which may be changed, if necessary, by the main program.
#define NUM_EXTRA_SHIPS 100

/* Ship classes. */
#define    PB                  0    /* Picketboat. */
#define    CT                  1    /* Corvette. */
#define    ES                  2    /* Escort. */
#define    DD                  3    /* Destroyer. */
#define    FG                  4    /* Frigate. */
#define    CL                  5    /* Light Cruiser. */
#define    CS                  6    /* Strike Cruiser. */
#define    CA                  7    /* Heavy Cruiser. */
#define    CC                  8    /* Command Cruiser. */
#define    BC                  9    /* Battlecruiser. */
#define    BS                  10   /* Battleship. */
#define    DN                  11   /* Dreadnought. */
#define    SD                  12   /* Super Dreadnought. */
#define    BM                  13   /* Battlemoon. */
#define    BW                  14   /* Battleworld. */
#define    BR                  15   /* Battlestar. */
#define    BA                  16   /* Starbase. */
#define    TR                  17   /* Transport. */
#define    NUM_SHIP_CLASSES    18

/* Ship types. */
#define    FTL        0
#define    SUB_LIGHT  1
#define    STARBASE   2

/* Ship status codes. */
#define    UNDER_CONSTRUCTION  0
#define    ON_SURFACE          1
#define    IN_ORBIT            2
#define    IN_DEEP_SPACE       3
#define    JUMPED_IN_COMBAT    4
#define    FORCED_JUMP         5

struct ship_data {
    char name[32];                  /* Name of ship. */
    char x, y, z, pn;               /* Current coordinates. */
    char status;                    /* Current status of ship. */
    char type;                      /* Ship type. */
    char dest_x, dest_y;            /* Destination if ship was forced to jump from combat. */
    char dest_z;                    /* Ditto. Also used by TELESCOPE command. */
    char just_jumped;               /* Set if ship jumped this turn. */
    char arrived_via_wormhole;      /* Ship arrived via wormhole in the PREVIOUS turn. */
    char reserved1;                 /* Unused. Zero for now. */
    short reserved2;                /* Unused. Zero for now. */
    short reserved3;                /* Unused. Zero for now. */
    short class;                    /* Ship class. */
    short tonnage;                  /* Ship tonnage divided by 10,000. */
    short item_quantity[MAX_ITEMS]; /* Quantity of each item carried. */
    short age;                      /* Ship age. */
    short remaining_cost;           /* The cost needed to complete the ship if still under construction. */
    short reserved4;                /* Unused. Zero for now. */
    short loading_point;            /* Nampla index for planet where ship was last loaded with CUs. Zero = none. Use 9999 for home planet. */
    short unloading_point;          /* Nampla index for planet that ship should be given orders to jump to where it will unload. Zero = none. Use 9999 for home planet. */
    long special;                   /* Different for each application. */
    char padding[28];               /* Use for expansion. Initialized to all zeroes. */
};

#endif //FAR_HORIZONS_SHIP_H