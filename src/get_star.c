
#include "fh.h"


/* In case gamemaster creates new star systems with Edit program. */
#define NUM_EXTRA_STARS	20


int			num_stars, star_data_modified;

struct star_data	*star_base;


get_star_data ()

{
    int		star_fd;

    long	byte_size, star_data_size, mem_size;


    /* Open star file. */
    star_fd = open ("stars.dat", 0);
    if (star_fd < 0)
    {
	fprintf (stderr, "\n\tCannot open file stars.dat!\n");
	exit (999);
    }

    byte_size = read (star_fd, &num_stars, sizeof(num_stars));
    if (byte_size != sizeof(num_stars))
    {
	fprintf (stderr, "\n\tCannot read num_stars in file 'stars.dat'!\n\n");
	exit (999);
    }

    /* Allocate enough memory for all stars. */
    mem_size =
	(long) (num_stars + NUM_EXTRA_STARS) * (long) sizeof(struct star_data);
    star_data_size =
	(long) num_stars * (long) sizeof(struct star_data);
    star_base = (struct star_data *) malloc (mem_size);
    if (star_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for star file!\n\n");
	exit (-1);
    }

    /* Read it all into memory. */
    byte_size = read (star_fd, star_base, star_data_size);
    if (byte_size != star_data_size)
    {
	fprintf (stderr, "\nCannot read star file into memory!\n\n");
	exit (-1);
    }
    close (star_fd);

    star_data_modified = FALSE;
}
