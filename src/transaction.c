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
#include <stdio.h>
#include "transaction.h"

int num_transactions;
struct trans_data transaction[MAX_TRANSACTIONS];

/* Read transactions from file. */
void get_transaction_data(void) {
    long fileSize;
    /* Open transactions file. */
    FILE *fp = fopen("interspecies.dat", "rb");
    if (fp == NULL) {
        num_transactions = 0;
        return;
    }
    /* Get size of file. */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    // get number of records in the file
    num_transactions = fileSize / sizeof(struct trans_data);
    if (fileSize != num_transactions * sizeof(struct trans_data)) {
        fprintf(stderr, "\nFile 'interspecies.dat' contains extra bytes (%d > %d)!\n\n", fileSize,
                num_transactions * sizeof(struct trans_data));
        exit(-1);
    } else if (num_transactions > MAX_TRANSACTIONS) {
        fprintf(stderr, "\nFile 'interspecies.dat' contains too many records (%d > %d)!\n\n", num_transactions,
                MAX_TRANSACTIONS);
        exit(-1);
    }
    /* Read it all into memory. */
    if (fread(transaction, sizeof(struct trans_data), num_transactions, fp) != num_transactions) {
        fprintf(stderr, "\nCannot read file 'interspecies.dat' into memory!\n\n");
        exit(-1);
    }
    fclose(fp);
}

void save_transaction_data(void) {
    /* Open file for writing. */
    FILE *fp = fopen("interspecies.dat", "wb");
    if (fp == NULL) {
        fprintf(stderr, "\n\n\tCannot create file 'interspecies.dat'!\n\n");
        exit(-1);
    }
    /* Write transactions to file. */
    if (num_transactions > 0) {
        if (fwrite(transaction, sizeof(struct trans_data), num_transactions, fp) != num_transactions) {
            fprintf(stderr, "\n\n\tError writing transaction to file 'interspecies.dat'!\n\n");
            exit(-1);
        }
    }
    fclose(fp);
}

void transactionDataAsSExpr(FILE *fp) {
    fprintf(fp, "(transactions");
    for (int i = 0; i < num_transactions; i++) {
        trans_data_t *t = &transaction[i];
        fprintf(fp,
                "\n  (transaction (type %2d) (donor %3d) (recipient %3d) (value %9d) (x %3d) (y %3d) (z %3d) (orbit %d) (args (arg (number %9d) (name \"%s\")) (arg (number %9d) (name \"%s\")) (arg (number %9d) (name \"%s\")))",
                t->type, t->donor, t->recipient, t->value, t->x, t->y, t->z, t->pn, t->number1, t->name1, t->number2,
                t->name2, t->number3, t->name3);

    }
    fprintf(fp, ")\n");
}