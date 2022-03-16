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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "engine.h"
#include "enginevars.h"
#include "galaxyio.h"
#include "transactionio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "stario.h"
#include "planetio.h"
#include "speciesvars.h"
#include "namplavars.h"
#include "planetvars.h"
#include "commandvars.h"
#include "command.h"
#include "logvars.h"
#include "production.h"
#include "productionvars.h"
#include "do.h"
#include "money.h"
#include "intercept.h"

int productionPass(int sp_num[], int num_species, int do_all_species, int first_pass);

int productionPassSpecies(int spNo, int do_all_species, int first_pass);


void do_production_orders(void) {
    int i, command;

    truncate_name = TRUE;    /* For these commands, do not display age or landed/orbital status of ships. */

    if (first_pass) {
        printf("\nStart of production orders for species #%d, SP %s...\n", species_number, species->name);
    }

    doing_production = FALSE;    /* This will be set as soon as production actually starts. */
    while (TRUE) {
        command = get_command();

        if (command == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Unknown or missing command.\n");
            continue;
        }

        if (end_of_file || command == END) {
            /* Handle planets that were not given PRODUCTION orders. */
            next_nampla = nampla_base - 1;
            for (i = 0; i < species->num_namplas; i++) {
                next_nampla++;

                if (production_done[i]) {
                    continue;
                }

                production_done[i] = TRUE;

                if (next_nampla->status & DISBANDED_COLONY) {
                    continue;
                }

                if (next_nampla->mi_base + next_nampla->ma_base == 0) {
                    continue;
                }

                next_nampla_index = i;

                do_PRODUCTION_command(TRUE);
            }

            transfer_balance();    /* Terminate production for last planet for this species. */

            if (first_pass) {
                gamemaster_abort_option();
                printf("\nEnd of production orders for species #%d, SP %s.\n", species_number, species->name);
            }

            break;            /* END for this species. */
        }

        switch (command) {
            case ALLY:
                do_ALLY_command();
                break;
            case AMBUSH:
                do_AMBUSH_command();
                break;
            case BUILD:
                do_BUILD_command(FALSE, FALSE);
                break;
            case CONTINUE:
                do_BUILD_command(TRUE, FALSE);
                break;
            case DEVELOP:
                do_DEVELOP_command();
                break;
            case ENEMY:
                do_ENEMY_command();
                break;
            case ESTIMATE:
                do_ESTIMATE_command();
                break;
            case HIDE:
                do_HIDE_command();
                break;
            case IBUILD:
                do_BUILD_command(FALSE, TRUE);
                break;
            case ICONTINUE:
                do_BUILD_command(TRUE, TRUE);
                break;
            case INTERCEPT:
                do_INTERCEPT_command();
                break;
            case NEUTRAL:
                do_NEUTRAL_command();
                break;
            case PRODUCTION:
                do_PRODUCTION_command(FALSE);
                break;
            case RECYCLE:
                do_RECYCLE_command();
                break;
            case RESEARCH:
                do_RESEARCH_command();
                break;
            case SHIPYARD:
                do_SHIPYARD_command();
                break;
            case UPGRADE:
                do_UPGRADE_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid production command.\n");
        }
    }
}


int productionCommand(int argc, char *argv[]) {
    int do_all_species = TRUE;
    int dryRun = FALSE;
    int num_species = 0;
    int sp_num[MAX_SPECIES];
    memset(sp_num, 0, sizeof(sp_num));

    first_pass = FALSE;
    ignore_field_distorters = TRUE;

    /* Get commonly used data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_transaction_data();

    /* Check arguments.
     * If an argument is -p, then do two passes.
     * In the first pass, display results and prompt the GM,
     * allowing the GM to abort if necessary before saving results to disk.
     * All other arguments must be species numbers.
    * If no species numbers are specified, then do all species. */
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
            fprintf(stderr, "usage: production [--dry-run | --test]\n");
            return 2;
        } else if (strcmp(opt, "-p") == 0 && val == NULL) {
            dryRun = TRUE;
            first_pass = TRUE;
        } else if (strcmp(opt, "-t") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (strcmp(opt, "-v") == 0 && val == NULL) {
            verbose_mode = TRUE;
        } else if (strcmp(opt, "--dry-run") == 0 && val == NULL) {
            dryRun = TRUE;
            first_pass = TRUE;
        } else if (strcmp(opt, "--test") == 0 && val == NULL) {
            test_mode = TRUE;
        } else if (val == NULL && isdigit(*opt)) {
            int n = atoi(opt);
            if (n < 1 || n > galaxy.num_species) {
                fprintf(stderr, "error: productionCommand: '%s' is not a valid species number!\n", opt);
                return 2;
            }
            int found = FALSE;
            for (int j = 0; found == FALSE && j < num_species; j++) {
                if (sp_num[j] == n) {
                    found = TRUE;
                }
            }
            if (found == FALSE) {
                sp_num[num_species] = n;
                num_species++;
            }
            do_all_species = FALSE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (num_species == 0) {
        do_all_species = TRUE;
        num_species = galaxy.num_species;
        for (int i = 0; i < num_species; i++) {
            sp_num[i] = i + 1;
        }
    } else {
        do_all_species = FALSE;
        // sort the species to get consistency on output
        for (int i = 0; i < num_species; i++) {
            for (int j = i + 1; j < num_species; j++) {
                if (sp_num[j] < sp_num[i]) {
                    int tmp = sp_num[i];
                    sp_num[i] = sp_num[j];
                    sp_num[j] = tmp;
                }
            }
        }
    }

    /* Two passes through all orders will be done.
     * The first pass will check for errors and abort if any are found.
     * Results will be written to disk only on the second pass. */
    if (first_pass != FALSE) {
        printf("\nStarting first pass...\n\n");
        productionPass(sp_num, num_species, do_all_species, TRUE);
    }
    productionPass(sp_num, num_species, do_all_species, FALSE);

    save_species_data();

    if (planet_data_modified) {
        save_planet_data();
    }

    save_transaction_data();

    free_species_data();
    free(planet_base);

    return 0;
}


int productionPass(int sp_num[], int num_species, int do_all_species, int first_pass) {
    if (first_pass) {
        printf("\nStarting first pass...\n\n");
    }

    get_species_data();

    /* Main loop. For each species, take appropriate action. */
    for (int sp_index = 0; sp_index < num_species; sp_index++) {
        species_number = sp_num[sp_index];
        species_index = species_number - 1;

        int found = data_in_memory[species_index];
        if (found == FALSE) {
            if (do_all_species) {
                if (first_pass) {
                    printf("\n    Skipping species #%d.\n", species_number);
                }
                continue;
            } else {
                fprintf(stderr, "\n    Cannot get data for species #%d!\n", species_number);
                exit(2);
            }
        }

        int result = productionPassSpecies(species_number, do_all_species, first_pass);
        if (result) {
            // ?
        }
    }

    if (first_pass) {
        printf("\nFinal chance to abort safely!\n");

        gamemaster_abort_option();

        first_pass = FALSE;

        free_species_data();

        free(star_base);    /* In case data was modified. */
        free(planet_base);    /* In case data was modified. */

        printf("\nStarting second pass...\n\n");
    }

    return 0;
}


int productionPassSpecies(int spNo, int do_all_species, int first_pass) {

    species = &spec_data[species_index];
    nampla_base = namp_data[species_index];
    ship_base = ship_data[species_index];

    home_planet = planet_base + (int) nampla_base->planet_index;

    /* Open orders file for this species. */
    char filename[128];
    sprintf(filename, "sp%02d.ord", species_number);
    input_file = fopen(filename, "r");
    if (input_file == NULL) {
        if (do_all_species) {
            if (first_pass) {
                printf("\n    No orders for species #%d.\n", species_number);
            }
            return 0;
        } else {
            fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
            exit(2);
        }
    }

    end_of_file = FALSE;
    just_opened_file = TRUE;    /* Tell command parser to skip mail header, if any. */

    /* Search for START PRODUCTION order. */
    int found = FALSE;
    for (; !found;) {
        int command = get_command();
        if (command == MESSAGE) {
            /* Skip MESSAGE text. It may contain a line that starts with "start". */
            for (; command > 0 && command != ZZZ;) {
                command = get_command();
            }
            if (command < 0) {
                /* End of file. */
                fprintf(stderr, "WARNING: Unterminated MESSAGE command in file '%s'!\n", filename);
            }
        }

        if (command == START) {
            /* Get the first three letters of the keyword and convert to upper case. */
            skip_whitespace();
            char keyword[4] = {0, 0, 0, 0};
            for (int i = 0; i < 3 && *input_line_pointer != 0; i++) {
                keyword[i] = toupper(*input_line_pointer);
                input_line_pointer++;
            }
            if (strcmp(keyword, "PRO") == 0) {
                found = TRUE;
            }
        }

        if (command < 0) {
            /* End of file. */
            if (first_pass) {
                printf("\nNo production orders for species #%d, SP %s.\n", species_number, species->name);
            }
            fclose(input_file);
            input_file = NULL;
            return 0;
        }
    }

    /* Open log file. Use stdout for first pass. */
    log_stdout = FALSE;  /* We will control value of log_file from here. */
    if (first_pass) {
        log_file = stdout;
    } else {
        /* Open log file for appending. */
        sprintf(filename, "sp%02d.log", species_number);
        log_file = fopen(filename, "a");
        if (log_file == NULL) {
            fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
            exit(2);
        }
        fprintf(log_file, "\nProduction orders:\n");
        fprintf(log_file, "\n  Number of economic units at start of production: %d\n\n", species->econ_units);
    }

    // initialize arrays
    for (int i = 0; i < species->num_namplas; i++) {
        // production_done prevents more than one set of orders per planet
        if (i > 999) {
            fprintf(stderr, "\n\n\tInternal error. production_done array overflow!\n\n");
            exit(2);
        }
        production_done[i] = FALSE;

        // do other initializations
        nampla = nampla_base + i;
        nampla->auto_IUs = 0;
        nampla->auto_AUs = 0;
        nampla->IUs_needed = 0;
        nampla->AUs_needed = 0;
    }

    /* Handle production orders for this species. */
    num_intercepts = 0;
    for (int i = 0; i < 6; i++) {
        sp_tech_level[i] = species->tech_level[i];
    }

    do_production_orders();

    for (int i = 0; i < 6; i++) {
        species->tech_level[i] = sp_tech_level[i];
    }

    for (int i = 0; i < num_intercepts; i++) {
        handle_intercept(i);
    }

    data_modified[species_index] = TRUE;

    /* If this is the second pass, close the log file. */
    if (!first_pass) {
        fclose(log_file);
    }

    fclose(input_file);
    input_file = NULL;

    return 0;
}
