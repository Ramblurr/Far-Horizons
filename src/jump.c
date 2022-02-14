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
#include "engine.h"
#include "enginevars.h"
#include "species.h"
#include "command.h"
#include "commandvars.h"
#include "do.h"
#include "jump.h"
#include "logvars.h"

void do_jump_orders(void) {
    int i, command;

    if (first_pass) {
        printf("\nStart of jump orders for species #%d, SP %s...\n", species_number, species->name);
    }

    while (TRUE) {
        command = get_command();

        if (command == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Unknown or missing command.\n");
            continue;
        }

        if (end_of_file || command == END) {
            if (first_pass) {
                printf("End of jump orders for species #%d, SP %s.\n", species_number, species->name);
            }
            if (first_pass) { gamemaster_abort_option(); }
            break;            /* END for this species. */
        }

        switch (command) {
            case JUMP:
                do_JUMP_command(FALSE, FALSE);
                break;
            case MOVE:
                do_MOVE_command();
                break;
            case PJUMP:
                do_JUMP_command(FALSE, TRUE);
                break;
            case VISITED:
                do_VISITED_command();
                break;
            case WORMHOLE:
                do_WORMHOLE_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid jump command.\n");
        }
    }
}
