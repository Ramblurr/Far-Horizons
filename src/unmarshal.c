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

#include <stdlib.h>
#include <string.h>
#include "unmarshal.h"
#include "cjson/helpers.h"
#include "stario.h"
#include "planetio.h"
#include "speciesvars.h"

static void unmarshalAtmosphericGas(cJSON *obj, int *code, int *pct);

static void unmarshalAtmosphericGases(cJSON *obj, int *codes, int *pcts);

static void unmarshalConsUnits(cJSON *obj, int *num_needed, int *num_autos, int *num_install);

static void unmarshalCoords(cJSON *root, int *x, int *y, int *z);

static void unmarshalCoordsWithOrbit(cJSON *root, int *x, int *y, int *z, int *orbit);

static void unmarshalGalaxy(cJSON *root, galaxy_data_t *g);

static void unmarshalGases(cJSON *array, int *gases);

static void unmarshalGovernment(cJSON *root, species_data_t *sp);

static void unmarshalItems(cJSON *array, int *items);

static void unmarshalMiningDifficulty(cJSON *obj, int *base, int *increase);

static void unmarshalNamedPlanets(cJSON *array, species_data_t *sp, nampla_data_t *npa);

static void unmarshalPlanet(cJSON *root, planet_data_t *pd);

static int unmarshalPlanets(cJSON *root, star_data_t *sd, planet_data_t *pa);

static void unmarshalRequireAtmosphericGas(cJSON *root, int *code, int *min_pct, int *max_pct);

static void unmarshalShip(cJSON *root, ship_data_t *sd);

static void unmarshalShips(cJSON *array, species_data_t *sp, ship_data_t *sa);

static void unmarshalSpecies(cJSON *root, species_data_t *sp);

static void unmarshalSpeciesAtmosphere(cJSON *root, species_data_t *sp);

static void unmarshalSpeciesBitfield(cJSON *array, uint32_t *bits);

static int unmarshalStarColor(cJSON *obj);

static int unmarshalStarType(cJSON *obj);

static void unmarshalSystem(cJSON *root, star_data_t *sd, planet_data_t *pa);

static void unmarshalTechnologies(cJSON *root, const char **codes, int *levels, int *knowledge, int *xp, int *init_levels);

static int unmarshalVersion(cJSON *elem);

void unmarshalAtmosphericGas(cJSON *obj, int *code, int *pct) {
    if (obj == 0) {
        *code = 0;
        *pct = 0;
    } else if (!cJSON_IsObject(obj)) {
        fprintf(stderr, "error: gas: element must be an object\n");
        exit(2);
    }
    *code = jsonGetInt(obj, "code");
    *pct = jsonGetInt(obj, "percent");
}

void unmarshalAtmosphericGases(cJSON *obj, int *codes, int *pcts) {
    if (obj == 0) {
        for (int i = 0; i < 4; i++) {
            codes[i] = 0;
            pcts[i] = 0;
        }
    } else if (!cJSON_IsArray(obj)) {
        fprintf(stderr, "error: atmosphere: property must be an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(obj) != 4) {
        fprintf(stderr, "error: atmosphere: want 4 elements, got %d\n", cJSON_GetArraySize(obj));
        exit(2);
    }
    int gas_index = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, obj) {
        unmarshalAtmosphericGas(elem, codes + gas_index, pcts + gas_index);
        gas_index++;
    }
}

void unmarshalConsUnits(cJSON *obj, int *num_needed, int *num_autos, int *num_install) {
    if (obj == 0) {
        fprintf(stderr, "error: cons_units: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(obj)) {
        fprintf(stderr, "error: cons_units: property must be an object\n");
        exit(2);
    }
    *num_needed = jsonGetInt(obj, "needed");
    *num_autos = jsonGetInt(obj, "auto");
    *num_install = jsonGetInt(obj, "install");
}

void unmarshalCoords(cJSON *root, int *x, int *y, int *z) {
    if (root == 0) {
        fprintf(stderr, "error: coords: element must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: coords: element must be an object\n");
        exit(2);
    }
    *x = jsonGetInt(root, "x");
    *y = jsonGetInt(root, "y");
    *z = jsonGetInt(root, "z");
}

void unmarshalCoordsWithOrbit(cJSON *root, int *x, int *y, int *z, int *orbit) {
    if (root == 0) {
        fprintf(stderr, "error: coords_with_orbit: element must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: coords_with_orbit: element must be an object\n");
        exit(2);
    }
    *x = jsonGetInt(root, "x");
    *y = jsonGetInt(root, "y");
    *z = jsonGetInt(root, "z");
    *orbit = jsonGetInt(root, "orbit");
}

void unmarshalGalaxy(cJSON *root, galaxy_data_t *g) {
    if (root == 0) {
        fprintf(stderr, "error: galaxy: property 'galaxy' must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: galaxy: property 'galaxy' must be an object\n");
        exit(2);
    }
    cJSON *prop;
    prop = cJSON_GetObjectItemCaseSensitive(root, "d_num_species");
    if (prop != 0 && cJSON_IsNumber(prop)) {
        g->d_num_species = prop->valueint;
    }
    prop = cJSON_GetObjectItemCaseSensitive(root, "num_species");
    if (prop != 0 && cJSON_IsNumber(prop)) {
        g->num_species = prop->valueint;
    }
    prop = cJSON_GetObjectItemCaseSensitive(root, "radius");
    if (prop != 0 && cJSON_IsNumber(prop)) {
        g->radius = prop->valueint;
    }
    prop = cJSON_GetObjectItemCaseSensitive(root, "turn_number");
    if (prop != 0 && cJSON_IsNumber(prop)) {
        g->turn_number = prop->valueint;
    }
}

void unmarshalGalaxyFile(cJSON *root, galaxy_data_t *g) {
    int version = unmarshalVersion(cJSON_GetObjectItem(root, "version"));
    if (version != 1) {
        fprintf(stderr, "error: galaxy: version: want 1, got %d\n", version);
        exit(2);
    }
    unmarshalGalaxy(cJSON_GetObjectItem(root, "galaxy"), g);
}

void unmarshalGases(cJSON *array, int *gases) {
    if (array == 0) {
        fprintf(stderr, "error: gases: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "error: gases: property must be an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(array) != 6) {
        fprintf(stderr, "error: gases: array must contain exactly 6 elements\n");
        exit(2);
    }
    int i = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, array) {
        if (!cJSON_IsNumber(elem)) {
            fprintf(stderr, "error: gases: elements must be integers\n");
            exit(2);
        }
        gases[i] = elem->valueint;
        i = i + 1;
    }
}

void unmarshalGovernment(cJSON *root, species_data_t *sp) {
    if (root == 0) {
        fprintf(stderr, "error: species: property 'government' must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: species: property 'government' must be an object\n");
        exit(2);
    }
    jsonGetString(root, "name", sp->govt_name, sizeof(sp->govt_name));
    jsonGetString(root, "type", sp->govt_type, sizeof(sp->govt_type));
}

void unmarshalItems(cJSON *array, int *items) {
    if (array == 0) {
        fprintf(stderr, "error: items: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "error: items: property must be an array\n");
        exit(2);
    }
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, array) {
        if (!cJSON_IsObject(elem)) {
            fprintf(stderr, "error: items: all elements must be objects\n");
            exit(2);
        }
        int code = jsonGetInt(elem, "code");
        if (0 <= code && code < MAX_ITEMS) {
            items[code] = jsonGetInt(elem, "qty");
        }
    }
}

void unmarshalMiningDifficulty(cJSON *obj, int *base, int *increase) {
    if (obj == 0) {
        fprintf(stderr, "error: mining_difficulty: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(obj)) {
        fprintf(stderr, "error: mining_difficulty: element must be an object\n");
        exit(2);
    }
    *base = jsonGetInt(obj, "base");
    *increase = jsonGetInt(obj, "increase");
}

void unmarshalNamedPlanet(cJSON *root, nampla_data_t *npd) {
    if (root == 0) {
        fprintf(stderr, "error: named_planet: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: named_planet: property must be an object\n");
        exit(2);
    }
    jsonGetString(root, "name", npd->name, sizeof(npd->name));
    unmarshalCoordsWithOrbit(cJSON_GetObjectItem(root, "location"), &npd->x, &npd->y, &npd->z, &npd->pn);
    npd->status = jsonGetInt(root, "status");
    npd->hiding = jsonGetInt(root, "hiding");
    npd->hidden = jsonGetInt(root, "hidden");
    npd->siege_eff = jsonGetInt(root, "siege_eff");
    npd->shipyards = jsonGetInt(root, "shipyards");
    unmarshalConsUnits(cJSON_GetObjectItem(root, "ius"), &npd->IUs_needed, &npd->auto_IUs, &npd->IUs_to_install);
    unmarshalConsUnits(cJSON_GetObjectItem(root, "aus"), &npd->AUs_needed, &npd->auto_AUs, &npd->AUs_to_install);
    npd->mi_base = jsonGetInt(root, "mi_base");
    npd->ma_base = jsonGetInt(root, "ma_base");
    npd->pop_units = jsonGetInt(root, "pop_units");
    npd->use_on_ambush = jsonGetInt(root, "use_on_ambush");
    unmarshalItems(cJSON_GetObjectItem(root, "items"), npd->item_quantity);
    npd->message = jsonGetInt(root, "message");
    npd->special = jsonGetInt(root, "special");

    // find star and planet
    for (int i = 0; i < num_stars; i++) {
        star_data_t *sd = star_base + i;
        if (sd->x == npd->x && sd->y == npd->y && sd->z == npd->z) {
            npd->star = sd;
            if (!(0 < npd->pn && npd->pn <= sd->num_planets)) {
                fprintf(stderr, "error: named_planet: orbit %d is not in range 1..%d\n", npd->pn, sd->num_planets);
                exit(2);
            }
            npd->planet = planet_base + sd->planet_index + npd->pn - 1;
            npd->planet_index = npd->planet->index;
        }
    }
}

void unmarshalNamedPlanets(cJSON *array, species_data_t *sp, nampla_data_t *npa) {
    if (array == 0) {
        fprintf(stderr, "error: named_planets: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "error: named_planets: property must be an array\n");
        exit(2);
    }
    sp->num_namplas = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, array) {
        nampla_data_t *npd = npa + sp->num_namplas;
        memset(npd, 0, sizeof(nampla_data_t));
        npd->id = sp->num_namplas + 1;
        unmarshalNamedPlanet(elem, npd);
        sp->num_namplas++;
    }
}

void unmarshalPlanet(cJSON *root, planet_data_t *pd) {
    if (root == 0) {
        fprintf(stderr, "error: planets: planet element must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: planets: planet element must be an object\n");
        exit(2);
    }
    pd->temperature_class = jsonGetInt(root, "temperature_class");
    pd->pressure_class = jsonGetInt(root, "pressure_class");
    pd->special = jsonGetInt(root, "special");
    unmarshalAtmosphericGases(cJSON_GetObjectItem(root, "atmosphere"), pd->gas, pd->gas_percent);
    pd->diameter = jsonGetInt(root, "diameter");
    pd->gravity = jsonGetInt(root, "gravity");
    unmarshalMiningDifficulty(cJSON_GetObjectItem(root, "mining_difficulty"), &pd->mining_difficulty, &pd->md_increase);
    pd->econ_efficiency = jsonGetInt(root, "econ_efficiency");
    pd->isValid = 0;
    pd->message = jsonGetInt(root, "message");
}

int unmarshalPlanets(cJSON *root, star_data_t *sd, planet_data_t *pa) {
    if (root == 0) {
        return 0;
    } else if (!cJSON_IsArray(root)) {
        fprintf(stderr, "error: planets: property must be an array\n");
        exit(2);
    }
    int orbit = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, root) {
        planet_data_t *pd = pa + orbit;
        pd->id = sd->planet_index + orbit + 1;
        pd->index = sd->planet_index + orbit;
        pd->star = sd;
        pd->orbit = orbit + 1;
        unmarshalPlanet(elem, pd);
        pd->isValid = TRUE;
        orbit = orbit + 1;
    }
    return orbit;
}

void unmarshalRequireAtmosphericGas(cJSON *root, int *code, int *min_pct, int *max_pct) {
    if (root == 0) {
        fprintf(stderr, "error: species: required_gas must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: species: required_gas must be an object\n");
        exit(2);
    }
    *code = jsonGetInt(root, "code");
    *min_pct = jsonGetInt(root, "min_pct");
    *max_pct = jsonGetInt(root, "max_pct");
}

void unmarshalShip(cJSON *root, ship_data_t *sd) {
    if (root == 0) {
        fprintf(stderr, "error: ship: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: ship: property must be an object\n");
        exit(2);
    }
    jsonGetString(root, "name", sd->name, sizeof(sd->name));
    unmarshalCoordsWithOrbit(cJSON_GetObjectItem(root, "location"), &sd->x, &sd->y, &sd->z, &sd->pn);
    sd->status = jsonGetInt(root, "status");
    sd->type = jsonGetInt(root, "type");
    unmarshalCoords(cJSON_GetObjectItem(root, "dest"), &sd->dest_x, &sd->dest_y, &sd->dest_z);
    sd->just_jumped = jsonGetInt(root, "just_jumped");
    sd->arrived_via_wormhole = jsonGetBool(root, "arrived_via_wormhole");
    sd->class = jsonGetInt(root, "class");
    sd->tonnage = jsonGetInt(root, "tonnage");
    unmarshalItems(cJSON_GetObjectItem(root, "cargo"), sd->item_quantity);
    sd->age = jsonGetInt(root, "age");
    sd->remaining_cost = jsonGetInt(root, "remaining_cost");
    sd->loading_point = jsonGetInt(root, "loading_point");
    sd->unloading_point = jsonGetInt(root, "unloading_point");
    sd->special = jsonGetInt(root, "special");
}

void unmarshalShips(cJSON *array, species_data_t *sp, ship_data_t *sa) {
    if (array == 0) {
        fprintf(stderr, "error: ships: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "error: ships: property must be an array\n");
        exit(2);
    }
    sp->num_ships = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, array) {
        ship_data_t *sd = sa + sp->num_ships;
        memset(sd, 0, sizeof(ship_data_t));
        sd->id = sp->num_ships + 1;
        unmarshalShip(elem, sd);
        sp->num_ships++;
    }
}

void unmarshalSpecies(cJSON *root, species_data_t *sp) {
    if (root == 0) {
        fprintf(stderr, "error: species: property 'species' must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "species: property 'species' must be an object\n");
        exit(2);
    }
    memset(sp, 0, sizeof(species_data_t));
    sp->id = jsonGetInt(root, "id");
    jsonGetString(root, "name", sp->name, sizeof(sp->name));
    unmarshalGovernment(cJSON_GetObjectItem(root, "government"), sp);
    unmarshalCoordsWithOrbit(cJSON_GetObjectItem(root, "home_world"), &sp->x, &sp->y, &sp->z, &sp->pn);
    unmarshalSpeciesAtmosphere(cJSON_GetObjectItem(root, "atmosphere"), sp);
    sp->auto_orders = jsonGetBool(root, "auto_orders");
    unmarshalTechnologies(cJSON_GetObjectItem(root, "tech"), tech_level_names, sp->tech_level, sp->tech_knowledge, sp->tech_eps, sp->init_tech_level);
    sp->hp_original_base = jsonGetInt(root, "hp_original_base");
    sp->econ_units = jsonGetInt(root, "econ_units");
    sp->fleet_cost = jsonGetInt(root, "fleet_cost");
    sp->fleet_percent_cost = jsonGetInt(root, "fleet_percent_cost");
    unmarshalSpeciesBitfield(cJSON_GetObjectItem(root, "contacts"), sp->contact);
    unmarshalSpeciesBitfield(cJSON_GetObjectItem(root, "allies"), sp->ally);
    unmarshalSpeciesBitfield(cJSON_GetObjectItem(root, "enemies"), sp->enemy);
}

void unmarshalSpeciesAtmosphere(cJSON *root, species_data_t *sp) {
    if (root == 0) {
        fprintf(stderr, "error: species: property 'atmosphere' must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "species: property 'atmosphere' must be an object\n");
        exit(2);
    }
    unmarshalRequireAtmosphericGas(cJSON_GetObjectItem(root, "required"), &sp->required_gas, &sp->required_gas_min, &sp->required_gas_max);
    unmarshalGases(cJSON_GetObjectItem(root, "neutral"), sp->neutral_gas);
    unmarshalGases(cJSON_GetObjectItem(root, "poison"), sp->poison_gas);
}

void unmarshalSpeciesBitfield(cJSON *array, uint32_t *bits) {
    if (array == 0) {
        fprintf(stderr, "error: species_bits: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "error: species_bits: property must be an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(array) == 0) {
        // nothing to do
        return;
    }
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, array) {
        if (!cJSON_IsNumber(elem)) {
            fprintf(stderr, "error: species_bits: elements must be numeric\n");
            exit(2);
        }
        int alien = elem->valueint;        // alien is 1..MAX_SPECIES
        if (!(0 < alien && alien <= MAX_SPECIES)) {
            fprintf(stderr, "error: species_bits: elements must be integers in range 1..%d\n", MAX_SPECIES);
            exit(2);
        }
        int word = (alien - 1) / 32;
        int bit = (alien - 1) % 32;
        long mask = 1 << bit;
        bits[word] |= mask;
    }
}

void unmarshalSpeciesFile(cJSON *root, species_data_t *sp, nampla_data_t *npa, ship_data_t *sa) {
    int version = unmarshalVersion(cJSON_GetObjectItem(root, "version"));
    if (version != 1) {
        fprintf(stderr, "error: species: version: want 1, got %d\n", version);
        exit(2);
    }
    unmarshalSpecies(cJSON_GetObjectItem(root, "species"), sp);
    unmarshalNamedPlanets(cJSON_GetObjectItem(root, "named_planets"), sp, npa);
    unmarshalShips(cJSON_GetObjectItem(root, "ships"), sp, sa);
    sp->home.nampla = npa;
    sp->home.planet = sp->home.nampla->planet;
    sp->home.star = sp->home.nampla->star;
    if (sp->home.planet->star != sp->home.nampla->star) {
        fprintf(stderr, "error: species: %d: home.planet.star != home.nampla.star\n", sp->id);
        exit(2);
    }
    printf(" info: species %3d: name %s planet %s\n", sp->id, sp->name, sp->home.nampla->name);
}

int unmarshalStarColor(cJSON *obj) {
    if (obj == 0 || !cJSON_IsString(obj)) {
        return 0;
    }
    const char *code = obj->valuestring;
    if (strlen(code) != 1) {
        return 0;
    }
    return chToStarColor(code[0]);
}

int unmarshalStarType(cJSON *obj) {
    if (obj == 0 || !cJSON_IsString(obj)) {
        return 0;
    }
    const char *code = obj->valuestring;
    if (strlen(code) != 1) {
        return 0;
    }
    return chToStarType(code[0]);
}

void unmarshalSystem(cJSON *root, star_data_t *sd, planet_data_t *pa) {
    sd->x = jsonGetInt(root, "x");
    sd->y = jsonGetInt(root, "y");
    sd->z = jsonGetInt(root, "z");
    sd->type = unmarshalStarType(cJSON_GetObjectItem(root, "type"));
    sd->color = unmarshalStarColor(cJSON_GetObjectItem(root, "color"));
    sd->size = jsonGetInt(root, "size");
    sd->num_planets = unmarshalPlanets(cJSON_GetObjectItem(root, "planets"), sd, pa);
    sd->home_system = jsonGetBool(root, "home_system");
    sd->worm_here = jsonGetBool(root, "worm_here");
    sd->worm_x = jsonGetInt(root, "worm_x");
    sd->worm_y = jsonGetInt(root, "worm_y");
    sd->worm_z = jsonGetInt(root, "worm_z");
    unmarshalSpeciesBitfield(cJSON_GetObjectItem(root, "visited_by"), sd->visited_by);
    sd->message = jsonGetInt(root, "message");
}

void unmarshalSystems(cJSON *root, star_data_t *sa, planet_data_t *pa) {
    if (root == 0) {
        fprintf(stderr, "error: systems: property 'systems' must not be null");
        exit(2);
    } else if (!cJSON_IsArray(root)) {
        fprintf(stderr, "error: systems: property 'systems' must be an array");
        exit(2);
    } else if (cJSON_GetArraySize(root) != num_stars) {
        fprintf(stderr, "error: systems: array should contain %d stars\n", num_stars);
        exit(2);
    }
    int star_index = 0;
    int planet_index = 0;
    cJSON *elem = 0;
    cJSON_ArrayForEach(elem, root) {
        star_data_t *sd = sa + star_index;
        sd->id = star_index + 1;
        sd->index = star_index;
        sd->planet_index = planet_index;
        unmarshalSystem(elem, sd, pa + planet_index);
        star_index = star_index + 1;
        planet_index = planet_index + sd->num_planets;
    }
    printf("unmarshal: systems: found %8d stars\n", star_index);
    printf("unmarshal: systems: found %8d planets\n", planet_index);
    for (int i = 0; i < num_stars; i++) {
        star_data_t *sd = star_base + i;
        if (sd->worm_here) {
            for (int j = 0; j < num_stars && sd->wormholeExit == 0; j++) {
                star_data_t *other = star_base + j;
                if (other->x == sd->worm_x && other->y == sd->worm_y && other->z == sd->z) {
                    sd->wormholeExit = other;
                    break;
                }
            }
        }
    }
}

void unmarshalSystemsFile(cJSON *root, star_data_t *sa, planet_data_t *pa) {
    int version = unmarshalVersion(cJSON_GetObjectItem(root, "version"));
    if (version != 1) {
        fprintf(stderr, "error: systems: version: want 1, got %d\n", version);
        exit(2);
    }
    unmarshalSystems(cJSON_GetObjectItem(root, "systems"), sa, pa);
    fflush(stdout);
}

void unmarshalTechnologies(cJSON *root, const char **codes, int *levels, int *knowledge, int *xp, int *init_levels) {
    if (root == 0) {
        fprintf(stderr, "error: technologies: property must not be null\n");
        exit(2);
    } else if (!cJSON_IsObject(root)) {
        fprintf(stderr, "error: technologies: property must be an object\n");
        exit(2);
    }
    for (int i = 0; i < 6; i++) {
        levels[i] = 0;
        knowledge[i] = 0;
        xp[i] = 0;
        init_levels[i] = 0;
    }
    for (int i = 0; i < 6; i++) {
        cJSON *elem = cJSON_GetObjectItem(root, codes[i]);
        if (elem == 0) {
            continue;
        } else if (!cJSON_IsObject(elem)) {
            fprintf(stderr, "error: technologies: '%s' must be an object\n", codes[i]);
            exit(2);
        }
        levels[i] = jsonGetInt(elem, "level");
        knowledge[i] = jsonGetInt(elem, "knowledge");
        xp[i] = jsonGetInt(elem, "xp");
        init_levels[i] = jsonGetInt(elem, "init_level");
        // printf("unTech: code '%s' level %3d knowl %3d xp %3d init %3d\n", codes[i], levels[i], knowledge[i], xp[i], init_levels[i]);
    }
}

int unmarshalVersion(cJSON *elem) {
    if (elem == 0) {
        return 0;
    } else if (!cJSON_IsNumber(elem)) {
        fprintf(stderr, "error: version: element must be an integer\n");
        exit(2);
    }
    return elem->valueint;
}
