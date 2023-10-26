//
// Created by Michael Henderson on 10/25/23.
//

#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

cJSON *jsonParseFile(const char *name) {
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
