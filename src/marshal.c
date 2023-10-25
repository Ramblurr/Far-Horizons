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


#include "marshal.h"
#include "shipvars.h"
#include "star.h"


json_value_t *marshalCluster(global_cluster_t *c) {
    json_value_t *j = json_map();
    json_add(j, "radius", json_number(c->radius));
    json_add(j, "d_num_species", json_number(c->d_num_species));
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
    if (c->hidden != FALSE) {
        json_add(j, "hidden", json_boolean(1));
    }
    if (c->hiding != FALSE) {
        json_add(j, "hiding", json_boolean(1));
    }
    if (c->inventory[0] != NULL) {
        json_add(j, "inventory", marshalInventory(c->inventory));
    }
    json_value_t *develop = NULL;
    for (global_develop_t **d = c->develop; *d != NULL; d++) {
        if (develop == NULL) {
            develop = json_list();
        }
        json_append(develop, marshalDevelop(*d));
    }
    if (develop != NULL) {
        json_add(j, "develop", develop);
    }
    if (c->ma_base) {
        json_add(j, "ma_base", json_number(c->ma_base));
    }
    if (c->message) {
        json_add(j, "message", json_number(c->message));
    }
    if (c->mi_base) {
        json_add(j, "mi_base", json_number(c->mi_base));
    }
    if (c->pop_units) {
        json_add(j, "pop_units", json_number(c->pop_units));
    }
    if (c->siege_eff != 0) {
        json_add(j, "siege_eff", json_number(c->siege_eff));
    }
    if (c->special != 0) {
        json_add(j, "special", json_number(c->special));
    }
    if (c->use_on_ambush != 0) {
        json_add(j, "use_on_ambush", json_boolean(1));
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


json_value_t *marshalDevelop(global_develop_t *d) {
    json_value_t *j = json_map();
    json_add(j, "code", json_string(d->code));
    if (d->auto_install) {
        json_add(j, "auto_install", json_number(d->auto_install));
    }
    if (d->units_needed) {
        json_add(j, "units_needed", json_number(d->units_needed));
    }
    if (d->units_to_install) {
        json_add(j, "units_to_install", json_number(d->units_to_install));
    }
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


json_value_t *marshalLocation(global_location_t l) {
    json_value_t *j = json_map();
    if (l.colony[0] != 0) {
        json_add(j, "colony", json_string(l.colony));
    } else {
        json_add(j, "x", json_number(l.x));
        json_add(j, "y", json_number(l.y));
        json_add(j, "z", json_number(l.z));
        if (l.orbit) {
            json_add(j, "z", json_number(l.orbit));
        }
    }
    if (l.deep_space) {
        json_add(j, "deep_space", json_boolean(1));
    }
    if (l.in_orbit) {
        json_add(j, "in_orbit", json_boolean(1));
    }
    if (l.on_surface) {
        json_add(j, "on_surface", json_boolean(1));
    }
    return j;
}


json_value_t *marshalPlanet(global_planet_t *p) {
    json_value_t *j = json_map();
    //json_add(j, "id", json_number(p->id));
    json_add(j, "orbit", json_number(p->orbit));
    json_add(j, "diameter", json_number(p->diameter));
    json_add(j, "econ_efficiency", json_number(p->econ_efficiency));
    if (p->gases[0] != NULL) {
        json_add(j, "gases", marshalGases(p->gases));
    }
    json_add(j, "gravity", json_number(p->gravity));
    if (p->idealHomePlanet) {
        json_add(j, "ideal_home_planet", json_boolean(1));
    }
    if (p->idealColonyPlanet) {
        json_add(j, "ideal_colony_planet", json_boolean(1));
    }
    if (p->md_increase != 0) {
        json_add(j, "md_increase", json_number(p->md_increase));
    }
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
    if (s->status == IN_DEEP_SPACE) {
        json_add(j, "in_deep_space", json_boolean(1));
    }
    if (s->status == IN_ORBIT) {
        json_add(j, "in_orbit", json_boolean(1));
    }
    if (s->inventory[0] != NULL) {
        json_add(j, "inventory", marshalInventory(s->inventory));
    }
    if (s->status == JUMPED_IN_COMBAT) {
        json_add(j, "jumped_in_combat", json_boolean(1));
    }
    if (s->just_jumped) {
        json_add(j, "just_jumped", json_boolean(1));
    }
    json_add(j, "location", marshalLocation(s->location));
    if (s->destination.x != 0 && s->destination.y != 0 && s->destination.z != 0) {
        json_add(j, "destination", marshalLocation(s->destination));
    }
    if (s->loading_point[0]) {
        json_add(j, "loading_point", json_string(s->loading_point));
    }
    if (s->status == ON_SURFACE) {
        json_add(j, "on_surface", json_boolean(1));
    }
    if (s->remaining_cost) {
        json_add(j, "remaining_cost", json_number(s->remaining_cost));
    }
    if (s->tonnage != 0) {
        json_add(j, "tonnage", json_number(s->tonnage));
    }
    if (s->status == UNDER_CONSTRUCTION) {
        json_add(j, "under_construction", json_boolean(1));
    }
    if (s->special != 0) {
        json_add(j, "special", json_number(s->special));
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
    json_add(j, "sp_no", json_number(s->id));
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
    global_location_t l = {.x =  s->coords.x, y: s->coords.y, z: s->coords.z};
    json_add(j, "coords", marshalLocation(l));
    switch (s->type) {
        case DEGENERATE:
            json_add(j, "type", json_string("degenerate"));
            break;
        case DWARF:
            json_add(j, "type", json_string("dwarf"));
            break;
        case GIANT:
            json_add(j, "type", json_string("giant"));
            break;
        case MAIN_SEQUENCE:
            json_add(j, "type", json_string("main_sequence"));
            break;
    }
    switch (s->color) {
        case BLUE:
            json_add(j, "color", json_string("blue"));
            break;
        case BLUE_WHITE:
            json_add(j, "color", json_string("blue_white"));
            break;
        case WHITE:
            json_add(j, "color", json_string("white"));
            break;
        case YELLOW_WHITE:
            json_add(j, "color", json_string("yellow_white"));
            break;
        case YELLOW:
            json_add(j, "color", json_string("yellow"));
            break;
        case ORANGE:
            json_add(j, "color", json_string("orange"));
            break;
        case RED:
            json_add(j, "color", json_string("red"));
            break;
    }
    json_add(j, "size", json_number(s->size));
    if (s->home_system) {
        json_add(j, "home_system", json_boolean(1));
    }
    if (s->message != 0) {
        json_add(j, "message", json_number(s->message));
    }
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


