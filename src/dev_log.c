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

#include "log.h"
#include "dev_log.h"

void start_dev_log(int num_CUs, int num_IUs, int num_AUs) {
    log_string("    ");
    log_int(num_CUs);
    log_string(" Colonist Unit");
    if (num_CUs != 1) {
        log_char('s'); }

    if (num_IUs + num_AUs == 0) {
        goto done;
    }

    if (num_IUs > 0) {
        if (num_AUs == 0) {
            log_string(" and ");
        } else {
            log_string(", ");
        }

        log_int(num_IUs);
        log_string(" Colonial Mining Unit");
        if (num_IUs != 1) { log_char('s'); }
    }

    if (num_AUs > 0) {
        if (num_IUs > 0) { log_char(','); }

        log_string(" and ");

        log_int(num_AUs);
        log_string(" Colonial Manufacturing Unit");
        if (num_AUs != 1) { log_char('s'); }
    }

    done:

    log_string(" were built");
}
