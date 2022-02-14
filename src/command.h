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

#ifndef FAR_HORIZONS_COMMAND_H
#define FAR_HORIZONS_COMMAND_H

/* Command codes. */
#define UNDEFINED    0
#define ALLY         1
#define AMBUSH       2
#define ATTACK       3
#define AUTO         4
#define BASE         5
#define BATTLE       6
#define BUILD        7
#define CONTINUE     8
#define DEEP         9
#define DESTROY      10
#define DEVELOP      11
#define DISBAND      12
#define END          13
#define ENEMY        14
#define ENGAGE       15
#define ESTIMATE     16
#define HAVEN        17
#define HIDE         18
#define HIJACK       19
#define IBUILD       20
#define ICONTINUE    21
#define INSTALL      22
#define INTERCEPT    23
#define JUMP         24
#define LAND         25
#define MESSAGE      26
#define MOVE         27
#define NAME         28
#define NEUTRAL      29
#define ORBIT        30
#define PJUMP        31
#define PRODUCTION   32
#define RECYCLE      33
#define REPAIR       34
#define RESEARCH     35
#define SCAN         36
#define SEND         37
#define SHIPYARD     38
#define START        39
#define SUMMARY      40
#define SURRENDER    41
#define TARGET       42
#define TEACH        43
#define TECH         44
#define TELESCOPE    45
#define TERRAFORM    46
#define TRANSFER     47
#define UNLOAD       48
#define UPGRADE      49
#define VISITED      50
#define WITHDRAW     51
#define WORMHOLE     52
#define ZZZ          53
#define NUM_COMMANDS ZZZ+1

/* Constants needed for parsing. */
#define UNKNOWN    0
#define TECH_ID    1
#define ITEM_CLASS 2
#define SHIP_CLASS 3
#define PLANET_ID  4
#define SPECIES_ID 5

void fix_separator(void);

int get_class_abbr(void);

int get_class_abbr_from_arg(char *arg);

int get_command(void);

int get_jump_portal(void);

int get_location(void);

int get_name(void);

int get_species_name(void);

int get_transfer_point(void);

int get_value(void);

void skip_junk(void);

void skip_whitespace(void);

#endif //FAR_HORIZONS_COMMAND_H
