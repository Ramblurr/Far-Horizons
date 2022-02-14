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

#ifndef FAR_HORIZONS_SHIPVARS_H
#define FAR_HORIZONS_SHIPVARS_H

#include "engine.h"
#include "ship.h"

// globals. ugh.

extern int ignore_field_distorters;
extern int num_new_ships[MAX_SPECIES];
extern struct ship_data *ship;
extern char ship_abbr[NUM_SHIP_CLASSES][4];
extern short ship_cost[NUM_SHIP_CLASSES];
extern struct ship_data *ship_base;
extern struct ship_data *ship_data[MAX_SPECIES];
extern int ship_index;
extern short ship_tonnage[NUM_SHIP_CLASSES];
extern char ship_type[3][2];
extern int truncate_name;

#endif //FAR_HORIZONS_SHIPVARS_H
