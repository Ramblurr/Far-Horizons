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

#ifndef FAR_HORIZONS_ENGINE_H
#define FAR_HORIZONS_ENGINE_H

#include <stdint.h>
#include <stdio.h>
#include "item.h"


// declare constants
#define TRUE  1
#define FALSE 0

#define STANDARD_NUMBER_OF_SPECIES      15 /* A standard game has 15 species. */
#define STANDARD_NUMBER_OF_STAR_SYSTEMS 90 /* A standard game has 90 star systems. */
#define STANDARD_GALACTIC_RADIUS        20 /* A standard game has a galaxy with a radius of 20 parsecs. */

/* Minimum and maximum values for a galaxy. */
#define MIN_SPECIES  1
#define MAX_SPECIES  100
#define MIN_STARS    12
#define MAX_STARS    1000
#define MIN_RADIUS   6
#define MAX_RADIUS   50
#define MAX_DIAMETER (2*MAX_RADIUS)
#define MAX_PLANETS  (9*MAX_STARS)
#define MAX_OBS_LOCS 5000
#define HP_AVAILABLE_POP 1500

/* Assume at least 32 bits per long word. */
#define NUM_CONTACT_WORDS    ((MAX_SPECIES - 1) / 32) + 1


struct star_data {
    int id;            // unique identifier for this system
    int index;         // index of this system in star_base array
    int x;             /* Coordinates. */
    int y;
    int z;
    int type;          /* Dwarf, degenerate, main sequence or giant. */
    int color;         /* Star color. Blue, blue-white, etc. */
    int size;          /* Star size, from 0 thru 9 inclusive. */
    int num_planets;   /* Number of usable planets in star system. */
    int home_system;   /* TRUE if this is a good potential home system. */
    int worm_here;     /* TRUE if wormhole entry/exit. */
    int worm_x;        /* Coordinates of wormhole's exit. */
    int worm_y;
    int worm_z;
    int planet_index; /* Index (starting at zero) into the file "planets.dat" of the first planet in the star system. */
    int message;       /* Message associated with this star system, if any. */
    uint32_t visited_by[NUM_CONTACT_WORDS]; /* A bit is set if corresponding species has been here. */
};
typedef struct star_data star_data_t;


struct planet_data {
    int id;                 // unique identifier for this planet
    int index;              // index of this planet into the planet_base array
    int temperature_class;  /* Temperature class, 1-30. */
    int pressure_class;     /* Pressure class, 0-29. */
    int special;            /* 0 = not special, 1 = ideal home planet, 2 = ideal colony planet, 3 = radioactive hellhole. */
    int gas[4];             /* Gas in atmosphere. Zero if none. */
    int gas_percent[4];     /* Percentage of gas in atmosphere. */
    int diameter;           /* Diameter in thousands of kilometers. */
    int gravity;            /* Surface gravity. Multiple of Earth gravity times 100. */
    int mining_difficulty;  /* Mining difficulty times 100. */
    int econ_efficiency;    /* Economic efficiency. Always 100 for a home planet. */
    int md_increase;        /* Increase in mining difficulty. */
    int message;            /* Message associated with this planet, if any. */
    int isValid;            // FALSE if the record is invalid
    star_data_t *system;    // pointer to the star the planet is orbiting
    int orbit;              // orbit of planet in the system
};
typedef struct planet_data planet_data_t;


struct nampla_data {
    char name[32];                 /* Name of planet. */
    int x, y, z, pn;               /* Coordinates. */
    int status;                    /* Status of planet. */
    int hiding;                    /* HIDE order given. */
    int hidden;                    /* Colony is hidden. */
    int planet_index;              /* Index (starting at zero) into the file "planets.dat" of this planet. */
    int siege_eff;                 /* Siege effectiveness - a percentage between 0 and 99. */
    int shipyards;                 /* Number of shipyards on planet. */
    int IUs_needed;                /* Incoming ship with only CUs on board. */
    int AUs_needed;                /* Incoming ship with only CUs on board. */
    int auto_IUs;                  /* Number of IUs to be automatically installed. */
    int auto_AUs;                  /* Number of AUs to be automatically installed. */
    int IUs_to_install;            /* Colonial mining units to be installed. */
    int AUs_to_install;            /* Colonial manufacturing units to be installed. */
    int mi_base;                   /* Mining base times 10. */
    int ma_base;                   /* Manufacturing base times 10. */
    int pop_units;                 /* Number of available population units. */
    int item_quantity[MAX_ITEMS];  /* Quantity of each item available. */
    int use_on_ambush;             /* Amount to use on ambush. */
    int message;                   /* Message associated with this planet, if any. */
    int special;                   /* Different for each application. */
    star_data_t *system;           // pointer to system the colony is in
    planet_data_t *planet;         // pointer to planet the colony is on
};
typedef struct nampla_data nampla_data_t;


struct ship_data {
    char name[32];                 /* Name of ship. */
    int x, y, z, pn;               /* Current coordinates. */
    int status;                    /* Current status of ship. */
    int type;                      /* Ship type. */
    int dest_x;                    /* Destination if ship was forced to jump from combat. */
    int dest_y;                    /* Ditto. */
    int dest_z;                    /* Ditto. Also used by TELESCOPE command. */
    int just_jumped;               /* Set if ship jumped this turn. */
    int arrived_via_wormhole;      /* Ship arrived via wormhole in the PREVIOUS turn. */
    int class;                     /* Ship class. */
    int tonnage;                   /* Ship tonnage divided by 10,000. */
    int item_quantity[MAX_ITEMS];  /* Quantity of each item carried. */
    int age;                       /* Ship age. */
    int remaining_cost;            /* The cost needed to complete the ship if still under construction. */
    int loading_point;             /* Nampla index for planet where ship was last loaded with CUs. Zero = none. Use 9999 for home planet. */
    int unloading_point;           /* Nampla index for planet that ship should be given orders to jump to where it will unload. Zero = none. Use 9999 for home planet. */
    int special;                   /* Different for each application. */
};
typedef struct ship_data ship_data_t;


struct species_data {
    int id;                              // unique identifier for this species
    int index;                           // index of this species in spec_data array
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
    star_data_t *homeSystem;             // pointer to the star containing the planet
    nampla_data_t *homeColony;           // not a pointer to the planet but a pointer to the nampla
};
typedef struct species_data species_data_t;


int agrep_score(char *correct_string, char *unknown_string);

char *commas(long value);

void gamemaster_abort_option(void);

int logRandomCommand(int argc, char *argv[]);

char *readln(char *dst, int len, FILE *fp);

int rnd(unsigned int max);

#endif //FAR_HORIZONS_ENGINE_H
