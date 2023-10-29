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

#ifndef FAR_HORIZONS_UNMARSHAL_H
#define FAR_HORIZONS_UNMARSHAL_H

#include "cjson/cJSON.h"
#include "engine.h"

void unmarshalGalaxyFile(cJSON *root, galaxy_data_t *g);

void unmarshalSpeciesFile(cJSON *root, species_data_t *sp, nampla_data_t *npa, ship_data_t *sa);

void unmarshalSystemsFile(cJSON *root, star_data_t *sa, planet_data_t *pa);

#endif //FAR_HORIZONS_UNMARSHAL_H
