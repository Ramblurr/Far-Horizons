
/* The following routine will append an interspecies transaction to the file
	interspecies.dat. */


#include "fh.h"


int	trans_fd;
int	num_transactions = 0;


extern int	first_pass;


add_transaction (transaction)

struct trans_data	*transaction;


{
    long	num_bytes;


    if (first_pass) return;


    /* If the file is not yet open, open it for appending. */
    if (num_transactions == 0)
    {
	/* Open for writing. */
	trans_fd = open ("interspecies.dat", 1);

	if (trans_fd < 0)
	{
	    /* File does not exist. Create it. */
	    trans_fd = creat ("interspecies.dat", 0600);
	    if (trans_fd < 0)
	    {
		fprintf (stderr, "\n\tCannot create file interspecies.dat!\n\n");
		exit (-1);
	    }
	}
	else
	{
	    /* Position to end-of-file for appending. */
	    num_bytes = lseek (trans_fd, 0L, 2);
	    num_transactions = num_bytes / sizeof(struct trans_data);
	}
    }

    /* Write transaction to file. */
    num_bytes = write (trans_fd, transaction, sizeof(struct trans_data));
    if (num_bytes != sizeof(struct trans_data))
    {
	fprintf (stderr, "\n\tCannot write transaction to file interspecies.dat!\n\n");
	exit (-1);
    }

    ++num_transactions;
}
