
/*
	This program performs the reverse operation of BinaryToAscii.
*/

#define THIS_IS_MAIN

#include "fh.h"


int			num_species, species_index, species_number;

struct galaxy_data	galaxy;
struct species_data	*species;
struct star_data	*star;
struct planet_data	*planet;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern int			num_stars, num_planets;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, n, galaxy_fd;

    char	*tp;

    FILE	*ascii_file;


    /* Open file 'ascii.dat'. */
    ascii_file = fopen ("ascii.dat", "r");
    if (ascii_file == NULL)
    {
	fprintf (stderr, "\n\tCannot open 'ascii.dat' for reading!\n\n");
	exit (-1);
    }

    /* Do galaxy data. */
    printf ("Doing galaxy data...\n");
    fscanf (ascii_file, "%d %d %d %d\n", &galaxy.d_num_species,
	&galaxy.num_species, &galaxy.radius, &galaxy.turn_number);

    galaxy_fd = creat ("galaxy.dat", 0600);
    if (galaxy_fd < 0)
    {
	fprintf (stderr, "\n  Cannot create new version of file galaxy.dat!\n");
	exit (-1);
    }

    n = write (galaxy_fd, &galaxy, sizeof (struct galaxy_data));
    if (n != sizeof (struct galaxy_data))
    {
	fprintf (stderr, "\n\tCannot write data to file 'galaxy.dat'!\n\n");
	exit (-1);
    }
    close (galaxy_fd);

    /* Do star data. */
    printf ("Doing star data...\n");
    fscanf (ascii_file, "%d\n", &num_stars);

    /* Allocate enough memory for all stars. */
    n = num_stars * sizeof(struct star_data);
    star_base = (struct star_data *) malloc (n);
    if (star_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for star data!\n\n");
	exit (-1);
    }

    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	/* Initialize structure to all zeros. */
	tp = (char *) star;
	for (j = 0; j < sizeof (struct star_data); j++) *tp++ = 0;

	fscanf (ascii_file, " %d", &n);
	if (n != i+1)
	{
	    fprintf (stderr, "\n\tStar data test failure!\n\n");
	    exit (-1);
	}

	fscanf (ascii_file, " %d", &n); star->x = n;
	fscanf (ascii_file, " %d", &n); star->y = n;
	fscanf (ascii_file, " %d", &n); star->z = n;
	fscanf (ascii_file, " %d", &n); star->type = n;
	fscanf (ascii_file, " %d", &n); star->color = n;
	fscanf (ascii_file, " %d", &n); star->size = n;
	fscanf (ascii_file, " %d", &n); star->num_planets = n;
	fscanf (ascii_file, " %d", &n); star->home_system = n;
	fscanf (ascii_file, " %d", &n); star->worm_here = n;
	fscanf (ascii_file, " %d", &n); star->worm_x = n;
	fscanf (ascii_file, " %d", &n); star->worm_y = n;
	fscanf (ascii_file, " %d", &n); star->worm_z = n;
	fscanf (ascii_file, " %d", &n); star->planet_index = n;
	fscanf (ascii_file, " %ld", &n); star->message = n;
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	{
	    fscanf (ascii_file, " %ld", &n);
	    star->visited_by[j] = n;
	}
	fscanf (ascii_file, "\n");
    }

    /* Do planet data. */
    printf ("Doing planet data...\n");
    fscanf (ascii_file, "%d\n", &num_planets);

    /* Allocate enough memory for all planets. */
    n = num_planets * sizeof(struct planet_data);
    planet_base = (struct planet_data *) malloc (n);
    if (planet_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for planet data!\n\n");
	exit (-1);
    }

    for (i = 0; i < num_planets; i++)
    {
	planet = planet_base + i;

	/* Initialize structure to all zeros. */
	tp = (char *) planet;
	for (j = 0; j < sizeof (struct planet_data); j++) *tp++ = 0;

	fscanf (ascii_file, " %d", &n);
	if (n != i+1)
	{
	    fprintf (stderr, "\n\tPlanet data test failure!\n\n");
	    exit (-1);
	}

	fscanf (ascii_file, " %d", &n); planet->temperature_class = n;
	fscanf (ascii_file, " %d", &n); planet->pressure_class = n;
	for (j = 0; j < 4; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    planet->gas[j] = n;
	}
	for (j = 0; j < 4; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    planet->gas_percent[j] = n;
	}
	fscanf (ascii_file, " %d", &n); planet->diameter = n;
	fscanf (ascii_file, " %d", &n); planet->gravity = n;
	fscanf (ascii_file, " %d", &n); planet->mining_difficulty = n;
	fscanf (ascii_file, " %d", &n); planet->econ_efficiency = n;
	fscanf (ascii_file, " %d", &n); planet->md_increase = n;
	fscanf (ascii_file, " %ld\n", &n); planet->message = n;
    }

    /* Do species data. */
    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	data_modified[species_index] = FALSE;
	data_in_memory[species_index] = FALSE;
    }

    while (! feof (ascii_file))
    {
	fscanf (ascii_file, "%d\n", &n);

	species_number = n;
	species_index = n - 1;

	data_in_memory[species_index] = TRUE;
	data_modified[species_index] = TRUE;

	species = &spec_data[species_index];

	/* Initialize structure to all zeros. */
	tp = (char *) species;
	for (j = 0; j < sizeof (struct species_data); j++) *tp++ = 0;

	fscanf (ascii_file, "%[^\n]", species->name);
	fscanf (ascii_file, "\n");
	printf ("Doing species #%d, SP %s...\n", species_number, species->name);

	fscanf (ascii_file, "%[^\n]", species->govt_name);
	fscanf (ascii_file, "\n");
	fscanf (ascii_file, "%[^\n]", species->govt_type);
	fscanf (ascii_file, "\n");
	fscanf (ascii_file, " %d", &n); species->x = n;
	fscanf (ascii_file, " %d", &n); species->y = n;
	fscanf (ascii_file, " %d", &n); species->z = n;
	fscanf (ascii_file, " %d", &n); species->pn = n;
	fscanf (ascii_file, " %d", &n); species->required_gas = n;
	fscanf (ascii_file, " %d", &n); species->required_gas_min = n;
	fscanf (ascii_file, " %d", &n); species->required_gas_max = n;
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    species->neutral_gas[j] = n;
	}
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    species->poison_gas[j] = n;
	}
	fscanf (ascii_file, " %d", &n); species->auto_orders = n;
	fscanf (ascii_file, " %d", &n); species->num_namplas = n;
	fscanf (ascii_file, " %d\n", &n); species->num_ships = n;
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    species->tech_level[j] = n;
	}
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    species->init_tech_level[j] = n;
	}
	fscanf (ascii_file, "\n");
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %d", &n);
	    species->tech_knowledge[j] = n;
	}
	for (j = 0; j < 6; j++)
	{
	    fscanf (ascii_file, " %ld", &n);
	    species->tech_eps[j] = n;
	}
	fscanf (ascii_file, " %ld", &n); species->econ_units = n;
	fscanf (ascii_file, " %ld", &n); species->hp_original_base = n;
	fscanf (ascii_file, " %ld", &n); species->fleet_cost = n;
	fscanf (ascii_file, " %ld\n", &n); species->fleet_percent_cost = n;
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	{
	    fscanf (ascii_file, " %ld", &n);
	    species->contact[j] = n;
	}
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	{
	    fscanf (ascii_file, " %ld", &n);
	    species->ally[j] = n;
	}
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	{
	    fscanf (ascii_file, " %ld", &n);
	    species->enemy[j] = n;
	}
	fscanf (ascii_file, "\n");

	/* Allocate enough memory for all namplas. */
	n = species->num_namplas * sizeof (struct nampla_data);
	namp_data[species_index] = (struct nampla_data *) malloc (n);
	if (namp_data[species_index] == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for nampla data!\n\n");
	    exit (-1);
	}

	/* Do namplas for this species. */
	nampla_base = namp_data[species_index];
	for (i = 0; i < species->num_namplas; i++)
	{
	    nampla = nampla_base + i;

	    /* Initialize structure to all zeros. */
	    tp = (char *) nampla;
	    for (j = 0; j < sizeof (struct nampla_data); j++) *tp++ = 0;

	    fscanf (ascii_file, "%d\n", &n);
	    if (n != i+1)
	    {
		fprintf (stderr,
		    "\n\tNampla data test failure, i = %d, n = %d\n\n", i, n);
		exit (-1);
	    }

	    fscanf (ascii_file, "%[^\n]", nampla->name);
	    fscanf (ascii_file, "\n");

	    fscanf (ascii_file, " %d", &n); nampla->x = n;
	    fscanf (ascii_file, " %d", &n); nampla->y = n;
	    fscanf (ascii_file, " %d", &n); nampla->z = n;
	    fscanf (ascii_file, " %d", &n); nampla->pn = n;
	    fscanf (ascii_file, " %d", &n); nampla->status = n;
	    fscanf (ascii_file, " %d", &n); nampla->hiding = n;
	    fscanf (ascii_file, " %d", &n); nampla->hidden = n;
	    fscanf (ascii_file, " %d", &n); nampla->planet_index = n;
	    fscanf (ascii_file, " %d", &n); nampla->siege_eff = n;
	    fscanf (ascii_file, " %d", &n); nampla->shipyards = n;
	    fscanf (ascii_file, " %d", &n); nampla->IUs_needed = n;
	    fscanf (ascii_file, " %d", &n); nampla->AUs_needed = n;
	    fscanf (ascii_file, " %d", &n); nampla->auto_IUs = n;
	    fscanf (ascii_file, " %d", &n); nampla->auto_AUs = n;
	    fscanf (ascii_file, " %d", &n); nampla->IUs_to_install = n;
	    fscanf (ascii_file, " %d", &n); nampla->AUs_to_install = n;
	    fscanf (ascii_file, " %ld", &n); nampla->mi_base = n;
	    fscanf (ascii_file, " %ld", &n); nampla->ma_base = n;
	    fscanf (ascii_file, " %ld\n", &n); nampla->pop_units = n;
	    for (j = 0; j < MAX_ITEMS; j++)
	    {
		fscanf (ascii_file, " %ld", &n);
		nampla->item_quantity[j] = n;
	    }
	    fscanf (ascii_file, " %ld", &n); nampla->use_on_ambush = n;
	    fscanf (ascii_file, " %ld\n", &n); nampla->message = n;
	}

	/* Allocate enough memory for all ships. */
	n = species->num_ships * sizeof (struct ship_data);
	ship_data[species_index] = (struct ship_data *) malloc (n);
	if (ship_data[species_index] == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for ship data!\n\n");
	    exit (-1);
	}

	/* Do ships for this species. */
	ship_base = ship_data[species_index];
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;

	    /* Initialize structure to all zeros. */
	    tp = (char *) ship;
	    for (j = 0; j < sizeof (struct ship_data); j++) *tp++ = 0;

	    fscanf (ascii_file, "%d\n", &n);
	    if (n != i+1)
	    {
		fprintf (stderr,
		    "\n\tShip data test failure, i = %d, n = %d\n\n", i, n);
		exit (-1);
	    }

	    fscanf (ascii_file, "%[^\n]", ship->name);
	    fscanf (ascii_file, "\n");

	    fscanf (ascii_file, " %d", &n); ship->x = n;
	    fscanf (ascii_file, " %d", &n); ship->y = n;
	    fscanf (ascii_file, " %d", &n); ship->z = n;
	    fscanf (ascii_file, " %d", &n); ship->pn = n;
	    fscanf (ascii_file, " %d", &n); ship->status = n;
	    fscanf (ascii_file, " %d", &n); ship->type = n;
	    fscanf (ascii_file, " %d", &n); ship->dest_x = n;
	    fscanf (ascii_file, " %d", &n); ship->dest_y = n;
	    fscanf (ascii_file, " %d", &n); ship->dest_z = n;
	    fscanf (ascii_file, " %d", &n); ship->just_jumped = n;
	    fscanf (ascii_file, " %d", &n); ship->arrived_via_wormhole = n;
	    fscanf (ascii_file, " %d", &n); ship->class = n;
	    fscanf (ascii_file, " %d\n", &n); ship->tonnage = n;
	    for (j = 0; j < MAX_ITEMS; j++)
	    {
		fscanf (ascii_file, " %d", &n);
		ship->item_quantity[j] = n;
	    }
	    fscanf (ascii_file, " %d", &n); ship->age = n;
	    fscanf (ascii_file, " %d", &n); ship->loading_point = n;
	    fscanf (ascii_file, " %d", &n); ship->unloading_point = n;
	    fscanf (ascii_file, " %d\n", &n); ship->remaining_cost = n;
	}
    }

    fclose (ascii_file);

    /* Save all binary data. */
    save_star_data ();
    save_planet_data ();
    save_species_data ();

    free_species_data ();
    free (planet_base);
    free (star_base);

    exit (0);
}
