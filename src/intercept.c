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

#include "galaxy.h"
#include "galaxyio.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "shipvars.h"
#include "log.h"
#include "transactionio.h"
#include "intercept.h"

#define MAX_ENEMY_SHIPS    400


int num_intercepts;

intercept_t intercept[MAX_INTERCEPTS];


void handle_intercept(int intercept_index) {
    int i, j, n, num_enemy_ships, alien_index, enemy_index, enemy_num, num_ships_left, array_index, bit_number, is_an_enemy, is_distorted;
    char enemy_number[MAX_ENEMY_SHIPS];
    long bit_mask, cost_to_destroy;
    struct species_data *alien;
    struct ship_data *alien_sh, *enemy_sh, *enemy_ship[MAX_ENEMY_SHIPS];


    /* Make a list of all enemy ships that jumped into this system. */
    num_enemy_ships = 0;
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
        if (!data_in_memory[alien_index]) { continue; }

        if (species_number == alien_index + 1) { continue; }

        /* Is it an enemy species? */
        array_index = (alien_index) / 32;
        bit_number = (alien_index) % 32;
        bit_mask = 1 << bit_number;
        if (species->enemy[array_index] & bit_mask) {
            is_an_enemy = TRUE;
        } else {
            is_an_enemy = FALSE;
        }

        /* Find enemy ships, if any, that jumped to this location. */
        alien = &spec_data[alien_index];
        alien_sh = ship_data[alien_index] - 1;
        for (i = 0; i < alien->num_ships; i++) {
            ++alien_sh;

            if (alien_sh->pn == 99) { continue; }

            /* Did it jump this turn? */
            if (!alien_sh->just_jumped) { continue; }
            if (alien_sh->just_jumped == 50) { continue; /* Ship MOVEd. */}

            /* Did it enter this star system? */
            if (alien_sh->x != intercept[intercept_index].x) { continue; }
            if (alien_sh->y != intercept[intercept_index].y) { continue; }
            if (alien_sh->z != intercept[intercept_index].z) { continue; }

            /* Is it field-distorted? */
            if (alien_sh->item_quantity[FD] == alien_sh->tonnage) {
                is_distorted = TRUE;
            } else {
                is_distorted = FALSE;
            }

            if (!is_an_enemy && !is_distorted) { continue; }

            /* This is an enemy ship that just jumped into the system. */
            if (num_enemy_ships == MAX_ENEMY_SHIPS) {
                fprintf(stderr, "\n\tERROR! Array overflow in handle_intercept!\n\n");
                exit(-1);
            }
            enemy_number[num_enemy_ships] = alien_index + 1;
            enemy_ship[num_enemy_ships] = alien_sh;
            ++num_enemy_ships;
        }
    }

    if (num_enemy_ships == 0) { return;    /* Nothing to intercept. */}

    num_ships_left = num_enemy_ships;
    while (num_ships_left > 0) {
        /* Select ship for interception. */
        enemy_index = rnd(num_enemy_ships) - 1;
        if (enemy_ship[enemy_index] == NULL) { continue;    /* We already did this one. */}
        enemy_num = enemy_number[enemy_index];
        enemy_sh = enemy_ship[enemy_index];

        /* Are there enough funds to destroy this ship? */
        cost_to_destroy = 100L * (long) enemy_sh->tonnage;
        if (enemy_sh->class == TR) { cost_to_destroy /= 10; }
        if (cost_to_destroy > intercept[intercept_index].amount_spent) { break; }

        /* Is the ship too large? Check only if ship did NOT arrive via a natural wormhole. */
        if (enemy_sh->just_jumped != 99) {
            if (enemy_sh->tonnage > 20) { break; }
            if (enemy_sh->class != TR && enemy_sh->tonnage > 5) { break; }
        }

        /* Update funds available. */
        intercept[intercept_index].amount_spent -= cost_to_destroy;

        /* Log the result for current species. */
        log_string("\n! ");
        n = enemy_sh->item_quantity[FD];    /* Show real name. */
        enemy_sh->item_quantity[FD] = 0;
        log_string(ship_name(enemy_sh));
        enemy_sh->item_quantity[FD] = n;

        /* List cargo destroyed. */
        n = 0;
        for (j = 0; j < MAX_ITEMS; j++) {
            if (enemy_sh->item_quantity[j] > 0) {
                if (n++ == 0) {
                    log_string(" (cargo: ");
                } else {
                    log_char(',');
                }
                log_int((int) enemy_sh->item_quantity[j]);
                log_char(' ');
                log_string(item_abbr[j]);
            }
        }
        if (n > 0) { log_char(')'); }

        log_string(", owned by SP ");
        log_string(spec_data[enemy_num - 1].name);
        log_string(", was successfully intercepted and destroyed in sector ");
        log_int(enemy_sh->x);
        log_char(' ');
        log_int(enemy_sh->y);
        log_char(' ');
        log_int(enemy_sh->z);
        log_string(".\n");

        /* Create interspecies transaction so that other player will be notified. */
        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in handle_intercept!\n\n");
            exit(-1);
        }

        n = num_transactions++;
        transaction[n].type = SHIP_MISHAP;
        transaction[n].value = 1;    /* Interception. */
        transaction[n].number1 = enemy_number[enemy_index];
        strcpy(transaction[n].name1, ship_name(enemy_sh));

        delete_ship(enemy_sh);

        enemy_ship[enemy_index] = NULL;    /* Don't select this ship again. */

        --num_ships_left;
    }
}
