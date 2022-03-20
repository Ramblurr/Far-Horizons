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
#include "json.h"


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

static void json__print(int indent, const char *name, json_value_t *value, FILE *fp);


json_value_t *json_read(FILE *fp) {
    json_parser_t p;
    memset(&p, 0, sizeof(p));
    p.fp = fp;
    p.line = 1;

    jp__nextsym(&p);
    json_value_t *value = jp__value(&p);
    jp__expect(&p, eof);

    return value;
}


int json_write(json_value_t *value, FILE *fp) {
    json__print(0, "", value, fp);
    fprintf(fp, "\n");
    return 0;
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

    json_node_t *t = calloc(1, sizeof(json_node_t));
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

    json_node_t *t = calloc(1, sizeof(json_node_t));
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
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_boolean:");
        exit(2);
    }
    j->flag = JSON_TYPE_BOOL;
    j->u.b = v ? 1 : 0;
    return j;
}


json_value_t *json_error(const char *fmt, ...) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
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


json_value_t *json_list(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_list:");
        exit(2);
    }
    j->flag = JSON_TYPE_LIST;
    return j;
}


json_value_t *json_map(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_map:");
        exit(2);
    }
    j->flag = JSON_TYPE_MAP;
    return j;
}


json_value_t *json_null(void) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_null:");
        exit(2);
    }
    j->flag = JSON_TYPE_NULL;
    return j;
}


json_value_t *json_number(int v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
    if (j == NULL) {
        perror("json_number:");
        exit(2);
    }
    j->flag = JSON_TYPE_NUMBER;
    j->u.n = v;
    return j;
}


json_value_t *json_string(char *v) {
    json_value_t *j = calloc(1, sizeof(json_value_t));
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
    json_value_t *j = calloc(1, sizeof(json_value_t));
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


void json__print(int indent, const char *name, json_value_t *value, FILE *fp) {
    for (int i = indent; i > 0; i--) {
        fprintf(fp, "\t");
    }
    if (name != NULL && *name) {
        fprintf(fp, "\"%s\":", name);
    }
    if (value == NULL) {
        fprintf(fp, "undefined");
    } else {
        switch (value->flag) {
            case JSON_TYPE_UNDEFINED:
                fprintf(fp, "undefined");
                break;
            case JSON_TYPE_BOOL:
                if (value->u.b) {
                    fprintf(fp, "true");
                } else {
                    fprintf(fp, "false");
                }
                break;
            case JSON_TYPE_ERROR:
                fprintf(fp, "error(%s)", value->u.s);
                break;
            case JSON_TYPE_LIST:
                if (value->u.a.root == NULL) {
                    fprintf(fp, "[]");
                } else if (value->u.a.root->next == NULL && json_is_atom(value->u.a.root->value)) {
                    fprintf(fp, "[");
                    json__print(0, value->u.a.root->key, value->u.a.root->value, fp);
                    fprintf(fp, "]");
                } else {
                    fprintf(fp, "[\n");
                    for (json_node_t *n = value->u.a.root; n != NULL; n = n->next) {
                        json__print(indent + 1, n->key, n->value, fp);
                        if (n->next != NULL) {
                            fprintf(fp, ",\n");
                        } else {
                            fprintf(fp, "\n");
                        }
                    }
                    for (int i = indent; i > 0; i--) {
                        fprintf(fp, "\t");
                    }
                    fprintf(fp, "]");
                }
                break;
            case JSON_TYPE_MAP:
                if (value->u.a.root == NULL) {
                    fprintf(fp, "{}");
                } else if (value->u.a.root->next == NULL && json_is_atom(value->u.a.root->value)) {
                    fprintf(fp, "{");
                    json__print(0, value->u.a.root->key, value->u.a.root->value, fp);
                    fprintf(fp, "}");
                } else {
                    fprintf(fp, "{\n");
                    for (json_node_t *n = value->u.a.root; n != NULL; n = n->next) {
                        json__print(indent + 1, n->key, n->value, fp);
                        if (n->next != NULL) {
                            fprintf(fp, ",\n");
                        } else {
                            fprintf(fp, "\n");
                        }
                    }
                    for (int i = indent; i > 0; i--) {
                        fprintf(fp, "\t");
                    }
                    fprintf(fp, "}");
                }
                break;
            case JSON_TYPE_NUMBER:
                fprintf(fp, "%d", value->u.n);
                break;
            case JSON_TYPE_NULL:
                fprintf(fp, "null");
                break;
            case JSON_TYPE_STRING:
                fprintf(fp, "\"%s\"", value->u.s);
                break;
            default:
                fprintf(fp, "undefined");
                break;
        }
    }
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