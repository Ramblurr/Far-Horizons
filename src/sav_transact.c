
#include "fh.h"


extern int	num_transactions;

extern struct trans_data	transaction[MAX_TRANSACTIONS];


save_transaction_data ()

{
    int		i, trans_fd;
    long	num_bytes;


    /* Open file for writing. */
    trans_fd = creat ("interspecies.dat", 0600);

    if (trans_fd < 0)
    {
	fprintf (stderr, "\n\n\tCannot create file 'interspecies.dat'!\n\n");
	exit (-1);
    }

    /* Write transactions to file. */
    for (i = 0; i < num_transactions; i++)
    {
	num_bytes = write (trans_fd, &transaction[i], sizeof(struct trans_data));

	if (num_bytes != sizeof(struct trans_data))
	{
	    fprintf (stderr, "\n\n\tError writing transaction to file 'interspecies.dat'!\n\n");
	    exit (-1);
	}
    }

    close (trans_fd);
}
