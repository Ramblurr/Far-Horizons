
/* This program will create the file locations.dat and will update the
economic efficiencies of all planets.  These functions are also performed
by Finish.c.  This program should be run before the strike phase or whenever
manual changes are made to the species data files that resulted in something
not being where it was or something being where it was not. It should also
be run if you run Finish on fewer than all species and decide to keep the
resulting planets.dat file. */


#define THIS_IS_MAIN

#include "fh.h"


int			species_number, species_index;
int			test_mode, verbose_mode;

struct galaxy_data	galaxy;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern int		num_locs, num_planets;

extern struct sp_loc_data	loc[MAX_LOCATIONS];
extern struct planet_data	*planet_base;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, nampla_index;

    long	diff, total, *total_econ_base;


    /* Check for options, if any. */
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-t") == 0) test_mode = TRUE;
      if (strcmp (argv[i], "-v") == 0) verbose_mode = TRUE;
    }

    /* Get commonly used data. */
    get_galaxy_data ();
    get_planet_data ();
    get_species_data ();

    /* Allocate memory for array "total_econ_base". */
    total = (long) num_planets * sizeof (long);
    total_econ_base = (long *) malloc (total);
    if (total_econ_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for total_econ_base!\n\n");
	exit (-1);
    }

    /* Initialize total econ base for each planet. */
    planet = planet_base;
    for (i = 0; i < num_planets; i++)
    {
	total_econ_base[i] = 0;

	++planet;
    }

    /* Get total economic base for each planet from nampla data. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	if (! data_in_memory[species_number - 1]) continue;

	data_modified[species_number - 1] = TRUE;

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];

	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    nampla = nampla_base + nampla_index;

	    if (nampla->pn == 99) continue;

	    if ((nampla->status & HOME_PLANET) == 0)
		total_econ_base[nampla->planet_index] +=
			nampla->mi_base + nampla->ma_base;
	}
    }

    /* Update economic efficiencies of all planets. */
    planet = planet_base;
    for (i = 0; i < num_planets; i++)
    {
	total = total_econ_base[i];
	diff = total - 2000;

	if (diff <= 0)
	    planet->econ_efficiency = 100;
	else
	    planet->econ_efficiency = (100 * (diff/20 + 2000)) / total;

	++planet;
    }

    /* Create new locations array. */
    do_locations ();

    /* Clean up and exit. */
    save_location_data ();
    save_planet_data ();
    free_species_data ();
    free (planet_base);
    exit (0);
}
