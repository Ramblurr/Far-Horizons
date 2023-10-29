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
#include "cjson/helpers.h"
#include "engine.h"
#include "galaxyio.h"
#include "planetio.h"
#include "speciesvars.h"
#include "stario.h"

static cJSON *marshalAtmosphericGas(int code, int pct);

static cJSON *marshalAtmosphericGases(int *code, int *pct);

static cJSON *marshalConsUnits(int num_needed, int num_auto, int num_install);

static cJSON *marshalCoords(int x, int y, int z);

static cJSON *marshalCoordsWithOrbit(int x, int y, int z, int orbit);

static cJSON *marshalGalaxy(galaxy_data_t *g);

static cJSON *marshalGases(int *code);

static cJSON *marshalGovernment(const char *name, const char *type);

static cJSON *marshalItem(int code, int qty);

static cJSON *marshalItems(int *items);

static cJSON *marshalMiningDifficulty(int base, int increase);

static cJSON *marshalNamedPlanet(nampla_data_t *npd);

static cJSON *marshalNamedPlanets(nampla_data_t *npa, int num);

static cJSON *marshalPlanet(planet_data_t *pd);

static cJSON *marshalPlanets(planet_data_t *planets, int num);

static cJSON *marshalRequiredAtmosphericGas(int code, int min_pct, int max_pct);

static cJSON *marshalShip(ship_data_t *sd);

static cJSON *marshalShips(ship_data_t *sa, int num);

static cJSON *marshalSpeciesAtmosphere(cJSON *required, cJSON *neutral, cJSON *poison);

static cJSON *marshalSpeciesBitfield(uint32_t *bits);

static cJSON *marshalSystem(star_data_t *sd);

static cJSON *marshalTechnologies(const char **codes, int *levels, int *knowledge, int *xp);

static cJSON *marshalTechnology(int level, int knowledge, int xp);

static cJSON *marshalVersion(int version);

cJSON *marshalAtmosphericGas(int code, int pct) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL) {
        fprintf(stderr, "error: gas: unable to allocate memory\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "gas", "code", code);
    jsonAddIntToObj(obj, "gas", "percent", pct);
    return obj;
}

cJSON *marshalAtmosphericGases(int *code, int *pct) {
    cJSON *array = cJSON_CreateArray();
    if (array == NULL) {
        fprintf(stderr, "error: atmosphere: unable to allocate memory\n");
        exit(2);
    }
    for (int i = 0; i < 4; i++) {
        if (cJSON_AddItemToArray(array, marshalAtmosphericGas(code[i], pct[i])) == 0) {
            fprintf(stderr, "error: atmosphere: unable to extend array\n");
            exit(2);
        }
    }
    return array;
}

cJSON *marshalConsUnits(int num_needed, int num_auto, int num_install) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL) {
        fprintf(stderr, "error: cons_units: unable to allocate memory\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "needed", (double) (num_needed)) == 0) {
        fprintf(stderr, "error: cons_units: unable to add property 'needed'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "auto", (double) (num_auto)) == 0) {
        fprintf(stderr, "error: cons_units: unable to add property 'needed'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "install", (double) (num_install)) == 0) {
        fprintf(stderr, "error: cons_units: unable to add property 'install'\n");
        exit(2);
    }
    return obj;
}

cJSON *marshalCoords(int x, int y, int z) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: coords: unable to allocate object\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "x", (double) (x)) == 0) {
        fprintf(stderr, "error: coords: unable to add property 'x'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "y", (double) (y)) == 0) {
        fprintf(stderr, "error: coords: unable to add property 'y'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "z", (double) (z)) == 0) {
        fprintf(stderr, "error: coords: unable to add property 'z'\n");
        exit(2);
    }
    return obj;
}

cJSON *marshalCoordsWithOrbit(int x, int y, int z, int orbit) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: coords_with_orbit: unable to allocate object\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "x", (double) (x)) == 0) {
        fprintf(stderr, "error: coords_with_orbit: unable to add property 'x'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "y", (double) (y)) == 0) {
        fprintf(stderr, "error: coords_with_orbit: unable to add property 'y'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "z", (double) (z)) == 0) {
        fprintf(stderr, "error: coords_with_orbit: unable to add property 'z'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "orbit", (double) (orbit)) == 0) {
        fprintf(stderr, "error: coords_with_orbit: unable to add property 'orbit'\n");
        exit(2);
    }
    return obj;
}

cJSON *marshalGalaxy(galaxy_data_t *g) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: galaxy: property 'galaxy': unable to allocate\n");
        exit(2);
    }
    jsonAddIntToObj(root, "galaxy", "turn_number", galaxy.turn_number);
    jsonAddIntToObj(root, "galaxy", "num_species", galaxy.num_species);
    jsonAddIntToObj(root, "galaxy", "d_num_species", galaxy.d_num_species);
    jsonAddIntToObj(root, "galaxy", "radius", galaxy.radius);
    return root;
}

cJSON *marshalGalaxyFile(void) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: galaxy: unable to allocate root\n");
        exit(2);
    }
    cJSON_AddItemToObject(root, "version", marshalVersion(1));
    cJSON_AddItemToObject(root, "galaxy", marshalGalaxy(&galaxy));
    return root;
}

cJSON *marshalGases(int *code) {
    cJSON *obj = cJSON_CreateArray();
    if (obj == NULL) {
        fprintf(stderr, "error: gases: unable to allocate memory\n");
        exit(2);
    }
    for (int i = 0; i < 6; i++) {
        if (cJSON_AddItemToArray(obj, cJSON_CreateNumber((double) code[i])) == 0) {
            fprintf(stderr, "error: gases: unable to extend array\n");
            exit(2);
        }
    }
    return obj;
}

cJSON *marshalGovernment(const char *name, const char *type) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL) {
        fprintf(stderr, "error: government: unable to allocate memory\n");
        exit(2);
    }
    jsonAddStringToObj(obj, "government", "name", name);
    jsonAddStringToObj(obj, "government", "type", type);
    return obj;
}

cJSON *marshalItem(int code, int qty) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL) {
        fprintf(stderr, "error: item: unable to allocate memory\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "code", (double) (code)) == 0) {
        fprintf(stderr, "error: item: unable to add property 'code'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "qty", (double) (qty)) == 0) {
        fprintf(stderr, "error: item: unable to add property 'qty'\n");
        exit(2);
    }
    return obj;
}

cJSON *marshalItems(int *items) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: items: unable to allocate array\n");
    }
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i] > 0) {
            cJSON_AddItemToArray(array, marshalItem(i, items[i]));
        }
    }
    return array;
}

cJSON *marshalMiningDifficulty(int base, int increase) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: mining_difficulty: unable to allocate object\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "mining_difficulty", "base", base);
    jsonAddIntToObj(obj, "mining_difficulty", "increase", increase);
    return obj;
}

cJSON *marshalNamedPlanet(nampla_data_t *npd) {
    char *objName = "named_planet";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: named_planet: unable to allocate object\n");
        exit(2);
    }
    jsonAddStringToObj(obj, objName, "name", npd->name);
    cJSON_AddItemToObject(obj, "location", marshalCoordsWithOrbit(npd->x, npd->y, npd->z, npd->pn));
    jsonAddIntToObj(obj, objName, "status", npd->status);
    jsonAddIntToObj(obj, objName, "hiding", npd->hiding);
    jsonAddIntToObj(obj, objName, "hidden", npd->hidden);
    jsonAddIntToObj(obj, objName, "siege_eff", npd->siege_eff);
    jsonAddIntToObj(obj, objName, "shipyards", npd->shipyards);
    cJSON_AddItemToObject(obj, "ius", marshalConsUnits(npd->IUs_needed, npd->auto_IUs, npd->IUs_to_install));
    cJSON_AddItemToObject(obj, "aus", marshalConsUnits(npd->AUs_needed, npd->auto_AUs, npd->AUs_to_install));
    jsonAddIntToObj(obj, objName, "mi_base", npd->mi_base);
    jsonAddIntToObj(obj, objName, "ma_base", npd->ma_base);
    jsonAddIntToObj(obj, objName, "pop_units", npd->pop_units);
    cJSON_AddItemToObject(obj, "items", marshalItems(npd->item_quantity));
    jsonAddIntToObj(obj, objName, "use_on_ambush", npd->use_on_ambush);
    jsonAddIntToObj(obj, objName, "message", npd->message);
    jsonAddIntToObj(obj, objName, "special", npd->special);
    return obj;
}

cJSON *marshalNamedPlanets(nampla_data_t *npa, int num) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: named_planets: unable to allocate memory\n");
        exit(2);
    }
    for (int i = 0; i < num; i++) {
        if (!cJSON_AddItemToArray(array, marshalNamedPlanet(npa + i))) {
            fprintf(stderr, "error: species: unable to extend array 'named_planets'");
            exit(2);
        }
    }
    return array;
}

cJSON *marshalPlanet(planet_data_t *pd) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: planet: unable to allocate object\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "planet", "temperature_class", pd->temperature_class);
    jsonAddIntToObj(obj, "planet", "pressure_class", pd->pressure_class);
    jsonAddIntToObj(obj, "planet", "special", pd->special);
    cJSON_AddItemToObject(obj, "atmosphere", marshalAtmosphericGases(pd->gas, pd->gas_percent));
    jsonAddIntToObj(obj, "planet", "diameter", pd->diameter);
    jsonAddIntToObj(obj, "planet", "gravity", pd->gravity);
    cJSON_AddItemToObject(obj, "mining_difficulty", marshalMiningDifficulty(pd->mining_difficulty, pd->md_increase));
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

cJSON *marshalRequiredAtmosphericGas(int code, int min_pct, int max_pct) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: required_atmospheric_gas: unable to allocate object\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "code", (double) (code)) == 0) {
        fprintf(stderr, "error: required_atmospheric_gas: unable to add property 'code'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "min_pct", (double) (min_pct)) == 0) {
        fprintf(stderr, "error: required_atmospheric_gas: unable to add property 'min_pct'\n");
        exit(2);
    }
    if (cJSON_AddNumberToObject(obj, "max_pct", (double) (max_pct)) == 0) {
        fprintf(stderr, "error: required_atmospheric_gas: unable to add property 'max_pct'\n");
        exit(2);
    }
    return obj;
}

cJSON *marshalShip(ship_data_t *sd) {
    char *objName = "ship";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: ship: unable to allocate object\n");
        exit(2);
    }
    jsonAddStringToObj(obj, objName, "name", sd->name);
    cJSON_AddItemToObject(obj, "location", marshalCoordsWithOrbit(sd->x, sd->y, sd->z, sd->pn));
    jsonAddIntToObj(obj, objName, "status", sd->status);
    jsonAddIntToObj(obj, objName, "type", sd->type);
    cJSON_AddItemToObject(obj, "dest", marshalCoords(sd->dest_x, sd->dest_y, sd->dest_z));
    jsonAddIntToObj(obj, objName, "just_jumped", sd->just_jumped);
    jsonAddBoolToObj(obj, objName, "arrived_via_wormhole", sd->arrived_via_wormhole);
    jsonAddIntToObj(obj, objName, "class", sd->class);
    jsonAddIntToObj(obj, objName, "tonnage", sd->tonnage);
    cJSON_AddItemToObject(obj, "cargo", marshalItems(sd->item_quantity));
    jsonAddIntToObj(obj, objName, "age", sd->age);
    jsonAddIntToObj(obj, objName, "remaining_cost", sd->remaining_cost);
    jsonAddIntToObj(obj, objName, "loading_point", sd->loading_point);
    jsonAddIntToObj(obj, objName, "unloading_point", sd->unloading_point);
    jsonAddIntToObj(obj, objName, "special", sd->special);
    return obj;
}

cJSON *marshalShips(ship_data_t *sa, int num) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: ships: unable to allocate memory\n");
        exit(2);
    }
    for (int i = 0; i < num; i++) {
        if (!cJSON_AddItemToArray(array, marshalShip(sa + i))) {
            fprintf(stderr, "error: ships: unable to extend array");
            exit(2);
        }
    }
    return array;
}

cJSON *marshalSpecies(species_data_t *sp) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: species: unable to allocate property 'species'\n");
        exit(2);
    }
    const char *objName = "species";
    jsonAddIntToObj(root, objName, "id", sp->id);
    jsonAddStringToObj(root, objName, "name", sp->name);
    cJSON_AddItemToObject(root, "government", marshalGovernment(sp->govt_name, sp->govt_type));
    cJSON_AddItemToObject(root, "home_world", marshalCoordsWithOrbit(sp->x, sp->y, sp->z, sp->pn));
    cJSON_AddItemToObject(root, "atmosphere", marshalSpeciesAtmosphere(
            marshalRequiredAtmosphericGas(sp->required_gas, sp->required_gas_min, sp->required_gas_max),
            marshalGases(sp->neutral_gas),
            marshalGases(sp->poison_gas)));
    jsonAddBoolToObj(root, objName, "auto_orders", sp->auto_orders);
    cJSON_AddItemToObject(root, "tech",
                          marshalTechnologies(tech_level_names, sp->tech_level, sp->tech_knowledge, sp->tech_eps));
    jsonAddIntToObj(root, objName, "hp_original_base", sp->hp_original_base);
    jsonAddIntToObj(root, objName, "econ_units", sp->econ_units);
    cJSON_AddItemToObject(root, "contacts", marshalSpeciesBitfield(sp->contact));
    cJSON_AddItemToObject(root, "allies", marshalSpeciesBitfield(sp->ally));
    cJSON_AddItemToObject(root, "enemies", marshalSpeciesBitfield(sp->enemy));
    return root;
}

cJSON *marshalSpeciesAtmosphere(cJSON *required, cJSON *neutral, cJSON *poison) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: species: unable to allocate atmosphere\n");
        exit(2);
    }
    cJSON_AddItemToObject(root, "required", required);
    cJSON_AddItemToObject(root, "neutral", neutral);
    cJSON_AddItemToObject(root, "poison", poison);
    return root;
}

cJSON *marshalSpeciesBitfield(uint32_t *bits) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: species_bits: unable to allocate memory\n");
        exit(2);
    }
    for (int spidx = 0; spidx < MAX_SPECIES; spidx++) {
        if ((bits[spidx / 32] & (1 << (spidx % 32))) != 0) {
            if (cJSON_AddItemToArray(array, cJSON_CreateNumber((double)(spidx + 1))) == 0) {
                fprintf(stderr, "error: species_bits: unable to extend array\n");
                exit(2);
            }
        }
    }
    return array;
}

cJSON *marshalSpeciesFile(species_data_t *sp, nampla_data_t *npa, ship_data_t *sa) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: species: unable to allocate root\n");
        exit(2);
    }
    cJSON_AddItemToObject(root, "version", marshalVersion(1));
    cJSON_AddItemToObject(root, "species", marshalSpecies(sp));
    cJSON_AddItemToObject(root, "named_planets", marshalNamedPlanets(npa, sp->num_namplas));
    cJSON_AddItemToObject(root, "ships", marshalShips(sa, sp->num_ships));
    return root;
}

cJSON *marshalSystem(star_data_t *sd) {
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "error: system: unable to allocate object\n");
        exit(2);
    }
    jsonAddIntToObj(obj, "system", "x", sd->x);
    jsonAddIntToObj(obj, "system", "y", sd->y);
    jsonAddIntToObj(obj, "system", "z", sd->z);
    char code[2];
    code[1] = 0;
    code[0] = star_type(sd->type);
    jsonAddStringToObj(obj, "system", "type", code);
    code[0] = star_color(sd->color);
    jsonAddStringToObj(obj, "system", "color", code);
    jsonAddIntToObj(obj, "system", "size", sd->size);
    jsonAddBoolToObj(obj, "system", "home_system", sd->home_system);
    jsonAddBoolToObj(obj, "system", "worm_here", sd->worm_here);
    jsonAddIntToObj(obj, "system", "worm_x", sd->worm_x);
    jsonAddIntToObj(obj, "system", "worm_y", sd->worm_y);
    jsonAddIntToObj(obj, "system", "worm_z", sd->worm_z);
    cJSON_AddItemToObject(obj, "visited_by", marshalSpeciesBitfield(sd->visited_by));
    jsonAddIntToObj(obj, "system", "message", sd->message);
    jsonAddItemToObj(obj, "system", "planets", marshalPlanets(planet_base + sd->planet_index, sd->num_planets));
    return obj;
}

cJSON *marshalSystems(star_data_t *sa, int num) {
    cJSON *array = cJSON_CreateArray();
    if (array == 0) {
        fprintf(stderr, "error: systems: property 'sysmtems': unable to allocate\n");
        exit(2);
    }
    for (int i = 0; i < num; i++) {
        if (!cJSON_AddItemToArray(array, marshalSystem(sa + i))) {
            fprintf(stderr, "error: systems: unable to extend array 'systems'");
            exit(2);
        }
    }
    return array;
}

cJSON *marshalSystemsFile(void) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: systems: unable to allocate root\n");
        exit(2);
    }
    cJSON_AddItemToObject(root, "version", marshalVersion(1));
    cJSON_AddItemToObject(root, "systems", marshalSystems(star_base, num_stars));
    return root;
}

cJSON *marshalTechnologies(const char **codes, int *levels, int *knowledge, int *xp) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: technologies: unable to allocate memory\n");
        exit(2);
    }
    for (int i = 0; i < 6; i++) {
        cJSON_AddItemToObject(root, codes[i], marshalTechnology(levels[i], knowledge[i], xp[i]));
    }
    return root;
}

cJSON *marshalTechnology(int level, int knowledge, int xp) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "error: technology: unable to allocate memory\n");
        exit(2);
    }
    cJSON_AddNumberToObject(root, "level", (double) level);
    cJSON_AddNumberToObject(root, "knowledge", (double) knowledge);
    cJSON_AddNumberToObject(root, "xp", (double) xp);
    return root;
}


cJSON *marshalVersion(int version) {
    cJSON *elem = cJSON_CreateNumber((double) version);
    if (elem == 0) {
        fprintf(stderr, "error: version: unable to allocate memory\n");
        exit(2);
    }
    return elem;
}