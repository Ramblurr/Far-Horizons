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

#ifndef FAR_HORIZONS_COMMANDVARS_H
#define FAR_HORIZONS_COMMANDVARS_H

#include <stdio.h>

// globals. ugh.

extern int abbr_index;
extern int abbr_type;
extern struct ship_data *alien_portal;
extern char command_abbr[NUM_COMMANDS][4];
extern int end_of_file;
extern int g_spec_number;
extern char g_spec_name[32];
extern char input_abbr[256];
extern FILE *input_file;
extern char input_line[256];
extern char *input_line_pointer;
extern int just_opened_file;
extern char original_line[256];
extern char original_name[32];
extern struct species_data *other_species;
extern int other_species_number;
extern int sub_light;
extern char tech_abbr[6][4];
extern int tonnage;
extern long value;

#endif //FAR_HORIZONS_COMMANDVARS_H
