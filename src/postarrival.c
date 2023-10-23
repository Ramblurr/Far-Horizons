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
#include "postarrival.h"
#include "command.h"
#include "engine.h"
#include "shipvars.h"
#include "enginevars.h"
#include "speciesvars.h"
#include "logvars.h"
#include "commandvars.h"
#include "do.h"
#include "log.h"
#include "galaxyio.h"
#include "stario.h"
#include "planetio.h"
#include "transactionio.h"
#include "speciesio.h"
#include "namplavars.h"


void do_postarrival_orders(void) {
    int command;

    if (first_pass) {
        printf("\nStart of post-arrival orders for species #%d, SP %s...\n", species_number, species->name);
    }

    /* For these commands, do not display age or landed/orbital status of ships. */
    truncate_name = TRUE;

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
                printf("End of post-arrival orders for species #%d, SP %s.\n", species_number, species->name);
            }
            if (first_pass) { gamemaster_abort_option(); }
            break;            /* END for this species. */
        }

        switch (command) {
            case ALLY:
                do_ALLY_command();
                break;
            case AUTO:
                species->auto_orders = TRUE;
                log_string("    An AUTO order was executed.\n");
                break;
            case DEEP:
                do_DEEP_command();
                break;
            case DESTROY:
                do_DESTROY_command();
                break;
            case ENEMY:
                do_ENEMY_command();
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
                do_SCAN_command();
                break;
            case SEND:
                do_SEND_command();
                break;
            case TEACH:
                do_TEACH_command();
                break;
            case TECH:
                // do_TECH_command();
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid post-arrival command.\n");
                break;
            case TRANSFER:
                do_TRANSFER_command();
                break;
            case TELESCOPE:
                do_TELESCOPE_command();
                break;
            case TERRAFORM:
                do_TERRAFORM_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid post-arrival command.\n");
        }
    }
}


int postArrivalCommand(int argc, char *argv[]) {
    int num_species, sp_num[MAX_SPECIES], sp_index, command, do_all_species;

    /* Get commonly used data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_transaction_data();

    ignore_field_distorters = TRUE;

    /* Check arguments.
     * If an argument is -p, then do two passes.
     * In the first pass, display results and prompt the GM, allowing him
     * to abort if necessary before saving results to disk.
     * All other arguments must be species numbers.
     * If no species numbers are specified, then do all species. */
    num_species = 0;
    first_pass = FALSE;
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            first_pass = TRUE;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else {
            int n = atoi(argv[i]);
            if (n < 1 || n > galaxy.num_species) {
                fprintf(stderr, "\n    '%s' is not a valid argument!\n", argv[i]);
                exit(2);
            }
            sp_num[num_species] = n;
            num_species++;
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

    start_pass:

    if (first_pass) {
        printf("\nStarting first pass...\n\n");
    }

    get_star_data();
    get_planet_data();
    get_species_data();

    /* Main loop. For each species, take appropriate action. */
    for (sp_index = 0; sp_index < num_species; sp_index++) {
        species_number = sp_num[sp_index];
        species_index = species_number - 1;

        int found = data_in_memory[species_index];
        if (!found) {
            if (do_all_species) {
                if (first_pass) { printf("\n    Skipping species #%d.\n", species_number); }
                continue;
            } else {
                fprintf(stderr, "\n    Cannot get data for species #%d!\n",
                        species_number);
                exit(-1);
            }
        }

        species = &spec_data[species_index];
        nampla_base = namp_data[species_index];
        ship_base = ship_data[species_index];

        /* Do some initializations. */
        species->auto_orders = FALSE;

        /* Open orders file for this species. */
        char filename[128];
        sprintf(filename, "sp%02d.ord", species_number);
        input_file = fopen(filename, "r");
        if (input_file == NULL) {
            if (do_all_species) {
                if (first_pass) {
                    printf("\n    No orders for species #%d.\n", species_number);
                }
                continue;
            } else {
                fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
                exit(-1);
            }
        }

        end_of_file = FALSE;

        /* Tell command parser to skip mail header, if any. */
        just_opened_file = TRUE;

        find_start:

        /* Search for START POST-ARRIVAL order. */
        found = FALSE;
        while (!found) {
            command = get_command();
            if (command == MESSAGE) {
                /* Skip MESSAGE text. It may contain a line that starts with "start". */
                while (TRUE) {
                    command = get_command();
                    if (command < 0) {
                        fprintf(stderr, "WARNING: Unterminated MESSAGE command in file %s!\n", filename);
                        break;
                    }

                    if (command == ZZZ) {
                        goto find_start;
                    }
                }
            }

            if (command < 0) {
                /* End of file. */
                break;
            }

            if (command != START) {
                continue;
            }

            /* Get the first three letters of the keyword and convert to upper case. */
            skip_whitespace();
            char keyword[4] = {0, 0, 0, 0};
            for (int i = 0; i < 3 && *input_line_pointer != 0; i++) {
                keyword[i] = (char) toupper(*input_line_pointer);
                input_line_pointer++;
            }

            if (strcmp(keyword, "POS") == 0) {
                found = TRUE;
            }
        }

        if (!found) {
            if (first_pass) {
                printf("\nNo post-arrival orders for species #%d, SP %s.\n", species_number, species->name);
            }
            goto done_orders;
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
            log_string("\nPost-arrival orders:\n");
        }

        /* For each ship, set dest_z to zero.
         * If a starbase is used as a gravitic telescope, it will be set to non-zero.
         * This will prevent more than one TELESCOPE order per turn per starbase. */
        ship = ship_base;
        for (int i = 0; i < species->num_ships; i++) {
            ship->dest_z = 0;
            ship++;
        }

        /* Handle post-arrival orders for this species. */
        do_postarrival_orders();

        data_modified[species_index] = TRUE;

        /* If this is the second pass, close the log file. */
        if (!first_pass) {
            fclose(log_file);
        }

        done_orders:

        fclose(input_file);
    }

    if (first_pass) {
        printf("\nFinal chance to abort safely!\n");
        gamemaster_abort_option();
        first_pass = FALSE;
        free_species_data();
        /* In case data was modified. */
        if (planet_base != NULL) {
            free(planet_base);
            planet_base = NULL;
        }
        if (star_base != NULL) {
            free(star_base);
            star_base = NULL;
        }

        printf("\nStarting second pass...\n\n");

        goto start_pass;
    }

    save_species_data();
    save_transaction_data();
    if (star_data_modified) {
        save_star_data(star_base, num_stars);
    }
    if (planet_data_modified) {
        save_planet_data(planet_base, num_planets);
    }
    free_species_data();
    free(planet_base);
    planet_base = NULL;
    free(star_base);
    star_base = NULL;

    return 0;
}