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

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "engine.h"
#include "enginevars.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "transactionio.h"
#include "shipvars.h"
#include "speciesio.h"
#include "stario.h"
#include "planetio.h"
#include "speciesvars.h"
#include "namplavars.h"
#include "planetvars.h"
#include "commandvars.h"
#include "command.h"
#include "logvars.h"
#include "production.h"
#include "productionvars.h"
#include "do.h"
#include "money.h"


void do_production_orders(void) {
    int i, command;

    truncate_name = TRUE;    /* For these commands, do not display age or landed/orbital status of ships. */

    if (first_pass) {
        printf("\nStart of production orders for species #%d, SP %s...\n", species_number, species->name);
    }

    doing_production = FALSE;    /* This will be set as soon as production actually starts. */
    while (TRUE) {
        command = get_command();

        if (command == 0) {
            fprintf(log_file, "!!! Order ignored:\n");
            fprintf(log_file, "!!! %s", input_line);
            fprintf(log_file, "!!! Unknown or missing command.\n");
            continue;
        }

        if (end_of_file || command == END) {
            /* Handle planets that were not given PRODUCTION orders. */
            next_nampla = nampla_base - 1;
            for (i = 0; i < species->num_namplas; i++) {
                ++next_nampla;

                if (production_done[i]) { continue; }

                production_done[i] = TRUE;

                if (next_nampla->status & DISBANDED_COLONY) { continue; }

                if (next_nampla->mi_base + next_nampla->ma_base == 0) { continue; }

                next_nampla_index = i;

                do_PRODUCTION_command(TRUE);
            }

            transfer_balance();    /* Terminate production for last planet for this species. */

            if (first_pass) {
                gamemaster_abort_option();
                printf("\nEnd of production orders for species #%d, SP %s.\n", species_number, species->name);
            }

            break;            /* END for this species. */
        }

        switch (command) {
            case ALLY:
                do_ALLY_command();
                break;
            case AMBUSH:
                do_AMBUSH_command();
                break;
            case BUILD:
                do_BUILD_command(FALSE, FALSE);
                break;
            case CONTINUE:
                do_BUILD_command(TRUE, FALSE);
                break;
            case DEVELOP:
                do_DEVELOP_command();
                break;
            case ENEMY:
                do_ENEMY_command();
                break;
            case ESTIMATE:
                do_ESTIMATE_command();
                break;
            case HIDE:
                do_HIDE_command();
                break;
            case IBUILD:
                do_BUILD_command(FALSE, TRUE);
                break;
            case ICONTINUE:
                do_BUILD_command(TRUE, TRUE);
                break;
            case INTERCEPT:
                do_INTERCEPT_command();
                break;
            case NEUTRAL:
                do_NEUTRAL_command();
                break;
            case PRODUCTION:
                do_PRODUCTION_command(FALSE);
                break;
            case RECYCLE:
                do_RECYCLE_command();
                break;
            case RESEARCH:
                do_RESEARCH_command();
                break;
            case SHIPYARD:
                do_SHIPYARD_command();
                break;
            case UPGRADE:
                do_UPGRADE_command();
                break;
            default:
                fprintf(log_file, "!!! Order ignored:\n");
                fprintf(log_file, "!!! %s", input_line);
                fprintf(log_file, "!!! Invalid production command.\n");
        }
    }
}
