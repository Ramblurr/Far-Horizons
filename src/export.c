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
#include <string.h>
#include <stdlib.h>
#include "export.h"
#include "marshal.h"
#include "galaxyio.h"
#include "planetio.h"
#include "shipio.h"
#include "speciesio.h"
#include "stario.h"
#include "namplavars.h"
#include "shipvars.h"
#include "cjson/helpers.h"
#include "data.h"


static int exportSpecies(int spNo);

static int exportToJson();


int exportCommand(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }

        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr, "usage: export json\n");
            return 2;
        } else if (strcmp(opt, "--species") == 0 && val && strcmp(val, "05") == 0) {
            exportSpecies(5);
        } else if (strcmp(opt, "json") == 0 && val == NULL) {
            return exportToJson();
        } else {
            fprintf(stderr, "fh: export: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return 0;
}

int exportSpecies(int spNo) {
    char filename[128];
    sprintf(filename, "sp%02d.dat", spNo);
    FILE *fp = fopen(filename, "rb");
    if (fp == 0) {
        perror(filename);
        exit(2);
    }
    long bytesRead = 0;
    binary_species_data_t sp;
    if (fread(&sp, sizeof(binary_species_data_t), 1, fp) != 1) {
        perror(filename);
        exit(2);
    } else {
        bytesRead += sizeof(binary_species_data_t);
    }
    printf("              name: '%s'\n", sp.name);
    printf("         govt_name: '%s'\n", sp.govt_name);
    printf("         govt_type: '%s'\n", sp.govt_type);
    printf("                 x: %3d\n", sp.x);
    printf("                 y: %3d\n", sp.y);
    printf("                 z: %3d\n", sp.z);
    printf("                pn: %3d\n", sp.pn);
    printf("      required_gas: %3d\n", sp.required_gas);
    printf("  required_gas_min: %3d\n", sp.required_gas_min);
    printf("  required_gas_max: %3d\n", sp.required_gas_max);
    printf("         reserved5: 0x%02x\n", sp.reserved5);
    printf("       neutral_gas: [%3d, %3d, %3d, %3d, %3d, %3d]\n",
           sp.neutral_gas[0], sp.neutral_gas[1], sp.neutral_gas[2],
           sp.neutral_gas[3], sp.neutral_gas[4], sp.neutral_gas[5]);
    printf("        poison_gas: [%3d, %3d, %3d, %3d, %3d, %3d]\n",
           sp.poison_gas[0], sp.poison_gas[1], sp.poison_gas[2],
           sp.poison_gas[3], sp.poison_gas[4], sp.poison_gas[5]);
    printf("       auto_orders: 0x%02x\n", sp.auto_orders);
    printf("         reserved3: 0x%02x\n", sp.reserved3);
    printf("         reserved4: 0x%04x\n", sp.reserved4);
    printf("        tech_level: [%4d, %4d, %4d, %4d, %4d, %4d]\n",
           sp.tech_level[0], sp.tech_level[1], sp.tech_level[2], sp.tech_level[3], sp.tech_level[4], sp.tech_level[5]);
    printf("   init_tech_level: [%4d, %4d, %4d, %4d, %4d, %4d]\n",
           sp.init_tech_level[0], sp.init_tech_level[1], sp.init_tech_level[2], sp.init_tech_level[3],
           sp.init_tech_level[4], sp.init_tech_level[5]);
    printf("    tech_knowledge: [%4d, %4d, %4d, %4d, %4d, %4d]\n",
           sp.tech_knowledge[0], sp.tech_knowledge[1], sp.tech_knowledge[2], sp.tech_knowledge[3], sp.tech_knowledge[4],
           sp.tech_knowledge[5]);
    printf("       num_namplas: %6d\n", sp.num_namplas);
    printf("         num_ships: %6d\n", sp.num_ships);
    printf("          tech_eps: [%4d, %4d, %4d, %4d, %4d, %4d]\n",
           sp.tech_eps[0], sp.tech_eps[1], sp.tech_eps[2], sp.tech_eps[3], sp.tech_eps[4], sp.tech_eps[5]);
    printf("  hp_original_base: %12d\n", sp.hp_original_base);
    printf("        econ_units: %12d\n", sp.econ_units);
    printf("        fleet_cost: %12d\n", sp.fleet_cost);
    printf("fleet_percent_cost: %4d\n", sp.fleet_percent_cost);
    printf("           contact: [%8x %8x %8x %8x]\n", sp.contact[1], sp.contact[1], sp.contact[2], sp.contact[3]);
    printf("              ally: [%8x %8x %8x %8x]\n", sp.ally[0], sp.ally[1], sp.ally[2], sp.ally[3]);
    printf("             enemy: [%8x %8x %8x %8x]\n", sp.enemy[0], sp.enemy[1], sp.enemy[2], sp.enemy[3]);
    printf("           padding: [");
    for (int n = 0; n < 12; n++) {
        if (n > 0 && (n % 10 == 0)) {
            printf("\n                     ");
        }
        printf(" %2x", sp.padding[n]);
    }
    printf("]\n");
    printf("------------------: -----------------------------------------------\n");

    printf("---- named_planets: -----------------------------------------------\n");
    for (int i = 0; i < sp.num_namplas; i++) {
        binary_nampla_data_t npd;
        if (fread(&npd, sizeof(binary_nampla_data_t), 1, fp) != 1) {
            perror("named planet data:");
            exit(2);
        } else {
            bytesRead += sizeof(binary_nampla_data_t);
        }
        printf("          name: '%s'\n", npd.name);
        printf("   nampla_name: ");
        for (int n = 0; n < 32; n++) {
            printf(" %2x", npd.name[n]);
        }
        printf("]\n");
        printf("             x: %3d\n", npd.x);
        printf("             y: %3d\n", npd.y);
        printf("             z: %3d\n", npd.z);
        printf("            pn: %3d\n", npd.pn);
        printf("        status: 0x%02x\n", npd.status);
        printf("     reserved1: 0x%02x\n", npd.reserved1);
        printf("        hiding: 0x%02x\n", npd.hiding);
        printf("        hidden: 0x%02x\n", npd.hidden);
        printf("     reserved2: 0x%04x\n", npd.reserved2);
        printf("  planet_index: %4d\n", npd.planet_index);
        printf("     siege_eff: %4d\n", npd.siege_eff);
        printf("     shipyards: %4d\n", npd.shipyards);
        printf("     reserved4: 0x%08x\n", npd.reserved4);
        printf("    IUs_needed: %12d\n", npd.IUs_needed);
        printf("    AUs_needed: %12d\n", npd.AUs_needed);
        printf("      auto_IUs: %12d\n", npd.auto_IUs);
        printf("      auto_AUs: %12d\n", npd.auto_AUs);
        printf("     reserved5: 0x%08x\n", npd.reserved5);
        printf("IUs_to_install: %12d\n", npd.IUs_to_install);
        printf("AUs_to_install: %12d\n", npd.AUs_to_install);
        printf("       mi_base: %12d\n", npd.mi_base);
        printf("       ma_base: %12d\n", npd.ma_base);
        printf("     pop_units: %12d\n", npd.pop_units);
        printf(" item_quantity: [");
        for (int n = 0; n < MAX_ITEMS; n++) {
            if (n > 0 && (n % 10 == 0)) {
                printf("\n                 ");
            }
            printf(" %4d", npd.item_quantity[n]);
        }
        printf("]\n");
        printf("     reserved6: 0x%08x\n", npd.reserved6);
        printf(" use_on_ambush: %12d\n", npd.use_on_ambush);
        printf("       message: %12d\n", npd.message);
        printf("       special: 0x%08x\n", npd.special);
        printf("       padding:");
        for (int n = 0; n < 28; n++) {
            if (n == 10 || n == 20) {
                printf("\n               ");
            }
            printf(" %2x", npd.padding[n]);
        }
        printf("\n");
    }
    printf("------------------: -----------------------------------------------\n");

    printf("-------------- ships: ---------------------------------------------\n");
    for (int i = 0; i < sp.num_ships; i++) {
        binary_ship_data_t sd;
        if (fread(&sd, sizeof(binary_ship_data_t), 1, fp) != 1) {
            perror("ship data:");
            exit(2);
        } else {
            bytesRead += sizeof(binary_ship_data_t);
        }
        printf("                name: '%s'\n", sd.name);
        printf("           ship_name: [");
        for (int n = 0; n < 32; n++) {
            printf(" %2x", sd.name[n]);
        }
        printf("]\n");
        printf("                   x: %3d\n", sd.x);
        printf("                   y: %3d\n", sd.y);
        printf("                   z: %3d\n", sd.z);
        printf("                  pn: %3d\n", sd.pn);
        printf("                type: %3d\n", sd.type);
        printf("              dest_x: %3d\n", sd.dest_x);
        printf("              dest_y: %3d\n", sd.dest_y);
        printf("              dest_z: %3d\n", sd.dest_z);
        printf("         just_jumped: 0x%02x\n", sd.just_jumped);
        printf("arrived_via_wormhole: 0x%02x\n", sd.arrived_via_wormhole);
        printf("           reserved1: 0x%02x\n", sd.reserved1);
        printf("           reserved2: 0x%04x\n", sd.reserved2);
        printf("           reserved3: 0x%04x\n", sd.reserved3);
        printf("               class: %8d\n", sd.class);
        printf("             tonnage: %8d\n", sd.tonnage);
        printf("       item_quantity: [");
        for (int n = 0; n < MAX_ITEMS; n++) {
            if (n > 0 && (n % 10 == 0)) {
                printf("\n                       ");
            }
            printf(" %4d", sd.item_quantity[n]);
        }
        printf("]\n");
        printf("                 age: %8d\n", sd.age);
        printf("      remaining_cost: %8d\n", sd.remaining_cost);
        printf("           reserved4: 0x%08x\n", sd.reserved4);
        printf("       loading_point: %8d\n", sd.loading_point);
        printf("     unloading_point: %8d\n", sd.unloading_point);
        printf("             special: %8d\n", sd.special);
        printf("             padding:");
        for (int n = 0; n < 28; n++) {
            if (n == 10 || n == 20) {
                printf("\n                     ");
            }
            printf(" %2x", sd.padding[n]);
        }
        printf("\n");
    }
    printf("--------------------: ---------------------------------------------\n");

    printf("  bytes_read: %12ld\n", (long)bytesRead);
    bytesRead = 0;
    for (;!feof(fp);) {
        char buffer[128];
        bytesRead += fread(buffer, 1, 128, fp);
    }
    printf("excess_bytes: %12ld\n", (long)bytesRead);

    fclose(fp);

    return 0;
}

int exportToJson(void) {
    printf(" info: loading binary data...\n");
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    printf(" info: exporting galaxy.json...\n");
    cJSON *root = marshalGalaxyFile();
    if (root == 0) {
        fprintf(stderr, "error: there was an error converting galaxy data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "galaxy", "galaxy.json");
    cJSON_Delete(root);

    printf(" info: exporting systems.json...\n");
    root = marshalSystemsFile();
    if (root == 0) {
        fprintf(stderr, "error: there was an error converting systems data to json\n");
        exit(2);
    }
    jsonWriteFile(root, "systems", "systems.json");
    cJSON_Delete(root);

    for (int i = 0; i < MAX_SPECIES; i++) {
        if (data_in_memory[i]) {
            root = marshalSpeciesFile(&spec_data[i], namp_data[i], ship_data[i]);
            if (root == 0) {
                fprintf(stderr, "error: there was an error converting species data to json\n");
                exit(2);
            }
            char filename[128];
            sprintf(filename, "species.%03d.json", i + 1);
            printf(" info: exporting %s...\n", filename);
            jsonWriteFile(root, "species", filename);
            cJSON_Delete(root);
        }
    }

    printf(" info: export complete\n");

    return 0;
}
