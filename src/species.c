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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "galaxy.h"
#include "species.h"
#include "nampla.h"
#include "ship.h"

int data_in_memory[MAX_SPECIES];
int data_modified[MAX_SPECIES];
struct species_data spec_data[MAX_SPECIES];
int species_index; // zero-based index, mostly for accessing arrays
int species_number; // one-based index, for reports and file names
struct species_data *species;

// free_species_data will free memory used for all species data
void free_species_data(void) {
    // from galaxy.c
    extern struct galaxy_data galaxy;

    // from nampla.c
    extern struct nampla_data *namp_data[MAX_SPECIES];

    // from ship.c
    extern struct ship_data *ship_data[MAX_SPECIES];;

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (data_in_memory[species_index] == FALSE) {
            continue;
        }
        free(namp_data[species_index]);
        if (spec_data[species_index].num_ships > 0) {
            free(ship_data[species_index]);
        }
        data_in_memory[species_index] = FALSE;
        data_modified[species_index] = FALSE;
    }
}

// get_species_data will read in data files for all species
void get_species_data(void) {
    // from galaxy.c
    extern struct galaxy_data galaxy;

    // from nampla.c
    extern int extra_namplas;
    extern struct nampla_data *namp_data[MAX_SPECIES];
    extern int num_new_namplas[MAX_SPECIES];

    // from ship.c
    extern int extra_ships;
    extern struct ship_data *ship_data[MAX_SPECIES];
    extern int num_new_ships[MAX_SPECIES];

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        FILE *fp;
        char filename[16];
        struct species_data *sp = &spec_data[species_index];
        data_modified[species_index] = FALSE;

        /* Open the species data file. */
        sprintf(filename, "sp%02d.dat", species_index + 1);
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            sp->pn = 0;    /* Extinct! */
            data_in_memory[species_index] = FALSE;
            continue;
        }
        /* Read in species data. */
        if (fread(sp, sizeof(struct species_data), 1, fp) != 1) {
            fprintf(stderr, "\n\tCannot read species record in file '%s'!\n\n", filename);
            exit(-1);
        }
        /* Allocate enough memory for all namplas. */
        namp_data[species_index] = (struct nampla_data *) calloc(sp->num_namplas + extra_namplas,
                                                                 sizeof(struct nampla_data));
        if (namp_data[species_index] == NULL) {
            fprintf(stderr, "\nCannot allocate enough memory for nampla data!\n\n");
            exit(-1);
        }
        /* Read it all into memory. */
        if (fread(namp_data[species_index], sizeof(struct nampla_data), sp->num_namplas, fp) != sp->num_namplas) {
            fprintf(stderr, "\nCannot read nampla data into memory!\n\n");
            exit(-1);
        }
        /* Allocate enough memory for all ships. */
        ship_data[species_index] = (struct ship_data *) calloc(sp->num_ships + extra_ships, sizeof(struct ship_data));
        if (ship_data[species_index] == NULL) {
            fprintf(stderr, "\nCannot allocate enough memory for ship data!\n\n");
            exit(-1);
        }
        if (sp->num_ships > 0) {
            /* Read it all into memory. */
            if (fread(ship_data[species_index], sizeof(struct ship_data), sp->num_ships, fp) != sp->num_ships) {
                fprintf(stderr, "\nCannot read ship data into memory!\n\n");
                exit(-1);
            }
        }
        fclose(fp);
        data_in_memory[species_index] = TRUE;
        num_new_namplas[species_index] = 0;
        num_new_ships[species_index] = 0;
    }
}
