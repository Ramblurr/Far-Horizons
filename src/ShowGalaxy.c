
/* This program will display a crude ASCII map of the galaxy intended to
   show the gamemaster the relative positions of home planets, ideal colonies
   and all other star systems. The gamemaster should run this program after
   running NewGalaxy to make sure that the distribution is not too lopsided.
   If it is, run NewGalaxy again until satisfied. In the display, "H" stands
   for ideal home planet, "C" stands for ideal colony, and "S" is used for
   all other stars. */


#define THIS_IS_MAIN

#include "fh.h"


int	species_number, star_here[MAX_DIAMETER][MAX_DIAMETER];

struct galaxy_data	galaxy;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;

extern int	num_stars, num_planets;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, n, x, y, z, galactic_diameter, star_index, special;

    long	size, star_data_size, planet_data_size;

    struct star_data	*star;
    struct planet_data	*planet;

    /* Check for valid command line. */
    if (argc != 1)
    {
	fprintf (stderr, "\n\tUsage: ShowGalaxy\n\n");
	exit (-1);
    }

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();

    galactic_diameter = 2 * galaxy.radius;

    /* For each star, set corresponding element of star_here[] to index
	into star array. */
    for (x = 0; x < galactic_diameter; x++)	/* Initialize array. */
     for (y = 0; y < galactic_diameter; y++)
	star_here[x][y] = -1;

    star = star_base;
    for (star_index = 0; star_index < num_stars; star_index++)
    {
	x = star->x;
	y = star->y;
	star_here[x][y] = star_index;
	++star;
    }

    for (i = 0; i < galactic_diameter; i++)
	printf ("-");
    printf ("\n");

    /* Outermost loop will control y-coordinates. */
    for (y = galactic_diameter - 1; y >= 0; y--)
    {
	/* Innermost loop will control x-coordinate. */
	for (x = 0; x <= galactic_diameter; x++)
	{
	    if (x == galactic_diameter)
	    {
		printf ("|\n");
		continue;
	    }

	    star_index = star_here[x][y];
	    if (star_index == -1)
	    {
		printf (" ");
		continue;
	    }

	    star = star_base;
	    star += star_index;

	    planet = planet_base + (long) star->planet_index;
	    for (i = 0; i < star->num_planets; i++)
	    {
		special = planet->special;
		if (special != 0) break;

		++planet;
	    }

	    switch (special)
	    {
		case 0:	printf ("S"); break;
		case 1:	printf ("H"); break;
		case 2:	printf ("C"); break;
		default: printf ("%d", planet->special);
	    }
	}
    }

    for (i = 0; i < galactic_diameter; i++)
	printf ("-");
    printf ("\n");

    /* Clean up and exit. */
    exit (0);
}
