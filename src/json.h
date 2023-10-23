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

#ifndef FAR_HORIZONS_JSON_H
#define FAR_HORIZONS_JSON_H

#include <stdint.h>
#include <stdio.h>
#include "cJSON.h"
#include "engine.h"
#include "location.h"
#include "transaction.h"


typedef struct json_value {
    uint8_t flag;
    union {
        struct {
            struct json_node *root;
            struct json_node *tail;
        } a;     // linked list for lists and maps
        int b;   // boolean (0 == false, 1 == true)
        int n;   // number (integer)
        char *s; // string (nul terminated!)
    } u;
} json_value_t;


typedef struct json_node {
    char *key; // always NULL for list nodes
    json_value_t *value;
    struct json_node *next; // for chaining
} json_node_t;


int json_marshal(json_value_t *j, int indent, FILE *fp);

json_value_t *json_unmarshal(FILE *fp);

json_value_t *json_boolean(int v);

json_value_t *json_error(const char *fmt, ...);

json_value_t *json_free(json_value_t *j);

json_value_t *json_list(void);

json_value_t *json_map(void);

json_value_t *json_null(void);

json_value_t *json_number(int v);

json_value_t *json_string(char *v);

json_value_t *json_undefined(void);


// add a named value to a map
json_value_t *json_add(json_value_t *j, const char *name, json_value_t *v);

// add a value to the end of a list
json_value_t *json_append(json_value_t *j, json_value_t *v);

// length of a list or map
int json_length(json_value_t *j);


int json_is_atom(json_value_t *j);

int json_is_bool(json_value_t *j);

int json_is_error(json_value_t *j);

int json_is_list(json_value_t *j);

int json_is_map(json_value_t *j);

int json_is_null(json_value_t *j);

int json_is_number(json_value_t *j);

int json_is_string(json_value_t *j);

int json_is_undefined(json_value_t *j);


cJSON *jsonParseFile(char *name);

void jsonWriteFile(cJSON *root, char *kind, char *name);

void jsonAddIntToArray(cJSON *array, char *arrayName, int value);

void jsonAddBoolToObj(cJSON *obj, char *objName, char *propName, int value);

void jsonAddIntToObj(cJSON *obj, char *objName, char *propName, int value);

void jsonAddStringToObj(cJSON *obj, char *objName, char *propName, char *value);

int jsonGetBool(cJSON *obj, char *property);

int jsonGetInt(cJSON *obj, char *property);

const char *jsonGetString(cJSON *obj, char *property, int maxLength);


galaxy_data_t *galaxyDataFromJson(cJSON *root);

cJSON *galaxyDataToJson(galaxy_data_t *gd);

sp_loc_data_t *locationFromJson(cJSON *item);

cJSON *locationToJson(sp_loc_data_t *loc);

sp_loc_data_t **locationsFromJson(cJSON *root);

cJSON *locationsToJson(sp_loc_data_t *loc, int numLocations);

nampla_data_t *namedPlanetFromJson(cJSON *item);

cJSON *namedPlanetToJson(nampla_data_t *namedPlanet);

nampla_data_t **namedPlanetsFromJson(cJSON *root);

cJSON *namedPlanetsToJson(nampla_data_t *namedPlanets, int numPlanets);

planet_data_t *planetFromJson(cJSON *item);

cJSON *planetToJson(planet_data_t *planet, int id);

planet_data_t **planetsDataFromJson(cJSON *root);

cJSON *planetsDataToJson(planet_data_t *planetBase, int numPlanets);

ship_data_t *shipFromJson(cJSON *item);

cJSON *shipToJson(ship_data_t *ship);

cJSON *shipsToJson(ship_data_t *ships, int numShips);

ship_data_t **shipsFromJson(cJSON *root);

species_data_t *speciesFromJson(cJSON *root);

cJSON *speciesToJson(species_data_t *sp);

star_data_t **starsDataFromJson(cJSON *root);

cJSON *starsDataToJson(star_data_t *starBase, int numStars);

star_data_t *starFromJson(cJSON *item);

cJSON *starToJson(star_data_t *sd, int id);

cJSON *transactionsToJson(trans_data_t *transData, int numTransactions);

cJSON *transactionToJson(trans_data_t *td);


#endif //FAR_HORIZONS_JSON_H
