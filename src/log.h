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

#ifndef FAR_HORIZONS_LOG_H
#define FAR_HORIZONS_LOG_H

#include <stdio.h>

void log_char(char c);

void log_int(int value);

void log_long(long value);

void log_message(char *message_filename);

void log_string(char *string);

// globals. ugh.

extern FILE *log_file;
extern int log_stdout;
extern int log_summary;
extern int log_to_file;
extern int logging_disabled;
extern FILE *summary_file;

#endif //FAR_HORIZONS_LOG_H
