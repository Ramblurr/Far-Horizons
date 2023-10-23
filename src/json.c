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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "engine.h"
#include "json.h"
#include "cjson/cJSON.h"
#include "galaxyio.h"
#include "star.h"
#include "speciesio.h"


#define JSON_TYPE_UNDEFINED  0
#define JSON_TYPE_BOOL       1
#define JSON_TYPE_ERROR      2
#define JSON_TYPE_LIST       4
#define JSON_TYPE_MAP        8
#define JSON_TYPE_NULL      16
#define JSON_TYPE_NUMBER    32
#define JSON_TYPE_STRING    64


// read https://en.wikipedia.org/wiki/Recursive_descent_parser for background on parsing.

typedef enum {
    unknown,
    beginlist, endlist,
    beginmap, endmap,
    boolean, null, number, string,
    colon, comma,
    eof
} json_symbol_t;


typedef struct json_parser {
    FILE *fp;
    int line;
    json_symbol_t sym;
    char buffer[64];
    char prior[64]; // prior symbol
    char errmsg[128];
} json_parser_t;


static int jp__accept(json_parser_t *p, json_symbol_t s);

static void jp__error(json_parser_t *p, const char *fmt, ...);

static int jp__expect(json_parser_t *p, json_symbol_t s);

static void jp__nextsym(json_parser_t *p);

static char *jp__stringer(json_symbol_t s);

static json_value_t *jp__value(json_parser_t *p);


int json_marshal(json_value_t *j, int indent, FILE *fp) {
    if (json_is_atom(j)) {
        if (j == NULL || j->flag == JSON_TYPE_UNDEFINED) {
            fprintf(fp, "undefined");
        } else if (j->flag == JSON_TYPE_BOOL) {
            if (j->u.b) {
                fprintf(fp, "true");
            } else {
                fprintf(fp, "false");
            }
        } else if (j->flag == JSON_TYPE_ERROR) {
            fprintf(fp, "error(%s)", j->u.s);
        } else if (j->flag == JSON_TYPE_NUMBER) {
            fprintf(fp, "%d", j->u.n);
        } else if (j->flag == JSON_TYPE_NULL) {
            fprintf(fp, "null");
        } else if (j->flag == JSON_TYPE_STRING) {
            fprintf(fp, "\"%s\"", j->u.s);
        } else {
            fprintf(fp, "error(internal)");
        }
        return 0;
    }

    fprintf(fp, "\n");
    for (int i = indent; i > 0; i--) {
        fprintf(fp, "\t");
    }
    fprintf(fp, "%c", j->flag == JSON_TYPE_LIST ? '[' : '{');
    if (j->u.a.root != NULL) {
        for (json_node_t *n = j->u.a.root; n != NULL; n = n->next) {
            fprintf(fp, "\n");
            for (int i = indent + 1; i > 0; i--) {
                fprintf(fp, "\t");
            }
            if (j->flag == JSON_TYPE_MAP) {
                fprintf(fp, "\"%s\":", n->key);
            }
            json_marshal(n->value, indent + 1, fp);
            fprintf(fp, "%s", n->next == NULL ? "" : ",");
        }
        fprintf(fp, "\n");
        for (int i = indent; i > 0; i--) {
            fprintf(fp, "\t");
        }
    }
    fprintf(fp, "%c", j->flag == JSON_TYPE_LIST ? ']' : '}');

    return 0;
}


json_value_t *json_unmarshal(FILE *fp) {
    json_parser_t p;
    memset(&p, 0, sizeof(p));
    p.fp = fp;
    p.line = 1;

    jp__nextsym(&p);
    json_value_t *value = jp__value(&p);
    jp__expect(&p, eof);

    return value;
}


// json_add adds a named value to a map.
// panics if object is not a map, null, or undefined value.
// (null and undefined values will be promoted to maps.)
json_value_t *json_add(json_value_t *j, const char *key, json_value_t *value) {
    if (j == NULL) {
        j = json_map();
        if (j == NULL) {
            perror("json_add:");
            exit(2);
        }
    } else if (j->flag == JSON_TYPE_UNDEFINED || j->flag == JSON_TYPE_NULL) {
        // auto-promote undefined and null to maps
        j->flag = JSON_TYPE_MAP;
    } else if (j->flag != JSON_TYPE_MAP) {
        fprintf(stderr, "json_add: j is not a valid map\n");
        exit(2);
    }

    json_node_t *t = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_node_t));
    if (t == NULL) {
        perror("json_add:");
        exit(2);
    }
    t->key = strdup(key);
    if (t->key == NULL) {
        perror("json_add:");
        exit(2);
    }
    t->value = value;

    if (j->u.a.root == NULL) {
        j->u.a.root = t;
    } else {
        for (json_node_t *c = j->u.a.root; c; c = c->next) {
            if (strcmp(c->key, key) == 0) {
                // json_free_value(t->value);
                c->value = value;
                break;
            } else if (c->next == NULL) {
                c->next = t;
                break;
            }
        }
    }

    return j;
}


// json_append adds a value to the end of a list.
// panics if array is not a list, null value, or undefined.
// (null and undefined values will be promoted to a list.)
json_value_t *json_append(json_value_t *j, json_value_t *value) {
    if (j == NULL) {
        j = json_list();
        if (j == NULL) {
            perror("json_append:");
            exit(2);
        }
    } else if (j->flag == JSON_TYPE_UNDEFINED || j->flag == JSON_TYPE_NULL) {
        j->flag = JSON_TYPE_LIST;
    } else if (j->flag != JSON_TYPE_LIST) {
        fprintf(stderr, "json_append: j is not a valid list\n");
        exit(2);
    }

    json_node_t *t = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_node_t));
    if (t == NULL) {
        perror("json_append:");
        exit(2);
    }
    t->value = value;

    if (j->u.a.root == NULL) {
        j->u.a.root = t;
        j->u.a.tail = t;
    } else {
        j->u.a.tail->next = t;
        j->u.a.tail = t;
    }

    return j;
}


json_value_t *json_boolean(int v) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_boolean:");
        exit(2);
    }
    j->flag = JSON_TYPE_BOOL;
    j->u.b = v ? 1 : 0;
    return j;
}


json_value_t *json_error(const char *fmt, ...) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_error:");
        exit(2);
    }
    char buffer[256];
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(buffer, 256, fmt, arg_ptr);
    va_end(arg_ptr);
    j->flag = JSON_TYPE_ERROR;
    j->u.s = strdup(buffer);
    if (j->u.s == NULL) {
        perror("json_error:");
        exit(2);
    }
    return j;
}


int json_length(json_value_t *j) {
    int length = 0;
    if (json_is_list(j) || json_is_map(j)) {
        for (json_node_t *t = j->u.a.root; t != NULL; t = t->next) {
            length++;
        }
    }
    return length;
}


json_value_t *json_list(void) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_list:");
        exit(2);
    }
    j->flag = JSON_TYPE_LIST;
    return j;
}


json_value_t *json_map(void) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_map:");
        exit(2);
    }
    j->flag = JSON_TYPE_MAP;
    return j;
}


json_value_t *json_null(void) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_null:");
        exit(2);
    }
    j->flag = JSON_TYPE_NULL;
    return j;
}


json_value_t *json_number(int v) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_number:");
        exit(2);
    }
    j->flag = JSON_TYPE_NUMBER;
    j->u.n = v;
    return j;
}


json_value_t *json_string(char *v) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_string:");
        exit(2);
    }
    j->flag = JSON_TYPE_STRING;
    j->u.s = strdup(v);
    if (j->u.s == NULL) {
        perror("json_string:");
        exit(2);
    }
    return j;
}


json_value_t *json_undefined(void) {
    json_value_t *j = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_undefined:");
        exit(2);
    }
    j->flag = JSON_TYPE_UNDEFINED;
    return j;
}


int json_is_atom(json_value_t *j) {
    return j == NULL || (j->flag != JSON_TYPE_LIST && j->flag != JSON_TYPE_MAP);
}


int json_is_bool(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_BOOL;
}


int json_is_list(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_LIST;
}


int json_is_map(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_MAP;
}


int json_is_null(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_NULL;
}


int json_is_number(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_NUMBER;
}


int json_is_string(json_value_t *j) {
    return j != NULL && j->flag == JSON_TYPE_STRING;
}


int json_is_undefined(json_value_t *j) {
    return j == NULL || j->flag == JSON_TYPE_UNDEFINED;
}


// jp__accept returns 0 (false) if the current symbol is not `s`.
// if it is, the parser advances to the next symbol, and we return 1 (true).
int jp__accept(json_parser_t *p, json_symbol_t s) {
    if (p->sym == s) {
        //if (p->sym == string) {
        //    printf("accept: accepted %-13s \"%s\"\n", jp__stringer(s), p->buffer);
        //} else {
        //    printf("accept: accepted %-13s '%s'\n", jp__stringer(s), p->buffer);
        //}
        jp__nextsym(p);
        return 1;
    }
    //printf("accept: not accepted %s (%s)\n", jp__stringer(s), p->buffer);
    return 0;
}


void jp__error(json_parser_t *p, const char *fmt, ...) {
    fflush(stdout);

    fprintf(stderr, "%d: ", p->line);

    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vfprintf(stderr, fmt, arg_ptr);
    va_end(arg_ptr);

    exit(2);
}


// expect throws and error if the current symbol is not `s`.
// if it is, the parser advances to the next symbol, and we return 1 (true).
int jp__expect(json_parser_t *p, json_symbol_t s) {
    if (jp__accept(p, s)) {
        return 1;
    }
    jp__error(p, "expected %s: got %s", jp__stringer(s), jp__stringer(p->sym));
    return 0;
}


void jp__nextsym(json_parser_t *p) {
    // save the current symbol
    strcpy(p->prior, p->buffer);
    p->errmsg[0] = 0;

    char *buf = p->buffer;
    char *eob = buf + 63;
    int ch;
    for (ch = fgetc(p->fp); isspace(ch); ch = fgetc(p->fp)) {
        if (ch == '\n') {
            p->line++;
        }
    }
    switch (ch) {
        case EOF:
            p->sym = eof;
            sprintf(p->buffer, "eof");
            break;
        case '{':
            p->sym = beginmap;
            *(buf++) = ch;
            *buf = 0;
            break;
        case '}':
            p->sym = endmap;
            *(buf++) = ch;
            *buf = 0;
            break;
        case '[':
            p->sym = beginlist;
            *(buf++) = ch;
            *buf = 0;
            break;
        case ']':
            p->sym = endlist;
            *(buf++) = ch;
            *buf = 0;
            break;
        case ':':
            p->sym = colon;
            *(buf++) = ch;
            *buf = 0;
            break;
        case ',':
            p->sym = comma;
            *(buf++) = ch;
            *buf = 0;
            break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            p->sym = number;
            *(buf++) = ch;
            for (ch = fgetc(p->fp); isdigit(ch); ch = fgetc(p->fp)) {
                if (buf < eob) {
                    *(buf++) = ch;
                }
            }
            if (ch != EOF) {
                ungetc(ch, p->fp);
            }
            *buf = 0;
            if (p->buffer[0] == '-' && p->buffer[1] == 0) {
                p->sym = unknown;
            }
            break;
        case '"':
            p->sym = string;
            for (ch = fgetc(p->fp); ch != '"' && ch != EOF; ch = fgetc(p->fp)) {
                if (ch == '\n') {
                    ungetc(ch, p->fp);
                    break;
                }
                if (buf < eob) {
                    *(buf++) = ch;
                }
            }
            *buf = 0;
            if (ch == EOF) {
                p->sym = unknown;
                jp__error(p, "%d: unterminated string", p->line);
            }
            break;
        default:
            *(buf++) = ch;
            for (ch = fgetc(p->fp); !(isspace(ch)
                                      || ch == ',' || ch == ':' || ch == '{' || ch == '}'
                                      || ch == '[' || ch == ']'); ch = fgetc(p->fp)) {
                if (buf < eob) {
                    *(buf++) = ch;
                }
            }
            if (ch != EOF) {
                ungetc(ch, p->fp);
            }
            *buf = 0; // must terminate before comparing
            if (strcmp(p->buffer, "false") == 0 || strcmp(p->buffer, "true") == 0) {
                p->sym = boolean;
            } else if (strcmp(p->buffer, "null") == 0) {
                p->sym = null;
            } else {
                p->sym = unknown;
            }
    }
}


json_value_t *jp__value(json_parser_t *p) {
    json_value_t *j = NULL;
    char name[64];

    if (jp__accept(p, boolean)) {
        j = json_boolean(p->prior[0] == 't');
    } else if (jp__accept(p, null)) {
        j = json_null();
    } else if (jp__accept(p, number)) {
        j = json_number(atoi(p->prior));
    } else if (jp__accept(p, string)) {
        j = json_string(p->prior);
    } else if (jp__accept(p, beginlist)) {
        j = json_list();
        if (p->sym != endlist) {
            do {
                json_append(j, jp__value(p));
            } while (jp__accept(p, comma));
        }
        if (!jp__accept(p, endlist)) {
            json_append(j, json_error("%d: unterminated list", p->line));
            // consume the remainder of the file
            while (!jp__accept(p, eof)) {
                jp__nextsym(p);
            }
        }
    } else if (jp__accept(p, beginmap)) {
        j = json_map();
        if (p->sym == string) {
            do {
                jp__expect(p, string);
                strcpy(name, p->prior);
                jp__expect(p, colon);
                json_add(j, name, jp__value(p));
            } while (jp__accept(p, comma));
        }
        if (!jp__accept(p, endmap)) {
            json_add(j, "error", json_error("%d: unterminated map", p->line));
            // consume the remainder of the file
            while (!jp__accept(p, eof)) {
                jp__nextsym(p);
            }
        }
    } else {
        j = json_error("%d: unexpected symbol '%s' (%s)", p->line, p->buffer, jp__stringer(p->sym));
        // consume the remainder of the file
        while (!jp__accept(p, eof)) {
            jp__nextsym(p);
        }
    }
    return j;
}


char *jp__stringer(json_symbol_t s) {
    switch (s) {
        case beginlist:
            return "beginlist";
        case beginmap:
            return "beginmap";
        case boolean:
            return "boolean";
        case colon:
            return "colon";
        case comma:
            return "comma";
        case endlist:
            return "endlist";
        case endmap:
            return "endmap";
        case eof:
            return "eof";
        case null:
            return "null";
        case number:
            return "number";
        case string:
            return "string";
        case unknown:
            return "unknown";
        default:
            return "*unknown*";
    }
}

void jsonAddIntToArray(cJSON *array, char *arrayName, int value) {
    cJSON *item = cJSON_CreateNumber((double) value);
    if (item == 0) {
        fprintf(stderr, "%s: unable to integer for array\n", arrayName);
        perror("cJSON_CreateNumber");
        exit(2);
    } else if (cJSON_AddItemToArray(array, item) == 0) {
        fprintf(stderr, "%s: unable to add integer to array\n", arrayName);
        perror("cJSON_AddItemToArray");
        exit(2);
    }
}

void jsonAddBoolToObj(cJSON *obj, char *objName, char *propName, int value) {
    if (cJSON_AddBoolToObject(obj, propName, value) == 0) {
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        perror("cJSON_AddBoolToObject");
        exit(2);
    }
}

void jsonAddIntToObj(cJSON *obj, char *objName, char *propName, int value) {
    if (cJSON_AddNumberToObject(obj, propName, (double) (value)) == 0) {
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        perror("cJSON_AddNumberToObject");
        exit(2);
    }
}

void jsonAddStringToObj(cJSON *obj, char *objName, char *propName, char *value) {
    if (cJSON_AddStringToObject(obj, propName, value) == 0) {
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        perror("cJSON_AddStringToObject");
        exit(2);
    }
}

int jsonGetBool(cJSON *obj, char *property) {
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, property);
    if (item == 0) {
        fprintf(stderr, "property: %s: missing\n", property);
        exit(2);
    } else if (!cJSON_IsBool(item)) {
        fprintf(stderr, "property: %s: not a boolean\n", property);
        exit(2);
    }
    if (item->valueint == 0) {
        return 0;
    }
    return 1;
}

int jsonGetInt(cJSON *obj, char *property) {
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, property);
    if (item == 0) {
        fprintf(stderr, "property: %s: missing\n", property);
        exit(2);
    } else if (!cJSON_IsNumber(item)) {
        fprintf(stderr, "property: %s: not an integer\n", property);
        exit(2);
    }
    return item->valueint;
}

// jsonGetString should copy up to maxLength - 1 bytes.
const char *jsonGetString(cJSON *obj, char *property, int maxLength) {
    static char buffer[1024];
    if (maxLength > 1023) {
        fprintf(stderr, "jsonGetString: maxLength %d exceeds limit\n", maxLength);
        exit(2);
    }
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, property);
    if (item == 0) {
        fprintf(stderr, "property: %s: missing\n", property);
        exit(2);
    } else if (!cJSON_IsString(item)) {
        fprintf(stderr, "property: %s: not a string\n", property);
        exit(2);
    }
    if (strlen(item->valuestring) > maxLength) {
        fprintf(stderr, "jsonGetString: strlen %d exceeds limit %d\n", (int) strlen(item->valuestring) + 1, maxLength);
        exit(2);
    }
    strcpy(buffer, item->valuestring);
    buffer[maxLength - 1] = 0;
    return buffer;
}

cJSON *jsonParseFile(char *name) {
    FILE *fp = fopen(name, "rb");
    if (fp == 0) {
        return 0;
    }

    // determine the length of the file
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("json: parseFile: seeking to end of input");
        exit(2);
    }
    long length = ftell(fp);
    char *buffer = malloc(length + 1);
    if (buffer == 0) {
        perror("json: parseFile: allocating buffer");
        exit(2);
    }
    // return to the beginning and read the entire file
    if (fseek(fp, 0, SEEK_SET) != 0) {
        perror("json: parseFile: seeking to start of input");
        exit(2);
    } else if (fread(buffer, 1, length, fp) != length) {
        perror("json: parseFile: reading entire input");
        exit(2);
    }
    // and close the file to avoid leaking a handle
    fclose(fp);

    // terminate the buffer with a nul byte
    buffer[length] = 0;

    // try to parse the JSON
    cJSON *root = cJSON_Parse(buffer);
    if (root == 0) {
        cJSON_Error_t err = cJSON_GetError();
        fprintf(stderr, "%s:%d:%d: error parsing just before\n\t%s\n",
                name, err.line, err.col, err.text);
        exit(2);
    }

    return root;
}

void jsonWriteFile(cJSON *root, char *kind, char *name) {
    // convert json to text
    char *string = cJSON_Print(root);
    if (string == 0) {
        fprintf(stderr, "json: there was an error converting %s json to text\n", kind);
        exit(2);
    }
    // save it to the file and close it
    FILE *fp = fopen(name, "wb");
    if (fp == NULL) {
        perror("fh: export: json:");
        fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", name);
        exit(2);
    }
    fprintf(fp, "%s\n", string);
    fclose(fp);
    // release the memory
    free(string);
}

galaxy_data_t *galaxyDataFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "galaxy.json: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *obj = cJSON_GetObjectItem(root, "galaxy");
    if (obj == 0) {
        fprintf(stderr, "galaxy.json: galaxy: missing property\n");
        exit(2);
    }
    galaxy_data_t *g = calloc(sizeof(struct galaxy_data), 1);
    if (g == 0) {
        perror("galaxyDataFromJson");
        exit(2);
    }
    g->turn_number = jsonGetInt(obj, "turn_number");
    g->num_species = jsonGetInt(obj, "num_species");
    g->d_num_species = jsonGetInt(obj, "d_num_species");
    g->radius = jsonGetInt(obj, "radius");
    return g;
}


cJSON *galaxyDataToJson(galaxy_data_t *gd) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "galaxy.json: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(root, "root", "version", 1);
    cJSON *obj = cJSON_AddObjectToObject(root, "galaxy");
    if (obj == 0) {
        fprintf(stderr, "galaxy.json: unable to allocate property 'galaxy'\n");
        perror("cJSON_AddObjectToObject");
        exit(2);
    }
    char *objName = "galaxy";
    jsonAddIntToObj(obj, objName, "turn_number", gd->turn_number);
    jsonAddIntToObj(obj, objName, "num_species", gd->num_species);
    jsonAddIntToObj(obj, objName, "d_num_species", gd->d_num_species);
    jsonAddIntToObj(obj, objName, "radius", gd->radius);
    return root;
}

sp_loc_data_t *locationFromJson(cJSON *item) {
    sp_loc_data_t *location = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(sp_loc_data_t));
    location->x = jsonGetInt(item, "x");
    location->y = jsonGetInt(item, "y");
    location->z = jsonGetInt(item, "z");
    location->s = jsonGetInt(item, "species");
    return location;
}


cJSON *locationToJson(sp_loc_data_t *loc) {
    char *objName = "location";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "x", loc->x);
    jsonAddIntToObj(obj, objName, "y", loc->y);
    jsonAddIntToObj(obj, objName, "z", loc->z);
    jsonAddIntToObj(obj, objName, "species", loc->s);
    return obj;
}

sp_loc_data_t **locationsFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "locations: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *array = cJSON_GetObjectItem(root, "locations");
    if (array == 0) {
        fprintf(stderr, "locations: missing property 'locations'\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "locations: property 'locations' is not array\n");
        exit(2);
    }
    int numLocations = cJSON_GetArraySize(array);
    if (numLocations < 0) {
        fprintf(stderr, "locations: expected at least %d locations, got %d\n", 0, numLocations);
        exit(2);
    } else if (numLocations > MAX_LOCATIONS) {
        fprintf(stderr, "locations: expected at most %d locations, got %d\n", MAX_LOCATIONS, numLocations);
        exit(2);
    }
    // allocate enough memory for all locations plus an empty terminator
    sp_loc_data_t **locations = ncalloc(__FUNCTION__, __LINE__, numLocations + 1, sizeof(sp_loc_data_t *));
    // and load them all into the array
    int idx = 0;
    cJSON *item = 0;
    cJSON_ArrayForEach(item, array) {
        locations[idx] = locationFromJson(item);
        idx++;
    }
    return locations;
}

cJSON *locationsToJson(sp_loc_data_t *locations, int numLocations) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "locations: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    char *objName = "locations";
    jsonAddIntToObj(root, objName, "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "locations");
    if (array == 0) {
        fprintf(stderr, "locations: unable to allocate property 'locations'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numLocations; i++) {
        if (!cJSON_AddItemToArray(array, locationToJson(&locations[i]))) {
            perror("locations: unable to extend array");
            exit(2);
        }
    }
    return root;
}

nampla_data_t **namedPlanetsFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "namedPlanets: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *obj = cJSON_GetObjectItem(root, "named_planets");
    if (obj == 0) {
        fprintf(stderr, "namedPlanets: named_planets: missing property\n");
        exit(2);
    }
    nampla_data_t **namedPlanets = 0;
    return namedPlanets;
}

cJSON *namedPlanetToJson(struct nampla_data *np) {
    char *objName = "named_planet";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "id", np->id);
    jsonAddStringToObj(obj, objName, "name", np->name);
    cJSON *planet = cJSON_AddObjectToObject(obj, "planet");
    if (planet == 0) {
        fprintf(stderr, "%s: unable to allocate property 'planet'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(planet, "planet.id", "id", np->planet_index);
        jsonAddIntToObj(planet, "planet.x", "x", np->x);
        jsonAddIntToObj(planet, "planet.y", "y", np->y);
        jsonAddIntToObj(planet, "planet.z", "z", np->z);
        jsonAddIntToObj(planet, "planet.orbit", "orbit", np->pn);
    }
    jsonAddIntToObj(obj, objName, "status", np->status);
    jsonAddIntToObj(obj, objName, "hiding", np->hiding);
    jsonAddIntToObj(obj, objName, "hidden", np->hidden);
    jsonAddIntToObj(obj, objName, "ma_base", np->ma_base);
    jsonAddIntToObj(obj, objName, "mi_base", np->mi_base);
    jsonAddIntToObj(obj, objName, "pop_units", np->pop_units);
    jsonAddIntToObj(obj, objName, "shipyards", np->shipyards);
    jsonAddIntToObj(obj, objName, "siege_eff", np->siege_eff);
    jsonAddIntToObj(obj, objName, "special", np->special);
    jsonAddIntToObj(obj, objName, "use_on_ambush", np->use_on_ambush);
    cJSON *au = cJSON_AddObjectToObject(obj, "au");
    if (au == 0) {
        fprintf(stderr, "%s: unable to allocate property 'au'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(au, "au.auto", "auto", np->auto_AUs);
        jsonAddIntToObj(au, "au.needed", "needed", np->AUs_needed);
        jsonAddIntToObj(au, "au.to_install", "to_install", np->AUs_to_install);
    }
    cJSON *iu = cJSON_AddObjectToObject(obj, "iu");
    if (iu == 0) {
        fprintf(stderr, "%s: unable to allocate property 'iu'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(iu, "iu.auto", "auto", np->auto_IUs);
        jsonAddIntToObj(iu, "iu.needed", "needed", np->IUs_needed);
        jsonAddIntToObj(iu, "iu.to_install", "to_install", np->IUs_to_install);
    }
    cJSON *items = cJSON_AddArrayToObject(obj, "items");
    if (items == 0) {
        fprintf(stderr, "%s: unable to allocate property 'items'\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    } else {
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (np->item_quantity[j] > 0) {
                cJSON *item = cJSON_CreateObject();
                jsonAddIntToObj(item, "item.code", "code", j);
                jsonAddIntToObj(item, "item.qty", "qty", np->item_quantity[j]);
                cJSON_AddItemToArray(items, item);
            }
        }
    }
    return obj;
}


cJSON *namedPlanetsToJson(nampla_data_t *namedPlanets, int numPlanets) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "named_planets: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(root, "named_planets", "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "named_planets");
    if (array == 0) {
        fprintf(stderr, "named_planets: unable to allocate property 'named_planets'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numPlanets; i++) {
        if (!cJSON_AddItemToArray(array, namedPlanetToJson(&namedPlanets[i]))) {
            fprintf(stderr, "named_planets: unable to extend array 'named_planets'\n");
            perror("cJSON_AddItemToArray");
            exit(2);
        }
    }
    return root;
}

planet_data_t *planetFromJson(cJSON *item) {
    planet_data_t *planet = (planet_data_t *) ncalloc(__FUNCTION__, __LINE__, 1, sizeof(planet_data_t));
    planet->id = jsonGetInt(item, "id");
    planet->diameter = jsonGetInt(item, "diameter");
    planet->gravity = jsonGetInt(item, "gravity");
    planet->temperature_class = jsonGetInt(item, "temperature_class");
    planet->pressure_class = jsonGetInt(item, "pressure_class");
    planet->special = jsonGetInt(item, "special");
    cJSON *gases = cJSON_GetObjectItem(item, "gases");
    if (gases == 0) {
        fprintf(stderr, "planet: %d: missing property 'gases'\n", planet->id);
        exit(2);
    } else if (!cJSON_IsArray(gases)) {
        fprintf(stderr, "planet: %d: gases is not an array\n", planet->id);
        exit(2);
    } else if (cJSON_GetArraySize(gases) != 4) {
        fprintf(stderr, "planet: %d: gases must contain exactly 4 items\n", planet->id);
        exit(2);
    } else {
        cJSON *gas = 0;
        int idx = 0;
        cJSON_ArrayForEach(gas, gases) {
            if (!cJSON_IsObject(gas)) {
                fprintf(stderr, "planet: %d: gases[%d] is not an object\n", planet->id, idx);
                exit(2);
            }
            planet->gas[idx] = jsonGetInt(gas, "code");
            planet->gas_percent[idx] = jsonGetInt(gas, "percent");
            idx++;
        }
    }
    cJSON *mining_difficulty = cJSON_GetObjectItem(item, "mining_difficulty");
    if (mining_difficulty == 0) {
        fprintf(stderr, "planet: %d: missing property 'mining_difficulty'\n", planet->id);
        exit(2);
    } else if (!cJSON_IsObject(mining_difficulty)) {
        fprintf(stderr, "planet: %d: mining_difficulty is not an object\n", planet->id);
        exit(2);
    } else {
        planet->mining_difficulty = jsonGetInt(mining_difficulty, "base");
        planet->md_increase = jsonGetInt(mining_difficulty, "increase");
    }
    planet->econ_efficiency = jsonGetInt(item, "econ_efficiency");
    planet->message = jsonGetInt(item, "message");
    return planet;
}

cJSON *planetToJson(planet_data_t *planet, int id) {
    char *objName = "planetToJson";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "id", id);
    jsonAddIntToObj(obj, objName, "diameter", planet->diameter);
    jsonAddIntToObj(obj, objName, "gravity", planet->gravity);
    jsonAddIntToObj(obj, objName, "temperature_class", planet->temperature_class);
    jsonAddIntToObj(obj, objName, "pressure_class", planet->pressure_class);
    jsonAddIntToObj(obj, objName, "special", planet->special);
    cJSON *gases = cJSON_AddArrayToObject(obj, "gases");
    if (gases == 0) {
        fprintf(stderr, "%s: unable to allocate gases property\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int j = 0; j < 4; j++) {
        cJSON *gas = cJSON_AddObjectToObject(gases, "gases");
        if (gas == NULL) {
            fprintf(stderr, "%s: unable to allocate gas property\n", objName);
            perror("cJSON_AddObjectToObject");
            exit(2);
        }
        jsonAddIntToObj(gas, "gases", "code", planet->gas[j]);
        jsonAddIntToObj(gas, "gases", "percent", planet->gas_percent[j]);
    }
    cJSON *miningDifficulty = cJSON_AddObjectToObject(obj, "mining_difficulty");
    if (miningDifficulty == 0) {
        fprintf(stderr, "%s: unable to allocate mining_difficulty property\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    }
    jsonAddIntToObj(miningDifficulty, "mining_difficulty", "base", planet->mining_difficulty);
    jsonAddIntToObj(miningDifficulty, "mining_difficulty", "increase", planet->md_increase);
    jsonAddIntToObj(obj, objName, "econ_efficiency", planet->econ_efficiency);
    jsonAddIntToObj(obj, objName, "message", planet->message);
    return obj;
}

planet_data_t **planetsDataFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "planets.json: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *array = cJSON_GetObjectItem(root, "planets");
    if (array == 0) {
        fprintf(stderr, "planets.json: planets: missing property\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "planets.json: planets: property is not array\n");
        exit(2);
    }
    int numPlanets = cJSON_GetArraySize(array);
    if (numPlanets < 1) {
        fprintf(stderr, "planets.json: stars: expected at least %d planets, got %d\n", 1, numPlanets);
        exit(2);
    } else if (numPlanets > MAX_PLANETS) {
        fprintf(stderr, "planets.json: stars: expected at most %d planets, got %d\n", MAX_PLANETS, numPlanets);
        exit(2);
    }
    // allocate enough memory for all planets plus an empty terminator
    planet_data_t **planets = (planet_data_t **) ncalloc(__FUNCTION__, __LINE__, numPlanets + 1,
                                                         sizeof(planet_data_t *));
    if (planets == NULL) {
        fprintf(stderr, "planets.json: stars: failed to allocate %d planets\n", numPlanets);
        exit(2);
    }
    // and load all the planets
    int idx = 0;
    cJSON *item = 0;
    cJSON_ArrayForEach(item, array) {
        planet_data_t *planet = planetFromJson(item);
        if (planet->id != idx + 1) {
            fprintf(stderr, "stars.json: stars: index %d: id: expect %d, got %d\n", idx, idx + 1, planet->id);
            exit(2);
        }
        planets[idx] = planet;
        idx = idx + 1;
    }
    return planets;
}

// planetsDataToJson translates the current planet_base array to JSON.
cJSON *planetsDataToJson(planet_data_t *planetBase, int numPlanets) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "planets.json: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    char *objName = "planets";
    jsonAddIntToObj(root, objName, "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "planets");
    if (array == 0) {
        fprintf(stderr, "planets.json: unable to allocate property 'planets'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numPlanets; i++) {
        int id = i + 1;
        if (!cJSON_AddItemToArray(array, planetToJson(&planetBase[i], id))) {
            perror("planets.json: unable to extend array");
            exit(2);
        }
    }
    return root;
}

ship_data_t *shipFromJson(cJSON *item) {
    ship_data_t *sd = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(ship_data_t));
    sd->id = jsonGetInt(item, "id");
    strcpy(sd->name, jsonGetString(item, "name", sizeof(sd->name)));
    cJSON *location = cJSON_GetObjectItem(item, "location");
    if (location == 0) {
        fprintf(stderr, "ship: missing property 'location'\n");
        exit(2);
    } else if (!cJSON_IsObject(location)) {
        fprintf(stderr, "ship: property 'location' is not an object\n");
        exit(2);
    } else {
        sd->x = jsonGetInt(location, "x");
        sd->y = jsonGetInt(location, "y");
        sd->z = jsonGetInt(location, "z");
        sd->pn = jsonGetInt(location, "orbit");
        sd->status = jsonGetInt(location, "status");
    }
    cJSON *destination = cJSON_GetObjectItem(item, "destination");
    if (destination == 0) {
        fprintf(stderr, "ship: missing property 'destination'\n");
        exit(2);
    } else if (!cJSON_IsObject(destination)) {
        fprintf(stderr, "ship: property 'destination' is not an object\n");
        exit(2);
    } else {
        sd->dest_x = jsonGetInt(destination, "x");
        sd->dest_y = jsonGetInt(destination, "y");
        sd->dest_z = jsonGetInt(destination, "z");
    }
    sd->age = jsonGetInt(item, "age");
    sd->arrived_via_wormhole = jsonGetBool(item, "arrived_via_wormhole");
    sd->class = jsonGetInt(item, "class");
    sd->just_jumped = jsonGetInt(item, "just_jumped");
    sd->loading_point = jsonGetInt(item, "loading_point");
    sd->remaining_cost = jsonGetInt(item, "remaining_cost");
    sd->tonnage = jsonGetInt(item, "tonnage");
    sd->type = jsonGetInt(item, "type");
    sd->unloading_point = jsonGetInt(item, "unloading_point");
    cJSON *cargo = cJSON_GetObjectItem(item, "cargo");
    if (cargo == 0) {
        fprintf(stderr, "ship: missing property 'cargo'\n");
        exit(2);
    } else if (!cJSON_IsArray(cargo)) {
        fprintf(stderr, "ship: property 'cargo' is not an array\n");
        exit(2);
    } else {
        int idx = 0;
        cJSON *elem = 0;
        cJSON_ArrayForEach(cargo, elem) {
            if (!cJSON_IsObject(elem)) {
                fprintf(stderr, "ship: property 'cargo' must contain only objects\n");
                exit(2);
            }
            int code = jsonGetInt(elem, "code");
            int qty = jsonGetInt(elem, "qty");
            if (code < 0 || code >= MAX_ITEMS) {
                fprintf(stderr, "ship: property 'cargo.code' must be in range 0..%d, not %d\n", MAX_ITEMS, code);
                exit(2);
            } else if (qty < 0) {
                fprintf(stderr, "ship: property 'cargo.qty' must be greater than 0, not %d\n", qty);
                exit(2);
            }
            sd->item_quantity[code] = jsonGetInt(elem, "qty");
            idx++;
        }
    }
    return sd;
}

cJSON *shipToJson(ship_data_t *sd) {
    char *objName = "ship";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }

    jsonAddIntToObj(obj, objName, "id", sd->id);
    jsonAddStringToObj(obj, objName, "name", sd->name);
    cJSON *location = cJSON_AddObjectToObject(obj, "location");
    if (location == 0) {
        fprintf(stderr, "%s: unable to allocate property 'location'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(location, "location.x", "x", sd->x);
        jsonAddIntToObj(location, "location.y", "y", sd->y);
        jsonAddIntToObj(location, "location.z", "z", sd->z);
        jsonAddIntToObj(location, "location.orbit", "orbit", sd->pn);
        jsonAddIntToObj(location, "location.status", "status", sd->status);
    }
    cJSON *destination = cJSON_AddObjectToObject(obj, "destination");
    if (destination == 0) {
        fprintf(stderr, "%s: unable to allocate property 'destination'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(destination, "destination.x", "x", sd->dest_x);
        jsonAddIntToObj(destination, "destination.y", "y", sd->dest_y);
        jsonAddIntToObj(destination, "destination.z", "z", sd->dest_z);
    }
    jsonAddIntToObj(obj, objName, "age", sd->age);
    jsonAddBoolToObj(obj, objName, "arrived_via_wormhole", sd->arrived_via_wormhole);
    jsonAddIntToObj(obj, objName, "class", sd->class);
    jsonAddIntToObj(obj, objName, "just_jumped", sd->just_jumped);
    jsonAddIntToObj(obj, objName, "loading_point", sd->loading_point);
    jsonAddIntToObj(obj, objName, "remaining_cost", sd->remaining_cost);
    jsonAddIntToObj(obj, objName, "tonnage", sd->tonnage);
    jsonAddIntToObj(obj, objName, "type", sd->type);
    jsonAddIntToObj(obj, objName, "unloading_point", sd->unloading_point);
    cJSON *cargo = cJSON_AddArrayToObject(obj, "cargo");
    if (cargo == 0) {
        fprintf(stderr, "%s: unable to allocate property 'cargo'\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    } else {
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (sd->item_quantity[j] > 0) {
                cJSON *item = cJSON_CreateObject();
                jsonAddIntToObj(item, "cargo.code", "code", j);
                jsonAddIntToObj(item, "cargo.qty", "qty", sd->item_quantity[j]);
                cJSON_AddItemToArray(cargo, item);
            }
        }
    }
    return obj;
}

ship_data_t **shipsFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "ships: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *array = cJSON_GetObjectItem(root, "ships");
    if (array == 0) {
        fprintf(stderr, "ships: missing property 'ships'\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "ships: property 'ships' is not array\n");
        exit(2);
    }
    int numShips = cJSON_GetArraySize(array);
    // allocate enough memory for all ships plus an empty terminator
    ship_data_t **ships = ncalloc(__FUNCTION__, __LINE__, numShips + 1, sizeof(ship_data_t *));
    // and load them all into the array
    int idx = 0;
    cJSON *item = 0;
    cJSON_ArrayForEach(item, array) {
        ships[idx] = shipFromJson(item);
        idx++;
    }
    return ships;
}

cJSON *shipsToJson(ship_data_t *ships, int numShips) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "ships: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    char *objName = "ships";
    jsonAddIntToObj(root, objName, "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "ships");
    if (array == 0) {
        fprintf(stderr, "ships: unable to allocate property 'ships'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numShips; i++) {
        if (!cJSON_AddItemToArray(array, shipToJson(&ships[i]))) {
            perror("ships: unable to extend array");
            exit(2);
        }
    }
    return root;
}

species_data_t *specieFromJson(cJSON *item) {
    species_data_t *sp = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(species_data_t));
    sp->id = jsonGetInt(item, "id");
    strcpy(sp->name, jsonGetString(item, "name", sizeof(sp->name)));
    cJSON *government = cJSON_GetObjectItem(item, "government");
    if (government == 0) {
        fprintf(stderr, "species: missing property 'government'\n");
        exit(2);
    } else if (!cJSON_IsObject(government)) {
        fprintf(stderr, "species: property 'government' is not an object\n");
        exit(2);
    } else {
        strcpy(sp->govt_name, jsonGetString(government, "name", sizeof(sp->govt_name)));
        strcpy(sp->govt_type, jsonGetString(government, "type", sizeof(sp->govt_type)));
    }
    cJSON *home_world = cJSON_GetObjectItem(item, "home_world");
    if (home_world == 0) {
        fprintf(stderr, "species: missing property 'home_world'\n");
        exit(2);
    } else if (!cJSON_IsObject(home_world)) {
        fprintf(stderr, "species: property 'home_world' is not an object\n");
        exit(2);
    } else {
        sp->x = jsonGetInt(home_world, "x");
        sp->y = jsonGetInt(home_world, "y");
        sp->z = jsonGetInt(home_world, "z");
        sp->pn = jsonGetInt(home_world, "orbit");
        sp->hp_original_base = jsonGetInt(home_world, "hp_base");
    }
    cJSON *atmosphere = cJSON_GetObjectItem(item, "atmosphere");
    if (atmosphere == 0) {
        fprintf(stderr, "species: missing property 'atmosphere'\n");
        exit(2);
    } else if (!cJSON_IsObject(atmosphere)) {
        fprintf(stderr, "species: property 'atmosphere' is not an object\n");
        exit(2);
    } else {
        cJSON *required = cJSON_GetObjectItem(atmosphere, "required");
        if (required == 0) {
            fprintf(stderr, "species: missing property 'atmosphere.required'\n");
            exit(2);
        } else if (!cJSON_IsObject(required)) {
            fprintf(stderr, "species: property 'atmosphere.required' is not an object\n");
            exit(2);
        } else {
            sp->required_gas = jsonGetInt(required, "gas");
            sp->required_gas_min = jsonGetInt(required, "min");
            sp->required_gas_max = jsonGetInt(required, "max");
        }
        cJSON *neutral = cJSON_GetObjectItem(atmosphere, "neutral");
        if (neutral == 0) {
            fprintf(stderr, "species: missing property 'atmosphere.neutral'\n");
            exit(2);
        } else if (!cJSON_IsArray(neutral)) {
            fprintf(stderr, "species: property 'atmosphere.neutral' is not an array\n");
            exit(2);
        } else {
            int length = cJSON_GetArraySize(neutral);
            if (length != 6) {
                fprintf(stderr, "species: property 'atmosphere.neutral' must contain exactly 6 items\n");
                exit(2);
            }
            int idx = 0;
            cJSON *elem = 0;
            cJSON_ArrayForEach(neutral, elem) {
                if (!cJSON_IsNumber(elem)) {
                    fprintf(stderr, "species: property 'atmosphere.neutral' must contain only integers\n");
                    exit(2);
                }
                sp->neutral_gas[idx] = elem->valueint;
                idx++;
            }
        }
        cJSON *poison = cJSON_GetObjectItem(atmosphere, "poison");
        if (poison == 0) {
            fprintf(stderr, "species: missing property 'atmosphere.poison'\n");
            exit(2);
        } else if (!cJSON_IsArray(poison)) {
            fprintf(stderr, "species: property 'atmosphere.poison' is not an array\n");
            exit(2);
        } else {
            int length = cJSON_GetArraySize(poison);
            if (length != 6) {
                fprintf(stderr, "species: property 'atmosphere.poison' must contain exactly 6 items\n");
                exit(2);
            }
            int idx = 0;
            cJSON *elem = 0;
            cJSON_ArrayForEach(poison, elem) {
                if (!cJSON_IsNumber(elem)) {
                    fprintf(stderr, "species: property 'atmosphere.poison' must contain only integers\n");
                    exit(2);
                }
                sp->poison_gas[idx] = elem->valueint;
                idx++;
            }
        }
    }
    cJSON *technology = cJSON_GetObjectItem(item, "technology");
    if (technology == 0) {
        fprintf(stderr, "species: missing property 'technology'\n");
        exit(2);
    } else if (!cJSON_IsObject(technology)) {
        fprintf(stderr, "species: property 'technology' is not an object\n");
        exit(2);
    } else {
        cJSON *mi = cJSON_GetObjectItem(technology, "MI");
        if (mi == 0) {
            fprintf(stderr, "species: missing property 'technology.MI'\n");
            exit(2);
        } else if (!cJSON_IsObject(mi)) {
            fprintf(stderr, "species: property 'technology.MI' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[MA] = jsonGetInt(mi, "level");
            sp->tech_knowledge[MA] = jsonGetInt(mi, "knowledge");
            sp->init_tech_level[MA] = jsonGetInt(mi, "init");
            sp->tech_eps[MA] = jsonGetInt(mi, "xp");
        }
        cJSON *ma = cJSON_GetObjectItem(technology, "MA");
        if (ma == 0) {
            fprintf(stderr, "species: missing property 'technology.MA'\n");
            exit(2);
        } else if (!cJSON_IsObject(ma)) {
            fprintf(stderr, "species: property 'technology.MA' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[MI] = jsonGetInt(ma, "level");
            sp->tech_knowledge[MI] = jsonGetInt(ma, "knowledge");
            sp->init_tech_level[MI] = jsonGetInt(ma, "init");
            sp->tech_eps[MI] = jsonGetInt(ma, "xp");
        }
        cJSON *ml = cJSON_GetObjectItem(technology, "ML");
        if (ml == 0) {
            fprintf(stderr, "species: missing property 'technology.ML'\n");
            exit(2);
        } else if (!cJSON_IsObject(ml)) {
            fprintf(stderr, "species: property 'technology.ML' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[ML] = jsonGetInt(ml, "level");
            sp->tech_knowledge[ML] = jsonGetInt(ml, "knowledge");
            sp->init_tech_level[ML] = jsonGetInt(ml, "init");
            sp->tech_eps[ML] = jsonGetInt(ml, "xp");
        }
        cJSON *gv = cJSON_GetObjectItem(technology, "GV");
        if (gv == 0) {
            fprintf(stderr, "species: missing property 'technology.GV'\n");
            exit(2);
        } else if (!cJSON_IsObject(gv)) {
            fprintf(stderr, "species: property 'technology.GV' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[GV] = jsonGetInt(gv, "level");
            sp->tech_knowledge[GV] = jsonGetInt(gv, "knowledge");
            sp->init_tech_level[GV] = jsonGetInt(gv, "init");
            sp->tech_eps[GV] = jsonGetInt(gv, "xp");
        }
        cJSON *ls = cJSON_GetObjectItem(technology, "LS");
        if (ls == 0) {
            fprintf(stderr, "species: missing property 'technology.LS'\n");
            exit(2);
        } else if (!cJSON_IsObject(ls)) {
            fprintf(stderr, "species: property 'technology.LS' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[LS] = jsonGetInt(ls, "level");
            sp->tech_knowledge[LS] = jsonGetInt(ls, "knowledge");
            sp->init_tech_level[LS] = jsonGetInt(ls, "init");
            sp->tech_eps[LS] = jsonGetInt(ls, "xp");
        }
        cJSON *bi = cJSON_GetObjectItem(technology, "BI");
        if (bi == 0) {
            fprintf(stderr, "species: missing property 'technology.BI'\n");
            exit(2);
        } else if (!cJSON_IsObject(bi)) {
            fprintf(stderr, "species: property 'technology.BI' is not an object\n");
            exit(2);
        } else {
            sp->tech_level[BI] = jsonGetInt(bi, "level");
            sp->tech_knowledge[BI] = jsonGetInt(bi, "knowledge");
            sp->init_tech_level[BI] = jsonGetInt(bi, "init");
            sp->tech_eps[BI] = jsonGetInt(bi, "xp");
        }
    }
    sp->num_namplas = jsonGetInt(item, "num_namplas");
    sp->num_ships = jsonGetInt(item, "num_ships");
    cJSON *fleet_maintenance = cJSON_GetObjectItem(item, "fleet_maintenance");
    if (fleet_maintenance == 0) {
        fprintf(stderr, "species: missing property 'fleet_maintenance'\n");
        exit(2);
    } else if (!cJSON_IsObject(fleet_maintenance)) {
        fprintf(stderr, "species: property 'fleet_maintenance' is not an object\n");
        exit(2);
    } else {
        sp->fleet_cost = jsonGetInt(fleet_maintenance, "cost");
        sp->fleet_percent_cost = jsonGetInt(fleet_maintenance, "percent");
    }
    sp->econ_units = jsonGetInt(item, "banked_eu");
    cJSON *contacts = cJSON_GetObjectItem(item, "contacts");
    if (contacts == 0) {
        // assume no contacts
    } else if (!cJSON_IsArray(contacts)) {
        fprintf(stderr, "species: property 'contacts' is not an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(contacts) > 0) {
        cJSON *no = 0;
        cJSON_ArrayForEach(no, contacts) {
            if (!cJSON_IsNumber(no)) {
                fprintf(stderr, "species: %d: contacts must contain only numbers\n", sp->id);
                exit(2);
            }
            int spNo = no->valueint;
            if (spNo < 1 || spNo > MAX_SPECIES) {
                fprintf(stderr, "species: %d: contacts must contain only numbers in range 1..%d\n", sp->id,
                        MAX_SPECIES);
                exit(2);
            }
            // array index and bit mask
            int species_array_index = (spNo - 1) / 32;
            int species_bit_number = (spNo - 1) % 32;
            long species_bit_mask = 1 << species_bit_number;
            // set the appropriate bit
            sp->contact[species_array_index] |= species_bit_mask;
        }
    }
    cJSON *allies = cJSON_GetObjectItem(item, "allies");
    if (allies == 0) {
        // assume no allies
    } else if (!cJSON_IsArray(contacts)) {
        fprintf(stderr, "species: property 'allies' is not an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(allies) > 0) {
        cJSON *no = 0;
        cJSON_ArrayForEach(no, allies) {
            if (!cJSON_IsNumber(no)) {
                fprintf(stderr, "species: %d: allies must contain only numbers\n", sp->id);
                exit(2);
            }
            int spNo = no->valueint;
            if (spNo < 1 || spNo > MAX_SPECIES) {
                fprintf(stderr, "species: %d: allies must contain only numbers in range 1..%d\n", sp->id, MAX_SPECIES);
                exit(2);
            }
            // array index and bit mask
            int species_array_index = (spNo - 1) / 32;
            int species_bit_number = (spNo - 1) % 32;
            long species_bit_mask = 1 << species_bit_number;
            // set the appropriate bit
            sp->ally[species_array_index] |= species_bit_mask;
        }
    }
    cJSON *enemies = cJSON_GetObjectItem(item, "enemies");
    if (enemies == 0) {
        // assume no enemies
    } else if (!cJSON_IsArray(enemies)) {
        fprintf(stderr, "species: property 'enemies' is not an array\n");
        exit(2);
    } else if (cJSON_GetArraySize(enemies) > 0) {
        cJSON *no = 0;
        cJSON_ArrayForEach(no, enemies) {
            if (!cJSON_IsNumber(no)) {
                fprintf(stderr, "species: %d: enemies must contain only numbers\n", sp->id);
                exit(2);
            }
            int spNo = no->valueint;
            if (spNo < 1 || spNo > MAX_SPECIES) {
                fprintf(stderr, "species: %d: enemies must contain only numbers in range 1..%d\n", sp->id, MAX_SPECIES);
                exit(2);
            }
            // array index and bit mask
            int species_array_index = (spNo - 1) / 32;
            int species_bit_number = (spNo - 1) % 32;
            long species_bit_mask = 1 << species_bit_number;
            // set the appropriate bit
            sp->enemy[species_array_index] |= species_bit_mask;
        }
    }
    return sp;
}

cJSON *specieToJson(species_data_t *sp) {
    char *objName = "species";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "id", sp->id);
    jsonAddStringToObj(obj, objName, "name", sp->name);
    jsonAddIntToObj(obj, objName, "auto", sp->auto_orders);
    cJSON *government = cJSON_AddObjectToObject(obj, "government");
    if (government == 0) {
        fprintf(stderr, "%s: unable to allocate property 'government'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddStringToObj(government, "government", "name", sp->govt_name);
        jsonAddStringToObj(government, "government", "type", sp->govt_type);
    }
    cJSON *home_world = cJSON_AddObjectToObject(obj, "home_world");
    if (home_world == 0) {
        fprintf(stderr, "%s: unable to allocate property 'home_world'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        jsonAddIntToObj(home_world, "home_world", "x", sp->x);
        jsonAddIntToObj(home_world, "home_world", "y", sp->y);
        jsonAddIntToObj(home_world, "home_world", "z", sp->z);
        jsonAddIntToObj(home_world, "home_world", "orbit", sp->pn);
        jsonAddIntToObj(home_world, "home_world", "hp_base", sp->hp_original_base);
    }
    cJSON *atmosphere = cJSON_AddObjectToObject(obj, "atmosphere");
    if (atmosphere == 0) {
        fprintf(stderr, "%s: unable to allocate property 'atmosphere'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    } else {
        cJSON *required = cJSON_AddObjectToObject(atmosphere, "required");
        if (required == 0) {
            fprintf(stderr, "%s: unable to allocate property 'atmosphere.required'\n", objName);
            perror("cJSON_AddObjectToObject");
            exit(2);
        } else {
            jsonAddIntToObj(required, "atmosphere.required", "gas", sp->required_gas);
            jsonAddIntToObj(required, "atmosphere.required", "min", sp->required_gas_min);
            jsonAddIntToObj(required, "atmosphere.required", "max", sp->required_gas_max);
        }
        cJSON *neutral = cJSON_AddArrayToObject(atmosphere, "neutral");
        if (neutral == 0) {
            fprintf(stderr, "%s: unable to allocate property 'atmosphere.neutral'\n", objName);
            perror("cJSON_AddArrayToObject");
            exit(2);
        } else {
            for (int j = 0; j < 6; j++) {
                jsonAddIntToArray(neutral, "atmosphere.neutral", sp->neutral_gas[j]);
            }
        }
        cJSON *poison = cJSON_AddArrayToObject(atmosphere, "poison");
        if (poison == 0) {
            fprintf(stderr, "%s: unable to allocate property 'atmosphere.poison'\n", objName);
            perror("cJSON_AddArrayToObject");
            exit(2);
        } else {
            for (int j = 0; j < 6; j++) {
                jsonAddIntToArray(poison, "atmosphere.poison", sp->poison_gas[j]);
            }
        }
    }
    cJSON *technology = cJSON_AddObjectToObject(obj, "technology");
    if (technology == 0) {
        fprintf(stderr, "%s: unable to allocate property 'technology'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    }
    for (int j = 0; j < 6; j++) {
        cJSON *named = cJSON_AddObjectToObject(technology, tech_level_names[j]);
        if (named == 0) {
            fprintf(stderr, "%s: unable to allocate property 'technology.%s'\n", objName, tech_level_names[j]);
            perror("cJSON_AddObjectToObject");
            exit(2);
        }
        jsonAddIntToObj(named, "technology", "level", sp->tech_level[j]);
        jsonAddIntToObj(named, "technology", "knowledge", sp->tech_knowledge[j]);
        jsonAddIntToObj(named, "technology", "init", sp->init_tech_level[j]);
        jsonAddIntToObj(named, "technology", "xp", sp->tech_eps[j]);
    }
    jsonAddIntToObj(obj, objName, "num_namplas", sp->num_namplas);
    jsonAddIntToObj(obj, objName, "num_ships", sp->num_ships);
    cJSON *fleet_maintenance = cJSON_AddObjectToObject(obj, "fleet_maintenance");
    if (fleet_maintenance == 0) {
        fprintf(stderr, "%s: unable to allocate property 'fleet_maintenance'\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    }
    jsonAddIntToObj(fleet_maintenance, "fleet_maintenance", "cost", sp->fleet_cost);
    jsonAddIntToObj(fleet_maintenance, "fleet_maintenance", "percent", sp->fleet_percent_cost);
    jsonAddIntToObj(obj, objName, "banked_eu", sp->econ_units);
    cJSON *contacts = cJSON_AddArrayToObject(obj, "contacts");
    if (contacts == 0) {
        fprintf(stderr, "%s: unable to allocate property 'contacts'\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->contact[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(contacts, "contacts", spidx + 1);
        }
    }
    cJSON *allies = cJSON_AddArrayToObject(obj, "allies");
    if (allies == 0) {
        fprintf(stderr, "%s: unable to allocate property 'allies'\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->ally[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(allies, "allies", spidx + 1);
        }
    }
    cJSON *enemies = cJSON_AddArrayToObject(obj, "enemies");
    if (enemies == 0) {
        fprintf(stderr, "%s: unable to allocate property 'enemies'\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->enemy[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(enemies, "enemies", spidx + 1);
        }
    }
    return obj;
}

species_data_t *speciesFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "species: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *obj = cJSON_GetObjectItem(root, "species");
    if (obj == 0) {
        fprintf(stderr, "species: missing property 'species'\n");
        exit(2);
    } else if (!cJSON_IsObject(obj)) {
        fprintf(stderr, "species: property 'species' is not an object\n");
        exit(2);
    }
    return specieFromJson(obj);
}

cJSON *speciesToJson(species_data_t *sp) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "species: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(root, "species", "version", 1);
    cJSON_AddItemToObject(root, "species", specieToJson(sp));
    return root;
}

star_data_t *starFromJson(cJSON *item) {
    star_data_t *star = (star_data_t *) ncalloc(__FUNCTION__, __LINE__, 1, sizeof(star_data_t));
    star->id = jsonGetInt(item, "id");
    star->x = jsonGetInt(item, "x");
    star->y = jsonGetInt(item, "y");
    star->z = jsonGetInt(item, "z");
    star->type = chToStarType(jsonGetString(item, "type", 2)[0]);
    star->color = chToStarColor(jsonGetString(item, "color", 2)[0]);
    star->size = jsonGetInt(item, "size");
    star->planet_index = jsonGetInt(item, "planet_index");
    cJSON *planets = cJSON_GetObjectItem(item, "planets");
    if (planets == 0) {
        fprintf(stderr, "stars: %d: missing property 'planets'\n", star->id);
        exit(2);
    } else if (!cJSON_IsArray(planets)) {
        fprintf(stderr, "stars: %d: planets is not an array\n", star->id);
        exit(2);
    } else {
        star->num_planets = cJSON_GetArraySize(planets);
        if (star->num_planets < 1) {
            fprintf(stderr, "star: %d: planets must not be empty\n", star->id);
            exit(2);
        } else if (star->num_planets > 9) {
            fprintf(stderr, "star: %d: planets must contain 1..9 entries\n", star->id);
            exit(2);
        } else {
            int orbit = 0;
            cJSON *planet = 0;
            cJSON_ArrayForEach(planet, planets) {
                if (!cJSON_IsNumber(planet)) {
                    fprintf(stderr, "star: %d: planets must contain only numbers\n", star->id);
                    exit(2);
                }
                planet_data_t *pd = ncalloc(__FUNCTION__, __LINE__, 1, sizeof(planet_data_t));
                pd->id = planet->valueint;
                pd->index = pd->id - 1;
                if (pd->id<1 || pd->id>MAX_PLANETS) {
                    fprintf(stderr, "star: %d: planets: planet.id must be in range 1..%d\n", star->id, MAX_PLANETS);
                    exit(2);
                }
                star->planets[orbit] = pd;
                orbit++;
            }
        }
    }
    star->home_system = jsonGetBool(item, "home_system");
    cJSON *visited_by = cJSON_GetObjectItem(item, "visited_by");
    if (visited_by == 0) {
        // assume no visitors
    } else if (!cJSON_IsArray(visited_by)) {
        fprintf(stderr, "star: %d: visited_by is not an array\n", star->id);
        exit(2);
    } else if (cJSON_GetArraySize(visited_by) > 0) {
        cJSON *no = 0;
        cJSON_ArrayForEach(no, visited_by) {
            if (!cJSON_IsNumber(no)) {
                fprintf(stderr, "star: %d: visited_by must contain only numbers\n", star->id);
                exit(2);
            }
            int spNo = no->valueint;
            if (spNo < 1 || spNo > MAX_SPECIES) {
                fprintf(stderr, "star: %d: visited_by must contain only numbers in range 1..%d\n",
                        star->id, MAX_SPECIES);
                exit(2);
            }
            // array index and bit mask
            int species_array_index = (spNo - 1) / 32;
            int species_bit_number = (spNo - 1) % 32;
            long species_bit_mask = 1 << species_bit_number;
            // set the appropriate bit
            star->visited_by[species_array_index] |= species_bit_mask;
        }
    }
    cJSON *worm_hole = cJSON_GetObjectItem(item, "worm_hole");
    if (worm_hole == 0) {
        // assume no wormhole here
    } else if (!cJSON_IsObject(worm_hole)) {
        fprintf(stderr, "star: %d: worm_hole is not an object\n", star->id);
        exit(2);
    } else {
        star->worm_here = jsonGetBool(worm_hole, "here");
        star->worm_x = jsonGetInt(worm_hole, "x");
        star->worm_y = jsonGetInt(worm_hole, "y");
        star->worm_z = jsonGetInt(worm_hole, "z");
    }
    star->message = jsonGetInt(item, "message");
    return star;
}


cJSON *starToJson(star_data_t *sd, int id) {
    char *objName = "star";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "id", id);
    jsonAddIntToObj(obj, objName, "x", sd->x);
    jsonAddIntToObj(obj, objName, "y", sd->y);
    jsonAddIntToObj(obj, objName, "z", sd->z);
    char code[2];
    code[1] = 0;
    code[0] = star_type(sd->type);
    jsonAddStringToObj(obj, objName, "type", code);
    code[0] = star_color(sd->color);
    jsonAddStringToObj(obj, objName, "color", code);
    jsonAddIntToObj(obj, objName, "size", sd->size);
    jsonAddIntToObj(obj, objName, "planet_index", sd->planet_index);
    cJSON *planets = cJSON_AddArrayToObject(obj, "planets");
    if (planets == 0) {
        fprintf(stderr, "%s: unable to allocate planets property\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int p = 0; p < sd->num_planets; p++) {
        cJSON_AddItemToArray(planets, cJSON_CreateNumber((double) (sd->planet_index + p + 1)));
    }
    jsonAddBoolToObj(obj, objName, "home_system", sd->home_system);
    cJSON *worm_hole = cJSON_AddObjectToObject(obj, "worm_hole");
    if (worm_hole == NULL) {
        fprintf(stderr, "%s: unable to allocate worm_hole property\n", objName);
        perror("cJSON_AddObjectToObject");
        exit(2);
    }
    jsonAddBoolToObj(worm_hole, "worm_hole", "here", sd->worm_here);
    jsonAddIntToObj(worm_hole, "worm_hole", "x", sd->worm_x);
    jsonAddIntToObj(worm_hole, "worm_hole", "y", sd->worm_x);
    jsonAddIntToObj(worm_hole, "worm_hole", "z", sd->worm_z);
    cJSON *visited_by = cJSON_AddArrayToObject(obj, "visited_by");
    if (visited_by == 0) {
        fprintf(stderr, "%s: unable to allocate visited_by property\n", objName);
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        // write the species only if it has visited this system
        if ((sd->visited_by[spidx / 32] & (1 << (spidx % 32))) != 0) {
            jsonAddIntToArray(visited_by, "visited_by", spidx + 1);
        }
    }
    jsonAddIntToObj(obj, objName, "message", sd->message);
    return obj;
}

star_data_t **starsDataFromJson(cJSON *root) {
    int version = jsonGetInt(root, "version");
    if (version != 1) {
        fprintf(stderr, "stars.json: version: expected %d, found %d\n", 1, version);
        exit(2);
    }
    cJSON *array = cJSON_GetObjectItem(root, "stars");
    if (array == 0) {
        fprintf(stderr, "stars.json: stars: missing property\n");
        exit(2);
    } else if (!cJSON_IsArray(array)) {
        fprintf(stderr, "stars.json: stars: property is not array\n");
        exit(2);
    }
    int numStars = cJSON_GetArraySize(array);
    if (numStars < MIN_STARS) {
        fprintf(stderr, "stars.json: stars: expected at least %d stars, got %d\n", MIN_STARS, numStars);
        exit(2);
    } else if (numStars > MAX_STARS) {
        fprintf(stderr, "stars.json: stars: expected at most %d stars, got %d\n", MAX_STARS, numStars);
        exit(2);
    }
    // allocate enough memory for all stars plus an empty terminator
    star_data_t **stars = (star_data_t **) ncalloc(__FUNCTION__, __LINE__, numStars + 1, sizeof(star_data_t *));
    // and load all the stars
    int idx = 0;
    cJSON *item = 0;
    cJSON_ArrayForEach(item, array) {
        star_data_t *star = starFromJson(item);
        if (star->id != idx + 1) {
            fprintf(stderr, "stars.json: stars: index %d: id: expect %d, got %d\n", idx, idx + 1, star->id);
            exit(2);
        } else if (star->size < 0 || star->size > 9) {
            fprintf(stderr, "stars.json: stars: star[%d]: size: expect 0..9, got %d\n", star->id, star->size);
            exit(2);
        }
        stars[idx] = star;
        idx = idx + 1;
    }

    // sanity checks on the data

    // should have read in the expected number of stars
    if (idx != numStars) {
        fprintf(stderr, "stars.json: stars: read %d of %d\n", idx, numStars);
        exit(2);
    }

    int foundErrors = 0;

    // verify that planets are in a single system.
    // create a map of planet id to star id to check for planets being in multiple star systems.
    int planetsByIdToStarId[MAX_PLANETS + 1];
    for (int i = 0; i < MAX_PLANETS + 1; i++) {
        planetsByIdToStarId[i] = 0;
    }
    // now use that map to check.
    for (int idx = 0; stars[idx] != 0; idx++) {
        star_data_t *sd = stars[idx];
        for (int pn = 0; stars[idx]->planets[pn] != 0; pn++) {
            planet_data_t *pd = stars[idx]->planets[pn];
            if (planetsByIdToStarId[pd->id] != 0) {
                fprintf(stderr, "stars.json: planet %5d in systems %5d and %5d\n", pd->id, sd->id,
                        planetsByIdToStarId[pd->id]);
                foundErrors++;
            }
            planetsByIdToStarId[stars[idx]->planets[pn]->index] = sd->id;
        }
    }

    if (foundErrors) {
        exit(2);
    }
    return stars;
}

// starsDataToJson translates the current star_base array to JSON
cJSON *starsDataToJson(star_data_t *starBase, int numStars) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "stars.json: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    char *objName = "stars";
    jsonAddIntToObj(root, objName, "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "stars");
    if (array == 0) {
        fprintf(stderr, "stars.json: unable to allocate property 'stars'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numStars; i++) {
        int id = i + 1;
        if (!cJSON_AddItemToArray(array, starToJson(&starBase[i], id))) {
            perror("stars.json: unable to extend array");
            exit(2);
        }
    }
    return root;
}


cJSON *transactionToJson(trans_data_t *td) {
    char *objName = "transaction";
    cJSON *obj = cJSON_CreateObject();
    if (obj == 0) {
        fprintf(stderr, "%s: unable to allocate object\n", objName);
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(obj, objName, "type", td->type);
    jsonAddIntToObj(obj, objName, "donor", td->donor);
    jsonAddIntToObj(obj, objName, "recipient", td->recipient);
    jsonAddIntToObj(obj, objName, "value", td->value);
    jsonAddIntToObj(obj, objName, "x", td->x);
    jsonAddIntToObj(obj, objName, "y", td->y);
    jsonAddIntToObj(obj, objName, "z", td->z);
    jsonAddIntToObj(obj, objName, "orbit", td->pn);
    cJSON *args = cJSON_AddArrayToObject(obj, "args");
    for (int arg = 1; arg <= 3; arg++) {
        int number;
        char *name;
        switch (arg) {
            case 1:
                number = td->number1;
                name = td->name1;
                break;
            case 2:
                number = td->number2;
                name = td->name2;
                break;
            case 3:
                number = td->number3;
                name = td->name3;
                break;
            default:
                fprintf(stderr, "assert(0 < arg < 3)\n");
                exit(2);
        }
        cJSON *item = cJSON_CreateObject();
        jsonAddIntToObj(item, "args", "number", number);
        jsonAddStringToObj(item, "args", "name", name);
        cJSON_AddItemToArray(args, item);
    }
    return obj;
}

cJSON *transactionsToJson(trans_data_t *transData, int numTransactions) {
    cJSON *root = cJSON_CreateObject();
    if (root == 0) {
        fprintf(stderr, "transactions: unable to allocate root\n");
        perror("cJSON_CreateObject");
        exit(2);
    }
    jsonAddIntToObj(root, "transactions", "version", 1);
    cJSON *array = cJSON_AddArrayToObject(root, "transactions");
    if (array == 0) {
        fprintf(stderr, "transactions: unable to allocate property 'stars'\n");
        perror("cJSON_AddArrayToObject");
        exit(2);
    }
    for (int i = 0; i < numTransactions; i++) {
        if (!cJSON_AddItemToArray(array, transactionToJson(&transData[i]))) {
            perror("transactions: unable to extend array");
            exit(2);
        }
    }
    return root;
}

