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
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include "engine.h"
#include "locationio.h"
#include "location.h"


struct sp_loc_data loc[MAX_LOCATIONS];
int num_locs;


typedef struct {
    uint8_t s;    /* Species number */
    uint8_t x;
    uint8_t y;
    uint8_t z;
} binary_data_t;


void get_location_data(void) {
    /* Get size of file. */
    struct stat sb;
    if (stat("locations.dat", &sb) != 0) {
        num_locs = 0;
        return;
    }

    // get number of records in the file
    num_locs = sb.st_size / sizeof(binary_data_t);
    if (sb.st_size != num_locs * sizeof(binary_data_t)) {
        fprintf(stderr, "\nFile locations.dat contains extra bytes (%ld > %ld)!\n\n",
                sb.st_size, num_locs * sizeof(binary_data_t));
        exit(-1);
    } else if (num_locs == 0) {
        // nothing to do
        return;
    } else if (num_locs > MAX_LOCATIONS) {
        fprintf(stderr, "\nFile locations.dat contains too many records (%d > %d)!\n\n", num_locs, MAX_LOCATIONS);
        exit(-1);
    }

    /* Allocate enough memory for all records. */
    binary_data_t *binData = (binary_data_t *) ncalloc(__FUNCTION__, __LINE__, num_locs, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("get_location_data");
        fprintf(stderr, "\nCannot allocate enough memory for location data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d location entries\n\n", num_locs);
        exit(-1);
    }

    /* Open locations file. */
    FILE *fp = fopen("locations.dat", "rb");
    if (fp == NULL) {
        perror("get_location_data");
        fprintf(stderr, "\nCannot open file 'locations.dat' for reading!\n\n");
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(binData, sizeof(binary_data_t), num_locs, fp) != num_locs) {
        fprintf(stderr, "\nCannot read file 'locations.dat' into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d location entries\n\n", num_locs);
        exit(-1);
    }

    /* translate data */
    for (int i = 0; i < num_locs; i++) {
        loc[i].x = binData[i].x;
        loc[i].y = binData[i].y;
        loc[i].z = binData[i].z;
        loc[i].s = binData[i].s;
    }

    fclose(fp);

    free(binData);
}


// locationDataAsJson writes the current location data to a text file as JSON.
void locationDataAsJson(FILE *fp) {
    const char *sep = "";
    fprintf(fp, "[");
    for (int i = 0; i < num_locs; i++) {
        sp_loc_data_t *p = &loc[i];
        fprintf(fp, "%s\n  {\"x\": %3d, \"y\": %3d, \"z\": %3d, \"species\": %3d}", sep, p->x, p->y, p->z, p->s);
        sep = ",";
    }
    fprintf(fp, "\n]\n");
}


// locationDataAsSExpr writes the current location data to a text file as an s-expression.
void locationDataAsSExpr(FILE *fp) {
    fprintf(fp, "(locations");
    for (int i = 0; i < num_locs; i++) {
        sp_loc_data_t *p = &loc[i];
        fprintf(fp, "\n  (location (x %3d) (y %3d) (z %3d) (species %3d))", p->x, p->y, p->z, p->s);
    }
    fprintf(fp, ")\n");
}


void save_location_data(void) {
    /* Open file 'locations.dat' for writing. */
    FILE *fp = fopen("locations.dat", "wb");
    if (fp == NULL) {
        perror("save_location_data");
        fprintf(stderr, "\n\tCannot create file 'locations.dat'!\n\n");
        exit(-1);
    }

    if (num_locs > 0) {
        /* Allocate enough memory for all records. */
        binary_data_t *binData = (binary_data_t *) ncalloc(__FUNCTION__, __LINE__, num_locs, sizeof(binary_data_t));
        if (binData == NULL) {
            perror("save_location_data");
            fprintf(stderr, "\nCannot allocate enough memory for location data!\n");
            fprintf(stderr, "\n\tattempted to allocate %d location entries\n\n", num_locs);
            exit(-1);
        }

        /* translate data */
        for (int i = 0; i < num_locs; i++) {
            binData[i].x = loc[i].x;
            binData[i].y = loc[i].y;
            binData[i].z = loc[i].z;
            binData[i].s = loc[i].s;
        }

        /* Write array to disk. */
        if (fwrite(binData, sizeof(binary_data_t), num_locs, fp) != num_locs) {
            perror("save_location_data");
            fprintf(stderr, "\n\n\tCannot write to 'locations.dat'!\n\n");
            exit(-1);
        }
        free(binData);
    }
    fclose(fp);
}



