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
#include "galaxy.h"
#include "planet.h"
#include "species.h"
#include "nampla.h"
#include "ship.h"

int data_in_memory[MAX_SPECIES];
int data_modified[MAX_SPECIES];
struct species_data *species;
struct species_data spec_data[MAX_SPECIES];
int species_index; // zero-based index, mostly for accessing arrays
int species_number; // one-based index, for reports and file names


/* The following routine provides the 'distorted' species number used to
	identify a species that uses field distortion units. The input
	variable 'species_number' is the same number used in filename
	creation for the species. */
int distorted(int species_number) {
    int i, j, n, ls;
    /* We must use the LS tech level at the start of the turn because
       the distorted species number must be the same throughout the
       turn, even if the tech level changes during production. */
    ls = spec_data[species_number - 1].init_tech_level[LS];
    i = species_number & 0x000F; /* Lower four bits. */
    j = (species_number >> 4) & 0x000F; /* Upper four bits. */
    n = (ls % 5 + 3) * (4 * i + j) + (ls % 11 + 7);
    return n;
}


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


/* Get life support tech level needed. */
int life_support_needed(struct species_data *species, struct planet_data *home, struct planet_data *colony) {
    int j, k, ls_needed;
    int i = colony->temperature_class - home->temperature_class;
    if (i < 0) { i = -i; }
    ls_needed = 3 * i;        /* Temperature class. */
    i = colony->pressure_class - home->pressure_class;
    if (i < 0) { i = -i; }
    ls_needed += 3 * i;        /* Pressure class. */
    /* Check gases. Assume required gas is NOT present. */
    ls_needed += 3;
    /* Check gases on planet. */
    for (j = 0; j < 4; j++) {
        if (colony->gas_percent[j] == 0) { continue; }
        /* Compare with poisonous gases. */
        for (i = 0; i < 6; i++) {
            if (species->poison_gas[i] == colony->gas[j]) {
                ls_needed += 3;
            }
        }
        if (colony->gas[j] == species->required_gas) {
            if (colony->gas_percent[j] >= species->required_gas_min
                && colony->gas_percent[j] <= species->required_gas_max) {
                ls_needed -= 3;
            }
        }
    }
    return ls_needed;
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

// save_species_data will write all data that has been modified
void save_species_data(void) {
    // from galaxy.c
    extern struct galaxy_data galaxy;
    // from nampla.c
    extern struct nampla_data *namp_data[MAX_SPECIES];
    // from ship.c
    extern struct ship_data *ship_data[MAX_SPECIES];;

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        FILE *fp;
        char filename[16];
        struct species_data *sp;

        if (!data_modified[species_index]) {
            continue;
        }
        sp = &spec_data[species_index];
        /* Open the species data file. */
        sprintf(filename, "sp%02d.dat", species_index + 1);
        fp = fopen(filename, "wb");
        if (fp == NULL) {
            perror("save_species_data");
            fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
            exit(-1);
        }
        /* Write species data. */
        if (fwrite(sp, sizeof(struct species_data), 1, fp) != 1) {
            perror("save_species_data");
            fprintf(stderr, "\n\tCannot write species record to file '%s'!\n\n", filename);
            exit(-1);
        }
        /* Write nampla data. */
        if (fwrite(namp_data[species_index], sizeof(struct nampla_data), 1, fp) != 1) {
            perror("save_species_data");
            fprintf(stderr, "\n\tCannot write nampla data to file '%s'!\n\n", filename);
            exit(-1);
        }
        /* Write ship data. */
        if (sp->num_ships > 0) {
            if (fwrite(ship_data[species_index], sizeof(struct ship_data), 1, fp) != 1) {
                perror("save_species_data");
                fprintf(stderr, "\n\tCannot write ship data to file '%s'!\n\n", filename);
                exit(-1);
            }
        }
        fclose(fp);
        data_modified[species_index] = FALSE;
    }
}

// speciesDataAsSExpr writes the current species data to a text file as an s-expression.
void speciesDataAsSExpr(FILE *fp, species_data_t *sp, int spNo) {
    fprintf(fp, "(species %3d (name \"%s\") (government (name \"%s\") (type \"%s\"))", spNo, sp->name, sp->govt_name,
            sp->govt_type);
    fprintf(fp, "\n  (name \"%s\")", sp->name);
    fprintf(fp, "\n  (government (name \"%s\") (type \"%s\"))", sp->govt_name, sp->govt_type);
    fprintf(fp, "\n  (homeworld (x %3d) (y %3d) (z %3d) (orbit %d))", sp->x, sp->y, sp->z, sp->pn);
    fprintf(fp, ")\n");
}

int undistorted(int distorted_species_number) {
    int i, species_number;
    for (i = 0; i < MAX_SPECIES; i++) {
        species_number = i + 1;
        if (distorted(species_number) == distorted_species_number) {
            return species_number;
        }
    }
    return 0;    /* Not a legitimate species. */
}


