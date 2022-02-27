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
#include "enginevars.h"
#include "export.h"
#include "galaxy.h"
#include "list.h"
#include "location.h"
#include "namplavars.h"
#include "report.h"
#include "scan.h"
#include "update.h"
#include "stats.h"
#include "turn.h"


int main(int argc, char *argv[]) {
    test_mode = FALSE;
    verbose_mode = FALSE;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "?") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("usage: fh [option...] command [argument...]\n");
            printf("  opt: --help     show this helpful text\n");
            printf("  opt: -t         enable test mode\n");
            printf("  opt: -v         enable verbose mode\n");
            printf("  cmd: turn       display the current turn number\n");
            printf("  cmd: locations  create locations data file and update economic efficiency\n");
            printf("                  in planets data file\n");
            printf("  cmd: report     create end of turn reports\n");
            printf("  cmd: stats      display statistics\n");
            printf("  cmd: export     convert binary .dat to json or s-expression\n");
            printf("           args:  (json | sexpr) galaxy | stars | planets | species | locations | transactions\n");
            printf("  cmd: logrnd     display a list of random values for testing the PRNG\n");
            printf("  cmd: scan       display a species-specific scan for a location\n");
            printf("           args:  _spNo_ _x_ _y_ _z_\n");
            printf("  cmd: scan-near  display ships and colonies near a location\n");
            printf("           args:  _x_ _y_ _z_ _radiusInParsecs_\n");
            printf("  cmd: set        update values for planet, species, or star\n");
            printf("         args:    (planet | species | star ) values\n");
            return 0;
        } else if (strcmp(argv[i], "-t") == 0) {
            test_mode = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = TRUE;
        } else if (strcmp(argv[i], "create-galaxy") == 0) {
            return createGalaxyCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "create-home-systems") == 0) {
            return createHomeSystemsCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "export") == 0) {
            return exportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "locations") == 0) {
            return locationCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "list") == 0) {
            return listCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "logrnd") == 0) {
            return logRandomCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "report") == 0) {
            return reportCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan") == 0) {
            return scanCommand(argc - i, argv + i);
        } else if (strcmp(argv[i], "scan-near") == 0) {
            return scanNearCommand(argc - i, argv + i);
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
