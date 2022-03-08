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

#include "engine.h"

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


int check_population(struct nampla_data *nampla);

void delete_nampla(struct nampla_data *nampla);

#endif //FAR_HORIZONS_NAMPLA_H
