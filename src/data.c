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
#include <stdio.h>
#include <string.h>
#include "engine.h"
#include "data.h"
#include "galaxyio.h"
#include "json.h"
#include "namplavars.h"
#include "planetio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "stario.h"
#include "planetvars.h"
#include "commandvars.h"


typedef struct global_cluster {
    int radius;
    int d_num_species;
    int num_species;
    struct global_system **systems;
} global_cluster_t;

typedef struct global_colony {
    int id;
    char name[32];
    int homeworld;
    struct global_item **inventory;
    struct {
        struct global_system *system;
        struct global_planet *planet;
    } location;
} global_colony_t;

typedef struct global_data {
    int turn;
    struct global_cluster *cluster;
    struct global_species **species;
} global_data_t;

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
    int class;
    struct global_item **inventory;
    struct {
        int x, y, z, pn;
        char colony[32];
    } location;
    int just_jumped;
    char loading_point[32];
    int remaining_cost;
    int special;
    int status;
    int tonnage;
    int type;
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
    int x, y, z;
    int color;
    int home_system;
    int message;
    int size;
    int type;
    int wormholeExit;
    struct global_planet **planets;
    int visited_by[MAX_SPECIES + 1];
} global_system_t;


static json_value_t *marshalCluster(global_cluster_t *c);

static json_value_t *marshalColony(global_colony_t *c);

static json_value_t *marshalColonies(global_colony_t **c);

static json_value_t *marshalGlobals(global_data_t *g);

static json_value_t *marshalInventory(global_item_t **i);

static json_value_t *marshalCoords(int x, int y, int z);

static json_value_t *marshalGas(global_gas_t *g);

static json_value_t *marshalGases(global_gas_t **g);

static json_value_t *marshalPlanet(global_planet_t *p);

static json_value_t *marshalPlanets(global_planet_t **p);

static json_value_t *marshalShip(global_ship_t *s);

static json_value_t *marshalShips(global_ship_t **s);

static json_value_t *marshalSkill(global_skill_t *s);

static json_value_t *marshalSkills(global_skill_t **s);

static json_value_t *marshalSpecie(global_species_t *s);

static json_value_t *marshalSpecies(global_species_t **s);

static json_value_t *marshalSystem(global_system_t *s);

static json_value_t *marshalSystems(global_system_t **s);


int exportData(FILE *fp) {
    global_data_t *g = calloc(1, sizeof(global_data_t));
    g->turn = galaxy.turn_number;

    g->cluster = calloc(1, sizeof(global_cluster_t));
    g->cluster->radius = galaxy.radius;
    g->cluster->d_num_species = galaxy.d_num_species;
    g->cluster->num_species = galaxy.num_species;
    g->cluster->systems = calloc(num_stars + 1, sizeof(global_system_t *));
    for (int i = 0; i < num_stars; i++) {
        g->cluster->systems[i] = calloc(1, sizeof(global_system_t));
        global_system_t *s = g->cluster->systems[i];
        star_data_t *star = star_base + i;
        s->id = star->id;
        s->x = star->x;
        s->y = star->y;
        s->z = star->z;
        s->color = star->color;
        s->home_system = star->home_system;
        s->size = star->size;
        s->type = star->type;
        for (int i = 0; i < galaxy.num_species; i++) {
            if ((star->visited_by[i / 32] & (1 << (i % 32))) != 0) {
                s->visited_by[i + 1] = TRUE;
            }
        }
        s->wormholeExit = star->wormholeExit ? star->wormholeExit->id : 0;
        s->planets = calloc(star->num_planets + 1, sizeof(global_planet_t *));
        for (int pn = 0; pn < star->num_planets; pn++) {
            s->planets[pn] = calloc(1, sizeof(global_planet_t));
            global_planet_t *p = s->planets[pn];
            planet_data_t *planet = planet_base + star->planet_index + pn;
            p->id = planet->id;
            p->orbit = planet->orbit;
            p->diameter = planet->diameter;
            p->econ_efficiency = planet->econ_efficiency;
            int index = 0;
            for (int g = 0; g < 4; g++) {
                if (planet->gas[g] != 0) {
                    p->gases[index] = calloc(1, sizeof(global_gas_t));
                    strcpy(p->gases[index]->code, gas_string[planet->gas[g]]);
                    p->gases[index]->atmos_pct = planet->gas_percent[g];
                    index++;
                }
            }
            p->gravity = planet->gravity;
            p->idealHomePlanet = planet->special == 1;
            p->idealColonyPlanet = planet->special == 2;
            p->md_increase = planet->md_increase;
            p->message = planet->message;
            p->mining_difficulty = planet->mining_difficulty;
            p->pressure_class = planet->pressure_class;
            p->radioactiveHellHole = planet->special == 3;
            p->temperature_class = planet->temperature_class;
        }
    }
    g->species = calloc(galaxy.num_species + 1, sizeof(global_species_t *));
    for (int i = 0; i < galaxy.num_species; i++) {
        g->species[i] = calloc(1, sizeof(global_species_t));
        global_species_t *s = g->species[i];
        species_data_t *species = spec_data + i;
        s->id = species->id;
        strcpy(s->name, species->name);
        strcpy(s->govt_name, species->govt_name);
        strcpy(s->govt_type, species->govt_type);
        s->auto_orders = species->auto_orders;
        s->econ_units = species->econ_units;
        s->hp_original_base = species->hp_original_base;

        for (int l = 0; l < 6; l++) {
            s->skills[l] = calloc(1, sizeof(global_skill_t));
            strcpy(s->skills[l]->code, tech_abbr[l]);
            strcpy(s->skills[l]->name, tech_name[l]);
            s->skills[l]->init_level = species->init_tech_level[l];
            s->skills[l]->current_level = species->tech_level[l];
            s->skills[l]->knowledge_level = species->tech_knowledge[l];
            s->skills[l]->xps = species->tech_eps[l];
        }
        s->required_gases[0] = calloc(1, sizeof(global_gas_t));
        sprintf(s->required_gases[0]->code, gas_string[species->required_gas]);
        s->required_gases[0]->max_pct = species->required_gas_max;
        s->required_gases[0]->min_pct = species->required_gas_min;
        int index = 0;
        for (int g = 0; g < 6; g++) {
            if (species->neutral_gas[g] == 0) {
                break;
            }
            s->neutral_gases[index] = calloc(1, sizeof(global_gas_t));
            sprintf(s->neutral_gases[index]->code, gas_string[species->neutral_gas[g]]);
            index++;
        }
        index = 0;
        for (int g = 0; g < 6; g++) {
            if (species->poison_gas[g] == 0) {
                break;
            }
            s->poison_gases[index] = calloc(1, sizeof(global_gas_t));
            sprintf(s->poison_gases[index]->code, gas_string[species->poison_gas[g]]);
            index++;
        }

        for (int b = 0; b < galaxy.num_species; b++) {
            if (b != i) {
                if ((species->ally[b / 32] & (1 << (b % 32))) != 0) {
                    s->allies[b + 1] = TRUE;
                }
                if ((species->contact[b / 32] & (1 << (b % 32))) != 0) {
                    s->contacts[b + 1] = TRUE;
                }
                if ((species->enemy[b / 32] & (1 << (b % 32))) != 0) {
                    s->enemies[b + 1] = TRUE;
                }
            }
        }

        s->colonies = calloc(species->num_namplas + 1, sizeof(global_colony_t *));
        for (int n = 0; n < species->num_namplas; n++) {
            s->colonies[n] = calloc(1, sizeof(global_colony_t));
            global_colony_t *p = s->colonies[n];
            nampla_data_t *nampla = namp_data[species->index] + n;
            p->id = nampla->id;
            strcpy(p->name, nampla->name);
            p->homeworld = n == 0;
            for (global_system_t **system = g->cluster->systems; *system; system++) {
                if (nampla->system->x == (*system)->x && nampla->system->y == (*system)->y &&
                    nampla->system->z == (*system)->z) {
                    p->location.system = *system;
                    for (global_planet_t **planet = (*system)->planets; *planet; planet++) {
                        if (nampla->planet->orbit == (*planet)->orbit) {
                            p->location.planet = *planet;
                            break;
                        }
                    }
                    break;
                }
            }
            int items = 0; // number of items in inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (nampla->item_quantity[k] != 0) {
                    items++;
                }
            }
            p->inventory = calloc(items + 1, sizeof(global_item_t *));
            items = 0; // reset and use as index to populate inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (nampla->item_quantity[k] != 0) {
                    p->inventory[items] = calloc(items + 1, sizeof(global_item_t));
                    strcpy(p->inventory[items]->code, item_abbr[k]);
                    p->inventory[items]->quantity = nampla->item_quantity[k];
                    items++;
                }
            }
        }

        s->ships = calloc(species->num_ships + 1, sizeof(global_ship_t *));
        int counter = 0;
        for (int n = 0; n < species->num_ships; n++) {
            ship_data_t *ship = ship_data[species->index] + n;
            if (strcmp(ship->name, "Unused") == 0) {
                continue;
            }
            s->ships[counter] = calloc(1, sizeof(global_ship_t));
            global_ship_t *p = s->ships[counter];
            counter++;
            strcpy(p->name, shipDisplayName(ship));
            p->age = ship->age;
            p->arrived_via_wormhole = ship->arrived_via_wormhole;
            p->class = ship->class;
            int items = 0; // number of items in inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (ship->item_quantity[k] != 0) {
                    items++;
                }
            }
            p->inventory = calloc(items + 1, sizeof(global_item_t *));
            items = 0; // reset and use as index to populate inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (ship->item_quantity[k] != 0) {
                    p->inventory[items] = calloc(items + 1, sizeof(global_item_t));
                    strcpy(p->inventory[items]->code, item_abbr[k]);
                    p->inventory[items]->quantity = ship->item_quantity[k];
                    items++;
                }
            }
            p->just_jumped = ship->just_jumped;
            if (ship->loading_point == 9999) {
                strcpy(p->loading_point, s->colonies[0]->name);
            } else if (ship->loading_point > 0) {
                strcpy(p->loading_point, s->colonies[ship->loading_point]->name);
            }
            // location can be either the name of a colony or x,y,z coordinates
            for (global_colony_t **c = s->colonies; *c; c++) {
                if ((*c)->location.system->x == ship->x && (*c)->location.system->y == ship->y &&
                    (*c)->location.system->z == ship->z) {
                    strcpy(p->location.colony, (*c)->name);
                    break;
                }
            }
            if (p->location.colony[0] == 0) {
                p->location.x = ship->x;
                p->location.y = ship->y;
                p->location.z = ship->z;
                p->location.pn = ship->pn;
            }
            p->remaining_cost = ship->remaining_cost;
            p->special = ship->special;
            p->status = ship->status;
            p->tonnage = ship->tonnage;
            p->type = ship->type;
            if (ship->unloading_point == 9999) {
                strcpy(p->unloading_point, s->colonies[0]->name);
            } else if (ship->unloading_point > 0) {
                strcpy(p->unloading_point, s->colonies[ship->unloading_point]->name);
            }
        }
    }

    json_value_t *j = marshalGlobals(g);
    json_write(j, fp);

    return 0;
}


json_value_t *marshalCluster(global_cluster_t *c) {
    json_value_t *j = json_map();
    json_add(j, "radius", json_number(c->radius));
    json_add(j, "d_num_species", json_number(c->d_num_species));
    json_add(j, "num_species", json_number(c->num_species));
    json_add(j, "systems", marshalSystems(c->systems));
    return j;
}


json_value_t *marshalColony(global_colony_t *c) {
    json_value_t *j = json_map();
    // json_add(j, "cid", json_number(c->id));
    json_add(j, "name", json_string(c->name));
    if (c->location.system) {
        json_add(j, "system", json_number(c->location.system->id));
        if (c->location.planet) {
            json_add(j, "orbit", json_number(c->location.planet->orbit));
        }
    }
    if (c->homeworld != FALSE) {
        json_add(j, "homeworld", json_boolean(1));
    }
    if (c->inventory[0] != NULL) {
        json_add(j, "inventory", marshalInventory(c->inventory));
    }
    return j;
}


json_value_t *marshalColonies(global_colony_t **c) {
    json_value_t *j = json_list();
    if (c != NULL) {
        for (; *c; c++) {
            json_append(j, marshalColony(*c));
        }
    }
    return j;
}


json_value_t *marshalCoords(int x, int y, int z) {
    json_value_t *j = json_map();
    json_add(j, "x", json_number(x));
    json_add(j, "y", json_number(y));
    json_add(j, "z", json_number(z));
    return j;
}


json_value_t *marshalGas(global_gas_t *g) {
    json_value_t *j = json_map();
    json_add(j, "code", json_string(g->code));
    if (g->atmos_pct) {
        json_add(j, "atmos_pct", json_number(g->atmos_pct));
    }
    if (g->min_pct || g->max_pct) {
        json_add(j, "min_pct", json_number(g->min_pct));
        json_add(j, "max_pct", json_number(g->max_pct));
    }
    if (g->required) {
        json_add(j, "required", json_boolean(1));
    }
    return j;
}


json_value_t *marshalGases(global_gas_t **g) {
    json_value_t *j = json_list();
    if (g != NULL) {
        for (; *g; g++) {
            json_append(j, marshalGas(*g));
        }
    }
    return j;
}


json_value_t *marshalGlobals(global_data_t *g) {
    json_value_t *j = json_map();
    json_add(j, "turn", json_number(g->turn));
    json_add(j, "cluster", marshalCluster(g->cluster));
    json_add(j, "species", marshalSpecies(g->species));
    return j;
}


json_value_t *marshalInventory(global_item_t **i) {
    json_value_t *j = json_map();
    if (i != NULL) {
        for (; *i; i++) {
            json_add(j, (*i)->code, json_number((*i)->quantity));
        }
    }
    return j;
}


json_value_t *marshalPlanet(global_planet_t *p) {
    json_value_t *j = json_map();
    json_add(j, "id", json_number(p->id));
    json_add(j, "orbit", json_number(p->orbit));
    json_add(j, "diameter", json_number(p->diameter));
    json_add(j, "econ_efficiency", json_number(p->econ_efficiency));
    json_add(j, "gases", marshalGases(p->gases));
    json_add(j, "gravity", json_number(p->gravity));
    if (p->idealHomePlanet) {
        json_add(j, "ideal_home_planet", json_boolean(1));
    }
    if (p->idealColonyPlanet) {
        json_add(j, "ideal_colony_planet", json_boolean(1));
    }
    json_add(j, "md_increase", json_number(p->md_increase));
    if (p->message != 0) {
        json_add(j, "message", json_number(p->message));
    }
    if (p->mining_difficulty != 0) {
        json_add(j, "mining_difficulty", json_number(p->mining_difficulty));
    }
    json_add(j, "pressure_class", json_number(p->pressure_class));
    if (p->radioactiveHellHole) {
        json_add(j, "radioactive_hell_hole", json_boolean(1));
    }
    json_add(j, "temperature_class", json_number(p->temperature_class));
    return j;
}


json_value_t *marshalPlanets(global_planet_t **p) {
    json_value_t *j = json_list();
    if (p != NULL) {
        for (; *p; p++) {
            json_append(j, marshalPlanet(*p));
        }
    }
    return j;
}


json_value_t *marshalShip(global_ship_t *s) {
    json_value_t *j = json_map();
    json_add(j, "name", json_string(s->name));
    if (s->age) {
        json_add(j, "age", json_number(s->age));
    }
    if (s->arrived_via_wormhole) {
        json_add(j, "arrived_via_wormhole", json_boolean(1));
    }
    if (s->status == FORCED_JUMP) {
        json_add(j, "forced_jump", json_boolean(1));
    }
    if (s->inventory[0] != NULL) {
        json_add(j, "inventory", marshalInventory(s->inventory));
    }
    json_value_t *location = json_map();
    if (s->location.colony[0]) {
        json_add(location, "colony", json_string(s->location.colony));
    } else {
        json_add(location, "x", json_number(s->location.x));
        json_add(location, "y", json_number(s->location.y));
        json_add(location, "z", json_number(s->location.z));
        if (s->location.pn) {
            json_add(location, "pn", json_number(s->location.pn));
        }
    }
    if (s->status == IN_DEEP_SPACE) {
        json_add(location, "deep_space", json_boolean(1));
    }
    if (s->status == IN_ORBIT) {
        json_add(location, "in_orbit", json_boolean(1));
    }
    if (s->status == ON_SURFACE) {
        json_add(location, "on_surface", json_boolean(1));
    }
    json_add(j, "location", location);
    if (s->status == JUMPED_IN_COMBAT) {
        json_add(j, "jumped_in_combat", json_boolean(1));
    }
    if (s->just_jumped) {
        json_add(j, "just_jumped", json_boolean(1));
    }
    if (s->loading_point[0]) {
        json_add(j, "loading_point", json_string(s->loading_point));
    }
    if (s->remaining_cost) {
        json_add(j, "remaining_cost", json_number(s->remaining_cost));
    }
    if (s->class == BA) {
        json_add(j, "tonnage", json_number(s->tonnage));
    }
    if (s->status == UNDER_CONSTRUCTION) {
        json_add(j, "under_construction", json_boolean(1));
    }
    if (s->unloading_point[0]) {
        json_add(j, "unloading_point", json_string(s->unloading_point));
    }
    return j;
}


json_value_t *marshalShips(global_ship_t **s) {
    json_value_t *j = json_list();
    if (s != NULL) {
        for (; *s; s++) {
            json_append(j, marshalShip(*s));
        }
    }
    return j;
}


json_value_t *marshalSkill(global_skill_t *s) {
    json_value_t *j = json_map();
    json_add(j, "code", json_string(s->code));
    if (s->init_level != 0) {
        json_add(j, "init_level", json_number(s->init_level));
    }
    json_add(j, "current_level", json_number(s->current_level));
    if (s->knowledge_level != 0) {
        json_add(j, "knowledge_level", json_number(s->knowledge_level));
    }
    json_add(j, "xps", json_number(s->xps));

    return j;
}


json_value_t *marshalSkills(global_skill_t **s) {
    json_value_t *j = json_list();
    if (s != NULL) {
        for (; *s; s++) {
            json_append(j, marshalSkill(*s));
        }
    }
    return j;
}


json_value_t *marshalSpecie(global_species_t *s) {
    json_value_t *j = json_map();
    json_add(j, "sp", json_number(s->id));
    json_add(j, "name", json_string(s->name));
    json_add(j, "govt_name", json_string(s->govt_name));
    json_add(j, "govt_type", json_string(s->govt_type));

    if (s->auto_orders) {
        json_add(j, "auto_orders", json_boolean(1));
    }
    json_add(j, "econ_units", json_number(s->econ_units));
    if (s->hp_original_base) {
        json_add(j, "hp_original_base", json_number(s->hp_original_base));
    }

    json_add(j, "skills", marshalSkills(s->skills));

    json_add(j, "required_gases", marshalGases(s->required_gases));
    json_add(j, "neutral_gases", marshalGases(s->neutral_gases));
    json_add(j, "poison_gases", marshalGases(s->poison_gases));

    json_value_t *v = NULL;
    for (int sp = 0; sp < MAX_SPECIES + 1; sp++) {
        if (s->contacts[sp]) {
            if (v == NULL) {
                v = json_list();
            }
            json_append(v, json_number(sp));
        }
    }
    if (v != NULL) {
        json_add(j, "contacts", v);
    }
    v = NULL;
    for (int sp = 0; sp < MAX_SPECIES + 1; sp++) {
        if (s->allies[sp]) {
            if (v == NULL) {
                v = json_list();
            }
            json_append(v, json_number(sp));
        }
    }
    if (v != NULL) {
        json_add(j, "allies", v);
    }
    v = NULL;
    for (int sp = 0; sp < MAX_SPECIES + 1; sp++) {
        if (s->enemies[sp]) {
            if (v == NULL) {
                v = json_list();
            }
            json_append(v, json_number(sp));
        }
    }
    if (v != NULL) {
        json_add(j, "enemies", v);
    }

    json_add(j, "colonies", marshalColonies(s->colonies));
    json_add(j, "ships", marshalShips(s->ships));
    return j;
}


json_value_t *marshalSpecies(global_species_t **s) {
    json_value_t *j = json_list();
    if (s != NULL) {
        for (; *s; s++) {
            json_append(j, marshalSpecie(*s));
        }
    }
    return j;
}


json_value_t *marshalSystem(global_system_t *s) {
    json_value_t *j = json_map();
    json_add(j, "id", json_number(s->id));
    json_add(j, "coords", marshalCoords(s->x, s->y, s->z));
    json_add(j, "type", json_number(s->type));
    json_add(j, "color", json_number(s->color));
    json_add(j, "size", json_number(s->size));
    if (s->home_system) {
        json_add(j, "home_system", json_boolean(1));
    }
    json_add(j, "message", json_number(s->id));
    if (s->wormholeExit != 0) {
        json_add(j, "wormhole_exit", json_number(s->wormholeExit));
    }
    json_value_t *v = NULL;
    for (int sp = 0; sp < MAX_SPECIES + 1; sp++) {
        if (s->visited_by[sp]) {
            if (v == NULL) {
                v = json_list();
            }
            json_append(v, json_number(sp));
        }
    }
    if (v != NULL) {
        json_add(j, "visited_by", v);
    }
    json_add(j, "planets", marshalPlanets(s->planets));
    return j;
}


json_value_t *marshalSystems(global_system_t **s) {
    json_value_t *j = json_list();
    if (s != NULL) {
        for (; *s; s++) {
            json_append(j, marshalSystem(*s));
        }
    }
    return j;
}
