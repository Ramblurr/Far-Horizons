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

#ifndef FAR_HORIZONS_TRANSACTION_H
#define FAR_HORIZONS_TRANSACTION_H

/* Interspecies transactions. */

#define MAX_TRANSACTIONS        1000

#define EU_TRANSFER                1
#define MESSAGE_TO_SPECIES         2
#define BESIEGE_PLANET             3
#define SIEGE_EU_TRANSFER          4
#define TECH_TRANSFER              5
#define DETECTION_DURING_SIEGE     6
#define SHIP_MISHAP                7
#define ASSIMILATION               8
#define INTERSPECIES_CONSTRUCTION  9
#define TELESCOPE_DETECTION        10
#define ALIEN_JUMP_PORTAL_USAGE    11
#define KNOWLEDGE_TRANSFER         12
#define LANDING_REQUEST            13
#define LOOTING_EU_TRANSFER        14
#define ALLIES_ORDER               15

struct trans_data {
    int type;       /* Transaction type. */
    int donor;
    int recipient;
    int value;      /* Value of transaction. */
    int x;
    int y;
    int z;
    int pn;         /* Location associated with transaction. */
    int number1;    /* Other items associated with transaction.*/
    char name1[40];
    int number2;
    char name2[40];
    int number3;
    char name3[40];
};
typedef struct trans_data trans_data_t;

#endif //FAR_HORIZONS_TRANSACTION_H
