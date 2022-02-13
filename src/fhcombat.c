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

// Terminology: A "battle" consists of one or more "actions", and each
// action can take place at a different location in a star system.
// In turn, each action can be broken down into one or more "rounds",
// in which actual combat occurs.
//
// A battle is defined by a variable of type "struct battle_data",
// and a pointer to a variable of this type, called "bat", is used
// throughout the combat routines.
//
// An action is defined by a variable of type "struct action_data",
// and a pointer to a variable of this type, called "act", is used
// throughout the combat routines.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "engine.h"
#include "combat.h"
#include "galaxy.h"
#include "planet.h"
#include "species.h"
#include "ship.h"
#include "transaction.h"
#include "location.h"

int main(int argc, char *argv[]) {
    // combat.c
    extern struct battle_data *battle_base;
    extern int strike_phase;
    // engine.c
    extern long last_random;
    extern int prompt_gm;
    extern int test_mode;
    extern int verbose_mode;
    // galaxy.c
    extern struct galaxy_data galaxy;
    // location.c
    extern struct sp_loc_data loc[MAX_LOCATIONS];
    // log.c
    extern int log_stdout;
    // species.c
    extern int data_in_memory[MAX_SPECIES];
    extern struct species_data spec_data[MAX_SPECIES];
    // ship.c
    extern struct ship_data *ship_data[MAX_SPECIES];
    extern struct ship_data *ship_base;
    extern struct ship_data *ship;

    int default_summary;
    int do_all_species;
    int i;
    long n;
    int num_species;
    int save = FALSE;
    struct species_data *sp;
    int sp_index;
    int sp_num[MAX_SPECIES];
    char *sp_name[MAX_SPECIES];

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) {
        rnd(10);
    }

    // initialize arrays?
    memset(sp_num, 0, sizeof(sp_num));

    /* Get commonly used data. */
    get_galaxy_data();
    get_planet_data();
    get_transaction_data();
    get_location_data();

    /* Allocate memory for battle data. */
    battle_base = (struct battle_data *) calloc(MAX_BATTLES, sizeof(struct battle_data));
    if (battle_base == NULL) {
        fprintf(stderr, "\nCannot allocate enough memory for battle data!\n\n");
        exit(-1);
    }

    /* Check arguments.
     * If an argument is -s, then set SUMMARY mode for everyone.
     * The default is for players to receive a detailed report of the battles.
     * If an argument is -p, then prompt the GM before saving results;
     * otherwise, operate quietly; i.e, do not prompt GM before saving results
     * and do not display anything except errors.
     * Any additional arguments must be species numbers.
     * If no species numbers are specified, then do all species. */
    num_species = 0;
    default_summary = FALSE;
    prompt_gm = FALSE;
    save = FALSE;
    test_mode = FALSE;
    verbose_mode = FALSE;

    if (strstr(argv[0], "Strike") != NULL) {
        strike_phase = TRUE;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            default_summary = TRUE;
        } else if (strcmp(argv[i], "-p") == 0) {
            prompt_gm = TRUE;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else if (strcmp(argv[i], "-S") == 0) {
            save = TRUE;
        } else {
            n = atoi(argv[i]);
            if (0 < n && n <= galaxy.num_species) {
                sp_num[num_species++] = n;
            }
        }
    }

    log_stdout = prompt_gm;

    if (num_species == 0) {
        num_species = galaxy.num_species;
        for (i = 0; i < num_species; i++) {
            sp_num[i] = i + 1;
        }
        do_all_species = TRUE;
    } else {
        do_all_species = FALSE;
    }

    if (default_summary && prompt_gm) {
        printf("\nSUMMARY mode is in effect for all species.\n\n");
    }

    /* Read in species data and make an uppercase copy of each name for comparison purposes later. Also do some initializations. */
    get_species_data();

    for (sp_index = 0; sp_index < galaxy.num_species; sp_index++) {
        sp_name[sp_index] = calloc(1, 32);
        if (!data_in_memory[sp_index]) {
            /* No longer in game. */
            continue;
        }
        sp = &spec_data[sp_index];
        ship_base = ship_data[sp_index];
        /* Convert name to upper case. */
        for (i = 0; i < 31; i++) {
            sp_name[sp_index][i] = toupper(sp->name[i]);
        }
        for (i = 0; i < sp->num_ships; i++) {
            ship = ship_base + i;
            ship->special = 0;
        }
    }

    combat(do_all_species, num_species, sp_num, sp_name, &loc[0]);

    if (save) {
        save_planet_data();
        save_species_data();
        save_transaction_data();
    }

    free_species_data();

    return 0;
}
