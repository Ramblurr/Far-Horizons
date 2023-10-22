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

#ifndef FAR_HORIZONS_STARIO_H
#define FAR_HORIZONS_STARIO_H

#include <stdio.h>
#include "star.h"


void get_star_data(void);

void save_star_data(void);

void saveStarData(star_data_t *starBase, int numStars, FILE *fp);

void starDataAsSExpr(star_data_t *starBase, int numStars, FILE *fp);

cJSON *starsDataToJson(star_data_t *starBase, int numStars);

cJSON *starToJson(star_data_t *sd, int id);

// globals. ugh.

extern int num_stars;
extern struct star_data *star_base;
extern int star_data_modified;

#endif //FAR_HORIZONS_STARIO_H
