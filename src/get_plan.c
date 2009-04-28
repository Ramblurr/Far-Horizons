
#include "fh.h"


/* In case gamemaster creates new star systems with Edit program. */
#define NUM_EXTRA_PLANETS	100


int			num_planets, planet_data_modified;

struct planet_data	*planet_base;


get_planet_data ()

{
    int		planet_fd;

    long	n, data_size, mem_size;


    /* Open planet file. */
    planet_fd = open ("planets.dat", 0);
    if (planet_fd < 0)
    {
	fprintf (stderr, "\n\tCannot open file planets.dat!\n");
	exit (-1);
    }

    /* Read header data. */
    data_size = read (planet_fd, &num_planets, sizeof(num_planets));
    if (data_size != sizeof(num_planets))
    {
	fprintf (stderr, "\n\tCannot read num_planets in file 'planets.dat'!\n\n");
	exit (-1);
    }

    /* Allocate enough memory for all planets. */
    mem_size =
	(long) (num_planets + NUM_EXTRA_PLANETS) * (long) sizeof(struct planet_data);
    data_size =
	(long) num_planets * (long) sizeof(struct planet_data);
    planet_base = (struct planet_data *) malloc (mem_size);
    if (planet_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for planet file!\n\n");
	exit (-1);
    }

    /* Read it all into memory. */
    n = read (planet_fd, planet_base, data_size);
    if (n != data_size)
    {
	fprintf (stderr, "\nCannot read planet file into memory!\n\n");
	exit (-1);
    }
    close (planet_fd);

    planet_data_modified = FALSE;
}
