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
#include <assert.h>
#include "combat.h"
#include "create.h"
#include "enginevars.h"
#include "export.h"
#include "finish.h"
#include "jump.h"
#include "list.h"
#include "location.h"
#include "namplavars.h"
#include "postarrival.h"
#include "predeparture.h"
#include "production.h"
#include "report.h"
#include "scan.h"
#include "sexpr.h"
#include "show.h"
#include "stats.h"
#include "turn.h"
#include "update.h"


int main(int argc, char *argv[]) {
    assert(sizeof(int) > 2); // we can not deal with 16-bit integers at the moment

    if (argc == 1) {
        return showHelp();
    }

    test_mode = FALSE;
    verbose_mode = FALSE;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "?") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
            return showHelp();
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("7.9.3\n");
            return 0;
        } else if (strcmp(argv[i], "combat") == 0) {
            return combatCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "create") == 0) {
            return createCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "export") == 0) {
            return exportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "finish") == 0) {
            return finishCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "inspect") == 0) {
            printf("inspect: sizeof(int)  == %5ld\n", (long unsigned int) sizeof(int));
            printf("inspect: sizeof(long) == %5ld\n", (long unsigned int) sizeof(long));
            return 0;
        } else if (strcmp(argv[i], "jump") == 0) {
            return jumpCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "locations") == 0) {
            return locationCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "list") == 0) {
            return listCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "logrnd") == 0) {
            return logRandomCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "pre-departure") == 0) {
            return preDepartureCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "post-arrival") == 0) {
            return postArrivalCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "production") == 0) {
            return productionCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "report") == 0) {
            return reportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan") == 0) {
            return scanCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan-near") == 0) {
            return scanNearCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "sexpr") == 0) {
            return sexprCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "show") == 0) {
            return showCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "stats") == 0) {
            return statsCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "turn") == 0) {
            return turnCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "update") == 0) {
            return updateCommand(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: unknown option '%s'\n", argv[i]);
            return 2;
        }
    }
    printf("fh: try `fh --help` for instructions\n");
    return 2;
}


