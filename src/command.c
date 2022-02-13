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
#include "ship.h"
#include "item.h"
#include "command.h"

int abbr_type;
int abbr_index;
char command_abbr[NUM_COMMANDS][4] = {
        "   ", "ALL", "AMB", "ATT", "AUT", "BAS", "BAT", "BUI", "CON",
        "DEE", "DES", "DEV", "DIS", "END", "ENE", "ENG", "EST", "HAV",
        "HID", "HIJ", "IBU", "ICO", "INS", "INT", "JUM", "LAN", "MES",
        "MOV", "NAM", "NEU", "ORB", "PJU", "PRO", "REC", "REP", "RES",
        "SCA", "SEN", "SHI", "STA", "SUM", "SUR", "TAR", "TEA", "TEC",
        "TEL", "TER", "TRA", "UNL", "UPG", "VIS", "WIT", "WOR", "ZZZ"
};
char command_name[NUM_COMMANDS][16] = {
        "Undefined", "Ally", "Ambush", "Attack", "Auto", "Base",
        "Battle", "Build", "Continue", "Deep", "Destroy", "Develop",
        "Disband", "End", "Enemy", "Engage", "Estimate", "Haven",
        "Hide", "Hijack", "Ibuild", "Icontinue", "Install", "Intercept",
        "Jump", "Land", "Message", "Move", "Name", "Neutral", "Orbit",
        "Pjump", "Production", "Recycle", "Repair", "Research", "Scan",
        "Send", "Shipyard", "Start", "Summary", "Surrender", "Target",
        "Teach", "Tech", "Telescope", "Terraform", "Transfer", "Unload",
        "Upgrade", "Visited", "Withdraw", "Wormhole", "ZZZ"
};
int end_of_file = FALSE;
char input_abbr[256];
FILE *input_file;
char input_line[256];
char *input_line_pointer;
int just_opened_file;
char original_line[256];
char original_name[32];
int sub_light;
char tech_abbr[6][4] = {
        "MI",
        "MA",
        "ML",
        "GV",
        "LS",
        "BI"
};
char tech_name[6][16] = {
        "Mining",
        "Manufacturing",
        "Military",
        "Gravitics",
        "Life Support",
        "Biology"
};
int tonnage;
long value;

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



