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

#ifndef FAR_HORIZONS_DATA_H
#define FAR_HORIZONS_DATA_H

#include <stdio.h>
#include "engine.h"


typedef struct global_cluster {
    int radius;
    int d_num_species;
    struct global_system **systems;
} global_cluster_t;

typedef struct global_coords {
    int x, y, z;
} global_coords_t;

typedef struct global_location {
    int x, y, z;
    int orbit;
    char colony[64];
    int deep_space;
    int in_orbit;
    int on_surface;
    struct global_system *system;
    int systemId;
    struct global_planet *planet;
    int planetId;
} global_location_t;

typedef struct global_colony {
    int id;
    char name[32];
    int homeworld;
    struct global_location location;
    struct global_develop *develop[3];
    int hiding;
    int hidden;
    struct global_item **inventory;
    int ma_base;
    int message;
    int mi_base;
    int pop_units;
    int shipyards;
    int siege_eff;
    int special;
    int status;
    int use_on_ambush;
} global_colony_t;

typedef struct global_data {
    int turn;
    struct global_cluster *cluster;
    struct global_species **species;
} global_data_t;

typedef struct global_develop {
    char code[4];
    int auto_install;
    int units_needed;
    int units_to_install;
} global_develop_t;

typedef struct global_gas {
    char code[4];
    int atmos_pct;
    int min_pct;
    int max_pct;
    int required;
} global_gas_t;

typedef struct global_item {
    char code[4];
    int quantity;
} global_item_t;

typedef struct global_planet {
    int id;
    int orbit;
    int diameter;
    int econ_efficiency;
    struct global_gas *gases[5];
    int gravity;
    int idealHomePlanet;
    int idealColonyPlanet;
    int md_increase;
    int message;
    int mining_difficulty;
    int pressure_class;
    int radioactiveHellHole;
    int temperature_class;
} global_planet_t;

typedef struct global_ship {
    int id;
    char name[64];
    int age;
    int arrived_via_wormhole;
    struct global_item **inventory;
    struct global_location location;
    struct global_location destination;
    int just_jumped;
    char loading_point[32];
    int remaining_cost;
    int special;
    int status;
    int tonnage; // valid only for starbases
    char unloading_point[32];
} global_ship_t;

typedef struct global_skill {
    char code[4];
    char name[32];
    int init_level;
    int current_level;
    int knowledge_level;
    int xps;
} global_skill_t;

typedef struct global_species {
    int id;
    char name[32];
    char govt_name[32];
    char govt_type[32];
    struct global_skill *skills[7];
    int auto_orders;
    int econ_units;
    int hp_original_base;
    global_gas_t *required_gases[2];
    global_gas_t *neutral_gases[7];
    global_gas_t *poison_gases[7];
    struct global_colony **colonies;
    struct global_ship **ships;
    int contacts[MAX_SPECIES + 1];
    int allies[MAX_SPECIES + 1];
    int enemies[MAX_SPECIES + 1];
} global_species_t;

typedef struct global_system {
    int id;
    struct global_coords coords;
    int color;
    int home_system;
    int message;
    int size;
    int type;
    int wormholeExit;
    struct global_planet **planets;
    int visited_by[MAX_SPECIES + 1];
} global_system_t;



int exportData(FILE *fp);

#endif //FAR_HORIZONS_DATA_H
