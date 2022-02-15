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
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "star.h"
#include "stario.h"
#include "starvars.h"
#include "planet.h"
#include "planetio.h"
#include "species.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "nampla.h"
#include "namplavars.h"
#include "ship.h"
#include "shipvars.h"
#include "locationvars.h"
#include "transaction.h"
#include "transactionio.h"
#include "log.h"
#include "logvars.h"
#include "command.h"
#include "commandvars.h"
#include "do.h"
#include "jumpvars.h"

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
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
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
            fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
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
        fprintf(stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
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
        input_line_pointer = fgets(input_line, 256, input_file);
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
        scan(x, y, z);
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
    if (item_count == 0) { item_count = num_available; }
    if (item_count == 0) { return; }
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

    if (first_pass) { return; }

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
                fprintf(log_file, "! Ship does not have %d units. Substituting %d for %d!\n", item_count, num_available,
                        item_count);
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
                fprintf(log_file, "! Planet does not have %d units. Substituting %d for %d!\n", item_count,
                        num_available, item_count);
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
            fprintf(log_file, " Changed %d to 0.\n", original_count);
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
