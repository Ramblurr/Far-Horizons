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

#ifndef FAR_HORIZONS_REPORT_H
#define FAR_HORIZONS_REPORT_H

#include "nampla.h"
#include "ship.h"
#include "species.h"

void do_planet_report(struct nampla_data *nampla, struct ship_data *s_base, struct species_data *species);

void print_mishap_chance(struct ship_data *ship, int destx, int desty, int destz);

void print_ship(struct ship_data *ship, struct species_data *species, int species_number);

void print_ship_header(void);

int reportCommand(int argc, char *argv[]);

#endif //FAR_HORIZONS_REPORT_H
