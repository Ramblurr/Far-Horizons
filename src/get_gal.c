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
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "galaxy.h"

void
get_galaxy_data(void) {
    extern struct galaxy_data galaxy;

    int galaxy_fd;
    int num_bytes;

    /* Open galaxy file. */
    galaxy_fd = open("galaxy.dat", O_RDONLY);
    if (galaxy_fd < 0) {
        fprintf(stderr, "\n\tCannot open file galaxy.dat!\n");
        exit(-1);
    }

    /* Read data. */
    num_bytes = read(galaxy_fd, &galaxy, sizeof(struct galaxy_data));
    if (num_bytes != sizeof(struct galaxy_data)) {
        fprintf(stderr, "\n\tCannot read data in file 'galaxy.dat'!\n\n");
        exit(-1);
    }

    close(galaxy_fd);
}
