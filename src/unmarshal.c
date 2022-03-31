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
#include "engine.h"
#include "star.h"
#include "unmarshal.h"
#include "species.h"


global_cluster_t *unmarshalCluster(json_value_t *j) {
    global_cluster_t *cluster = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_data_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "d_num_species") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: cluster.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                }
                cluster->d_num_species = t->value->u.n;
            } else if (strcmp(t->key, "radius") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: cluster.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                }
                cluster->radius = t->value->u.n;
            } else if (strcmp(t->key, "systems") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: cluster.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                cluster->systems = unmarshalSystems(t->value);
            } else {
                fprintf(stderr, "%s: unknown key 'cluster.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return cluster;
}


global_colony_t *unmarshalColony(json_value_t *j) {
    global_colony_t *colony = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_colony_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "develop") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "developList");
                    exit(2);
                }
                global_develop_t **developList = unmarshalDevelopList(t->value);
                for (int n = 0; n < 2 && developList[n]; n++) {
                    colony->develop[n] = developList[n];
                }
                free(developList);
            } else if (strcmp(t->key, "homeworld") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                colony->homeworld = t->value->u.b;
            } else if (strcmp(t->key, "inventory") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                colony->inventory = unmarshalInventory(t->value);
            } else if (strcmp(t->key, "ma_base") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: colony.%s must be number greater than zero\n", __FUNCTION__, t->key);
                    exit(2);
                }
                colony->ma_base = t->value->u.n;
            } else if (strcmp(t->key, "mi_base") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: colony.%s must be number greater than zero\n", __FUNCTION__, t->key);
                    exit(2);
                }
                colony->ma_base = t->value->u.n;
            } else if (strcmp(t->key, "name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(colony->name, t->value->u.s, 32);
            } else if (strcmp(t->key, "orbit") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > 9) {
                    fprintf(stderr, "%s: colony.%s must be number between 1 and 9\n", __FUNCTION__, t->key);
                    exit(2);
                }
                colony->location.orbit = t->value->u.n;
            } else if (strcmp(t->key, "pop_units") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: colony.%s must be number greater than zero\n", __FUNCTION__, t->key);
                    exit(2);
                }
                colony->pop_units = t->value->u.n;
            } else if (strcmp(t->key, "siege_eff") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: colony.%s must be number greater than or equal to zero\n", __FUNCTION__,
                            t->key);
                    exit(2);
                }
                colony->siege_eff = t->value->u.n;
            } else if (strcmp(t->key, "system") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > MAX_STARS) {
                    fprintf(stderr, "%s: colony.%s must be number between 1 and %d\n", __FUNCTION__, t->key, MAX_STARS);
                    exit(2);
                }
                colony->location.systemId = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'colony.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return colony;
}


global_colony_t **unmarshalColonies(json_value_t *j) {
    global_colony_t **colonies = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_colony_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: colonies.%s must be %s\n", __FUNCTION__, "colony", "map");
                exit(2);
            }
            colonies[index] = unmarshalColony(t->value);
            index++;
        }
    }
    return colonies;
}


global_coords_t *unmarshalCoords(json_value_t *j) {
    global_coords_t *coords = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_coords_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "x") == 0 && json_is_number(t->value)) {
                coords->x = t->value->u.n;
            } else if (strcmp(t->key, "y") == 0 && json_is_number(t->value)) {
                coords->y = t->value->u.n;
            } else if (strcmp(t->key, "z") == 0 && json_is_number(t->value)) {
                coords->z = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'coords.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return coords;
}


global_data_t *unmarshalData(json_value_t *j) {
    global_data_t *d = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_data_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "cluster") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: data.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                d->cluster = unmarshalCluster(t->value);
            } else if (strcmp(t->key, "species") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: data.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                d->species = unmarshalSpecies(t->value);
            } else if (strcmp(t->key, "turn") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: data.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                }
                d->turn = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'data.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return d;
}


global_develop_t *unmarshalDevelop(json_value_t *j) {
    global_develop_t *develop = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_develop_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "auto_install") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: develop.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                develop->auto_install = t->value->u.n;
            } else if (strcmp(t->key, "code") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: develop.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (!(strcmp(t->value->u.s, "AU") == 0 || strcmp(t->value->u.s, "IU") == 0)) {
                    fprintf(stderr, "%s: develop.%s must be AU or IU, not '%s'\n", __FUNCTION__, t->key, t->value->u.s);
                    exit(2);
                }
                strncpy(develop->code, t->value->u.s, 3);
            } else if (strcmp(t->key, "units_needed") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: develop.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                develop->units_needed = t->value->u.n;
            } else if (strcmp(t->key, "units_to_install") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: develop.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                develop->units_to_install = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'develop.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return develop;
}


global_develop_t **unmarshalDevelopList(json_value_t *j) {
    global_develop_t **developList = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_develop_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: develop.%s must be %s\n", __FUNCTION__, "entry", "map");
                exit(2);
            }
            developList[index] = unmarshalDevelop(t->value);
            index++;
        }
    }
    return developList;
}


global_gas_t *unmarshalGas(json_value_t *j) {
    global_gas_t *gas = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_gas_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "atmos_pct") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: gas.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 100) {
                    fprintf(stderr, "%s: gas.%s must be number between 0 and 100\n", __FUNCTION__, t->key);
                    exit(2);
                }
                gas->atmos_pct = t->value->u.n;
            } else if (strcmp(t->key, "code") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: gas.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->value->u.s) < 1 || strlen(t->value->u.s) > 3) {
                    fprintf(stderr, "%s: gas.%s must be string with 1 to 3 characters\n", __FUNCTION__, t->key);
                    exit(2);
                }
                strncpy(gas->code, t->value->u.s, 4);
            } else if (strcmp(t->key, "max_pct") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: gas.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 100) {
                    fprintf(stderr, "%s: gas.%s must be number between 0 and 100\n", __FUNCTION__, t->key);
                    exit(2);
                }
                gas->max_pct = t->value->u.n;
            } else if (strcmp(t->key, "min_pct") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: gas.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 100) {
                    fprintf(stderr, "%s: gas.%s must be number between 0 and 100\n", __FUNCTION__, t->key);
                    exit(2);
                }
                gas->min_pct = t->value->u.n;
            } else if (strcmp(t->key, "required") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: gas.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 100) {
                    fprintf(stderr, "%s: gas.%s must be number between 0 and 100\n", __FUNCTION__, t->key);
                    exit(2);
                }
                gas->required = t->value->u.b;
            } else {
                fprintf(stderr, "%s: unknown key 'data.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return gas;
}


global_gas_t **unmarshalGases(json_value_t *j) {
    global_gas_t **gases = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_gas_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (json_is_map(t->value)) {
                gases[index] = unmarshalGas(t->value);
                index++;
            }
        }
    }
    return gases;
}


global_item_t **unmarshalInventory(json_value_t *j) {
    global_item_t **inventory = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_item_t *));
    if (json_is_map(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (json_is_number(t->value)) {
                global_item_t *item = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_item_t));
                strncpy(item->code, t->key, 4);
                item->quantity = t->value->u.n;
                inventory[index] = item;
                index++;
            }
        }
    }
    return inventory;
}


global_location_t *unmarshalLocation(json_value_t *j) {
    global_location_t *location = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_location_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "colony") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->value->u.s) < 1 || strlen(t->value->u.s) > 31) {
                    fprintf(stderr, "%s: location.%s must be a string of 1 to 31 characters, not '%s'\n", __FUNCTION__,
                            t->key, t->value->u.s);
                    exit(2);
                }
                strncpy(location->colony, t->value->u.s, 32);
            } else if (strcmp(t->key, "deep_space") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                location->deep_space = t->value->u.b;
            } else if (strcmp(t->key, "in_orbit") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                location->in_orbit = t->value->u.b;
            } else if (strcmp(t->key, "on_surface") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                location->on_surface = t->value->u.b;
            } else if (strcmp(t->key, "orbit") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > 9) {
                    fprintf(stderr, "%s: location.%s must be %s between 1 and 9\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                location->orbit = t->value->u.n;
            } else if (strcmp(t->key, "x") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > MAX_STARS) {
                    fprintf(stderr, "%s: location.%s must be %s between 1 and %d\n", __FUNCTION__, t->key, "number",
                            MAX_STARS);
                    exit(2);
                }
                location->x = t->value->u.n;
            } else if (strcmp(t->key, "y") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > MAX_STARS) {
                    fprintf(stderr, "%s: location.%s must be %s between 1 and %d\n", __FUNCTION__, t->key, "number",
                            MAX_STARS);
                    exit(2);
                }
                location->y = t->value->u.n;
            } else if (strcmp(t->key, "z") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: location.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > MAX_STARS) {
                    fprintf(stderr, "%s: location.%s must be %s between 1 and %d\n", __FUNCTION__, t->key, "number",
                            MAX_STARS);
                    exit(2);
                }
                location->z = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'location.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    if (location->deep_space) {
        location->in_orbit = FALSE;
        location->on_surface = FALSE;
    } else if (location->in_orbit) {
        location->on_surface = FALSE;
    }
    return location;
}


global_planet_t *unmarshalPlanet(json_value_t *j) {
    global_planet_t *planet = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_planet_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "diameter") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                planet->diameter = t->value->u.n;
            } else if (strcmp(t->key, "econ_efficiency") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 100) {
                    fprintf(stderr, "%s: planet.%s must be %s between 0 and 100\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                planet->econ_efficiency = t->value->u.n;
            } else if (strcmp(t->key, "gases") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                global_gas_t **gases = unmarshalGases(t->value);
                for (int index = 0; gases[index] != NULL; index++) {
                    planet->gases[index] = gases[index];
                }
                free(gases);
            } else if (strcmp(t->key, "gravity") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                planet->gravity = t->value->u.n;
            } else if (strcmp(t->key, "ideal_home_planet") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                planet->idealColonyPlanet = t->value->u.b;
            } else if (strcmp(t->key, "md_increase") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than or equal to zero\n", __FUNCTION__, t->key,
                            "number");
                    exit(2);
                }
                planet->md_increase = t->value->u.n;
            } else if (strcmp(t->key, "mining_difficulty") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                planet->mining_difficulty = t->value->u.n;
            } else if (strcmp(t->key, "orbit") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > 9) {
                    fprintf(stderr, "%s: planet.%s must be %s between 1 and 9\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                planet->orbit = t->value->u.n;
            } else if (strcmp(t->key, "pressure_class") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than or equal to zero\n", __FUNCTION__, t->key,
                            "number");
                    exit(2);
                }
                planet->pressure_class = t->value->u.n;
            } else if (strcmp(t->key, "temperature_class") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: planet.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: planet.%s must be %s greater than or equal to zero\n", __FUNCTION__, t->key,
                            "number");
                    exit(2);
                }
                planet->temperature_class = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'planet.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return planet;
}


global_planet_t **unmarshalPlanets(json_value_t *j) {
    global_planet_t **planets = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_planet_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: planets.%s must be %s\n", __FUNCTION__, "planet", "map");
                exit(2);
            }
            planets[index] = unmarshalPlanet(t->value);
            index++;
        }
    }
    return planets;
}


global_ship_t *unmarshalShip(json_value_t *j) {
    global_ship_t *ship = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_ship_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "age") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: ship.%s must be %s greater than or equal to zero\n", __FUNCTION__, t->key,
                            "number");
                    exit(2);
                }
                ship->age = t->value->u.n;
            } else if (strcmp(t->key, "inventory") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                ship->inventory = unmarshalInventory(t->value);
            } else if (strcmp(t->key, "loading_point") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->key) < 5 || strlen(t->key) > 31) {
                    fprintf(stderr, "%s: ship.%s must be %s of 5 to 31 characters\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(ship->loading_point, t->value->u.s, 32);
            } else if (strcmp(t->key, "location") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                global_location_t *location = unmarshalLocation(t->value);
                strncpy(ship->location.colony, location->colony, 32);
                ship->location.deep_space = location->deep_space;
                ship->location.in_orbit = location->in_orbit;
                ship->location.on_surface = location->on_surface;
                ship->location.orbit = location->orbit;
                ship->location.planetId = location->planetId;
                ship->location.systemId = location->systemId;
                ship->location.x = location->x;
                ship->location.y = location->y;
                ship->location.z = location->z;
                free(location);
            } else if (strcmp(t->key, "name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(ship->name, t->value->u.s, 32);
            } else if (strcmp(t->key, "remaining_cost") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: ship.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                ship->remaining_cost = t->value->u.n;
            } else if (strcmp(t->key, "tonnage") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1) {
                    fprintf(stderr, "%s: ship.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                ship->tonnage = t->value->u.n;
            } else if (strcmp(t->key, "unloading_point") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->key) < 5 || strlen(t->key) > 31) {
                    fprintf(stderr, "%s: ship.%s must be %s of 5 to 31 characters\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(ship->unloading_point, t->value->u.s, 32);
            } else {
                fprintf(stderr, "%s: unknown key 'ship.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return ship;
}


global_ship_t **unmarshalShips(json_value_t *j) {
    global_ship_t **ships = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_ship_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: ships.%s must be %s\n", __FUNCTION__, "ship", "map");
                exit(2);
            }
            ships[index] = unmarshalShip(t->value);
            index++;
        }
    }
    return ships;
}


global_skill_t *unmarshalSkill(json_value_t *j) {
    global_skill_t *skill = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_species_t *));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp("code", t->key) == 0) {
                strncpy(skill->code, t->value->u.s, 4);
            } else if (strcmp("current_level", t->key) == 0) {
                if (t->value->u.n > 0) {
                    skill->current_level = t->value->u.n;
                }
            } else if (strcmp("init_level", t->key) == 0) {
                if (t->value->u.n > 0) {
                    skill->init_level = t->value->u.n;
                }
            } else if (strcmp("knowledge_level", t->key) == 0) {
                if (t->value->u.n > 0) {
                    skill->knowledge_level = t->value->u.n;
                }
            } else if (strcmp("xps", t->key) == 0) {
                if (t->value->u.n > 0) {
                    skill->xps = t->value->u.n;
                }
            } else {
                fprintf(stderr, "%s: unknown key 'skill.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return skill;
}


global_species_t *unmarshalSpecie(json_value_t *j) {
    global_species_t *species = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_species_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "allies") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                for (json_node_t *spNo = t->value->u.a.root; spNo != NULL; spNo = spNo->next) {
                    if (!json_is_number(spNo->value)) {
                        fprintf(stderr, "%s: species.%s values must be %s\n", __FUNCTION__, t->key, "number");
                        exit(2);
                    } else if (spNo->value->u.n < 1 || spNo->value->u.n > MAX_SPECIES) {
                        fprintf(stderr, "%s: species.%s values must be between 1 and %d\n", __FUNCTION__, t->key,
                                MAX_SPECIES);
                        exit(2);
                    }
                    species->allies[(spNo->value->u.n - 1) / 32] ^= (1 << ((spNo->value->u.n - 1) % 32));
                }
            } else if (strcmp(t->key, "auto_orders") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                species->auto_orders = t->value->u.b;
            } else if (strcmp(t->key, "colonies") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                species->colonies = unmarshalColonies(t->value);
            } else if (strcmp(t->key, "contacts") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                for (json_node_t *spNo = t->value->u.a.root; spNo != NULL; spNo = spNo->next) {
                    if (!json_is_number(spNo->value)) {
                        fprintf(stderr, "%s: species.%s values must be %s\n", __FUNCTION__, t->key, "number");
                        exit(2);
                    } else if (spNo->value->u.n < 1 || spNo->value->u.n > MAX_SPECIES) {
                        fprintf(stderr, "%s: species.%s values must be between 1 and %d\n", __FUNCTION__, t->key,
                                MAX_SPECIES);
                        exit(2);
                    }
                    species->contacts[(spNo->value->u.n - 1) / 32] ^= (1 << ((spNo->value->u.n - 1) % 32));
                }
            } else if (strcmp(t->key, "econ_units") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0) {
                    fprintf(stderr, "%s: species.%s must be %s greater than zero\n", __FUNCTION__, t->key, "number");
                    exit(2);
                }
                species->econ_units = t->value->u.n;
            } else if (strcmp(t->key, "enemies") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                for (json_node_t *spNo = t->value->u.a.root; spNo != NULL; spNo = spNo->next) {
                    if (!json_is_number(spNo->value)) {
                        fprintf(stderr, "%s: species.%s values must be %s\n", __FUNCTION__, t->key, "number");
                        exit(2);
                    } else if (spNo->value->u.n < 1 || spNo->value->u.n > MAX_SPECIES) {
                        fprintf(stderr, "%s: species.%s values must be between 1 and %d\n", __FUNCTION__, t->key,
                                MAX_SPECIES);
                        exit(2);
                    }
                    species->enemies[(spNo->value->u.n - 1) / 32] ^= (1 << ((spNo->value->u.n - 1) % 32));
                }
            } else if (strcmp(t->key, "govt_name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->value->u.s) < 5 || strlen(t->value->u.s) > 32) {
                    fprintf(stderr, "%s: species.%s must be a string between 5 and 31 characters\n", __FUNCTION__,
                            t->key);
                    exit(2);
                }
                strncpy(species->govt_name, t->value->u.s, 32);
            } else if (strcmp(t->key, "govt_type") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->value->u.s) < 5 || strlen(t->value->u.s) > 32) {
                    fprintf(stderr, "%s: species.%s must be a string between 5 and 31 characters\n", __FUNCTION__,
                            t->key);
                    exit(2);
                }
                strncpy(species->govt_type, t->value->u.s, 32);
            } else if (strcmp(t->key, "name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strlen(t->value->u.s) < 5 || strlen(t->value->u.s) > 32) {
                    fprintf(stderr, "%s: species.%s must be a string between 5 and 31 characters\n", __FUNCTION__,
                            t->key);
                    exit(2);
                }
                strncpy(species->name, t->value->u.s, 32);
            } else if (strcmp(t->key, "neutral_gases") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                global_gas_t **gases = unmarshalGases(t->value);
                for (int index = 0; gases[index] != NULL; index++) {
                    species->neutral_gases[index] = gases[index];
                }
                free(gases);
            } else if (strcmp(t->key, "poison_gases") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                global_gas_t **gases = unmarshalGases(t->value);
                for (int index = 0; gases[index] != NULL; index++) {
                    species->poison_gases[index] = gases[index];
                }
                free(gases);
            } else if (strcmp(t->key, "required_gases") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                global_gas_t **gases = unmarshalGases(t->value);
                for (int index = 0; gases[index] != NULL; index++) {
                    species->required_gases[index] = gases[index];
                }
                free(gases);
            } else if (strcmp(t->key, "ships") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                species->ships = unmarshalShips(t->value);
            } else if (strcmp(t->key, "skills") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                for (json_node_t *sk = t->value->u.a.root; sk != NULL; sk = sk->next) {
                    if (!json_is_map(sk->value)) {
                        fprintf(stderr, "%s: species.%s must be a list of map\n", __FUNCTION__, t->key);
                        exit(2);
                    }
                    global_skill_t *skill = unmarshalSkill(sk->value);
                    if (skill != NULL) {
                        if (strcmp(skill->code, "MI") == 0) {
                            species->skills[MI] = skill;
                        } else if (strcmp(skill->code, "MA") == 0) {
                            species->skills[MA] = skill;
                        } else if (strcmp(skill->code, "ML") == 0) {
                            species->skills[ML] = skill;
                        } else if (strcmp(skill->code, "GV") == 0) {
                            species->skills[GV] = skill;
                        } else if (strcmp(skill->code, "LS") == 0) {
                            species->skills[LS] = skill;
                        } else if (strcmp(skill->code, "BI") == 0) {
                            species->skills[BI] = skill;
                        } else {
                            fprintf(stderr, "%s: species.%s.code of '%s' is not valid\n", __FUNCTION__, t->key,
                                    skill->code);
                            exit(2);
                        }
                    }
                }
            } else if (strcmp(t->key, "sp_no") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 1 || t->value->u.n > MAX_SPECIES) {
                    fprintf(stderr, "%s: species.%s must be a number between 1 and %d\n", __FUNCTION__, t->key,
                            MAX_SPECIES);
                    exit(2);
                }
                species->id = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key 'species.%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return species;
}


global_species_t **unmarshalSpecies(json_value_t *j) {
    global_species_t **species = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_species_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, "species", "map");
                exit(2);
            }
            species[index] = unmarshalSpecie(t->value);
            index++;
        }
    }
    return species;
}


global_system_t *unmarshalSystem(json_value_t *j) {
    global_system_t *system = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_system_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "id") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "numeric");
                    exit(2);
                }
                system->id = t->value->u.n;
            } else if (strcmp(t->key, "color") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strcmp(t->value->u.s, "blue") == 0) {
                    system->color = BLUE;
                } else if (strcmp(t->value->u.s, "blue_white") == 0) {
                    system->color = BLUE_WHITE;
                } else if (strcmp(t->value->u.s, "white") == 0) {
                    system->color = WHITE;
                } else if (strcmp(t->value->u.s, "yellow_white") == 0) {
                    system->color = YELLOW_WHITE;
                } else if (strcmp(t->value->u.s, "yellow") == 0) {
                    system->color = YELLOW;
                } else if (strcmp(t->value->u.s, "orange") == 0) {
                    system->color = ORANGE;
                } else if (strcmp(t->value->u.s, "red") == 0) {
                    system->color = RED;
                } else {
                    fprintf(stderr, "%s: system.%s value of '%s' is not valid\n", __FUNCTION__, t->key, t->value->u.s);
                    exit(2);
                }
            } else if (strcmp(t->key, "coords") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                global_coords_t *coords = unmarshalCoords(t->value);
                system->coords.x = coords->x;
                system->coords.y = coords->y;
                system->coords.z = coords->z;
                free(coords);
            } else if (strcmp(t->key, "home_system") == 0) {
                if (!json_is_bool(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "boolean");
                    exit(2);
                }
                system->home_system = t->value->u.b;
            } else if (strcmp(t->key, "planets") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                system->planets = unmarshalPlanets(t->value);
            } else if (strcmp(t->key, "size") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > 9) {
                    fprintf(stderr, "%s: system.%s must be between 0 and 9\n", __FUNCTION__, t->key);
                    exit(2);
                }
                system->size = t->value->u.n;
            } else if (strcmp(t->key, "type") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                } else if (strcmp(t->value->u.s, "degenerate") == 0) {
                    system->type = DEGENERATE;
                } else if (strcmp(t->value->u.s, "dwarf") == 0) {
                    system->type = DWARF;
                } else if (strcmp(t->value->u.s, "giant") == 0) {
                    system->type = GIANT;
                } else if (strcmp(t->value->u.s, "main_sequence") == 0) {
                    system->type = MAIN_SEQUENCE;
                } else {
                    fprintf(stderr, "%s: system.%s value of '%s' is not valid\n", __FUNCTION__, t->key,
                            t->value->u.s);
                    exit(2);
                }
            } else if (strcmp(t->key, "visited_by") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                for (json_node_t *spNo = t->value->u.a.root; spNo != NULL; spNo = spNo->next) {
                    if (!json_is_number(spNo->value)) {
                        fprintf(stderr, "%s: system.%s values must be %s\n", __FUNCTION__, t->key, "number");
                        exit(2);
                    } else if (spNo->value->u.n < 1 || spNo->value->u.n > MAX_SPECIES) {
                        fprintf(stderr, "%s: system.%s values must be between 1 and %d\n", __FUNCTION__, t->key,
                                MAX_SPECIES);
                        exit(2);
                    }
                    system->visited_by[(spNo->value->u.n - 1) / 32] ^= (1 << ((spNo->value->u.n - 1) % 32));
                }
            } else if (strcmp(t->key, "wormhole_exit") == 0) {
                if (!json_is_number(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "number");
                    exit(2);
                } else if (t->value->u.n < 0 || t->value->u.n > MAX_STARS) {
                    fprintf(stderr, "%s: system.%s must be between 0 and %d\n", __FUNCTION__, t->key, MAX_STARS);
                    exit(2);
                }
                system->wormholeExit = t->value->u.n;
            } else {
                fprintf(stderr, "%s: unknown key '%s'\n", __FUNCTION__, t->key);
                exit(2);
            }
        }
    }
    return system;
}


global_system_t **unmarshalSystems(json_value_t *j) {
    global_system_t **systems = ncalloc(__FUNCTION__, __LINE__, json_length(j) + 1, sizeof(global_system_t *));
    if (json_is_list(j)) {
        int index = 0;
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (!json_is_map(t->value)) {
                fprintf(stderr, "%s: systems.%s must be %s\n", __FUNCTION__, "system", "map");
                exit(2);
            }
            systems[index] = unmarshalSystem(t->value);
            index++;
        }
    }
    return systems;
}
