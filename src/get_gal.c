
#include "fh.h"


extern struct galaxy_data	galaxy;


get_galaxy_data ()

{
    int		galaxy_fd;

    long	n, num_bytes, byte_size;


    /* Open galaxy file. */
    galaxy_fd = open ("galaxy.dat", 0);
    if (galaxy_fd < 0)
    {
	fprintf (stderr, "\n\tCannot open file galaxy.dat!\n");
	exit (-1);
    }

    /* Read data. */
    byte_size = sizeof (struct galaxy_data);
    num_bytes = read (galaxy_fd, &galaxy, byte_size);
    if (num_bytes != byte_size)
    {
	fprintf (stderr, "\n\tCannot read data in file 'galaxy.dat'!\n\n");
	exit (-1);
    }

    close (galaxy_fd);
}
