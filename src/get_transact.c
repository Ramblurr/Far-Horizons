
#include "fh.h"


int	num_transactions;

struct trans_data	transaction[MAX_TRANSACTIONS];


get_transaction_data ()


{
    int		i, trans_fd;
    long	num_bytes;


    /* Open file for reading. */
    trans_fd = open ("interspecies.dat", 0);

    if (trans_fd < 0)
    {
	num_transactions = 0;
	return;
    }

    /* Read transactions from file. */
    i = 0;
    while (1)
    {
	num_bytes = read (trans_fd, &transaction[i], sizeof(struct trans_data));
	if (num_bytes == 0) break;	/* End of file. */

	if (num_bytes != sizeof(struct trans_data))
	{
	    fprintf (stderr, "\n\n\tError reading transaction from file 'interspecies.dat'!\n\n");
	    exit (-1);
	}

	++i;
    }

    num_transactions = i;

    close (trans_fd);
}
