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
#include "engine.h"
#include "log.h"
#include "logvars.h"

static int log_indentation = 0;
static char log_line[128];
static int log_position = 0;
static int log_start_of_line = TRUE;

/* The following routines will post an item to standard output and to an externally defined log file and summary file. */

void log_char(char c) {
    int i, temp_position;
    char temp_char;

    if (logging_disabled) {
        return;
    }

    /* Check if current line is getting too long. */
    if ((c == ' ' || c == '\n') && log_position > 77) {
        /* Find closest preceeding space. */
        temp_position = log_position - 1;
        while (log_line[temp_position] != ' ') {
            --temp_position;
        }
        /* Write front of line to files. */
        temp_char = log_line[temp_position + 1];
        log_line[temp_position] = '\n';
        log_line[temp_position + 1] = '\0';
        if (log_to_file) {
            fputs(log_line, log_file);
        }
        if (log_stdout) {
            fputs(log_line, stdout);
        }
        if (log_summary) {
            fputs(log_line, summary_file);
        }
        log_line[temp_position + 1] = temp_char;
        /* Copy overflow word to beginning of next line. */
        log_line[log_position] = '\0';
        log_position = log_indentation + 2;
        for (i = 0; i < log_position; i++) {
            log_line[i] = ' ';
        }
        strcpy(&log_line[log_position], &log_line[temp_position + 1]);
        log_position = strlen(log_line);
        if (c == ' ') {
            log_line[log_position++] = ' ';
            return;
        }
    }

    /* Check if line is being manually terminated. */
    if (c == '\n') {
        /* Write current line to output. */
        log_line[log_position] = '\n';
        log_line[log_position + 1] = '\0';
        if (log_to_file) {
            fputs(log_line, log_file);
        }
        if (log_stdout) {
            fputs(log_line, stdout);
        }
        if (log_summary) {
            fputs(log_line, summary_file);
        }
        /* Set up for next line. */
        log_position = 0;
        log_indentation = 0;
        log_start_of_line = TRUE;
        return;
    }

    /* Save this character. */
    log_line[log_position] = c;
    ++log_position;

    if (log_start_of_line && c == ' ') {
        /* Determine number of indenting spaces for current line. */
        ++log_indentation;
    } else {
        log_start_of_line = FALSE;
    }
}

void log_int(int value) {
    char string[16];
    if (logging_disabled) {
        return;
    }
    sprintf(string, "%d\0", value);
    log_string(string);
}

void log_long(long value) {
    char string[16];
    if (logging_disabled) {
        return;
    }
    sprintf(string, "%ld\0", value);
    log_string(string);
}

void log_message(char *message_filename) {
    char message_line[256];
    /* Open message file. */
    FILE *message_file = fopen(message_filename, "r");
    if (message_file == NULL) {
        fprintf(stderr, "\n\tWARNING! utils.c: cannot open message file '%s'!\n\n", message_filename);
        return;
    }
    /* Copy message to log file. */
    while (fgets(message_line, 256, message_file) != NULL) {
        fputs(message_line, log_file);
    }
    fclose(message_file);
}


void log_string(char *string) {
    int i, length;
    if (logging_disabled) {
        return;
    }

    length = strlen(string);
    for (i = 0; i < length; i++) {
        log_char(string[i]);
    }
}


void print_header(void) {
    log_string("\nOther events:\n");
    header_printed = TRUE;
}


