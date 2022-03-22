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
#include <sys/stat.h>
#include "command.h"
#include "commandvars.h"
#include "dev_log.h"
#include "do.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "intercept.h"
#include "jumpvars.h"
#include "locationvars.h"
#include "log.h"
#include "logvars.h"
#include "money.h"
#include "nampla.h"
#include "namplavars.h"
#include "productionvars.h"
#include "planet.h"
#include "planetio.h"
#include "planetvars.h"
#include "ship.h"
#include "shipvars.h"
#include "species.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "star.h"
#include "stario.h"
#include "starvars.h"
#include "transaction.h"
#include "transactionio.h"


void do_AMBUSH_command(void) {
    int n, status;
    long cost;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get amount to spend. */
    status = get_value();
    if (status == 0 || value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid or missing amount.\n");
        return;
    }
    if (value == 0) { value = balance; }
    if (value == 0) { return; }
    cost = value;

    /* Check if planet is under siege. */
    if (nampla->siege_eff != 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Besieged planet cannot ambush!\n");
        return;
    }

    /* Check if sufficient funds are available. */
    if (check_bounced(cost)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    /* Increment amount spent on ambush. */
    nampla->use_on_ambush += cost;

    /* Log transaction. */
    log_string("    Spent ");
    log_long(cost);
    log_string(" in preparation for an ambush.\n");
}


void do_ALLY_command(void) {
    int i, array_index, bit_number;
    long bit_mask;

    /* Get name of species that is being declared an ally. */
    if (!get_species_name()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid or missing argument in ALLY command.\n");
        return;
    }

    /* Get array index and bit mask. */
    array_index = (g_spec_number - 1) / 32;
    bit_number = (g_spec_number - 1) % 32;
    bit_mask = 1 << bit_number;

    /* Check if we've met this species and make sure it is not an enemy. */
    if ((species->contact[array_index] & bit_mask) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't declare alliance with a species you haven't met.\n");
        return;
    }

    /* Set/clear the appropriate bit. */
    species->ally[array_index] |= bit_mask;    /* Set ally bit. */
    species->enemy[array_index] &= ~bit_mask;    /* Clear enemy bit. */

    /* Log the result. */
    log_string("    Alliance was declared with ");
    if (bit_mask == 0) {
        log_string("ALL species");
    } else {
        log_string("SP ");
        log_string(g_spec_name);
    }
    log_string(".\n");
}

void do_BASE_command(void) {
    int i, n, found, su_count, original_count, item_class, name_length;
    int unused_ship_available, new_tonnage, max_tonnage, new_starbase;
    int source_is_a_planet, age_new;
    char x, y, z, pn, upper_ship_name[32], *original_line_pointer;
    struct nampla_data *source_nampla;
    struct ship_data *source_ship, *starbase, *unused_ship;

    /* Get number of starbase units to use. */
    i = get_value();
    if (i == 0) {
        value = 0;
    } else {
        /* Make sure value is meaningful. */
        if (value < 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid SU count in BASE command.\n");
            return;
        }
    }
    su_count = value;
    original_count = su_count;

    /* Get source of starbase units. */
    original_line_pointer = input_line_pointer;
    if (!get_transfer_point()) {
        input_line_pointer = original_line_pointer;
        fix_separator();    /* Check for missing comma or tab. */
        if (!get_transfer_point()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid source location in BASE command.\n");
            return;
        }
    }

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS) {
        source_is_a_planet = FALSE;
        source_ship = ship;

        if (source_ship->status == UNDER_CONSTRUCTION) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s is still under construction!\n",
                    ship_name(source_ship));
            return;
        }

        if (source_ship->status == FORCED_JUMP
            || source_ship->status == JUMPED_IN_COMBAT) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
            return;
        }

        if (su_count == 0) { su_count = source_ship->item_quantity[SU]; }
        if (su_count == 0) { return; }
        if (source_ship->item_quantity[SU] < su_count) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s does not enough starbase units!\n",
                    ship_name(source_ship));
            return;
        }

        x = source_ship->x;
        y = source_ship->y;
        z = source_ship->z;
        pn = source_ship->pn;
    } else    /* Source is a planet. */
    {
        source_is_a_planet = TRUE;
        source_nampla = nampla;

        if (su_count == 0) { su_count = source_nampla->item_quantity[SU]; }
        if (su_count == 0) { return; }
        if (source_nampla->item_quantity[SU] < su_count) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! PL %s does not have enough starbase units!\n",
                    source_nampla->name);
            return;
        }

        x = source_nampla->x;
        y = source_nampla->y;
        z = source_nampla->z;
        pn = source_nampla->pn;
    }

    /* Get starbase name. */
    if (get_class_abbr() != SHIP_CLASS || abbr_index != BA) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid starbase name.\n");
        return;
    }
    name_length = get_name();

    /* Search all ships for name. */
    found = FALSE;
    ship = ship_base - 1;
    unused_ship_available = FALSE;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
        ++ship;

        if (ship->pn == 99) {
            unused_ship_available = TRUE;
            unused_ship = ship;
            continue;
        }

        /* Make upper case copy of ship name. */
        for (i = 0; i < 32; i++) {
            upper_ship_name[i] = toupper(ship->name[i]);
        }

        /* Compare names. */
        if (strcmp(upper_ship_name, upper_name) == 0) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        if (ship->type != STARBASE) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship name already in use.\n");
            return;
        }

        if (ship->x != x || ship->y != y || ship->z != z) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Starbase units and starbase are not at same X Y Z.\n");
            return;
        }
        starbase = ship;
        new_starbase = FALSE;
    } else {
        if (unused_ship_available) {
            starbase = unused_ship;
        } else {
            /* Make sure we have enough memory for new starbase. */
            if (num_new_ships[species_index] == NUM_EXTRA_SHIPS) {
                fprintf(stderr, "\n\n\tInsufficient memory for new starbase!\n\n");
                exit(-1);
            }
            ++num_new_ships[species_index];
            starbase = ship_base + (int) species->num_ships;
            ++species->num_ships;
            delete_ship(starbase);        /* Initialize everything to zero. */
        }

        /* Initialize non-zero data for new ship. */
        strcpy(starbase->name, original_name);
        starbase->x = x;
        starbase->y = y;
        starbase->z = z;
        starbase->pn = pn;
        if (pn == 0) {
            starbase->status = IN_DEEP_SPACE;
        } else {
            starbase->status = IN_ORBIT;
        }
        starbase->type = STARBASE;
        starbase->class = BA;
        starbase->tonnage = 0;
        starbase->age = -1;
        starbase->remaining_cost = 0;

        /* Everything else was set to zero in above call to 'delete_ship'. */

        new_starbase = TRUE;
    }

    /* Make sure that starbase is not being built in the deep space section
	of a star system .*/
    if (starbase->pn == 0) {
        star = star_base - 1;
        for (i = 0; i < num_stars; i++) {
            ++star;

            if (star->x != x) { continue; }
            if (star->y != y) { continue; }
            if (star->z != z) { continue; }

            if (star->num_planets < 1) { break; }

            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Starbase cannot be built in deep space if there are planets available!\n");
            if (new_starbase) { delete_ship(starbase); }
            return;
        }
    }

    /* Make sure species can build a starbase of this size. */
    max_tonnage = species->tech_level[MA] / 2;
    new_tonnage = starbase->tonnage + su_count;
    if (new_tonnage > max_tonnage && original_count == 0) {
        su_count = max_tonnage - starbase->tonnage;
        if (su_count < 1) {
            if (new_starbase) { delete_ship(starbase); }
            return;
        }
        new_tonnage = starbase->tonnage + su_count;
    }

    if (new_tonnage > max_tonnage) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Maximum allowable tonnage exceeded.\n");
        if (new_starbase) { delete_ship(starbase); }
        return;
    }

    /* Finish up and log results. */
    log_string("    ");
    if (starbase->tonnage == 0) {
        log_string(ship_name(starbase));
        log_string(" was constructed.\n");
    } else {
        /* Weighted average. */
        starbase->age = ((starbase->age * starbase->tonnage) - su_count) / new_tonnage;
        log_string("Size of ");
        log_string(ship_name(starbase));
        log_string(" was increased to ");
        log_string(commas(10000L * (long) new_tonnage));
        log_string(" tons.\n");
    }

    starbase->tonnage = new_tonnage;

    if (source_is_a_planet) {
        source_nampla->item_quantity[SU] -= su_count;
    } else {
        source_ship->item_quantity[SU] -= su_count;
    }
}


void do_BUILD_command(int continuing_construction, int interspecies_construction) {
    int i, n, class, critical_tech, found, name_length,
            siege_effectiveness, cost_given, new_ship, max_tonnage,
            tonnage_increase, alien_number, cargo_on_board,
            unused_nampla_available, unused_ship_available, capacity,
            pop_check_needed, contact_word_number, contact_bit_number,
            already_notified[MAX_SPECIES];

    char upper_ship_name[32], *commas(), *src, *dest,
            *original_line_pointer;

    long cost, cost_argument, unit_cost, num_items, pop_reduction,
            premium, total_cost, original_num_items, contact_mask,
            max_funds_available;

    struct species_data *recipient_species;
    struct nampla_data *recipient_nampla, *unused_nampla,
            *destination_nampla, *temp_nampla;
    struct ship_data *recipient_ship, *unused_ship;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get ready if planet is under siege. */
    if (nampla->siege_eff < 0) {
        siege_effectiveness = -nampla->siege_eff;
    } else {
        siege_effectiveness = nampla->siege_eff;
    }

    /* Get species name and make appropriate tests if this is an interspecies construction order. */
    if (interspecies_construction) {
        original_line_pointer = input_line_pointer;
        if (!get_species_name()) {
            /* Check for missing comma or tab after species name. */
            input_line_pointer = original_line_pointer;
            fix_separator();
            if (!get_species_name()) {
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", original_line);
                fprintf(log_file, "!!! Invalid species name.\n");
                return;
            }
        }
        recipient_species = &spec_data[g_spec_number - 1];

        if (species->tech_level[MA] < 25) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! MA tech level must be at least 25 to do interspecies construction.\n");
            return;
        }

        /* Check if we've met this species and make sure it is not an enemy. */
        contact_word_number = (g_spec_number - 1) / 32;
        contact_bit_number = (g_spec_number - 1) % 32;
        contact_mask = 1 << contact_bit_number;
        if ((species->contact[contact_word_number] & contact_mask) == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! You can't do interspecies construction for a species you haven't met.\n");
            return;
        }
        if (species->enemy[contact_word_number] & contact_mask) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! You can't do interspecies construction for an ENEMY.\n");
            return;
        }
    }

    /* Get number of items to build. */
    i = get_value();

    if (i == 0) {
        goto build_ship;
    }    /* Not an item. */
    num_items = value;
    original_num_items = value;

    /* Get class of item. */
    class = get_class_abbr();

    if (class != ITEM_CLASS || abbr_index == RM) {
        /* Players sometimes accidentally use "MI" for "IU" or "MA" for "AU". */
        if (class == TECH_ID && abbr_index == MI) {
            abbr_index = IU;
        } else if (class == TECH_ID && abbr_index == MA) {
            abbr_index = AU;
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid item class.\n");
            return;
        }
    }
    class = abbr_index;

    if (interspecies_construction) {
        if (class == PD || class == CU) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! You cannot build CUs or PDs for another species.\n");
            return;
        }
    }

    /* Make sure species knows how to build this item. */
    critical_tech = item_critical_tech[class];
    if (species->tech_level[critical_tech] < item_tech_requirment[class]) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Insufficient tech level to build item.\n");
        return;
    }

    /* Get cost of item. */
    if (class == TP) {
        /* Terraforming plant. */
        unit_cost = item_cost[class] / species->tech_level[critical_tech];
    } else {
        unit_cost = item_cost[class];
    }

    if (num_items == 0) { num_items = balance / unit_cost; }
    if (num_items == 0) { return; }

    /* Make sure item count is meaningful. */
    if (num_items < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Meaningless item count.\n");
        return;
    }

    /* Make sure there is enough available population. */
    pop_reduction = 0;
    if (class == CU || class == PD) {
        if (nampla->pop_units < num_items) {
            if (original_num_items == 0) {
                num_items = nampla->pop_units;
                if (num_items == 0) { return; }
            } else {
                if (nampla->pop_units > 0) {
                    fprintf(log_file, "! WARNING: %s", original_line);
                    fprintf(log_file, "! Insufficient available population units. Substituting %d for %ld.\n",
                            nampla->pop_units, num_items);
                    num_items = nampla->pop_units;
                } else {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", original_line);
                    fprintf(log_file, "!!! Insufficient available population units.\n");
                    return;
                }
            }
        }
        pop_reduction = num_items;
    }

    /* Calculate total cost and see if planet has enough money. */
    do_cost:
    cost = num_items * unit_cost;
    if (interspecies_construction) {
        premium = (cost + 9) / 10;
    } else {
        premium = 0;
    }

    cost += premium;

    if (check_bounced(cost)) {
        if (interspecies_construction && original_num_items == 0) {
            --num_items;
            if (num_items < 1) { return; }
            goto do_cost;
        }

        max_funds_available = species->econ_units;
        if (max_funds_available > EU_spending_limit) {
            max_funds_available = EU_spending_limit;
        }
        max_funds_available += balance;

        num_items = max_funds_available / unit_cost;
        if (interspecies_construction) { num_items -= (num_items + 9) / 10; }

        if (num_items > 0) {
            fprintf(log_file, "! WARNING: %s", original_line);
            fprintf(log_file, "! Insufficient funds. Substituting %ld for %ld.\n",
                    num_items, original_num_items);
            goto do_cost;
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Insufficient funds to execute order.\n");
            return;
        }
    }

    /* Update planet inventory. */
    nampla->item_quantity[class] += num_items;
    nampla->pop_units -= pop_reduction;

    /* Log what was produced. */
    log_string("    ");
    log_long(num_items);
    log_char(' ');
    log_string(item_name[class]);

    if (num_items > 1) {
        log_string("s were");
    } else {
        log_string(" was");
    }

    if (first_pass && class == PD && siege_effectiveness > 0) {
        log_string(" scheduled for production despite the siege.\n");
        return;
    } else {
        log_string(" produced");
        if (interspecies_construction) {
            log_string(" for SP ");
            log_string(recipient_species->name);
        }
    }

    if (unit_cost != 1 || premium != 0) {
        log_string(" at a cost of ");
        log_long(cost);
    }

    /* Check if planet is under siege and if production of planetary defenses was detected. */
    if (class == PD && rnd(100) <= siege_effectiveness) {
        log_string(". However, they were detected and destroyed by the besiegers!!!\n");
        nampla->item_quantity[PD] = 0;

        /* Make sure we don't notify the same species more than once. */
        for (i = 0; i < MAX_SPECIES; i++) { already_notified[i] = FALSE; }

        for (i = 0; i < num_transactions; i++) {
            /* Find out who is besieging this planet. */
            if (transaction[i].type != BESIEGE_PLANET) { continue; }
            if (transaction[i].x != nampla->x) { continue; }
            if (transaction[i].y != nampla->y) { continue; }
            if (transaction[i].z != nampla->z) { continue; }
            if (transaction[i].pn != nampla->pn) { continue; }
            if (transaction[i].number2 != species_number) { continue; }

            alien_number = transaction[i].number1;

            if (already_notified[alien_number - 1]) { continue; }

            /* Define a 'detection' transaction. */
            if (num_transactions == MAX_TRANSACTIONS) {
                fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                exit(-1);
            }

            n = num_transactions++;
            transaction[n].type = DETECTION_DURING_SIEGE;
            transaction[n].value = 3;    /* Construction of PDs. */
            strcpy(transaction[n].name1, nampla->name);
            strcpy(transaction[n].name3, species->name);
            transaction[n].number3 = alien_number;

            already_notified[alien_number - 1] = TRUE;
        }
        return;
    }

    if (!interspecies_construction) {
        /* Get destination of transfer, if any. */
        pop_check_needed = FALSE;
        temp_nampla = nampla;
        found = get_transfer_point();
        destination_nampla = nampla;
        nampla = temp_nampla;
        if (!found) { goto done_transfer; }

        if (abbr_type == SHIP_CLASS)    /* Destination is 'ship'. */
        {
            if (ship->x != nampla->x || ship->y != nampla->y || ship->z != nampla->z ||
                ship->status == UNDER_CONSTRUCTION) {
                goto done_transfer;
            }

            if (ship->class == TR) {
                capacity = (10 + ((int) ship->tonnage / 2)) * (int) ship->tonnage;
            } else if (ship->class == BA) {
                capacity = 10 * ship->tonnage;
            } else {
                capacity = ship->tonnage;
            }

            for (i = 0; i < MAX_ITEMS; i++) {
                capacity -= ship->item_quantity[i] * item_carry_capacity[i];
            }

            n = num_items;
            if (num_items * item_carry_capacity[class] > capacity) {
                num_items = capacity / item_carry_capacity[class];
            }

            ship->item_quantity[class] += num_items;
            nampla->item_quantity[class] -= num_items;
            log_string(" and ");
            if (n > num_items) {
                log_long(num_items);
                log_string(" of them ");
            }
            if (num_items == 1) {
                log_string("was");
            } else {
                log_string("were");
            }
            log_string(" transferred to ");
            log_string(ship_name(ship));

            if (class == CU && num_items > 0) {
                if (nampla == nampla_base) {
                    ship->loading_point = 9999;    /* Home planet. */
                } else {
                    ship->loading_point = (nampla - nampla_base);
                }
            }
        } else {
            /* Destination is 'destination_nampla'. */
            if (destination_nampla->x != nampla->x || destination_nampla->y != nampla->y ||
                destination_nampla->z != nampla->z) {
                goto done_transfer;
            }

            if (nampla->siege_eff != 0) { goto done_transfer; }
            if (destination_nampla->siege_eff != 0) { goto done_transfer; }

            destination_nampla->item_quantity[class] += num_items;
            nampla->item_quantity[class] -= num_items;
            log_string(" and transferred to PL ");
            log_string(destination_nampla->name);
            pop_check_needed = TRUE;
        }

        done_transfer:

        log_string(".\n");

        if (pop_check_needed) { check_population(destination_nampla); }

        return;
    }

    log_string(".\n");

    /* Check if recipient species has a nampla at this location. */
    found = FALSE;
    unused_nampla_available = FALSE;
    recipient_nampla = namp_data[g_spec_number - 1] - 1;
    for (i = 0; i < recipient_species->num_namplas; i++) {
        ++recipient_nampla;

        if (recipient_nampla->pn == 99) {
            unused_nampla = recipient_nampla;
            unused_nampla_available = TRUE;
        }

        if (recipient_nampla->x != nampla->x) { continue; }
        if (recipient_nampla->y != nampla->y) { continue; }
        if (recipient_nampla->z != nampla->z) { continue; }
        if (recipient_nampla->pn != nampla->pn) { continue; }

        found = TRUE;
        break;
    }

    if (!found) {
        /* Add new nampla to database for the recipient species. */
        if (unused_nampla_available) {
            recipient_nampla = unused_nampla;
        } else {
            ++num_new_namplas[species_index];
            if (num_new_namplas[species_index] > NUM_EXTRA_NAMPLAS) {
                fprintf(stderr, "\n\n\tInsufficient memory for new planet name in do_BUILD_command!\n");
                exit(-1);
            }
            recipient_nampla = namp_data[g_spec_number - 1] + recipient_species->num_namplas;
            recipient_species->num_namplas += 1;
            delete_nampla(recipient_nampla);    /* Set everything to zero. */
        }

        /* Initialize new nampla. */
        strcpy(recipient_nampla->name, nampla->name);
        recipient_nampla->x = nampla->x;
        recipient_nampla->y = nampla->y;
        recipient_nampla->z = nampla->z;
        recipient_nampla->pn = nampla->pn;
        recipient_nampla->planet_index = nampla->planet_index;
        recipient_nampla->status = COLONY;
    }

    /* Transfer the goods. */
    nampla->item_quantity[class] -= num_items;
    recipient_nampla->item_quantity[class] += num_items;
    data_modified[g_spec_number - 1] = TRUE;

    if (first_pass) { return; }

    /* Define transaction so that recipient will be notified. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    n = num_transactions++;
    transaction[n].type = INTERSPECIES_CONSTRUCTION;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = 1;    /* Items, not ships. */
    transaction[n].number1 = num_items;
    transaction[n].number2 = class;
    transaction[n].number3 = cost;
    strcpy(transaction[n].name1, species->name);
    strcpy(transaction[n].name2, recipient_nampla->name);

    return;


    build_ship:

    original_line_pointer = input_line_pointer;
    if (continuing_construction) {
        found = get_ship();
        if (!found) {
            /* Check for missing comma or tab after ship name. */
            input_line_pointer = original_line_pointer;
            fix_separator();
            found = get_ship();
        }

        if (found) { goto check_ship; }
        input_line_pointer = original_line_pointer;
    }

    class = get_class_abbr();

    if (class != SHIP_CLASS || tonnage < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid ship class.\n");
        return;
    }
    class = abbr_index;

    /* Get ship name. */
    name_length = get_name();
    if (name_length < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid ship name.\n");
        return;
    }

    /* Search all ships for name. */
    found = FALSE;
    ship = ship_base - 1;
    unused_ship_available = FALSE;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
        ++ship;

        if (ship->pn == 99) {
            unused_ship_available = TRUE;
            unused_ship = ship;
            continue;
        }

        /* Make upper case copy of ship name. */
        for (i = 0; i < 32; i++) {
            upper_ship_name[i] = toupper(ship->name[i]);
        }

        /* Compare names. */
        if (strcmp(upper_ship_name, upper_name) == 0) {
            found = TRUE;
            break;
        }
    }

    check_ship:

    if (found) {
        /* Check if BUILD was accidentally used instead of CONTINUE. */
        if ((ship->status == UNDER_CONSTRUCTION || ship->type == STARBASE) && ship->x == nampla->x &&
            ship->y == nampla->y && ship->z == nampla->z && ship->pn == nampla->pn) {
            continuing_construction = TRUE;
        }

        if ((ship->status != UNDER_CONSTRUCTION && ship->type != STARBASE) || (!continuing_construction)) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship name already in use.\n");
            return;
        }

        new_ship = FALSE;
    } else {
        /* If CONTINUE command was used, the player probably mis-spelled the name. */
        if (continuing_construction) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name.\n");
            return;
        }

        if (unused_ship_available) {
            ship = unused_ship;
        } else {
            /* Make sure we have enough memory for new ship. */
            if (num_new_ships[species_index] >= NUM_EXTRA_SHIPS) {
                if (num_new_ships[species_index] == 9999) { return; }

                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", original_line);
                fprintf(log_file, "!!! You cannot build more than %d ships per turn!\n", NUM_EXTRA_SHIPS);
                num_new_ships[species_index] = 9999;
                return;
            }
            new_ship = TRUE;
            ship = ship_base + (int) species->num_ships;
            /* Initialize everything to zero. */
            delete_ship(ship);
        }

        /* Initialize non-zero data for new ship. */
        strcpy(ship->name, original_name);
        ship->x = nampla->x;
        ship->y = nampla->y;
        ship->z = nampla->z;
        ship->pn = nampla->pn;
        ship->status = UNDER_CONSTRUCTION;
        if (class == BA) {
            ship->type = STARBASE;
            ship->status = IN_ORBIT;
        } else if (sub_light) {
            ship->type = SUB_LIGHT;
        } else {
            ship->type = FTL;
        }
        ship->class = class;
        ship->age = -1;
        if (ship->type != STARBASE) { ship->tonnage = tonnage; }
        ship->remaining_cost = ship_cost[class];
        if (ship->class == TR) {
            ship->remaining_cost = ship_cost[TR] * tonnage;
        }
        if (ship->type == SUB_LIGHT) {
            ship->remaining_cost = (3L * (long) ship->remaining_cost) / 4L;
        }
        ship->just_jumped = FALSE;

        /* Everything else was set to zero in above call to 'delete_ship'. */
    }

    /* Check if amount to spend was specified. */
    cost_given = get_value();
    cost = value;
    cost_argument = value;

    if (cost_given) {
        if (interspecies_construction && (ship->type != STARBASE)) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Amount to spend may not be specified.\n");
            return;
        }

        if (cost == 0) {
            cost = balance;
            if (ship->type == STARBASE) {
                if (cost % ship_cost[BA] != 0) {
                    cost = ship_cost[BA] * (cost / ship_cost[BA]);
                }
            }
            if (cost < 1) {
                if (new_ship) { delete_ship(ship); }
                return;
            }
        }

        if (cost < 1) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Amount specified is meaningless.\n");
            if (new_ship) { delete_ship(ship); }
            return;
        }

        if (ship->type == STARBASE) {
            if (cost % ship_cost[BA] != 0) {
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", original_line);
                fprintf(log_file, "!!! Amount spent on starbase must be multiple of %d.\n", ship_cost[BA]);
                if (new_ship) { delete_ship(ship); }
                return;
            }
        }
    } else {
        if (ship->type == STARBASE) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Amount to spend MUST be specified for starbase.\n");
            if (new_ship) { delete_ship(ship); }
            return;
        }

        cost = ship->remaining_cost;
    }

    /* Make sure species can build a ship of this size. */
    max_tonnage = species->tech_level[MA] / 2;
    if (ship->type == STARBASE) {
        tonnage_increase = cost / (long) ship_cost[BA];
        tonnage = ship->tonnage + tonnage_increase;
        if (tonnage > max_tonnage && cost_argument == 0) {
            tonnage_increase = max_tonnage - ship->tonnage;
            if (tonnage_increase < 1) { return; }
            tonnage = ship->tonnage + tonnage_increase;
            cost = tonnage_increase * (int) ship_cost[BA];
        }
    }

    if (tonnage > max_tonnage) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Maximum allowable tonnage exceeded.\n");
        if (new_ship) { delete_ship(ship); }
        return;
    }

    /* Make sure species has gravitics technology if this is an FTL ship. */
    if (ship->type == FTL && species->tech_level[GV] < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Gravitics tech needed to build FTL ship!\n");
        if (new_ship) { delete_ship(ship); }
        return;
    }

    /* Make sure amount specified is not an overpayment. */
    if (ship->type != STARBASE && cost > ship->remaining_cost) {
        cost = ship->remaining_cost;
    }

    /* Make sure planet has sufficient shipyards. */
    if (shipyard_capacity < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Shipyard capacity exceeded!\n");
        if (new_ship) { delete_ship(ship); }
        return;
    }

    /* Make sure there is enough money to pay for it. */
    premium = 0;
    if (interspecies_construction) {
        if (ship->class == TR || ship->type == STARBASE) {
            total_cost = ship_cost[ship->class] * tonnage;
        } else {
            total_cost = ship_cost[ship->class];
        }

        if (ship->type == SUB_LIGHT) {
            total_cost = (3 * total_cost) / 4;
        }

        premium = total_cost / 10;
        if (total_cost % 10) { ++premium; }
    }

    if (check_bounced(cost + premium)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        if (new_ship) { delete_ship(ship); }
        return;
    }

    --shipyard_capacity;

    /* Test if this is a starbase and if planet is under siege. */
    if (ship->type == STARBASE && siege_effectiveness > 0) {
        log_string("    Your attempt to build ");
        log_string(ship_name(ship));
        log_string(" was detected by the besiegers and the starbase was destroyed!!!\n");

        /* Make sure we don't notify the same species more than once. */
        for (i = 0; i < MAX_SPECIES; i++) { already_notified[i] = FALSE; }

        for (i = 0; i < num_transactions; i++) {
            /* Find out who is besieging this planet. */
            if (transaction[i].type != BESIEGE_PLANET) { continue; }
            if (transaction[i].x != nampla->x) { continue; }
            if (transaction[i].y != nampla->y) { continue; }
            if (transaction[i].z != nampla->z) { continue; }
            if (transaction[i].pn != nampla->pn) { continue; }
            if (transaction[i].number2 != species_number) { continue; }

            alien_number = transaction[i].number1;

            if (already_notified[alien_number - 1]) { continue; }

            /* Define a 'detection' transaction. */
            if (num_transactions == MAX_TRANSACTIONS) {
                fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                exit(-1);
            }

            n = num_transactions++;
            transaction[n].type = DETECTION_DURING_SIEGE;
            transaction[n].value = 2;    /* Construction of ship/starbase. */
            strcpy(transaction[n].name1, nampla->name);
            strcpy(transaction[n].name2, ship_name(ship));
            strcpy(transaction[n].name3, species->name);
            transaction[n].number3 = alien_number;

            already_notified[alien_number - 1] = TRUE;
        }

        delete_ship(ship);

        return;
    }

    /* Finish up and log results. */
    log_string("    ");
    if (ship->type == STARBASE) {
        if (ship->tonnage == 0) {
            log_string(ship_name(ship));
            log_string(" was constructed");
        } else {
            /* Weighted average. */
            ship->age = ((ship->age * ship->tonnage) - tonnage_increase) / tonnage;
            log_string("Size of ");
            log_string(ship_name(ship));
            log_string(" was increased to ");
            log_string(commas(10000L * (long) tonnage));
            log_string(" tons");
        }

        ship->tonnage = tonnage;
    } else {
        ship->remaining_cost -= cost;
        if (ship->remaining_cost == 0) {
            ship->status = ON_SURFACE;    /* Construction is complete. */
            if (continuing_construction) {
                if (first_pass && siege_effectiveness > 0) {
                    log_string("An attempt will be made to finish construction on ");
                } else {
                    log_string("Construction finished on ");
                }
                log_string(ship_name(ship));
                if (first_pass && siege_effectiveness > 0) {
                    log_string(" despite the siege");
                }
            } else {
                if (first_pass && siege_effectiveness > 0) {
                    log_string("An attempt will be made to construct ");
                }
                log_string(ship_name(ship));
                if (first_pass && siege_effectiveness > 0) {
                    log_string(" despite the siege");
                } else {
                    log_string(" was constructed");
                }
            }
        } else {
            if (continuing_construction) {
                if (first_pass && siege_effectiveness > 0) {
                    log_string("An attempt will be made to continue construction on ");
                } else {
                    log_string("Construction continued on ");
                }
                log_string(ship_name(ship));
                if (first_pass && siege_effectiveness > 0) {
                    log_string(" despite the siege");
                }
            } else {
                if (first_pass && siege_effectiveness > 0) {
                    log_string("An attempt will be made to start construction on ");
                } else {
                    log_string("Construction started on ");
                }
                log_string(ship_name(ship));
                if (first_pass && siege_effectiveness > 0) {
                    log_string(" despite the siege");
                }
            }
        }
    }
    log_string(" at a cost of ");
    log_long(cost + premium);

    if (interspecies_construction) {
        log_string(" for SP ");
        log_string(recipient_species->name);
    }

    log_char('.');

    if (new_ship && (!unused_ship_available)) {
        ++num_new_ships[species_index];
        ++species->num_ships;
    }

    /* Check if planet is under siege and if construction was detected. */
    if (!first_pass && rnd(100) <= siege_effectiveness) {
        log_string(" However, the work was detected by the besiegers and the ship was destroyed!!!");

        /* Make sure we don't notify the same species more than once. */
        for (i = 0; i < MAX_SPECIES; i++) { already_notified[i] = FALSE; }

        for (i = 0; i < num_transactions; i++) {
            /* Find out who is besieging this planet. */
            if (transaction[i].type != BESIEGE_PLANET) { continue; }
            if (transaction[i].x != nampla->x) { continue; }
            if (transaction[i].y != nampla->y) { continue; }
            if (transaction[i].z != nampla->z) { continue; }
            if (transaction[i].pn != nampla->pn) { continue; }
            if (transaction[i].number2 != species_number) { continue; }

            alien_number = transaction[i].number1;

            if (already_notified[alien_number - 1]) { continue; }

            /* Define a 'detection' transaction. */
            if (num_transactions == MAX_TRANSACTIONS) {
                fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                exit(-1);
            }

            n = num_transactions++;
            transaction[n].type = DETECTION_DURING_SIEGE;
            transaction[n].value = 2;    /* Construction of ship/starbase. */
            strcpy(transaction[n].name1, nampla->name);
            strcpy(transaction[n].name2, ship_name(ship));
            strcpy(transaction[n].name3, species->name);
            transaction[n].number3 = alien_number;

            already_notified[alien_number - 1] = TRUE;
        }

        /* Remove ship from inventory. */
        delete_ship(ship);
    }

    log_char('\n');

    if (!interspecies_construction) { return; }

    /* Transfer any cargo on the ship to the planet. */
    cargo_on_board = FALSE;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (ship->item_quantity[i] > 0) {
            nampla->item_quantity[i] += ship->item_quantity[i];
            ship->item_quantity[i] = 0;
            cargo_on_board = TRUE;
        }
    }
    if (cargo_on_board) {
        log_string("      Forgotten cargo on the ship was first transferred to the planet.\n");
    }

    /* Transfer the ship to the recipient species. */
    unused_ship_available = FALSE;
    recipient_ship = ship_data[g_spec_number - 1];
    for (i = 0; i < recipient_species->num_ships; i++) {
        if (recipient_ship->pn == 99) {
            unused_ship_available = TRUE;
            break;
        }

        ++recipient_ship;
    }

    if (!unused_ship_available) {
        /* Make sure we have enough memory for new ship. */
        if (num_new_ships[g_spec_number - 1] == NUM_EXTRA_SHIPS) {
            fprintf(stderr, "\n\n\tInsufficient memory for new recipient ship!\n\n");
            exit(-1);
        }
        recipient_ship = ship_data[g_spec_number - 1] + (int) recipient_species->num_ships;
        ++recipient_species->num_ships;
        ++num_new_ships[g_spec_number - 1];
    }

    /* Copy donor ship to recipient ship. */
    src = (char *) ship;
    dest = (char *) recipient_ship;
    for (i = 0; i < sizeof(struct ship_data); i++) {
        *dest++ = *src++;
    }

    recipient_ship->status = IN_ORBIT;

    data_modified[g_spec_number - 1] = TRUE;

    /* Delete donor ship. */
    delete_ship(ship);

    if (first_pass) { return; }

    /* Define transaction so that recipient will be notified. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    n = num_transactions++;
    transaction[n].type = INTERSPECIES_CONSTRUCTION;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = 2;    /* Ship, not items. */
    transaction[n].number3 = total_cost + premium;
    strcpy(transaction[n].name1, species->name);
    strcpy(transaction[n].name2, ship_name(recipient_ship));
}


void do_DEEP_command(void) {
    /* Get the ship. */
    char *original_line_pointer = input_line_pointer;
    int found = get_ship();
    if (!found) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        found = get_ship();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in ORBIT command.\n");
            return;
        }
    }
    if (ship->type == STARBASE) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! DEEP order may not be given for a starbase.\n");
        return;
    }
    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }
    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }
    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship(ship)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! This ship is salvage of a disbanded colony!\n");
        return;
    }

    /* Move the ship. */
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;

    /* Log result. */
    log_string("    ");
    log_string(ship_name(ship));
    log_string(" moved into deep space.\n");
}


void do_DESTROY_command(void) {
    int found;
    /* Get the ship. */
    correct_spelling_required = TRUE;
    found = get_ship();
    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid ship or starbase name in DESTROY command.\n");
        return;
    }
    /* Log result. */
    log_string("    ");
    log_string(ship_name(ship));
    if (first_pass) {
        log_string(" will be destroyed.\n");
        return;
    }
    log_string(" was destroyed.\n");
    delete_ship(ship);
}


void do_DEVELOP_command(void) {
    int i, num_CUs, num_AUs, num_IUs, more_args, load_transport,
            capacity, resort_colony, mining_colony, production_penalty,
            CUs_only;

    char c, *original_line_pointer, *tp;

    long n, ni, na, amount_to_spend, original_cost, max_funds_available,
            ls_needed, raw_material_units, production_capacity,
            colony_production, ib, ab, md, denom, reb, specified_max;

    struct planet_data *colony_planet, *home_planet;
    struct nampla_data *temp_nampla, *colony_nampla;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get default spending limit. */
    max_funds_available = species->econ_units;
    if (max_funds_available > EU_spending_limit) {
        max_funds_available = EU_spending_limit;
    }
    max_funds_available += balance;

    /* Get specified spending limit, if any. */
    specified_max = -1;
    if (get_value()) {
        if (value == 0) {
            max_funds_available = balance;
        } else if (value > 0) {
            specified_max = value;
            if (value <= max_funds_available) {
                max_funds_available = value;
            } else {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient funds. Substituting %ld for %ld.\n", max_funds_available, value);
                if (max_funds_available == 0) { return; }
            }
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid spending limit.\n");
            return;
        }
    }

    /* See if there are any more arguments. */
    tp = input_line_pointer;
    more_args = FALSE;
    while (c = *tp++) {
        if (c == ';' || c == '\n') { break; }
        if (c == ' ' || c == '\t') { continue; }
        more_args = TRUE;
        break;
    }

    if (!more_args) {
        /* Make sure planet is not a healthy home planet. */
        if (nampla->status & HOME_PLANET) {
            reb = species->hp_original_base - (nampla->mi_base + nampla->ma_base);
            if (reb > 0) {
                /* Home planet is recovering from bombing. */
                if (reb < max_funds_available) { max_funds_available = reb; }
            } else {
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! You can only DEVELOP a home planet if it is recovering from bombing.\n");
                return;
            }
        }

        /* No arguments. Order is for this planet. */
        num_CUs = nampla->pop_units;
        if (2 * num_CUs > max_funds_available) {
            num_CUs = max_funds_available / 2;
        }
        if (num_CUs <= 0) { return; }

        colony_planet = planet_base + (long) nampla->planet_index;
        ib = nampla->mi_base + nampla->IUs_to_install;
        ab = nampla->ma_base + nampla->AUs_to_install;
        md = colony_planet->mining_difficulty;

        denom = 100 + md;
        num_AUs =
                (100 * (num_CUs + ib) - (md * ab) + denom / 2) / denom;
        num_IUs = num_CUs - num_AUs;

        if (num_IUs < 0) {
            num_AUs = num_CUs;
            num_IUs = 0;
        }
        if (num_AUs < 0) {
            num_IUs = num_CUs;
            num_AUs = 0;
        }

        amount_to_spend = num_CUs + num_AUs + num_IUs;

        if (check_bounced(amount_to_spend)) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Internal error. Please notify GM!\n");
            return;
        }

        nampla->pop_units -= num_CUs;
        nampla->item_quantity[CU] += num_CUs;
        nampla->item_quantity[IU] += num_IUs;
        nampla->item_quantity[AU] += num_AUs;

        nampla->auto_IUs += num_IUs;
        nampla->auto_AUs += num_AUs;

        start_dev_log(num_CUs, num_IUs, num_AUs);
        log_string(".\n");

        check_population(nampla);

        return;
    }

    /* Get the planet to be developed. */
    temp_nampla = nampla;
    original_line_pointer = input_line_pointer;
    i = get_location();
    if (!i || nampla == NULL) {
        /* Check for missing comma or tab after source name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        i = get_location();
    }
    colony_nampla = nampla;
    nampla = temp_nampla;
    if (!i || colony_nampla == NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in DEVELOP command.\n");
        return;
    }

    /* Make sure planet is not a healthy home planet. */
    if (colony_nampla->status & HOME_PLANET) {
        reb = species->hp_original_base - (colony_nampla->mi_base + colony_nampla->ma_base);
        if (reb > 0) {
            /* Home planet is recovering from bombing. */
            if (reb < max_funds_available) { max_funds_available = reb; }
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! You can only DEVELOP a home planet if it is recovering from bombing.\n");
            return;
        }
    }

    /* Determine if its a mining or resort colony, and if it can afford to
	build its own IUs and AUs. Note that we cannot use nampla->status
	because it is not correctly set until the Finish program is run. */

    home_planet = planet_base + (long) nampla_base->planet_index;
    colony_planet = planet_base + (long) colony_nampla->planet_index;
    ls_needed = life_support_needed(species, home_planet, colony_planet);

    ni = colony_nampla->mi_base + colony_nampla->IUs_to_install;
    na = colony_nampla->ma_base + colony_nampla->AUs_to_install;

    if (ni > 0 && na == 0) {
        colony_production = 0;
        mining_colony = TRUE;
        resort_colony = FALSE;
    } else if (na > 0 && ni == 0 && ls_needed <= 6 && colony_planet->gravity <= home_planet->gravity) {
        colony_production = 0;
        resort_colony = TRUE;
        mining_colony = FALSE;
    } else {
        mining_colony = FALSE;
        resort_colony = FALSE;

        raw_material_units = (10L * (long) species->tech_level[MI] * ni) / (long) colony_planet->mining_difficulty;
        production_capacity = ((long) species->tech_level[MA] * na) / 10L;

        if (ls_needed == 0) {
            production_penalty = 0;
        } else {
            production_penalty = (100 * ls_needed) / species->tech_level[LS];
        }

        raw_material_units -= (production_penalty * raw_material_units) / 100;
        production_capacity -= (production_penalty * production_capacity) / 100;

        colony_production = (production_capacity > raw_material_units) ? raw_material_units : production_capacity;

        colony_production -= colony_nampla->IUs_needed + colony_nampla->AUs_needed;
        /* In case there is more than one DEVELOP order for this colony. */
    }

    /* See if there are more arguments. */
    tp = input_line_pointer;
    more_args = FALSE;
    while (c = *tp++) {
        if (c == ';' || c == '\n') { break; }
        if (c == ' ' || c == '\t') { continue; }
        more_args = TRUE;
        break;
    }

    if (more_args) {
        load_transport = TRUE;

        /* Get the ship to receive the cargo. */
        if (!get_ship()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship to be loaded does not exist!\n");
            return;
        }

        if (ship->class == TR) {
            capacity = (10 + ((int) ship->tonnage / 2)) * (int) ship->tonnage;
        } else if (ship->class == BA) {
            capacity = 10 * ship->tonnage;
        } else {
            capacity = ship->tonnage;
        }

        for (i = 0; i < MAX_ITEMS; i++) {
            capacity -= ship->item_quantity[i] * item_carry_capacity[i];
        }

        if (capacity <= 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s was already full and could take no more cargo!\n", ship_name(ship));
            return;
        }

        if (capacity > max_funds_available) {
            capacity = max_funds_available;
            if (max_funds_available != specified_max) {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient funds to completely fill %s!\n", ship_name(ship));
                fprintf(log_file, "! Will use all remaining funds (= %d).\n", capacity);
            }
        }
    } else {
        load_transport = FALSE;

        /* No more arguments. Order is for a colony in the same sector as the producing planet. */
        if (nampla->x != colony_nampla->x || nampla->y != colony_nampla->y || nampla->z != colony_nampla->z) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Colony and producing planet are not in the same sector.\n");
            return;
        }

        num_CUs = nampla->pop_units;
        if (2 * num_CUs > max_funds_available) {
            num_CUs = max_funds_available / 2;
        }
    }

    CUs_only = FALSE;
    if (mining_colony) {
        if (load_transport) {
            num_CUs = capacity / 2;
            if (num_CUs > nampla->pop_units) {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient available population! %d CUs are needed", num_CUs);
                num_CUs = nampla->pop_units;
                fprintf(log_file, " to fill ship but only %d can be built.\n", num_CUs);
            }
        }

        num_AUs = 0;
        num_IUs = num_CUs;
    } else if (resort_colony) {
        if (load_transport) {
            num_CUs = capacity / 2;
            if (num_CUs > nampla->pop_units) {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient available population! %d CUs are needed", num_CUs);
                num_CUs = nampla->pop_units;
                fprintf(log_file, " to fill ship but only %d can be built.\n", num_CUs);
            }
        }

        num_IUs = 0;
        num_AUs = num_CUs;
    } else {
        if (load_transport) {
            if (colony_production >= capacity) {
                /* Colony can build its own IUs and AUs. */
                num_CUs = capacity;
                CUs_only = TRUE;
            } else {
                /* Build IUs and AUs for the colony. */
                num_CUs = capacity / 2;
            }

            if (num_CUs > nampla->pop_units) {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient available population! %d CUs are needed", num_CUs);
                num_CUs = nampla->pop_units;
                fprintf(log_file, " to fill ship, but\n!   only %d can be built.\n", num_CUs);
            }
        }

        colony_planet = planet_base + (long) colony_nampla->planet_index;

        i = 100 + (int) colony_planet->mining_difficulty;
        num_AUs = ((100 * num_CUs) + (i + 1) / 2) / i;
        num_IUs = num_CUs - num_AUs;
    }

    if (num_CUs <= 0) { return; }

    /* Make sure there's enough money to pay for it all. */
    if (load_transport && CUs_only) {
        amount_to_spend = num_CUs;
    } else {
        amount_to_spend = num_CUs + num_IUs + num_AUs;
    }

    if (check_bounced(amount_to_spend)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Internal error. Notify GM!\n");
        return;
    }

    /* Start logging what happened. */
    if (load_transport && CUs_only) {
        start_dev_log(num_CUs, 0, 0);
    } else {
        start_dev_log(num_CUs, num_IUs, num_AUs);
    }

    log_string(" for PL ");
    log_string(colony_nampla->name);

    nampla->pop_units -= num_CUs;

    if (load_transport) {
        if (CUs_only) {
            colony_nampla->IUs_needed += num_IUs;
            colony_nampla->AUs_needed += num_AUs;
        }

        if (nampla->x != ship->x || nampla->y != ship->y || nampla->z != ship->z) {
            nampla->item_quantity[CU] += num_CUs;
            if (!CUs_only) {
                nampla->item_quantity[IU] += num_IUs;
                nampla->item_quantity[AU] += num_AUs;
            }

            log_string(" but will remain on the planet's surface because ");
            log_string(ship_name(ship));
            log_string(" is not in the same sector.");
        } else {
            ship->item_quantity[CU] += num_CUs;
            if (!CUs_only) {
                ship->item_quantity[IU] += num_IUs;
                ship->item_quantity[AU] += num_AUs;
            }

            n = colony_nampla - nampla_base;
            if (n == 0) {
                /* Home planet. */
                n = 9999;
            }
            ship->unloading_point = n;

            n = nampla - nampla_base;
            if (n == 0) {
                /* Home planet. */
                n = 9999;
            }
            ship->loading_point = n;

            log_string(" and transferred to ");
            log_string(ship_name(ship));
        }
    } else {
        colony_nampla->item_quantity[CU] += num_CUs;
        colony_nampla->item_quantity[IU] += num_IUs;
        colony_nampla->item_quantity[AU] += num_AUs;

        colony_nampla->auto_IUs += num_IUs;
        colony_nampla->auto_AUs += num_AUs;

        log_string(" and transferred to PL ");
        log_string(colony_nampla->name);

        check_population(colony_nampla);
    }

    log_string(".\n");
}


void do_DISBAND_command(void) {
    /* Get the planet. */
    int found = get_location();
    if (!found || nampla == NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in DISBAND command.\n");
        return;
    }
    /* Make sure planet is not the home planet. */
    if (nampla->status & HOME_PLANET) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You cannot disband your home planet!\n");
        return;
    }
    /* Make sure planet is not under siege. */
    if (nampla->siege_eff) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You cannot disband a planet that is under siege!\n");
        return;
    }
    /* Mark the colony as "disbanded" and convert mining and manufacturing base to CUs, IUs, and AUs. */
    nampla->status |= DISBANDED_COLONY;
    nampla->item_quantity[CU] += nampla->mi_base + nampla->ma_base;
    nampla->item_quantity[IU] += nampla->mi_base / 2;
    nampla->item_quantity[AU] += nampla->ma_base / 2;
    nampla->mi_base = 0;
    nampla->ma_base = 0;
    /* Log the event. */
    log_string("    The colony on PL ");
    log_string(nampla->name);
    log_string(" was ordered to disband.\n");
}


void do_ENEMY_command(void) {
    int i, array_index, bit_number;
    long bit_mask;
    /* See if declaration is for all species. */
    if (get_value()) {
        bit_mask = 0;
        for (i = 0; i < NUM_CONTACT_WORDS; i++) {
            species->enemy[i] = ~bit_mask;    /* Set all enemy bits. */
            species->ally[i] = bit_mask;    /* Clear all ally bits. */
        }
    } else {
        /* Get name of species that is being declared an enemy. */
        if (!get_species_name()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid or missing argument in ENEMY command.\n");
            return;
        }
        /* Get array index and bit mask. */
        array_index = (g_spec_number - 1) / 32;
        bit_number = (g_spec_number - 1) % 32;
        bit_mask = 1 << bit_number;
        /* Set/clear the appropriate bit. */
        species->enemy[array_index] |= bit_mask;    /* Set enemy bit. */
        species->ally[array_index] &= ~bit_mask;    /* Clear ally bit. */
    }
    /* Log the result. */
    log_string("    Enmity was declared towards ");
    if (bit_mask == 0) {
        log_string("ALL species");
    } else {
        log_string("SP ");
        log_string(g_spec_name);
    }
    log_string(".\n");
}


void do_ESTIMATE_command(void) {
    int i, max_error, estimate[6], contact_word_number, contact_bit_number;
    long cost, contact_mask;
    struct species_data *alien;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get name of alien species. */
    if (!get_species_name()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid species name in ESTIMATE command.\n");
        return;
    }

    /* Check if we've met this species. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't do an estimate of a species you haven't met.\n");
        return;
    }

    /* Check if sufficient funds are available. */
    cost = 25;
    if (check_bounced(cost)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    /* Log the result. */
    if (first_pass) {
        log_string("    An estimate of the technology of SP ");
        log_string(g_spec_name);
        log_string(" was made at a cost of ");
        log_long(cost);
        log_string(".\n");
        return;
    }

    /* Make the estimates. */
    alien = &spec_data[g_spec_number - 1];
    for (i = 0; i < 6; i++) {
        max_error = (int) alien->tech_level[i] - (int) species->tech_level[i];
        if (max_error < 1) { max_error = 1; }
        estimate[i] = (int) alien->tech_level[i] + rnd((2 * max_error) + 1) - (max_error + 1);
        if (alien->tech_level[i] == 0) { estimate[i] = 0; }
        if (estimate[i] < 0) { estimate[i] = 0; }
    }

    log_string("    Estimate of the technology of SP ");
    log_string(alien->name);
    log_string(" (government name '");
    log_string(alien->govt_name);
    log_string("', government type '");
    log_string(alien->govt_type);
    log_string("'):\n      MI = ");
    log_int(estimate[MI]);
    log_string(", MA = ");
    log_int(estimate[MA]);
    log_string(", ML = ");
    log_int(estimate[ML]);
    log_string(", GV = ");
    log_int(estimate[GV]);
    log_string(", LS = ");
    log_int(estimate[LS]);
    log_string(", BI = ");
    log_int(estimate[BI]);
    log_string(".\n");
}


void do_HIDE_command(void) {
    int n, status;
    long cost;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Make sure this is not a mining colony or home planet. */
    if (nampla->status & HOME_PLANET) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not HIDE a home planet.\n");
        return;
    }
    if (nampla->status & RESORT_COLONY) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not HIDE a resort colony.\n");
        return;
    }

    /* Check if planet is under siege. */
    if (nampla->siege_eff != 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Besieged planet cannot HIDE!\n");
        return;
    }

    /* Check if sufficient funds are available. */
    cost = (nampla->mi_base + nampla->ma_base) / 10L;
    if (nampla->status & MINING_COLONY) {
        if (cost > species->econ_units) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Mining colony does not have sufficient EUs to hide.\n");
            return;
        } else {
            species->econ_units -= cost;
        }
    } else if (check_bounced(cost)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    /* Set 'hiding' flag. */
    nampla->hiding = TRUE;

    /* Log transaction. */
    log_string("    Spent ");
    log_long(cost);
    log_string(" hiding this colony.\n");
}


void do_INSTALL_command(void) {
    int i, item_class, item_count, num_available, do_all_units, recovering_home_planet, alien_index;
    long n, current_pop, reb;
    struct nampla_data *alien_home_nampla;
    /* Get number of items to install. */
    if (get_value()) {
        do_all_units = FALSE;
    } else {
        do_all_units = TRUE;
        item_count = 0;
        item_class = IU;
        goto get_planet;
    }
    /* Make sure value is meaningful. */
    if (value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid item count in INSTALL command.\n");
        return;
    }
    item_count = value;
    /* Get class of item. */
    item_class = get_class_abbr();
    if (item_class != ITEM_CLASS || (abbr_index != IU && abbr_index != AU)) {
        /* Players sometimes accidentally use "MI" for "IU"
            or "MA" for "AU". */
        if (item_class == TECH_ID && abbr_index == MI) {
            abbr_index = IU;
        } else if (item_class == TECH_ID && abbr_index == MA) {
            abbr_index = AU;
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid item class!\n");
            return;
        }
    }
    item_class = abbr_index;

    get_planet:

    /* Get planet where items are to be installed. */
    if (!get_location() || nampla == NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in INSTALL command.\n");
        return;
    }
    /* Make sure this is not someone else's populated homeworld. */
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
        if (species_number == alien_index + 1) { continue; }
        if (!data_in_memory[alien_index]) { continue; }
        alien_home_nampla = namp_data[alien_index];
        if (alien_home_nampla->x != nampla->x) { continue; }
        if (alien_home_nampla->y != nampla->y) { continue; }
        if (alien_home_nampla->z != nampla->z) { continue; }
        if (alien_home_nampla->pn != nampla->pn) { continue; }
        if ((alien_home_nampla->status & POPULATED) == 0) { continue; }
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not colonize someone else's populated home planet!\n");
        return;
    }
    /* Make sure it's not a healthy home planet. */
    recovering_home_planet = FALSE;
    if (nampla->status & HOME_PLANET) {
        n = nampla->mi_base + nampla->ma_base + nampla->IUs_to_install + nampla->AUs_to_install;
        reb = species->hp_original_base - n;
        if (reb > 0) {
            recovering_home_planet = TRUE;    /* HP was bombed. */
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Installation not allowed on a healthy home planet!\n");
            return;
        }
    }

    check_items:

    /* Make sure planet has the specified items. */
    if (item_count == 0) {
        item_count = nampla->item_quantity[item_class];
        if (nampla->item_quantity[CU] < item_count) {
            item_count = nampla->item_quantity[CU];
        }
        if (item_count == 0) {
            if (do_all_units) {
                item_count = 0;
                item_class = AU;
                do_all_units = FALSE;
                goto check_items;
            } else {
                return;
            }
        }
    } else if (nampla->item_quantity[item_class] < item_count) {
        fprintf(log_file, "! WARNING: %s", input_line);
        fprintf(log_file, "! Planet does not have %d %ss. Substituting 0 for %d!\n", item_count, item_abbr[item_class],
                item_count);
        item_count = 0;
        goto check_items;
    }
    if (recovering_home_planet) {
        if (item_count > reb) {
            item_count = reb;
        }
        reb -= item_count;
    }
    /* Make sure planet has enough colonist units. */
    num_available = nampla->item_quantity[CU];
    if (num_available < item_count) {
        if (num_available > 0) {
            fprintf(log_file, "! WARNING: %s", input_line);
            fprintf(log_file, "! Planet does not have %d CUs. Substituting %d for %d!\n",
                    item_count, num_available, item_count);
            item_count = num_available;
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! No colonist units on planet for installation.\n");
            return;
        }
    }
    /* Start the installation. */
    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[item_class] -= item_count;
    if (item_class == IU) {
        nampla->IUs_to_install += item_count;
    } else {
        nampla->AUs_to_install += item_count;
    }
    /* Log result. */
    log_string("    Installation of ");
    log_int(item_count);
    log_char(' ');
    log_string(item_name[item_class]);
    if (item_count != 1) { log_char('s'); }
    log_string(" began on PL ");
    log_string(nampla->name);
    log_string(".\n");
    if (do_all_units) {
        item_count = 0;
        item_class = AU;
        do_all_units = FALSE;
        goto check_items;
    }
    check_population(nampla);
}


void do_INTERCEPT_command(void) {
    int i, n, status;
    long cost;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get amount to spend. */
    status = get_value();
    if (status == 0 || value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid or missing amount.\n");
        return;
    }
    if (value == 0) { value = balance; }
    if (value == 0) { return; }
    cost = value;

    /* Check if planet is under siege. */
    if (nampla->siege_eff != 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Besieged planet cannot INTERCEPT!\n");
        return;
    }

    /* Check if sufficient funds are available. */
    if (check_bounced(cost)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    log_string("    Preparations were made for an interception at a cost of ");
    log_long(cost);
    log_string(".\n");

    if (first_pass) { return; }

    /* Allocate funds. */
    for (i = 0; i < num_intercepts; i++) {
        if (nampla->x != intercept[i].x) { continue; }
        if (nampla->y != intercept[i].y) { continue; }
        if (nampla->z != intercept[i].z) { continue; }

        /* This interception was started by another planet in the same star system. */
        intercept[i].amount_spent += cost;
        return;
    }

    if (num_intercepts == MAX_INTERCEPTS) {
        fprintf(stderr, "\n\tMAX_INTERCEPTS exceeded in do_JUMP_command!\n\n");
        exit(-1);
    }

    intercept[num_intercepts].x = nampla->x;
    intercept[num_intercepts].y = nampla->y;
    intercept[num_intercepts].z = nampla->z;
    intercept[num_intercepts].amount_spent = cost;

    ++num_intercepts;
}


void do_JUMP_command(int jumped_in_combat, int using_jump_portal) {
    int i, n, found, max_xyz, temp_x, temp_y, temp_z, difference;
    int status, mishap_gv;

    long mishap_chance, success_chance;

    char temp_string[32], *original_line_pointer;

    short mishap_age;

    struct ship_data *jump_portal;


    /* Set default status at end of jump. */
    status = IN_DEEP_SPACE;

    /* Check if this ship jumped in combat. */
    if (jumped_in_combat) {
        x = ship->dest_x;
        y = ship->dest_y;
        z = ship->dest_z;
        pn = 0;
        using_jump_portal = FALSE;
        nampla = NULL;
        goto do_jump;
    }

    /* Get ship making the jump. */
    original_line_pointer = input_line_pointer;
    found = get_ship();
    if (!found) {
        input_line_pointer = original_line_pointer;
        fix_separator();    /* Check for missing comma or tab. */
        found = get_ship();    /* Try again. */
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in JUMP or PJUMP command.\n");
            return;
        }
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship(ship)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! This ship is salvage of a disbanded colony!\n");
        return;
    }

    /* Check if this ship withdrew or was was forced to jump from combat.
	If so, ignore specified coordinates and use those provided by the
	combat program. */
    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        x = ship->dest_x;
        y = ship->dest_y;
        z = ship->dest_z;
        pn = 0;
        jumped_in_combat = TRUE;
        using_jump_portal = FALSE;
        nampla = NULL;
        goto do_jump;
    }

    /* Make sure ship can jump. */
    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s is still under construction!\n",
                ship_name(ship));
        return;
    }

    if (ship->type == STARBASE
        || (!using_jump_portal && ship->type == SUB_LIGHT)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s cannot make an interstellar jump!\n",
                ship_name(ship));
        return;
    }

    /* Check if JUMP, MOVE, or WORMHOLE was already done for this ship. */
    if (ship->just_jumped) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s already jumped or moved this turn!\n",
                ship_name(ship));
        return;
    }

    /* Get the destination. */
    original_line_pointer = input_line_pointer;
    found = get_location();
    if (!found) {
        if (using_jump_portal) {
            input_line_pointer = original_line_pointer;
            fix_separator();    /* Check for missing comma or tab. */
            found = get_location();    /* Try again. */
        }

        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid destination in JUMP or PJUMP command.\n");
            return;
        }
    }

    /* Set status to IN_ORBIT if destination is a planet. */
    if (pn > 0) { status = IN_ORBIT; }

    /* Check if a jump portal is being used. */
    if (using_jump_portal) {
        found = get_jump_portal();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid starbase name in PJUMP command.\n");
            return;
        }
    }

    /* If using a jump portal, make sure that starbase has sufficient number
	of jump portal units. */
    if (using_jump_portal) {
        if (jump_portal_units < ship->tonnage) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Starbase does not have enough Jump Portal Units!\n");
            return;
        }
    }

    do_jump:

    if (x == ship->x && y == ship->y && z == ship->z) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s was already at specified x,y,z.\n",
                ship_name(ship));
        return;
    }

    /* Set flags to show that ship jumped this turn. */
    ship->just_jumped = TRUE;

    /* Calculate basic mishap probability. */
    if (using_jump_portal) {
        mishap_age = jump_portal_age;
        mishap_gv = jump_portal_gv;
    } else {
        mishap_age = ship->age;
        mishap_gv = species->tech_level[GV];
    }
    mishap_chance = (100L * (long) (((x - ship->x) * (x - ship->x))
                                    + ((y - ship->y) * (y - ship->y))
                                    + ((z - ship->z) * (z - ship->z))))
                    / (long) mishap_gv;

    if (mishap_chance > 10000L) {
        mishap_chance = 10000L;
        goto start_jump;
    }

    /* Add aging effect. */
    if (mishap_age > 0) {
        success_chance = 10000L - mishap_chance;
        success_chance -= (2L * (long) mishap_age * success_chance) / 100L;
        if (success_chance < 0) { success_chance = 0; }
        mishap_chance = 10000L - success_chance;
    }


    start_jump:

    log_string("    ");
    log_string(ship_name(ship));
    log_string(" will try to jump to ");

    if (nampla == NULL) {
        log_int(x);
        log_char(' ');
        log_int(y);
        log_char(' ');
        log_int(z);
    } else {
        log_string("PL ");
        log_string(nampla->name);
    }

    if (using_jump_portal) {
        log_string(" via jump portal ");
        log_string(jump_portal_name);

        if (using_alien_portal && !first_pass) {
            /* Define this transaction. */
            if (num_transactions == MAX_TRANSACTIONS) {
                fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                exit(-1);
            }

            n = num_transactions++;
            transaction[n].type = ALIEN_JUMP_PORTAL_USAGE;
            transaction[n].number1 = other_species_number;
            strcpy(transaction[n].name1, species->name);
            strcpy(transaction[n].name2, ship_name(ship));
            strcpy(transaction[n].name3, ship_name(alien_portal));
        }
    }

    sprintf(temp_string, " (%ld.%02ld%%).\n", mishap_chance / 100L, mishap_chance % 100L);
    log_string(temp_string);

    jump_again:

    if (first_pass || (rnd(10000) > mishap_chance)) {
        ship->x = x;
        ship->y = y;
        ship->z = z;
        ship->pn = pn;
        ship->status = status;

        if (!first_pass) { star_visited(x, y, z); }

        return;
    }

    /* Ship had a mishap. Check if it has any fail-safe jump units. */
    if (ship->item_quantity[FS] > 0) {
        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_JUMP_command!\n\n");
            exit(-1);
        }

        n = num_transactions++;
        transaction[n].type = SHIP_MISHAP;
        transaction[n].value = 4;    /* Use of one FS. */
        transaction[n].number1 = species_number;
        strcpy(transaction[n].name1, ship_name(ship));

        ship->item_quantity[FS] -= 1;
        goto jump_again;
    }

    /* If ship was forced to jump, and it reached this point, then it
	self-destructed. */
    if (ship->status == FORCED_JUMP) { goto self_destruct; }

    /* Check if ship self-destructed or just mis-jumped. */
    if (rnd(10000) > mishap_chance) {
        /* Calculate mis-jump location. */
        max_xyz = 2 * galaxy.radius - 1;

        try_again:
        temp_x = -1;
        difference = (ship->x > x) ? (ship->x - x) : (x - ship->x);
        difference = (2L * mishap_chance * difference) / 10000L;
        if (difference < 3) { difference = 3; }
        while (temp_x < 0 || temp_x > max_xyz) {
            temp_x = x - rnd(difference) + rnd(difference);
        }

        temp_y = -1;
        difference = (ship->y > y) ? (ship->y - y) : (y - ship->y);
        difference = (2L * mishap_chance * difference) / 10000L;
        if (difference < 3) { difference = 3; }
        while (temp_y < 0 || temp_y > max_xyz) {
            temp_y = y - rnd(difference) + rnd(difference);
        }

        temp_z = -1;
        difference = (ship->z > z) ? (ship->z - z) : (z - ship->z);
        difference = (2L * mishap_chance * difference) / 10000L;
        if (difference < 3) { difference = 3; }
        while (temp_z < 0 || temp_z > max_xyz) {
            temp_z = z - rnd(difference) + rnd(difference);
        }

        if (x == temp_x && y == temp_y && z == temp_z) {
            goto try_again;
        }

        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_JUMP_command!\n\n");
            exit(-1);
        }

        n = num_transactions++;
        transaction[n].type = SHIP_MISHAP;
        transaction[n].value = 3;    /* Mis-jump. */
        transaction[n].number1 = species_number;
        strcpy(transaction[n].name1, ship_name(ship));
        transaction[n].x = temp_x;
        transaction[n].y = temp_y;
        transaction[n].z = temp_z;

        ship->x = temp_x;
        ship->y = temp_y;
        ship->z = temp_z;
        ship->pn = 0;

        ship->status = IN_DEEP_SPACE;

        star_visited(temp_x, temp_y, temp_z);

        return;
    }

    self_destruct:

    /* Ship self-destructed. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_JUMP_command!\n\n");
        exit(-1);
    }

    n = num_transactions++;
    transaction[n].type = SHIP_MISHAP;
    transaction[n].value = 2;    /* Self-destruction. */
    transaction[n].number1 = species_number;
    strcpy(transaction[n].name1, ship_name(ship));

    delete_ship(ship);
}


void do_LAND_command(void) {
    int i, n, found, siege_effectiveness, landing_detected, landed;
    int alien_number, alien_index, alien_pn, array_index, bit_number;
    int requested_alien_landing, alien_here, already_logged;
    long bit_mask;
    char *original_line_pointer;
    struct species_data *alien;
    struct nampla_data *alien_nampla;

    /* Get the ship. */
    original_line_pointer = input_line_pointer;
    found = get_ship();
    if (!found) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        found = get_ship();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in LAND command.\n");
            return;
        }
    }

    /* Make sure the ship is not a starbase. */
    if (ship->type == STARBASE) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! A starbase cannot land on a planet!\n");
        return;
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }

    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }

    /* Get the planet number, if specified. */
    found = get_value();

    get_planet:

    alien_pn = 0;
    alien_here = FALSE;
    requested_alien_landing = FALSE;
    landed = FALSE;
    if (!found) {
        found = get_location();
        if (!found || nampla == NULL) { found = FALSE; }
    } else {
        /* Check if we or another species that has declared us ALLY has a colony on this planet. */
        found = FALSE;
        alien_pn = value;
        requested_alien_landing = TRUE;
        array_index = (species_number - 1) / 32;
        bit_number = (species_number - 1) % 32;
        bit_mask = 1 << bit_number;
        for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
            if (!data_in_memory[alien_index]) { continue; }
            alien = &spec_data[alien_index];
            alien_nampla = namp_data[alien_index] - 1;
            for (i = 0; i < alien->num_namplas; i++) {
                ++alien_nampla;
                if (ship->x != alien_nampla->x) { continue; }
                if (ship->y != alien_nampla->y) { continue; }
                if (ship->z != alien_nampla->z) { continue; }
                if (alien_pn != alien_nampla->pn) { continue; }
                if ((alien_nampla->status & POPULATED) == 0) { continue; }
                if (alien_index == species_number - 1) {
                    /* We have a colony here. No permission needed. */
                    nampla = alien_nampla;
                    found = TRUE;
                    alien_here = FALSE;
                    requested_alien_landing = FALSE;
                    goto finish_up;
                }
                alien_here = TRUE;
                if ((alien->ally[array_index] & bit_mask) == 0) { continue; }
                found = TRUE;
                break;
            }
            if (found) { break; }
        }
    }

    finish_up:

    already_logged = FALSE;
    if (requested_alien_landing && alien_here) {
        /* Notify the other alien(s). */
        landed = found;
        for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
            if (!data_in_memory[alien_index]) { continue; }
            if (alien_index == species_number - 1) { continue; }
            alien = &spec_data[alien_index];
            alien_nampla = namp_data[alien_index] - 1;
            for (i = 0; i < alien->num_namplas; i++) {
                ++alien_nampla;
                if (ship->x != alien_nampla->x) { continue; }
                if (ship->y != alien_nampla->y) { continue; }
                if (ship->z != alien_nampla->z) { continue; }
                if (alien_pn != alien_nampla->pn) { continue; }
                if ((alien_nampla->status & POPULATED) == 0) { continue; }
                if ((alien->ally[array_index] & bit_mask) != 0) {
                    found = TRUE;
                } else {
                    found = FALSE;
                }
                if (landed && !found) { continue; }
                if (landed) {
                    log_string("    ");
                } else {
                    log_string("!!! ");
                }
                log_string(ship_name(ship));
                if (landed) {
                    log_string(" was granted");
                } else {
                    log_string(" was denied");
                }
                log_string(" permission to land on PL ");
                log_string(alien_nampla->name);
                log_string(" by SP ");
                log_string(alien->name);
                log_string(".\n");
                already_logged = TRUE;
                nampla = alien_nampla;
                if (first_pass) { break; }
                /* Define a 'landing request' transaction. */
                if (num_transactions == MAX_TRANSACTIONS) {
                    fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                    exit(-1);
                }
                n = num_transactions++;
                transaction[n].type = LANDING_REQUEST;
                transaction[n].value = landed;
                transaction[n].number1 = alien_index + 1;
                strcpy(transaction[n].name1, alien_nampla->name);
                strcpy(transaction[n].name2, ship_name(ship));
                strcpy(transaction[n].name3, species->name);
                break;
            }
        }
        found = TRUE;
    }
    if (alien_here && !landed) { return; }
    if (!found) {
        if ((ship->status == IN_ORBIT || ship->status == ON_SURFACE) && !requested_alien_landing) {
            /* Player forgot to specify planet. Use the one it's already at. */
            value = ship->pn;
            found = TRUE;
            goto get_planet;
        }
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid or missing planet in LAND command.\n");
        return;
    }

    /* Make sure the ship and the planet are in the same star system. */
    if (ship->x != nampla->x || ship->y != nampla->y || ship->z != nampla->z) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship and planet are not in the same sector.\n");
        return;
    }

    /* Make sure planet is populated. */
    if ((nampla->status & POPULATED) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Planet in LAND command is not populated.\n");
        return;
    }

    /* Move the ship. */
    ship->pn = nampla->pn;
    ship->status = ON_SURFACE;

    if (already_logged) { return; }

    /* If the planet is under siege, the landing may be detected by the besiegers. */
    log_string("    ");
    log_string(ship_name(ship));

    if (nampla->siege_eff != 0) {
        if (first_pass) {
            log_string(" will attempt to land on PL ");
            log_string(nampla->name);
            log_string(" in spite of the siege");
        } else {
            if (nampla->siege_eff < 0) {
                siege_effectiveness = -nampla->siege_eff;
            } else {
                siege_effectiveness = nampla->siege_eff;
            }
            landing_detected = FALSE;
            if (rnd(100) <= siege_effectiveness) {
                landing_detected = TRUE;
                for (i = 0; i < num_transactions; i++) {
                    /* Find out who is besieging this planet. */
                    if (transaction[i].type != BESIEGE_PLANET) { continue; }
                    if (transaction[i].x != nampla->x) { continue; }
                    if (transaction[i].y != nampla->y) { continue; }
                    if (transaction[i].z != nampla->z) { continue; }
                    if (transaction[i].pn != nampla->pn) { continue; }
                    if (transaction[i].number2 != species_number) { continue; }
                    alien_number = transaction[i].number1;
                    /* Define a 'detection' transaction. */
                    if (num_transactions == MAX_TRANSACTIONS) {
                        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                        exit(-1);
                    }
                    n = num_transactions++;
                    transaction[n].type = DETECTION_DURING_SIEGE;
                    transaction[n].value = 1;    /* Landing. */
                    strcpy(transaction[n].name1, nampla->name);
                    strcpy(transaction[n].name2, ship_name(ship));
                    strcpy(transaction[n].name3, species->name);
                    transaction[n].number3 = alien_number;
                }
            }
            if (rnd(100) <= siege_effectiveness) {
                /* Ship doesn't know if it was detected. */
                log_string(" may have been detected by the besiegers when it landed on PL ");
                log_string(nampla->name);
            } else {
                /* Ship knows whether or not it was detected. */
                if (landing_detected) {
                    log_string(" was detected by the besiegers when it landed on PL ");
                    log_string(nampla->name);
                } else {
                    log_string(" landed on PL ");
                    log_string(nampla->name);
                    log_string(" without being detected by the besiegers");
                }
            }
        }
    } else {
        if (first_pass) {
            log_string(" will land on PL ");
        } else {
            log_string(" landed on PL ");
        }
        log_string(nampla->name);
    }
    log_string(".\n");
}


void do_MESSAGE_command(void) {
    int i, message_number, message_fd, bad_species;
    int unterminated_message;
    char c1, c2, c3, filename[32];
    FILE *message_file;

    /* Get destination of message. */
    if (!get_species_name()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid species name in MESSAGE command.\n");
        bad_species = TRUE;
    } else {
        bad_species = FALSE;
    }

    /* Generate a random number, create a filename with it, and use it to store message. */
    if (!first_pass && !bad_species) {
        while (1) {
            struct stat sb;
            /* Generate a random filename. */
            message_number = 100000 + rnd(32000);
            sprintf(filename, "m%d.msg", message_number);
            /* Make sure that this filename is not already in use. */
            if (stat(filename, &sb) == 0) {
                /* File already exists. Try again. */
                continue;
            }
            break;
        }
        message_file = fopen(filename, "w");
        if (message_file == NULL) {
            perror("do_MESSAGE_command");
            fprintf(stderr, "\n\n!!! Cannot open message file '%s' for writing !!!\n\n", filename);
            exit(-1);
        }
    }

    /* Copy message to file. */
    unterminated_message = FALSE;
    while (1) {
        /* Read next line. */
        input_line_pointer = readln(input_line, 256, input_file);
        if (input_line_pointer == NULL) {
            unterminated_message = TRUE;
            end_of_file = TRUE;
            break;
        }
        skip_whitespace();
        c1 = *input_line_pointer++;
        c2 = *input_line_pointer++;
        c3 = *input_line_pointer;
        c1 = toupper(c1);
        c2 = toupper(c2);
        c3 = toupper(c3);
        if (c1 == 'Z' && c2 == 'Z' && c3 == 'Z') { break; }
        if (!first_pass && !bad_species) { fputs(input_line, message_file); }
    }

    if (bad_species) { return; }

    /* Log the result. */
    log_string("    A message was sent to SP ");
    log_string(g_spec_name);
    log_string(".\n");

    if (unterminated_message) {
        log_string("  ! WARNING: Message was not properly terminated with ZZZ!");
        log_string(" Any orders that follow the message will be assumed");
        log_string(" to be part of the message and will be ignored!\n");
    }

    if (first_pass) { return; }

    fclose(message_file);

    /* Define this message transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    i = num_transactions++;
    transaction[i].type = MESSAGE_TO_SPECIES;
    transaction[i].value = message_number;
    transaction[i].number1 = species_number;
    strcpy(transaction[i].name1, species->name);
    transaction[i].number2 = g_spec_number;
    strcpy(transaction[i].name2, g_spec_name);
}


void do_MOVE_command(void) {
    int i, n;
    char *original_line_pointer = input_line_pointer;
    int found = get_ship();
    if (!found) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        found = get_ship();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in MOVE command.\n");
            return;
        }
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }

    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }

    /* Check if JUMP or MOVE was already done for this ship. */
    if (ship->just_jumped) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s already jumped or moved this turn!\n",
                ship_name(ship));
        return;
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship(ship)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! This ship is salvage of a disbanded colony!\n");
        return;
    }

    /* Get the planet. */
    found = get_location();
    if (!found || nampla != NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! You may not use a planet name in MOVE command.\n");
        return;
    }

    /* Check if deltas are acceptable. */
    i = x - ship->x;
    if (i < 0) { n = -i; } else { n = i; }
    i = y - ship->y;
    if (i < 0) { n += -i; } else { n += i; }
    i = z - ship->z;
    if (i < 0) { n += -i; } else { n += i; }
    if (n > 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Destination is too far in MOVE command.\n");
        return;
    }

    /* Move the ship. */
    ship->x = x;
    ship->y = y;
    ship->z = z;
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;
    ship->just_jumped = 50;

    if (!first_pass) { star_visited(x, y, z); }

    /* Log result. */
    log_string("    ");
    log_string(ship_name(ship));
    if (first_pass) {
        log_string(" will move to sector ");
    } else {
        log_string(" moved to sector ");
    }
    log_int(x);
    log_char(' ');
    log_int(y);
    log_char(' ');
    log_int(z);
    log_string(".\n");
}


void do_NAME_command(void) {
    int i, found, name_length, unused_nampla_available;
    char upper_nampla_name[32], *original_line_pointer;
    struct planet_data *planet;
    struct nampla_data *unused_nampla;

    /* Get x y z coordinates. */
    found = get_location();
    if (!found || nampla != NULL || pn == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid coordinates in NAME command.\n");
        return;
    }

    /* Get planet abbreviation. */
    skip_whitespace();
    original_line_pointer = input_line_pointer;
    if (get_class_abbr() != PLANET_ID) {
        /* Check if PL was mispelled (i.e, "PT" or "PN"). Otherwise
            assume that it was accidentally omitted. */
        if (tolower(*original_line_pointer) != 'p'
            || isalnum(*(original_line_pointer + 2))) {
            input_line_pointer = original_line_pointer;
        }
    }

    /* Get planet name. */
    name_length = get_name();
    if (name_length < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in NAME command.\n");
        return;
    }

    /* Search existing namplas for name and location. */
    found = FALSE;
    unused_nampla_available = FALSE;
    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
        ++nampla;

        if (nampla->pn == 99) {
            /* We can re-use this nampla rather than append a new one. */
            unused_nampla = nampla;
            unused_nampla_available = TRUE;
            continue;
        }

        /* Check if a named planet already exists at this location. */
        if (nampla->x == x && nampla->y == y && nampla->z == z && nampla->pn == pn) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! The planet at these coordinates already has a name.\n");
            return;
        }

        /* Make upper case copy of nampla name. */
        for (i = 0; i < 32; i++) {
            upper_nampla_name[i] = toupper(nampla->name[i]);
        }

        /* Compare names. */
        if (strcmp(upper_nampla_name, upper_name) == 0) {
            found = TRUE;
            break;
        }
    }

    if (found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Planet in NAME command already exists.\n");
        return;
    }

    /* Add new nampla to database for this species. */
    if (unused_nampla_available) {
        nampla = unused_nampla;
    } else {
        num_new_namplas[species_index]++;
        if (num_new_namplas[species_index] > NUM_EXTRA_NAMPLAS) {
            fprintf(stderr, "\n\n\tInsufficient memory for new planet name:\n");
            fprintf(stderr, "\n\t%s\n", input_line);
            exit(-1);
        }
        nampla = nampla_base + species->num_namplas;
        species->num_namplas += 1;
        /* Set everything to zero. */
        delete_nampla(nampla);
    }

    /* Initialize new nampla. */
    strcpy(nampla->name, original_name);
    nampla->x = x;
    nampla->y = y;
    nampla->z = z;
    nampla->pn = pn;
    nampla->status = COLONY;
    nampla->planet_index = star->planet_index + pn - 1;
    planet = planet_base + (long) nampla->planet_index;
    nampla->message = planet->message;
    /* Everything else was set to zero in above call to 'delete_nampla'. */

    /* Mark sector as having been visited. */
    star_visited(x, y, z);

    /* Log result. */
    log_string("    Named PL ");
    log_string(nampla->name);
    log_string(" at ");
    log_int(nampla->x);
    log_char(' ');
    log_int(nampla->y);
    log_char(' ');
    log_int(nampla->z);
    log_string(", planet #");
    log_int(nampla->pn);
    log_string(".\n");
}


void do_NEUTRAL_command(void) {
    int i, array_index, bit_number;
    long bit_mask;

    /* See if declaration is for all species. */
    if (get_value()) {
        bit_mask = 0;
        for (i = 0; i < NUM_CONTACT_WORDS; i++) {
            species->enemy[i] = bit_mask;    /* Clear all enemy bits. */
            species->ally[i] = bit_mask;    /* Clear all ally bits. */
        }
    } else {
        /* Get name of species. */
        if (!get_species_name()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid or missing argument in NEUTRAL command.\n");
            return;
        }

        /* Get array index and bit mask. */
        array_index = (g_spec_number - 1) / 32;
        bit_number = (g_spec_number - 1) % 32;
        bit_mask = 1 << bit_number;

        /* Clear the appropriate bit. */
        species->enemy[array_index] &= ~bit_mask;   /* Clear enemy bit. */
        species->ally[array_index] &= ~bit_mask;  /* Clear ally bit. */
    }

    /* Log the result. */
    log_string("    Neutrality was declared towards ");
    if (bit_mask == 0) {
        log_string("ALL species");
    } else {
        log_string("SP ");
        log_string(g_spec_name);
    }
    log_string(".\n");
}


void do_ORBIT_command(void) {
    int i, found, specified_planet_number;
    char *original_line_pointer;

    /* Get the ship. */
    original_line_pointer = input_line_pointer;
    found = get_ship();
    if (!found) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        found = get_ship();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in ORBIT command.\n");
            return;
        }
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }

    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }

    /* Make sure this ship didn't just arrive via a MOVE command. */
    if (ship->just_jumped == 50) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! ORBIT not allowed immediately after a MOVE!\n");
        return;
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship(ship)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! This ship is salvage of a disbanded colony!\n");
        return;
    }

    /* Get the planet. */
    specified_planet_number = get_value();

    get_planet:

    if (specified_planet_number) {
        found = FALSE;
        specified_planet_number = value;
        for (i = 0; i < num_stars; i++) {
            star = star_base + i;
            if (star->x != ship->x) { continue; }
            if (star->y != ship->y) { continue; }
            if (star->z != ship->z) { continue; }
            if (specified_planet_number >= 1 && specified_planet_number <= star->num_planets) {
                found = TRUE;
            }
            break;
        }

        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid planet in ORBIT command.\n");
            return;
        }

        ship->pn = specified_planet_number;

        goto finish_up;
    }

    found = get_location();
    if (!found || nampla == NULL) {
        if (ship->status == IN_ORBIT || ship->status == ON_SURFACE) {
            /* Player forgot to specify planet. Use the one it's already at. */
            specified_planet_number = ship->pn;
            value = specified_planet_number;
            goto get_planet;
        }

        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid or missing planet in ORBIT command.\n");
        return;
    }

    /* Make sure the ship and the planet are in the same star system. */
    if (ship->x != nampla->x || ship->y != nampla->y || ship->z != nampla->z) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship and planet are not in the same sector.\n");
        return;
    }

    /* Move the ship. */
    ship->pn = nampla->pn;

    finish_up:

    ship->status = IN_ORBIT;

    /* If a planet number is being used, see if it has a name.  If so, use the name. */
    if (specified_planet_number) {
        for (i = 0; i < species->num_namplas; i++) {
            nampla = nampla_base + i;
            if (nampla->x != ship->x) { continue; }
            if (nampla->y != ship->y) { continue; }
            if (nampla->z != ship->z) { continue; }
            if (nampla->pn != specified_planet_number) { continue; }
            specified_planet_number = 0;
            break;
        }
    }

    /* Log result. */
    log_string("    ");
    log_string(ship_name(ship));
    if (first_pass) {
        log_string(" will enter orbit around ");
    } else {
        log_string(" entered orbit around ");
    }
    if (specified_planet_number) {
        log_string("planet number ");
        log_int(specified_planet_number);
    } else {
        log_string("PL ");
        log_string(nampla->name);
    }
    log_string(".\n");
}


void do_PRODUCTION_command(int missing_production_order) {
    int i, j, abbr_type, name_length, found, alien_number, under_siege,
            siege_percent_effectiveness, new_alien, num_siege_ships,
            mining_colony, resort_colony, special_colony, ship_index,
            enemy_on_same_planet, trans_index, production_penalty,
            ls_needed, shipyards_for_this_species;

    char upper_nampla_name[32];

    long n, RMs_produced, num_bytes, total_siege_effectiveness,
            siege_effectiveness[MAX_SPECIES + 1], EUs_available_for_siege,
            EUs_for_distribution, EUs_for_this_species, total_EUs_stolen,
            special_production, pop_units_here[MAX_SPECIES + 1],
            alien_pop_units, total_alien_pop_here, total_besieged_pop,
            ib_for_this_species, ab_for_this_species, total_ib, total_ab,
            total_effective_tonnage;

    struct species_data *alien;
    struct nampla_data *alien_nampla_base, *alien_nampla;
    struct ship_data *alien_ship_base, *alien_ship, *ship;


    if (doing_production) {
        /* Terminate production for previous planet. */
        if (last_planet_produced) {
            transfer_balance();
            last_planet_produced = FALSE;
        }

        /* Give gamemaster option to abort. */
        if (first_pass) { gamemaster_abort_option(); }
        log_char('\n');
    }

    doing_production = TRUE;

    if (missing_production_order) {
        nampla = next_nampla;
        nampla_index = next_nampla_index;

        goto got_nampla;
    }

    /* Get PL abbreviation. */
    abbr_type = get_class_abbr();

    if (abbr_type != PLANET_ID) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in PRODUCTION command.\n");
        return;
    }

    /* Get planet name. */
    name_length = get_name();

    /* Search all namplas for name. */
    found = FALSE;
    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++) {
        ++nampla;

        if (nampla->pn == 99) { continue; }

        /* Make upper case copy of nampla name. */
        for (i = 0; i < 32; i++) {
            upper_nampla_name[i] = toupper(nampla->name[i]);
        }

        /* Compare names. */
        if (strcmp(upper_nampla_name, upper_name) == 0) {
            found = TRUE;
            break;
        }
    }

    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in PRODUCTION command.\n");
        return;
    }

    /* Check if production was already done for this planet. */
    if (production_done[nampla_index]) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! More than one PRODUCTION command for planet.\n");
        return;
    }
    production_done[nampla_index] = TRUE;

    /* Check if this colony was disbanded. */
    if (nampla->status & DISBANDED_COLONY) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Production orders cannot be given for a disbanded colony!\n");
        return;
    }

    got_nampla:

    last_planet_produced = TRUE;
    shipyard_built = FALSE;
    shipyard_capacity = nampla->shipyards;

    /* See if this is a mining or resort colony. */
    mining_colony = FALSE;
    resort_colony = FALSE;
    special_colony = FALSE;
    if (nampla->status & MINING_COLONY) {
        mining_colony = TRUE;
        special_colony = TRUE;
    } else if (nampla->status & RESORT_COLONY) {
        resort_colony = TRUE;
        special_colony = TRUE;
    }

    /* Get planet data for this nampla. */
    planet = planet_base + (long) nampla->planet_index;

    /* Check if fleet maintenance cost is so high that riots ensued. */
    i = 0;
    j = (species->fleet_percent_cost - 10000) / 100;
    if (rnd(100) <= j) {
        log_string("!!! WARNING! Riots on PL ");
        log_string(nampla->name);
        log_string(" due to excessive and unpopular military build-up reduced ");

        if (mining_colony || !special_colony) {
            log_string("mining base by ");
            i = rnd(j);
            log_int(i);
            log_string(" percent ");
            nampla->mi_base -= (i * nampla->mi_base) / 100;
        }

        if (resort_colony || !special_colony) {
            if (i) { log_string("and "); }
            log_string("manufacturing base by ");
            i = rnd(j);
            log_int(i);
            log_string(" percent");
            nampla->ma_base -= (i * nampla->ma_base) / 100;
        }
        log_string("!\n\n");
    }

    /* Calculate "balance" available for spending and create pseudo "checking account". */
    ls_needed = life_support_needed(species, home_planet, planet);

    if (ls_needed == 0) {
        production_penalty = 0;
    } else {
        production_penalty = (100 * ls_needed) / species->tech_level[LS];
    }

    RMs_produced = (10L * (long) species->tech_level[MI] * (long) nampla->mi_base) / (long) planet->mining_difficulty;
    RMs_produced -= (production_penalty * RMs_produced) / 100;
    RMs_produced = (((long) planet->econ_efficiency * RMs_produced) + 50) / 100;

    if (special_colony) {
        /* RMs just 'sitting' on the planet cannot be converted to EUs on a mining colony, and cannot create a 'balance' on a resort colony. */
        raw_material_units = 0;
    } else {
        raw_material_units = RMs_produced + nampla->item_quantity[RM];
    }

    production_capacity = ((long) species->tech_level[MA] * (long) nampla->ma_base) / 10L;
    production_capacity -= (production_penalty * production_capacity) / 100;
    production_capacity = (((long) planet->econ_efficiency * production_capacity) + 50) / 100;

    balance = (raw_material_units > production_capacity) ? production_capacity : raw_material_units;

    if (species->fleet_percent_cost > 10000) {
        n = 10000;
    } else {
        n = species->fleet_percent_cost;
    }

    if (special_colony) {
        EU_spending_limit = 0;
    } else {
        /* Only excess RMs may be recycled. */
        nampla->item_quantity[RM] = raw_material_units - balance;

        balance -= ((n * balance) + 5000) / 10000;
        raw_material_units = balance;
        production_capacity = balance;
        EUs_available_for_siege = balance;
        if (nampla->status & HOME_PLANET) {
            if (species->hp_original_base != 0) {
                /* HP was bombed. */
                EU_spending_limit = 4 * balance;  /* Factor = 4 + 1 = 5. */
            } else {
                EU_spending_limit = species->econ_units;
            }
        } else {
            EU_spending_limit = balance;
        }
    }

    /* Log what was done. Balances for mining and resort colonies will always
	be zero and should not be printed. */
    log_string("  Start of production on PL ");
    log_string(nampla->name);
    log_char('.');
    if (!special_colony) {
        log_string(" (Initial balance is ");
        log_long(balance);
        log_string(".)");
    }
    log_char('\n');

    /* If this IS a mining or resort colony, convert RMs or production capacity to EUs. */
    if (mining_colony) {
        special_production = (2 * RMs_produced) / 3;
        special_production -= ((n * special_production) + 5000) / 10000;
        log_string("    Mining colony ");
    } else if (resort_colony) {
        special_production = (2 * production_capacity) / 3;
        special_production -= ((n * special_production) + 5000) / 10000;
        log_string("    Resort colony ");
    }

    if (special_colony) {
        log_string(nampla->name);
        log_string(" generated ");
        log_long(special_production);
        log_string(" economic units.\n");

        EUs_available_for_siege = special_production;
        species->econ_units += special_production;

        if (mining_colony && !first_pass) {
            planet->mining_difficulty += RMs_produced / 150;
            planet_data_modified = TRUE;
        }
    }

    /* Check if this planet is under siege. */
    nampla->siege_eff = 0;
    under_siege = FALSE;
    alien_number = 0;
    num_siege_ships = 0;
    total_siege_effectiveness = 0;
    enemy_on_same_planet = FALSE;
    total_alien_pop_here = 0;
    for (i = 1; i <= MAX_SPECIES; i++) {
        siege_effectiveness[i] = 0;
        pop_units_here[i] = 0;
    }

    for (trans_index = 0; trans_index < num_transactions; trans_index++) {
        /* Check if this is a siege of this nampla. */
        if (transaction[trans_index].type != BESIEGE_PLANET) { continue; }
        if (transaction[trans_index].x != nampla->x) { continue; }
        if (transaction[trans_index].y != nampla->y) { continue; }
        if (transaction[trans_index].z != nampla->z) { continue; }
        if (transaction[trans_index].pn != nampla->pn) { continue; }
        if (transaction[trans_index].number2 != species_number) { continue; }

        /* Check if alien ship is still in the same star system as the planet. */
        if (alien_number != transaction[trans_index].number1) {
            /* First transaction for this alien. */
            alien_number = transaction[trans_index].number1;
            if (!data_in_memory[alien_number - 1]) {
                fprintf(stderr, "\n\tData for species #%d should be in memory but is not!\n\n", alien_number);
                exit(-1);
            }
            alien = &spec_data[alien_number - 1];
            alien_nampla_base = namp_data[alien_number - 1];
            alien_ship_base = ship_data[alien_number - 1];

            new_alien = TRUE;
        }

        /* Find the alien ship. */
        found = FALSE;
        alien_ship = alien_ship_base - 1;
        for (i = 0; i < alien->num_ships; i++) {
            ++alien_ship;

            if (alien_ship->pn == 99) { continue; }

            if (strcmp(alien_ship->name, transaction[trans_index].name3) == 0) {
                found = TRUE;
                break;
            }
        }

        /* Check if alien ship is still at the siege location. */
        if (!found) {
            /* It must have jumped away and self-destructed, or was recycled. */
            continue;
        }
        if (alien_ship->x != nampla->x) { continue; }
        if (alien_ship->y != nampla->y) { continue; }
        if (alien_ship->z != nampla->z) { continue; }
        if (alien_ship->class == TR) { continue; }

        /* This nampla is under siege. */
        if (!under_siege) {
            log_string("\n    WARNING! PL ");
            log_string(nampla->name);
            log_string(" is under siege by the following:\n      ");
            under_siege = TRUE;
        }

        if (num_siege_ships++ > 0) { log_string(", "); }
        if (new_alien) {
            log_string(alien->name);
            log_char(' ');
            new_alien = FALSE;

            /* Check if this alien has a colony on the same planet. */
            alien_nampla = alien_nampla_base - 1;
            for (i = 0; i < alien->num_namplas; i++) {
                ++alien_nampla;

                if (alien_nampla->x != nampla->x) { continue; }
                if (alien_nampla->y != nampla->y) { continue; }
                if (alien_nampla->z != nampla->z) { continue; }
                if (alien_nampla->pn != nampla->pn) { continue; }

                /* Enemy population that will count for both detection AND assimilation. */
                alien_pop_units = alien_nampla->mi_base + alien_nampla->ma_base + alien_nampla->IUs_to_install +
                                  alien_nampla->AUs_to_install;

                /* Any base over 200.0 has only 5% effectiveness. */
                if (alien_pop_units > 2000) {
                    alien_pop_units = (alien_pop_units - 2000) / 20 + 2000;
                }

                /* Enemy population that counts ONLY for detection. */
                n = alien_nampla->pop_units + alien_nampla->item_quantity[CU] + alien_nampla->item_quantity[PD];

                if (alien_pop_units > 0) {
                    enemy_on_same_planet = TRUE;
                    pop_units_here[alien_number] = alien_pop_units;
                    total_alien_pop_here += alien_pop_units;
                } else if (n > 0) {
                    enemy_on_same_planet = TRUE;
                }

                if (alien_nampla->item_quantity[PD] == 0) { continue; }

                log_string("planetary defenses of PL ");
                log_string(alien_nampla->name);
                log_string(", ");

                n = (4 * alien_nampla->item_quantity[PD]) / 5;
                n = (n * (long) alien->tech_level[ML]) / ((long) species->tech_level[ML] + 1);
                total_siege_effectiveness += n;
                siege_effectiveness[alien_number] += n;
            }
        }
        log_string(ship_name(alien_ship));

        /* Determine the number of planets that this ship is besieging. */
        n = 0;
        for (j = 0; j < num_transactions; j++) {
            if (transaction[j].type != BESIEGE_PLANET) { continue; }
            if (transaction[j].number1 != alien_number) { continue; }
            if (strcmp(transaction[j].name3, alien_ship->name) != 0) { continue; }

            ++n;
        }

        /* Determine the effectiveness of this ship on the siege. */
        if (alien_ship->type == STARBASE) {
            i = alien_ship->tonnage;    /* One quarter of normal ships. */
        } else {
            i = 4 * (int) alien_ship->tonnage;
        }

        i = (i * (int) alien->tech_level[ML]) / ((int) species->tech_level[ML] + 1);

        i /= n;

        total_siege_effectiveness += i;
        siege_effectiveness[alien_number] += i;
    }

    if (under_siege) {
        log_string(".\n");
    } else {
        return;
    }

    /* Determine percent effectiveness of the siege. */
    total_effective_tonnage = 2500 * total_siege_effectiveness;

    if (nampla->mi_base + nampla->ma_base == 0) {
        siege_percent_effectiveness = -9999;    /* New colony with nothing installed yet. */
    } else {
        siege_percent_effectiveness = total_effective_tonnage /
                                      ((((long) species->tech_level[MI] * (long) nampla->mi_base) +
                                        ((long) species->tech_level[MA] * (long) nampla->ma_base)) / 10L);
    }

    if (siege_percent_effectiveness > 95) {
        siege_percent_effectiveness = 95;
    } else if (siege_percent_effectiveness == -9999) {
        log_string("      However, although planet is populated, it has no economic base.\n\n");
        return;
    } else if (siege_percent_effectiveness < 1) {
        log_string("      However, because of the weakness of the siege, it was completely ineffective!\n\n");
        return;
    }

    if (enemy_on_same_planet) {
        nampla->siege_eff = -siege_percent_effectiveness;
    } else {
        nampla->siege_eff = siege_percent_effectiveness;
    }

    log_string("      The siege is approximately ");
    log_int(siege_percent_effectiveness);
    log_string("% effective.\n");

    /* Add siege EU transfer(s). */
    EUs_for_distribution = (siege_percent_effectiveness * EUs_available_for_siege) / 100;

    total_EUs_stolen = 0;

    for (alien_number = 1; alien_number <= MAX_SPECIES; alien_number++) {
        n = siege_effectiveness[alien_number];
        if (n < 1) { continue; }
        alien = &spec_data[alien_number - 1];
        EUs_for_this_species = (n * EUs_for_distribution) / total_siege_effectiveness;
        if (EUs_for_this_species < 1) { continue; }
        total_EUs_stolen += EUs_for_this_species;
        log_string("      ");
        log_long(EUs_for_this_species);
        log_string(" economic unit");
        if (EUs_for_this_species > 1) {
            log_string("s were");
        } else {
            log_string(" was");
        }
        log_string(" lost and 25% of the amount was transferred to SP ");
        log_string(alien->name);
        log_string(".\n");

        if (first_pass) { continue; }

        /* Define this transaction and add to list of transactions. */
        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
            exit(-1);
        }

        trans_index = num_transactions++;
        transaction[trans_index].type = SIEGE_EU_TRANSFER;
        transaction[trans_index].donor = species_number;
        transaction[trans_index].recipient = alien_number;
        transaction[trans_index].value = EUs_for_this_species / 4;
        transaction[trans_index].x = nampla->x;
        transaction[trans_index].y = nampla->y;
        transaction[trans_index].z = nampla->z;
        transaction[trans_index].number1 = siege_percent_effectiveness;
        strcpy(transaction[trans_index].name1, species->name);
        strcpy(transaction[trans_index].name2, alien->name);
        strcpy(transaction[trans_index].name3, nampla->name);
    }
    log_char('\n');

    /* Correct balances. */
    if (special_colony) {
        species->econ_units -= total_EUs_stolen;
    } else {
        if (check_bounced(total_EUs_stolen)) {
            fprintf(stderr, "\nWARNING! Internal error! Should never reach this point!\n\n");
            exit(-1);
        }
    }

    if (!enemy_on_same_planet) { return; }

    /* All ships currently under construction may be detected by the besiegers and destroyed. */
    for (ship_index = 0; ship_index < species->num_ships; ship_index++) {
        ship = ship_base + ship_index;

        if (ship->status == UNDER_CONSTRUCTION && ship->x == nampla->x && ship->y == nampla->y &&
            ship->z == nampla->z && ship->pn == nampla->pn) {
            if (rnd(100) > siege_percent_effectiveness) { continue; }

            log_string("      ");
            log_string(ship_name(ship));
            log_string(", under construction when the siege began, was detected by the besiegers and destroyed!\n");
            if (!first_pass) { delete_ship(ship); }
        }
    }

    /* Check for assimilation. */
    if (nampla->status & HOME_PLANET) { return; }
    if (total_alien_pop_here < 1) { return; }

    total_besieged_pop = nampla->mi_base + nampla->ma_base + nampla->IUs_to_install + nampla->AUs_to_install;

    /* Any base over 200.0 has only 5% effectiveness. */
    if (total_besieged_pop > 2000) {
        total_besieged_pop = (total_besieged_pop - 2000) / 20 + 2000;
    }

    if (total_besieged_pop / total_alien_pop_here >= 5) { return; }
    if (siege_percent_effectiveness < 95) { return; }

    log_string("      PL ");
    log_string(nampla->name);
    log_string(" has become assimilated by the besieging species");
    log_string(" and is no longer under your control.\n\n");

    total_ib = nampla->mi_base;  /* My stupid compiler can't add an int and an unsigned short. */
    total_ib += nampla->IUs_to_install;
    total_ab = nampla->ma_base;
    total_ab += nampla->AUs_to_install;

    for (alien_number = 1; alien_number <= MAX_SPECIES; alien_number++) {
        n = pop_units_here[alien_number];
        if (n < 1) { continue; }

        shipyards_for_this_species = (n * nampla->shipyards) / total_alien_pop_here;

        ib_for_this_species = (n * total_ib) / total_alien_pop_here;
        total_ib -= ib_for_this_species;

        ab_for_this_species = (n * total_ab) / total_alien_pop_here;
        total_ab -= ab_for_this_species;

        if (ib_for_this_species == 0 && ab_for_this_species == 0) { continue; }

        if (first_pass) { continue; }

        /* Define this transaction and add to list of transactions. */
        if (num_transactions == MAX_TRANSACTIONS) {
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
            exit(-1);
        }

        trans_index = num_transactions++;
        transaction[trans_index].type = ASSIMILATION;
        transaction[trans_index].value = alien_number;
        transaction[trans_index].x = nampla->x;
        transaction[trans_index].y = nampla->y;
        transaction[trans_index].z = nampla->z;
        transaction[trans_index].pn = nampla->pn;
        transaction[trans_index].number1 = ib_for_this_species / 2;
        transaction[trans_index].number2 = ab_for_this_species / 2;
        transaction[trans_index].number3 = shipyards_for_this_species;
        strcpy(transaction[trans_index].name1, species->name);
        strcpy(transaction[trans_index].name2, nampla->name);
    }

    /* Erase the original colony. */
    balance = 0;
    EU_spending_limit = 0;
    raw_material_units = 0;
    production_capacity = 0;
    nampla->mi_base = 0;
    nampla->ma_base = 0;
    nampla->IUs_to_install = 0;
    nampla->AUs_to_install = 0;
    nampla->pop_units = 0;
    nampla->siege_eff = 0;
    nampla->status = COLONY;
    nampla->shipyards = 0;
    nampla->hiding = 0;
    nampla->hidden = 0;
    nampla->use_on_ambush = 0;

    for (i = 0; i < MAX_ITEMS; i++) { nampla->item_quantity[i] = 0; }
}


void do_RECYCLE_command(void) {
    int i, class, cargo;
    long recycle_value, original_cost, units_available;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get number of items to recycle. */
    i = get_value();

    if (i == 0) {
        goto recycle_ship;
    }    /* Not an item. */

    /* Get class of item. */
    class = get_class_abbr();

    if (class != ITEM_CLASS) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid item class in RECYCLE command.\n");
        return;
    }
    class = abbr_index;

    /* Make sure value is meaningful. */
    if (value == 0) { value = nampla->item_quantity[class]; }
    if (value == 0) { return; }
    if (value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid item count in RECYCLE command.\n");
        return;
    }

    /* Make sure that items exist. */
    units_available = nampla->item_quantity[class];
    if (value > units_available) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Attempt to recycle more items than are available.\n");
        return;
    }

    /* Determine recycle value. */
    if (class == TP) {
        recycle_value = (value * item_cost[class]) / (2L * (long) species->tech_level[BI]);
    } else if (class == RM) {
        recycle_value = value / 5L;
    } else {
        recycle_value = (value * item_cost[class]) / 2L;
    }

    /* Update inventories. */
    nampla->item_quantity[class] -= value;
    if (class == PD || class == CU) { nampla->pop_units += value; }
    species->econ_units += recycle_value;
    if (nampla->status & HOME_PLANET) { EU_spending_limit += recycle_value; }

    /* Log what was recycled. */
    log_string("    ");
    log_long(value);
    log_char(' ');
    log_string(item_name[class]);

    if (value > 1) {
        log_string("s were");
    } else {
        log_string(" was");
    }

    log_string(" recycled, generating ");
    log_long(recycle_value);
    log_string(" economic units.\n");

    return;


    recycle_ship:

    correct_spelling_required = TRUE;
    if (!get_ship()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship to be recycled does not exist.\n");
        return;
    }

    /* Make sure it didn't just jump. */
    if (ship->just_jumped) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship just jumped and is still in transit.\n");
        return;
    }

    /* Make sure item is at producing planet. */
    if (ship->x != nampla->x || ship->y != nampla->y || ship->z != nampla->z || ship->pn != nampla->pn) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship is not at the production planet.\n");
        return;
    }

    /* Calculate recycled value. */
    if (ship->class == TR || ship->type == STARBASE) {
        original_cost = ship_cost[ship->class] * ship->tonnage;
    } else {
        original_cost = ship_cost[ship->class];
    }

    if (ship->type == SUB_LIGHT) {
        original_cost = (3 * original_cost) / 4;
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        recycle_value = (original_cost - (long) ship->remaining_cost) / 2;
    } else {
        recycle_value = (3 * original_cost * (60 - (long) ship->age)) / 200;
    }

    species->econ_units += recycle_value;
    if (nampla->status & HOME_PLANET) { EU_spending_limit += recycle_value; }

    /* Log what was recycled. */
    log_string("    ");
    log_string(ship_name(ship));
    log_string(" was recycled, generating ");
    log_long(recycle_value);
    log_string(" economic units");

    /* Transfer cargo, if any, from ship to planet. */
    cargo = FALSE;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (ship->item_quantity[i] > 0) {
            nampla->item_quantity[i] += ship->item_quantity[i];
            cargo = TRUE;
        }
    }

    if (cargo) {
        log_string(". Cargo onboard ");
        log_string(ship_name(ship));
        log_string(" was first transferred to PL ");
        log_string(nampla->name);
    }

    log_string(".\n");

    /* Remove ship from inventory. */
    delete_ship(ship);
}


void do_REPAIR_command(void) {
    int i, j, n, x, y, z, age_reduction, num_dr_units;
    int total_dr_units, dr_units_used, max_age, desired_age;
    char *original_line_pointer;
    struct ship_data *damaged_ship;

    /* See if this is a "pool" repair. */
    if (get_value()) {
        x = value;
        get_value();
        y = value;
        get_value();
        z = value;
        if (get_value()) {
            desired_age = value;
        } else {
            desired_age = 0;
        }
        goto pool_repair;
    }

    /* Get the ship to be repaired. */
    original_line_pointer = input_line_pointer;
    if (!get_ship()) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        if (!get_ship()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship to be repaired does not exist.\n");
            return;
        }
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Item to be repaired is still under construction.\n");
        return;
    }

    if (ship->age < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship or starbase is too new to repair.\n");
        return;
    }

    /* Get number of damage repair units to use. */
    if (get_value()) {
        if (value == 0) {
            num_dr_units = ship->item_quantity[DR];
        } else {
            num_dr_units = value;
        }

        age_reduction = (16 * num_dr_units) / ship->tonnage;
        if (age_reduction > ship->age) {
            age_reduction = ship->age;
            n = age_reduction * ship->tonnage;
            num_dr_units = (n + 15) / 16;
        }
    } else {
        age_reduction = ship->age;
        n = age_reduction * ship->tonnage;
        num_dr_units = (n + 15) / 16;
    }

    /* Check if sufficient units are available. */
    if (num_dr_units > ship->item_quantity[DR]) {
        if (ship->item_quantity[DR] == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship does not have any DRs!\n");
            return;
        }
        fprintf(log_file, "! WARNING: %s", original_line);
        fprintf(log_file, "! Ship does not have %d DRs. Substituting %d for %d.\n", num_dr_units,
                ship->item_quantity[DR], num_dr_units);
        num_dr_units = ship->item_quantity[DR];
    }

    /* Check if repair will have any effect. */
    age_reduction = (16 * num_dr_units) / ship->tonnage;
    if (age_reduction < 1) {
        if (value == 0) { return; }
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %d DRs is not enough to do a repair.\n", num_dr_units);
        return;
    }

    /* Log what was repaired. */
    log_string("    ");
    log_string(ship_name(ship));
    log_string(" was repaired using ");
    log_int(num_dr_units);
    log_char(' ');
    log_string(item_name[DR]);
    if (num_dr_units != 1) { log_char('s'); }
    log_string(". Age went from ");
    log_int((int) ship->age);
    log_string(" to ");
    ship->age -= age_reduction;
    if (ship->age < 0) { ship->age = 0; }
    ship->item_quantity[DR] -= num_dr_units;
    log_int((int) ship->age);
    log_string(".\n");

    return;

    pool_repair:

    /* Get total number of DR units available. */
    total_dr_units = 0;
    ship = ship_base - 1;
    for (i = 0; i < species->num_ships; i++) {
        ++ship;
        if (ship->pn == 99) { continue; }
        if (ship->x != x) { continue; }
        if (ship->y != y) { continue; }
        if (ship->z != z) { continue; }
        total_dr_units += ship->item_quantity[DR];
        ship->special = 0;
    }

    /* Repair ships, starting with the most heavily damaged. */
    dr_units_used = 0;
    while (total_dr_units > 0) {
        /* Find most heavily damaged ship. */
        max_age = 0;
        ship = ship_base - 1;
        for (i = 0; i < species->num_ships; i++) {
            ++ship;
            if (ship->pn == 99) { continue; }
            if (ship->x != x) { continue; }
            if (ship->y != y) { continue; }
            if (ship->z != z) { continue; }
            if (ship->special != 0) { continue; }
            if (ship->status == UNDER_CONSTRUCTION) { continue; }
            n = ship->age;
            if (n > max_age) {
                max_age = n;
                damaged_ship = ship;
            }
        }

        if (max_age == 0) { break; }

        damaged_ship->special = 99;

        age_reduction = max_age - desired_age;
        n = age_reduction * damaged_ship->tonnage;
        num_dr_units = (n + 15) / 16;

        if (num_dr_units > total_dr_units) {
            num_dr_units = total_dr_units;
            age_reduction = (16 * num_dr_units) / damaged_ship->tonnage;
        }

        if (age_reduction < 1) { continue; }  /* This ship is too big. */

        log_string("    ");
        log_string(ship_name(damaged_ship));
        log_string(" was repaired using ");
        log_int(num_dr_units);
        log_char(' ');
        log_string(item_name[DR]);
        if (num_dr_units != 1) { log_char('s'); }
        log_string(". Age went from ");
        log_int((int) damaged_ship->age);
        log_string(" to ");
        damaged_ship->age -= age_reduction;
        if (damaged_ship->age < 0) { damaged_ship->age = 0; }
        log_int((int) damaged_ship->age);
        log_string(".\n");

        total_dr_units -= num_dr_units;
        dr_units_used += num_dr_units;
    }

    if (dr_units_used == 0) { return; }

    /* Subtract units used from ships at the location. */
    ship = ship_base - 1;
    for (i = 0; i < species->num_ships; i++) {
        ++ship;
        if (ship->pn == 99) { continue; }
        if (ship->x != x) { continue; }
        if (ship->y != y) { continue; }
        if (ship->z != z) { continue; }

        n = ship->item_quantity[DR];
        if (n < 1) { continue; }
        if (n > dr_units_used) { n = dr_units_used; }

        ship->item_quantity[DR] -= n;
        dr_units_used -= n;

        if (dr_units_used == 0) { break; }
    }
}


void do_RESEARCH_command(void) {
    int n, status, tech, initial_level, current_level, need_amount_to_spend;
    long cost, amount_spent, cost_for_one_level, funds_remaining, max_funds_available;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get amount to spend. */
    status = get_value();
    need_amount_to_spend = (status == 0);    /* Sometimes players reverse
						   the arguments. */
    /* Get technology. */
    if (get_class_abbr() != TECH_ID) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid or missing technology.\n");
        return;
    }
    tech = abbr_index;

    if (species->tech_knowledge[tech] == 0 && sp_tech_level[tech] == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Zero level can only be raised via TECH or TEACH.\n");
        return;
    }

    /* Get amount to spend if it was not obtained above. */
    if (need_amount_to_spend) { status = get_value(); }

    if (status == 0 || value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid or missing amount to spend!\n");
        return;
    }

    do_cost:

    if (value == 0) { value = balance; }
    if (value == 0) { return; }
    cost = value;

    /* Check if sufficient funds are available. */
    if (check_bounced(cost)) {
        max_funds_available = species->econ_units;
        if (max_funds_available > EU_spending_limit) {
            max_funds_available = EU_spending_limit;
        }
        max_funds_available += balance;

        if (max_funds_available > 0) {
            fprintf(log_file, "! WARNING: %s", input_line);
            fprintf(log_file, "! Insufficient funds. Substituting %ld for %ld.\n",
                    max_funds_available, cost);
            value = max_funds_available;
            goto do_cost;
        }

        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    /* Check if we already have knowledge of this technology. */
    funds_remaining = cost;
    amount_spent = 0;
    initial_level = sp_tech_level[tech];
    current_level = initial_level;
    while (current_level < species->tech_knowledge[tech]) {
        cost_for_one_level = current_level * current_level;
        cost_for_one_level -= cost_for_one_level / 4;    /* 25% discount. */
        if (funds_remaining < cost_for_one_level) { break; }
        funds_remaining -= cost_for_one_level;
        amount_spent += cost_for_one_level;
        ++current_level;
    }

    if (current_level > initial_level) {
        log_string("    Spent ");
        log_long(amount_spent);
        log_string(" raising ");
        log_string(tech_name[tech]);
        log_string(" tech level from ");
        log_int(initial_level);
        log_string(" to ");
        log_int(current_level);
        log_string(" using transferred knowledge.\n");

        sp_tech_level[tech] = current_level;
    }

    if (funds_remaining == 0) { return; }

    /* Increase in experience points is equal to whatever was not spent above. */
    species->tech_eps[tech] += funds_remaining;

    /* Log transaction. */
    log_string("    Spent ");
    log_long(funds_remaining);
    log_string(" on ");
    log_string(tech_name[tech]);
    log_string(" research.\n");
}


void do_SCAN_command(void) {
    int i, x, y, z;
    int found = get_ship();
    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid ship name in SCAN command.\n");
        return;
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }

    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }

    /* Log the result. */
    if (first_pass) {
        log_string("    A scan will be done by ");
        log_string(ship_name(ship));
        log_string(".\n");
        return;
    }

    /* Write scan of ship's location to log file. */
    x = ship->x;
    y = ship->y;
    z = ship->z;

    if (test_mode) {
        fprintf(log_file, "\nA scan will be done by %s.\n\n", ship_name(ship));
    } else {
        fprintf(log_file, "\nScan done by %s:\n\n", ship_name(ship));
        scan(x, y, z, TRUE);
    }
    fprintf(log_file, "\n");
}


void do_SEND_command(void) {
    int i, n, found, contact_word_number, contact_bit_number;
    char *temp_pointer;
    long num_available, contact_mask, item_count;
    struct nampla_data *nampla1, *nampla2;

    /* Get number of EUs to transfer. */
    i = get_value();

    /* Make sure value is meaningful. */
    if (i == 0 || value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid item count in SEND command.\n");
        return;
    }
    item_count = value;

    num_available = species->econ_units;
    if (item_count == 0) {
        item_count = num_available;
    }
    if (item_count == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You do not have any EUs available!\n");
        return;
    }
    if (num_available < item_count) {
        if (num_available == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! You do not have any EUs!\n");
            return;
        }
        fprintf(log_file, "! WARNING: %s", input_line);
        fprintf(log_file, "! You do not have %ld EUs! Substituting %ld for %ld.\n",
                item_count, num_available, item_count);
        item_count = num_available;
    }

    /* Get destination of transfer. */
    found = get_species_name();
    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid species name in SEND command.\n");
        return;
    }
    fprintf(log_file, "!!! Order: SEND %ld/%d SP%02d %s\n",
            num_available, species->econ_units, g_spec_number, g_spec_name);

    /* Check if we've met this species and make sure it is not an enemy. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't SEND to a species you haven't met.\n");
        return;
    }
    if (species->enemy[contact_word_number] & contact_mask) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not SEND economic units to an ENEMY.\n");
        return;
    }

    /* Make the transfer and log the result. */
    log_string("    ");
    log_long(item_count);
    log_string(" economic unit");
    if (item_count > 1) {
        log_string("s were");
    } else {
        log_string(" was");
    }
    log_string(" sent to SP ");
    log_string(g_spec_name);
    log_string(".\n");
    species->econ_units -= item_count;

    if (first_pass) {
        return;
    }

    /* Define this transaction. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    n = num_transactions++;
    transaction[n].type = EU_TRANSFER;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = item_count;
    strcpy(transaction[n].name1, species->name);
    strcpy(transaction[n].name2, g_spec_name);

    /* Make the transfer to the alien. */
    spec_data[g_spec_number - 1].econ_units += item_count;
    data_modified[g_spec_number - 1] = TRUE;
}


void do_SHIPYARD_command(void) {
    long cost;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Make sure this is not a mining or resort colony. */
    if ((nampla->status & MINING_COLONY) || (nampla->status & RESORT_COLONY)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not build shipyards on a mining or resort colony!\n");
        return;
    }

    /* Check if planet has already built a shipyard. */
    if (shipyard_built) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Only one shipyard can be built per planet per turn!\n");
        return;
    }

    /* Check if sufficient funds are available. */
    cost = 10 * species->tech_level[MA];
    if (check_bounced(cost)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    ++nampla->shipyards;

    shipyard_built = TRUE;

    /* Log transaction. */
    log_string("    Spent ");
    log_long(cost);
    log_string(" to increase shipyard capacity by 1.\n");
}


void do_TEACH_command(void) {
    int i, tech, contact_word_number, contact_bit_number, max_level_specified, need_technology;
    char *temp_ptr;
    short max_tech_level;
    long contact_mask;

    /* Get technology. */
    temp_ptr = input_line_pointer;
    if (get_class_abbr() != TECH_ID) {
        need_technology = TRUE;        /* Sometimes players accidentally reverse the arguments. */
        input_line_pointer = temp_ptr;
    } else {
        need_technology = FALSE;
        tech = abbr_index;
    }

    /* See if a maximum tech level was specified. */
    max_level_specified = get_value();
    if (max_level_specified) {
        max_tech_level = value;
        if (max_tech_level > species->tech_level[tech]) {
            max_tech_level = species->tech_level[tech];
        }
    } else {
        max_tech_level = species->tech_level[tech];
    }

    /* Get the technology now if it wasn't obtained above. */
    if (need_technology) {
        if (get_class_abbr() != TECH_ID) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid or missing technology!\n");
            return;
        }
        tech = abbr_index;
    }

    /* Get species to transfer knowledge to. */
    if (!get_species_name()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid species name in TEACH command.\n");
        return;
    }

    /* Check if we've met this species and make sure it is not an enemy. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't TEACH a species you haven't met.\n");
        return;
    }

    if (species->enemy[contact_word_number] & contact_mask) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't TEACH an ENEMY.\n");
        return;
    }

    if (first_pass) { return; }

    /* Define this transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    i = num_transactions++;
    transaction[i].type = KNOWLEDGE_TRANSFER;
    transaction[i].donor = species_number;
    transaction[i].recipient = g_spec_number;
    transaction[i].value = tech;
    strcpy(transaction[i].name1, species->name);
    transaction[i].number3 = max_tech_level;
}


void do_TECH_command(void) {
    int i, tech, contact_word_number, contact_bit_number, max_level_specified, max_tech_level, max_cost_specified, need_technology;
    long contact_mask, max_cost;

    /* See if a maximum cost was specified. */
    max_cost_specified = get_value();
    if (max_cost_specified) {
        max_cost = value;
    } else {
        max_cost = 0;
    }

    /* Get technology. */
    if (get_class_abbr() != TECH_ID) {
        need_technology = TRUE;        /* Sometimes players accidentally reverse the arguments. */
    } else {
        need_technology = FALSE;
        tech = abbr_index;
    }

    /* See if a maximum tech level was specified. */
    max_level_specified = get_value();
    max_tech_level = value;

    /* Get species to transfer tech to. */
    if (!get_species_name()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid species name in TECH command.\n");
        return;
    }

    /* Check if we've met this species and make sure it is not an enemy. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask) == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't transfer tech to a species you haven't met.\n");
        return;
    }
    if (species->enemy[contact_word_number] & contact_mask) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't transfer tech to an ENEMY.\n");
        return;
    }

    /* Get the technology now if it wasn't obtained above. */
    if (need_technology) {
        if (get_class_abbr() != TECH_ID) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Invalid or missing technology!\n");
            return;
        }
        tech = abbr_index;
    }

    /* Make sure there isn't already a transfer of the same technology from the same donor species to the same recipient species. */
    for (i = 0; i < num_transactions; i++) {
        if (transaction[i].type != TECH_TRANSFER) { continue; }
        if (transaction[i].value != tech) { continue; }
        if (transaction[i].number1 != species_number) { continue; }
        if (transaction[i].number2 != g_spec_number) { continue; }

        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You can't transfer the same tech to the same species more than once!\n");
        return;
    }

    /* Log the result. */
    log_string("    Will attempt to transfer ");
    log_string(tech_name[tech]);
    log_string(" technology to SP ");
    log_string(g_spec_name);
    log_string(".\n");

    if (first_pass) { return; }

    /* Define this transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS) {
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
        exit(-1);
    }

    i = num_transactions++;
    transaction[i].type = TECH_TRANSFER;
    transaction[i].donor = species_number;
    transaction[i].recipient = g_spec_number;
    transaction[i].value = tech;
    strcpy(transaction[i].name1, species->name);
    transaction[i].number1 = max_cost;
    strcpy(transaction[i].name2, g_spec_name);
    if (max_level_specified && (max_tech_level < species->tech_level[tech])) {
        transaction[i].number3 = max_tech_level;
    } else {
        transaction[i].number3 = species->tech_level[tech];
    }
}


void do_TELESCOPE_command(void) {
    int i, n, found, range_in_parsecs, max_range, alien_index, alien_number, alien_nampla_index, alien_ship_index, location_printed, industry, detection_chance, num_obs_locs, alien_name_printed, loc_index, success_chance, something_found;
    long x, y, z, max_distance, max_distance_squared, delta_x, delta_y, delta_z, distance_squared;
    char planet_type[32], obs_x[MAX_OBS_LOCS], obs_y[MAX_OBS_LOCS], obs_z[MAX_OBS_LOCS];
    struct species_data *alien;
    struct nampla_data *alien_nampla;
    struct ship_data *starbase, *alien_ship;

    found = get_ship();
    if (!found || ship->type != STARBASE) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid starbase name in TELESCOPE command.\n");
        return;
    }
    starbase = ship;

    /* Make sure starbase does not get more than one TELESCOPE order per turn. */
    if (starbase->dest_z != 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! A starbase may only be given one TELESCOPE order per turn.\n");
        return;
    }
    starbase->dest_z = 99;

    /* Get range of telescope. */
    range_in_parsecs = starbase->item_quantity[GT] / 2;
    if (range_in_parsecs < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Starbase is not carrying enough gravitic telescope units.\n");
        return;
    }

    /* Log the result. */
    if (first_pass) {
        log_string("    A gravitic telescope at ");
        log_int(starbase->x);
        log_char(' ');
        log_int(starbase->y);
        log_char(' ');
        log_int(starbase->z);
        log_string(" will be operated by ");
        log_string(ship_name(starbase));
        log_string(".\n");
        return;
    }

    /* Define range parameters. */
    max_range = (int) species->tech_level[GV] / 10;
    if (range_in_parsecs > max_range) { range_in_parsecs = max_range; }

    x = starbase->x;
    y = starbase->y;
    z = starbase->z;

    max_distance = range_in_parsecs;
    max_distance_squared = max_distance * max_distance;

    /* First pass. Simply create a list of X Y Z locations that have observable aliens. */
    num_obs_locs = 0;
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
        if (!data_in_memory[alien_index]) { continue; }

        alien_number = alien_index + 1;
        if (alien_number == species_number) { continue; }

        alien = &spec_data[alien_index];

        alien_nampla = namp_data[alien_index] - 1;
        for (alien_nampla_index = 0; alien_nampla_index < alien->num_namplas;
             alien_nampla_index++) {
            ++alien_nampla;

            if ((alien_nampla->status & POPULATED) == 0) { continue; }

            delta_x = x - alien_nampla->x;
            delta_y = y - alien_nampla->y;
            delta_z = z - alien_nampla->z;
            distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
                               + (delta_z * delta_z);

            if (distance_squared == 0) { continue; }  /* Same loc as telescope. */
            if (distance_squared > max_distance_squared) { continue; }

            found = FALSE;
            for (i = 0; i < num_obs_locs; i++) {
                if (alien_nampla->x != obs_x[i]) { continue; }
                if (alien_nampla->y != obs_y[i]) { continue; }
                if (alien_nampla->z != obs_z[i]) { continue; }

                found = TRUE;
                break;
            }
            if (!found) {
                if (num_obs_locs == MAX_OBS_LOCS) {
                    fprintf(stderr, "\n\nInternal error! MAX_OBS_LOCS exceeded in do_TELESCOPE_command!\n\n");
                    exit(-1);
                }
                obs_x[num_obs_locs] = alien_nampla->x;
                obs_y[num_obs_locs] = alien_nampla->y;
                obs_z[num_obs_locs] = alien_nampla->z;

                ++num_obs_locs;
            }
        }

        alien_ship = ship_data[alien_index] - 1;
        for (alien_ship_index = 0; alien_ship_index < alien->num_ships;
             alien_ship_index++) {
            ++alien_ship;

            if (alien_ship->status == UNDER_CONSTRUCTION) { continue; }
            if (alien_ship->status == ON_SURFACE) { continue; }
            if (alien_ship->item_quantity[FD] == alien_ship->tonnage) { continue; }

            delta_x = x - alien_ship->x;
            delta_y = y - alien_ship->y;
            delta_z = z - alien_ship->z;
            distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
                               + (delta_z * delta_z);

            if (distance_squared == 0) { continue; }  /* Same loc as telescope. */
            if (distance_squared > max_distance_squared) { continue; }

            found = FALSE;
            for (i = 0; i < num_obs_locs; i++) {
                if (alien_ship->x != obs_x[i]) { continue; }
                if (alien_ship->y != obs_y[i]) { continue; }
                if (alien_ship->z != obs_z[i]) { continue; }

                found = TRUE;
                break;
            }
            if (!found) {
                if (num_obs_locs == MAX_OBS_LOCS) {
                    fprintf(stderr, "\n\nInternal error! MAX_OBS_LOCS exceeded in do_TELESCOPE_command!\n\n");
                    exit(-1);
                }
                obs_x[num_obs_locs] = alien_ship->x;
                obs_y[num_obs_locs] = alien_ship->y;
                obs_z[num_obs_locs] = alien_ship->z;

                ++num_obs_locs;
            }
        }
    }

    /* Operate the gravitic telescope. */
    log_string("\n  Results of operation of gravitic telescope by ");
    log_string(ship_name(starbase));
    log_string(" (location = ");
    log_int(starbase->x);
    log_char(' ');
    log_int(starbase->y);
    log_char(' ');
    log_int(starbase->z);
    log_string(", max range = ");
    log_int(range_in_parsecs);
    log_string(" parsecs):\n");

    something_found = FALSE;

    for (loc_index = 0; loc_index < num_obs_locs; loc_index++) {
        x = obs_x[loc_index];
        y = obs_y[loc_index];
        z = obs_z[loc_index];

        location_printed = FALSE;

        for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
            if (!data_in_memory[alien_index]) { continue; }

            alien_number = alien_index + 1;
            if (alien_number == species_number) { continue; }

            alien = &spec_data[alien_index];

            alien_name_printed = FALSE;

            alien_nampla = namp_data[alien_index] - 1;
            for (alien_nampla_index = 0; alien_nampla_index < alien->num_namplas;
                 alien_nampla_index++) {
                ++alien_nampla;

                if ((alien_nampla->status & POPULATED) == 0) { continue; }
                if (alien_nampla->x != x) { continue; }
                if (alien_nampla->y != y) { continue; }
                if (alien_nampla->z != z) { continue; }

                industry = alien_nampla->mi_base + alien_nampla->ma_base;

                success_chance = species->tech_level[GV];
                success_chance += starbase->item_quantity[GT];
                success_chance += (industry - 500) / 20;
                if (alien_nampla->hiding || alien_nampla->hidden) {
                    success_chance /= 10;
                }

                if (rnd(100) > success_chance) { continue; }

                if (industry < 100) {
                    industry = (industry + 5) / 10;
                } else {
                    industry = ((industry + 50) / 100) * 10;
                }

                if (alien_nampla->status & HOME_PLANET) {
                    strcpy(planet_type, "Home planet");
                } else if (alien_nampla->status & RESORT_COLONY) {
                    strcpy(planet_type, "Resort colony");
                } else if (alien_nampla->status & MINING_COLONY) {
                    strcpy(planet_type, "Mining colony");
                } else {
                    strcpy(planet_type, "Colony");
                }

                if (!alien_name_printed) {
                    if (!location_printed) {
                        fprintf(log_file, "\n    %ld%3ld%3ld:\n", x, y, z);
                        location_printed = TRUE;
                        something_found = TRUE;
                    }
                    fprintf(log_file, "      SP %s:\n", alien->name);
                    alien_name_printed = TRUE;
                }

                fprintf(log_file, "\t#%d: %s PL %s (%d)\n",
                        alien_nampla->pn, planet_type, alien_nampla->name, industry);
            }

            alien_ship = ship_data[alien_index] - 1;
            for (alien_ship_index = 0; alien_ship_index < alien->num_ships;
                 alien_ship_index++) {
                ++alien_ship;

                if (alien_ship->x != x) { continue; }
                if (alien_ship->y != y) { continue; }
                if (alien_ship->z != z) { continue; }
                if (alien_ship->status == UNDER_CONSTRUCTION) { continue; }
                if (alien_ship->status == ON_SURFACE) { continue; }
                if (alien_ship->item_quantity[FD] == alien_ship->tonnage) { continue; }

                success_chance = species->tech_level[GV];
                success_chance += starbase->item_quantity[GT];
                success_chance += alien_ship->tonnage - 10;
                if (alien_ship->type == STARBASE) { success_chance *= 2; }
                if (alien_ship->class == TR) {
                    success_chance = (3 * success_chance) / 2;
                }
                if (rnd(100) > success_chance) { continue; }

                if (!alien_name_printed) {
                    if (!location_printed) {
                        fprintf(log_file, "\n    %ld%3ld%3ld:\n", x, y, z);
                        location_printed = TRUE;
                        something_found = TRUE;
                    }
                    fprintf(log_file, "      SP %s:\n", alien->name);
                    alien_name_printed = TRUE;
                }

                truncate_name = FALSE;
                fprintf(log_file, "\t%s", ship_name(alien_ship));
                truncate_name = TRUE;

                /* See if alien detected that it is being observed. */
                if (alien_ship->type == STARBASE) {
                    detection_chance = 2 * alien_ship->item_quantity[GT];
                    if (detection_chance > 0) {
                        fprintf(log_file, " <- %d GTs installed!",
                                alien_ship->item_quantity[GT]);
                    }
                } else {
                    detection_chance = 0;
                }

                fprintf(log_file, "\n");

                detection_chance += 2 *
                                    ((int) alien->tech_level[GV] - (int) species->tech_level[GV]);

                if (rnd(100) > detection_chance) { continue; }

                /* Define this transaction. */
                if (num_transactions == MAX_TRANSACTIONS) {
                    fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                    exit(-1);
                }

                n = num_transactions++;
                transaction[n].type = TELESCOPE_DETECTION;
                transaction[n].x = starbase->x;
                transaction[n].y = starbase->y;
                transaction[n].z = starbase->z;
                transaction[n].number1 = alien_number;
                strcpy(transaction[n].name1, ship_name(alien_ship));
            }
        }
    }

    if (something_found) {
        log_char('\n');
    } else {
        log_string("    No alien ships or planets were detected.\n\n");
    }
}


void do_TERRAFORM_command(void) {
    int i, j, ls_needed, num_plants, got_required_gas, correct_percentage;
    struct planet_data *home_planet, *colony_planet;

    /* Get number of TPs to use. */
    if (get_value()) {
        num_plants = value;
    } else {
        num_plants = 0;
    }

    /* Get planet where terraforming is to be done. */
    if (!get_location() || nampla == NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid planet name in TERRAFORM command.\n");
        return;
    }

    /* Make sure planet is not a home planet. */
    if (nampla->status & HOME_PLANET) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Terraforming may not be done on a home planet.\n");
        return;
    }

    /* Find out how many terraforming plants are needed. */
    colony_planet = planet_base + (long) nampla->planet_index;
    home_planet = planet_base + (long) nampla_base->planet_index;

    ls_needed = life_support_needed(species, home_planet, colony_planet);

    if (ls_needed == 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Colony does not need to be terraformed.\n");
        return;
    }

    if (num_plants == 0) { num_plants = nampla->item_quantity[TP]; }
    if (num_plants > ls_needed) { num_plants = ls_needed; }
    num_plants = num_plants / 3;
    num_plants *= 3;

    if (num_plants < 3) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! At least three TPs are needed to terraform.\n");
        return;
    }

    if (num_plants > nampla->item_quantity[TP]) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! PL %s doesn't have that many TPs!\n",
                nampla->name);
        return;
    }

    /* Log results. */
    log_string("    PL ");
    log_string(nampla->name);
    log_string(" was terraformed using ");
    log_int(num_plants);
    log_string(" Terraforming Unit");
    if (num_plants != 1) { log_char('s'); }
    log_string(".\n");

    nampla->item_quantity[TP] -= num_plants;
    planet_data_modified = TRUE;

    /* Terraform the planet. */
    while (num_plants > 1) {
        got_required_gas = FALSE;
        correct_percentage = FALSE;
        for (j = 0; j < 4; j++)    /* Check gases on planet. */
        {
            for (i = 0; i < 6; i++)    /* Compare with poisonous gases. */
            {
                if (colony_planet->gas[j] == species->required_gas) {
                    got_required_gas = j + 1;

                    if (colony_planet->gas_percent[j] >= species->required_gas_min
                        && colony_planet->gas_percent[j] <= species->required_gas_max) {
                        correct_percentage = TRUE;
                    }
                }

                if (species->poison_gas[i] == colony_planet->gas[j]) {
                    colony_planet->gas[j] = 0;
                    colony_planet->gas_percent[j] = 0;

                    /* Make sure percentages add up to 100%. */
                    fix_gases(colony_planet);

                    goto next_change;
                }
            }
        }

        if (got_required_gas && correct_percentage) { goto do_temp; }

        j = 0;    /* If all 4 gases are neutral gases, replace the first one. */

        if (got_required_gas) {
            j = got_required_gas - 1;
        } else {
            for (i = 0; i < 4; i++) {
                if (colony_planet->gas_percent[i] == 0) {
                    j = i;
                    break;
                }
            }
        }

        colony_planet->gas[j] = species->required_gas;
        i = species->required_gas_max - species->required_gas_min;
        colony_planet->gas_percent[j] = species->required_gas_min + rnd(i);

        /* Make sure percentages add up to 100%. */
        fix_gases(colony_planet);

        goto next_change;

        do_temp:

        if (colony_planet->temperature_class != home_planet->temperature_class) {
            if (colony_planet->temperature_class > home_planet->temperature_class) {
                --colony_planet->temperature_class;
            } else {
                ++colony_planet->temperature_class;
            }

            goto next_change;
        }

        if (colony_planet->pressure_class != home_planet->pressure_class) {
            if (colony_planet->pressure_class > home_planet->pressure_class) {
                --colony_planet->pressure_class;
            } else {
                ++colony_planet->pressure_class;
            }
        }

        next_change:

        num_plants -= 3;
    }
}


void do_TRANSFER_command(void) {
    int i, n, item_class, item_count, capacity, transfer_type;
    int attempt_during_siege, siege_1_chance, siege_2_chance;
    int alien_number, first_try, both_args_present, need_destination;
    char c, x1, x2, y1, y2, z1, z2, *original_line_pointer, *temp_ptr;
    char already_notified[MAX_SPECIES];
    long num_available, original_count;
    struct nampla_data *nampla1, *nampla2, *temp_nampla;
    struct ship_data *ship1, *ship2;

    /* Get number of items to transfer. */
    i = get_value();

    /* Make sure value is meaningful. */
    if (i == 0 || value < 0) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Invalid item count in TRANSFER command.\n");
        return;
    }
    original_count = value;
    item_count = value;

    /* Get class of item. */
    item_class = get_class_abbr();

    if (item_class != ITEM_CLASS) {
        /* Players sometimes accidentally use "MI" for "IU" or "MA" for "AU". */
        if (item_class == TECH_ID && abbr_index == MI) {
            abbr_index = IU;
        } else if (item_class == TECH_ID && abbr_index == MA) {
            abbr_index = AU;
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid item class!\n");
            return;
        }
    }
    item_class = abbr_index;

    /* Get source of transfer. */
    nampla1 = NULL;
    nampla2 = NULL;
    original_line_pointer = input_line_pointer;
    if (!get_transfer_point()) {
        /* Check for missing comma or tab after source name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        if (!get_transfer_point()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid source location in TRANSFER command.\n");
            return;
        }
    }

    /* Test if the order has both a source and a destination.
     * Sometimes, the player will accidentally omit the source if it's "obvious". */
    temp_ptr = input_line_pointer;
    both_args_present = FALSE;
    while (1) {
        c = *temp_ptr++;
        if (c == ';' || c == '\n') { /* End of order. */ break; }
        if (isalpha(c)) {
            both_args_present = TRUE;
            break;
        }
    }

    need_destination = TRUE;

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS) {
        ship1 = ship;

        if (ship1->status == UNDER_CONSTRUCTION) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s is still under construction!\n", ship_name(ship1));
            return;
        }

        if (ship1->status == FORCED_JUMP || ship1->status == JUMPED_IN_COMBAT) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
            return;
        }

        x1 = ship1->x;
        y1 = ship1->y;
        z1 = ship1->z;

        num_available = ship1->item_quantity[item_class];

        check_ship_items:

        if (item_count == 0) { item_count = num_available; }
        if (item_count == 0) { return; }

        if (num_available < item_count) {
            if (both_args_present)    /* Change item count to "0". */
            {
                if (num_available == 0) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", original_line);
                    fprintf(log_file, "!!! %s does not have specified item(s)!\n", ship_name(ship1));
                    return;
                }

                fprintf(log_file, "! WARNING: %s", original_line);
                fprintf(log_file, "! Ship does not have %d units. Substituting %ld for %d!\n",
                        item_count, num_available, item_count);
                item_count = 0;
                goto check_ship_items;
            }

            /* Check if ship is at a planet that has the items.
             * If so, we'll assume that the planet is the source and the ship is the destination.
             * We'll look first for a planet that the ship is actually landed on or orbiting.
             * If that fails, then we'll look for a planet in the same sector. */
            first_try = TRUE;

            next_ship_try:

            nampla1 = nampla_base - 1;
            for (i = 0; i < species->num_namplas; i++) {
                ++nampla1;
                if (nampla1->x != ship1->x) { continue; }
                if (nampla1->y != ship1->y) { continue; }
                if (nampla1->z != ship1->z) { continue; }
                if (first_try) {
                    if (nampla1->pn != ship1->pn) {
                        continue;
                    }
                }

                num_available = nampla1->item_quantity[item_class];
                if (num_available < item_count) { continue; }

                ship = ship1;        /* Destination. */
                transfer_type = 1;    /* Source = planet. */
                abbr_type = SHIP_CLASS;    /* Destination type. */

                need_destination = FALSE;

                goto get_destination;
            }

            if (first_try) {
                first_try = FALSE;
                goto next_ship_try;
            }

            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s does not have specified item(s)!\n", ship_name(ship1));
            return;
        }

        transfer_type = 0;    /* Source = ship. */
    } else {
        /* Source is a planet. */
        nampla1 = nampla;

        x1 = nampla1->x;
        y1 = nampla1->y;
        z1 = nampla1->z;

        num_available = nampla1->item_quantity[item_class];

        check_planet_items:

        if (item_count == 0) { item_count = num_available; }
        if (item_count == 0) { return; }

        if (num_available < item_count) {
            if (both_args_present) {
                /* Change item count to "0". */
                if (num_available == 0) {
                    fprintf(log_file, "!!! Order ignored:\n");
                    fprintf(log_file, "!!! %s", original_line);
                    fprintf(log_file, "!!! PL %s does not have specified item(s)!\n", nampla1->name);
                    return;
                }

                fprintf(log_file, "! WARNING: %s", original_line);
                fprintf(log_file, "! Planet does not have %d units. Substituting %ld for %d!\n",
                        item_count, num_available, item_count);
                item_count = 0;
                goto check_planet_items;
            }

            /* Check if another planet in the same sector has the items.
             * If so, we'll assume that it is the source and that the named planet is the destination. */
            temp_nampla = nampla_base - 1;
            for (i = 0; i < species->num_namplas; i++) {
                ++temp_nampla;
                if (temp_nampla->x != nampla1->x) { continue; }
                if (temp_nampla->y != nampla1->y) { continue; }
                if (temp_nampla->z != nampla1->z) { continue; }

                num_available = temp_nampla->item_quantity[item_class];
                if (num_available < item_count) { continue; }

                nampla = nampla1;    /* Destination. */
                nampla1 = temp_nampla;    /* Source. */
                transfer_type = 1;    /* Source = planet. */
                abbr_type = PLANET_ID;    /* Destination type. */

                need_destination = FALSE;

                goto get_destination;
            }

            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! PL %s does not have specified item(s)!\n", nampla1->name);
            return;
        }

        transfer_type = 1;    /* Source = planet. */
    }

    get_destination:

    /* Get destination of transfer. */
    if (need_destination) {
        if (!get_transfer_point()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid destination location.\n");
            return;
        }
    }

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS) {
        ship2 = ship;

        if (ship2->status == UNDER_CONSTRUCTION) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! %s is still under construction!\n", ship_name(ship2));
            return;
        }

        if (ship2->status == FORCED_JUMP || ship2->status == JUMPED_IN_COMBAT) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
            return;
        }

        /* Check if destination ship has sufficient carrying capacity. */
        if (ship2->class == TR) {
            capacity = (10 + ((int) ship2->tonnage / 2)) * (int) ship2->tonnage;
        } else if (ship2->class == BA) {
            capacity = 10 * ship2->tonnage;
        } else {
            capacity = ship2->tonnage;
        }

        for (i = 0; i < MAX_ITEMS; i++) {
            capacity -= ship2->item_quantity[i] * item_carry_capacity[i];
        }

        do_capacity:

        if (original_count == 0) {
            i = capacity / item_carry_capacity[item_class];
            if (i < item_count) { item_count = i; }
            if (item_count == 0) { return; }
        }

        if (capacity < item_count * item_carry_capacity[item_class]) {
            fprintf(log_file, "! WARNING: %s", original_line);
            fprintf(log_file, "! %s does not have sufficient carrying capacity!",
                    ship_name(ship2));
            fprintf(log_file, " Changed %ld to 0.\n", original_count);
            original_count = 0;
            goto do_capacity;
        }

        x2 = ship2->x;
        y2 = ship2->y;
        z2 = ship2->z;
    } else {
        nampla2 = nampla;

        x2 = nampla2->x;
        y2 = nampla2->y;
        z2 = nampla2->z;

        transfer_type |= 2;

        /* If this is the post-arrival phase, then make sure the planet is populated. */
        if (post_arrival_phase && ((nampla2->status & POPULATED) == 0)) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Destination planet must be populated for post-arrival TRANSFERs.\n");
            return;
        }
    }

    /* Check if source and destination are in same system. */
    if (x1 != x2 || y1 != y2 || z1 != z2) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Source and destination are not at same 'x y z' in TRANSFER command.\n");
        return;
    }

    /* Check for siege. */
    siege_1_chance = 0;
    siege_2_chance = 0;
    if (transfer_type == 3    /* Planet to planet. */
        && (nampla1->siege_eff != 0 || nampla2->siege_eff != 0)) {
        if (nampla1->siege_eff >= 0) {
            siege_1_chance = nampla1->siege_eff;
        } else {
            siege_1_chance = -nampla1->siege_eff;
        }

        if (nampla2->siege_eff >= 0) {
            siege_2_chance = nampla2->siege_eff;
        } else {
            siege_2_chance = -nampla2->siege_eff;
        }

        attempt_during_siege = TRUE;
    } else {
        attempt_during_siege = FALSE;
    }

    /* Make the transfer and log the result. */
    log_string("    ");

    if (attempt_during_siege && first_pass) {
        log_string("An attempt will be made to transfer ");
    }

    log_int(item_count);
    log_char(' ');
    log_string(item_name[item_class]);

    if (attempt_during_siege && first_pass) {
        if (item_count > 1) { log_char('s'); }
        log_char(' ');
    } else {
        if (item_count > 1) {
            log_string("s were transferred from ");
        } else {
            log_string(" was transferred from ");
        }
    }

    switch (transfer_type) {
        case 0:        /* Ship to ship. */
            ship1->item_quantity[item_class] -= item_count;
            ship2->item_quantity[item_class] += item_count;
            log_string(ship_name(ship1));
            log_string(" to ");
            log_string(ship_name(ship2));
            log_char('.');
            break;

        case 1:        /* Planet to ship. */
            nampla1->item_quantity[item_class] -= item_count;
            ship2->item_quantity[item_class] += item_count;
            if (item_class == CU) {
                if (nampla1 == nampla_base) {
                    ship2->loading_point = 9999;    /* Home planet. */
                } else {
                    ship2->loading_point = (nampla1 - nampla_base);
                }
            }
            log_string("PL ");
            log_string(nampla1->name);
            log_string(" to ");
            log_string(ship_name(ship2));
            log_char('.');
            break;

        case 2:        /* Ship to planet. */
            ship1->item_quantity[item_class] -= item_count;
            nampla2->item_quantity[item_class] += item_count;
            log_string(ship_name(ship1));
            log_string(" to PL ");
            log_string(nampla2->name);
            log_char('.');
            break;

        case 3:        /* Planet to planet. */
            nampla1->item_quantity[item_class] -= item_count;
            nampla2->item_quantity[item_class] += item_count;

            log_string("PL ");
            log_string(nampla1->name);
            log_string(" to PL ");
            log_string(nampla2->name);
            if (attempt_during_siege) { log_string(" despite the siege"); }
            log_char('.');

            if (first_pass) { break; }

            /* Check if either planet is under siege and if transfer
                was detected by the besiegers. */
            if (rnd(100) > siege_1_chance && rnd(100) > siege_2_chance) {
                break;
            }

            log_string(" However, the transfer was detected by the besiegers and the items were destroyed!!!");
            nampla2->item_quantity[item_class] -= item_count;

            for (i = 0; i < MAX_SPECIES; i++) { already_notified[i] = FALSE; }

            for (i = 0; i < num_transactions; i++) {
                /* Find out who is besieging this planet. */
                if (transaction[i].type != BESIEGE_PLANET) { continue; }
                if (transaction[i].x != nampla->x) { continue; }
                if (transaction[i].y != nampla->y) { continue; }
                if (transaction[i].z != nampla->z) { continue; }
                if (transaction[i].pn != nampla->pn) { continue; }
                if (transaction[i].number2 != species_number) { continue; }

                alien_number = transaction[i].number1;

                if (already_notified[alien_number - 1]) { continue; }

                /* Define a 'detection' transaction. */
                if (num_transactions == MAX_TRANSACTIONS) {
                    fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
                    exit(-1);
                }

                n = num_transactions++;
                transaction[n].type = DETECTION_DURING_SIEGE;
                transaction[n].value = 4;    /* Transfer of items. */
                transaction[n].number1 = item_count;
                transaction[n].number2 = item_class;
                if (siege_1_chance > siege_2_chance) {
                    /* Besieged planet is the source of the transfer. */
                    transaction[n].value = 4;
                    strcpy(transaction[n].name1, nampla1->name);
                    strcpy(transaction[n].name2, nampla2->name);
                } else {
                    /* Besieged planet is the destination of the transfer. */
                    transaction[n].value = 5;
                    strcpy(transaction[n].name1, nampla2->name);
                    strcpy(transaction[n].name2, nampla1->name);
                }
                strcpy(transaction[n].name3, species->name);
                transaction[n].number3 = alien_number;

                already_notified[alien_number - 1] = TRUE;
            }

            break;

        default:    /* Internal error. */
            fprintf(stderr, "\n\n\tInternal error: transfer type!\n\n");
            exit(-1);
    }

    log_char('\n');

    if (nampla1 != NULL) { check_population(nampla1); }
    if (nampla2 != NULL) { check_population(nampla2); }
}


void do_UNLOAD_command(void) {
    int i, found, item_count, recovering_home_planet, alien_index;
    long n, reb, current_pop;
    struct nampla_data *alien_home_nampla;

    /* Get the ship. */
    if (!get_ship()) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid ship name in UNLOAD command.\n");
        return;
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship is still under construction.\n");
        return;
    }

    if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship jumped during combat and is still in transit.\n");
        return;
    }

    /* Find which planet the ship is at. */
    found = FALSE;
    nampla = nampla_base - 1;
    for (i = 0; i < species->num_namplas; i++) {
        ++nampla;
        if (ship->x != nampla->x) { continue; }
        if (ship->y != nampla->y) { continue; }
        if (ship->z != nampla->z) { continue; }
        if (ship->pn != nampla->pn) { continue; }
        found = TRUE;
        break;
    }

    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Ship is not at a named planet.\n");
        return;
    }

    /* Make sure this is not someone else's populated homeworld. */
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++) {
        if (species_number == alien_index + 1) { continue; }
        if (!data_in_memory[alien_index]) { continue; }

        alien_home_nampla = namp_data[alien_index];

        if (alien_home_nampla->x != nampla->x) { continue; }
        if (alien_home_nampla->y != nampla->y) { continue; }
        if (alien_home_nampla->z != nampla->z) { continue; }
        if (alien_home_nampla->pn != nampla->pn) { continue; }
        if ((alien_home_nampla->status & POPULATED) == 0) { continue; }

        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! You may not colonize someone else's populated home planet!\n");

        return;
    }

    /* Make sure it's not a healthy home planet. */
    recovering_home_planet = FALSE;
    if (nampla->status & HOME_PLANET) {
        n = nampla->mi_base + nampla->ma_base + nampla->IUs_to_install +
            nampla->AUs_to_install;
        reb = species->hp_original_base - n;

        if (reb > 0) {
            recovering_home_planet = TRUE;    /* HP was bombed. */
        } else {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Installation not allowed on a healthy home planet!\n");
            return;
        }
    }

    /* Transfer the items from the ship to the planet. */
    log_string("    ");

    item_count = ship->item_quantity[CU];
    nampla->item_quantity[CU] += item_count;
    log_int(item_count);
    log_char(' ');
    log_string(item_abbr[CU]);
    if (item_count != 1) { log_char('s'); }
    ship->item_quantity[CU] = 0;

    item_count = ship->item_quantity[IU];
    nampla->item_quantity[IU] += item_count;
    log_string(", ");
    log_int(item_count);
    log_char(' ');
    log_string(item_abbr[IU]);
    if (item_count != 1) { log_char('s'); }
    ship->item_quantity[IU] = 0;

    item_count = ship->item_quantity[AU];
    nampla->item_quantity[AU] += item_count;
    log_string(", and ");
    log_int(item_count);
    log_char(' ');
    log_string(item_abbr[AU]);
    if (item_count != 1) { log_char('s'); }
    ship->item_quantity[AU] = 0;

    log_string(" were transferred from ");
    log_string(ship_name(ship));
    log_string(" to PL ");
    log_string(nampla->name);
    log_string(". ");

    /* Do the installation. */
    item_count = nampla->item_quantity[CU];
    if (item_count > nampla->item_quantity[IU]) {
        item_count = nampla->item_quantity[IU];
    }
    if (recovering_home_planet) {
        if (item_count > reb) { item_count = reb; }
        reb -= item_count;
    }

    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[IU] -= item_count;
    nampla->IUs_to_install += item_count;
    current_pop += item_count;

    log_string("Installation of ");
    log_int(item_count);
    log_char(' ');
    log_string(item_abbr[IU]);
    if (item_count != 1) { log_char('s'); }

    item_count = nampla->item_quantity[CU];
    if (item_count > nampla->item_quantity[AU]) {
        item_count = nampla->item_quantity[AU];
    }
    if (recovering_home_planet) {
        if (item_count > reb) { item_count = reb; }
        reb -= item_count;
    }

    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[AU] -= item_count;
    nampla->AUs_to_install += item_count;

    log_string(" and ");
    log_int(item_count);
    log_char(' ');
    log_string(item_abbr[AU]);
    if (item_count != 1) { log_char('s'); }
    log_string(" began on the planet.\n");

    check_population(nampla);
}


void do_UPGRADE_command(void) {
    int age_reduction, value_specified;
    char *original_line_pointer;
    long amount_to_spend, original_cost, max_funds_available;

    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Missing PRODUCTION order!\n");
        return;
    }

    /* Get the ship to be upgraded. */
    original_line_pointer = input_line_pointer;
    if (!get_ship()) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        if (!get_ship()) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Ship to be upgraded does not exist.\n");
            return;
        }
    }

    /* Make sure it didn't just jump. */
    if (ship->just_jumped) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship just jumped and is still in transit.\n");
        return;
    }

    /* Make sure it's in the same sector as the producing planet. */
    if (ship->x != nampla->x || ship->y != nampla->y || ship->z != nampla->z) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Item to be upgraded is not in the same sector as the production planet.\n");
        return;
    }

    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Item to be upgraded is still under construction.\n");
        return;
    }

    if (ship->age < 1) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Ship or starbase is too new to upgrade.\n");
        return;
    }

    /* Calculate the original cost of the ship. */
    if (ship->class == TR || ship->type == STARBASE) {
        original_cost = ship_cost[ship->class] * ship->tonnage;
    } else {
        original_cost = ship_cost[ship->class];
    }

    if (ship->type == SUB_LIGHT) {
        original_cost = (3 * original_cost) / 4;
    }

    /* Get amount to be spent. */
    if (value_specified = get_value()) {
        if (value == 0) {
            amount_to_spend = balance;
        } else {
            amount_to_spend = value;
        }

        age_reduction = (40 * amount_to_spend) / original_cost;
    } else {
        age_reduction = ship->age;
    }

    try_again:

    if (age_reduction < 1) {
        if (value == 0) { return; }
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! Amount specified is not enough to do an upgrade.\n");
        return;
    }

    if (age_reduction > ship->age) { age_reduction = ship->age; }

    /* Check if sufficient funds are available. */
    amount_to_spend = ((age_reduction * original_cost) + 39) / 40;
    if (check_bounced(amount_to_spend)) {
        max_funds_available = species->econ_units;
        if (max_funds_available > EU_spending_limit) {
            max_funds_available = EU_spending_limit;
        }
        max_funds_available += balance;

        if (max_funds_available > 0) {
            if (value_specified) {
                fprintf(log_file, "! WARNING: %s", input_line);
                fprintf(log_file, "! Insufficient funds. Substituting %ld for %ld.\n", max_funds_available, value);
            }
            amount_to_spend = max_funds_available;
            age_reduction = (40 * amount_to_spend) / original_cost;
            goto try_again;
        }

        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Insufficient funds to execute order.\n");
        return;
    }

    /* Log what was upgraded. */
    log_string("    ");
    log_string(ship_name(ship));
    log_string(" was upgraded from age ");
    log_int((int) ship->age);
    log_string(" to age ");
    ship->age -= age_reduction;
    log_int((int) ship->age);
    log_string(" at a cost of ");
    log_long(amount_to_spend);
    log_string(".\n");
}


void do_VISITED_command(void) {
    /* Get x y z coordinates. */
    int found = get_location();
    if (!found || nampla != NULL) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! Invalid coordinates in VISITED command.\n");
        return;
    }

    found = star_visited(x, y, z);

    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", input_line);
        fprintf(log_file, "!!! There is no star system at these coordinates.\n");
        return;
    }

    /* Log result. */
    log_string("    The star system at ");
    log_int(x);
    log_char(' ');
    log_int(y);
    log_char(' ');
    log_int(z);
    log_string(" was marked as visited.\n");
}


void do_WORMHOLE_command(void) {
    int i, status;
    struct star_data *star;

    /* Get ship making the jump. */
    char *original_line_pointer = input_line_pointer;
    int found = get_ship();
    if (!found) {
        /* Check for missing comma or tab after ship name. */
        input_line_pointer = original_line_pointer;
        fix_separator();
        found = get_ship();
        if (!found) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", original_line);
            fprintf(log_file, "!!! Invalid ship name in WORMHOLE command.\n");
            return;
        }
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship(ship)) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! This ship is salvage of a disbanded colony!\n");
        return;
    }

    /* Make sure ship can jump. */
    if (ship->status == UNDER_CONSTRUCTION) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s is still under construction!\n", ship_name(ship));
        return;
    }

    /* Check if JUMP, MOVE, or WORMHOLE was already done for this ship. */
    if (ship->just_jumped) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! %s already jumped or moved this turn!\n", ship_name(ship));
        return;
    }

    /* Find star. */
    star = star_base;
    found = FALSE;
    for (i = 0; i < num_stars; i++) {
        if (star->x == ship->x && star->y == ship->y && star->z == ship->z) {
            found = star->worm_here;
            break;
        }
        ++star;
    }

    if (!found) {
        fprintf(log_file, "!!! Order ignored:\n");
        fprintf(log_file, "!!! %s", original_line);
        fprintf(log_file, "!!! There is no wormhole at ship's location!\n");
        return;
    }

    /* Get the destination planet, if any. */
    get_location();
    if (nampla != NULL) {
        if (nampla->x != star->worm_x || nampla->y != star->worm_y || nampla->z != star->worm_z) {
            fprintf(log_file, "!!! WARNING - Destination planet is not at other end of wormhole!\n");
            nampla = NULL;
        }
    }

    /* Do the jump. */
    log_string("    ");
    log_string(ship_name(ship));
    log_string(" will jump via natural wormhole at ");
    log_int(ship->x);
    log_char(' ');
    log_int(ship->y);
    log_char(' ');
    log_int(ship->z);
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;

    if (nampla != NULL) {
        log_string(" to PL ");
        log_string(nampla->name);
        ship->pn = nampla->pn;
        ship->status = IN_ORBIT;
    }
    log_string(".\n");
    ship->x = star->worm_x;
    ship->y = star->worm_y;
    ship->z = star->worm_z;
    ship->just_jumped = 99;    /* 99 indicates that a wormhole was used. */

    if (!first_pass) { star_visited(ship->x, ship->y, ship->z); }
}
