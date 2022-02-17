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
#include <string.h>
#include "nampla.h"
#include "namplaio.h"
#include "item.h"

typedef struct {
    uint8_t name[32];                   /* Name of planet. */
    uint8_t x, y, z, pn;                /* Coordinates. */
    uint8_t status;                     /* Status of planet. */
    uint8_t reserved1;                  /* Zero for now. */
    uint8_t hiding;                     /* HIDE order given. */
    uint8_t hidden;                     /* Colony is hidden. */
    int16_t reserved2;                  /* Zero for now. */
    int16_t planet_index;               /* Index (starting at zero) into the file "planets.dat" of this planet. */
    int16_t siege_eff;                  /* Siege effectiveness - a percentage between 0 and 99. */
    int16_t shipyards;                  /* Number of shipyards on planet. */
    int32_t reserved4;                  /* Zero for now. */
    int32_t IUs_needed;                 /* Incoming nampla with only CUs on board. */
    int32_t AUs_needed;                 /* Incoming nampla with only CUs on board. */
    int32_t auto_IUs;                   /* Number of IUs to be automatically installed. */
    int32_t auto_AUs;                   /* Number of AUs to be automatically installed. */
    int32_t reserved5;                  /* Zero for now. */
    int32_t IUs_to_install;             /* Colonial mining units to be installed. */
    int32_t AUs_to_install;             /* Colonial manufacturing units to be installed. */
    int32_t mi_base;                    /* Mining base times 10. */
    int32_t ma_base;                    /* Manufacturing base times 10. */
    int32_t pop_units;                  /* Number of available population units. */
    int32_t item_quantity[MAX_ITEMS];   /* Quantity of each item available. */
    int32_t reserved6;                  /* Zero for now. */
    int32_t use_on_ambush;              /* Amount to use on ambush. */
    int32_t message;                    /* Message associated with this planet, if any. */
    int32_t special;                    /* Different for each application. */
    uint8_t padding[28];                /* Use for expansion. Initialized to all zeroes. */
} binary_data_t;


/* load named planet data from file and create empty slots for future use */
struct nampla_data *get_nampla_data(int numNamplas, int extraNamplas, FILE *fp) {
    /* Allocate enough memory for all namplas. */
    binary_data_t *binData = (binary_data_t *) calloc(numNamplas + extraNamplas, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("get_nampla_data");
        fprintf(stderr, "\nCannot allocate enough memory for nampla data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d nampla entries\n\n", numNamplas, extraNamplas);
        exit(-1);
    }
    /* Read it all into memory. */
    if (numNamplas > 0 && fread(binData, sizeof(binary_data_t), numNamplas, fp) != numNamplas) {
        perror("get_nampla_data");
        fprintf(stderr, "\nCannot read nampla data into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* translate between the structures */
    struct nampla_data *namplaData = (struct nampla_data *) calloc(numNamplas + extraNamplas,
                                                                   sizeof(struct nampla_data));
    for (int i = 0; i < numNamplas; i++) {
        struct nampla_data *s = &namplaData[i];
        binary_data_t *data = &binData[i];
        memcpy(s->name, data->name, 32);
        s->x = data->x;
        s->y = data->y;
        s->z = data->z;
        s->pn = data->pn;
        s->status = data->status;
        s->hiding = data->hiding;
        s->hidden = data->hidden;
        s->planet_index = data->planet_index;
        s->siege_eff = data->siege_eff;
        s->shipyards = data->shipyards;
        s->IUs_needed = data->IUs_needed;
        s->AUs_needed = data->AUs_needed;
        s->auto_IUs = data->auto_IUs;
        s->auto_AUs = data->auto_AUs;
        s->IUs_to_install = data->IUs_to_install;
        s->AUs_to_install = data->AUs_to_install;
        s->mi_base = data->mi_base;
        s->ma_base = data->ma_base;
        s->pop_units = data->pop_units;
        for (int j = 0; j < MAX_ITEMS; j++) {
            s->item_quantity[j] = data->item_quantity[j];
        }
        s->use_on_ambush = data->use_on_ambush;
        s->message = data->message;
        s->special = data->special;
    }
    /* release the binary data memory we allocated */
    free(binData);

    return namplaData;
}


void namplaDataAsJson(int spNo, struct nampla_data *namplaData, int num_namplas, FILE *fp) {
    fprintf(fp, "{\n  \"species_no\": %d,\n  \"num_namplas\": %d\n}\n", spNo, num_namplas);
}


void namplaDataAsSExpr(int spNo, struct nampla_data *namplaData, int num_namplas, FILE *fp) {
    fprintf(fp, "(namplas (species_no %3d) %4d", spNo, num_namplas);
    fprintf(fp, ")\n");
}


void save_nampla_data(struct nampla_data *namplaData, int numNamplas, FILE *fp) {
    /* Allocate enough memory for all namplas. */
    binary_data_t *binData = (binary_data_t *) calloc(numNamplas, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("save_nampla_data");
        fprintf(stderr, "\nCannot allocate enough memory for nampla data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* translate between the structures */
    for (int i = 0; i < numNamplas; i++) {
        struct nampla_data *s = &namplaData[i];
        binary_data_t *data = &binData[i];
        memcpy(s->name, data->name, 32);
        data->x = s->x;
        data->y = s->y;
        data->z = s->z;
        data->pn = s->pn;
        data->status = s->status;
        data->hiding = s->hiding;
        data->hidden = s->hidden;
        data->planet_index = (int16_t) (s->planet_index);
        data->siege_eff = (int16_t) (s->siege_eff);
        data->shipyards = (int16_t) (s->shipyards);
        data->IUs_needed = s->IUs_needed;
        data->AUs_needed = s->AUs_needed;
        data->auto_IUs = s->auto_IUs;
        data->auto_AUs = s->auto_AUs;
        data->IUs_to_install = s->IUs_to_install;
        data->AUs_to_install = s->AUs_to_install;
        data->mi_base = s->mi_base;
        data->ma_base = s->ma_base;
        data->pop_units = s->pop_units;
        for (int j = 0; j < MAX_ITEMS; j++) {
            data->item_quantity[j] = s->item_quantity[j];
        }
        data->use_on_ambush = s->use_on_ambush;
        data->message = s->message;
        data->special = s->special;
    }
    /* Write nampla data. */
    if (numNamplas > 0 && fwrite(binData, sizeof(struct nampla_data), numNamplas, fp) != numNamplas) {
        perror("save_nampla_data");
        fprintf(stderr, "\nCannot write nampla data to file!\n");
        fprintf(stderr, "\n\tattempted to write %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* release the binary data memory we allocated */
    free(binData);
}


