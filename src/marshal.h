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

#ifndef FAR_HORIZONS_MARSHAL_H
#define FAR_HORIZONS_MARSHAL_H


#include "data.h"
#include "json.h"


json_value_t *marshalCluster(global_cluster_t *c);

json_value_t *marshalColony(global_colony_t *c);

json_value_t *marshalColonies(global_colony_t **c);

json_value_t *marshalDevelop(global_develop_t *d);

json_value_t *marshalGas(global_gas_t *g);

json_value_t *marshalGases(global_gas_t **g);

json_value_t *marshalGlobals(global_data_t *g);

json_value_t *marshalInventory(global_item_t **i);

json_value_t *marshalLocation(global_location_t l);

json_value_t *marshalPlanet(global_planet_t *p);

json_value_t *marshalPlanets(global_planet_t **p);

json_value_t *marshalShip(global_ship_t *s);

json_value_t *marshalShips(global_ship_t **s);

json_value_t *marshalSkill(global_skill_t *s);

json_value_t *marshalSkills(global_skill_t **s);

json_value_t *marshalSpecie(global_species_t *s);

json_value_t *marshalSpecies(global_species_t **s);

json_value_t *marshalSystem(global_system_t *s);

json_value_t *marshalSystems(global_system_t **s);


#endif //FAR_HORIZONS_MARSHAL_H
