
#define	THIS_IS_MAIN

#include "fh.h"


int		species_number;

extern int ignore_field_distorters;

extern FILE			*log_file;
struct galaxy_data		galaxy;
struct species_data		*species;
struct nampla_data		*nampla_base;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, species_index, species_printed, nampla_index,
		ship_index, industry;

    long	x, y, z, max_distance, max_distance_squared,
		delta_x, delta_y, delta_z, distance_squared;

    char	answer[256];

    struct nampla_data		*nampla;
    struct ship_data		*ship;


    ignore_field_distorters = TRUE;
    log_file = stdout;

    /* Check for valid command line. */
    if (argc == 2)
	max_distance = (long) atoi (argv[1]);
    else if (argc == 5)
    {
	x = (long) atoi (argv[1]);
	y = (long) atoi (argv[2]);
	z = (long) atoi (argv[3]);
 	max_distance = (long) atoi (argv[4]);
    }
    else
    {
	fprintf (stderr, "\n\tUsage: Near [x y z] distance\n\n");
	exit (-1);
    }

    max_distance_squared = max_distance * max_distance;

    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_species_data ();

again:

    if (argc == 2)
    {
	printf ("Enter x y z OR sn to scan for species n (-1 to quit): ");
	fflush (stdout);
	fgets (answer, sizeof (answer), stdin);
	i = sscanf (answer, "%d", &x);

	if (i == 0)
	{
	    i = sscanf (answer, "s%d", &species_number);
	    if (i == 1)
	    {
		species_index = species_number - 1;
		species = &spec_data[species_index];
		nampla_base = namp_data[species_index];
		printf ("Scan for SP %s:\n", species->name);

		scan (x, y, z);
	    }
	    goto again;
	}

	if (x < 0) goto done;

	sscanf (answer, "%d %d %d", &x, &y, &z);
    }

    /* Display scan. */
    printf ("Ships and populated planets within %ld parsecs of %ld %ld %ld:\n",
	max_distance, x, y, z);

    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (! data_in_memory[species_index]) continue;

	species_number = species_index + 1;
	species = &spec_data[species_index];

	species_printed = FALSE;

	/* Set dest_x for all ships to zero. We will use this to prevent
		multiple listings of a ship. */
	ship = ship_data[species_index];
	for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	{
		ship->dest_x = 0;
		++ship;
	}

	/* Check all namplas for this species. */
	nampla = namp_data[species_index] - 1;
	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    ++nampla;

	    if ((nampla->status & POPULATED) == 0) continue;

	    delta_x = x - nampla->x;
	    delta_y = y - nampla->y;
	    delta_z = z - nampla->z;
	    distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
		+ (delta_z * delta_z);

	    if (distance_squared > max_distance_squared) continue;

	    if (! species_printed)
	    {
		printf ("  Species #%d, SP %s:\n", species_number,
				species->name);
		species_printed = TRUE;
	    }

	    printf ("    %2d %2d %2d #%d", nampla->x, nampla->y, nampla->z,
		nampla->pn);

	    if (nampla->status & HOME_PLANET)
		printf ("  Home planet");
	    else if (nampla->status & MINING_COLONY)
		printf ("  Mining colony");
	    else if (nampla->status & RESORT_COLONY)
		printf ("  Resort colony");
	    else
		printf ("  Normal colony");

	    printf (" PL %s, ", nampla->name);

	    industry = nampla->mi_base + nampla->ma_base;
	    printf ("EB = %d.%d", industry/10, industry%10);

	    printf (", %d Yrds", nampla->shipyards);

	    for (i = 0; i < MAX_ITEMS; i++)
	    {
		if (nampla->item_quantity[i] > 0)
		    printf (", %d %s", nampla->item_quantity[i], item_abbr[i]);
	    }

	    if (nampla->hidden) printf (", HIDING!");

	    printf ("\n");

	    /* List ships at this planet. */
	    ship = ship_data[species_index] - 1;
	    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	    {
		++ship;

		if (ship->dest_x != 0) continue;	/* Already listed. */

		if (ship->x != nampla->x) continue;
		if (ship->y != nampla->y) continue;
		if (ship->z != nampla->z) continue;
		if (ship->pn != nampla->pn) continue;

		printf ("                 %s", ship_name (ship));

		for (i = 0; i < MAX_ITEMS; i++)
		{
		    if (ship->item_quantity[i] > 0)
			printf (", %d %s", ship->item_quantity[i],
				item_abbr[i]);
		}

		printf ("\n");

		ship->dest_x = 99;  /* Do not list this ship again. */
	    }
	}

	ship = ship_data[species_index] - 1;
	for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;

	    if (ship->dest_x != 0) continue;	/* Already listed above. */

	    delta_x = x - ship->x;
	    delta_y = y - ship->y;
	    delta_z = z - ship->z;
	    distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
		+ (delta_z * delta_z);

	    if (distance_squared > max_distance_squared) continue;

	    if (! species_printed)
	    {
		printf ("  Species #%d, SP %s:\n", species_number,
				species->name);
		species_printed = TRUE;
	    }

	    printf ("    %2d %2d %2d", ship->x, ship->y, ship->z);

	    printf ("     %s", ship_name(ship));

	    for (i = 0; i < MAX_ITEMS; i++)
	    {
		if (ship->item_quantity[i] > 0)
		    printf (", %d %s", ship->item_quantity[i],
			item_abbr[i]);
	    }

	    printf ("\n");
	}
    }

    if (argc == 2) goto again;

done:

    /* Clean up and exit. */
    free_species_data ();
    exit (0);
}
