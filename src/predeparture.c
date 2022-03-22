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
#include <ctype.h>
#include <stdlib.h>
#include "engine.h"
#include "command.h"
#include "commandvars.h"
#include "do.h"
#include "enginevars.h"
#include "galaxyio.h"
#include "logvars.h"
#include "planetio.h"
#include "predeparture.h"
#include "shipvars.h"
#include "speciesvars.h"
#include "transactionio.h"
#include "speciesio.h"
#include "stario.h"
#include "namplavars.h"
#include "log.h"


int preDeparturePass(int sp_num[], int do_all_species, int first_pass);
int preDepartureSpecies(int spNo, int do_all_species, int first_pass);


void do_predeparture_orders(void) {
    int i, command, old_test_mode;

    if (first_pass) {
        printf("\nStart of pre-departure orders for species #%d, SP %s...\n", species_number, species->name);
    }

    truncate_name = TRUE;    /* For these commands, do not display age or landed/orbital status of ships. */

    while (TRUE) {
        command = get_command();

        if (command == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Unknown or missing command.\n");
            continue;
        }

        if (end_of_file || command == END) {
            if (first_pass) {
                printf("End of pre-departure orders for species #%d, SP %s.\n", species_number, species->name);
            }

            if (first_pass) {
                gamemaster_abort_option();
            }

            break;            /* END for this species. */
        }

        switch (command) {
            case ALLY:
                do_ALLY_command();
                break;
            case BASE:
                do_BASE_command();
                break;
            case DEEP:
                do_DEEP_command();
                break;
            case DESTROY:
                do_DESTROY_command();
                break;
            case DISBAND:
                do_DISBAND_command();
                break;
            case ENEMY:
                do_ENEMY_command();
                break;
            case INSTALL:
                do_INSTALL_command();
                break;
            case LAND:
                do_LAND_command();
                break;
            case MESSAGE:
                do_MESSAGE_command();
                break;
            case NAME:
                do_NAME_command();
                break;
            case NEUTRAL:
                do_NEUTRAL_command();
                break;
            case ORBIT:
                do_ORBIT_command();
                break;
            case REPAIR:
                do_REPAIR_command();
                break;
            case SCAN:
                /* Scan is okay in test mode for pre-departure. */
                old_test_mode = test_mode;
                test_mode = FALSE;
                do_SCAN_command();
                test_mode = old_test_mode;
                break;
            case SEND:
                do_SEND_command();
                break;
            case TRANSFER:
                do_TRANSFER_command();
                break;
            case UNLOAD:
                do_UNLOAD_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid pre-departure command.\n");
        }
    }
}


int preDepartureCommand(int argc, char *argv[]) {
    int do_all_species = TRUE;
    int dryRun = FALSE;
    int num_species = 0;
    int sp_num[MAX_SPECIES];
    memset(sp_num, 0, sizeof(sp_num));

    /* Get commonly used data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();
    get_transaction_data();

    // set important globals
    ignore_field_distorters = TRUE;

    /* Check arguments.
     * If an argument is -p, then do two passes.
     * In the first pass, display results and prompt the GM,
     * allowing the GM to abort if necessary before saving results to disk.
     * If an argument is -t, then set test mode.
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
            fprintf(stderr, "fh: usage: pre-departure [--dry-run | --test]\n");
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
                fprintf(stderr, "error: preDepartureCommand: '%s' is not a valid species number!\n", opt);
                return 2;
            }
            int found = FALSE;
            for (int j = 0; found == FALSE && j < num_species; j++) {
                if (sp_num[j] == n) {
                    found = TRUE;
                }
            }
            if (found == FALSE) {
                sp_num[num_species++] = n;
            }
            do_all_species = FALSE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (num_species == 0 || do_all_species != FALSE) {
        num_species = galaxy.num_species;
        for (num_species = 0; num_species < galaxy.num_species; num_species++) {
            sp_num[num_species] = num_species + 1;
        }
        do_all_species = TRUE;
    }

    /* Two passes through all orders will be done.
     * The first pass will check for errors and abort if any are found.
     * Results will be written to disk only on the second pass. */
    if (first_pass != FALSE) {
        printf("\nStarting first pass...\n\n");
        preDeparturePass(sp_num, do_all_species, TRUE);
    }
    preDeparturePass(sp_num, do_all_species, FALSE);

    return 0;
}


int preDeparturePass(int sp_num[], int do_all_species, int first_pass) {
    get_star_data();
    get_planet_data();
    get_species_data();

    /* Main loop. For each species, take appropriate action. */
    for (int sp_index = 0; sp_index < sp_num[sp_index] != 0; sp_index++) {
        species_number = sp_num[sp_index];
        species_index = species_number - 1;

        int found = data_in_memory[species_index];
        if (found == FALSE) {
            if (do_all_species == FALSE) {
                fprintf(stderr, "\n    Cannot get data for species #%d!\n", species_number);
                return 2;
            }
            if (first_pass != FALSE) {
                printf("\n    Skipping species #%d.\n", species_number);
            }
            continue;
        }

        int result = preDepartureSpecies(species_number, do_all_species, first_pass);
        if (result != 0) {
            fprintf(stderr, "error: unable to process pre-departure errors for species #%d\n", species_number);
            return result;
        }
    }

    return 0;
}


int preDepartureSpecies(int spNo, int do_all_species, int first_pass) {
    species_number = spNo;
    species_index = species_number - 1;

    species = &spec_data[species_index];
    nampla_base = namp_data[species_index];
    ship_base = ship_data[species_index];

    /* Open orders file for this species. */
    char filename[128];
    sprintf(filename, "sp%02d.ord", species_number);
    input_file = fopen(filename, "r");
    if (input_file == NULL) {
        if (do_all_species == FALSE) {
            fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
            return 2;
        }
        if (first_pass != FALSE) {
            printf("\n    No orders for species #%d.\n", species_number);
        }
        return 0;
    }

    end_of_file = FALSE;
    just_opened_file = TRUE;    /* Tell command parser to skip mail header, if any. */

    /* Search for START PRE-DEPARTURE order. */
    int foundStart = FALSE;
    for (int found = FALSE; found == FALSE;) {
        int command = get_command();
        if (command < 0) {
            /* End of file. */
            break;
        } else if (command == MESSAGE) {
            /* Skip MESSAGE text. It may contain a line that starts with "start". */
            for (command = get_command(); command != ZZZ; command = get_command()) {
                if (command < 0) {
                    fprintf(stderr, "WARNING: Unterminated MESSAGE command in file '%s'!\n", filename);
                    fclose(input_file);
                    input_file = NULL;
                    return 0;
                }
            }
        } else if (command == START) {
            /* Get the first three letters of the keyword and convert to upper case. */
            skip_whitespace();

            char keyword[4] = {0, 0, 0, 0};
            for (int i = 0; i < 3; i++) {
                keyword[i] = toupper(*input_line_pointer);
                input_line_pointer++;
            }

            if (strcmp(keyword, "PRE") == 0) {
                found = TRUE;
                foundStart = TRUE;
            }
        }
    }

    if (foundStart == FALSE) {
        if (first_pass != FALSE) {
            printf("\nNo pre-departure orders for species #%d, SP %s.\n", species_number, species->name);
        }
        fclose(input_file);
        input_file = NULL;
        return 0;
    }

    /* Open log file. Use stdout for first pass. */
    log_stdout = FALSE;  /* We will control value of log_file from here. */
    if (first_pass != FALSE) {
        log_file = stdout;
    } else {
        /* Open log file for appending. */
        sprintf(filename, "sp%02d.log", species_number);
        log_file = fopen(filename, "a");
        if (log_file == NULL) {
            perror("preDepartureSpecies:");
            fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
            exit(2);
        }
        log_string("\nPre-departure orders:\n");
    }

    fprintf(stderr, "debug: preDepartureSpecies: SP%02d data_modified %s\n", species_index + 1, data_modified[species_index] ? "true" : "false");
    /* Handle predeparture orders for this species. */
    do_predeparture_orders();
    fprintf(stderr, "debug: preDepartureSpecies: ..%02d data_modified %s\n", species_index + 1, data_modified[species_index] ? "true" : "false");
    fprintf(stderr, "debug: preDepartureSpecies: ..%02d data_modified %s\n", 15, data_modified[15 - 1] ? "true" : "false");

    data_modified[species_index] = TRUE;

    /* If this is the second pass, close the log file. */
    if (first_pass == FALSE) {
        fclose(log_file);
    }

    fclose(input_file);
    input_file = NULL;

    return 0;
}