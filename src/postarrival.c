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

#include "postarrival.h"
#include "command.h"
#include "engine.h"
#include "shipvars.h"
#include "enginevars.h"
#include "speciesvars.h"
#include "logvars.h"
#include "commandvars.h"
#include "do.h"
#include "log.h"


void do_postarrival_orders(void) {
    int i, command;

    if (first_pass) {
        printf("\nStart of post-arrival orders for species #%d, SP %s...\n", species_number, species->name);
    }

    /* For these commands, do not display age or landed/orbital status of ships. */
    truncate_name = TRUE;

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
                printf("End of post-arrival orders for species #%d, SP %s.\n", species_number, species->name);
            }
            if (first_pass) { gamemaster_abort_option(); }
            break;            /* END for this species. */
        }

        switch (command) {
            case ALLY:
                do_ALLY_command();
                break;
            case AUTO:
                species->auto_orders = TRUE;
                log_string("    An AUTO order was executed.\n");
                break;
            case DEEP:
                do_DEEP_command();
                break;
            case DESTROY:
                do_DESTROY_command();
                break;
            case ENEMY:
                do_ENEMY_command();
                break;
            case LAND:
                do_LAND_command();
                break;
            case MESSAGE:
                do_MESSAGE_command();
                break;
            case NAME:
                do_NAME_command();
                break;
            case NEUTRAL:
                do_NEUTRAL_command();
                break;
            case ORBIT:
                do_ORBIT_command();
                break;
            case REPAIR:
                do_REPAIR_command();
                break;
            case SCAN:
                do_SCAN_command();
                break;
            case SEND:
                do_SEND_command();
                break;
            case TEACH:
                do_TEACH_command();
                break;
            case TECH:
                // do_TECH_command();
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid post-arrival command.\n");
                break;
            case TRANSFER:
                do_TRANSFER_command();
                break;
            case TELESCOPE:
                do_TELESCOPE_command();
                break;
            case TERRAFORM:
                do_TERRAFORM_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid post-arrival command.\n");
        }
    }
}

