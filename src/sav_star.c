
#include "fh.h"


extern int			num_stars;

extern struct star_data		*star_base;


save_star_data ()

{
    int		star_fd;

    long	n, byte_size;


    /* Open star file for writing. */
    star_fd = creat ("stars.dat", 0600);
    if (star_fd < 0)
    {
	fprintf (stderr, "\n\tCannot create file 'stars.dat'!\n");
	exit (-1);
    }

    /* Write header data. */
    byte_size = write (star_fd, &num_stars, sizeof(num_stars));
    if (byte_size != sizeof(num_stars))
    {
	fprintf (stderr, "\n\tCannot write num_stars to file 'stars.dat'!\n\n");
	exit (-1);
    }

    /* Write star data to disk. */
    byte_size = (long) num_stars * sizeof(struct star_data);
    n = write (star_fd, star_base, byte_size);
    if (n != byte_size)
    {
	fprintf (stderr, "\nCannot write star data to disk!\n\n");
	exit (-1);
    }

    close (star_fd);
}
