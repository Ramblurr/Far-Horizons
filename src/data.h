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

#ifndef FAR_HORIZONS_DATA_H
#define FAR_HORIZONS_DATA_H

#include <stdint.h>
#include "const.h"
#include "item.h"


typedef struct {
    int32_t d_num_species;  /* Design number of species in galaxy. */
    int32_t num_species;    /* Actual number of species allocated. */
    int32_t radius;         /* Galactic radius in parsecs. */
    int32_t turn_number;    /* Current turn number. */
} binary_galaxy_data_t;


typedef struct {
    uint8_t name[32];                   /* Name of planet. */
    uint8_t x, y, z, pn;                /* Coordinates. */
    uint8_t status;                     /* Status of planet. */
    uint8_t reserved1;                  /* Zero for now. */
    uint8_t hiding;                     /* HIDE order given. */
    uint8_t hidden;                     /* Colony is hidden. */
    int16_t reserved2;                  /* Zero for now. */
    int16_t planet_index;               /* Index (starting at zero) into the file "planets.dat" of this planet. */
    int16_t siege_eff;                  /* Siege effectiveness - a percentage between 0 and 99. */
    int16_t shipyards;                  /* Number of shipyards on planet. */
    int32_t reserved4;                  /* Zero for now. */
    int32_t IUs_needed;                 /* Incoming nampla with only CUs on board. */
    int32_t AUs_needed;                 /* Incoming nampla with only CUs on board. */
    int32_t auto_IUs;                   /* Number of IUs to be automatically installed. */
    int32_t auto_AUs;                   /* Number of AUs to be automatically installed. */
    int32_t reserved5;                  /* Zero for now. */
    int32_t IUs_to_install;             /* Colonial mining units to be installed. */
    int32_t AUs_to_install;             /* Colonial manufacturing units to be installed. */
    int32_t mi_base;                    /* Mining base times 10. */
    int32_t ma_base;                    /* Manufacturing base times 10. */
    int32_t pop_units;                  /* Number of available population units. */
    int32_t item_quantity[MAX_ITEMS];   /* Quantity of each item available. */
    int32_t reserved6;                  /* Zero for now. */
    int32_t use_on_ambush;              /* Amount to use on ambush. */
    int32_t message;                    /* Message associated with this planet, if any. */
    int32_t special;                    /* Different for each application. */
    uint8_t padding[28];                /* Use for expansion. Initialized to all zeroes. */
} binary_nampla_data_t;


typedef struct {
    uint8_t temperature_class;  /* Temperature class, 1-30. */
    uint8_t pressure_class;     /* Pressure class, 0-29. */
    uint8_t special;            /* 0 = not special, 1 = ideal home planet, 2 = ideal colony planet, 3 = radioactive hellhole. */
    uint8_t reserved1;          /* Reserved for future use. Zero for now. */
    uint8_t gas[4];             /* Gas in atmosphere. Zero if none. */
    uint8_t gas_percent[4];     /* Percentage of gas in atmosphere. */
    int16_t reserved2;          /* Reserved for future use. Zero for now. */
    int16_t diameter;           /* Diameter in thousands of kilometers. */
    int16_t gravity;            /* Surface gravity. Multiple of Earth gravity times 100. */
    int16_t mining_difficulty;  /* Mining difficulty times 100. */
    int16_t econ_efficiency;    /* Economic efficiency. Always 100 for a home planet. */
    int16_t md_increase;        /* Increase in mining difficulty. */
    int32_t message;            /* Message associated with this planet, if any. */
    int32_t reserved3;          /* Reserved for future use. Zero for now. */
    int32_t reserved4;          /* Reserved for future use. Zero for now. */
    int32_t reserved5;          /* Reserved for future use. Zero for now. */
} binary_planet_data_t;


typedef struct {
    uint8_t name[32];                   /* Name of ship. */
    uint8_t x, y, z, pn;                /* Current coordinates. */
    uint8_t status;                     /* Current status of ship. */
    uint8_t type;                       /* Ship type. */
    uint8_t dest_x, dest_y;             /* Destination if ship was forced to jump from combat. */
    uint8_t dest_z;                     /* Ditto. Also used by TELESCOPE command. */
    uint8_t just_jumped;                /* Set if ship jumped this turn. */
    uint8_t arrived_via_wormhole;       /* Ship arrived via wormhole in the PREVIOUS turn. */
    uint8_t reserved1;                  /* Unused. Zero for now. */
    int16_t reserved2;                  /* Unused. Zero for now. */
    int16_t reserved3;                  /* Unused. Zero for now. */
    int16_t class;                      /* Ship class. */
    int16_t tonnage;                    /* Ship tonnage divided by 10,000. */
    int16_t item_quantity[MAX_ITEMS];   /* Quantity of each item carried. */
    int16_t age;                        /* Ship age. */
    int16_t remaining_cost;             /* The cost needed to complete the ship if still under construction. */
    int16_t reserved4;                  /* Unused. Zero for now. */
    int16_t loading_point;              /* Nampla index for planet where ship was last loaded with CUs. Zero = none. Use 9999 for home planet. */
    int16_t unloading_point;            /* Nampla index for planet that ship should be given orders to jump to where it will unload. Zero = none. Use 9999 for home planet. */
    int32_t special;                    /* Different for each application. */
    uint8_t padding[28];                /* Use for expansion. Initialized to all zeroes. */
} binary_ship_data_t;


typedef struct {
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
} binary_species_data_t;


typedef struct {
    uint8_t x;              /* Coordinates. */
    uint8_t y;
    uint8_t z;
    uint8_t type;           /* Dwarf, degenerate, main sequence or giant. */
    uint8_t color;          /* Star color. Blue, blue-white, etc. */
    uint8_t size;           /* Star size, from 0 thru 9 inclusive. */
    uint8_t num_planets;    /* Number of usable planets in star system. */
    uint8_t home_system;    /* TRUE if this is a good potential home system. */
    uint8_t worm_here;      /* TRUE if wormhole entry/exit. */
    uint8_t worm_x;         /* Coordinates of wormhole exit (if there is one) */
    uint8_t worm_y;
    uint8_t worm_z;
    int16_t reserved1;      /* Reserved for future use. Zero for now. */
    int16_t reserved2;      /* Reserved for future use. Zero for now. */
    int16_t planet_index;   /* Index (starting at zero) into the file "planets.dat" of the first planet in the star system. */
    int32_t message;        /* Message associated with this star system, if any. */
    uint32_t visited_by[NUM_CONTACT_WORDS]; /* A bit is set if corresponding species has been here. */
    int32_t reserved3;      /* Reserved for future use. Zero for now. */
    int32_t reserved4;      /* Reserved for future use. Zero for now. */
    int32_t reserved5;      /* Reserved for future use. Zero for now. */
} binary_star_data_t;


#endif //FAR_HORIZONS_DATA_H
