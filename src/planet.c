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

#include "engine.h"
#include "planet.h"
#include "speciesvars.h"


void fix_gases(struct planet_data *pl) {
    int i, j, total, left, add_neutral;
    long n;

    total = 0;
    for (i = 0; i < 4; i++) { total += pl->gas_percent[i]; }

    if (total == 100) { return; }

    left = 100 - total;

    /* If we have at least one gas that is not the required gas, then we simply need to adjust existing gases.
     * Otherwise, we have to add a neutral gas. */
    add_neutral = TRUE;
    for (i = 0; i < 4; i++) {
        if (pl->gas_percent[i] == 0) { continue; }
        if (pl->gas[i] == species->required_gas) { continue; }
        add_neutral = FALSE;
        break;
    }

    if (add_neutral) { goto add_neutral_gas; }

    /* Randomly modify existing non-required gases until total percentage is exactly 100. */
    while (left != 0) {
        i = rnd(4) - 1;
        if (pl->gas_percent[i] == 0) { continue; }
        if (pl->gas[i] == species->required_gas) { continue; }
        if (left > 0) {
            if (left > 2) {
                j = rnd(left);
            } else {
                j = left;
            }

            pl->gas_percent[i] += j;
            left -= j;
        } else {
            if (-left > 2) {
                j = rnd(-left);
            } else {
                j = -left;
            }

            if (j < pl->gas_percent[i]) {
                pl->gas_percent[i] -= j;
                left += j;
            }
        }
    }

    return;

    add_neutral_gas:

    /* If we reach this point, there is either no atmosphere or it contains only the required gas.
     * In either case, add a random neutral gas. */
    for (i = 0; i < 4; i++) {
        if (pl->gas_percent[i] > 0) { continue; }

        j = rnd(6) - 1;
        pl->gas[i] = species->neutral_gas[j];
        pl->gas_percent[i] = left;

        break;
    }
}
