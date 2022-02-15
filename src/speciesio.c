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
#include "galaxy.h"
#include "galaxyio.h"
#include "species.h"
#include "namplaio.h"
#include "namplavars.h"
#include "shipio.h"
#include "shipvars.h"

int data_in_memory[MAX_SPECIES];

int data_modified[MAX_SPECIES];

struct species_data spec_data[MAX_SPECIES];


typedef struct {
    uint8_t name[32];                      /* Name of species. */
    uint8_t govt_name[32];                 /* Name of government. */
    uint8_t govt_type[32];                 /* Type of government. */
    uint8_t x, y, z, pn;                   /* Coordinates of home planet. */
    uint8_t required_gas;                  /* Gas required by species. */
    uint8_t required_gas_min;              /* Minimum needed percentage. */
    uint8_t required_gas_max;              /* Maximum allowed percentage. */
    uint8_t reserved5;                     /* Zero for now. */
    uint8_t neutral_gas[6];                /* Gases neutral to species. */
    uint8_t poison_gas[6];                 /* Gases poisonous to species. */
    uint8_t auto_orders;                   /* AUTO command was issued. */
    uint8_t reserved3;                     /* Zero for now. */
    int16_t reserved4;                     /* Zero for now. */
    int16_t tech_level[6];                 /* Actual tech levels. */
    int16_t init_tech_level[6];            /* Tech levels at start of turn. */
    int16_t tech_knowledge[6];             /* Unapplied tech level knowledge. */
    int32_t num_namplas;                   /* Number of named planets, including home planet and colonies. */
    int32_t num_ships;                     /* Number of ships. */
    int32_t tech_eps[6];                   /* Experience points for tech levels. */
    int32_t hp_original_base;              /* If non-zero, home planet was bombed either by bombardment or germ warfare and has not yet fully recovered. Value is total economic base before bombing. */
    int32_t econ_units;                    /* Number of economic units. */
    int32_t fleet_cost;                    /* Total fleet maintenance cost. */
    int32_t fleet_percent_cost;            /* Fleet maintenance cost as a percentage times one hundred. */
    uint32_t contact[NUM_CONTACT_WORDS];   /* A bit is set if corresponding species has been met. */
    uint32_t ally[NUM_CONTACT_WORDS];      /* A bit is set if corresponding species is considered an ally. */
    uint32_t enemy[NUM_CONTACT_WORDS];     /* A bit is set if corresponding species is considered an enemy. */
    uint8_t padding[12];                   /* Use for expansion. Initialized to all zeroes. */
} binary_data_t;


// get_species_data will read in data files for all species
void get_species_data(void) {
    FILE *fp;
    struct stat sb;
    char filename[16];

    // allocate memory to load the data into memory
    binary_data_t *data = (binary_data_t *) calloc(sizeof(binary_data_t), 1);
    if (data == NULL) {
        perror("get_species_data");
        fprintf(stderr, "\nCannot allocate enough memory for species file!\n\n");
        exit(-1);
    }

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        struct species_data *sp = &spec_data[species_index];

        // clear out any existing species data
        memset(&spec_data[species_index], 0, sizeof(struct species_data));
        if (namp_data[species_index] != NULL) {
            free(namp_data[species_index]);
            namp_data[species_index] = NULL;
        }
        if (ship_data[species_index] != NULL) {
            free(ship_data[species_index]);
            ship_data[species_index] = NULL;
        }
        num_new_namplas[species_index] = 0;
        num_new_ships[species_index] = 0;
        data_modified[species_index] = FALSE;
        data_in_memory[species_index] = FALSE;
    }

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        struct species_data *sp = &spec_data[species_index];

        // get the filename for the species
        sprintf(filename, "sp%02d.dat", species_index + 1);

        // see if it exists
        if (stat(filename, &sb) != 0) {
            sp->pn = 0;    /* Extinct! */
            continue;
        }

        /* Open the species data file. */
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror("get_species_data");
            continue;
        }

        // clear out the translation buffer
        memset(sp, 0, sizeof(binary_data_t));

        /* Read in species data. */
        if (fread(data, sizeof(binary_data_t), 1, fp) != 1) {
            perror("get_species_data");
            fprintf(stderr, "\nCannot read species record in file '%s'!\n\n", filename);
            exit(-1);
        }

        // translate data
        memcpy(sp->name, data->name, 32);
        memcpy(sp->govt_name, data->govt_name, 32);
        memcpy(sp->govt_type, data->govt_type, 32);
        sp->x = data->x;
        sp->y = data->y;
        sp->z = data->z;
        sp->pn = data->pn;
        sp->required_gas = data->required_gas;
        sp->required_gas_min = data->required_gas_min;
        sp->required_gas_max = data->required_gas_max;
        for (int g = 0; g < 6; g++) {
            sp->neutral_gas[g] = data->neutral_gas[g];
            sp->poison_gas[g] = data->poison_gas[g];
        }
        sp->auto_orders = data->auto_orders;
        for (int j = 0; j < 6; j++) {
            sp->tech_level[j] = data->tech_level[j];
            sp->init_tech_level[j] = data->init_tech_level[j];
            sp->tech_knowledge[j] = data->tech_knowledge[j];
            sp->tech_eps[j] = data->tech_eps[j];
        }
        sp->num_namplas = data->num_namplas;
        sp->num_ships = data->num_ships;
        sp->hp_original_base = data->hp_original_base;
        sp->econ_units = data->econ_units;
        sp->fleet_cost = data->fleet_cost;
        sp->fleet_percent_cost = data->fleet_percent_cost;
        for (int j = 0; j < NUM_CONTACT_WORDS; j++) {
            sp->contact[j] = data->contact[j];
            sp->ally[j] = data->ally[j];
            sp->enemy[j] = data->enemy[j];
        }

        /* load nampla data from file and create empty slots for future use */
        namp_data[species_index] = get_nampla_data(sp->num_namplas, extra_namplas, fp);

        /* load ship data from file and create empty slots for future use */
        ship_data[species_index] = get_ship_data(sp->num_ships, extra_ships, fp);

        data_in_memory[species_index] = TRUE;
        num_new_namplas[species_index] = 0;
        num_new_ships[species_index] = 0;

        fclose(fp);
    }

    free(data);
}

// save_species_data will write all data that has been modified
void save_species_data(void) {
    FILE *fp;
    char filename[16];

    // allocate memory to translate the data
    binary_data_t *data = (binary_data_t *) calloc(sizeof(binary_data_t), 1);
    if (data == NULL) {
        perror("save_species_data");
        fprintf(stderr, "\nCannot allocate enough memory for species file!\n\n");
        exit(-1);
    }

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (data_modified[species_index]) {
            struct species_data *sp = &spec_data[species_index];

            // clear out the translation buffer
            memset(sp, 0, sizeof(binary_data_t));

            // translate data
            data->x = sp->x;
            data->y = sp->y;
            data->z = sp->z;
            data->pn = sp->pn;
            data->required_gas = sp->required_gas;
            data->required_gas_min = sp->required_gas_min;
            data->required_gas_max = sp->required_gas_max;
            for (int g = 0; g < 6; g++) {
                data->neutral_gas[g] = sp->neutral_gas[g];
                data->poison_gas[g] = sp->poison_gas[g];
            }
            data->auto_orders = sp->auto_orders;
            for (int j = 0; j < 6; j++) {
                data->tech_level[j] = sp->tech_level[j];
                data->init_tech_level[j] = sp->init_tech_level[j];
                data->tech_knowledge[j] = sp->tech_knowledge[j];
                data->tech_eps[j] = sp->tech_eps[j];
            }
            data->num_namplas = sp->num_namplas;
            data->num_ships = sp->num_ships;
            data->hp_original_base = sp->hp_original_base;
            data->econ_units = sp->econ_units;
            data->fleet_cost = sp->fleet_cost;
            data->fleet_percent_cost = sp->fleet_percent_cost;
            for (int j = 0; j < NUM_CONTACT_WORDS; j++) {
                data->contact[j] = sp->contact[j];
                data->ally[j] = sp->ally[j];
                data->enemy[j] = sp->enemy[j];
            }

            // get the filename for the species
            sprintf(filename, "sp%02d.dat", species_index + 1);

            /* Open the species data file. */
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
            save_nampla_data(namp_data[species_index], sp->num_namplas, fp);

            /* Write ship data. */
            save_ship_data(ship_data[species_index], sp->num_ships, fp);

            data_modified[species_index] = FALSE;

            fclose(fp);
        }
    }

    free(data);
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
