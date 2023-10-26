//
// Created by Michael Henderson on 10/25/23.
//

#ifndef FAR_HORIZONS_HELPERS_H
#define FAR_HORIZONS_HELPERS_H

#include "cJSON.h"

void jsonAddIntToArray(cJSON *array, const char *arrayName, int value);

void jsonAddBoolToObj(cJSON *obj, const char *objName, const char *propName, int value);

void jsonAddIntToObj(cJSON *obj, const char *objName, const char *propName, int value);

void jsonAddItemToArray(cJSON *array, const char *objName, cJSON *value);

void jsonAddItemToObj(cJSON *obj, const char *objName, const char *propName, cJSON *value);

void jsonAddStringToObj(cJSON *obj, const char *objName, const char *propName, const char *value);

int jsonGetBool(cJSON *obj, const char *property);

int jsonGetInt(cJSON *obj, const char *property);

const char *jsonGetString(cJSON *obj, const char *property, int maxLength);

cJSON *jsonParseFile(const char *name);

void jsonWriteFile(cJSON *root, const char *kind, const char *name);

#endif //FAR_HORIZONS_HELPERS_H
