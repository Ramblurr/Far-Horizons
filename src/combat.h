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

#ifndef FAR_HORIZONS_COMBAT_H
#define FAR_HORIZONS_COMBAT_H

#include "ship.h"
#include "location.h"

#define MAX_BATTLES        50  /* Maximum number of battle locations for all players. */
#define MAX_SHIPS          200 /* Maximum number of ships at a single battle. */
#define MAX_ENGAGE_OPTIONS 20  /* Maximum number of engagement options that a player may specify for a single battle. */

/* Types of combatants. */
#define SHIP            1
#define NAMPLA          2
#define GENOCIDE_NAMPLA 3
#define BESIEGED_NAMPLA 4

/* Types of special targets. */
#define TARGET_WARSHIPS   1
#define TARGET_TRANSPORTS 2
#define TARGET_STARBASES  3
#define TARGET_PDS        4

/* Types of actions. */
#define DEFENSE_IN_PLACE    0
#define DEEP_SPACE_DEFENSE  1
#define PLANET_DEFENSE      2
#define DEEP_SPACE_FIGHT    3
#define PLANET_ATTACK       4
#define PLANET_BOMBARDMENT  5
#define GERM_WARFARE        6
#define SIEGE               7

/* Special types. */
#define NON_COMBATANT       1

struct battle_data {
    char x, y, z, num_species_here;
    char spec_num[MAX_SPECIES];
    char summary_only[MAX_SPECIES];
    char transport_withdraw_age[MAX_SPECIES];
    char warship_withdraw_age[MAX_SPECIES];
    char fleet_withdraw_percentage[MAX_SPECIES];
    char haven_x[MAX_SPECIES];
    char haven_y[MAX_SPECIES];
    char haven_z[MAX_SPECIES];
    char special_target[MAX_SPECIES];
    char hijacker[MAX_SPECIES];
    char can_be_surprised[MAX_SPECIES];
    char enemy_mine[MAX_SPECIES][MAX_SPECIES];
    char num_engage_options[MAX_SPECIES];
    char engage_option[MAX_SPECIES][MAX_ENGAGE_OPTIONS];
    char engage_planet[MAX_SPECIES][MAX_ENGAGE_OPTIONS];
    long ambush_amount[MAX_SPECIES];
};

struct action_data {
    int num_units_fighting;
    int fighting_species_index[MAX_SHIPS];
    int num_shots[MAX_SHIPS];
    int shots_left[MAX_SHIPS];
    long weapon_damage[MAX_SHIPS];
    long shield_strength[MAX_SHIPS];
    long shield_strength_left[MAX_SHIPS];
    long original_age_or_PDs[MAX_SHIPS];
    long bomb_damage[MAX_SHIPS];
    char surprised[MAX_SHIPS];
    char unit_type[MAX_SHIPS];
    char *fighting_unit[MAX_SHIPS];
};

void auto_enemy(int traitor_species_number, int betrayed_species_number);

void bad_argument(void);

void bad_coordinates(void);

void bad_species(void);

void battle_error(int species_number);

// combat returns TRUE if planet, species, and transaction data should be saved
int combat(int default_summary, int do_all_species, int num_species, int *sp_num, char **sp_name, sp_loc_data_t *locations_base);

void consolidate_option(char option, char location);

int disbanded_species_ship(int species_index, struct ship_data *sh);

void do_ambush(int ambushing_species_index, struct battle_data *bat);

void do_battle(struct battle_data *bat);

void do_bombardment(int unit_index, struct action_data *act);

void do_germ_warfare(int attacking_species, int defending_species, int defender_index, struct battle_data *bat,
                     struct action_data *act);

int do_round(char option, int round_number, struct battle_data *bat, struct action_data *act);

void do_siege(struct battle_data *bat, struct action_data *act);

int fighting_params(char option, char location, struct battle_data *bat, struct action_data *act);

int forced_jump_units_used(int attacker_index, int defender_index, int *total_shots, struct battle_data *bat,
                           struct action_data *act);

void regenerate_shields(struct action_data *act);

void withdrawal_check(struct battle_data *bat, struct action_data *act);

// globals. ugh.

extern struct battle_data *battle_base;
extern int strike_phase;

#endif //FAR_HORIZONS_COMBAT_H
