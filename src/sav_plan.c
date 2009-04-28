
#include "fh.h"


extern int			num_planets;

extern struct planet_data	*planet_base;


save_planet_data ()

{
    int		planet_fd;

    long	n, byte_size;


    /* Open planet file for writing. */
    planet_fd = creat ("planets.dat", 0600);
    if (planet_fd < 0)
    {
	fprintf (stderr, "\n\tCannot create file 'planets.dat'!\n");
	exit (-1);
    }

    /* Write header data. */
    byte_size = write (planet_fd, &num_planets, sizeof(num_planets));
    if (byte_size != sizeof(num_planets))
    {
	fprintf (stderr, "\n\tCannot write num_planets to file 'planets.dat'!\n\n");
	exit (-1);
    }

    /* Write planet data to disk. */
    byte_size = (long) num_planets * sizeof(struct planet_data);
    n = write (planet_fd, planet_base, byte_size);
    if (n != byte_size)
    {
	fprintf (stderr, "\nCannot write planet data to disk!\n\n");
	exit (-1);
    }

    close (planet_fd);
}
