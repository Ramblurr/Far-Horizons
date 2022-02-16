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

#ifndef FAR_HORIZONS_SPECIESIO_H
#define FAR_HORIZONS_SPECIESIO_H

#include <stdio.h>
#include "species.h"

void get_species_data(void);

void save_species_data(void);

void speciesDataAsSExpr(species_data_t *sp, int spNo, FILE *fp);

// globals. ugh.

extern int data_in_memory[MAX_SPECIES];

extern int data_modified[MAX_SPECIES];

extern struct species_data spec_data[MAX_SPECIES];

#endif //FAR_HORIZONS_SPECIESIO_H
