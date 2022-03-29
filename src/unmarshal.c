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
#include "unmarshal.h"


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
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: cluster.%s must be %s\n", __FUNCTION__, t->key, "map");
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
            if (strcmp(t->key, "name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: colony.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(colony->name, t->value->u.s, 32);
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
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: data.%s must be %s\n", __FUNCTION__, t->key, "map");
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


global_planet_t *unmarshalPlanet(json_value_t *j) {
    global_planet_t *planet = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_planet_t));
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
            if (strcmp(t->key, "name") == 0) {
                if (!json_is_string(t->value)) {
                    fprintf(stderr, "%s: ship.%s must be %s\n", __FUNCTION__, t->key, "string");
                    exit(2);
                }
                strncpy(ship->name, t->value->u.s, 32);
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


global_species_t *unmarshalSpecie(json_value_t *j) {
    global_species_t *species = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(global_species_t));
    if (json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            if (strcmp(t->key, "colonies") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                species->colonies = unmarshalColonies(t->value);
            } else if (strcmp(t->key, "ships") == 0) {
                if (!json_is_map(t->value)) {
                    fprintf(stderr, "%s: species.%s must be %s\n", __FUNCTION__, t->key, "map");
                    exit(2);
                }
                species->ships = unmarshalShips(t->value);
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
            } else if (strcmp(t->key, "coords") == 0) {
            } else if (strcmp(t->key, "planets") == 0) {
                if (!json_is_list(t->value)) {
                    fprintf(stderr, "%s: system.%s must be %s\n", __FUNCTION__, t->key, "list");
                    exit(2);
                }
                system->planets = unmarshalPlanets(t->value);
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
