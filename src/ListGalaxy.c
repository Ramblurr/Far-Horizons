
/* This program will list all of the planets in the galaxy using the data
   in the files "galaxy.dat" and "planets.dat". If the -p option is
   specified, planets will not be listed. If the -w option is specified,
   only wormhole locations will be listed. */


#define THIS_IS_MAIN

#include "fh.h"


int	species_number;

struct galaxy_data	galaxy;

extern int	num_stars, num_planets;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, n, star_index, total_planets, list_planets, usage_error,
		type_count[10], num_gases, home_system, total_wormstars,
		list_wormholes;

    struct star_data	*star, *worm_star;
    struct planet_data	*planet, *home_planet;


    /* Check for valid command line. */
    usage_error = FALSE;
    list_planets = FALSE;
    list_wormholes = FALSE;
    if (argc == 2)
    {
	if (argv[1][0] == '-'  &&  argv[1][1] == 'p')
	    list_planets = FALSE;
	else if (argv[1][0] == '-'  &&  argv[1][1] == 'w')
	    list_wormholes = TRUE;
	else
	    usage_error = TRUE;
    }
    else
	list_planets = TRUE;

    if (usage_error || argc > 2)
    {
	fprintf (stderr, "\n  Usage: ListPlanets [-p | -w]\n");
	fprintf (stderr, "\tUse -p option to NOT list planets.\n");
	fprintf (stderr, "\tUse -w option to list only wormholes.\n\n");
	exit (-1);
    }

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();

    /* Initialize counts. */
    for (i = DWARF; i <= GIANT; i++) type_count[i] = 0;
    total_planets = 0;
    total_wormstars = 0;

    /* For each star, list info. */
    star = (struct star_data *) star_base;
    planet = (struct planet_data *) planet_base;
    for (star_index = 0; star_index < num_stars; star_index++)
    {
	if (! list_wormholes)
	{
	    if (list_planets)
		printf ("System #%d:\t", star_index + 1);
	    printf ("x = %d\ty = %d\tz = %d", star->x, star->y, star->z);
	    printf ("\tstellar type = %c%c%c", type_char[star->type],
		color_char[star->color], size_char[star->size]);
	    if (list_planets)
		printf ("\t%d planets.", star->num_planets);
	    printf ("\n");

	    if (star->num_planets == 0)
	    {
		printf ("\tStar #%d went nova!", star_index + 1);
		printf (" All planets were blown away!\n");
	    }
	}

	total_planets += star->num_planets;
	type_count[star->type] += 1;

	if (star->worm_here)
	{
	    ++total_wormstars;
	    if (list_planets)
		printf ("!!! Natural wormhole from here to %d %d %d\n",
		    star->worm_x, star->worm_y, star->worm_z);
	    else if (list_wormholes)
	    {
		printf ("Wormhole #%d: from %d %d %d to %d %d %d\n",
			total_wormstars, star->x, star->y, star->z,
			star->worm_x, star->worm_y, star->worm_z);
		worm_star = (struct star_data *) star_base;
		for (i = 0; i < num_stars; i++)
		{
		    if (star->worm_x == worm_star->x
			&&  star->worm_y == worm_star->y
			&&  star->worm_z == worm_star->z)
		    {
			worm_star->worm_here = FALSE;
			break;
		    }
		    ++worm_star;
		}
	    }
	}

	home_system = FALSE;
	home_planet = planet;
	if (list_planets)	/* Check if system has a home planet. */
	  for (i = 1; i <= star->num_planets; i++)
	  {
	    if (home_planet->special == 1  ||  home_planet->special == 2)
	    {
		home_system = TRUE;
		break;
	    }
	    ++home_planet;
	  }

	if (list_planets)
	  for (i = 1; i <= star->num_planets; i++)
	{
	    switch (planet->special)
	    {
		case 0:	printf ("     "); break;
		case 1:	printf (" HOM "); break;  /* Ideal home planet. */
		case 2: printf (" COL "); break;  /* Ideal colony planet. */
	    }
	    printf ("#%d dia=%3d g=%d.%02d tc=%2d pc=%2d md=%d.%02d", i,
		planet->diameter,
		planet->gravity/100,
		planet->gravity%100,
		planet->temperature_class,
		planet->pressure_class,
		planet->mining_difficulty/100,
		planet->mining_difficulty%100);

	    if (home_system)
		print_LSN (planet, home_planet);
	    else
		printf ("  ");

	    num_gases = 0;
	    for (n = 0; n < 4; n++)
	    {
		if (planet->gas_percent[n] > 0)
		{
		    if (num_gases > 0) printf (",");
		    printf ("%s(%d%%)", gas_string[planet->gas[n]],
			    planet->gas_percent[n]);
		    ++num_gases;
		}
	    }

	    if (num_gases == 0) printf ("No atmosphere");

	    printf ("\n");
	    ++planet;
	}

	if (list_planets) printf ("\n");

	++star;
    }

    if (list_wormholes) goto done;

    /* Print summary. */
    printf ("\nThe galaxy has a radius of %d parsecs.\n", galaxy.radius);
    printf ("It contains %d dwarf stars, %d degenerate stars, ",
	type_count[DWARF], type_count[DEGENERATE]);
    printf ("%d main sequence stars,\n    and %d giant stars, ",
	type_count[MAIN_SEQUENCE], type_count[GIANT]);
    printf ("for a total of %d stars.\n", num_stars);

    if (! list_planets) goto done;

    printf ("The total number of planets in the galaxy is %d.\n", total_planets);
    printf ("The total number of natural wormholes in the galaxy is %d.\n",
	total_wormstars/2);
    printf ("The galaxy was designed for %d species.\n", galaxy.d_num_species);
    printf ("A total of %d species have been designated so far.\n\n",
		galaxy.num_species);

done:

    /* Internal test. */
    if (num_planets != total_planets)
    {
	fprintf (stderr, "\n\nWARNING!  Program error!  Internal inconsistency!\n\n");
	exit (-1);
    }

    /* Clean up and exit. */
    exit (0);
}


print_LSN (planet, home_planet)

struct planet_data	*planet, *home_planet;

{
    /* This routine provides an approximate LSN for a planet. It assumes
	that oxygen is required and any gas that does not appear on the
	home planet is poisonous. */

    int		i, j, k, ls_needed, poison;

    ls_needed = 0;

    j = planet->temperature_class - home_planet->temperature_class;
    if (j < 0) j = -j;
    ls_needed = 2 * j;		/* Temperature class. */

    j = planet->pressure_class - home_planet->pressure_class;
    if (j < 0) j = -j;
    ls_needed += 2 * j;		/* Pressure class. */

    /* Check gases. Assume oxygen is not present. */
    ls_needed += 2;
    for (j = 0; j < 4; j++)	/* Check gases on planet. */
    {
	if (planet->gas[j] == 0) continue;

	if (planet->gas[j] == O2) ls_needed -= 2;

	poison = TRUE;
	for (k = 0; k < 4; k++)	/* Compare with home planet. */
	{
	    if (planet->gas[j] == home_planet->gas[k])
	    {
		poison = FALSE;
		break;
	    }
	}
	if (poison) ls_needed += 2;
    }

    printf ("%4d ", ls_needed);
}
