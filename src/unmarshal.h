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

#include "data.h"
#include "json.h"


global_cluster_t *unmarshalCluster(json_value_t *j);

global_colony_t *unmarshalColony(json_value_t *j);

global_colony_t **unmarshalColonies(json_value_t *j);

global_coords_t *unmarshalCoords(json_value_t *j);

global_data_t *unmarshalData(json_value_t *j);

global_develop_t *unmarshalDevelop(json_value_t *j);

global_develop_t **unmarshalDevelopList(json_value_t *j);

global_gas_t *unmarshalGas(json_value_t *j);

global_gas_t **unmarshalGases(json_value_t *j);

global_item_t **unmarshalInventory(json_value_t *j);

global_location_t *unmarshalLocation(json_value_t *j);

global_planet_t *unmarshalPlanet(json_value_t *j);

global_planet_t **unmarshalPlanets(json_value_t *j);

global_ship_t *unmarshalShip(json_value_t *j);

global_ship_t **unmarshalShips(json_value_t *j);

global_skill_t *unmarshalSkill(json_value_t *j);

global_species_t *unmarshalSpecie(json_value_t *j);

global_species_t **unmarshalSpecies(json_value_t *j);

global_system_t *unmarshalSystem(json_value_t *j);

global_system_t **unmarshalSystems(json_value_t *j);




#endif //FAR_HORIZONS_UNMARSHAL_H
