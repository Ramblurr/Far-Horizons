//
// Created by Michael Henderson on 11/14/23.
//

#include "memsafe.h"

// zstrcpy safely copies a c-string from src to dst, taking care
// to avoid overflow and to always terminate the destination.
// The "size" parameter is the size (in bytes) of the destination.
// if the source is shorter than the destination, the remainder
// of the destination will be filled with zeroes.
int zstrcpy(char *dst, const char *src, size_t size) {
    int s = 0;  // index into source buffer
    int d = 0; // index into destination buffer
    if (size > 0) {
        for (; d < size; d++) {
            dst[d] = src[s];
            if (src[s] != 0) {
                s++;
            }
        }
        dst[size-1] = 0;
    }
    return s;
}