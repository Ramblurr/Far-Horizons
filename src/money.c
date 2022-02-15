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
#include "speciesvars.h"
#include "namplavars.h"
#include "logvars.h"
#include "money.h"


long balance;
long EU_spending_limit;
long production_capacity;
long raw_material_units;


int check_bounced(long amount_needed) {
    long take_from_EUs, limiting_balance;

    /* Check if we have sufficient funds for this purchase. */
    if (amount_needed > balance) {
        take_from_EUs = amount_needed - balance;

        if (take_from_EUs <= EU_spending_limit && take_from_EUs <= species->econ_units) {
            species->econ_units -= take_from_EUs;
            EU_spending_limit -= take_from_EUs;
            balance = amount_needed;
        } else {
            return TRUE;
        }
    }

    /* Reduce various balances appropriately. */
    if (raw_material_units >= amount_needed) {
        if (production_capacity >= amount_needed) {
            /* Enough of both. */
            raw_material_units -= amount_needed;
            production_capacity -= amount_needed;
        } else {
            /* Enough RMs but not enough PC. */
            raw_material_units -= production_capacity;
            production_capacity = 0;
        }
    } else {
        if (production_capacity >= amount_needed) {
            /* Enough PC but not enough RMs. */
            production_capacity -= raw_material_units;
            raw_material_units = 0;
        } else {
            /* Not enough RMs or PC. */
            limiting_balance = (raw_material_units > production_capacity) ? production_capacity : raw_material_units;
            raw_material_units -= limiting_balance;
            production_capacity -= limiting_balance;
        }
    }

    balance -= amount_needed;

    return FALSE;
}


void transfer_balance(void) {
    long limiting_amount;

    /* Log end of production.
     * Do not print ending balance for mining or resort colonies. */
    limiting_amount = 0;
    fprintf(log_file, "  End of production on PL %s.", nampla->name);
    if (!(nampla->status & (MINING_COLONY | RESORT_COLONY))) {
        limiting_amount = (raw_material_units > production_capacity) ? production_capacity : raw_material_units;
        fprintf(log_file, " (Ending balance is %ld.)", limiting_amount);
    }
    fprintf(log_file, "\n");

    /* Convert unused balance to economic units. */
    species->econ_units += limiting_amount;
    raw_material_units -= limiting_amount;

    /* Carry over unused raw material units into next turn. */
    nampla->item_quantity[RM] += raw_material_units;

    balance = 0;
}
