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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cfgfile.h"

typedef struct {
    char *key;
    char *value;
} key_value_t;


// cfgReadLine reads one line from the input file and returns
// the key/value pair on the line.
// the key will have leading and trailing spaces removed.
// if there is no key on the line, the returned key will be an empty string.
// the value will have leading and trailing spaces removed.
// all interior runs of spaces will be condensed into a single space.
// if there is no value on the line, the returned value will be NULL.
// if there is a comment character ('#'), the remainder of the line is consumed.
// if the key and value are longer than the internal buffer,
// the remaining extra characters in the line are consumed but not returned.
// returns a key of NULL on end of input.
key_value_t cfgReadLine(FILE *fp) {
    static char buffer[128];
    static char *eob = buffer + sizeof(buffer) - 2; // should point to the last byte of buffer

    key_value_t kv;
    kv.key = NULL;
    kv.value = NULL;

    if (fp == NULL || feof(fp)) {
        return kv;
    }
    kv.key = buffer;

    char *p = buffer;
    int ch = fgetc(fp);
    for (; ch != '#' && ch != '\n' && ch != EOF; ch = fgetc(fp)) {
        // convert unwanted characters into spaces
        if (!isascii(ch) || ch == '\t' || ch == '\r' || isspace(ch) || iscntrl(ch) || !isgraph(ch)) {
            ch = ' ';
        } else if (ch == '"' || ch == '\\' || ch == '&' || ch == '<' || ch == '>' || ch == '!' || ch == ',') {
            ch = ' ';
        }
        // ignore leading spaces and condense runs of spaces
        if (ch == ' ' && (p == buffer || *(p - 1) == ' ')) {
            continue;
        }
        // don't add past the end of the buffer
        if (p != eob) {
            *p++ = ch;
        }
    }
    // consume the rest of the input line
    for (; ch != '\n' && ch != EOF;) {
        ch = fgetc(fp);
    }
    // trim trailing spaces
    for (; p != buffer && *(p - 1) == ' ';) {
        p--;
    }
    // terminate the buffer
    *p = 0;
    // split buffer into key and value based on the first space found in the buffer.
    // leave value set to NULL if there are no spaces.
    for (p = buffer; *p; p++) {
        if (*p == ' ') {
            *p = 0;
            kv.value = p + 1;
            break;
        }
    }
    return kv;
}


species_cfg_t *CfgSpeciesFree(species_cfg_t *c) {
    for (; c != NULL;) {
        species_cfg_t *tmp = c;
        c = c->next;
        if (tmp->email != NULL) {
            free(tmp->email);
        }
        if (tmp->govtname != NULL) {
            free(tmp->govtname);
        }
        if (tmp->govttype != NULL) {
            free(tmp->govttype);
        }
        if (tmp->homeworld != NULL) {
            free(tmp->homeworld);
        }
        if (tmp->name != NULL) {
            free(tmp->name);
        }
        free(tmp);
    }
    return c;
}


species_cfg_t *CfgSpeciesFromFile(const char *name) {
    FILE *fp = fopen(name, "rb");
    if (fp == NULL) {
        perror("CfgSpeciesFromFile:");
        exit(2);
    }
    int line = 0;
    species_cfg_t *root = NULL;
    species_cfg_t *curr = NULL;
    for (;;) {
        key_value_t kv = cfgReadLine(fp);
        if (kv.key == NULL) {
            break;
        }
        line++;
        if (strlen(kv.key) == 0) {
            continue;
        } else if (strcmp(kv.key, "species") == 0) {
            // append a new section to the list
            if (root == NULL) {
                curr = calloc(1, sizeof(species_cfg_t));
                root = curr;
            } else {
                curr->next = calloc(1, sizeof(species_cfg_t));
                curr = curr->next;
            }
            if (curr == NULL) {
                perror("CfgSpeciesFromFile:");
                exit(2);
            }
            curr->next = NULL;
            // fprintf(stderr, "CfgSpeciesFromFile: key '%s' val '%s'\n", kv.key, kv.value ? kv.value : "null");
        } else if (curr == NULL) {
            fprintf(stderr, "error: %d: key %s: found key outside of section\n", line, kv.key);
            exit(2);
        } else if (strcmp(kv.key, "bi") == 0) {
            int val = 0;
            if (kv.value != NULL) {
                val = atoi(kv.value);
                if (val < 1 || 15 < val) {
                    fprintf(stderr, "error: %d: key %s: value must be between 1 and 15\n", line, kv.key, kv.value);
                    exit(2);
                }
            }
            curr->bi = val;
        } else if (strcmp(kv.key, "email") == 0) {
            if (curr->email != NULL) {
                free(curr->email);
                curr->email = NULL;
            }
            if (kv.value != NULL) {
                curr->email = strdup(kv.value);
                if (curr->email == NULL) {
                    perror("CfgSpeciesFromFile:");
                    exit(2);
                }
            }
        } else if (strcmp(kv.key, "govtname") == 0) {
            if (curr->govtname != NULL) {
                free(curr->govtname);
                curr->govtname = NULL;
            }
            if (kv.value != NULL) {
                curr->govtname = strdup(kv.value);
                if (curr->govtname == NULL) {
                    perror("CfgSpeciesFromFile:");
                    exit(2);
                }
            }
        } else if (strcmp(kv.key, "govttype") == 0) {
            if (curr->govttype != NULL) {
                free(curr->govttype);
                curr->govttype = NULL;
            }
            if (kv.value != NULL) {
                curr->govttype = strdup(kv.value);
                if (curr->govttype == NULL) {
                    perror("CfgSpeciesFromFile:");
                    exit(2);
                }
            }
        } else if (strcmp(kv.key, "gv") == 0) {
            int val = 0;
            if (kv.value != NULL) {
                val = atoi(kv.value);
                if (val < 1 || 15 < val) {
                    fprintf(stderr, "error: %d: key %s: value must be between 1 and 15\n", line, kv.key, kv.value);
                    exit(2);
                }
            }
            curr->gv = val;
        } else if (strcmp(kv.key, "homeworld") == 0) {
            if (curr->homeworld != NULL) {
                free(curr->homeworld);
                curr->homeworld = NULL;
            }
            if (kv.value != NULL) {
                curr->homeworld = strdup(kv.value);
                if (curr->homeworld == NULL) {
                    perror("CfgSpeciesFromFile:");
                    exit(2);
                }
            }
        } else if (strcmp(kv.key, "ls") == 0) {
            int val = 0;
            if (kv.value != NULL) {
                val = atoi(kv.value);
                if (val < 1 || 15 < val) {
                    fprintf(stderr, "error: %d: key %s: value must be between 1 and 15\n", line, kv.key, kv.value);
                    exit(2);
                }
            }
            curr->ls = val;
        } else if (strcmp(kv.key, "ml") == 0) {
            int val = 0;
            if (kv.value != NULL) {
                val = atoi(kv.value);
                if (val < 1 || 15 < val) {
                    fprintf(stderr, "error: %d: key %s: value must be between 1 and 15\n", line, kv.key, kv.value);
                    exit(2);
                }
            }
            curr->ml = val;
        } else if (strcmp(kv.key, "name") == 0) {
            // fprintf(stderr, "CfgSpeciesFromFile: key '%s' val '%s'\n", kv.key, kv.value ? kv.value : "null");
            if (curr->name != NULL) {
                free(curr->name);
                curr->name = NULL;
            }
            if (kv.value != NULL) {
                curr->name = strdup(kv.value);
                if (curr->name == NULL) {
                    perror("CfgSpeciesFromFile:");
                    exit(2);
                }
            }
        } else {
            fprintf(stderr, "error: %d: key %s: unknown key\n", line, kv.key);
            exit(2);
        }
    }
    printf(" info: read %d lines from '%s'\n", line, name);

    return root;
}
