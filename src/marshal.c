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


#include <stdio.h>
#include <stdlib.h>
#include "marshal.h"
#include "engine.h"
#include "galaxyio.h"
#include "planetio.h"
#include "speciesvars.h"
#include "stario.h"

static cJSON *marshalNamedPlanet(nampla_data_t *npd);

static cJSON *marshalPlanet(planet_data_t *pd);

static cJSON *marshalPlanets(planet_data_t *planets, int num);

static cJSON *marshalShip(ship_data_t *sd);

static cJSON *marshalStar(star_data_t *sd);

cJSON *marshalGalaxy(void) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "galaxy: unable to allocate root\n");
        exit(2);
    }
    jsonAddIntToObj(root, "root", "version", 1);
    cJSON *obj = cJSON_AddObjectToObject(root, "galaxy");
    if (obj == 0) {
        fprintf(stderr, "galaxy: unable to allocate property 'galaxy'\n");
        perror("cJSON_AddObjectToObject:");
        exit(2);
    }
    jsonAddIntToObj(obj, "galaxy", "turn_number", galaxy.turn_number);
    jsonAddIntToObj(obj, "galaxy", "num_species", galaxy.num_species);
    jsonAddIntToObj(obj, "galaxy", "d_num_species", galaxy.d_num_species);
    jsonAddIntToObj(obj, "galaxy", "radius", galaxy.radius);
    return root;
}

cJSON *marshalNamedPlanet(nampla_data_t *npd) {
    char *objName = "named_planet";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        perror("cJSON_CreateObject:");
        fprintf(stderr, "error: named_planet: unable to allocate object\n");
        exit(2);
    }
    jsonAddStringToObj(obj, objName, "name", npd->name);
    jsonAddIntToObj(obj, objName, "star_x", npd->x);
    jsonAddIntToObj(obj, objName, "star_y", npd->y);
    jsonAddIntToObj(obj, objName, "star_z", npd->z);
    jsonAddIntToObj(obj, objName, "planet_orbit", npd->pn);
    jsonAddIntToObj(obj, objName, "status", npd->status);
    jsonAddIntToObj(obj, objName, "hiding", npd->hiding);
    jsonAddIntToObj(obj, objName, "hidden", npd->hidden);
    jsonAddIntToObj(obj, objName, "ma_base", npd->ma_base);
    jsonAddIntToObj(obj, objName, "mi_base", npd->mi_base);
    jsonAddIntToObj(obj, objName, "pop_units", npd->pop_units);
    jsonAddIntToObj(obj, objName, "shipyards", npd->shipyards);
    jsonAddIntToObj(obj, objName, "siege_eff", npd->siege_eff);
    jsonAddIntToObj(obj, objName, "special", npd->special);
    jsonAddIntToObj(obj, objName, "use_on_ambush", npd->use_on_ambush);
    jsonAddIntToObj(obj, objName, "au_auto", npd->auto_AUs);
    jsonAddIntToObj(obj, objName, "au_needed", npd->AUs_needed);
    jsonAddIntToObj(obj, objName, "au_to_install", npd->AUs_to_install);
    jsonAddIntToObj(obj, objName, "iu_auto", npd->auto_IUs);
    jsonAddIntToObj(obj, objName, "iu_needed", npd->IUs_needed);
    jsonAddIntToObj(obj, objName, "iu_to_install", npd->IUs_to_install);
    cJSON *items = cJSON_AddArrayToObject(obj, "items");
    if (items == 0) {
        perror("cJSON_AddArrayToObject:");
        fprintf(stderr, "error: named_planet: unable to allocate property 'items'\n");
        exit(2);
    } else {
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (npd->item_quantity[j] > 0) {
                cJSON *item = cJSON_CreateObject();
                jsonAddIntToObj(item, "item.code", "code", j);
                jsonAddIntToObj(item, "item.qty", "qty", npd->item_quantity[j]);
                cJSON_AddItemToArray(items, item);
            }
        }
    }
    return obj;
}

cJSON *marshalPlanet(planet_data_t *pd) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        perror("cJSON_CreateObject:");
        fprintf(stderr, "error: planet: unable to allocate object\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "planet", "diameter", pd->diameter);
    jsonAddIntToObj(obj, "planet", "gravity", pd->gravity);
    jsonAddIntToObj(obj, "planet", "temperature_class", pd->temperature_class);
    jsonAddIntToObj(obj, "planet", "pressure_class", pd->pressure_class);
    jsonAddIntToObj(obj, "planet", "special", pd->special);
    cJSON *gases = cJSON_AddArrayToObject(obj, "gases");
    if (gases == 0) {
        perror("cJSON_AddArrayToObject:");
        fprintf(stderr, "error: planet: unable to allocate property 'gases'\n");
        exit(2);
    }
    for (int j = 0; j < 4; j++) {
        cJSON *gas = cJSON_AddObjectToObject(gases, "gases");
        if (gas == NULL) {
            perror("cJSON_AddObjectToObject:");
            fprintf(stderr, "error: planet: unable to allocate property 'gas'\n");
            exit(2);
        }
        jsonAddIntToObj(gas, "gases", "code", pd->gas[j]);
        jsonAddIntToObj(gas, "gases", "percent", pd->gas_percent[j]);
    }
    cJSON *miningDifficulty = cJSON_AddObjectToObject(obj, "mining_difficulty");
    if (miningDifficulty == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: planet: unable to allocate property 'mining_difficulty'\n");
        exit(2);
    }
    jsonAddIntToObj(miningDifficulty, "mining_difficulty", "base", pd->mining_difficulty);
    jsonAddIntToObj(miningDifficulty, "mining_difficulty", "increase", pd->md_increase);
    jsonAddIntToObj(obj, "planet", "econ_efficiency", pd->econ_efficiency);
    jsonAddIntToObj(obj, "planet", "message", pd->message);
    return obj;
}

cJSON *marshalPlanets(planet_data_t *planets, int num) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: planets: unable to allocate array\n");
        perror("cJSON_CreateArray");
        exit(2);
    }
    for (int p = 0; p < num; p++) {
        planet_data_t *pd = &planets[p];
        jsonAddItemToArray(array, "planets", marshalPlanet(pd));
    }
    return array;
}

cJSON *marshalShip(ship_data_t *sd) {
    char *objName = "ship";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: ship: unable to allocate object\n");
        exit(2);
    }
    jsonAddStringToObj(obj, objName, "name", sd->name);
    jsonAddIntToObj(obj, objName, "type", sd->type);
    jsonAddIntToObj(obj, objName, "tonnage", sd->tonnage);
    jsonAddIntToObj(obj, objName, "age", sd->age);
    jsonAddIntToObj(obj, objName, "loc_x", sd->x);
    jsonAddIntToObj(obj, objName, "loc_y", sd->y);
    jsonAddIntToObj(obj, objName, "loc_z", sd->z);
    jsonAddIntToObj(obj, objName, "loc_orbit", sd->pn);
    jsonAddIntToObj(obj, objName, "loc_status", sd->status);
    jsonAddIntToObj(obj, objName, "dest_x", sd->dest_x);
    jsonAddIntToObj(obj, objName, "dest_y", sd->dest_y);
    jsonAddIntToObj(obj, objName, "dest_z", sd->dest_z);
    jsonAddBoolToObj(obj, objName, "arrived_via_wormhole", sd->arrived_via_wormhole);
    jsonAddIntToObj(obj, objName, "class", sd->class);
    jsonAddIntToObj(obj, objName, "just_jumped", sd->just_jumped);
    jsonAddIntToObj(obj, objName, "loading_point", sd->loading_point);
    jsonAddIntToObj(obj, objName, "remaining_cost", sd->remaining_cost);
    jsonAddIntToObj(obj, objName, "unloading_point", sd->unloading_point);
    cJSON *cargo = cJSON_AddArrayToObject(obj, "cargo");
    if (cargo == 0) {
        fprintf(stderr, "error: ship: unable to allocate property 'cargo'\n");
        exit(2);
    } else {
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (sd->item_quantity[j] > 0) {
                cJSON *item = cJSON_CreateObject();
                jsonAddIntToObj(item, "cargo.code", "code", j);
                jsonAddIntToObj(item, "cargo.qty", "qty", sd->item_quantity[j]);
                cJSON_AddItemToArray(cargo, item);
            }
        }
    }
    return obj;
}

cJSON *marshalSpecies(species_data_t *sp, nampla_data_t *npa, ship_data_t *sa) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        perror("cJSON_CreateObject:");
        fprintf(stderr, "error: species: unable to allocate root\n");
        exit(2);
    }
    jsonAddIntToObj(root, "root", "version", 1);
    cJSON *obj = cJSON_AddObjectToObject(root, "species");
    if (obj == 0) {
        fprintf(stderr, "error: species: unable to allocate property 'species'\n");
        exit(2);
    }
    const char *objName = "species";
    jsonAddIntToObj(obj, objName, "id", sp->id);
    jsonAddStringToObj(obj, objName, "name", sp->name);
    jsonAddIntToObj(obj, objName, "auto", sp->auto_orders);
    cJSON *government = cJSON_AddObjectToObject(obj, "government");
    if (government == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'government'\n");
        exit(2);
    } else {
        jsonAddStringToObj(government, "government", "name", sp->govt_name);
        jsonAddStringToObj(government, "government", "type", sp->govt_type);
    }
    cJSON *home_world = cJSON_AddObjectToObject(obj, "home_world");
    if (home_world == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'home_world'\n");
        exit(2);
    } else {
        jsonAddIntToObj(home_world, "home_world", "x", sp->x);
        jsonAddIntToObj(home_world, "home_world", "y", sp->y);
        jsonAddIntToObj(home_world, "home_world", "z", sp->z);
        jsonAddIntToObj(home_world, "home_world", "orbit", sp->pn);
        jsonAddIntToObj(home_world, "home_world", "hp_base", sp->hp_original_base);
    }
    cJSON *atmosphere = cJSON_AddObjectToObject(obj, "atmosphere");
    if (atmosphere == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'atmosphere'\n");
        exit(2);
    } else {
        cJSON *required = cJSON_AddObjectToObject(atmosphere, "required");
        if (required == 0) {
            perror("cJSON_AddObjectToObject:");
            fprintf(stderr, "error: species: unable to allocate property 'atmosphere.required'\n");
            exit(2);
        } else {
            jsonAddIntToObj(required, "atmosphere.required", "gas", sp->required_gas);
            jsonAddIntToObj(required, "atmosphere.required", "min", sp->required_gas_min);
            jsonAddIntToObj(required, "atmosphere.required", "max", sp->required_gas_max);
        }
        cJSON *neutral = cJSON_AddArrayToObject(atmosphere, "neutral");
        if (neutral == 0) {
            perror("cJSON_AddArrayToObject:");
            fprintf(stderr, "error: species: unable to allocate property 'atmosphere.neutral'\n");
            exit(2);
        } else {
            for (int j = 0; j < 6; j++) {
                jsonAddIntToArray(neutral, "atmosphere.neutral", sp->neutral_gas[j]);
            }
        }
        cJSON *poison = cJSON_AddArrayToObject(atmosphere, "poison");
        if (poison == 0) {
            perror("cJSON_AddArrayToObject:");
            fprintf(stderr, "error: species: unable to allocate property 'atmosphere.poison'\n");
            exit(2);
        } else {
            for (int j = 0; j < 6; j++) {
                jsonAddIntToArray(poison, "atmosphere.poison", sp->poison_gas[j]);
            }
        }
    }
    cJSON *technology = cJSON_AddObjectToObject(obj, "technology");
    if (technology == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'technology'\n");
        exit(2);
    }
    for (int j = 0; j < 6; j++) {
        cJSON *named = cJSON_AddObjectToObject(technology, tech_level_names[j]);
        if (named == 0) {
            perror("cJSON_AddObjectToObject:");
            fprintf(stderr, "error: species: unable to allocate property 'technology.%s'\n", tech_level_names[j]);
            exit(2);
        }
        jsonAddIntToObj(named, "technology", "level", sp->tech_level[j]);
        jsonAddIntToObj(named, "technology", "knowledge", sp->tech_knowledge[j]);
        jsonAddIntToObj(named, "technology", "init", sp->init_tech_level[j]);
        jsonAddIntToObj(named, "technology", "xp", sp->tech_eps[j]);
    }
    cJSON *fleet_maintenance = cJSON_AddObjectToObject(obj, "fleet_maintenance");
    if (fleet_maintenance == 0) {
        perror("cJSON_AddObjectToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'fleet_maintenance'\n");
        exit(2);
    }
    jsonAddIntToObj(fleet_maintenance, "fleet_maintenance", "cost", sp->fleet_cost);
    jsonAddIntToObj(fleet_maintenance, "fleet_maintenance", "percent", sp->fleet_percent_cost);
    jsonAddIntToObj(obj, objName, "banked_eu", sp->econ_units);
    cJSON *contacts = cJSON_AddArrayToObject(obj, "contacts");
    if (contacts == 0) {
        perror("cJSON_AddArrayToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'contacts'\n");
        exit(2);
    }
    for (int spidx = 0; spidx < MAX_SPECIES; spidx++) {
        if ((sp->contact[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(contacts, "contacts", spidx + 1);
        }
    }
    cJSON *allies = cJSON_AddArrayToObject(obj, "allies");
    if (allies == 0) {
        perror("cJSON_AddArrayToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'allies'\n");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->ally[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(allies, "allies", spidx + 1);
        }
    }
    cJSON *enemies = cJSON_AddArrayToObject(obj, "enemies");
    if (enemies == 0) {
        perror("cJSON_AddArrayToObject:");
        fprintf(stderr, "error: species: unable to allocate property 'enemies'\n");
        exit(2);
    }
    for (int spidx = 0; spidx < MAX_SPECIES; spidx++) {
        if ((sp->enemy[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(enemies, "enemies", spidx + 1);
        }
    }
    cJSON *namedPlanets = cJSON_AddArrayToObject(root, "named_planets");
    if (namedPlanets == 0) {
        fprintf(stderr, "species: unable to allocate property 'named_planets'\n");
        exit(2);
    }
    for (int i = 0; i < sp->num_namplas; i++) {
        if (!cJSON_AddItemToArray(namedPlanets, marshalNamedPlanet(npa + i))) {
            perror("cJSON_AddItemToArray:");
            fprintf(stderr, "error: species: unable to extend array 'named_planets'");
            exit(2);
        }
    }
    cJSON *ships = cJSON_AddArrayToObject(root, "ships");
    if (ships == 0) {
        fprintf(stderr, "species: unable to allocate property 'ships'\n");
        exit(2);
    }
    for (int i = 0; i < sp->num_ships; i++) {
        if (!cJSON_AddItemToArray(ships, marshalShip(sa + i))) {
            perror("cJSON_AddItemToArray:");
            fprintf(stderr, "error: species: unable to extend array 'ships'");
            exit(2);
        }
    }
    return root;
}

cJSON *marshalStar(star_data_t *sd) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        perror("cJSON_CreateObject:");
        fprintf(stderr, "error: star: unable to allocate object\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "star", "x", sd->x);
    jsonAddIntToObj(obj, "star", "y", sd->y);
    jsonAddIntToObj(obj, "star", "z", sd->z);
    char code[2];
    code[1] = 0;
    code[0] = star_type(sd->type);
    jsonAddStringToObj(obj, "star", "type", code);
    code[0] = star_color(sd->color);
    jsonAddStringToObj(obj, "star", "color", code);
    jsonAddIntToObj(obj, "star", "size", sd->size);
    jsonAddBoolToObj(obj, "star", "home_system", sd->home_system);
    cJSON *worm_hole = cJSON_AddObjectToObject(obj, "worm_hole");
    if (worm_hole == NULL) {
        fprintf(stderr, "error: star: unable to allocate property 'worm_hole'\n");
        perror("cJSON_AddObjectToObject:");
        exit(2);
    }
    jsonAddBoolToObj(worm_hole, "worm_hole", "here", sd->worm_here);
    jsonAddIntToObj(worm_hole, "worm_hole", "x", sd->worm_x);
    jsonAddIntToObj(worm_hole, "worm_hole", "y", sd->worm_y);
    jsonAddIntToObj(worm_hole, "worm_hole", "z", sd->worm_z);
    cJSON *visited_by = cJSON_AddArrayToObject(obj, "visited_by");
    if (visited_by == 0) {
        fprintf(stderr, "error: star: unable to allocate property 'visited_by'\n");
        perror("cJSON_AddArrayToObject:");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        // write the species only if it has visited this system
        if ((sd->visited_by[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(visited_by, "visited_by", spidx + 1);
        }
    }
    jsonAddIntToObj(obj, "star", "message", sd->message);
    jsonAddItemToObj(obj, "star", "planets", marshalPlanets(planet_base + sd->planet_index, sd->num_planets));
    return obj;
}

cJSON *marshalSystems(void) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: systems: unable to allocate root\n");
        exit(2);
    }
    jsonAddIntToObj(root, "root", "version", 1);
    cJSON *systems = cJSON_AddArrayToObject(root, "systems");
    if (systems == 0) {
        fprintf(stderr, "systems: unable to allocate property 'systems'\n");
        exit(2);
    }
    for (int i = 0; i < num_stars; i++) {
        if (!cJSON_AddItemToArray(systems, marshalStar(&star_base[i]))) {
            perror("cJSON_AddItemToArray:");
            fprintf(stderr, "error: stars: unable to extend array 'systems'");
            exit(2);
        }
    }
    return root;
}

