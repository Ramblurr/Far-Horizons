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

#include "species.h"
#include "item.h"


char item_abbr[MAX_ITEMS][4] = {
        "RM", "PD", "SU", "DR", "CU", "IU", "AU", "FS",
        "JP", "FM", "FJ", "GT", "FD", "TP", "GW", "SG1",
        "SG2", "SG3", "SG4", "SG5", "SG6", "SG7", "SG8", "SG9",
        "GU1", "GU2", "GU3", "GU4", "GU5", "GU6", "GU7", "GU8",
        "GU9", "X1", "X2", "X3", "X4", "X5"
};

short item_carry_capacity[MAX_ITEMS] = {
        1, 3, 20, 1, 1, 1, 1, 1,
        10, 5, 5, 20, 1, 100, 100, 5,
        10, 15, 20, 25, 30, 35, 40, 45,
        5, 10, 15, 20, 25, 30, 35, 40,
        45, 9999, 9999, 9999, 9999, 9999
};

long item_cost[MAX_ITEMS] = {
        1, 1, 110, 50, 1, 1, 1, 25,
        100, 100, 125, 500, 50, 50000, 1000, 250,
        500, 750, 1000, 1250, 1500, 1750, 2000, 2250,
        250, 500, 750, 1000, 1250, 1500, 1750, 2000,
        2250, 9999, 9999, 9999, 9999, 9999
};

char item_critical_tech[MAX_ITEMS] = {
        MI, ML, MA, MA, LS, MI, MA, GV,
        GV, GV, GV, GV, LS, BI, BI, LS,
        LS, LS, LS, LS, LS, LS, LS, LS,
        ML, ML, ML, ML, ML, ML, ML, ML,
        ML, 99, 99, 99, 99, 99
};

char item_name[MAX_ITEMS][32] = {
        "Raw Material Unit",
        "Planetary Defense Unit",
        "Starbase Unit",
        "Damage Repair Unit",
        "Colonist Unit",
        "Colonial Mining Unit",
        "Colonial Manufacturing Unit",
        "Fail-Safe Jump Unit",
        "Jump Portal Unit",
        "Forced Misjump Unit",
        "Forced Jump Unit",
        "Gravitic Telescope Unit",
        "Field Distortion Unit",
        "Terraforming Plant",
        "Germ Warfare Bomb",
        "Mark-1 Shield Generator",
        "Mark-2 Shield Generator",
        "Mark-3 Shield Generator",
        "Mark-4 Shield Generator",
        "Mark-5 Shield Generator",
        "Mark-6 Shield Generator",
        "Mark-7 Shield Generator",
        "Mark-8 Shield Generator",
        "Mark-9 Shield Generator",
        "Mark-1 Gun Unit",
        "Mark-2 Gun Unit",
        "Mark-3 Gun Unit",
        "Mark-4 Gun Unit",
        "Mark-5 Gun Unit",
        "Mark-6 Gun Unit",
        "Mark-7 Gun Unit",
        "Mark-8 Gun Unit",
        "Mark-9 Gun Unit",
        "X1 Unit",
        "X2 Unit",
        "X3 Unit",
        "X4 Unit",
        "X5 Unit",
};

short item_tech_requirment[MAX_ITEMS] = {
        1, 1, 20, 30, 1, 1, 1, 20,
        25, 30, 40, 50, 20, 40, 50, 10,
        20, 30, 40, 50, 60, 70, 80, 90,
        10, 20, 30, 40, 50, 60, 70, 80,
        90, 999, 999, 999, 999, 999
};
