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
#include <stdio.h>
#include "engine.h"
#include "planet.h"

/* Tech level ids. */
#define MI 0 /* Mining tech level. */
#define MA 1 /* Manufacturing tech level. */
#define ML 2 /* Military tech level. */
#define GV 3 /* Gravitics tech level. */
#define LS 4 /* Life Support tech level. */
#define BI 5 /* Biology tech level. */

struct species_data {
    char name[32];                       /* Name of species. */
    char govt_name[32];                  /* Name of government. */
    char govt_type[32];                  /* Type of government. */
    int x, y, z, pn;                     /* Coordinates of home planet. */
    int required_gas;                    /* Gas required by species. */
    int required_gas_min;                /* Minimum needed percentage. */
    int required_gas_max;                /* Maximum allowed percentage. */
    int neutral_gas[6];                  /* Gases neutral to species. */
    int poison_gas[6];                   /* Gases poisonous to species. */
    int auto_orders;                     /* AUTO command was issued. */
    int tech_level[6];                   /* Actual tech levels. */
    int init_tech_level[6];              /* Tech levels at start of turn. */
    int tech_knowledge[6];               /* Unapplied tech level knowledge. */
    int num_namplas;                     /* Number of named planets, including home planet and colonies. */
    int num_ships;                       /* Number of ships. */
    int tech_eps[6];                     /* Experience points for tech levels. */
    int hp_original_base;                /* If non-zero, home planet was bombed either by bombardment or germ warfare and has not yet fully recovered. Value is total economic base before bombing. */
    int econ_units;                      /* Number of economic units. */
    int fleet_cost;                      /* Total fleet maintenance cost. */
    int fleet_percent_cost;              /* Fleet maintenance cost as a percentage times one hundred. */
    uint32_t contact[NUM_CONTACT_WORDS]; /* A bit is set if corresponding species has been met. */
    uint32_t ally[NUM_CONTACT_WORDS];    /* A bit is set if corresponding species is considered an ally. */
    uint32_t enemy[NUM_CONTACT_WORDS];   /* A bit is set if corresponding species is considered an enemy. */
};

typedef struct species_data species_data_t;

int distorted(int species_number);

void free_species_data(void);

int life_support_needed(struct species_data *species, struct planet_data *home, struct planet_data *colony);

int undistorted(int distorted_species_number);

#endif //FAR_HORIZONS_SPECIES_H
