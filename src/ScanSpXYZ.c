
#define	THIS_IS_MAIN

#include "fh.h"


int	species_number;

struct galaxy_data	galaxy;
struct star_data	*star;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern FILE		*log_file;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    char	x, y, z;

    struct species_data		sp;


    /* Check for valid command line. */
    if (argc != 5)
    {
	fprintf (stderr, "\n\tUsage: ScanSpXYZ species_number x y z\n\n");
	exit (-1);
    }

    species_number = atoi (argv[1]);
    x = atoi (argv[2]);
    y = atoi (argv[3]);
    z = atoi (argv[4]);
    log_file = stdout;

    /* Get commonly used data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();

    /* Read data for this species from disk. */
    if (! get_spec_data (species_number, &sp, FALSE, FALSE))
    {
	fprintf (stderr, "\n\n\tCannot open data file for species #%d!\n\n",
	    species_number);
	exit (-1);
    }

    /* Display scan. */
    fprintf (log_file, "Scan for SP %s:\n", sp.name);
    species = &sp;
    scan (x, y, z);

    /* Release memory used for this species. */
    if (sp.num_ships > 0) free (ship_base);
    free (nampla_base);

    exit (0);
}



/* The following routine will read species data from disk and return
   TRUE if it succeeds. If the file does not exist, it will return FALSE. */

int get_spec_data (spec_number, spec,
			need_extra_namplas, need_extra_ships)

int			spec_number, need_extra_namplas, need_extra_ships;
struct species_data	*spec;

{
    int		spec_fd, extra_namplas, extra_ships;

    long	n, num_bytes;

    char	filename[16];


    /* First, check if extras are needed. */
    if (need_extra_namplas)
	extra_namplas = NUM_EXTRA_NAMPLAS;
    else
	extra_namplas = 0;

    if (need_extra_ships)
	extra_ships = NUM_EXTRA_SHIPS;
    else
	extra_ships = 0;

    /* Open the species data file. */
    sprintf (filename, "sp%02d.dat\0", spec_number);
    spec_fd = open (filename, 0);
    if (spec_fd < 0)
    {
	spec->pn = 0;	/* Extinct! */
	return FALSE;
    }

    /* Read in species data. */
    num_bytes = read (spec_fd, spec, sizeof(struct species_data));
    if (num_bytes != sizeof(struct species_data))
    {
	fprintf (stderr, "\n\tCannot read species record in file '%s'!\n\n",
		filename);
	exit (-1);
    }

    /* Allocate enough memory for all namplas. */
    num_bytes = (spec->num_namplas + extra_namplas) * sizeof (struct nampla_data);
    nampla_base = (struct nampla_data *) malloc (num_bytes);
    if (nampla_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for nampla data!\n\n");
	exit (-1);
    }

    /* Read it all into memory. */
    num_bytes = spec->num_namplas * sizeof (struct nampla_data);
    n = read (spec_fd, nampla_base, num_bytes);
    if (n != num_bytes)
    {
	fprintf (stderr, "\nCannot read nampla data into memory!\n\n");
	exit (-1);
    }

    /* Allocate enough memory for all ships. */
    if ((spec->num_ships + extra_ships)  >  0)
    {
 	num_bytes = (spec->num_ships + extra_ships) * sizeof (struct ship_data);
	ship_base = (struct ship_data *) malloc (num_bytes);
	if (ship_base == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for ship data!\n\n");
	    exit (-1);
	}
    }

    if (spec->num_ships > 0)
    {
	/* Read it all into memory. */
	num_bytes = (long) spec->num_ships * sizeof (struct ship_data);
	n = read (spec_fd, ship_base, num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\nCannot read ship data into memory!\n\n");
	    exit (-1);
	}
    }

    close (spec_fd);

    return TRUE;
}
