/* This program will generate a completely new galaxy and write the results
   to files "galaxy.dat", "stars.dat" and "planets.dat". */


#define THIS_IS_MAIN

#include "fh.h"


int	species_number;

struct galaxy_data	galaxy;

extern unsigned long	last_random;


main (argc, argv)

int argc;
char *argv[];

{
    int		d, i, n, galaxy_file, star_file, planet_file, using_defaults;

    int		x, y, z, real_x, real_y, real_z, desired_num_stars,
		galactic_diameter, sq_distance_from_center;

    int		st_index, pl_index, star_type, star_color, star_size,
		d_num_species, star_num_planets;

    int		num_stars, galactic_radius, num_planets, max_planets,
		num_wormholes;

    long	volume, l_radius, chance_of_star, num_bytes, star_data_size,
		planet_data_size, dx, dy, dz, distance_squared;

    char	*cp, star_here[MAX_DIAMETER][MAX_DIAMETER];

    struct star_data	*current_star, *star_base, *star, *worm_star;
    struct planet_data	*planet_base, *planet;


    /* Check for valid command line. */
    if (argc == 1)
	using_defaults = FALSE;
    else if (argc == 2  &&  argv[1][0] != '?')
    {
	d_num_species = atoi (argv[1]);
	using_defaults = TRUE;
    }
    else
    {
	fprintf (stderr, "\n  Usage: NewGalaxy [num_species]\n\n");
	fprintf (stderr, "  This program will create files 'galaxy.dat', 'stars.dat' and 'planets.dat'.\n");
	fprintf (stderr, "  If num_species is given, then defaults will be used for everything else.\n\n");
	exit (0);
    }

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) rnd(10);

    /* Get number of species. */
  again1:
    if (! using_defaults)
    {
	printf ("\nHow many species will be in the game? "); fflush (stdout);
	scanf ("%d", &d_num_species);
    }
    if (d_num_species < MIN_SPECIES || d_num_species > MAX_SPECIES)
    {
	printf ("\n  A game must have between %d and %d species, inclusive. Try again.\n",
		MIN_SPECIES, MAX_SPECIES);
	using_defaults = FALSE;
	goto again1;
    }

    /* Get approximate number of star systems to generate. */
    n = (d_num_species * STANDARD_NUMBER_OF_STAR_SYSTEMS)
		/ STANDARD_NUMBER_OF_SPECIES;
  again2:
    if (using_defaults)
    {
	printf ("For %d species, there should be about %d stars.\n",
			d_num_species, n);
	desired_num_stars = n;
    }
    else
    {
	printf ("\nFor %d species, a game needs about %d star systems.\n",
	    d_num_species, n);
	printf ("Approximately how many star systems do you want me to generate? ");
	fflush (stdout);
	scanf ("%d", &desired_num_stars);
    }
    if (desired_num_stars < MIN_STARS || desired_num_stars > MAX_STARS)
    {
	printf ("\n  A game must have between %d and %d star systems, inclusive. Try again.\n",
		MIN_STARS, MAX_STARS);
	using_defaults = FALSE;
	goto again2;
    }

    /* Get size of galaxy to generate. Use long values to prevent loss of
	data by compilers that use 16-bit ints. */
    volume = (long) desired_num_stars
		* (long) STANDARD_GALACTIC_RADIUS
		* (long) STANDARD_GALACTIC_RADIUS
		* (long) STANDARD_GALACTIC_RADIUS
			/ (long) STANDARD_NUMBER_OF_STAR_SYSTEMS;
    l_radius = 2L;
    while (l_radius * l_radius * l_radius < volume) ++l_radius;
  again3:
    galactic_radius = l_radius;
    if (! using_defaults)
    {
	printf ("\nFor %d stars, the galaxy should have a radius of about %d parsecs.\n",
	    desired_num_stars, galactic_radius);
	printf ("What radius (in parsecs) do you want the galaxy to have? ");
	fflush (stdout);
	scanf ("%d", &galactic_radius);
    }
    if (galactic_radius < MIN_RADIUS || galactic_radius > MAX_RADIUS)
    {
	printf ("\n  Radius must be between %d and %d parsecs, inclusive. Try again.\n",
		MIN_RADIUS, MAX_RADIUS);
	using_defaults = FALSE;
	goto again3;
    }
    galactic_diameter = 2 * galactic_radius;

    /* Get the number of cubic parsecs within a sphere with a radius of
	galactic_radius parsecs. Again, use long values to prevent loss
	of data by compilers that use 16-bit ints. */
    volume = (4L * 314L	* (long) galactic_radius
			* (long) galactic_radius
			* (long) galactic_radius)
				/ 300L;

    /* The probability of a star system existing at any particular
	set of x,y,z coordinates is one in chance_of_star. */
    chance_of_star = volume / (long) desired_num_stars;
    if (chance_of_star < 50)
    {
	printf ("\n  Galactic radius is too small for %d stars. Please try again.\n\n",
		desired_num_stars);
	goto again1;
    }
    if (chance_of_star > 3200)
    {
	printf ("\n  Galactic radius is too large for %d stars. Please try again.\n\n",
		desired_num_stars);
	goto again1;
    }

    /* Initialize star location data. */
    for (x = 0; x < galactic_diameter; x++)
     for (y = 0; y < galactic_diameter; y++)
	star_here[x][y] = -1;

    /* Get locations of stars. */
    num_stars = 0;
    while (num_stars < desired_num_stars)
    {
	x = rnd (galactic_diameter) - 1;
	y = rnd (galactic_diameter) - 1;
	z = rnd (galactic_diameter) - 1;

	real_x = x - galactic_radius;
	real_y = y - galactic_radius;
	real_z = z - galactic_radius;

	sq_distance_from_center =
	    (real_x * real_x) + (real_y * real_y) + (real_z * real_z);
	if (sq_distance_from_center >= galactic_radius*galactic_radius)
	    continue;

	/* Coordinate is within the galactic boundary. */
	if (star_here[x][y] < 0)
	{
	    star_here[x][y] = z;	/* z-coordinate. */
	    ++num_stars;
	    if (num_stars == MAX_STARS) break;
	}
    }

    /* Create output file for galaxy. */
    galaxy_file = creat ("galaxy.dat", 0600);
    if (galaxy_file < 0)
    {
	fprintf (stderr, "\n  Cannot create file galaxy.dat!\n");
	exit (-1);
    }

    galaxy.d_num_species = d_num_species;
    galaxy.num_species = 0;
    galaxy.radius = galactic_radius;
    galaxy.turn_number = 0;

    num_bytes = write (galaxy_file, &galaxy, sizeof (struct galaxy_data));
    if (num_bytes != sizeof (struct galaxy_data))
    {
	fprintf (stderr, "\n\tCannot write data to file 'galaxy.dat'!\n\n");
	exit (-1);
    }

    close (galaxy_file);

    /* Allocate enough memory for star and planet data. */
    star_data_size = (long) num_stars * (long) sizeof(struct star_data);
    star_base = (struct star_data *) malloc (star_data_size);
    if (star_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for star data!\n\n");
	exit (-1);
    }

    max_planets = 9 * num_stars;	/* Maximum number possible. */
    planet_data_size = (long) max_planets * (long) sizeof(struct planet_data);
    planet_base = (struct planet_data *) malloc (planet_data_size);
    if (planet_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for planet data!\n\n");
	exit (-1);
    }

    fprintf (stdout, "\nGenerating star number     ");  fflush (stdout);

    st_index = 0;
    pl_index = 0;
    star = star_base;
    planet = planet_base;
    for (x = 0; x < galactic_diameter; x++)
     for (y = 0; y < galactic_diameter; y++)
    {
	if (star_here[x][y] >= 0)
	{
	    /* Initialize all bytes of record to zero. */
	    cp = (char *) star;
	    for (i = 0; i < sizeof (struct star_data); i++)
		*cp++ = 0;

	    /* Set coordinates. */
	    star->x = x;
	    star->y = y;
	    star->z = star_here[x][y];

	    /* Determine type of star. Make MAIN_SEQUENCE the most common
		star type. */
	    star_type = rnd(GIANT+6);
	    if (star_type > GIANT) star_type = MAIN_SEQUENCE;
	    star->type = star_type;

	    /* Color and size of star are totally random. */
	    star_color = rnd(RED);
	    star->color = star_color;
	    star_size = rnd(10) - 1;
	    star->size = star_size;

	    /* Determine the number of planets in orbit around the star.
		The algorithm is something I tweaked until I liked it.
		It's weird, but it works. */
	    d = RED + 2 - star_color;
		/* Size of die. Big stars (blue, blue-white) roll bigger
		   dice. Smaller stars (orange, red) roll smaller dice. */
	    n = star_type; if (n > 2) n -= 1;
		/* Number of rolls: dwarves have 1 roll, degenerates and
		   main sequence stars have 2 rolls, and giants have 3 rolls. */
	    star_num_planets = -2;
	    for (i=1; i<=n; i++) star_num_planets += rnd(d);
	    while (star_num_planets > 9)	/* Trim down if too many. */
		star_num_planets -= rnd(3);
	    if (star_num_planets < 1) star_num_planets = 1;
	    star->num_planets = star_num_planets;

	    /* Determine pl_index of first planet in file "planets.dat". */
	    star->planet_index = pl_index;

	    /* Generate planets and write to file "planets.dat". */
	    current_star = star;
	    generate_planets (planet, star_num_planets);

	    star->home_system = FALSE;
	    star->worm_here = FALSE;

	    /* Update pointers and indices. */
	    pl_index += star_num_planets;
	    planet += star_num_planets;
	    ++st_index;
	    ++star;

	    if (st_index % 10  ==  0)
		fprintf (stdout, "\b\b\b\b%4d", st_index);  fflush (stdout);
	}
    }

    fprintf (stdout, "\b\b\b\b%4d\n", st_index);

    /* Allocate natural wormholes. */
    num_wormholes = 0;
    star = star_base - 1;
    for (i = 0; i < num_stars; i++)
    {
	++star;

	if (star->home_system) continue;
	if (star->worm_here) continue;

	if (rnd(100) < 92) continue;

	/* There is a wormhole here. Get coordinates of other end. */
	while (TRUE)
	{
	    n = rnd(num_stars);
	    worm_star = star_base + n - 1;
	    if (worm_star == star) continue;
	    if (worm_star->home_system) continue;
	    if (worm_star->worm_here) continue;

	    break;
	}

	/* Eliminate wormholes less than 20 parsecs in length. */
	dx = (long) star->x - (long) worm_star->x;
	dy = (long) star->y - (long) worm_star->y;
	dz = (long) star->z - (long) worm_star->z;
	distance_squared = (dx * dx)  +  (dy * dy)  +  (dz * dz);
	if (distance_squared < 400) continue;

	star->worm_here = TRUE;
	star->worm_x = worm_star->x;
	star->worm_y = worm_star->y;
	star->worm_z = worm_star->z;

	worm_star->worm_here = TRUE;
	worm_star->worm_x = star->x;
	worm_star->worm_y = star->y;
	worm_star->worm_z = star->z;

	++num_wormholes;
    }

    /* Create output file for stars. */
    star_file = creat ("stars.dat", 0600);
    if (star_file < 0)
    {
	fprintf (stderr, "\n  Cannot create file stars.dat!\n");
	exit (-1);
    }

    num_bytes = write (star_file, &num_stars, sizeof(num_stars));
    if (num_bytes != sizeof(num_stars))
    {
	fprintf (stderr, "\n  Cannot write star count to file stars.dat!\n");
	exit (-1);
    }

    /* Write the result to the file "stars.dat". */
    num_bytes = write (star_file, star_base, num_stars * sizeof(struct star_data));
    if (num_bytes != num_stars * sizeof(struct star_data))
    {
	fprintf (stderr, "\n  Cannot write star data to file stars.dat!\n");
	exit (-1);
    }

    close (star_file);

    num_planets = pl_index;
    printf ("\nThis galaxy contains a total of %d stars and ", num_stars);
    printf ("%d planets.\n", num_planets);
    printf ("  The galaxy contains %d natural wormholes.\n\n", num_wormholes);

    if (st_index != num_stars)
	fprintf (stderr, "\n  Internal consistency check #1 failed!!!\n\n");

    /* Create output file for planets. */
    planet_file = creat ("planets.dat", 0600);
    if (planet_file < 0)
    {
	fprintf (stderr, "\n  Cannot create file planets.dat!\n");
	exit (-1);
    }

    /* Write planet data to file "planets.dat". */
    num_bytes = write (planet_file, &num_planets, sizeof(num_planets));
    if (num_bytes != sizeof(num_planets))
    {
	fprintf (stderr, "\n  Cannot write number of planets to file planets.dat!\n");
	exit (-1);
    }

    planet = planet_base;
    for (i = 0; i < num_planets; i++)
    {
	num_bytes = write (planet_file, planet, sizeof(struct planet_data));
	if (num_bytes != sizeof(struct planet_data))
	{
	    fprintf (stderr, "\n  Cannot write planet data to file planets.dat!\n");
	    exit (-1);
	}
	++planet;
    }

    close (planet_file);

    free (star_base);
    free (planet_base);

    exit (0);
}
