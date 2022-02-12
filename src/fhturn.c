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
#include <stdlib.h>
#include "galaxy.h"

int main(int argc, char *argv[]) {
    // galaxy.c
    extern struct galaxy_data galaxy;

    /* Check for valid command line. */
    if (argc != 1) {
        fprintf(stderr, "\n\tUsage: TurnNumber\n\n");
        exit(0);
    }

    /* Get galaxy data. */
    get_galaxy_data();

    /* Print the current turn number. */
    printf("%d\n", galaxy.turn_number);

    exit(0);
}
