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
        struct json_array *a;  // array
        int b;                 // boolean (0 == false, 1 == true)
        int i;                 // number (integer)
        double n;              // number (real)
        struct json_object *o; // object
        char *s;               // string (nul terminated!)
    } u;
} json_value_t;


typedef struct json_array {
    struct json_array *next;
    struct json_value *v;
} json_array_t;


typedef struct json_kvp {
    char *k;              // key, string (nul terminated!)
    struct json_value *v; // value
} json_kvp_t;


typedef struct json_object {
    json_kvp_t **kv;
} json_object_t;


json_value_t *json_read(FILE *fp);

json_value_t *json_new_array(void);

json_value_t *json_new_object(void);

json_value_t *json_append(json_value_t *j, json_value_t *v);

json_value_t *json_box_bool(int v);

json_value_t *json_box_integer(int v);

json_kvp_t *json_box_kvp(char *key, json_value_t *value);

json_value_t *json_box_null(void);

json_value_t *json_box_number(double v);

json_value_t *json_box_string(char *v);

json_value_t *json_box_undefined(void);

int json_is_array(json_value_t *j);

int json_is_atom(json_value_t *j);

int json_is_bool(json_value_t *j);

int json_is_integer(json_value_t *j);

int json_is_list(json_value_t *j);

int json_is_number(json_value_t *j);

int json_is_null(json_value_t *j);

int json_is_object(json_value_t *j);

int json_is_string(json_value_t *j);

int json_is_undefined(json_value_t *j);

int json_write(json_value_t *j, FILE *fp);


#endif //FAR_HORIZONS_JSON_H
