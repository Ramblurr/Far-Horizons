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


json_value_t *json_read(FILE *fp);

int json_write(json_value_t *j, FILE *fp);

json_value_t *json_boolean(int v);

json_value_t *json_error(const char *fmt, ...);

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


int json_is_atom(json_value_t *j);

int json_is_bool(json_value_t *j);

int json_is_error(json_value_t *j);

int json_is_list(json_value_t *j);

int json_is_map(json_value_t *j);

int json_is_null(json_value_t *j);

int json_is_number(json_value_t *j);

int json_is_string(json_value_t *j);

int json_is_undefined(json_value_t *j);


#endif //FAR_HORIZONS_JSON_H
