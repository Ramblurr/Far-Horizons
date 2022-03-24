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
#include <stdarg.h>
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
    if (!logging_disabled) {
        log_printf("%d", value);
    }
}


void log_long(long value) {
    if (!logging_disabled) {
        log_printf("%ld", value);
    }
}


void log_message(char *message_filename) {
    char message_line[256];
    /* Open message file. */
    FILE *message_file = fopen(message_filename, "r");
    if (message_file == NULL) {
        fprintf(stderr, "\n\tWARNING! log_message: cannot open message file '%s'!\n\n", message_filename);
        return;
    }
    /* Copy message to log file. */
    while (readln(message_line, 256, message_file) != NULL) {
        fputs(message_line, log_file);
    }
    fclose(message_file);
}


void log_printf(char *fmt, ...) {
    static char buffer[4096];
    if (!logging_disabled) {
        va_list arg_ptr;
        va_start(arg_ptr, fmt);
        vsnprintf(buffer, 4096, fmt, arg_ptr);
        va_end(arg_ptr);

        for (char *s = buffer; *s; s++) {
            log_char(*s);
        }
    }
}


void log_string(char *string) {
    if (!logging_disabled) {
        for (char *s = string; *s; s++) {
            log_char(*s);
        }
    }
}


void print_header(void) {
    if (!logging_disabled) {
        log_string("\nOther events:\n");
    }
    header_printed = TRUE;
}
