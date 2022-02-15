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
#include "engine.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "stario.h"
#include "starvars.h"
#include "speciesio.h"
#include "speciesvars.h"
#include "nampla.h"
#include "namplavars.h"
#include "ship.h"
#include "shipvars.h"
#include "item.h"
#include "locationvars.h"
#include "command.h"
#include "commandvars.h"
#include "jumpvars.h"


/* The following routine will check that the next argument in the current command line is followed by a comma or tab.
 * If not present, it will try to insert a comma in the proper position.
 * This routine should be called only AFTER an error has been detected. */
void fix_separator(void) {
    int n, first_class, fix_made, num_commas;
    char c, *temp_ptr, *temp2_ptr, *first_comma;

    skip_whitespace();
    if (isdigit(*input_line_pointer)) {
        /* Nothing can be done. */
        return;
    }
    if (strchr(input_line_pointer, ' ') == NULL) {
        /* Ditto. */
        return;
    }
    fix_made = FALSE;

    /* Look for a ship, planet, or species abbreviation after the first one.
     * If it is preceeded by a space, convert the space to a comma. */
    temp_ptr = input_line_pointer;
    first_class = get_class_abbr();    /* Skip first one but remember what it was. */
    while (1) {
        skip_whitespace();
        temp2_ptr = input_line_pointer - 1;
        if (*input_line_pointer == '\n') {
            break;
        }
        if (*input_line_pointer == ';') {
            break;
        }
        /* The following is to prevent an infinite loop. */
        if (!isalnum(*input_line_pointer)) {
            ++input_line_pointer;
            continue;
        }

        n = get_class_abbr();
        if (n == SHIP_CLASS || n == PLANET_ID || n == SPECIES_ID) {
            /* Convert space preceeding abbreviation to a comma. */
            if (*temp2_ptr == ' ') {
                *temp2_ptr = ',';
                fix_made = TRUE;
            }
        }
    }
    input_line_pointer = temp_ptr;

    if (fix_made) {
        return;
    }

    /* Look for a space followed by a digit.
     * If found, convert the space to a comma.
     * If exactly two or four commas are added, re-convert the first one back to a space;
     * e.g. Jump TR1 Seeker,7,99,99,99 or Build TR1 Seeker,7,50. */
    num_commas = 0;
    while (1) {
        c = *temp_ptr++;
        if (c == '\n') {
            break;
        }
        if (c == ';') {
            break;
        }
        if (c != ' ') {
            continue;
        }
        if (isdigit(*temp_ptr)) {
            --temp_ptr;        /* Convert space to a comma. */
            *temp_ptr = ',';
            if (num_commas++ == 0) {
                first_comma = temp_ptr;
            }
            ++temp_ptr;
            fix_made = TRUE;
        }
    }

    if (fix_made) {
        if (num_commas == 2 || num_commas == 4) {
            *first_comma = ' ';
        }
        return;
    }

    /* Now's the time for wild guesses. */
    temp_ptr = input_line_pointer;

    /* If first word is a valid abbreviation, put a comma after the second word. */
    if (first_class == SHIP_CLASS || first_class == PLANET_ID || first_class == SPECIES_ID) {
        temp_ptr = strchr(temp_ptr, ' ') + 1;
        temp_ptr = strchr(temp_ptr, ' ');
        if (temp_ptr != NULL) {
            *temp_ptr = ',';
        }
        return;
    }

    /* First word is not a valid abbreviation.  Put a comma after it. */
    temp_ptr = strchr(temp_ptr, ' ');
    if (temp_ptr != NULL) {
        *temp_ptr = ',';
    }
}


/* get_class_abbr will return 0 if the item found was not of the appropriate type, and 1 or greater if an item of the correct type was found. */
/* Get a class abbreviation and return TECH_ID, ITEM_CLASS, SHIP_CLASS,
   PLANET_ID, SPECIES_ID or ALLIANCE_ID as appropriate, or UNKNOWN if it
   cannot be identified. Also, set "abbr_type" to this value. If it is
   TECH_ID, ITEM_CLASS or SHIP_CLASS, "abbr_index" will contain the
   abbreviation index. If it is a ship, "tonnage" will contain tonnage/10,000,
   and "sub_light" will be TRUE or FALSE. (Tonnage value returned is based
   ONLY on abbreviation.) */
int get_class_abbr(void) {
    // item.c
    extern char item_abbr[MAX_ITEMS][4];
    // ship.c
    extern char ship_abbr[NUM_SHIP_CLASSES][4];
    extern short ship_tonnage[NUM_SHIP_CLASSES];

    int i;
    char *digit_start;

    skip_whitespace();

    abbr_type = UNKNOWN;

    if (!isalnum(*input_line_pointer)) {
        return UNKNOWN;
    }
    input_abbr[0] = toupper(*input_line_pointer);
    ++input_line_pointer;

    if (!isalnum(*input_line_pointer)) {
        return UNKNOWN;
    }
    input_abbr[1] = toupper(*input_line_pointer);
    ++input_line_pointer;

    input_abbr[2] = '\0';

    /* Check for IDs that are followed by one or more digits or letters. */
    i = 2;
    digit_start = input_line_pointer;
    while (isalnum(*input_line_pointer)) {
        input_abbr[i++] = *input_line_pointer++;
        input_abbr[i] = '\0';
    }

    /* Check tech ID. */
    for (i = 0; i < 6; i++) {
        if (strcmp(input_abbr, tech_abbr[i]) == 0) {
            abbr_index = i;
            abbr_type = TECH_ID;
            return abbr_type;
        }
    }

    /* Check item abbreviations. */
    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(input_abbr, item_abbr[i]) == 0) {
            abbr_index = i;
            abbr_type = ITEM_CLASS;
            return abbr_type;
        }
    }

    /* Check ship abbreviations. */
    for (i = 0; i < NUM_SHIP_CLASSES; i++) {
        if (strncmp(input_abbr, ship_abbr[i], 2) == 0) {
            input_line_pointer = digit_start;
            abbr_index = i;
            tonnage = ship_tonnage[i];
            if (i == TR) {
                tonnage = 0;
                while (isdigit(*input_line_pointer)) {
                    tonnage = (10 * tonnage) + (*input_line_pointer - '0');
                    ++input_line_pointer;
                }
            }
            if (toupper(*input_line_pointer) == 'S') {
                sub_light = TRUE;
                ++input_line_pointer;
            } else {
                sub_light = FALSE;
            }
            if (isalnum(*input_line_pointer)) {
                /* Garbage. */
                break;
            }
            abbr_type = SHIP_CLASS;
            return abbr_type;
        }
    }

    /* Check for planet name. */
    if (strcmp(input_abbr, "PL") == 0) {
        abbr_type = PLANET_ID;
        return abbr_type;
    }

    /* Check for species name. */
    if (strcmp(input_abbr, "SP") == 0) {
        abbr_type = SPECIES_ID;
        return abbr_type;
    }

    abbr_type = UNKNOWN;
    return abbr_type;
}

/* Get a class abbreviation and return TECH_ID, ITEM_CLASS, SHIP_CLASS,
   PLANET_ID, SPECIES_ID or ALLIANCE_ID as appropriate, or UNKNOWN if it
   cannot be identified. Also, set "abbr_type" to this value. If it is
   TECH_ID, ITEM_CLASS or SHIP_CLASS, "abbr_index" will contain the
   abbreviation index. If it is a ship, "tonnage" will contain tonnage/10,000,
   and "sub_light" will be TRUE or FALSE. (Tonnage value returned is based
   ONLY on abbreviation.) */
int get_class_abbr_from_arg(char *arg) {
    // item.c
    extern char item_abbr[MAX_ITEMS][4];
    // ship.c
    extern char ship_abbr[NUM_SHIP_CLASSES][4];
    extern short ship_tonnage[NUM_SHIP_CLASSES];

    int i;
    char *digit_start;

    abbr_type = UNKNOWN;

    if (!isalnum(*arg)) {
        return UNKNOWN;
    }
    input_abbr[0] = toupper(*arg);
    ++arg;

    if (!isalnum(*arg)) {
        return UNKNOWN;
    }
    input_abbr[1] = toupper(*arg);
    ++arg;

    input_abbr[2] = '\0';

    /* Check for IDs that are followed by one or more digits or letters. */
    i = 2;
    digit_start = arg;
    while (isalnum(*arg)) {
        input_abbr[i++] = *arg++;
        input_abbr[i] = '\0';
    }

    /* Check tech ID. */
    for (i = 0; i < 6; i++) {
        if (strcmp(input_abbr, tech_abbr[i]) == 0) {
            abbr_index = i;
            abbr_type = TECH_ID;
            return abbr_type;
        }
    }

    /* Check item abbreviations. */
    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(input_abbr, item_abbr[i]) == 0) {
            abbr_index = i;
            abbr_type = ITEM_CLASS;
            return abbr_type;
        }
    }

    /* Check ship abbreviations. */
    for (i = 0; i < NUM_SHIP_CLASSES; i++) {
        if (strncmp(input_abbr, ship_abbr[i], 2) == 0) {
            arg = digit_start;
            abbr_index = i;
            tonnage = ship_tonnage[i];
            if (i == TR) {
                tonnage = 0;
                while (isdigit(*arg)) {
                    tonnage = (10 * tonnage) + (*arg - '0');
                    ++arg;
                }
            }
            if (toupper(*arg) == 'S') {
                sub_light = TRUE;
                ++arg;
            } else {
                sub_light = FALSE;
            }
            if (isalnum(*arg)) {
                /* Garbage. */
                break;
            }
            abbr_type = SHIP_CLASS;
            return abbr_type;
        }
    }

    /* Check for planet name. */
    if (strcmp(input_abbr, "PL") == 0) {
        abbr_type = PLANET_ID;
        return abbr_type;
    }

    /* Check for species name. */
    if (strcmp(input_abbr, "SP") == 0) {
        abbr_type = SPECIES_ID;
        return abbr_type;
    }

    abbr_type = UNKNOWN;
    return abbr_type;
}


/* get_command will return 0 if the item found was not of the appropriate type, and 1 or greater if an item of the correct type was found. */
/* Get a command and return its index. */
int get_command(void) {
    int i;
    int cmd_n;
    char c;
    char cmd_s[4];

    skip_junk();
    if (end_of_file) {
        return -1;
    }

    c = *input_line_pointer;
    /* Get first three characters of command word. */
    for (i = 0; i < 3; i++) {
        if (!isalpha(c)) {
            return 0;
        }
        cmd_s[i] = toupper(c);
        ++input_line_pointer;
        c = *input_line_pointer;
    }
    cmd_s[3] = '\0';

    /* Skip everything after third character of command word. */
    while (1) {
        switch (c) {
            case '\t':
            case '\n':
            case ' ':
            case ',':
            case ';':
                goto find_cmd;

            default:
                ++input_line_pointer;
                c = *input_line_pointer;
        }
    }

    find_cmd:

    /* Find corresponding string in list. */
    cmd_n = UNKNOWN;
    for (i = 1; i < NUM_COMMANDS; i++) {
        if (strcmp(cmd_s, command_abbr[i]) == 0) {
            cmd_n = i;
            break;
        }
    }

    return cmd_n;
}


int get_jump_portal(void) {
    int i, j, k, found, array_index, bit_number;
    long bit_mask;
    char start_x, start_y, start_z, upper_ship_name[32], *original_line_pointer;
    struct species_data *original_species;
    struct ship_data *temp_ship, *portal, *original_ship, *original_ship_base;

    /* See if specified starbase is owned by the current species. */
    original_line_pointer = input_line_pointer;
    temp_ship = ship;
    found = get_ship();
    portal = ship;
    ship = temp_ship;
    using_alien_portal = FALSE;

    if (found) {
        if (portal->type != STARBASE) { return FALSE; }
        jump_portal_age = portal->age;
        jump_portal_gv = species->tech_level[GV];
        jump_portal_units = portal->item_quantity[JP];
        strcpy(jump_portal_name, ship_name(portal));
        return TRUE;
    }

    start_x = ship->x;
    start_y = ship->y;
    start_z = ship->z;

    if (abbr_type != SHIP_CLASS) { goto check_for_bad_spelling; }
    if (abbr_index != BA) { goto check_for_bad_spelling; }

    /* It IS the name of a starbase.  See if another species has given permission to use their starbase. */
    for (other_species_number = 1; other_species_number <= galaxy.num_species; other_species_number++) {
        if (!data_in_memory[other_species_number - 1]) { continue; }
        if (other_species_number == species_number) { continue; }

        other_species = &spec_data[other_species_number - 1];

        found = FALSE;

        /* Check if other species has declared this species as an ally. */
        array_index = (species_number - 1) / 32;
        bit_number = (species_number - 1) % 32;
        bit_mask = 1 << bit_number;
        if ((other_species->ally[array_index] & bit_mask) == 0) { continue; }

        /* See if other species has a starbase with the specified name at the start location. */
        alien_portal = ship_data[other_species_number - 1] - 1;
        for (j = 0; j < other_species->num_ships; j++) {
            ++alien_portal;
            if (alien_portal->type != STARBASE) { continue; }
            if (alien_portal->x != start_x) { continue; }
            if (alien_portal->y != start_y) { continue; }
            if (alien_portal->z != start_z) { continue; }
            if (alien_portal->pn == 99) { continue; }
            /* Make upper case copy of ship name. */
            for (k = 0; k < 32; k++) {
                upper_ship_name[k] = toupper(alien_portal->name[k]);
            }
            /* Compare names. */
            if (strcmp(upper_ship_name, upper_name) == 0) {
                found = TRUE;
                break;
            }
        }

        if (found) {
            jump_portal_units = alien_portal->item_quantity[JP];
            jump_portal_age = alien_portal->age;
            jump_portal_gv = other_species->tech_level[GV];
            strcpy(jump_portal_name, ship_name(alien_portal));
            strcat(jump_portal_name, " owned by SP ");
            strcat(jump_portal_name, other_species->name);
            using_alien_portal = TRUE;
            break;
        }
    }

    if (found) { return TRUE; }

    check_for_bad_spelling:

    /* Try again, but allow spelling errors. */
    original_ship = ship;
    original_ship_base = ship_base;
    original_species = species;

    for (other_species_number = 1; other_species_number <= galaxy.num_species; other_species_number++) {
        if (!data_in_memory[other_species_number - 1]) { continue; }
        if (other_species_number == species_number) { continue; }
        species = &spec_data[other_species_number - 1];

        /* Check if other species has declared this species as an ally. */
        array_index = (species_number - 1) / 32;
        bit_number = (species_number - 1) % 32;
        bit_mask = 1 << bit_number;
        if ((species->ally[array_index] & bit_mask) == 0) { continue; }
        input_line_pointer = original_line_pointer;
        ship_base = ship_data[other_species_number - 1];
        found = get_ship();
        if (found) {
            found = FALSE;
            if (ship->type != STARBASE) { continue; }
            if (ship->x != start_x) { continue; }
            if (ship->y != start_y) { continue; }
            if (ship->z != start_z) { continue; }
            if (ship->pn == 99) { continue; }
            found = TRUE;
            break;
        }
    }

    if (found) {
        jump_portal_units = ship->item_quantity[JP];
        jump_portal_age = ship->age;
        jump_portal_gv = species->tech_level[GV];
        strcpy(jump_portal_name, ship_name(ship));
        strcat(jump_portal_name, " owned by SP ");
        strcat(jump_portal_name, species->name);
        using_alien_portal = TRUE;
    }

    species = original_species;
    ship = original_ship;
    ship_base = original_ship_base;

    return found;
}


/* This routine will assign values to global variables x, y, z, pn, star and nampla.
 * If the location is not a named planet, then nampla will be set to NULL.
 * If planet is not specified, pn will be set to zero.
 * If location is valid, TRUE will be returned, otherwise FALSE will be returned. */
int get_location(void) {
    int i, n, found, temp_nampla_index, first_try, name_length;
    int best_score, next_best_score, best_nampla_index;
    int minimum_score;
    char upper_nampla_name[32], *temp1_ptr, *temp2_ptr;
    struct nampla_data *temp_nampla;

    /* Check first if x, y, z are specified. */
    nampla = NULL;
    skip_whitespace();

    if (get_value() == 0) {
        goto get_planet;
    }
    x = value;

    if (get_value() == 0) {
        return FALSE;
    }
    y = value;

    if (get_value() == 0) {
        return FALSE;
    }
    z = value;

    if (get_value() == 0) {
        pn = 0;
    } else {
        pn = value;
    }

    if (pn == 0) {
        return TRUE;
    }

    /* Get star. Check if planet exists. */
    found = FALSE;
    star = star_base - 1;
    for (i = 0; i < num_stars; i++) {
        star++;
        if (star->x != x) {
            continue;
        }
        if (star->y != y) {
            continue;
        }
        if (star->z != z) {
            continue;
        }
        if (pn > star->num_planets) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    return FALSE;

    get_planet:

    /* Save pointers in case of error. */
    temp1_ptr = input_line_pointer;

    get_class_abbr();

    temp2_ptr = input_line_pointer;

    first_try = TRUE;

    again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != PLANET_ID && !first_try) {
        /* Assume abbreviation was accidentally omitted. */
        input_line_pointer = temp1_ptr;
    }

    /* Get planet name. */
    get_name();

    /* Search all temp_namplas for name. */
    temp_nampla = nampla_base - 1;
    for (temp_nampla_index = 0; temp_nampla_index < species->num_namplas; temp_nampla_index++) {
        temp_nampla++;
        if (temp_nampla->pn == 99) {
            continue;
        }
        /* Make upper case copy of temp_nampla name. */
        for (i = 0; i < 32; i++) {
            upper_nampla_name[i] = toupper(temp_nampla->name[i]);
        }
        /* Compare names. */
        if (strcmp(upper_nampla_name, upper_name) == 0) { goto done; }
    }

    if (first_try) {
        first_try = FALSE;
        goto again;
    }

    /* Possibly a spelling error.  Find the best match that is approximately the same. */
    first_try = TRUE;

    yet_again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != PLANET_ID && !first_try) {
        /* Assume abbreviation was accidentally omitted. */
        input_line_pointer = temp1_ptr;
    }

    /* Get planet name. */
    get_name();

    best_nampla_index = 0;
    best_score = -9999;
    next_best_score = -9999;
    for (temp_nampla_index = 0; temp_nampla_index < species->num_namplas; temp_nampla_index++) {
        temp_nampla = nampla_base + temp_nampla_index;
        if (temp_nampla->pn == 99) {
            continue;
        }
        /* Make upper case copy of temp_nampla name. */
        for (i = 0; i < 32; i++) {
            upper_nampla_name[i] = toupper(temp_nampla->name[i]);
        }
        /* Compare names. */
        n = agrep_score(upper_nampla_name, upper_name);
        if (n > best_score) {
            best_score = n;    /* Best match so far. */
            best_nampla_index = temp_nampla_index;
        } else if (n > next_best_score) {
            next_best_score = n;
        }
    }

    temp_nampla = nampla_base + best_nampla_index;
    name_length = strlen(temp_nampla->name);
    minimum_score = name_length - ((name_length / 7) + 1);

    if (best_score < minimum_score        /* Score too low. */
        || name_length < 5            /* No errors allowed. */
        || best_score == next_best_score)    /* Another name with equal score. */
    {
        if (first_try) {
            first_try = FALSE;
            goto yet_again;
        } else {
            return FALSE;
        }
    }

    done:

    abbr_type = PLANET_ID;

    x = temp_nampla->x;
    y = temp_nampla->y;
    z = temp_nampla->z;
    pn = temp_nampla->pn;
    nampla = temp_nampla;

    return TRUE;
}


/* get_name will return 0 if the item found was not of the appropriate type, and 1 or greater if an item of the correct type was found. */
/* Get a name and copy original version to "original_name" and upper case version to "upper_name". Return length of name. */
int get_name(void) {
    // engine.c
    extern char upper_name[32];

    int name_length = 0;

    skip_whitespace();
    while (TRUE) {
        char c = *input_line_pointer;
        if (c == ';') {
            break;
        }
        ++input_line_pointer;
        if (c == ',' || c == '\t' || c == '\n') {
            break;
        }
        if (name_length < 31) {
            original_name[name_length] = c;
            upper_name[name_length] = toupper(c);
            ++name_length;
        }
    }

    /* Remove any final spaces in name. */
    while (name_length > 0) {
        char c = original_name[name_length - 1];
        if (c != ' ') {
            break;
        }
        --name_length;
    }
    /* Terminate strings. */
    original_name[name_length] = '\0';
    upper_name[name_length] = '\0';
    return name_length;
}


/* This routine will get a species name and return TRUE if found and if it is valid.
 * It will also set global values "g_species_number" and "g_species_name".
 * The algorithm employed allows minor spelling errors, as well as accidental deletion of the SP abbreviation. */
int get_species_name(void) {
    int i, n, species_index, best_score, best_species_index, next_best_score, first_try, minimum_score, name_length;
    char sp_name[32], *temp1_ptr, *temp2_ptr;
    struct species_data *sp;

    g_spec_number = 0;
    /* Save pointers in case of error. */
    temp1_ptr = input_line_pointer;
    get_class_abbr();
    temp2_ptr = input_line_pointer;

    first_try = TRUE;

    again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != SPECIES_ID && !first_try) {
        /* Assume abbreviation was accidentally omitted. */
        input_line_pointer = temp1_ptr;
    }

    /* Get species name. */
    get_name();

    for (species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (!data_in_memory[species_index]) {
            continue;
        }
        sp = &spec_data[species_index];

        /* Copy name to g_spec_name and convert it to upper case. */
        for (i = 0; i < 31; i++) {
            g_spec_name[i] = sp->name[i];
            sp_name[i] = toupper(g_spec_name[i]);
        }
        if (strcmp(sp_name, upper_name) == 0) {
            g_spec_number = species_index + 1;
            abbr_type = SPECIES_ID;
            return TRUE;
        }
    }

    if (first_try) {
        first_try = FALSE;
        goto again;
    }

    /* Possibly a spelling error.  Find the best match that is approximately the same. */
    first_try = TRUE;

    yet_again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != SPECIES_ID && !first_try) {
        /* Assume abbreviation was accidentally omitted. */
        input_line_pointer = temp1_ptr;
    }

    /* Get species name. */
    get_name();

    best_score = -9999;
    next_best_score = -9999;
    for (species_index = 0; species_index < galaxy.num_species; species_index++) {
        if (!data_in_memory[species_index]) {
            continue;
        }
        sp = &spec_data[species_index];
        /* Convert name to upper case. */
        for (i = 0; i < 31; i++) {
            sp_name[i] = toupper(sp->name[i]);
        }
        n = agrep_score(sp_name, upper_name);
        if (n > best_score) {
            /* Best match so far. */
            best_score = n;
            best_species_index = species_index;
        } else if (n > next_best_score) {
            next_best_score = n;
        }
    }

    sp = &spec_data[best_species_index];
    name_length = strlen(sp->name);
    minimum_score = name_length - ((name_length / 7) + 1);

    if (best_score < minimum_score        /* Score too low. */
        || name_length < 5            /* No errors allowed. */
        || best_score == next_best_score)    /* Another name with equal score. */
    {
        if (first_try) {
            first_try = FALSE;
            goto yet_again;
        } else {
            return FALSE;
        }
    }

    /* Copy name to g_spec_name. */
    for (i = 0; i < 31; i++) {
        g_spec_name[i] = sp->name[i];
    }
    g_spec_number = best_species_index + 1;
    abbr_type = SPECIES_ID;
    return TRUE;
}


int get_transfer_point(void) {
    char *temp_ptr;
    /* Find out if it is a ship or a planet. First try for a correctly spelled ship name. */
    temp_ptr = input_line_pointer;
    correct_spelling_required = TRUE;
    if (get_ship()) {
        return TRUE;
    }
    /* Probably not a ship. See if it's a planet. */
    input_line_pointer = temp_ptr;
    if (get_location()) {
        if (nampla != NULL) {
            return TRUE;
        }
        return FALSE;
    }
    /* Now check for an incorrectly spelled ship name. */
    input_line_pointer = temp_ptr;
    if (get_ship()) {
        return TRUE;
    }
    return FALSE;
}


/* Read a long decimal and place its value in 'value'. */
int get_value(void) {
    int n;
    skip_whitespace();
    n = sscanf(input_line_pointer, "%ld", &value);
    if (n != 1) {
        /* Not a numeric value. */
        return 0;
    }
    /* Skip numeric string. */
    ++input_line_pointer;    /* Skip first sign or digit. */
    while (isdigit(*input_line_pointer)) {
        ++input_line_pointer;
    }
    return 1;
}


/* Skip white space and comments. */
void skip_junk(void) {
    again:

    /* Read next line. */
    input_line_pointer = fgets(input_line, 256, input_file);
    if (input_line_pointer == NULL) {
        end_of_file = TRUE;
        return;
    }

    if (just_opened_file) {
        /* Skip mail header, if any. */
        if (*input_line == '\n') {
            goto again;
        }
        just_opened_file = FALSE;
        if (strncmp(input_line, "From ", 5) == 0) {
            /* This is a mail header. */
            while (TRUE) {
                input_line_pointer = fgets(input_line, 256, input_file);
                if (input_line_pointer == NULL) {
                    end_of_file = TRUE;        /* Weird. */
                    return;
                }
                if (*input_line == '\n') {
                    /* End of header. */
                    break;
                }
            }
            goto again;
        }
    }

    strcpy(original_line, input_line);        /* Make a copy. */

    /* Skip white space and comments. */
    while (TRUE) {
        switch (*input_line_pointer) {
            case ';':            /* Semi-colon. */
            case '\n':            /* Newline. */
                goto again;
            case '\t':            /* Tab. */
            case ' ':            /* Space. */
            case ',':            /* Comma. */
                ++input_line_pointer;
                continue;
            default:
                return;
        }
    }
}


void skip_whitespace(void) {
    while (TRUE) {
        switch (*input_line_pointer) {
            case '\t':            /* Tab. */
            case ' ':            /* Space. */
            case ',':            /* Comma. */
                ++input_line_pointer;
                break;
            default:
                return;
        }
    }
}
