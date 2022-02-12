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

#include <stdlib.h>
#include <stdio.h>
#include "location.h"
#include "galaxy.h"
#include "species.h"
#include "nampla.h"
#include "ship.h"

// from galaxy.c
extern struct galaxy_data galaxy;

// from species.c
extern int species_number;
extern int data_in_memory[];
extern struct species_data *species;
extern struct species_data *spec_data;

// from nampla.c
extern struct nampla_data **namp_data;
extern struct nampla_data *nampla_base;
extern struct nampla_data *nampla;

// from ship.c
extern struct ship_data **ship_data;
extern struct ship_data *ship_base;
extern struct ship_data *ship;

int num_locs = 0;
struct sp_loc_data loc[MAX_LOCATIONS];

void add_location(char x, char y, char z) {
    for (int i = 0; i < num_locs; i++) {
        if (loc[i].x == x && loc[i].y == y && loc[i].z == z && loc[i].s == species_number) {
            return; /* This location is already in list for this species. */
        }
    }

/* Add new location to the list. */
    loc[num_locs].x = x;
    loc[num_locs].y = y;
    loc[num_locs].z = z;
    loc[num_locs].s = species_number;
    num_locs++;
    if (num_locs < MAX_LOCATIONS) {
        return;
    }
    fprintf(stderr, "\n\n\tInternal error. Overflow of 'loc' arrays!\n\n");
    exit(-1);
}

/* This routine will create the "loc" array based on current species' data. */
void do_locations(void) {
    num_locs = 0;
    for (species_number = 1; species_number <= galaxy.num_species; species_number++) {
        int spidx = species_number - 1;
        if (data_in_memory[spidx] == FALSE) {
            continue;
        }

        species = &spec_data[spidx];
        nampla_base = namp_data[spidx];
        ship_base = ship_data[spidx];

        nampla = nampla_base - 1;
        for (int i = 0; i < species->num_namplas; i++) {
            nampla++;
            if (nampla->pn == 99) {
                continue;
            }
            if (nampla->status & POPULATED) {
                add_location(nampla->x, nampla->y, nampla->z);
            }
        }

        ship = ship_base - 1;
        for (int i = 0; i < species->num_ships; i++) {
            ship++;
            if (ship->pn == 99) {
                continue;
            } else if (ship->status == FORCED_JUMP || ship->status == JUMPED_IN_COMBAT) {
                continue;
            }
            add_location(ship->x, ship->y, ship->z);
        }
    }
}

void get_location_data(void) {
    long fileSize;

    /* Open locations file. */
    FILE *fp = fopen("locations.dat", "rb");
    if (fp == NULL) {
        fprintf(stderr, "\nCannot open file 'locations.dat' for reading!\n\n");
        exit(-1);
    }
    /* Get size of file. */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    // get number of records in the file
    num_locs = fileSize / sizeof(struct sp_loc_data);
    if (fileSize != num_locs * sizeof(struct sp_loc_data)) {
        fprintf(stderr, "\nFile locations.dat contains extra bytes (%d > %d)!\n\n", fileSize,
                num_locs * sizeof(struct sp_loc_data));
        exit(-1);
    } else if (num_locs > MAX_LOCATIONS) {
        fprintf(stderr, "\nFile locations.dat contains too many records (%d > %d)!\n\n", num_locs, MAX_LOCATIONS);
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(loc, sizeof(struct sp_loc_data), num_locs, fp) != num_locs) {
        fprintf(stderr, "\nCannot read file 'locations.dat' into memory!\n\n");
        exit(-1);
    }
    fclose(fp);
}

// locationDataAsSExpr writes the current location data to a text file as an s-expression.
void locationDataAsSExpr(FILE *fp) {
    fprintf(fp, "(locations");
    for (int i = 0; i < num_locs; i++) {
        sp_loc_data_t *p = &loc[i];
        fprintf(fp, "\n  (location (x %3d) (y %3d) (z %d) (species %3d))", p->x, p->y, p->z, p->s);
    }
    fprintf(fp, ")\n");
}

void save_location_data(void) {
    /* Open file 'locations.dat' for writing. */
    FILE *fp = fopen("locations.dat", "wb");
    if (fp == NULL) {
        perror("save_location_data");
        fprintf(stderr, "\n\tCannot create file 'locations.dat'!\n\n");
        exit(-1);
    } else if (num_locs == 0) {
        fclose(fp);
        return;
    }
    /* Write array to disk. */
    if (fwrite(loc, sizeof(struct sp_loc_data), num_locs, fp) != num_locs) {
        perror("save_location_data");
        fprintf(stderr, "\n\n\tCannot write to 'locations.dat'!\n\n");
        exit(-1);
    }
    fclose(fp);
}



