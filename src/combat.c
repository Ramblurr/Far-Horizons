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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "engine.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "planet.h"
#include "planetio.h"
#include "species.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "nampla.h"
#include "namplavars.h"
#include "ship.h"
#include "shipvars.h"
#include "item.h"
#include "location.h"
#include "locationio.h"
#include "command.h"
#include "commandvars.h"
#include "transaction.h"
#include "transactionio.h"
#include "log.h"
#include "logvars.h"
#include "combat.h"

int ambush_took_place;
char append_log[MAX_SPECIES];
int attacking_ML;
struct battle_data *battle_base;
struct nampla_data *c_nampla[MAX_SPECIES];
struct ship_data *c_ship[MAX_SPECIES];
struct species_data *c_species[MAX_SPECIES];
char combat_location[1000];
char combat_option[1000];
int deep_space_defense;
int defending_ML;
char field_distorted[MAX_SPECIES];
int first_battle = TRUE;
short germ_bombs_used[MAX_SPECIES][MAX_SPECIES];
char make_enemy[MAX_SPECIES][MAX_SPECIES];
int num_combat_options;
int strike_phase = FALSE;
char x_attacked_y[MAX_SPECIES][MAX_SPECIES];


/* This routine will find all species that have declared alliance with both a traitor and betrayed species.
 * It will then set a flag to indicate that their allegiance should be changed from ALLY to ENEMY. */
void auto_enemy(int traitor_species_number, int betrayed_species_number) {
    int traitor_array_index = (traitor_species_number - 1) / 32;
    long traitor_bit_mask = 1 << ((traitor_species_number - 1) % 32);

    int betrayed_array_index = (betrayed_species_number - 1) / 32;
    long betrayed_bit_mask = 1 << ((betrayed_species_number - 1) % 32);

    for (int species_index = 0; species_index < galaxy.num_species; species_index++) {
        if ((spec_data[species_index].ally[traitor_array_index] & traitor_bit_mask) == 0) {
            continue;
        }
        if ((spec_data[species_index].ally[betrayed_array_index] & betrayed_bit_mask) == 0) {
            continue;
        }
        if ((spec_data[species_index].contact[traitor_array_index] & traitor_bit_mask) == 0) {
            continue;
        }
        if ((spec_data[species_index].contact[betrayed_array_index] & betrayed_bit_mask) == 0) {
            continue;
        }
        make_enemy[species_index][traitor_species_number - 1] = betrayed_species_number;
    }
}

void bad_argument(void) {
    fprintf(log_file, "!!! Order ignored:\n");
    fprintf(log_file, "!!! %s", input_line);
    fprintf(log_file, "!!! Invalid argument in command.\n");
}

void bad_coordinates(void) {
    fprintf(log_file, "!!! Order ignored:\n");
    fprintf(log_file, "!!! %s", input_line);
    fprintf(log_file, "!!! Invalid coordinates in command.\n");
}

void bad_species(void) {
    fprintf(log_file, "!!! Order ignored:\n");
    fprintf(log_file, "!!! %s", input_line);
    fprintf(log_file, "!!! Invalid species name!\n");
}

void battle_error(int species_number) {
    fprintf(log_file, "!!! Order ignored:\n");
    fprintf(log_file, "!!! %s", input_line);
    fprintf(log_file, "!!! Missing BATTLE command!\n");
}

// combat returns TRUE if planet, species, and transaction data should be saved
int combat(int default_summary, int do_all_species, int num_species, int *sp_num, char **sp_name,
           sp_loc_data_t *locations_base) {
    int save = TRUE;
    int i;
    int j;
    int k;
    int found;
    int command;
    int species_number;
    int sp_index;
    int num_battles;
    int location_index;
    int species_fd;
    int num_enemies;
    int battle_index;
    int option_index;
    int arg_index;
    int at_number;
    int at_index;
    int really_hidden;
    int num_pls;
    int pl_num[9];
    int enemy_word_number;
    int enemy_bit_number;
    int log_open;
    int distorted_name;
    int best_score;
    int next_best_score;
    int best_species_index;
    int betrayed_species_number;
    int name_length;
    int minimum_score;
    int first_line;
    long n;
    long n_bytes;
    long enemy_mask;
    char x;
    char y;
    char z;
    char option;
    char filename[32];
    char keyword[4];
    char answer[16];
    char log_line[256];
    char *temp_ptr;
    FILE *temp_species_log;
    FILE *species_log;
    struct species_data *sp;
    struct species_data *at_sp;
    struct nampla_data *namp;
    struct nampla_data *at_namp;
    struct ship_data *sh;
    struct ship_data *at_sh;
    struct battle_data *bat;
    struct sp_loc_data *location;

    /* Main loop. For each species, take appropriate action. */
    num_battles = 0;
    for (arg_index = 0; arg_index < num_species; arg_index++) {
        species_number = sp_num[arg_index];
        if (!data_in_memory[species_number - 1]) {
            continue;
        }

        sp = &spec_data[species_number - 1];

        /* The following two items are needed by get_ship(). */
        species = sp;
        ship_base = ship_data[species_number - 1];

        /* Open orders file for this species. */
        sprintf(filename, "sp%02d.ord", species_number);
        input_file = fopen(filename, "r");
        if (input_file == NULL) {
            if (do_all_species) {
                if (prompt_gm) {
                    printf("\nNo orders for species #%d, SP %s.\n", species_number, sp->name);
                }
                continue;
            } else {
                fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
                exit(-1);
            }
        }

        end_of_file = FALSE;

        just_opened_file = TRUE;    /* Tell command parser to skip mail header, if any. */
        find_start:

        /* Search for START COMBAT order. */
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

            for (i = 0; i < 3; i++) {
                keyword[i] = toupper(*input_line_pointer);
                input_line_pointer++;
            }
            keyword[3] = '\0';

            if (strike_phase) {
                if (strcmp(keyword, "STR") == 0) {
                    found = TRUE;
                }
            } else {
                if (strcmp(keyword, "COM") == 0) {
                    found = TRUE;
                }
            }
        }

        if (found) {
            if (prompt_gm) {
                if (strike_phase) {
                    printf("\nStrike orders for species #%d, SP %s...\n", species_number, sp->name);
                } else {
                    printf("\nCombat orders for species #%d, SP %s...\n", species_number, sp->name);
                }
            }
        } else {
            if (prompt_gm) {
                if (strike_phase) {
                    printf("\nNo strike orders for species #%d, SP %s...\n", species_number, sp->name);
                } else {
                    printf("\nNo combat orders for species #%d, SP %s...\n", species_number, sp->name);
                }
            }
            goto done_orders;
        }

        /* Open temporary log file for appending. */
        sprintf(filename, "sp%02d.temp.log", species_number);
        log_file = fopen(filename, "a");
        if (log_file == NULL) {
            fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
            exit(-1);
        }

        append_log[species_number - 1] = TRUE;

        log_stdout = FALSE;
        if (strike_phase) {
            log_string("\nStrike orders:\n");
        } else {
            log_string("\nCombat orders:\n");
        }
        log_stdout = prompt_gm;

        /* Parse all combat commands for this species and save results for later use. */
        battle_index = -1;
        while (TRUE) {
            command = get_command();
            if (end_of_file) {
                break;
            }

            if (command == END) {
                break;
            }

            if (command == BATTLE) {
                num_enemies = 0;    /* No enemies specified yet. */

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                x = value;

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                y = value;

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                z = value;

                /* Make sure that species is present at battle location. */
                found = FALSE;
                location = locations_base - 1;
                for (i = 0; i < num_locs; i++) {
                    location++;
                    if (location->s != species_number) {
                        continue;
                    }
                    if (location->x != x) {
                        continue;
                    }
                    if (location->y != y) {
                        continue;
                    }
                    if (location->z != z) {
                        continue;
                    }

                    found = TRUE;
                    break;
                }
                if (!found) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Your species is not at this location!\n");
                    continue;
                }

                log_string("  A battle order was issued for sector ");
                log_int(x);
                log_char(' ');
                log_int(y);
                log_char(' ');
                log_int(z);
                log_string(".\n");

                /* Add coordinates to list if not already there. */
                found = FALSE;
                bat = battle_base;
                for (i = 0; i < num_battles; i++) {
                    if (x == bat->x && y == bat->y && z == bat->z) {
                        found = TRUE;
                        battle_index = i;
                        break;
                    }

                    bat++;
                }

                if (!found) {
                    /* This is a new battle location. */
                    if (num_battles == MAX_BATTLES) {
                        fprintf(stderr, "\n\n\tMAX_BATTLES exceeded! Edit file 'combat.h' and re-compile!\n\n");
                        exit(-1);
                    }
                    battle_index = num_battles;
                    sp_index = 0;
                    bat->x = x;
                    bat->y = y;
                    bat->z = z;
                    bat->spec_num[0] = species_number;
                    bat->special_target[0] = 0;  /* Default. */
                    bat->transport_withdraw_age[0] = 0;  /* Default. */
                    bat->warship_withdraw_age[0] = 100;  /* Default. */
                    bat->fleet_withdraw_percentage[0] = 100;  /* Default. */
                    bat->haven_x[0] = 127;
                    /* 127 means not yet specified. */
                    bat->engage_option[sp_index][0] = DEFENSE_IN_PLACE;
                    bat->num_engage_options[0] = 1;
                    bat->can_be_surprised[0] = FALSE;
                    bat->hijacker[0] = FALSE;
                    bat->summary_only[0] = default_summary;
                    bat->num_species_here = 1;
                    for (i = 0; i < MAX_SPECIES; i++) {
                        bat->enemy_mine[0][i] = 0;
                    }
                    num_battles++;
                } else {
                    /* Add another species to existing battle location. */
                    sp_index = bat->num_species_here;
                    bat->spec_num[sp_index] = species_number;
                    bat->special_target[sp_index] = 0;  /* Default. */
                    bat->transport_withdraw_age[sp_index] = 0;  /* Default. */
                    bat->warship_withdraw_age[sp_index] = 100;  /* Default. */
                    bat->fleet_withdraw_percentage[sp_index] = 100;  /* Default. */
                    bat->haven_x[sp_index] = 127;
                    /* 127 means not yet specified. */
                    bat->engage_option[sp_index][0] = DEFENSE_IN_PLACE;
                    bat->num_engage_options[sp_index] = 1;
                    bat->can_be_surprised[sp_index] = FALSE;
                    bat->hijacker[sp_index] = FALSE;
                    bat->summary_only[sp_index] = default_summary;
                    bat->num_species_here++;
                    for (i = 0; i < MAX_SPECIES; i++) {
                        bat->enemy_mine[sp_index][i] = 0;
                    }
                }
                continue;
            }

            if (command == SUMMARY) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                bat->summary_only[sp_index] = TRUE;

                log_string("    Summary mode was specified.\n");

                continue;
            }

            if (command == WITHDRAW) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                if (get_value() == 0 || value < 0 || value > 100) {
                    bad_argument();
                    continue;
                }
                i = value;
                bat->transport_withdraw_age[sp_index] = i;

                if (get_value() == 0 || value < 0 || value > 100) {
                    bad_argument();
                    continue;
                }
                j = value;
                bat->warship_withdraw_age[sp_index] = j;

                if (get_value() == 0 || value < 0 || value > 100) {
                    bad_argument();
                    continue;
                }
                k = value;
                bat->fleet_withdraw_percentage[sp_index] = k;

                log_string("    Withdrawal conditions were set to ");
                log_int(i);
                log_char(' ');
                log_int(j);
                log_char(' ');
                log_int(k);
                log_string(".\n");

                continue;
            }

            if (command == HAVEN) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                i = value;
                bat->haven_x[sp_index] = value;

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                j = value;
                bat->haven_y[sp_index] = value;

                if (get_value() == 0) {
                    bad_coordinates();
                    continue;
                }
                k = value;
                bat->haven_z[sp_index] = value;

                log_string("    Haven location set to sector ");
                log_int(i);
                log_char(' ');
                log_int(j);
                log_char(' ');
                log_int(k);
                log_string(".\n");

                continue;
            }

            if (command == ENGAGE) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                option_index = bat->num_engage_options[sp_index];
                if (option_index >= MAX_ENGAGE_OPTIONS) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Too many ENGAGE orders!\n");
                    continue;
                }

                if (get_value() == 0 || value < 0 || value > 7) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Invalid ENGAGE option!\n");
                    continue;
                }
                option = value;

                if (strike_phase && (option > 4)) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Invalid ENGAGE option for strike phase!\n");
                    continue;
                }

                bat->engage_option[sp_index][option_index] = option;

                /* Get planet to attack/defend, if any. */
                if (option == PLANET_DEFENSE || (option >= PLANET_ATTACK && option <= SIEGE)) {
                    if (get_value() == 0) {
                        fprintf(log_file, "!!! Order ignored:\n");
                        fprintf(log_file, "!!! %s", input_line);
                        fprintf(log_file, "!!! Missing planet argument in ENGAGE order!\n");
                        continue;
                    }

                    if (value < 1 || value > 9) {
                        fprintf(log_file, "!!! Order ignored:\n");
                        fprintf(log_file, "!!! %s", input_line);
                        fprintf(log_file, "!!! Invalid planet argument in ENGAGE order!\n");
                        continue;
                    }

                    bat->engage_planet[sp_index][option_index] = value;
                } else {
                    value = 0;
                    bat->engage_planet[sp_index][option_index] = 0;
                }

                bat->num_engage_options[sp_index]++;

                log_string("    Engagement order ");
                log_int(option);
                if (value != 0) {
                    log_char(' ');
                    log_long(value);
                }
                log_string(" was specified.\n");

                continue;
            }

            if (command == HIDE) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }
                if (!get_ship()) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Invalid or missing ship name!\n");
                    continue;
                }
                if (ship->status != ON_SURFACE) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Ship must be landed to HIDE!\n");
                    continue;
                }
                ship->special = NON_COMBATANT;
                log_string("    ");
                log_string(ship_name(ship));
                log_string(" will attempt to stay out of the battle.\n");
                continue;
            }

            if (command == TARGET) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                if (get_value() == 0 || value < 1 || value > 4) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", input_line);
                    fprintf(log_file, "!!! Invalid TARGET option!\n");
                    continue;
                }
                bat->special_target[sp_index] = value;

                log_string("    Strategic target ");
                log_long(value);
                log_string(" was specified.\n");

                continue;
            }

            if (command == ATTACK || command == HIJACK) {
                if (battle_index < 0) {
                    battle_error(species_number);
                    continue;
                }

                if (command == HIJACK) {
                    bat->hijacker[sp_index] = TRUE;
                }

                /* Check if this is an order to attack all declared enemies. */
                if (get_value() && value == 0) {
                    for (i = 0; i < galaxy.num_species; i++) {
                        if (species_number == i + 1) {
                            continue;
                        }
                        if (!data_in_memory[i]) {
                            continue;
                        }

                        enemy_word_number = i / 32;
                        enemy_bit_number = i % 32;
                        enemy_mask = 1 << enemy_bit_number;

                        if (sp->enemy[enemy_word_number] & enemy_mask) {
                            if (num_enemies == MAX_SPECIES) {
                                fprintf(stderr, "\n\n\tToo many enemies to ATTACK or HIJACK!\n\n");
                                exit(-1);
                            }
                            if (command == HIJACK) {
                                bat->enemy_mine[sp_index][num_enemies] = -(i + 1);
                            } else {
                                bat->enemy_mine[sp_index][num_enemies] = i + 1;
                            }
                            num_enemies++;
                        }
                    }
                    if (command == HIJACK) {
                        log_string("    An order was given to hijack all declared enemies.\n");
                    } else {
                        log_string("    An order was given to attack all declared enemies.\n");
                    }
                    continue;
                }

                if (num_enemies == MAX_SPECIES) {
                    fprintf(stderr, "\n\n\tToo many enemies to ATTACK or HIJACK!\n\n");
                    exit(-1);
                }

                /* Set 'n' to the species number of the named enemy. */
                temp_ptr = input_line_pointer;
                if (get_class_abbr() != SPECIES_ID) {
                    /* Check if SP abbreviation was accidentally omitted. */
                    if (isdigit(*temp_ptr)) {
                        input_line_pointer = temp_ptr;
                    } else if (*input_line_pointer != ' ' && *input_line_pointer != '\t') {
                        input_line_pointer = temp_ptr;
                    }
                }

                distorted_name = FALSE;
                if (get_value() && !isalpha(*input_line_pointer) && ((n = undistorted((int) value)) != 0)) {
                    distorted_name = TRUE;
                    goto att1;
                } else if (get_name() < 5) {
                    bad_species();
                    continue;
                }

                /* Check for spelling error. */
                best_score = -9999;
                next_best_score = -9999;
                for (i = 0; i < galaxy.num_species; i++) {
                    if (*sp_name[i] == '\0') {
                        continue;
                    }
                    n = agrep_score(sp_name[i], upper_name);
                    if (n > best_score) {
                        best_score = n;
                        best_species_index = i;
                    } else if (n > next_best_score) {
                        next_best_score = n;
                    }
                }

                name_length = strlen(sp_name[best_species_index]);
                minimum_score = name_length - ((name_length / 7) + 1);

                if (best_score < minimum_score || best_score == next_best_score) {
                    /* Score too low or another name with equal score. */
                    bad_species();
                    continue;
                }

                n = best_species_index + 1;

                att1:

                /* Make sure the named species is at the battle location. */
                found = FALSE;
                location = locations_base - 1;
                for (i = 0; i < num_locs; i++) {
                    location++;
                    if (location->s != n) {
                        continue;
                    }
                    if (location->x != bat->x) {
                        continue;
                    }
                    if (location->y != bat->y) {
                        continue;
                    }
                    if (location->z != bat->z) {
                        continue;
                    }

                    found = TRUE;
                    break;
                }

                /* Save species number temporarily in enemy_mine array. */
                if (found) {
                    if (command == HIJACK) {
                        bat->enemy_mine[sp_index][num_enemies] = -n;
                    } else {
                        bat->enemy_mine[sp_index][num_enemies] = n;
                    }
                    num_enemies++;
                }

                if (command == HIJACK) {
                    log_string("    An order was given to hijack SP ");
                } else {
                    log_string("    An order was given to attack SP ");
                }

                if (distorted_name) {
                    log_int(distorted((int) n));
                } else {
                    log_string(spec_data[n - 1].name);
                }
                log_string(".\n");

                continue;
            }

            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid combat command.\n");
        }

        fclose(log_file);

        done_orders:

        fclose(input_file);
    }

    /* Check each battle.  If a species specified a BATTLE command but did not specify any engage options, then add a DEFENSE_IN_PLACE option. */
    bat = battle_base;
    for (battle_index = 0; battle_index < num_battles; battle_index++) {
        for (i = 0; i < bat->num_species_here; i++) {
            if (bat->num_engage_options[i] == 0) {
                bat->num_engage_options[i] = 1;
                bat->engage_option[i][0] = DEFENSE_IN_PLACE;
            }
        }

        bat++;
    }

    /* Initialize make_enemy array. */
    for (i = 0; i < galaxy.num_species; i++) {
        for (j = 0; j < galaxy.num_species; j++) {
            make_enemy[i][j] = 0;
        }
    }

    /* Check each battle location. If a species is at the location
    but has no combat orders, add it to the list of species at that
    battle, and apply defaults. After all species are accounted for
    at the current battle location, do battle. */
    bat = battle_base - 1;
    for (battle_index = 0; battle_index < num_battles; battle_index++) {
        bat++;

        x = bat->x;
        y = bat->y;
        z = bat->z;

        /* Check file 'locations.dat' for other species at this location. */
        location = locations_base - 1;
        for (location_index = 0; location_index < num_locs; location_index++) {
            location++;
            if (location->x != x) {
                continue;
            }
            if (location->y != y) {
                continue;
            }
            if (location->z != z) {
                continue;
            }

            /* Check if species is already accounted for. */
            found = FALSE;
            species_number = location->s;
            for (sp_index = 0; sp_index < bat->num_species_here; sp_index++) {
                if (bat->spec_num[sp_index] == species_number) {
                    found = TRUE;
                    break;
                }
            }

            if (found) {
                continue;
            }

            /* Species is present but did not give any combat orders.
            This species will be included in the battle ONLY if it has
            ships in deep space or in orbit or if it has an unhidden,
            populated planet in this sector or if it has a hidden
            planet that is being explicitly attacked. */
            found = FALSE;

            sp = &spec_data[species_number - 1];

            num_pls = 0;

            namp = namp_data[species_number - 1] - 1;
            for (i = 0; i < sp->num_namplas; i++) {
                namp++;

                if (namp->pn == 99) {
                    continue;
                }
                if (namp->x != x) {
                    continue;
                }
                if (namp->y != y) {
                    continue;
                }
                if (namp->z != z) {
                    continue;
                }
                if ((namp->status & POPULATED) == 0) {
                    continue;
                }

                really_hidden = FALSE;
                if (namp->hidden) {
                    /* If this species and planet is explicitly mentioned in ATTACK/ENGAGE orders, then the planet cannot hide during the battle. */
                    really_hidden = TRUE;

                    for (at_index = 0; at_index < bat->num_species_here; at_index++) {
                        for (j = 0; j < MAX_SPECIES; j++) {
                            k = bat->enemy_mine[at_index][j];
                            if (k < 0) {
                                k = -k;
                            }
                            if (k == species_number) {
                                for (k = 0; k < bat->num_engage_options[at_index]; k++) {
                                    if (bat->engage_option[at_index][k] >= PLANET_ATTACK &&
                                        bat->engage_option[at_index][k] <= SIEGE &&
                                        bat->engage_planet[at_index][k] == namp->pn) {
                                        really_hidden = FALSE;
                                        break;
                                    }
                                }
                                if (!really_hidden) {
                                    break;
                                }
                            }
                        }
                        if (!really_hidden) {
                            break;
                        }
                    }
                }

                if (really_hidden) {
                    continue;
                }

                found = TRUE;
                pl_num[num_pls++] = namp->pn;
            }

            sh = ship_data[species_number - 1] - 1;
            for (i = 0; i < sp->num_ships; i++) {
                sh++;

                if (sh->pn == 99) {
                    continue;
                }
                if (sh->x != x) {
                    continue;
                }
                if (sh->y != y) {
                    continue;
                }
                if (sh->z != z) {
                    continue;
                }
                if (sh->status == UNDER_CONSTRUCTION) {
                    continue;
                }
                if (sh->status == ON_SURFACE) {
                    continue;
                }
                if (sh->status == JUMPED_IN_COMBAT) {
                    continue;
                }
                if (sh->status == FORCED_JUMP) {
                    continue;
                }
                found = TRUE;

                break;
            }

            if (!found) {
                continue;
            }

            sp_index = bat->num_species_here;
            bat->spec_num[sp_index] = location->s;
            bat->special_target[sp_index] = 0;
            bat->transport_withdraw_age[sp_index] = 0;
            bat->warship_withdraw_age[sp_index] = 100;
            bat->fleet_withdraw_percentage[sp_index] = 100;
            bat->haven_x[sp_index] = 127;
            bat->engage_option[sp_index][0] = DEFENSE_IN_PLACE;
            bat->num_engage_options[sp_index] = 1;
            if (num_pls > 0) {
                /* Provide default Engage 2 options. */
                for (i = 0; i < num_pls; i++) {
                    bat->engage_option[sp_index][i + 1] = PLANET_DEFENSE;
                    bat->engage_planet[sp_index][i + 1] = pl_num[i];
                }
                bat->num_engage_options[sp_index] = num_pls + 1;
            }
            bat->can_be_surprised[sp_index] = TRUE;
            bat->hijacker[sp_index] = FALSE;
            bat->summary_only[sp_index] = default_summary;
            for (i = 0; i < MAX_SPECIES; i++) {
                bat->enemy_mine[sp_index][i] = 0;
            }
            bat->num_species_here++;
        }

        /* If haven locations have not been specified, provide random locations nearby. */
        for (sp_index = 0; sp_index < bat->num_species_here; sp_index++) {
            if (bat->haven_x[sp_index] != 127) {
                continue;
            }

            while (1) {
                i = x + 2 - rnd(3);
                j = y + 2 - rnd(3);
                k = z + 2 - rnd(3);

                if (i != x || j != y || k != z) {
                    break;
                }
            }

            bat->haven_x[sp_index] = i;
            bat->haven_y[sp_index] = j;
            bat->haven_z[sp_index] = k;
        }

        /* Do battle at this battle location. */
        do_battle(bat);

        if (prompt_gm) {
            printf("Hit RETURN to continue...");

            fflush(stdout);
            fgets(answer, 16, stdin);
        }
    }

    /* Declare new enmities. */
    for (i = 0; i < galaxy.num_species; i++) {
        log_open = FALSE;

        for (j = 0; j < galaxy.num_species; j++) {
            if (i == j) {
                continue;
            }

            betrayed_species_number = make_enemy[i][j];
            if (betrayed_species_number == 0) {
                continue;
            }

            enemy_word_number = j / 32;
            enemy_bit_number = j % 32;
            enemy_mask = 1 << enemy_bit_number;

            /* Clear ally bit. */
            spec_data[i].ally[enemy_word_number] &= ~enemy_mask;

            /* Set enemy and contact bits (in case this is first encounter). */
            spec_data[i].enemy[enemy_word_number] |= enemy_mask;
            spec_data[i].contact[enemy_word_number] |= enemy_mask;

            data_modified[i] = TRUE;

            if (!log_open) {
                /* Open temporary species log file for appending. */
                sprintf(filename, "sp%02d.temp.log", i + 1);
                log_file = fopen(filename, "a");
                if (log_file == NULL) {
                    fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
                    exit(-1);
                }

                append_log[i] = TRUE;
                log_open = TRUE;
            }

            log_string("\n!!! WARNING: Enmity has been automatically declared towards SP ");
            log_string(spec_data[j].name);
            log_string(" because they surprise-attacked SP ");
            log_string(spec_data[betrayed_species_number - 1].name);
            log_string("!\n");
        }

        if (log_open) {
            fclose(log_file);
        }
    }

    if (prompt_gm) {
        printf("\n*** Gamemaster safe-abort option ... type q or Q to quit: ");

        fflush(stdout);
        fgets(answer, 16, stdin);
        if (answer[0] == 'q' || answer[0] == 'Q') {
            save = FALSE;
        }
    }

    /* If results are to be saved, append temporary logs to actual species logs. In either case, delete temporary logs. */
    for (i = 0; i < galaxy.num_species; i++) {
        if (!append_log[i]) {
            continue;
        }

        if (save) {
            sprintf(filename, "sp%02d.log", i + 1);
            species_log = fopen(filename, "a");
            if (species_log == NULL) {
                fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
                exit(-1);
            }
        }

        sprintf(filename, "sp%02d.temp.log", i + 1);

        if (save) {
            temp_species_log = fopen(filename, "r");
            if (temp_species_log == NULL) {
                fprintf(stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
                exit(-1);
            }

            /* Copy temporary log to permanent species log. */
            while (readln(log_line, 256, temp_species_log) != NULL) {
                fputs(log_line, species_log);
            }

            fclose(temp_species_log);
            fclose(species_log);
        }

        /* Delete temporary log file. */
        unlink(filename);
    }

    return save;
}


int combatCommand(int argc, char *argv[]) {
    int default_summary = FALSE;
    int do_all_species = TRUE;
    int num_species = 0;
    struct species_data *sp = NULL;
    char *sp_name[MAX_SPECIES];

    int sp_num[MAX_SPECIES];
    memset(sp_num, 0, sizeof(sp_num));

    prompt_gm = FALSE;
    strike_phase = FALSE; // assume combat mode

    /* Get commonly used data. */
    get_galaxy_data();
    get_planet_data();
    get_transaction_data();
    get_location_data();

    /* Allocate memory for battle data. */
    battle_base = (struct battle_data *) ncalloc(__FUNCTION__, __LINE__, MAX_BATTLES, sizeof(struct battle_data));
    if (battle_base == NULL) {
        perror("combatCommand:");
        fprintf(stderr, "\nCannot allocate enough memory for battle data!\n\n");
        exit(2);
    }

    /* Check arguments.
     * If an argument is -s, then set SUMMARY mode for everyone.
     * The default is for players to receive a detailed report of the battles.
     * If an argument is -p, then prompt the GM before saving results;
     * otherwise, operate quietly; i.e, do not prompt GM before saving results
     * and do not display anything except errors.
     * Any additional arguments must be species numbers.
     * If no species numbers are specified, then do all species. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            default_summary = TRUE;
        } else if (strcmp(argv[i], "-p") == 0) {
            prompt_gm = TRUE;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
            printf(" info: combat: last_random is %12lu\n", last_random);
        } else if (strcmp(argv[i], "--combat") == 0) {
            strike_phase = FALSE;
        } else if (strcmp(argv[i], "--strike") == 0) {
            strike_phase = TRUE;
        } else {
            int n = atoi(argv[i]);
            if (0 < n && n <= galaxy.num_species) {
                sp_num[num_species] = n;
                num_species++;
            }
        }
    }

    printf(" info: combat: running %s mode\n", strike_phase ? "strike" : "combat");

    log_stdout = prompt_gm;

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

    if (default_summary && prompt_gm) {
        printf("\nSUMMARY mode is in effect for all species.\n\n");
    }

    /* Read in species data and make an uppercase copy of each name for comparison purposes later. Also do some initializations. */
    get_species_data();

    for (int sp_index = 0; sp_index < galaxy.num_species; sp_index++) {
        sp_name[sp_index] = ncalloc(__FUNCTION__, __LINE__, 1, 32);
        if (!data_in_memory[sp_index]) {
            /* No longer in game. */
            continue;
        }
        sp = &spec_data[sp_index];
        ship_base = ship_data[sp_index];
        /* Convert name to upper case. */
        for (int i = 0; i < 31; i++) {
            sp_name[sp_index][i] = toupper(sp->name[i]);
        }
        for (int i = 0; i < sp->num_ships; i++) {
            ship = ship_base + i;
            ship->special = 0;
        }
    }

    int save = combat(default_summary, do_all_species, num_species, sp_num, sp_name, &loc[0]);
    if (save) {
        save_planet_data();
        save_species_data();
        save_transaction_data();
    }

    free_species_data();

    return 0;
}


void consolidate_option(char option, char location) {
    /* Only attack options go in list. */
    if (option < DEEP_SPACE_FIGHT) {
        return;
    }
    /* Make sure pre-requisites are already in the list. Bombardment, and germ warfare must follow a successful planet attack. */
    if (option > PLANET_ATTACK) {
        consolidate_option(PLANET_ATTACK, location);
    }
    /* Check if option and location are already in list. */
    for (int i = 0; i < num_combat_options; i++) {
        if (option == combat_option[i] && location == combat_location[i]) {
            return;
        }
    }
    /* Add new option to list. */
    combat_option[num_combat_options] = option;
    combat_location[num_combat_options] = location;
    num_combat_options++;
}


int disbanded_species_ship(int species_index, struct ship_data *sh) {
    struct nampla_data *nam = c_nampla[species_index] - 1;
    for (int nampla_index = 0; nampla_index < c_species[species_index]->num_namplas; nampla_index++) {
        ++nam;
        if (nam->x != sh->x) { continue; }
        if (nam->y != sh->y) { continue; }
        if (nam->z != sh->z) { continue; }
        if (nam->pn != sh->pn) { continue; }
        if ((nam->status & DISBANDED_COLONY) == 0) { continue; }
        if (sh->type != STARBASE && sh->status == IN_ORBIT) { continue; }
        /* This ship is either on the surface of a disbanded colony or is a starbase orbiting a disbanded colony. */
        return TRUE;
    }
    return FALSE;
}


void do_ambush(int ambushing_species_index, struct battle_data *bat) {
    int i, j, n, num_sp, ambushed_species_index, num_ships, age_increment;
    int species_number;
    int old_truncate_name;
    long friendly_tonnage, enemy_tonnage;
    struct ship_data *sh;

    /* Get total ambushing tonnage. */
    friendly_tonnage = 0;
    num_ships = c_species[ambushing_species_index]->num_ships;
    sh = c_ship[ambushing_species_index] - 1;
    for (i = 0; i < num_ships; i++) {
        ++sh;
        if (sh->pn == 99) {
            continue;
        }
        if (sh->x != bat->x) {
            continue;
        }
        if (sh->y != bat->y) {
            continue;
        }
        if (sh->z != bat->z) {
            continue;
        }
        if (sh->class != TR && sh->class != BA) {
            friendly_tonnage += sh->
                    tonnage;
        }
    }

    /* Determine which species are being ambushed and get total enemy tonnage. */
    num_sp = bat->num_species_here;
    enemy_tonnage = 0;
    for (ambushed_species_index = 0; ambushed_species_index < num_sp; ++ambushed_species_index) {
        if (!bat->enemy_mine[ambushing_species_index][ambushed_species_index]) {
            continue;
        }

        /* This species is being ambushed.  Get total effective tonnage. */
        num_ships = c_species[ambushed_species_index]->num_ships;
        sh = c_ship[ambushed_species_index] - 1;
        for (i = 0; i < num_ships; i++) {
            ++sh;

            if (sh->pn == 99) {
                continue;
            }
            if (sh->x != bat->x) {
                continue;
            }
            if (sh->y != bat->y) {
                continue;
            }
            if (sh->z != bat->z) {
                continue;
            }
            if (sh->class == TR) {
                enemy_tonnage += sh->tonnage;
            } else {
                enemy_tonnage += 10 * sh->tonnage;
            }
        }
    }

    /* Determine the amount of aging that will be added to each ambushed ship. */
    if (enemy_tonnage == 0) {
        return;
    }
    age_increment = (10L * bat->ambush_amount[ambushing_species_index]) / enemy_tonnage;
    age_increment = (friendly_tonnage * age_increment) / enemy_tonnage;
    ambush_took_place = TRUE;

    if (age_increment < 1) {
        log_string("\n    SP ");
        log_string(c_species[ambushing_species_index]->name);
        log_string(" attempted an ambush, but the ambush was completely ineffective!\n");
        return;
    }

    /* Age each ambushed ship. */
    for (ambushed_species_index = 0; ambushed_species_index < num_sp; ++ambushed_species_index) {
        if (!bat->enemy_mine[ambushing_species_index][ambushed_species_index]) {
            continue;
        }
        log_string("\n    SP ");
        species_number = bat->spec_num[ambushed_species_index];
        if (field_distorted[ambushed_species_index]) {
            log_int(distorted(species_number));
        } else {
            log_string(c_species[ambushed_species_index]->name);
        }
        log_string(" was ambushed by SP ");
        log_string(c_species[ambushing_species_index]->name);
        log_string("!\n");
        num_ships = c_species[ambushed_species_index]->num_ships;
        sh = c_ship[ambushed_species_index] - 1;
        for (i = 0; i < num_ships; i++) {
            ++sh;
            if (sh->pn == 99) {
                continue;
            }
            if (sh->x != bat->x) {
                continue;
            }
            if (sh->y != bat->y) {
                continue;
            }
            if (sh->z != bat->z) {
                continue;
            }
            sh->age += age_increment;
            if (sh->arrived_via_wormhole) {
                sh->age += age_increment;
            }

            if (sh->age > 49) {
                old_truncate_name = truncate_name;
                truncate_name = TRUE;
                log_string("      ");
                log_string(ship_name(sh));
                if (field_distorted[ambushed_species_index]) {
                    log_string(" = ");
                    log_string(c_species[ambushed_species_index]->name);
                    log_char(' ');
                    n = sh->item_quantity[FD];
                    sh->item_quantity[FD] = 0;
                    log_string(ship_name(sh));
                    sh->item_quantity[FD] = n;
                }
                n = 0;
                for (j = 0; j < MAX_ITEMS; j++) {
                    if (sh->item_quantity[j] > 0) {
                        if (n++ == 0) {
                            log_string(" (cargo: ");
                        } else {
                            log_char(',');
                        }
                        log_int((int) sh->item_quantity[j]);
                        log_char(' ');
                        log_string(item_abbr[j]);
                    }
                }
                if (n > 0) {
                    log_char(')');
                }
                log_string(" was destroyed in the ambush!\n");
                truncate_name = old_truncate_name;
            }
        }
    }
}

void do_battle(struct battle_data *bat) {
    int i, j, k;
    int species_index;
    int species_number;
    int num_sp, save,
            max_rounds, round_number, battle_here, fight_here,
            unit_index, option_index, current_species, temp_status,
            temp_pn, num_namplas, array_index, bit_number, first_action,
            traitor_number, betrayed_number, betrayal, need_comma,
            TRUE_value, do_withdraw_check_first;
    short identifiable_units[MAX_SPECIES], unidentifiable_units[MAX_SPECIES];
    long n, bit_mask;
    char x, y, z, where, option, filename[32], enemy, enemy_num[MAX_SPECIES], log_line[256];
    FILE *combat_log, *species_log;
    struct action_data act;
    struct nampla_data *namp, *attacked_nampla;
    struct ship_data *sh;

    ambush_took_place = FALSE;

    /* Open log file for writing. */
    log_file = fopen("combat.log", "w");
    if (log_file == NULL) {
        fprintf(stderr, "\n\tCannot open 'combat.log' for writing!\n\n");
        exit(-1);
    }

    /* Open summary file for writing. */
    summary_file = fopen("summary.log", "w");
    if (summary_file == NULL) {
        fprintf(stderr, "\n\tCannot open 'summary.log' for writing!\n\n");
        exit(-1);
    }
    log_summary = TRUE;

    /* Get data for all species present at this battle. */
    num_sp = bat->num_species_here;
    for (species_index = 0; species_index < num_sp; ++species_index) {
        species_number = bat->spec_num[species_index];
        c_species[species_index] = &spec_data[species_number - 1];
        c_nampla[species_index] = namp_data[species_number - 1];
        c_ship[species_index] = ship_data[species_number - 1];
        if (data_in_memory[species_number - 1]) {
            data_modified[species_number - 1] = TRUE;
        } else {
            fprintf(stderr, "\n\tData for species #%d is needed but is not available!\n\n",
                    species_number);
            exit(-1);
        }

        /* Determine number of identifiable and unidentifiable units present. */
        identifiable_units[species_index] = 0;
        unidentifiable_units[species_index] = 0;

        namp = c_nampla[species_index] - 1;
        for (i = 0; i < c_species[species_index]->num_namplas; i++) {
            ++namp;

            if (namp->x != bat->x) { continue; }
            if (namp->y != bat->y) { continue; }
            if (namp->z != bat->z) { continue; }

            if (namp->status & POPULATED) { ++identifiable_units[species_index]; }
        }

        sh = c_ship[species_index] - 1;
        for (i = 0; i < c_species[species_index]->num_ships; i++) {
            ++sh;

            if (sh->x != bat->x) { continue; }
            if (sh->y != bat->y) { continue; }
            if (sh->z != bat->z) { continue; }
            if (sh->status == UNDER_CONSTRUCTION) { continue; }
            if (sh->status == JUMPED_IN_COMBAT) { continue; }
            if (sh->status == FORCED_JUMP) { continue; }

            sh->dest_x = 0;    /* Not yet exposed. */
            sh->dest_y = 100;    /* Shields at 100%. */

            if (sh->item_quantity[FD] == sh->tonnage) {
                ++unidentifiable_units[species_index];
            } else {
                ++identifiable_units[species_index];
            }
        }

        if (identifiable_units[species_index] > 0 || unidentifiable_units[species_index] == 0) {
            field_distorted[species_index] = FALSE;
        } else {
            field_distorted[species_index] = TRUE;
        }
    }

    /* Start log of what's happening. */
    if (strike_phase) {
        log_string("\nStrike log:\n");
    } else {
        log_string("\nCombat log:\n");
    }
    first_battle = FALSE;

    log_string("\n  Battle orders were received for sector ");
    log_int(bat->x);
    log_string(", ");
    log_int(bat->y);
    log_string(", ");
    log_int(bat->z);
    log_string(". The following species are present:\n\n");

    /* Convert enemy_mine array from a list of species numbers to an array
	of TRUE/FALSE values whose indices are:

			[species_index1][species_index2]

	such that the value will be TRUE if #1 mentioned #2 in an ATTACK
	or HIJACK command.  The actual TRUE value will be 1 for ATTACK or
	2 for HIJACK. */

    for (species_index = 0; species_index < num_sp; ++species_index) {
        /* Make copy of list of enemies. */
        for (i = 0; i < MAX_SPECIES; i++) {
            enemy_num[i] = bat->enemy_mine[species_index][i];
            bat->enemy_mine[species_index][i] = FALSE;
        }

        for (i = 0; i < MAX_SPECIES; i++) {
            enemy = enemy_num[i];
            if (enemy == 0) { break; }    /* No more enemies in list. */

            if (enemy < 0) {
                enemy = -enemy;
                TRUE_value = 2;        /* This is a hijacking. */
            } else {
                TRUE_value = 1;
            }        /* This is a normal attack. */

            /* Convert absolute species numbers to species indices that
            have been assigned in the current battle. */
            for (j = 0; j < num_sp; j++) {
                if (enemy == bat->spec_num[j]) {
                    bat->enemy_mine[species_index][j] = TRUE_value;
                }
            }
        }
    }

    /* For each species that has been mentioned in an attack order, check
	if it can be surprised. A species can only be surprised if it has
	not given a BATTLE order and if it is being attacked ONLY by one
	or more ALLIES. */
    for (species_index = 0; species_index < num_sp; ++species_index) {
        j = bat->spec_num[species_index] - 1;
        array_index = j / 32;
        bit_number = j % 32;
        bit_mask = 1 << bit_number;

        for (i = 0; i < num_sp; i++) {
            if (i == species_index) { continue; }

            if (!bat->enemy_mine[species_index][i]) { continue; }

            if (field_distorted[species_index]) {
                /* Attacker is field-distorted. Surprise not possible. */
                bat->can_be_surprised[i] = FALSE;
                continue;
            }

            if ((c_species[i]->ally[array_index] & bit_mask)) {
                betrayal = TRUE;
            } else {
                betrayal = FALSE;
            }

            if (betrayal) {
                /* Someone is being attacked by an ALLY. */
                traitor_number = bat->spec_num[species_index];
                betrayed_number = bat->spec_num[i];
                make_enemy[betrayed_number - 1][traitor_number - 1] = betrayed_number;
                auto_enemy(traitor_number, betrayed_number);
            }

            if (!bat->can_be_surprised[i]) { continue; }

            if (!betrayal) {    /* At least one attacker is not an ally. */
                bat->can_be_surprised[i] = FALSE;
            }
        }
    }

    /* For each species that has been mentioned in an attack order, see if
	there are other species present that have declared it as an ALLY.
	If so, have the attacker attack the other species and vice-versa. */
    for (species_index = 0; species_index < num_sp; ++species_index) {
        for (i = 0; i < num_sp; i++) {
            if (i == species_index) { continue; }

            if (!bat->enemy_mine[species_index][i]) { continue; }

            j = bat->spec_num[i] - 1;
            array_index = j / 32;
            bit_number = j % 32;
            bit_mask = 1 << bit_number;

            for (k = 0; k < num_sp; k++) {
                if (k == species_index) { continue; }
                if (k == i) { continue; }

                if (c_species[k]->ally[array_index] & bit_mask) {
                    /* Make sure it's not already set (it may already be set
                    for HIJACK and we don't want to accidentally change
                    it to ATTACK). */
                    if (!bat->enemy_mine[species_index][k]) {
                        bat->enemy_mine[species_index][k] = TRUE;
                    }
                    if (!bat->enemy_mine[k][species_index]) {
                        bat->enemy_mine[k][species_index] = TRUE;
                    }
                }
            }
        }
    }

    /* If a species did not give a battle order and is not the target of an
	attack, set can_be_surprised flag to a special value. */
    for (species_index = 0; species_index < num_sp; ++species_index) {
        if (!bat->can_be_surprised[species_index]) { continue; }

        bat->can_be_surprised[species_index] = 55;

        for (i = 0; i < num_sp; i++) {
            if (i == species_index) { continue; }

            if (!bat->enemy_mine[i][species_index]) { continue; }

            bat->can_be_surprised[species_index] = TRUE;

            break;
        }
    }

    /* List combatants. */
    for (species_index = 0; species_index < num_sp; ++species_index) {
        species_number = bat->spec_num[species_index];

        log_string("    SP ");
        if (field_distorted[species_index]) {
            log_int(distorted(species_number));
        } else {
            log_string(c_species[species_index]->name);
        }
        if (bat->can_be_surprised[species_index]) {
            log_string(" does not appear to be ready for combat.\n");
        } else {
            log_string(" is mobilized and ready for combat.\n");
        }
    }

    /* Check if a declared enemy is being ambushed. */
    for (i = 0; i < num_sp; i++) {
        namp = c_nampla[i] - 1;
        num_namplas = c_species[i]->num_namplas;
        bat->ambush_amount[i] = 0;
        for (j = 0; j < num_namplas; j++) {
            ++namp;

            if (namp->x != bat->x) { continue; }
            if (namp->y != bat->y) { continue; }
            if (namp->z != bat->z) { continue; }

            bat->ambush_amount[i] += namp->use_on_ambush;
        }

        if (bat->ambush_amount[i] == 0) { continue; }

        for (j = 0; j < num_sp; j++) {
            if (bat->enemy_mine[i][j]) {
                do_ambush(i, bat);
            }
        }
    }

    /* For all species that specified enemies, make the feeling mutual. */
    for (i = 0; i < num_sp; i++) {
        for (j = 0; j < num_sp; j++) {
            if (bat->enemy_mine[i][j]) {
                /* Make sure it's not already set (it may already be set for
                HIJACK and we don't want to accidentally change it to
                ATTACK). */
                if (!bat->enemy_mine[j][i]) { bat->enemy_mine[j][i] = TRUE; }
            }
        }
    }

    /* Create a sequential list of combat options. First check if a
	deep space defense has been ordered. If so, then make sure that
	first option is DEEP_SPACE_FIGHT. */
    num_combat_options = 0;
    for (species_index = 0; species_index < num_sp; ++species_index) {
        for (i = 0; i < bat->num_engage_options[species_index]; i++) {
            option = bat->engage_option[species_index][i];
            if (option == DEEP_SPACE_DEFENSE) {
                consolidate_option(DEEP_SPACE_FIGHT, 0);
                goto consolidate;
            }
        }
    }

    consolidate:
    for (species_index = 0; species_index < num_sp; ++species_index) {
        for (i = 0; i < bat->num_engage_options[species_index]; i++) {
            option = bat->engage_option[species_index][i];
            where = bat->engage_planet[species_index][i];
            consolidate_option(option, where);
        }
    }

    /* If ships are given unconditional withdraw orders, they will always have
	time to escape if fighting occurs first in a different part of the
	sector. The flag "do_withdraw_check_first" will be set only after the
	first round of combat. */
    do_withdraw_check_first = FALSE;

    /* Handle each combat option. */
    battle_here = FALSE;
    first_action = TRUE;
    for (option_index = 0; option_index < num_combat_options; option_index++) {
        option = combat_option[option_index];
        where = combat_location[option_index];

        /* Fill action arrays with data about ships taking part in current action. */
        fight_here = fighting_params(option, where, bat, &act);
        /* Check if a fight will take place here. */
        if (!fight_here) {
            continue;
        }
        /* See if anyone is taken by surprise. */
        if (!battle_here) {
            /* Combat is just starting. */
            for (species_index = 0; species_index < num_sp; ++species_index) {
                species_number = bat->spec_num[species_index];
                if (bat->can_be_surprised[species_index] == 55) {
                    continue;
                }
                if (bat->can_be_surprised[species_index]) {
                    log_string("\n    SP ");
                    if (field_distorted[species_index]) {
                        log_int(distorted(species_number));
                    } else {
                        log_string(c_species[species_index]->name);
                    }
                    log_string(" is taken by surprise!\n");
                }
            }
        }

        battle_here = TRUE;

        /* Clear out can_be_surprised array. */
        for (i = 0; i < MAX_SPECIES; i++) {
            bat->can_be_surprised[i] = FALSE;
        }

        /* Determine maximum number of rounds. */
        max_rounds = 10000;    /* Something ridiculously large. */
        if (option == DEEP_SPACE_FIGHT && attacking_ML > 0 && defending_ML > 0 && deep_space_defense) {
            /* This is the initial deep space fight and the defender wants the
            fight to remain in deep space for as long as possible. */
            if (defending_ML > attacking_ML) {
                max_rounds = defending_ML - attacking_ML;
            } else {
                max_rounds = 1;
            }
        } else if (option == PLANET_BOMBARDMENT) {
            /* To determine the effectiveness of the bombardment, we will
            simulate ten rounds of combat and add up the damage. */
            max_rounds = 10;
        } else if (option == GERM_WARFARE || option == SIEGE) {
            /* We just need to see who is attacking whom and get the number
            of germ warfare bombs being used. */
            max_rounds = 1;
        }

        /* Log start of action. */
        if (where == 0) {
            log_string("\n    The battle begins in deep space, outside the range of planetary defenses...\n");
        } else if (option == PLANET_ATTACK) {
            log_string("\n    The battle ");
            if (first_action) {
                log_string("begins");
            } else {
                log_string("moves");
            }
            log_string(" within range of planet #");
            log_int(where);
            log_string("...\n");
        } else if (option == PLANET_BOMBARDMENT) {
            log_string("\n    Bombardment of planet #");
            log_int(where);
            log_string(" begins...\n");
        } else if (option == GERM_WARFARE) {
            log_string("\n    Germ warfare commences against planet #");
            log_int(where);
            log_string("...\n");
        } else if (option == SIEGE) {
            log_string("\n    Siege of planet #");
            log_int(where);
            log_string(" is now in effect...\n\n");
            goto do_combat;
        }

        /* List combatants. */
        truncate_name = FALSE;
        log_string("\n      Units present:");
        current_species = -1;
        for (unit_index = 0; unit_index < act.num_units_fighting; unit_index++) {
            if (act.fighting_species_index[unit_index] != current_species) {
                /* Display species name. */
                i = act.fighting_species_index[unit_index];
                log_string("\n        SP ");
                species_number = bat->spec_num[i];
                if (field_distorted[i]) {
                    log_int(distorted(species_number));
                } else {
                    log_string(c_species[i]->name);
                }
                log_string(": ");
                current_species = i;
                need_comma = FALSE;
            }

            if (act.unit_type[unit_index] == SHIP) {
                sh = (struct ship_data *) act.fighting_unit[unit_index];
                temp_status = sh->status;
                temp_pn = sh->pn;
                if (option == DEEP_SPACE_FIGHT) {
                    sh->status = IN_DEEP_SPACE;
                    sh->pn = 0;
                } else {
                    sh->status = IN_ORBIT;
                    sh->pn = where;
                }
                ignore_field_distorters = !field_distorted[current_species];
                if (sh->special != NON_COMBATANT) {
                    if (need_comma) { log_string(", "); }
                    log_string(ship_name(sh));
                    need_comma = TRUE;
                }
                ignore_field_distorters = FALSE;
                sh->status = temp_status;
                sh->pn = temp_pn;
            } else {
                namp = (struct nampla_data *) act.fighting_unit[unit_index];
                if (need_comma) { log_string(", "); }
                log_string("PL ");
                log_string(namp->name);
                need_comma = TRUE;
            }
        }
        log_string("\n\n");

        do_combat:

        /* Long names are not necessary for the rest of the action. */
        truncate_name = TRUE;

        /* Do combat rounds. Stop if maximum count is reached, or if combat does not occur when do_round() is called. */

        round_number = 1;

        log_summary = FALSE;    /* do_round() and the routines that it calls will set this for important stuff. */

        if (option == PLANET_BOMBARDMENT || option == GERM_WARFARE || option == SIEGE) {
            logging_disabled = TRUE;
        } /* Disable logging during simulation. */

        while (round_number <= max_rounds) {
            if (do_withdraw_check_first) {
                withdrawal_check(bat, &act);
            }

            if (!do_round(option, round_number, bat, &act)) { break; }

            if (!do_withdraw_check_first) { withdrawal_check(bat, &act); }

            do_withdraw_check_first = TRUE;

            regenerate_shields(&act);

            ++round_number;
        }

        log_summary = TRUE;
        logging_disabled = FALSE;

        if (round_number == 1) {
            log_string("      ...But it seems that the attackers had nothing to attack!\n");
            continue;
        }

        if (option == PLANET_BOMBARDMENT || option == GERM_WARFARE) {
            for (unit_index = 0; unit_index < act.num_units_fighting; unit_index++) {
                if (act.unit_type[unit_index] == GENOCIDE_NAMPLA) {
                    attacked_nampla = (struct nampla_data *) act.fighting_unit[unit_index];
                    j = act.fighting_species_index[unit_index];
                    for (i = 0; i < num_sp; i++) {
                        if (x_attacked_y[i][j]) {
                            species_number = bat->spec_num[i];
                            log_string("      SP ");
                            if (field_distorted[i]) {
                                log_int(distorted(species_number));
                            } else {
                                log_string(c_species[i]->name);
                            }
                            log_string(" bombards SP ");
                            log_string(c_species[j]->name);
                            log_string(" on PL ");
                            log_string(attacked_nampla->name);
                            log_string(".\n");

                            if (option == GERM_WARFARE) {
                                do_germ_warfare(i, j, unit_index, bat, &act);
                            }
                        }
                    }

                    /* Determine results of bombardment. */
                    if (option == PLANET_BOMBARDMENT) {
                        do_bombardment(unit_index, &act);
                    }
                }
            }
        } else if (option == SIEGE) {
            do_siege(bat, &act);
        }
        truncate_name = FALSE;
        first_action = FALSE;
    }

    if (!battle_here) {
        if (bat->num_species_here == 1) {
            log_string("    But there was no one to fight with!\n");
        } else if (!ambush_took_place) {
            log_string("    But no one was willing to throw the first punch!\n");
        }
    }

    /* Close combat log and append it to the log files of all species involved in this battle. */
    if (prompt_gm) {
        printf("\n  End of battle in sector %d, %d, %d.\n", bat->x, bat->y, bat->z);
    }
    fprintf(log_file, "\n  End of battle in sector %d, %d, %d.\n", bat->x, bat->y, bat->z);
    fprintf(summary_file, "\n  End of battle in sector %d, %d, %d.\n", bat->x, bat->y, bat->z);
    fclose(log_file);
    fclose(summary_file);

    for (species_index = 0; species_index < num_sp; ++species_index) {
        species_number = bat->spec_num[species_index];
        /* Open combat log file for reading. */
        if (bat->summary_only[species_index]) {
            combat_log = fopen("summary.log", "r");
        } else {
            combat_log = fopen("combat.log", "r");
        }

        if (combat_log == NULL) {
            fprintf(stderr, "\n\tCannot open combat log for reading!\n\n");
            exit(-1);
        }

        /* Open a temporary species log file for appending. */
        sprintf(filename, "sp%02d.temp.log", species_number);
        species_log = fopen(filename, "a");
        if (species_log == NULL) {
            fprintf(stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
            exit(-1);
        }

        /* Copy combat log to temporary species log. */
        while (readln(log_line, 256, combat_log) != NULL) {
            fputs(log_line, species_log);
        }

        fclose(species_log);
        fclose(combat_log);

        append_log[species_number - 1] = TRUE;

        /* Get rid of ships that were destroyed. */
        if (!data_modified[species_number - 1]) { continue; }
        sh = c_ship[species_index] - 1;
        for (i = 0; i < c_species[species_index]->num_ships; i++) {
            ++sh;

            if (sh->age < 50) { continue; }
            if (sh->pn == 99) { continue; }
            if (sh->x != bat->x) { continue; }
            if (sh->y != bat->y) { continue; }
            if (sh->z != bat->z) { continue; }
            if (sh->status == UNDER_CONSTRUCTION) { continue; }

            delete_ship(sh);
        }
    }
}

void do_bombardment(int unit_index, struct action_data *act) {
    int i, new_mi, new_ma, defending_species;
    long n, total_bomb_damage, CS_bomb_damage, new_pop, initial_base, total_pop, percent_damage;
    struct nampla_data *attacked_nampla;
    struct planet_data *planet;
    struct ship_data *sh;

    attacked_nampla = (struct nampla_data *) act->fighting_unit[unit_index];
    planet = planet_base + (long) attacked_nampla->planet_index;

    initial_base = attacked_nampla->mi_base + attacked_nampla->ma_base;
    total_pop = initial_base;

    if (attacked_nampla->item_quantity[CU] > 0) {
        total_pop += 1;
    }
    if (total_pop < 1) {
        log_string("        The planet is completely uninhabited. There is nothing to bomb!\n");
        return;
    }

    /* Total damage done by ten strike cruisers (ML = 50) in ten rounds
     * is 100 x 4 x the power value for a single ship. To eliminate the
     * chance of overflow, the algorithm has been carefully chosen. */
    CS_bomb_damage = 400 * power(ship_tonnage[CS]); /* Should be 400 * 4759 = 1,903,600. */

    total_bomb_damage = act->bomb_damage[unit_index];

    /* Keep about 2 significant digits. */
    while (total_bomb_damage > 1000) {
        total_bomb_damage /= 10;
        CS_bomb_damage /= 10;
    }

    if (CS_bomb_damage == 0) {
        percent_damage = 101;
    } else {
        percent_damage = ((total_bomb_damage * 250000L) / CS_bomb_damage) / total_pop;
    }
    if (percent_damage > 100) {
        percent_damage = 101;
    }

    new_mi = attacked_nampla->mi_base - (percent_damage * attacked_nampla->mi_base) / 100;
    new_ma = attacked_nampla->ma_base - (percent_damage * attacked_nampla->ma_base) / 100;
    new_pop = attacked_nampla->pop_units - (percent_damage * attacked_nampla->pop_units) / 100;

    if (new_mi == attacked_nampla->mi_base && new_ma == attacked_nampla->ma_base &&
        new_pop == attacked_nampla->pop_units) {
        log_string("        Damage due to bombardment was insignificant.\n");
        return;
    }

    defending_species = act->fighting_species_index[unit_index];
    if (attacked_nampla->status & HOME_PLANET) {
        n = attacked_nampla->mi_base + attacked_nampla->ma_base;
        if (c_species[defending_species]->hp_original_base < n) {
            c_species[defending_species]->hp_original_base = n;
        }
    }

    if (new_mi <= 0 && new_ma <= 0 && new_pop <= 0) {
        log_string("        Everyone and everything was completely wiped out!\n");

        attacked_nampla->mi_base = 0;
        attacked_nampla->ma_base = 0;
        attacked_nampla->pop_units = 0;
        attacked_nampla->siege_eff = 0;
        attacked_nampla->shipyards = 0;
        attacked_nampla->hiding = 0;
        attacked_nampla->hidden = 0;
        attacked_nampla->use_on_ambush = 0;

        /* Reset status. */
        if (attacked_nampla->status & HOME_PLANET) {
            attacked_nampla->status = HOME_PLANET;
        } else {
            attacked_nampla->status = COLONY;
        }

        for (i = 0; i < MAX_ITEMS; i++) {
            attacked_nampla->item_quantity[i] = 0;
        }

        /* Delete any ships that were under construction on the planet. */
        sh = c_ship[defending_species] - 1;
        for (i = 0; i < c_species[defending_species]->num_ships; i++) {
            ++sh;
            if (sh->x != attacked_nampla->x) { continue; }
            if (sh->y != attacked_nampla->y) { continue; }
            if (sh->z != attacked_nampla->z) { continue; }
            if (sh->pn != attacked_nampla->pn) { continue; }
            delete_ship(sh);
        }

        return;
    }

    log_string("        Mining base of PL ");
    log_string(attacked_nampla->name);
    log_string(" went from ");
    log_int(attacked_nampla->mi_base / 10);
    log_char('.');
    log_int(attacked_nampla->mi_base % 10);
    log_string(" to ");
    attacked_nampla->mi_base = new_mi;
    log_int(new_mi / 10);
    log_char('.');
    log_int(new_mi % 10);
    log_string(".\n");

    log_string("        Manufacturing base of PL ");
    log_string(attacked_nampla->name);
    log_string(" went from ");
    log_int(attacked_nampla->ma_base / 10);
    log_char('.');
    log_int(attacked_nampla->ma_base % 10);
    log_string(" to ");
    attacked_nampla->ma_base = new_ma;
    log_int(new_ma / 10);
    log_char('.');
    log_int(new_ma % 10);
    log_string(".\n");

    attacked_nampla->pop_units = new_pop;

    for (i = 0; i < MAX_ITEMS; i++) {
        n = (percent_damage * attacked_nampla->item_quantity[i]) / 100;
        if (n > 0) {
            attacked_nampla->item_quantity[i] -= n;
            log_string("        ");
            log_long(n);
            log_char(' ');
            log_string(item_name[i]);
            if (n > 1) {
                log_string("s were");
            } else {
                log_string(" was");
            }
            log_string(" destroyed.\n");
        }
    }

    n = (percent_damage * (long) attacked_nampla->shipyards) / 100;
    if (n > 0) {
        attacked_nampla->shipyards -= n;
        log_string("        ");
        log_long(n);
        log_string(" shipyard");
        if (n > 1) {
            log_string("s were");
        } else {
            log_string(" was");
        }
        log_string(" also destroyed.\n");
    }

    check_population(attacked_nampla);
}

void do_germ_warfare(int attacking_species, int defending_species, int defender_index, struct battle_data *bat,
                     struct action_data *act) {
    int i, attacker_BI, defender_BI, success_chance, num_bombs, success;
    long econ_units_from_looting;
    struct planet_data *planet;
    struct nampla_data *attacked_nampla;
    struct ship_data *sh;

    attacker_BI = c_species[attacking_species]->tech_level[BI];
    defender_BI = c_species[defending_species]->tech_level[BI];
    attacked_nampla = (struct nampla_data *) act->fighting_unit[defender_index];
    planet = planet_base + (long) attacked_nampla->planet_index;

    success_chance = 50 + (2 * (attacker_BI - defender_BI));
    success = FALSE;
    num_bombs = germ_bombs_used[attacking_species][defending_species];

    for (i = 0; i < num_bombs; i++) {
        if (rnd(100) <= success_chance) {
            success = TRUE;
            break;
        }
    }

    if (success) {
        log_string("        Unfortunately");
    } else {
        log_string("        Fortunately");
    }
    log_string(" for the ");
    log_string(c_species[defending_species]->name);
    log_string(" defenders of PL ");
    log_string(attacked_nampla->name);
    log_string(", the ");
    i = bat->spec_num[attacking_species];
    if (field_distorted[attacking_species]) {
        log_int(distorted(i));
    } else {
        log_string(c_species[attacking_species]->name);
    }
    log_string(" attackers ");

    if (!success) {
        log_string("failed");
        if (num_bombs <= 0) {
            log_string(" because they didn't have any germ warfare bombs");
        }
        log_string("!\n");
        return;
    }

    log_string("succeeded, using ");
    log_int(num_bombs);
    log_string(" germ warfare bombs. The defenders were wiped out!\n");

    /* Take care of looting. */
    econ_units_from_looting = (long) attacked_nampla->mi_base + (long) attacked_nampla->ma_base;

    if (attacked_nampla->status & HOME_PLANET) {
        if (c_species[defending_species]->hp_original_base < econ_units_from_looting) {
            c_species[defending_species]->hp_original_base = econ_units_from_looting;
        }
        econ_units_from_looting *= 5;
    }

    if (econ_units_from_looting > 0) {
        /* Check if there's enough memory for a new interspecies transaction. */
        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\nRan out of memory! MAX_TRANSACTIONS is too small!\n\n");
            exit(-1);
        }
        i = num_transactions++;

        /* Define this transaction. */
        transaction[i].type = LOOTING_EU_TRANSFER;
        transaction[i].donor = bat->spec_num[defending_species];
        transaction[i].recipient = bat->spec_num[attacking_species];
        transaction[i].value = econ_units_from_looting;
        strcpy(transaction[i].name1, c_species[defending_species]->name);
        strcpy(transaction[i].name2, c_species[attacking_species]->name);
        strcpy(transaction[i].name3, attacked_nampla->name);
    }

    /* Finish off defenders. */
    attacked_nampla->mi_base = 0;
    attacked_nampla->ma_base = 0;
    attacked_nampla->IUs_to_install = 0;
    attacked_nampla->AUs_to_install = 0;
    attacked_nampla->pop_units = 0;
    attacked_nampla->siege_eff = 0;
    attacked_nampla->shipyards = 0;
    attacked_nampla->hiding = 0;
    attacked_nampla->hidden = 0;
    attacked_nampla->use_on_ambush = 0;

    for (i = 0; i < MAX_ITEMS; i++) {
        attacked_nampla->item_quantity[i] = 0;
    }

    /* Reset status word. */
    if (attacked_nampla->status & HOME_PLANET) {
        attacked_nampla->status = HOME_PLANET;
    } else {
        attacked_nampla->status = COLONY;
    }

    /* Delete any ships that were under construction on the planet. */
    sh = c_ship[defending_species] - 1;
    for (i = 0; i < c_species[defending_species]->num_ships; i++) {
        ++sh;
        if (sh->x != attacked_nampla->x) {
            continue;
        }
        if (sh->y != attacked_nampla->y) {
            continue;
        }
        if (sh->z != attacked_nampla->z) {
            continue;
        }
        if (sh->pn != attacked_nampla->pn) {
            continue;
        }
        delete_ship(sh);
    }
}

/* The following routine will return TRUE if a round of combat actually occurred. Otherwise, it will return false. */
int do_round(char option, int round_number, struct battle_data *bat, struct action_data *act) {
    int i, j, n, unit_index, combat_occurred, total_shots,
            attacker_index, defender_index, found, chance_to_hit,
            attacker_ml, attacker_gv, defender_ml, target_index[MAX_SHIPS],
            num_targets, header_printed, num_sp, fj_chance, shields_up,
            FDs_were_destroyed, di[3], start_unit, current_species,
            this_is_a_hijacking;
    long aux_shield_power, units_destroyed, tons, percent_decrease,
            damage_done, damage_to_ship, damage_to_shields, op1, op2,
            original_cost, recycle_value, economic_units;
    char attacker_name[64], defender_name[64];
    struct species_data *attacking_species, *defending_species;
    struct ship_data *sh, *attacking_ship, *defending_ship;
    struct nampla_data *attacking_nampla, *defending_nampla;

    /* Clear out x_attacked_y and germ_bombs_used arrays.  They will be used to log who bombed who, or how many GWs were used. */
    num_sp = bat->num_species_here;
    for (i = 0; i < num_sp; i++) {
        for (j = 0; j < num_sp; j++) {
            x_attacked_y[i][j] = FALSE;
            germ_bombs_used[i][j] = 0;
        }
    }

    /* If a species has ONLY non-combatants left, then let them fight. */
    start_unit = 0;
    total_shots = 0;
    current_species = act->fighting_species_index[0];
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++) {
        if (act->fighting_species_index[unit_index] != current_species) {
            if (total_shots == 0) {
                /* Convert all non-combatants, if any, to combatants. */
                for (i = start_unit; i < unit_index; i++) {
                    if (act->unit_type[i] == SHIP) {
                        sh = (struct ship_data *) act->fighting_unit[i];
                        sh->special = 0;
                    }
                }
            }
            start_unit = unit_index;
            total_shots = 0;
        }

        n = act->num_shots[unit_index];
        if (act->surprised[unit_index]) {
            n = 0;
        }
        if (act->unit_type[unit_index] == SHIP) {
            sh = (struct ship_data *) act->fighting_unit[unit_index];
            if (sh->special == NON_COMBATANT) {
                n = 0;
            }
        }
        total_shots += n;
    }

    /* Determine total number of shots for all species present. */
    total_shots = 0;
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++) {
        n = act->num_shots[unit_index];
        if (act->surprised[unit_index]) {
            n = 0;
        }
        if (act->unit_type[unit_index] == SHIP) {
            sh = (struct ship_data *) act->fighting_unit[unit_index];
            if (sh->special == NON_COMBATANT) {
                n = 0;
            }
        }
        act->shots_left[unit_index] = n;
        total_shots += n;
    }

    /* Handle all shots. */
    header_printed = FALSE;
    combat_occurred = FALSE;
    //int infiniteShotsGuard = total_shots;
    while (total_shots > 0) {
        /* check to make sure we arent in infinite loop
         * that usually happens when there are shots remaining
         * but the side with the shots has no more ships left*/
        for (i = 0; i < act->num_units_fighting; ++i) {
            attacking_ship = (struct ship_data *) act->fighting_unit[i];
            if (attacking_ship->age > 49 || attacking_ship->status == FORCED_JUMP ||
                attacking_ship->status == JUMPED_IN_COMBAT ||
                (attacking_ship->special == NON_COMBATANT && option != GERM_WARFARE)) {
                total_shots -= act->shots_left[i];
                act->shots_left[i] = 0;
            }
        }
        //// second test to prevent infinite loop due to the shot counter not being decremented.
        //if (total_shots > infiniteShotsGuard) {
        //    total_shots = infiniteShotsGuard;
        //}
        //infiniteShotsGuard--;

        /* Determine who fires next. */
        attacker_index = rnd(act->num_units_fighting) - 1;
        if (act->unit_type[attacker_index] == SHIP) {
            attacking_ship = (struct ship_data *) act->fighting_unit[attacker_index];
            i = act->fighting_species_index[attacker_index];
            ignore_field_distorters = !field_distorted[i];
            sprintf(attacker_name, "%s", ship_name(attacking_ship));
            ignore_field_distorters = FALSE;

            /* Check if ship can fight. */
            if (attacking_ship->age > 49) {
                continue;
            }
            if (attacking_ship->status == FORCED_JUMP) {
                continue;
            }
            if (attacking_ship->status == JUMPED_IN_COMBAT) {
                continue;
            }
            if (attacking_ship->special == NON_COMBATANT &&
                option != GERM_WARFARE) {
                continue;
            }
        } else {
            attacking_nampla = (struct nampla_data *) act->fighting_unit[attacker_index];
            sprintf(attacker_name, "PL %s", attacking_nampla->name);
            /* Check if planet still has defenses. */
            if (attacking_nampla->item_quantity[PD] == 0) { continue; }
        }

        /* Make sure attacker is not someone who is being taken by surprise this round. */
        if (act->surprised[attacker_index]) {
            continue;
        }

        /* Find an enemy. */
        num_targets = 0;
        i = act->fighting_species_index[attacker_index];
        attacker_ml = c_species[i]->tech_level[ML];
        attacker_gv = c_species[i]->tech_level[GV];
        for (defender_index = 0; defender_index < act->num_units_fighting; defender_index++) {
            j = act->fighting_species_index[defender_index];
            if (!bat->enemy_mine[i][j]) {
                continue;
            }

            if (act->unit_type[defender_index] == SHIP) {
                defending_ship = (struct ship_data *) act->fighting_unit[defender_index];
                if (defending_ship->age > 49) {
                    /* Already destroyed. */
                    continue;
                }
                if (defending_ship->status == FORCED_JUMP) {
                    continue;
                }
                if (defending_ship->status == JUMPED_IN_COMBAT) {
                    continue;
                }
                if (defending_ship->special == NON_COMBATANT) {
                    continue;
                }
            } else {
                defending_nampla = (struct nampla_data *) act->fighting_unit[defender_index];

                if (defending_nampla->item_quantity[PD] == 0 && option == PLANET_ATTACK) {
                    continue;
                }
            }

            target_index[num_targets] = defender_index;
            ++num_targets;
        }

        if (num_targets == 0) {
            /* Attacker has no enemies left. */
            total_shots -= act->shots_left[attacker_index];
            act->shots_left[attacker_index] = 0;
            continue;
        }

        /* Randomly choose a target. Choose the toughest of four. */
        defender_index = target_index[rnd(num_targets) - 1];
        op1 = (long) act->num_shots[defender_index] * act->weapon_damage[defender_index];
        di[0] = target_index[rnd(num_targets) - 1];
        di[1] = target_index[rnd(num_targets) - 1];
        di[2] = target_index[rnd(num_targets) - 1];
        for (i = 0; i < 3; i++) {
            op2 = (long) act->num_shots[di[i]] * act->weapon_damage[di[i]];
            if (op2 > op1) {
                defender_index = di[i];
                op1 = op2;
            }
        }

        j = act->fighting_species_index[defender_index];
        defender_ml = c_species[j]->tech_level[ML];

        if (act->unit_type[defender_index] == SHIP) {
            defending_ship = (struct ship_data *) act->fighting_unit[defender_index];
            ignore_field_distorters = !field_distorted[j];
            sprintf(defender_name, "%s", ship_name(defending_ship));
            ignore_field_distorters = FALSE;
        } else {
            defending_nampla = (struct nampla_data *) act->fighting_unit[defender_index];
            sprintf(defender_name, "PL %s", defending_nampla->name);
        }

        /* Print round number. */
        if (!header_printed) {
            log_string("      Now doing round ");
            log_int(round_number);
            log_string(":\n");
            header_printed = TRUE;
        }
        int attackerGvMl = attacker_gv + attacker_ml;
        if (attackerGvMl <= 0) {
            attackerGvMl = 1;
        }
        /* Check if attacker has any forced jump units.
         * The attacker will place more emphasis on the use of these devices if he emphasizes gravitics technology over military technology. */
        fj_chance = 50 * attacker_gv / attackerGvMl;
        if (rnd(100) < fj_chance
            && act->unit_type[attacker_index] == SHIP
            && act->unit_type[defender_index] == SHIP) {
            if (forced_jump_units_used(attacker_index, defender_index, &total_shots, bat, act)) {
                combat_occurred = TRUE;
                continue;
            }
        }

        if (act->shots_left[attacker_index] == 0) {
            continue;
        }

        /* Since transports generally avoid combat, there is only a 10% chance that they will be targeted, unless they are being explicitly targeted. */
        i = act->fighting_species_index[attacker_index];
        j = act->fighting_species_index[defender_index];
        if (act->unit_type[defender_index] == SHIP && defending_ship->class == TR &&
            bat->special_target[i] != TARGET_TRANSPORTS && rnd(10) != 5) {
            continue;
        }

        /* If a special target has been specified, then there is a 75% chance that it will be attacked if it is available. */
        if (bat->special_target[i] && rnd(100) < 76) {
            if (bat->special_target[i] == TARGET_PDS) {
                if (act->unit_type[defender_index] != SHIP) {
                    goto fire;
                } else {
                    continue;
                }
            }

            if (act->unit_type[defender_index] != SHIP) {
                continue;
            }

            if (bat->special_target[i] == TARGET_STARBASES && defending_ship->class != BA) {
                continue;
            }
            if (bat->special_target[i] == TARGET_TRANSPORTS && defending_ship->class != TR) {
                continue;
            }
            if (bat->special_target[i] == TARGET_WARSHIPS) {
                if (defending_ship->class == TR) {
                    continue;
                }
                if (defending_ship->class == BA) {
                    continue;
                }
            }
        }

        fire:
        /* Update counts. */
        --act->shots_left[attacker_index];
        --total_shots;

        /* Since transports generally avoid combat, there is only a 10% chance that they will attack. */
        if (act->unit_type[attacker_index] == SHIP && attacking_ship->class == TR && option != GERM_WARFARE &&
            rnd(10) != 5) {
            continue;
        }

        /* Fire! */
        combat_occurred = TRUE;
        log_string("        ");
        log_string(attacker_name);
        log_string(" fires on ");
        log_string(defender_name);
        if (act->unit_type[defender_index] == NAMPLA) {
            log_string(" defenses");
        }

        int combinedMl = attacker_ml + defender_ml;
        if (combinedMl <= 0) {
            combinedMl = 1;
        }
        /* Get hit probability.
         * The basic chance to hit is 1.5 times attackers ML over the sum of attacker's and defender's ML.
         * Double this value if defender is surprised. */
        chance_to_hit = (150 * attacker_ml) / combinedMl;
        if (act->surprised[defender_index]) {
            chance_to_hit *= 2;
            shields_up = FALSE;
        } else {
            shields_up = TRUE;
        }

        /* If defending ship is field-distorted, chance-to-hit is reduced by 25%. */
        j = act->fighting_species_index[defender_index];
        if (act->unit_type[defender_index] == SHIP && field_distorted[j] &&
            defending_ship->item_quantity[FD] == defending_ship->tonnage) {
            chance_to_hit = (3 * chance_to_hit) / 4;
        }
        if (chance_to_hit > 98) {
            chance_to_hit = 98;
        }
        if (chance_to_hit < 2) {
            chance_to_hit = 2;
        }

        /* Adjust for age. */
        if (act->unit_type[attacker_index] == SHIP) {
            chance_to_hit -= (2 * attacking_ship->age * chance_to_hit) / 100;
        }

        /* Calculate damage that shot will do if it hits. */
        damage_done = act->weapon_damage[attacker_index];
        damage_done += ((26 - rnd(51)) * damage_done) / 100;

        /* Take care of attempted annihilation and sieges. */
        if (option == PLANET_BOMBARDMENT || option == GERM_WARFARE || option == SIEGE) {
            /* Indicate the action that was attempted against this nampla. */
            if (option == SIEGE) {
                act->unit_type[defender_index] = BESIEGED_NAMPLA;
            } else {
                act->unit_type[defender_index] = GENOCIDE_NAMPLA;
            }

            /* Indicate who attacked who. */
            i = act->fighting_species_index[attacker_index];
            j = act->fighting_species_index[defender_index];
            x_attacked_y[i][j] = TRUE;

            /* Update bombardment damage. */
            if (option == PLANET_BOMBARDMENT) {
                act->bomb_damage[defender_index] += damage_done;
            } else if (option == GERM_WARFARE) {
                if (act->unit_type[attacker_index] == SHIP) {
                    germ_bombs_used[i][j] += attacking_ship->item_quantity[GW];
                    attacking_ship->item_quantity[GW] = 0;
                } else {
                    germ_bombs_used[i][j] += attacking_nampla->item_quantity[GW];
                    attacking_nampla->item_quantity[GW] = 0;
                }
            }

            continue;
        }

        /* Check if shot hit. */
        if (rnd(100) <= chance_to_hit) {
            log_string(" and hits!\n");
        } else {
            log_string(" and misses!\n");
            continue;
        }

        /* Subtract damage from defender's shields, if they're up. */
        damage_to_ship = 0;
        if (shields_up) {
            if (act->unit_type[defender_index] == SHIP) {
                damage_to_shields = ((long) defending_ship->dest_y * damage_done) / 100;
                damage_to_ship = damage_done - damage_to_shields;
                act->shield_strength_left[defender_index] -= damage_to_shields;

                /* Calculate percentage of shields left. */
                if (act->shield_strength_left[defender_index] > 0) {
                    long int defenderShieldStrength = act->shield_strength[defender_index];
                    if (defenderShieldStrength <= 0) {
                        defenderShieldStrength = 1;
                    }
                    defending_ship->dest_y =
                            (100L * act->shield_strength_left[defender_index]) / defenderShieldStrength;
                } else {
                    defending_ship->dest_y = 0;
                }
            } else {
                /* Planetary defenses. */
                act->shield_strength_left[defender_index] -= damage_done;
            }
        }

        /* See if it got through shields. */
        units_destroyed = 0;
        percent_decrease = 0;
        if (!shields_up || act->shield_strength_left[defender_index] < 0 || damage_to_ship > 0) {
            /* Get net damage to ship or PDs. */
            if (shields_up) {
                if (act->unit_type[defender_index] == SHIP) {
                    /* Total damage to ship is direct damage plus damage that shields could not absorb. */
                    damage_done = damage_to_ship;
                    if (act->shield_strength_left[defender_index] < 0) {
                        damage_done -= act->shield_strength_left[defender_index];
                    }
                } else {
                    damage_done = -act->shield_strength_left[defender_index];
                }
            }

            long defenderShieldStrength = act->shield_strength[defender_index];
            if (defenderShieldStrength <= 0) {
                defenderShieldStrength = 1;
            }

            percent_decrease = (50L * damage_done) / defenderShieldStrength;

            percent_decrease += ((rnd(51) - 26) * percent_decrease) / 100;
            if (percent_decrease > 100) { percent_decrease = 100; }

            if (act->unit_type[defender_index] == SHIP) {
                defending_ship->age += percent_decrease / 2;
                units_destroyed = (defending_ship->age > 49);
            } else {
                units_destroyed = (percent_decrease * act->original_age_or_PDs[defender_index]) / 100L;
                if (units_destroyed > defending_nampla->item_quantity[PD]) {
                    units_destroyed = defending_nampla->item_quantity[PD];
                }
                if (units_destroyed < 1) { units_destroyed = 1; }
                defending_nampla->item_quantity[PD] -= units_destroyed;
            }

            if (act->shield_strength_left[defender_index] < 0) {
                act->shield_strength_left[defender_index] = 0;
            }
        }

        /* See if this is a hijacking. */
        i = act->fighting_species_index[attacker_index];
        j = act->fighting_species_index[defender_index];
        if (bat->enemy_mine[i][j] == 2 && (option == DEEP_SPACE_FIGHT || option == PLANET_ATTACK)) {
            this_is_a_hijacking = TRUE;
        } else {
            this_is_a_hijacking = FALSE;
        }

        attacking_species = c_species[i];
        defending_species = c_species[j];

        /* Report if anything was destroyed. */
        FDs_were_destroyed = FALSE;
        if (units_destroyed) {
            if (act->unit_type[defender_index] == SHIP) {
                log_summary = TRUE;
                log_string("        ");
                log_string(defender_name);
                if (this_is_a_hijacking) {
                    log_string(" was successfully hijacked and will generate ");

                    if (defending_ship->class == TR || defending_ship->type == STARBASE) {
                        original_cost = ship_cost[defending_ship->class] * defending_ship->tonnage;
                    } else {
                        original_cost = ship_cost[defending_ship->class];
                    }

                    if (defending_ship->type == SUB_LIGHT) {
                        original_cost = (3 * original_cost) / 4;
                    }

                    if (defending_ship->status == UNDER_CONSTRUCTION) {
                        recycle_value = (original_cost - (long) defending_ship->remaining_cost) / 2;
                    } else {
                        recycle_value = (3 * original_cost * (60 - act->original_age_or_PDs[defender_index])) / 200;
                    }

                    economic_units = recycle_value;

                    for (i = 0; i < MAX_ITEMS; i++) {
                        j = defending_ship->item_quantity[i];
                        if (j > 0) {
                            if (i == TP) {
                                long int techLevel_2x = 2L * (long) defending_species->tech_level[BI];
                                if (techLevel_2x <= 0) {
                                    techLevel_2x = 1;
                                }
                                recycle_value = (j * item_cost[i]) / techLevel_2x;
                            } else if (i == RM) {
                                recycle_value = j / 5;
                            } else {
                                recycle_value = (j * item_cost[i]) / 2;
                            }

                            economic_units += recycle_value;
                        }
                    }

                    attacking_species->econ_units += economic_units;

                    log_long(economic_units);
                    log_string(" economic units for the hijackers.\n");
                } else {
                    log_string(" was destroyed.\n");
                }

                for (i = 0; i < MAX_ITEMS; i++) {
                    if (defending_ship->item_quantity[i] > 0) {
                        /* If this is a hijacking of a field-distorted ship, we want the true name of the hijacked species to be announced, but we don't want any cargo to be destroyed. */
                        if (i == FD) {
                            FDs_were_destroyed = TRUE;
                        }
                        if (!this_is_a_hijacking) {
                            defending_ship->item_quantity[FD] = 0;
                        }
                    }
                }
                log_to_file = FALSE;
                if (this_is_a_hijacking) {
                    log_string("          The hijacker was ");
                } else {
                    log_string("          The killing blow was delivered by ");
                }
                log_string(attacker_name);
                log_string(".\n");
                log_to_file = TRUE;
                log_summary = FALSE;

                total_shots -= act->shots_left[defender_index];
                act->shots_left[defender_index] = 0;
                act->num_shots[defender_index] = 0;
            } else {
                log_summary = TRUE;
                log_string("        ");
                log_int(units_destroyed);
                if (units_destroyed > 1) {
                    log_string(" PDs on PL ");
                } else {
                    log_string(" PD on PL ");
                }
                log_string(defending_nampla->name);
                if (units_destroyed > 1) {
                    log_string(" were destroyed by ");
                } else {
                    log_string(" was destroyed by ");
                }

                log_string(attacker_name);
                log_string(".\n");

                if (defending_nampla->item_quantity[PD] == 0) {
                    total_shots -= act->shots_left[defender_index];
                    act->shots_left[defender_index] = 0;
                    act->num_shots[defender_index] = 0;
                    log_string("        All planetary defenses have been destroyed on ");
                    log_string(defender_name);
                    log_string("!\n");
                }
                log_summary = FALSE;
            }
        } else if (percent_decrease > 0 && !this_is_a_hijacking && act->unit_type[defender_index] == SHIP) {
            /* See if anything carried by the ship was also destroyed. */
            for (i = 0; i < MAX_ITEMS; i++) {
                j = defending_ship->item_quantity[i];
                if (j > 0) {
                    j = (percent_decrease * j) / 100;
                    if (j > 0) {
                        defending_ship->item_quantity[i] -= j;
                        if (i == FD) { FDs_were_destroyed = TRUE; }
                    }
                }
            }
        }

        j = act->fighting_species_index[defender_index];
        if (FDs_were_destroyed && field_distorted[j] && defending_ship->dest_x == 0) {
            /* Reveal the true name of the ship and the owning species. */
            log_summary = TRUE;
            if (this_is_a_hijacking) {
                log_string("        Hijacking of ");
            } else {
                log_string("        Damage to ");
            }
            log_string(defender_name);
            log_string(" caused collapse of distortion field. Real name of ship is ");
            log_string(ship_name(defending_ship));
            log_string(" owned by SP ");
            log_string(defending_species->name);
            log_string(".\n");
            log_summary = FALSE;
            defending_ship->dest_x = 127;    /* Ship is now exposed. */
        }
    }

    /* No more surprises. */
    for (i = 0; i < act->num_units_fighting; i++) {
        act->surprised[i] = FALSE;
    }

    return combat_occurred;
}

void do_siege(struct battle_data *bat, struct action_data *act) {
    int a, d, i, attacker_index, defender_index, attacking_species_number, defending_species_number;
    struct nampla_data *defending_nampla;
    struct ship_data *attacking_ship;
    struct species_data *defending_species, *attacking_species;

    for (defender_index = 0; defender_index < act->num_units_fighting; defender_index++) {
        if (act->unit_type[defender_index] == BESIEGED_NAMPLA) {
            defending_nampla = (struct nampla_data *) act->fighting_unit[defender_index];
            defending_nampla->siege_eff = TRUE;
            d = act->fighting_species_index[defender_index];
            defending_species = c_species[d];
            defending_species_number = bat->spec_num[d];
            for (attacker_index = 0; attacker_index < act->num_units_fighting; attacker_index++) {
                if (act->unit_type[attacker_index] == SHIP) {
                    attacking_ship = (struct ship_data *) act->fighting_unit[attacker_index];
                    a = act->fighting_species_index[attacker_index];
                    if (x_attacked_y[a][d]) {
                        attacking_species = c_species[a];
                        attacking_species_number = bat->spec_num[a];
                        /* Check if there's enough memory for a new interspecies transaction. */
                        if (num_transactions == MAX_TRANSACTIONS) {
                            fprintf(stderr, "\nRan out of memory! MAX_TRANSACTIONS is too small!\n\n");
                            exit(-1);
                        }
                        i = num_transactions++;
                        /* Define this transaction. */
                        transaction[i].type = BESIEGE_PLANET;
                        transaction[i].x = defending_nampla->x;
                        transaction[i].y = defending_nampla->y;
                        transaction[i].z = defending_nampla->z;
                        transaction[i].pn = defending_nampla->pn;
                        transaction[i].number1 = attacking_species_number;
                        strcpy(transaction[i].name1, attacking_species->name);
                        transaction[i].number2 = defending_species_number;
                        strcpy(transaction[i].name2, defending_species->name);
                        strcpy(transaction[i].name3, attacking_ship->name);
                    }
                }
            }
        }
    }
    log_string("      Only those ships that actually remain in the system will take part in the siege.\n");
}

/* The following routine will fill "act" with ship and nampla data necessary
   for an action; i.e., number of shots per round, damage done per shot,
   total shield power, etc. Note that this routine always restores shields
   completely. It is assumed that a sufficient number of rounds passes
   between actions of a battle to completely regenerate shields.

   The routine will return TRUE if the action can take place, otherwise FALSE.
*/
int fighting_params(char option, char location, struct battle_data *bat, struct action_data *act) {
    char x, y, z, pn;
    int i, j, found, type, num_sp, unit_index;
    int species_index;
    int ship_index;
    int nampla_index;
    int sp1, sp2, use_this_ship, n_shots;
    int engage_option, engage_location, attacking_ships_here;
    int defending_ships_here, attacking_pds_here, defending_pds_here;
    int num_fighting_units;
    short tons;
    long ml, ls, unit_power, offensive_power, defensive_power;
    struct ship_data *sh;
    struct nampla_data *nam;

    /* Add fighting units to "act" arrays. At the same time, check if
	a fight of the current option type will occur at the current
	location. */
    num_fighting_units = 0;
    x = bat->x;
    y = bat->y;
    z = bat->z;
    attacking_ML = 0;
    defending_ML = 0;
    attacking_ships_here = FALSE;
    defending_ships_here = FALSE;
    attacking_pds_here = FALSE;
    defending_pds_here = FALSE;
    deep_space_defense = FALSE;
    num_sp = bat->num_species_here;

    for (species_index = 0; species_index < num_sp; ++species_index) {
        /* Check which ships can take part in fight. */
        sh = c_ship[species_index] - 1;
        for (ship_index = 0; ship_index < c_species[species_index]->num_ships; ship_index++) {
            ++sh;
            use_this_ship = FALSE;

            if (sh->x != x) { continue; }
            if (sh->y != y) { continue; }
            if (sh->z != z) { continue; }
            if (sh->pn == 99) { continue; }
            if (sh->age > 49) { continue; }
            if (sh->status == UNDER_CONSTRUCTION) { continue; }
            if (sh->status == FORCED_JUMP) { continue; }
            if (sh->status == JUMPED_IN_COMBAT) { continue; }
            if (sh->class == TR && sh->pn != location && option != GERM_WARFARE) { continue; }
            if (disbanded_species_ship(species_index, sh)) { continue; }
            if (option == SIEGE || option == PLANET_BOMBARDMENT) {
                if (sh->special == NON_COMBATANT) { continue; }
            }

            for (i = 0; i < bat->num_engage_options[species_index]; i++) {
                engage_option = bat->engage_option[species_index][i];
                engage_location = bat->engage_planet[species_index][i];

                switch (engage_option) {
                    case DEFENSE_IN_PLACE:
                        if (sh->pn != location) { break; }
                        defending_ships_here = TRUE;
                        use_this_ship = TRUE;
                        break;

                    case DEEP_SPACE_DEFENSE:
                        if (option != DEEP_SPACE_FIGHT) { break; }
                        if (sh->class == BA && sh->pn != 0) { break; }
                        defending_ships_here = TRUE;
                        use_this_ship = TRUE;
                        deep_space_defense = TRUE;
                        if (c_species[species_index]->tech_level[ML] > defending_ML) {
                            defending_ML = c_species[species_index]->tech_level[ML];
                        }
                        break;

                    case PLANET_DEFENSE:
                        if (location != engage_location) { break; }
                        if (sh->class == BA && sh->pn != location) { break; }
                        defending_ships_here = TRUE;
                        use_this_ship = TRUE;
                        break;

                    case DEEP_SPACE_FIGHT:
                        if (option != DEEP_SPACE_FIGHT) { break; }
                        if (sh->class == BA && sh->pn != 0) { break; }
                        if (c_species[species_index]->tech_level[ML] > defending_ML) {
                            defending_ML = c_species[species_index]->tech_level[ML];
                        }
                        defending_ships_here = TRUE;
                        attacking_ships_here = TRUE;
                        use_this_ship = TRUE;
                        break;

                    case PLANET_ATTACK:
                    case PLANET_BOMBARDMENT:
                    case GERM_WARFARE:
                    case SIEGE:
                        if (sh->class == BA && sh->pn != location) { break; }
                        if (sh->class == TR && option == SIEGE) { break; }
                        if (option == DEEP_SPACE_FIGHT) {
                            /* There are two possibilities here: 1. outsiders
                            are attacking locals, or 2. locals are attacking
                            locals. If (1), we want outsiders to first fight
                            in deep space. If (2), locals will not first
                            fight in deep space (unless other explicit
                            orders were given). The case is (2) if current
                            species has a planet here. */

                            found = FALSE;
                            for (nampla_index = 0;
                                 nampla_index < c_species[species_index]->num_namplas; nampla_index++) {
                                nam = c_nampla[species_index] + nampla_index;

                                if (nam->x != x) { continue; }
                                if (nam->y != y) { continue; }
                                if (nam->z != z) { continue; }
                                if ((nam->status & POPULATED) == 0) { continue; }

                                found = TRUE;
                                break;
                            }

                            if (!found) {
                                attacking_ships_here = TRUE;
                                use_this_ship = TRUE;
                                if (c_species[species_index]->tech_level[ML] > attacking_ML) {
                                    attacking_ML = c_species[species_index]->tech_level[ML];
                                }
                                break;
                            }
                        }
                        if (option != engage_option
                            && option != PLANET_ATTACK) {
                            break;
                        }
                        if (location != engage_location) { break; }
                        attacking_ships_here = TRUE;
                        use_this_ship = TRUE;
                        break;

                    default:
                        fprintf(stderr, "\n\n\tInternal error #1 in fighting_params - invalid engage option!\n\n");
                        exit(-1);
                }
            }

            add_ship:
            if (use_this_ship) {
                /* Add data for this ship to action array. */
                act->fighting_species_index[num_fighting_units] = species_index;
                act->unit_type[num_fighting_units] = SHIP;
                act->fighting_unit[num_fighting_units] = (char *) sh;
                act->original_age_or_PDs[num_fighting_units] = sh->age;
                ++num_fighting_units;
            }
        }

        /* Check which namplas can take part in fight. */
        nam = c_nampla[species_index] - 1;
        for (nampla_index = 0; nampla_index < c_species[species_index]->num_namplas; nampla_index++) {
            ++nam;

            if (nam->x != x) { continue; }
            if (nam->y != y) { continue; }
            if (nam->z != z) { continue; }
            if (nam->pn != location) { continue; }
            if ((nam->status & POPULATED) == 0) { continue; }
            if (nam->status & DISBANDED_COLONY) { continue; }

            /* This planet has been targeted for some kind of attack. In
            most cases, one species will attack a planet inhabited by
            another species. However, it is also possible for two or
            more species to have colonies on the SAME planet, and for
            one to attack the other. */

            for (i = 0; i < bat->num_engage_options[species_index]; i++) {
                engage_option = bat->engage_option[species_index][i];
                engage_location = bat->engage_planet[species_index][i];
                if (engage_location != location) { continue; }

                switch (engage_option) {
                    case DEFENSE_IN_PLACE:
                    case DEEP_SPACE_DEFENSE:
                    case PLANET_DEFENSE:
                    case DEEP_SPACE_FIGHT:
                        break;

                    case PLANET_ATTACK:
                    case PLANET_BOMBARDMENT:
                    case GERM_WARFARE:
                    case SIEGE:
                        if (option != engage_option
                            && option != PLANET_ATTACK) {
                            break;
                        }
                        if (nam->item_quantity[PD] > 0) {
                            attacking_pds_here = TRUE;
                        }
                        break;

                    default:
                        fprintf(stderr, "\n\n\tInternal error #2 in fighting_params - invalid engage option!\n\n");
                        exit(-1);
                }
            }

            if (nam->item_quantity[PD] > 0) { defending_pds_here = TRUE; }

            /* Add data for this nampla to action array. */
            act->fighting_species_index[num_fighting_units] = species_index;
            act->unit_type[num_fighting_units] = NAMPLA;
            act->fighting_unit[num_fighting_units] = (char *) nam;
            act->original_age_or_PDs[num_fighting_units] = nam->item_quantity[PD];
            ++num_fighting_units;
        }
    }

    /* Depending on option, see if the right combination of combatants
	are present. */
    switch (option) {
        case DEEP_SPACE_FIGHT:
            if (!attacking_ships_here || !defending_ships_here) { return FALSE; }
            break;

        case PLANET_ATTACK:
        case PLANET_BOMBARDMENT:
            if (!attacking_ships_here && !attacking_pds_here) { return FALSE; }
            break;

        case SIEGE:
        case GERM_WARFARE:
            if (!attacking_ships_here) { return FALSE; }
            break;

        default:
            fprintf(stderr, "\n\n\tInternal error #3 in fighting_params - invalid engage option!\n\n");
            exit(-1);
    }

    /* There is at least one attacker and one defender here. See if they
	are enemies. */
    for (i = 0; i < num_fighting_units; i++) {
        sp1 = act->fighting_species_index[i];
        for (j = 0; j < num_fighting_units; j++) {
            sp2 = act->fighting_species_index[j];
            if (bat->enemy_mine[sp1][sp2]) { goto next_step; }
        }
    }

    return FALSE;

    next_step:

    act->num_units_fighting = num_fighting_units;

    /* Determine number of shots, shield power and weapons power for
	all combatants. */
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++) {
        type = act->unit_type[unit_index];
        if (type == SHIP) {
            sh = (struct ship_data *) act->fighting_unit[unit_index];
            tons = sh->tonnage;
        } else {
            nam = (struct nampla_data *) act->fighting_unit[unit_index];
            tons = nam->item_quantity[PD] / 200;
            if (tons < 1 && nam->item_quantity[PD] > 0) { tons = 1; }
        }

        species_index = act->fighting_species_index[unit_index];

        unit_power = power(tons);
        offensive_power = unit_power;
        defensive_power = unit_power;

        if (type == SHIP) {
            if (sh->class == TR) {
                /* Transports are not designed for combat. */
                offensive_power /= 10;
                defensive_power /= 10;
            } else if (sh->class != BA) {
                /* Add auxiliary shield generator contribution, if any. */
                tons = 5;
                for (i = SG1; i <= SG9; i++) {
                    if (sh->item_quantity[i] > 0) {
                        defensive_power +=
                                (long) sh->item_quantity[i] * power(tons);
                    }
                    tons += 5;
                }

                /* Add auxiliary gun unit contribution, if any. */
                tons = 5;
                for (i = GU1; i <= GU9; i++) {
                    if (sh->item_quantity[i] > 0) {
                        offensive_power +=
                                (long) sh->item_quantity[i] * power(tons);
                    }
                    tons += 5;
                }
            }

            /* Adjust for ship aging. */
            offensive_power -= ((long) sh->age * offensive_power) / 50;
            defensive_power -= ((long) sh->age * defensive_power) / 50;
        }

        /* Adjust values for tech levels. */
        ml = c_species[species_index]->tech_level[ML];
        ls = c_species[species_index]->tech_level[LS];
        offensive_power += (ml * offensive_power) / 50;
        defensive_power += (ls * defensive_power) / 50;

        /* Adjust values if this species is hijacking anyone. */
        if (bat->hijacker[species_index] && (option == DEEP_SPACE_FIGHT
                                             || option == PLANET_ATTACK)) {
            offensive_power /= 4;
            defensive_power /= 4;
        }

        /* Get number of shots per round. */
        n_shots = (offensive_power / 1500) + 1;
        if (ml == 0 || offensive_power == 0) { n_shots = 0; }
        if (n_shots > 5) { n_shots = 5; }
        act->num_shots[unit_index] = n_shots;
        act->shots_left[unit_index] = n_shots;

        /* Get damage per shot. */
        if (n_shots > 0) {
            act->weapon_damage[unit_index] = (2 * offensive_power) / n_shots;
        } else {
            act->weapon_damage[unit_index] = 0;
        }

        /* Do defensive shields. */
        act->shield_strength[unit_index] = defensive_power;
        if (type == SHIP) {
            /* Adjust for results of previous action, if any. "dest_y"
            contains the percentage of shields that remained at end
            of last action. */
            defensive_power = ((long) sh->dest_y * defensive_power) / 100L;
        }
        act->shield_strength_left[unit_index] = defensive_power;

        /* Set bomb damage to zero in case this is planet bombardment or
            germ warfare. */
        act->bomb_damage[unit_index] = 0;

        /* Set flag for individual unit if species can be surprised. */
        if (bat->can_be_surprised[species_index]) {
            act->surprised[unit_index] = TRUE;
        } else {
            act->surprised[unit_index] = FALSE;
        }
    }

    return TRUE;    /* There will be a fight here. */
}

/* This routine will return TRUE if forced jump or misjump units are used, even if they fail.
 * It will return FALSE if the attacker has none or not enough. */
int forced_jump_units_used(int attacker_index, int defender_index, int *total_shots, struct battle_data *bat,
                           struct action_data *act) {
    int i, att_sp_index, def_sp_index, attacker_gv, defender_gv, type, fj_num, fm_num, number, success_chance, failure;
    char x, y, z;
    struct ship_data *attacking_ship, *defending_ship;

    /* Make sure attacking unit is a starbase. */
    attacking_ship = (struct ship_data *) act->fighting_unit[attacker_index];
    if (attacking_ship->type != STARBASE) {
        return FALSE;
    }
    /* See if attacker has any forced jump units. */
    fj_num = attacking_ship->item_quantity[FJ];
    fm_num = attacking_ship->item_quantity[FM];
    if (fj_num == 0 && fm_num == 0) {
        return FALSE;
    }
    /* If both types are being carried, choose one randomly. */
    if (fj_num > 0 && fm_num > 0) {
        if (rnd(2) == 1) {
            type = FJ;
            number = fj_num;
        } else {
            type = FM;
            number = fm_num;
        }
    } else if (fj_num > 0) {
        type = FJ;
        number = fj_num;
    } else {
        type = FM;
        number = fm_num;
    }

    /* Get gravitics tech levels. */
    att_sp_index = act->fighting_species_index[attacker_index];
    attacker_gv = c_species[att_sp_index]->tech_level[GV];

    def_sp_index = act->fighting_species_index[defender_index];
    defender_gv = c_species[def_sp_index]->tech_level[GV];

    /* Check if sufficient units are available. */
    defending_ship = (struct ship_data *) act->fighting_unit[defender_index];
    if (number < defending_ship->tonnage) {
        return FALSE;
    }

    /* Make sure defender is not a starbase. */
    if (defending_ship->type == STARBASE) {
        return FALSE;
    }

    /* Calculate percent chance of success. */
    success_chance = 2 * ((number - defending_ship->tonnage) + (attacker_gv - defender_gv));

    /* See if it worked. */
    failure = rnd(100) > success_chance;

    log_summary = !failure;

    log_string("        ");
    log_string(ship_name(attacking_ship));
    log_string(" attempts to use ");
    log_string(item_name[type]);
    log_string("s against ");

    ignore_field_distorters = !field_distorted[def_sp_index];
    log_string(ship_name(defending_ship));
    ignore_field_distorters = FALSE;

    if (failure) {
        log_string(", but fails.\n");
        return TRUE;
    }

    log_string(", and succeeds!\n");
    log_summary = FALSE;

    /* Determine destination. */
    if (type == FM) {
        /* Destination is totally random. */
        x = rnd(100) - 1;
        y = rnd(100) - 1;
        z = rnd(100) - 1;
    } else {
        /* Random location close to battle. */
        i = 3;
        while (i == 3) {
            i = rnd(5);
        }
        x = bat->x + i - 3;
        if (x < 0) {
            x = 0;
        }

        i = 3;
        while (i == 3) {
            i = rnd(5);
        }
        y = bat->y + i - 3;
        if (y < 0) {
            y = 0;
        }

        i = 3;
        while (i == 3) {
            i = rnd(5);
        }
        z = bat->z + i - 3;
        if (z < 0) {
            z = 0;
        }
    }
    defending_ship->dest_x = x;
    defending_ship->dest_y = y;
    defending_ship->dest_z = z;

    /* Make sure this ship can no longer take part in the battle. */
    defending_ship->status = FORCED_JUMP;
    defending_ship->pn = -1;
    *total_shots -= act->shots_left[defender_index];
    act->shots_left[defender_index] = 0;
    act->num_shots[defender_index] = 0;

    return TRUE;
}

void regenerate_shields(struct action_data *act) {
    int i, species_index, unit_index;
    long ls, max_shield_strength, percent;
    struct ship_data *sh;
    /* Shields are regenerated by 5 + LS/10 percent per round. */
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++) {
        species_index = act->fighting_species_index[unit_index];
        ls = c_species[species_index]->tech_level[LS];
        max_shield_strength = act->shield_strength[unit_index];
        percent = (ls / 10L) + 5L;
        act->shield_strength_left[unit_index] += (percent * max_shield_strength) / 100L;
        if (act->shield_strength_left[unit_index] > max_shield_strength) {
            act->shield_strength_left[unit_index] = max_shield_strength;
        }
    }
}

/* This routine will check all fighting ships and see if any wish to
 * withdraw. If so, it will set the ship's status to JUMPED_IN_COMBAT.
 * The actual jump will be handled by the Jump program. */
void withdrawal_check(struct battle_data *bat, struct action_data *act) {
    int i, old_trunc;
    int ship_index;
    int species_index;
    int percent_loss, num_ships_gone[MAX_SPECIES], num_ships_total[MAX_SPECIES];
    char withdraw_age;
    struct ship_data *sh;

    for (i = 0; i < MAX_SPECIES; i++) {
        num_ships_gone[i] = 0;
        num_ships_total[i] = 0;
    }

    old_trunc = truncate_name;    /* Show age of ship here. */
    truncate_name = FALSE;

    /* Compile statistics and handle individual ships that must leave. */
    for (ship_index = 0; ship_index < act->num_units_fighting; ship_index++) {
        if (act->unit_type[ship_index] != SHIP) {
            continue;
        }

        sh = (struct ship_data *) act->fighting_unit[ship_index];
        species_index = act->fighting_species_index[ship_index];
        ++num_ships_total[species_index];

        if (sh->status == JUMPED_IN_COMBAT) {
            /* Already withdrawn. */
            ++num_ships_gone[species_index];
            continue;
        }
        if (sh->status == FORCED_JUMP) {
            /* Forced to leave. */
            ++num_ships_gone[species_index];
            continue;
        }
        if (sh->age > 49) {
            /* Already destroyed. */
            ++num_ships_gone[species_index];
            continue;
        }
        if (sh->type != FTL) {
            continue;
        }    /* Ship can't jump. */

        if (sh->class == TR) {
            withdraw_age = bat->transport_withdraw_age[species_index];
            if (withdraw_age == 0) {
                /* Transports will withdraw only when entire fleet withdraws. */
                continue;
            }
        } else {
            withdraw_age = bat->warship_withdraw_age[species_index];
        }

        if (sh->age > withdraw_age) {
            act->num_shots[ship_index] = 0;
            act->shots_left[ship_index] = 0;
            sh->pn = 0;

            ignore_field_distorters = !field_distorted[species_index];

            fprintf(log_file, "        %s jumps away from the battle.\n", ship_name(sh));
            fprintf(summary_file, "        %s jumps away from the battle.\n", ship_name(sh));

            ignore_field_distorters = FALSE;

            sh->dest_x = bat->haven_x[species_index];
            sh->dest_y = bat->haven_y[species_index];
            sh->dest_z = bat->haven_z[species_index];

            sh->status = JUMPED_IN_COMBAT;

            ++num_ships_gone[species_index];
        }
    }

    /* Now check if a fleet has reached its limit. */
    for (ship_index = 0; ship_index < act->num_units_fighting; ship_index++) {
        if (act->unit_type[ship_index] != SHIP) {
            continue;
        }

        sh = (struct ship_data *) act->fighting_unit[ship_index];
        species_index = act->fighting_species_index[ship_index];

        if (sh->type != FTL) {
            /* Ship can't jump. */
            continue;
        }
        if (sh->status == JUMPED_IN_COMBAT) {
            /* Already withdrawn. */
            continue;
        }
        if (sh->status == FORCED_JUMP) {
            /* Already gone. */
            continue;
        }
        if (sh->age > 49) {
            /* Already destroyed. */
            continue;
        }

        if (bat->fleet_withdraw_percentage[species_index] == 0) {
            percent_loss = 101;        /* Always withdraw immediately. */
        } else {
            percent_loss = (100 * num_ships_gone[species_index]) / num_ships_total[species_index];
        }

        if (percent_loss > bat->fleet_withdraw_percentage[species_index]) {
            act->num_shots[ship_index] = 0;
            act->shots_left[ship_index] = 0;
            sh->pn = 0;

            ignore_field_distorters = !field_distorted[species_index];

            fprintf(log_file, "        %s jumps away from the battle.\n", ship_name(sh));
            fprintf(summary_file, "        %s jumps away from the battle.\n", ship_name(sh));

            ignore_field_distorters = FALSE;

            sh->dest_x = bat->haven_x[species_index];
            sh->dest_y = bat->haven_y[species_index];
            sh->dest_z = bat->haven_z[species_index];

            sh->status = JUMPED_IN_COMBAT;
        }
    }

    truncate_name = old_trunc;
}
