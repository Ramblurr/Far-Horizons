
/* This is a modified non-interactive version of HomeSystem */



#define THIS_IS_MAIN

#include "fh.h"


int			x, y, z, species_number;

long			min_distance, min_d_squared;

struct galaxy_data	galaxy;
struct star_data	*star;
struct planet_data	*planet, *first_planet;

extern int		num_stars, num_planets;
extern unsigned long	last_random;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, found, home_system_fd, home_planet_number,
		num_home_systems;

    long	n, num_bytes;

    char	filename[32], answer[16];

    /* Check for valid command line. */
    min_distance = 10;
    if (argc == 1)
	;
    else if (argc == 2)
	min_distance = atoi (argv[1]);
    else
    {
	fprintf (stderr, "\n  Usage: HomeSystem min_distance or HomeSystem [x y z]\n");
	fprintf (stderr, "\t\t (default min_distance is 10 parsecs)\n\n");
	exit (-1);
    }
    min_d_squared = min_distance * min_distance;

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) rnd(10);

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_random_xyz ();

    /* Open file HSn, where n is the number of planets in the system. We will
	overwrite the existing planet data with the data in the file. */
    sprintf (filename, "HS%d\0", star->num_planets);
    home_system_fd = open (filename, 0);
    if (home_system_fd < 0)
    {
	fprintf (stderr, "\n\tFile '%s' does not exist!\n\n", filename);
	exit (-1);
    }

    first_planet = planet_base + (long) star->planet_index;

    num_bytes = (long) star->num_planets * sizeof (struct planet_data);
    n = read (home_system_fd, first_planet, num_bytes);
    if (n != num_bytes)
    {
	fprintf (stderr, "\n\tCannot read home system data in file '%s'!\n\n",
	    	filename);
	exit (-1);
    }

    /* Make minor random modifications to data for planets in this system. */
    planet = first_planet;
    home_planet_number = 999;
    for (i = 0; i < star->num_planets; i++)
    {
	if (planet->special == 1) home_planet_number = i + 1;

	if (planet->temperature_class == 0)
	    ;
	else if (planet->temperature_class > 12)
	     planet->temperature_class -= rnd(3) - 1;
	else
	     planet->temperature_class += rnd(3) - 1;

	if (planet->pressure_class == 0)
	    ;
	else if (planet->pressure_class > 12)
	    planet->pressure_class -= rnd(3) - 1;
	else
	    planet->pressure_class += rnd(3) - 1;

	if (planet->gas[2] > 0)
	{
	    j = rnd(25) + 10;
	    if (planet->gas_percent[2] > 50)
	    {
		planet->gas_percent[1] += j;
		planet->gas_percent[2] -= j;
	    }
	    else if (planet->gas_percent[1] > 50)
	    {
		planet->gas_percent[1] -= j;
		planet->gas_percent[2] += j;
	    }
	}

	if (planet->diameter > 12)
	    planet->diameter -= rnd(3) - 1;
	else
	    planet->diameter += rnd(3) - 1;

	if (planet->gravity > 100)
	    planet->gravity -= rnd(10);
	else
	    planet->gravity += rnd(10);

	if (planet->mining_difficulty > 100)
	    planet->mining_difficulty -= rnd(10);
	else
	    planet->mining_difficulty += rnd(10);

	++planet;
    }

    star->home_system = TRUE;

    /* Count number of home systems. */
    star = star_base;
    num_home_systems = 0;
    for (i = 0; i < num_stars; i++)
    {
	if (star->home_system) ++num_home_systems;

	++star;
    }

    /* Save data and clean up. */
    save_star_data ();
    save_planet_data ();
    free (star_base);
    free (planet_base);

    fprintf (stdout, "%d %d %d %d", x, y, z, home_planet_number);

    exit (0);
}



get_random_xyz ()

{
    int		i, found;

    long	n, max_checks, dx, dy, dz, d_sq;

    struct star_data	*star2;


    n = 0;
    max_checks = 25L * num_stars;

    while (1)
    {
	if (n++ > max_checks)
	{
	    fprintf (stderr, "\n\tIt appears that all suitable systems are within %ld parsecs of an\n", min_distance);
	    fprintf (stderr, "\texisting home system. You'll have to select a system manually.\n\n");
	    exit (-1);
	}

	i = rnd(num_stars);
	star = star_base + (i - 1);

	if (star->home_system) continue;
	if (star->worm_here) continue;
	if (star->num_planets < 3) continue;

	x = star->x;
	y = star->y;
	z = star->z;

	/* See if there is another home system within 'min-distance' parsecs. */
	found = FALSE;
	star2 = star_base - 1;
	for (i = 0; i < num_stars; i++)
	{
	    ++star2;

	    if (star2->home_system == FALSE) continue;

	    dx = (long) x - (long) star2->x;
	    dy = (long) y - (long) star2->y;
	    dz = (long) z - (long) star2->z;

	    d_sq = dx * dx  +  dy * dy  + dz * dz;

	    if (d_sq < min_d_squared)
	    {
		found = TRUE;
		break;
	    }
	}

	if (! found) return;
    }
}
