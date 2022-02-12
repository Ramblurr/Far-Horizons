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

#ifndef FAR_HORIZONS_SPECIES_H
#define FAR_HORIZONS_SPECIES_H

#include <stdint.h>
#include "engine.h"

/* Tech level ids. */
#define MI 0 /* Mining tech level. */
#define MA 1 /* Manufacturing tech level. */
#define ML 2 /* Military tech level. */
#define GV 3 /* Gravitics tech level. */
#define LS 4 /* Life Support tech level. */
#define BI 5 /* Biology tech level. */

struct species_data {
    uint8_t name[32];                      /* Name of species. */
    uint8_t govt_name[32];                 /* Name of government. */
    uint8_t govt_type[32];                 /* Type of government. */
    uint8_t x, y, z, pn;                   /* Coordinates of home planet. */
    uint8_t required_gas;                  /* Gas required by species. */
    uint8_t required_gas_min;              /* Minimum needed percentage. */
    uint8_t required_gas_max;              /* Maximum allowed percentage. */
    uint8_t reserved5;                     /* Zero for now. */
    uint8_t neutral_gas[6];                /* Gases neutral to species. */
    uint8_t poison_gas[6];                 /* Gases poisonous to species. */
    uint8_t auto_orders;                   /* AUTO command was issued. */
    uint8_t reserved3;                     /* Zero for now. */
    int16_t reserved4;                     /* Zero for now. */
    int16_t tech_level[6];                 /* Actual tech levels. */
    int16_t init_tech_level[6];            /* Tech levels at start of turn. */
    int16_t tech_knowledge[6];             /* Unapplied tech level knowledge. */
    int32_t num_namplas;                   /* Number of named planets, including home planet and colonies. */
    int32_t num_ships;                     /* Number of ships. */
    int32_t tech_eps[6];                   /* Experience points for tech levels. */
    int32_t hp_original_base;              /* If non-zero, home planet was bombed either by bombardment or germ warfare and has not yet fully recovered. Value is total economic base before bombing. */
    int32_t econ_units;                    /* Number of economic units. */
    int32_t fleet_cost;                    /* Total fleet maintenance cost. */
    int32_t fleet_percent_cost;            /* Fleet maintenance cost as a percentage times one hundred. */
    uint32_t contact[NUM_CONTACT_WORDS];   /* A bit is set if corresponding species has been met. */
    uint32_t ally[NUM_CONTACT_WORDS];      /* A bit is set if corresponding species is considered an ally. */
    uint32_t enemy[NUM_CONTACT_WORDS];     /* A bit is set if corresponding species is considered an enemy. */
    uint8_t padding[12];                   /* Use for expansion. Initialized to all zeroes. */
};

void free_species_data(void);

void get_species_data(void);

#endif //FAR_HORIZONS_SPECIES_H
