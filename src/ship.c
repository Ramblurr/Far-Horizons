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

#include "ship.h"
#include "species.h"

struct ship_data *ship_data[MAX_SPECIES];
struct ship_data *ship_base;
struct ship_data *ship;

// Additional memory must be allocated for routines that build ships.
// This is the default 'extras', which may be changed, if necessary.
int extra_ships = NUM_EXTRA_SHIPS;
int num_new_ships[MAX_SPECIES];