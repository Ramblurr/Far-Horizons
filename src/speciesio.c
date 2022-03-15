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
#include <sys/stat.h>
#include "engine.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "species.h"
#include "speciesio.h"
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
    fprintf(stderr, "get_species_data: enter\n");

    // allocate memory to load the data into memory
    binary_data_t *data = (binary_data_t *) calloc(sizeof(binary_data_t), 1);
    if (data == NULL) {
        perror("get_species_data");
        fprintf(stderr, "\nCannot allocate enough memory for species file!\n\n");
        exit(2);
    }

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        fprintf(stderr, "get_species_data: species_index = %d\n", species_index);
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
        fprintf(stderr, "get_species_data: species_index = %d\n", species_index);
        struct species_data *sp = &spec_data[species_index];

        // get the filename for the species
        char filename[128];
        sprintf(filename, "sp%02d.dat", species_index + 1);
        fprintf(stderr, "get_species_data: filename = '%s'\n", species_index);

        // see if it exists
        struct stat sb;
        if (stat(filename, &sb) != 0) {
            sp->pn = 0;    /* Extinct! */
            continue;
        }

        /* Open the species data file. */
        FILE *fp = fopen(filename, "rb");
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
            exit(2);
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

        // mdhender: added fields to help clean up code
        sp->homeColony = &namp_data[species_index][0];
        sp->homeSystem = sp->homeColony->system;

        fclose(fp);
    }

    free(data);
}


// save_species_data will write all data that has been modified
void save_species_data(void) {
    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (data_in_memory[species_index] != FALSE && data_modified[species_index] != FALSE) {
            // get the filename for the species
            char filename[128];
            sprintf(filename, "sp%02d.dat", species_index + 1);

            /* Open the species data file. */
            FILE *fp = fopen(filename, "wb");
            if (fp == NULL) {
                perror("save_species_data");
                fprintf(stderr, "\n\tCannot create new version of file '%s'!\n", filename);
                exit(2);
            }
            // save the species, colonies, and ship data
            saveSpeciesData(&spec_data[species_index], namp_data[species_index], ship_data[species_index], fp);
            // be kind and signal that it's been saved
            data_modified[species_index] = FALSE;
            // closing the file is always nice
            fclose(fp);
        }
    }
}


void saveSpeciesData(species_data_t *sp, nampla_data_t *colonies, ship_data_t *ships, FILE *fp) {
    // use a buffer on the stack to translate the data
    binary_data_t spData;
    memset(&spData, 0, sizeof(binary_data_t));

    // translate data
    memcpy(spData.name, sp->name, 32);
    memcpy(spData.govt_name, sp->govt_name, 32);
    memcpy(spData.govt_type, sp->govt_type, 32);
    spData.x = sp->x;
    spData.y = sp->y;
    spData.z = sp->z;
    spData.pn = sp->pn;
    spData.required_gas = sp->required_gas;
    spData.required_gas_min = sp->required_gas_min;
    spData.required_gas_max = sp->required_gas_max;
    for (int g = 0; g < 6; g++) {
        spData.neutral_gas[g] = sp->neutral_gas[g];
        spData.poison_gas[g] = sp->poison_gas[g];
    }
    spData.auto_orders = sp->auto_orders;
    for (int j = 0; j < 6; j++) {
        spData.tech_level[j] = sp->tech_level[j];
        spData.init_tech_level[j] = sp->init_tech_level[j];
        spData.tech_knowledge[j] = sp->tech_knowledge[j];
        spData.tech_eps[j] = sp->tech_eps[j];
    }
    spData.num_namplas = sp->num_namplas;
    spData.num_ships = sp->num_ships;
    spData.hp_original_base = sp->hp_original_base;
    spData.econ_units = sp->econ_units;
    spData.fleet_cost = sp->fleet_cost;
    spData.fleet_percent_cost = sp->fleet_percent_cost;
    for (int j = 0; j < NUM_CONTACT_WORDS; j++) {
        spData.contact[j] = sp->contact[j];
        spData.ally[j] = sp->ally[j];
        spData.enemy[j] = sp->enemy[j];
    }

    // save the translated data
    if (fwrite(&spData, sizeof(binary_data_t), 1, fp) != 1) {
        perror("saveSpeciesData:");
        fprintf(stderr, "error: cannot write species record to file\n");
        exit(2);
    }
    // save colonies data
    save_nampla_data(colonies, sp->num_namplas, fp);
    // save ships data
    save_ship_data(ships, sp->num_ships, fp);

    char filename[128];
    sprintf(filename, "species%03d.txt", sp->id);
    fp = fopen(filename, "wb");
    if (fp != NULL) {
        speciesDataAsSExpr(sp, fp);
        fclose(fp);
    }
}


static const char *tech_level_names[6] = {"MI", "MA", "ML", "GV", "LS", "BI"};


// speciesDataAsJson writes the current species data to a text file as JSON.
void speciesDataAsJson(species_data_t *sp, FILE *fp) {
    fprintf(fp, "{\n");
    fprintf(fp, "  \"id\": %d,\n", sp->id);
    fprintf(fp, "  \"sp_no\": %d,\n", sp->id);
    fprintf(fp, "  \"name\": \"%s\",\n", sp->name);
    fprintf(fp, "  \"auto\": %s,\n", sp->auto_orders ? "true" : "false");
    fprintf(fp, "  \"government\": {\"name\": \"%s\", \"type\": \"%s\"},\n",
            sp->govt_name, sp->govt_type);
    fprintf(fp, "  \"homeworld\": {\"x\": %d, \"y\": %d, \"z\": %d, \"orbit\": %d, \"hp_base\": %d},\n",
            sp->x, sp->y, sp->z, sp->pn, sp->hp_original_base);
    fprintf(fp, "  \"atmosphere\": {\n");
    fprintf(fp, "      \"required\": {\"gas\": %d, \"min\": %d, \"max\": %d},\n",
            sp->required_gas, sp->required_gas_min, sp->required_gas_max);
    fprintf(fp, "      \"neutral\": [%d, %d, %d, %d, %d, %d],\n",
            sp->neutral_gas[0], sp->neutral_gas[1], sp->neutral_gas[2], sp->neutral_gas[3],
            sp->neutral_gas[4], sp->neutral_gas[5]);
    fprintf(fp, "      \"poison\": [%d, %d, %d, %d, %d, %d]\n  },\n",
            sp->poison_gas[0], sp->poison_gas[1], sp->poison_gas[2], sp->poison_gas[3],
            sp->poison_gas[4], sp->poison_gas[5]);
    fprintf(fp, "  \"technology\": {\n");
    for (int i = 0; i < 6; i++) {
        fprintf(fp, "    \"%s\": {\"level\": %d, \"knowledge\": %d, \"init\": %d, \"xp\": %d}",
                tech_level_names[i], sp->tech_level[i], sp->tech_knowledge[i], sp->init_tech_level[i], sp->tech_eps[i]);
        if (i != 5) {
            fprintf(fp, ",\n");
        }
    }
    fprintf(fp, "\n  },\n");
    fprintf(fp, "  \"num_namplas\": %d,\n", sp->num_namplas);
    fprintf(fp, "  \"num_ships\": %d,\n", sp->num_ships);
    fprintf(fp, "  \"fleet_maintenance\": {\"cost\": %d, \"percent\": %d},\n", sp->fleet_cost, sp->fleet_percent_cost);
    fprintf(fp, "  \"banked_eu\": %d,\n", sp->econ_units);
    fprintf(fp, "  \"contacts\": [");
    const char *sep = "";
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->contact[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, "%s%d", sep, spidx + 1);
            sep = ", ";
        }
    }
    fprintf(fp, "],\n");
    fprintf(fp, "  \"allies\": [");
    sep = "";
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->ally[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, "%s%d", sep, spidx + 1);
            sep = ", ";
        }
    }
    fprintf(fp, "],\n");
    fprintf(fp, "  \"enemies\": [");
    sep = "";
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->enemy[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, "%s%d", sep, spidx + 1);
            sep = ", ";
        }
    }
    fprintf(fp, "]\n}\n");
}


// speciesDataAsSExpr writes the current species data to a text file as an s-expression.
void speciesDataAsSExpr(species_data_t *sp, FILE *fp) {
    fprintf(fp, "(species (id %3d) (name '%s') (auto %s)", sp->id, sp->name, sp->auto_orders ? "true" : "false");
    fprintf(fp, "\n         (government (name '%s') (type '%s'))", sp->govt_name, sp->govt_type);
    fprintf(fp, "\n         (homeworld (x %3d) (y %3d) (z %3d) (orbit %d) (hp_base %d))",
            sp->x, sp->y, sp->z, sp->pn, sp->hp_original_base);
    fprintf(fp, "\n         (atmosphere");
    fprintf(fp, "\n           (required (gas %2d) (min %3d) (max %3d))",
            sp->required_gas, sp->required_gas_min, sp->required_gas_max);
    fprintf(fp, "\n           (neutral %2d %2d %2d %2d %2d %2d)",
            sp->neutral_gas[0], sp->neutral_gas[1], sp->neutral_gas[2],
            sp->neutral_gas[3], sp->neutral_gas[4], sp->neutral_gas[5]);
    fprintf(fp, "\n           (poison  %2d %2d %2d %2d %2d %2d)",
            sp->poison_gas[0], sp->poison_gas[1], sp->poison_gas[2],
            sp->poison_gas[3], sp->poison_gas[4], sp->poison_gas[5]);
    fprintf(fp, ")");
    fprintf(fp, "\n         (technology");
    for (int i = 0; i < 6; i++) {
        fprintf(fp, "\n           (tech (code '%s') (level %3d) (knowledge %3d) (init %2d) (xp %5d))",
                tech_level_names[i], sp->tech_level[i], sp->tech_knowledge[i], sp->init_tech_level[i], sp->tech_eps[i]);
    }
    fprintf(fp, ")");
    fprintf(fp, "\n         (fleet (num_ships %5d) (maintenance (cost %9d) (percent %6d)))",
            sp->num_ships, sp->fleet_cost, sp->fleet_percent_cost);
    fprintf(fp, "\n         (num_namplas %7d)", sp->num_namplas);
    fprintf(fp, "\n         (banked_eu %9d)", sp->econ_units);
    fprintf(fp, "\n         (contacts");
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->contact[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, " %3d", spidx + 1);
        }
    }
    fprintf(fp, ")");
    fprintf(fp, "\n         (allies  ");
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->ally[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, " %3d", spidx + 1);
        }
    }
    fprintf(fp, ")");
    fprintf(fp, "\n         (enemies ");
    for (int spidx = 0; spidx < galaxy.num_species; spidx++) {
        if ((sp->enemy[spidx / 32] & (1 << (spidx % 32))) != 0) {
            fprintf(fp, " %3d", spidx + 1);
        }
    }
    fprintf(fp, ")");
    fprintf(fp, ")\n");
}

