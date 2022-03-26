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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "engine.h"
#include "transactionio.h"


int num_transactions;

struct trans_data transaction[MAX_TRANSACTIONS];


typedef struct {
    int32_t type;       /* Transaction type. */
    int16_t donor;
    int16_t recipient;
    int32_t value;      /* Value of transaction. */
    uint8_t x;          /* Location associated with transaction. */
    uint8_t y;
    uint8_t z;
    uint8_t pn;
    int32_t number1;    /* Other items associated with transaction.*/
    uint8_t name1[40];
    int32_t number2;
    uint8_t name2[40];
    int32_t number3;
    uint8_t name3[40];
} binary_data_t;

/* Read transactions from file. */
void get_transaction_data(void) {
    /* Get size of file. */
    struct stat sb;
    if (stat("interspecies.dat", &sb) != 0) {
        num_transactions = 0;
        return;
    }

    // get number of records in the file
    num_transactions = sb.st_size / sizeof(binary_data_t);
    if (sb.st_size != num_transactions * sizeof(binary_data_t)) {
        fprintf(stderr, "\nFile interspecies.dat contains extra bytes (%ld > %ld)!\n\n",
                sb.st_size, num_transactions * sizeof(binary_data_t));
        exit(-1);
    } else if (num_transactions == 0) {
        // nothing to do
        return;
    } else if (num_transactions > MAX_TRANSACTIONS) {
        fprintf(stderr, "\nFile interspecies.dat contains too many records (%d > %d)!\n\n", num_transactions,
                MAX_TRANSACTIONS);
        exit(-1);
    }

    /* Allocate enough memory for all records. */
    binary_data_t *binData = (binary_data_t *) ncalloc(__FUNCTION__, __LINE__, num_transactions, sizeof(binary_data_t));
    if (binData == NULL) {
        perror("get_transaction_data");
        fprintf(stderr, "\nCannot allocate enough memory for transaction data!\n");
        fprintf(stderr, "\n\tattempted to allocate %d transaction entries\n\n", num_transactions);
        exit(-1);
    }

    /* Open transactions file. */
    FILE *fp = fopen("interspecies.dat", "rb");
    if (fp == NULL) {
        perror("get_transaction_data");
        fprintf(stderr, "\nCannot open file 'interspecies.dat' for reading!\n\n");
        exit(-1);
    }

    /* Read it all into memory. */
    if (fread(binData, sizeof(binary_data_t), num_transactions, fp) != num_transactions) {
        fprintf(stderr, "\nCannot read file 'interspecies.dat' into memory!\n");
        fprintf(stderr, "\n\tattempted to read %d transaction entries\n\n", num_transactions);
        exit(-1);
    }

    /* translate data */
    for (int i = 0; i < num_transactions; i++) {
        transaction[i].type = binData[i].type;
        transaction[i].donor = binData[i].donor;
        transaction[i].recipient = binData[i].recipient;
        transaction[i].value = binData[i].value;
        transaction[i].x = binData[i].x;
        transaction[i].y = binData[i].y;
        transaction[i].z = binData[i].z;
        transaction[i].pn = binData[i].pn;
        transaction[i].number1 = binData[i].number1;
        transaction[i].number2 = binData[i].number2;
        transaction[i].number3 = binData[i].number3;
        memcpy(transaction[i].name1, binData[i].name1, 40);
        memcpy(transaction[i].name2, binData[i].name2, 40);
        memcpy(transaction[i].name3, binData[i].name3, 40);
    }

    fclose(fp);

    free(binData);
}


void save_transaction_data(void) {
    /* Open file 'interspecies.dat' for writing. */
    FILE *fp = fopen("interspecies.dat", "wb");
    if (fp == NULL) {
        perror("save_transaction_data");
        fprintf(stderr, "\n\tCannot create file 'interspecies.dat'!\n\n");
        exit(-1);
    }

    if (num_transactions > 0) {
        /* Allocate enough memory for all records. */
        binary_data_t *binData = (binary_data_t *) ncalloc(__FUNCTION__, __LINE__, num_transactions, sizeof(binary_data_t));
        if (binData == NULL) {
            perror("save_transaction_data");
            fprintf(stderr, "\nCannot allocate enough memory for transaction data!\n");
            fprintf(stderr, "\n\tattempted to allocate %d transaction entries\n\n", num_transactions);
            exit(-1);
        }

        /* translate data */
        for (int i = 0; i < num_transactions; i++) {
            binData[i].type = transaction[i].type;
            binData[i].donor = transaction[i].donor;
            binData[i].recipient = transaction[i].recipient;
            binData[i].value = transaction[i].value;
            binData[i].x = transaction[i].x;
            binData[i].y = transaction[i].y;
            binData[i].z = transaction[i].z;
            binData[i].pn = transaction[i].pn;
            binData[i].number1 = transaction[i].number1;
            binData[i].number2 = transaction[i].number2;
            binData[i].number3 = transaction[i].number3;
            memcpy(binData[i].name1, transaction[i].name1, 40);
            memcpy(binData[i].name2, transaction[i].name2, 40);
            memcpy(binData[i].name3, transaction[i].name3, 40);
        }

        /* Write array to disk. */
        if (num_transactions > 0) {
            if (fwrite(binData, sizeof(binary_data_t), num_transactions, fp) != num_transactions) {
                perror("save_transaction_data");
                fprintf(stderr, "\n\n\tCannot write to 'interspecies.dat'!\n\n");
                exit(-1);
            }
        }

        free(binData);
    }
    fclose(fp);
}


void transactionDataAsJson(FILE *fp) {
    fprintf(fp, "[\n");
    for (int i = 0; i < num_transactions; i++) {
        trans_data_t *t = &transaction[i];
        fprintf(fp,
                "  {\"type\": %2d, \"donor\": %3d, \"recipient\": %3d, \"value\": %9d, \"x\": %3d, \"y\": %3d, \"z\": %3d, \"orbit\": %d, \"args\": [{\"number\": %9d, \"name\": \"%s\"}, {\"number\": %9d, \"name\": \"%s\"}, {\"number\": %9d, \"name\": \"%s\"}]}",
                t->type, t->donor, t->recipient, t->value, t->x, t->y, t->z, t->pn, t->number1, t->name1, t->number2,
                t->name2, t->number3, t->name3);
        if (i + 1 < num_transactions) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");

    }
    fprintf(fp, "]\n");
}


void transactionDataAsSExpr(FILE *fp) {
    fprintf(fp, "(transactions");
    for (int i = 0; i < num_transactions; i++) {
        trans_data_t *t = &transaction[i];
        fprintf(fp, "\n  (transaction (type       %9d)", t->type);
        fprintf(fp, "\n               (donor      %9d)", t->donor);
        fprintf(fp, "\n               (recipient  %9d)", t->recipient);
        fprintf(fp, "\n               (value      %9d)", t->recipient);
        fprintf(fp, "\n               (location   (x %3d) (y %3d) (z %3d) (orbit %d))", t->x, t->y, t->z, t->pn);
        fprintf(fp, "\n               (args       (arg (number %9d) (name \"%s\"))", t->number1, t->name1);
        fprintf(fp, "\n                           (arg (number %9d) (name \"%s\"))", t->number2, t->name2);
        fprintf(fp, "\n                           (arg (number %9d) (name \"%s\"))))", t->number3, t->name3);
    }
    fprintf(fp, ")\n");
}
