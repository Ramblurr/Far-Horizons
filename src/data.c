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
#include <string.h>
#include "engine.h"
#include "commandvars.h"
#include "data.h"
#include "galaxyio.h"
#include "json.h"
#include "marshal.h"
#include "namplavars.h"
#include "planetio.h"
#include "planetvars.h"
#include "shipvars.h"
#include "speciesio.h"
#include "stario.h"
#include "unmarshal.h"


int exportData(FILE *fp) {
    global_data_t *g = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_data_t));
    g->turn = galaxy.turn_number;

    g->cluster = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_cluster_t));
    g->cluster->radius = galaxy.radius;
    g->cluster->d_num_species = galaxy.d_num_species;
    g->cluster->systems = ncalloc(__FUNCTION__, __LINE__, num_stars + 1, sizeof(global_system_t *));
    for (int i = 0; i < num_stars; i++) {
        g->cluster->systems[i] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_system_t));
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
        s->planets = ncalloc(__FUNCTION__, __LINE__, star->num_planets + 1, sizeof(global_planet_t *));
        for (int pn = 0; pn < star->num_planets; pn++) {
            s->planets[pn] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_planet_t));
            global_planet_t *p = s->planets[pn];
            planet_data_t *planet = planet_base + star->planet_index + pn;
            p->id = planet->id;
            p->orbit = planet->orbit;
            p->diameter = planet->diameter;
            p->econ_efficiency = planet->econ_efficiency;
            int index = 0;
            for (int g = 0; g < 4; g++) {
                if (planet->gas[g] != 0) {
                    p->gases[index] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_gas_t));
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
    g->species = ncalloc(__FUNCTION__, __LINE__, galaxy.num_species + 1, sizeof(global_species_t *));
    for (int i = 0; i < galaxy.num_species; i++) {
        g->species[i] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_species_t));
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
            s->skills[l] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_skill_t));
            strcpy(s->skills[l]->code, tech_abbr[l]);
            strcpy(s->skills[l]->name, tech_name[l]);
            s->skills[l]->init_level = species->init_tech_level[l];
            s->skills[l]->current_level = species->tech_level[l];
            s->skills[l]->knowledge_level = species->tech_knowledge[l];
            s->skills[l]->xps = species->tech_eps[l];
        }
        s->required_gases[0] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_gas_t));
        strcpy(s->required_gases[0]->code, gas_string[species->required_gas]);
        s->required_gases[0]->max_pct = species->required_gas_max;
        s->required_gases[0]->min_pct = species->required_gas_min;
        int index = 0;
        for (int g = 0; g < 6; g++) {
            if (species->neutral_gas[g] == 0) {
                break;
            }
            s->neutral_gases[index] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_gas_t));
            strcpy(s->neutral_gases[index]->code, gas_string[species->neutral_gas[g]]);
            index++;
        }
        index = 0;
        for (int g = 0; g < 6; g++) {
            if (species->poison_gas[g] == 0) {
                break;
            }
            s->poison_gases[index] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_gas_t));
            strcpy(s->poison_gases[index]->code, gas_string[species->poison_gas[g]]);
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

        s->colonies = ncalloc(__FUNCTION__, __LINE__, species->num_namplas + 1, sizeof(global_colony_t *));
        for (int n = 0; n < species->num_namplas; n++) {
            s->colonies[n] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_colony_t));
            global_colony_t *p = s->colonies[n];
            nampla_data_t *nampla = namp_data[species->index] + n;
            p->id = nampla->id;
            strcpy(p->name, nampla->name);
            p->hidden = nampla->hidden;
            p->hiding = nampla->hiding;
            p->homeworld = n == 0;
            p->ma_base = nampla->ma_base;
            p->message = nampla->message;
            p->mi_base = nampla->mi_base;
            p->pop_units = nampla->pop_units;
            p->siege_eff = nampla->siege_eff;
            p->special = nampla->special;
            p->use_on_ambush = nampla->use_on_ambush;
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
            p->inventory = ncalloc(__FUNCTION__, __LINE__, items + 1, sizeof(global_item_t *));
            items = 0; // reset and use as index to populate inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (nampla->item_quantity[k] != 0) {
                    p->inventory[items] = ncalloc(__FUNCTION__, __LINE__, items + 1, sizeof(global_item_t));
                    strcpy(p->inventory[items]->code, item_abbr[k]);
                    p->inventory[items]->quantity = nampla->item_quantity[k];
                    items++;
                }
            }
            index = 0;
            if (nampla->auto_AUs || nampla->AUs_needed || nampla->AUs_to_install) {
                p->develop[index] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_develop_t));
                strcpy(p->develop[index]->code, "AU");
                p->develop[index]->auto_install = nampla->auto_AUs;
                p->develop[index]->units_to_install = nampla->AUs_to_install;
                p->develop[index]->units_needed = nampla->AUs_needed;
                index++;
            }
            if (nampla->auto_IUs || nampla->IUs_needed || nampla->IUs_to_install) {
                p->develop[index] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_develop_t));
                strcpy(p->develop[index]->code, "IU");
                p->develop[index]->auto_install = nampla->auto_IUs;
                p->develop[index]->units_to_install = nampla->IUs_to_install;
                p->develop[index]->units_needed = nampla->IUs_needed;
                index++;
            }
        }

        s->ships = ncalloc(__FUNCTION__, __LINE__, species->num_ships + 1, sizeof(global_ship_t *));
        int counter = 0;
        for (int n = 0; n < species->num_ships; n++) {
            ship_data_t *ship = ship_data[species->index] + n;
            if (strcmp(ship->name, "Unused") == 0) {
                continue;
            }
            s->ships[counter] = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_ship_t));
            global_ship_t *p = s->ships[counter];
            counter++;
            strcpy(p->name, shipDisplayName(ship));
            p->age = ship->age;
            p->arrived_via_wormhole = ship->arrived_via_wormhole;
            int items = 0; // number of items in inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (ship->item_quantity[k] != 0) {
                    items++;
                }
            }
            p->inventory = ncalloc(__FUNCTION__, __LINE__, items + 1, sizeof(global_item_t *));
            items = 0; // reset and use as index to populate inventory
            for (int k = 0; k < MAX_ITEMS; k++) {
                if (ship->item_quantity[k] != 0) {
                    p->inventory[items] = ncalloc(__FUNCTION__, __LINE__, items + 1, sizeof(global_item_t));
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
            p->location.x = ship->x;
            p->location.y = ship->y;
            p->location.z = ship->z;
            p->location.orbit = ship->pn;
            p->location.deep_space = ship->status == IN_DEEP_SPACE ? TRUE : FALSE;
            p->location.in_orbit = ship->status == IN_ORBIT ? TRUE : FALSE;
            p->location.on_surface = ship->status == ON_SURFACE ? TRUE : FALSE;

            p->destination.x = ship->dest_x;
            p->destination.y = ship->dest_y;
            p->destination.z = ship->dest_z;

            p->remaining_cost = ship->remaining_cost;
            p->special = ship->special;
            p->status = ship->status;
            if (ship->class == BA) {
                p->tonnage = ship->tonnage;
            }

            if (ship->unloading_point == 9999) {
                strcpy(p->unloading_point, s->colonies[0]->name);
            } else if (ship->unloading_point > 0) {
                strcpy(p->unloading_point, s->colonies[ship->unloading_point]->name);
            }
        }
    }

    json_marshal(marshalGlobals(g), 0, fp);

    return 0;
}


int importData(FILE *fp) {
    json_value_t *j = json_unmarshal(fp);
    global_data_t *d = unmarshalData(j);

    galaxy.turn_number = d->turn;
    galaxy.radius = d->cluster->radius;
    galaxy.d_num_species = d->cluster->d_num_species;
    if (d->cluster != NULL) {
        for (global_species_t **species = d->species; species != NULL; species++) {
            galaxy.num_species++;
        }
    }

    return 0;
}


