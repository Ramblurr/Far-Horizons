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
#include <assert.h>
#include "data.h"
#include "item.h"
#include "nampla.h"
#include "namplaio.h"
#include "planetio.h"


/* load named planet data from file and create empty slots for future use */
struct nampla_data *get_nampla_data(int numNamplas, int extraNamplas, FILE *fp) {
    assert(planet_base != NULL);

    /* Allocate enough memory for all namplas. */
    binary_nampla_data_t *binData = (binary_nampla_data_t *) ncalloc(__FUNCTION__, __LINE__, numNamplas + extraNamplas,
                                                                     sizeof(binary_nampla_data_t));
    if (binData == NULL) {
        perror("get_nampla_data");
        fprintf(stderr, "\nCannot allocate enough memory for nampla data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d + %d nampla entries\n\n", numNamplas, extraNamplas);
        exit(-1);
    }
    /* Read it all into memory. */
    if (numNamplas > 0 && fread(binData, sizeof(binary_nampla_data_t), numNamplas, fp) != numNamplas) {
        perror("get_nampla_data");
        fprintf(stderr, "\nCannot read nampla data into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* translate between the structures */
    struct nampla_data *namplaData = (struct nampla_data *) ncalloc(__FUNCTION__, __LINE__, numNamplas + extraNamplas,
                                                                    sizeof(struct nampla_data));
    for (int i = 0; i < numNamplas; i++) {
        struct nampla_data *nampla = &namplaData[i];
        binary_nampla_data_t *data = &binData[i];
        memcpy(nampla->name, data->name, 32);
        nampla->x = data->x;
        nampla->y = data->y;
        nampla->z = data->z;
        nampla->pn = data->pn;
        nampla->status = data->status;
        nampla->hiding = data->hiding;
        nampla->hidden = data->hidden;
        nampla->planet_index = data->planet_index;
        nampla->siege_eff = data->siege_eff;
        nampla->shipyards = data->shipyards;
        nampla->IUs_needed = data->IUs_needed;
        nampla->AUs_needed = data->AUs_needed;
        nampla->auto_IUs = data->auto_IUs;
        nampla->auto_AUs = data->auto_AUs;
        nampla->IUs_to_install = data->IUs_to_install;
        nampla->AUs_to_install = data->AUs_to_install;
        nampla->mi_base = data->mi_base;
        nampla->ma_base = data->ma_base;
        nampla->pop_units = data->pop_units;
        for (int j = 0; j < MAX_ITEMS; j++) {
            nampla->item_quantity[j] = data->item_quantity[j];
        }
        nampla->use_on_ambush = data->use_on_ambush;
        nampla->message = data->message;
        nampla->special = data->special;

        // mdhender: added fields to help clean up code
        nampla->id = i + 1;
        nampla->planet = planet_base + nampla->planet_index;
        nampla->star = nampla->planet->star;
    }
    /* release the binary data memory we allocated */
    free(binData);

    return namplaData;
}


void namplaDataAsJson(int spNo, struct nampla_data *namplaData, int num_namplas, FILE *fp) {
    fprintf(fp, "{\n");
    fprintf(fp, "  \"species_no\": %d,\n", spNo);
    fprintf(fp, "  \"namplas\": [");
    for (int i = 0; i < num_namplas; i++) {
        struct nampla_data *np = &namplaData[i];
        fprintf(fp, "{\n");
        fprintf(fp, "      \"id\": %d,\n", i + 1);
        fprintf(fp, "      \"name\": \"%s\",\n", np->name);
        fprintf(fp, "      \"planet\": {\"id\": %5d, \"x\": %3d, \"y\": %3d, \"z\": %3d, \"orbit\": %d},\n",
                np->planet_index, np->x, np->y, np->z, np->pn);
        fprintf(fp, "      \"status\":        %9d,\n", np->status);
        fprintf(fp, "      \"hiding\":        %9s,\n", np->hiding ? "true" : "false");
        fprintf(fp, "      \"hiding_val\":    %9d,\n", np->hiding);
        fprintf(fp, "      \"hidden\":        %9s,\n", np->hidden ? "true" : "false");
        fprintf(fp, "      \"hidden_val\":    %9d,\n", np->hidden);
        fprintf(fp, "      \"ma_base\":       %9d,\n", np->ma_base);
        fprintf(fp, "      \"mi_base\":       %9d,\n", np->mi_base);
        fprintf(fp, "      \"pop_units\":     %9d,\n", np->pop_units);
        fprintf(fp, "      \"shipyards\":     %9d,\n", np->shipyards);
        fprintf(fp, "      \"siege_eff\":     %9d,\n", np->siege_eff);
        fprintf(fp, "      \"special\":       %9d,\n", np->special);
        fprintf(fp, "      \"use_on_ambush\": %9d,\n", np->use_on_ambush);
        fprintf(fp, "      \"au\": {\"auto\": %6d, \"needed\": %6d, \"to_install\": %6d},\n",
                np->auto_AUs, np->AUs_needed, np->AUs_to_install);
        fprintf(fp, "      \"iu\": {\"auto\": %6d, \"needed\": %6d, \"to_install\": %6d},\n",
                np->auto_IUs, np->IUs_needed, np->IUs_to_install);
        const char *sep = "\n";
        fprintf(fp, "      \"items\": [");
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (np->item_quantity[j] > 0) {
                fprintf(fp, "%s        {\"code\": %2d, \"qty\": %6d}", sep, j, np->item_quantity[j]);
                sep = ",\n";
            }
        }
        if (*sep == ',') {
            // not an empty list so put closing bracket on new line
            fprintf(fp, "\n      ");
        }
        fprintf(fp, "]\n");
        fprintf(fp, "    }");
        if (i + 1 < num_namplas) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "]\n");
    fprintf(fp, "}\n");
}


void namplaDataAsSExpr(int spNo, struct nampla_data *namplaData, int num_namplas, FILE *fp) {
    fprintf(fp, "(namplas (species_no %3d)", spNo);
    for (int i = 0; i < num_namplas; i++) {
        struct nampla_data *np = &namplaData[i];
        fprintf(fp, "\n         (nampla (id %5d) (name \"%s\")", i + 1, np->name);
        fprintf(fp, "\n                 (planet (id %5d) (x %3d) (y %3d) (z %3d) (orbit %d))",
                np->planet_index, np->x, np->y, np->z, np->pn);
        fprintf(fp, "\n                 (status        %9d)", np->status);
        fprintf(fp, "\n                 (hiding        %-5s %3d)", np->hiding ? "true" : "false", np->hiding);
        fprintf(fp, "\n                 (hidden        %-5s %3d)", np->hidden ? "true" : "false", np->hidden);
        fprintf(fp, "\n                 (ma_base       %9d)", np->ma_base);
        fprintf(fp, "\n                 (mi_base       %9d)", np->mi_base);
        fprintf(fp, "\n                 (pop_units     %9d)", np->pop_units);
        fprintf(fp, "\n                 (shipyards     %9d)", np->shipyards);
        fprintf(fp, "\n                 (siege_eff     %9d)", np->siege_eff);
        fprintf(fp, "\n                 (special       %9d)", np->special);
        fprintf(fp, "\n                 (use_on_ambush %9d)", np->use_on_ambush);
        fprintf(fp, "\n                 (au (auto %6d) (needed %6d) (to_install %6d))",
                np->auto_AUs, np->AUs_needed, np->AUs_to_install);
        fprintf(fp, "\n                 (iu (auto %6d) (needed %6d) (to_install %6d))",
                np->auto_IUs, np->IUs_needed, np->IUs_to_install);
        const char *sep = "";
        fprintf(fp, "\n                 (items");
        for (int j = 0; j < MAX_ITEMS; j++) {
            if (np->item_quantity[j] > 0) {
                fprintf(fp, "%s (item (code %2d) (qty %6d))", sep, j, np->item_quantity[j]);
                sep = "\n                       ";
            }
        }
        fprintf(fp, "))");
    }
    fprintf(fp, ")\n");
}


void save_nampla_data(struct nampla_data *namplaData, int numNamplas, FILE *fp) {
    /* Allocate enough memory for all namplas. */
    binary_nampla_data_t *binData = (binary_nampla_data_t *) ncalloc(__FUNCTION__, __LINE__, numNamplas,
                                                                     sizeof(binary_nampla_data_t));
    if (binData == NULL) {
        perror("save_nampla_data");
        fprintf(stderr, "\nCannot allocate enough memory for nampla data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* translate between the structures */
    for (int i = 0; i < numNamplas; i++) {
        struct nampla_data *nampla = &namplaData[i];
        binary_nampla_data_t *data = &binData[i];
        strncpy((char *)(data->name), nampla->name, 32);
        data->x = nampla->x;
        data->y = nampla->y;
        data->z = nampla->z;
        data->pn = nampla->pn;
        data->status = nampla->status;
        data->hiding = nampla->hiding;
        data->hidden = nampla->hidden;
        data->planet_index = (int16_t) (nampla->planet_index);
        data->siege_eff = (int16_t) (nampla->siege_eff);
        data->shipyards = (int16_t) (nampla->shipyards);
        data->IUs_needed = nampla->IUs_needed;
        data->AUs_needed = nampla->AUs_needed;
        data->auto_IUs = nampla->auto_IUs;
        data->auto_AUs = nampla->auto_AUs;
        data->IUs_to_install = nampla->IUs_to_install;
        data->AUs_to_install = nampla->AUs_to_install;
        data->mi_base = nampla->mi_base;
        data->ma_base = nampla->ma_base;
        data->pop_units = nampla->pop_units;
        for (int j = 0; j < MAX_ITEMS; j++) {
            data->item_quantity[j] = nampla->item_quantity[j];
        }
        data->use_on_ambush = nampla->use_on_ambush;
        data->message = nampla->message;
        data->special = nampla->special;
    }
    /* Write nampla data. */
    if (numNamplas > 0 && fwrite(binData, sizeof(binary_nampla_data_t), numNamplas, fp) != numNamplas) {
        perror("save_nampla_data");
        fprintf(stderr, "\nCannot write nampla data to file!\n");
        fprintf(stderr, "\n\tattempted to write %d nampla entries\n\n", numNamplas);
        exit(-1);
    }
    /* release the binary data memory we allocated */
    free(binData);
}


