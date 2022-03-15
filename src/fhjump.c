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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "engine.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "star.h"
#include "stario.h"
#include "planetio.h"
#include "species.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "namplavars.h"
#include "shipvars.h"
#include "transactionio.h"
#include "log.h"
#include "logvars.h"
#include "command.h"
#include "commandvars.h"
#include "jump.h"
#include "do.h"


int main(int argc, char *argv[]) {
    int i, n, found, num_species, sp_num[MAX_SPECIES], sp_index;
    int command, log_file_open, do_all_species;
    char filename[32], species_jumped[MAX_SPECIES], keyword[4];


    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) { rnd(10); }

    /* Get commonly used data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_transaction_data();

    ignore_field_distorters = TRUE;

    /* Check arguments. If an argument is -p, then do two passes. In the
	first pass, display results and prompt the GM, allowing him to
	abort if necessary before saving results to disk. All other
	arguments must be species numbers. If no species numbers are
	specified, then do all species. */
    num_species = 0;
    first_pass = FALSE;
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            first_pass = TRUE;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else {
            n = atoi(argv[i]);
            if (n < 1 || n > galaxy.num_species) {
                fprintf(stderr,
                        "\n    '%s' is not a valid argument!\n", argv[i]);
                exit(-1);
            }
            sp_num[num_species++] = n;
        }
    }

    if (num_species == 0) {
        num_species = galaxy.num_species;
        for (i = 0; i < num_species; i++) {
            sp_num[i] = i + 1;
        }
        do_all_species = TRUE;
    } else {
        do_all_species = FALSE;
    }

    /* For these commands, do not display age or landed/orbital status
	of ships. */
    truncate_name = TRUE;
    log_stdout = FALSE;  /* We will control value of log_file from here. */

    /* Initialize array that will indicate which species provided jump
	orders. If ships of a species withdrew or were forced from combat
	and there were no jump orders for that species, then combat jumps
	will not take place. This array will allow us to handle them
	separately. */
    for (i = 0; i < galaxy.num_species; i++) {
        species_jumped[i] = FALSE;
    }

    /* Two passes through all orders will be done. The first pass will
	check for errors and abort if any are found. Results will be written
	to disk only on the second pass. */

    start_pass:

    if (first_pass) { printf("\nStarting first pass...\n\n"); }

    get_species_data();
    get_star_data();
    get_planet_data();

    /* Main loop. For each species, take appropriate action. */
    for (sp_index = 0; sp_index < num_species; sp_index++) {
        species_number = sp_num[sp_index];

        found = data_in_memory[species_number - 1];
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

        species = &spec_data[species_number - 1];
        nampla_base = namp_data[species_number - 1];
        ship_base = ship_data[species_number - 1];

        /* Open orders file for this species. */
        sprintf(filename, "sp%02d.ord", species_number);
        input_file = fopen(filename, "r");
        if (input_file == NULL) {
            if (do_all_species) {
                if (first_pass) { printf("\n    No orders for species #%d.\n", species_number); }
                continue;
            } else {
                fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
                exit(-1);
            }
        }

        /* Open log file. Use stdout for first pass. */
        if (first_pass) {
            log_file = stdout;
        } else {
            /* Open log file for appending. */
            sprintf(filename, "sp%02d.log", species_number);
            log_file = fopen(filename, "a");
            if (log_file == NULL) {
                fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
                exit(-1);
            }
        }

        end_of_file = FALSE;

        just_opened_file = TRUE;    /* Tell command parser to skip mail header,
						if any. */
        find_start:

        /* Search for START JUMPS order. */
        found = FALSE;
        while (!found) {
            command = get_command();
            if (command == MESSAGE) {
                /* Skip MESSAGE text. It may contain a line that starts
                    with "start". */
                while (TRUE) {
                    command = get_command();
                    if (command < 0) {
                        fprintf(stderr,
                                "WARNING: Unterminated MESSAGE command in file %s!\n",
                                filename);
                        break;
                    }

                    if (command == ZZZ) { goto find_start; }
                }
            }

            if (command < 0) {
                break;
            }        /* End of file. */

            if (command != START) {
                continue;
            }

            /* Get the first three letters of the keyword and convert to
            upper case. */
            skip_whitespace();
            for (i = 0; i < 3; i++) {
                keyword[i] = toupper(*input_line_pointer);
                ++input_line_pointer;
            }
            keyword[3] = '\0';

            if (strcmp(keyword, "JUM") == 0) { found = TRUE; }
        }

        if (!found) {
            if (first_pass) {
                printf("\nNo jump orders for species #%d, SP %s.\n",
                       species_number, species->name);
            }
            goto done_orders;
        }

        /* Handle jump orders for this species. */
        log_string("\nJump orders:\n");
        do_jump_orders();
        species_jumped[species_number - 1] = TRUE;
        data_modified[species_number - 1] = TRUE;

        done_orders:

        fclose(input_file);

        /* Take care of any ships that withdrew or were forced to jump during
            combat. */
        ship = ship_base;
        for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
            if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
                do_JUMP_command(TRUE, FALSE);
            }
            ++ship;
        }

        /* If this is the second pass, close the log file. */
        if (!first_pass) { fclose(log_file); }
    }

    if (first_pass) {
        printf("\nFinal chance to abort safely!\n");
        gamemaster_abort_option();
        first_pass = FALSE;
        free_species_data();
        free(star_base);    /* In case data was modified. */
        free(planet_base);    /* In case data was modified. */

        printf("\nStarting second pass...\n\n");

        goto start_pass;
    }

    no_jump_orders:

    /* Take care of any ships that withdrew from combat but were not
	handled above because no jump orders were received for species. */
    log_stdout = TRUE;
    log_file_open = FALSE;
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        if (species_jumped[species_number - 1]) { continue; }
        if (!data_in_memory[species_number - 1]) { continue; }

        species = &spec_data[species_number - 1];
        nampla_base = namp_data[species_number - 1];
        ship_base = ship_data[species_number - 1];

        ship = ship_base;
        for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
            if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
                if (!log_file_open) {
                    sprintf(filename, "sp%02d.log", species_number);
                    log_file = fopen(filename, "a");
                    if (log_file == NULL) {
                        fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
                        exit(-1);
                    }
                    log_file_open = TRUE;
                    log_string("\nWithdrawals and forced jumps during combat:\n");
                }

                do_JUMP_command(TRUE, FALSE);
            }
            ++ship;
        }

        data_modified[species_number - 1] = log_file_open;

        if (log_file_open) {
            fclose(log_file);
            log_file_open = FALSE;
        }
    }

    save_species_data();
    save_transaction_data();
    if (star_data_modified) { save_star_data(); }
    if (planet_data_modified) { save_planet_data(); }
    free_species_data();
    free(star_base);
    free(planet_base);
    exit(0);
}


