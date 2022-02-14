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

#include "shipvars.h"

int ignore_field_distorters = FALSE;

struct ship_data *ship;

char ship_abbr[NUM_SHIP_CLASSES][4] = {
        "PB", "CT", "ES", "FF", "DD", "CL", "CS",
        "CA", "CC", "BC", "BS", "DN", "SD", "BM",
        "BW", "BR", "BA", "TR"
};

struct ship_data *ship_base;

struct ship_data *ship_data[MAX_SPECIES];

int ship_index;

short ship_cost[NUM_SHIP_CLASSES] = {
        100, 200, 500, 1000, 1500, 2000, 2500,
        3000, 3500, 4000, 4500, 5000, 5500, 6000,
        6500, 7000, 100, 100
};

int ship_index;

short ship_tonnage[NUM_SHIP_CLASSES] = {
        1, 2, 5, 10, 15, 20, 25,
        30, 35, 40, 45, 50, 55, 60,
        65, 70, 1, 1
};

char ship_type[3][2] = {"", "S", "S"};

int truncate_name = FALSE;


// Additional memory must be allocated for routines that build ships.
// This is the default 'extras', which may be changed, if necessary.
int extra_ships = NUM_EXTRA_SHIPS;
int num_new_ships[MAX_SPECIES];

