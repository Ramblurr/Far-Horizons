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
#include "json.h"


#define JSON_TYPE_UNDEFINED  0
#define JSON_TYPE_ARRAY      1
#define JSON_TYPE_BOOL       2
#define JSON_TYPE_INTEGER    4
#define JSON_TYPE_NUMBER     8
#define JSON_TYPE_NULL      16
#define JSON_TYPE_OBJECT    32
#define JSON_TYPE_STRING    64


void json__print(int indent, json_value_t *j, FILE *fp);


// json_add_kvp adds a key value pair to an object.
// panics if object is undefined, not an object, or not a null value.
json_value_t *json_add_kvp(json_value_t *j, json_kvp_t *kvp) {
    if (j == NULL) {
        fprintf(stderr, "json_add_kvp: j is NULL\n");
        exit(2);
    }
    // auto-promote undefined and null to objects
    if (j->flag == JSON_TYPE_UNDEFINED || j->flag == JSON_TYPE_NULL) {
        j->flag = JSON_TYPE_OBJECT;
    } else if (j->flag != JSON_TYPE_OBJECT) {
        fprintf(stderr, "json_add_kvp: j is not a valid object\n");
        exit(2);
    }

    return j;
}


json_value_t *json_append(json_value_t *j, json_value_t *v) {
    if (j == NULL) {
        j = json_new_array();
    } else if (j->flag == JSON_TYPE_UNDEFINED || j->flag == JSON_TYPE_NULL) {
        j->flag = JSON_TYPE_ARRAY;
    } else if (j->flag != JSON_TYPE_ARRAY) {
        fprintf(stderr, "json_append: j is not a valid array\n");
        exit(2);
    }
    json_array_t *e = calloc(1, sizeof(json_array_t));
    if (e == NULL) {
        perror("json_append:");
        exit(2);
    }
    e->v = v;
    if (j->u.a == NULL) {
        j->u.a = e;
    } else {
        json_array_t *t = j->u.a;
        for (; t->next != NULL; t = t->next) {
            //
        }
        t->next = e;
    }
    return j;
}


json_value_t *json_box_bool(int v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_bool:");
        exit(2);
    }
    j->flag = JSON_TYPE_BOOL;
    j->u.b = v ? 1 : 0;
    return j;
}


json_value_t *json_box_integer(int v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_integer:");
        exit(2);
    }
    j->flag = JSON_TYPE_INTEGER;
    j->u.i = v;
    return j;
}


json_kvp_t *json_box_kvp(char *key, json_value_t *value) {
    json_kvp_t *kvp = calloc(1, sizeof(json_kvp_t));
    if (kvp == NULL) {
        perror("json_box_kvp:");
        exit(2);
    }
    kvp->k = strdup(key);
    if (kvp->k == NULL) {
        perror("json_box_kvp:");
        exit(2);
    }
    kvp->v = value;
    return kvp;
}


json_value_t *json_box_null(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_null:");
        exit(2);
    }
    j->flag = JSON_TYPE_NULL;
    return j;
}


json_value_t *json_box_number(double v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_number:");
        exit(2);
    }
    j->flag = JSON_TYPE_NUMBER;
    j->u.n = v;
    return j;
}


json_value_t *json_box_string(char *v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_string:");
        exit(2);
    }
    j->flag = JSON_TYPE_STRING;
    j->u.s = strdup(v);
    if (j->u.s == NULL) {
        perror("json_box_string:");
        exit(2);
    }
    return j;
}


json_value_t *json_box_undefined(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_box_undefined:");
        exit(2);
    }
    j->flag = JSON_TYPE_UNDEFINED;
    return j;
}


int json_is_array(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_ARRAY;
}


int json_is_atom(json_value_t *j) {
    return !json_is_list(j);
}


int json_is_bool(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_BOOL;
}


int json_is_integer(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_INTEGER;
}


int json_is_list(json_value_t *j) {
    return j != NULL && (j->flag == JSON_TYPE_ARRAY || j->flag == JSON_TYPE_OBJECT);
}


int json_is_null(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_NULL;
}


int json_is_number(json_value_t *j) {
    return j != NULL && (j->flag == JSON_TYPE_NUMBER || j->flag == JSON_TYPE_INTEGER);
}


int json_is_object(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_OBJECT;
}


int json_is_string(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_STRING;
}


int json_is_undefined(json_value_t *j) {
    return j == NULL || j->flag == JSON_TYPE_UNDEFINED;
}


json_value_t *json_new_array(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_new_array:");
        exit(2);
    }
    j->flag = JSON_TYPE_ARRAY;
    return j;
}


json_value_t *json_new_object(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_new_object:");
        exit(2);
    }
    j->flag = JSON_TYPE_OBJECT;
    return j;
}


json_value_t *json_read(FILE *fp) {
    return NULL;
}


int json_write(json_value_t *j, FILE *fp) {
    json__print(0, j, fp);
    fprintf(fp, "\n");
    return 0;
}


void json__print(int indent, json_value_t *j, FILE *fp) {
    for (int i = indent; i > 0; i--) {
        fprintf(fp, " ");
    }
    if (j == NULL) {
        fprintf(fp, "undefined");
    } else {
        switch (j->flag) {
            case JSON_TYPE_UNDEFINED:
                fprintf(fp, "undefined");
                break;
            case JSON_TYPE_ARRAY:
                if (j->u.a == NULL) {
                    fprintf(fp, "[]");
                } else if (j->u.a->next == NULL && json_is_atom(j->u.a->v)) {
                    fprintf(fp, "[");
                    json__print(0, j->u.a->v, fp);
                    fprintf(fp, "]");
                } else {
                    fprintf(fp, "[\n");
                    for (json_array_t *t = j->u.a; t != NULL; t = t->next) {
                        json__print(indent + 2, t->v, fp);
                        if (t->next != NULL) {
                            fprintf(fp, ",\n");
                        } else {
                            fprintf(fp, "\n");
                        }
                    }
                    fprintf(fp, "]");
                }
                break;
            case JSON_TYPE_BOOL:
                if (j->u.b) {
                    fprintf(fp, "true");
                } else {
                    fprintf(fp, "false");
                }
                break;
            case JSON_TYPE_INTEGER:
                fprintf(fp, "%d", j->u.n);
                break;
            case JSON_TYPE_NUMBER:
                fprintf(fp, "%f", j->u.n);
                break;
            case JSON_TYPE_NULL:
                fprintf(fp, "null");
                break;
            case JSON_TYPE_OBJECT:
                fprintf(fp, "{\n");
                fprintf(fp, "  ...\n");
                fprintf(fp, "}");
                break;
            case JSON_TYPE_STRING:
                fprintf(fp, "\"%s\"", j->u.s);
                break;
            default:
                fprintf(fp, "undefined");
                break;
        }
    }
}