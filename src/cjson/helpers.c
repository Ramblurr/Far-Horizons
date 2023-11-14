//
// Created by Michael Henderson on 10/25/23.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"


void jsonAddIntToArray(cJSON *array, const char *arrayName, int value) {
    cJSON *item = cJSON_CreateNumber((double) value);
    if (item == 0) {
        perror("cJSON_CreateNumber:");
        fprintf(stderr, "%s: unable to integer for array\n", arrayName);
        exit(2);
    } else if (cJSON_AddItemToArray(array, item) == 0) {
        perror("cJSON_AddItemToArray:");
        fprintf(stderr, "%s: unable to add integer to array\n", arrayName);
        exit(2);
    }
}

void jsonAddBoolToObj(cJSON *obj, const char *objName, const char *propName, int value) {
    if (cJSON_AddBoolToObject(obj, propName, value) == 0) {
        perror("cJSON_AddBoolToObject:");
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        exit(2);
    }
}

void jsonAddIntToObj(cJSON *obj, const char *objName, const char *propName, int value) {
    if (cJSON_AddNumberToObject(obj, propName, (double) (value)) == 0) {
        perror("cJSON_AddNumberToObject:");
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        exit(2);
    }
}

void jsonAddItemToArray(cJSON *array, const char *objName, cJSON *value) {
    if (cJSON_AddItemToArray(array, value) == 0) {
        perror("cJSON_AddItemToArray:");
        fprintf(stderr, "%s: unable to extend array\n", objName);
        exit(2);
    }
}

void jsonAddItemToObj(cJSON *obj, const char *objName, const char *propName, cJSON *value) {
    if (cJSON_AddItemToObject(obj, propName, value) == 0) {
        perror("cJSON_AddItemToObject:");
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        exit(2);
    }
}

void jsonAddStringToObj(cJSON *obj, const char *objName, const char *propName, const char *value) {
    if (cJSON_AddStringToObject(obj, propName, value) == 0) {
        perror("cJSON_AddStringToObject:");
        fprintf(stderr, "%s: unable to add property '%s'\n", objName, propName);
        exit(2);
    }
}

int jsonGetBool(cJSON *obj, const char *property) {
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

int jsonGetInt(cJSON *obj, const char *property) {
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
void jsonGetString(cJSON *obj, const char *property, char *dst, int size) {
    memset(dst, 0, size);
    cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, property);
    if (item == 0) {
        fprintf(stderr, "property: %s: must not be null\n", property);
        exit(2);
    } else if (!cJSON_IsString(item)) {
        fprintf(stderr, "property: %s: not a string\n", property);
        exit(2);
    } else if (strlen(item->valuestring)+1 > size) {
        fprintf(stderr, "jsonGetString: strlen %d exceeds limit %d\n", (int) strlen(item->valuestring) + 1, size);
        exit(2);
    }
    strncpy(dst, item->valuestring, size);
}


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
    char *buffer = calloc(1, length + 1);
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


void jsonWriteFile(cJSON *root, const char *kind, const char *name) {
    // convert json to text
    char *string = cJSON_Print(root);
    if (string == 0) {
        fprintf(stderr, "error: %s: json print failed\n", kind);
        exit(2);
    }
    // save it to the file and close it
    FILE *fp = fopen(name, "wb");
    if (fp == NULL) {
        perror("fh: export: json:");
        fprintf(stderr, "error: %s: can not create file!\n", name);
        exit(2);
    }
    fprintf(fp, "%s\n", string);
    fclose(fp);
    // release the memory
    free(string);
}

