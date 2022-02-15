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

#ifndef FAR_HORIZONS_ITEM_H
#define FAR_HORIZONS_ITEM_H

/* Item IDs. */
#define RM           0     /* Raw Material Units. */
#define PD           1     /* Planetary Defense Units. */
#define SU           2     /* Starbase Units. */
#define DR           3     /* Damage Repair Units. */
#define CU           4     /* Colonist Units. */
#define IU           5     /* Colonial Mining Units. */
#define AU           6     /* Colonial Manufacturing Units. */
#define FS           7     /* Fail-Safe Jump Units. */
#define JP           8     /* Jump Portal Units. */
#define FM           9     /* Forced Misjump Units. */
#define FJ           10    /* Forced Jump Units. */
#define GT           11    /* Gravitic Telescope Units. */
#define FD           12    /* Field Distortion Units. */
#define TP           13    /* Terraforming Plants. */
#define GW           14    /* Germ Warfare Bombs. */
#define SG1          15    /* Mark-1 Auxiliary Shield Generators. */
#define SG2          16    /* Mark-2. */
#define SG3          17    /* Mark-3. */
#define SG4          18    /* Mark-4. */
#define SG5          19    /* Mark-5. */
#define SG6          20    /* Mark-6. */
#define SG7          21    /* Mark-7. */
#define SG8          22    /* Mark-8. */
#define SG9          23    /* Mark-9. */
#define GU1          24    /* Mark-1 Auxiliary Gun Units. */
#define GU2          25    /* Mark-2. */
#define GU3          26    /* Mark-3. */
#define GU4          27    /* Mark-4. */
#define GU5          28    /* Mark-5. */
#define GU6          29    /* Mark-6. */
#define GU7          30    /* Mark-7. */
#define GU8          31    /* Mark-8. */
#define GU9          32    /* Mark-9. */
#define X1           33    /* Unassigned. */
#define X2           34    /* Unassigned. */
#define X3           35    /* Unassigned. */
#define X4           36    /* Unassigned. */
#define X5           37    /* Unassigned. */
#define MAX_ITEMS    38    /* Always bump this up to a multiple of two. Don't forget to make room for zeroth element! */

// globals. ugh.

extern char item_abbr[MAX_ITEMS][4];
extern short item_carry_capacity[MAX_ITEMS];
extern long item_cost[MAX_ITEMS];
extern char item_critical_tech[MAX_ITEMS];
extern char item_name[MAX_ITEMS][32];
extern short item_tech_requirment[MAX_ITEMS];

#endif //FAR_HORIZONS_ITEM_H
