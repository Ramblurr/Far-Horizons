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

#include "engine.h"
#include "command.h"
#include "commandvars.h"

int abbr_type;

int abbr_index;

struct ship_data *alien_portal;

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

int g_spec_number;

char g_spec_name[32];

char input_abbr[256];

FILE *input_file;

char input_line[256];

char *input_line_pointer;

int just_opened_file;

char original_line[256];

char original_name[32];

struct species_data *other_species;

int other_species_number;

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

