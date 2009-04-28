
/*
	This program will create the single ASCII file 'ascii.dat' which
	will contain an ASCII version of all '.dat' files in the game
	(except 'locations.dat' which can be generated using the separate
	Locations program).  The companion program AsciiToBinary will
	perform the reverse operation.
*/

#define THIS_IS_MAIN

#include "fh.h"


extern int		num_stars, num_planets;

int			species_index, species_number;


struct galaxy_data	galaxy;
struct star_data	*star;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, j;

    FILE	*ascii_file;


    /* Get all binary data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_species_data ();

    /* Create file 'ascii.dat'. */
    ascii_file = fopen ("ascii.dat", "w");
    if (ascii_file == NULL)
    {
	fprintf (stderr, "\n\tCannot open 'ascii.dat' for writing!\n\n");
	exit (-1);
    }

    /* Do galaxy data. */
    printf ("Doing galaxy data...\n");
    fprintf (ascii_file, "%d %d %d %d\n", galaxy.d_num_species,
	galaxy.num_species, galaxy.radius, galaxy.turn_number);

    /* Do star data. */
    printf ("Doing star data...\n");
    fprintf (ascii_file, "%d\n", num_stars);
    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	fprintf (ascii_file, " %d", i+1);
	fprintf (ascii_file, " %d", star->x);
	fprintf (ascii_file, " %d", star->y);
	fprintf (ascii_file, " %d", star->z);
	fprintf (ascii_file, " %d", star->type);
	fprintf (ascii_file, " %d", star->color);
	fprintf (ascii_file, " %d", star->size);
	fprintf (ascii_file, " %d", star->num_planets);
	fprintf (ascii_file, " %d", star->home_system);
	fprintf (ascii_file, " %d", star->worm_here);
	fprintf (ascii_file, " %d", star->worm_x);
	fprintf (ascii_file, " %d", star->worm_y);
	fprintf (ascii_file, " %d", star->worm_z);
	fprintf (ascii_file, " %d", star->planet_index);
	fprintf (ascii_file, " %ld", star->message);
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	    fprintf (ascii_file, " %ld", star->visited_by[j]);
	fprintf (ascii_file, "\n");
    }

    /* Do planet data. */
    printf ("Doing planet data...\n");
    fprintf (ascii_file, "%d\n", num_planets);
    for (i = 0; i < num_planets; i++)
    {
	planet = planet_base + i;

	fprintf (ascii_file, " %d", i+1);
	fprintf (ascii_file, " %d", planet->temperature_class);
	fprintf (ascii_file, " %d", planet->pressure_class);
	for (j = 0; j < 4; j++)
	    fprintf (ascii_file, " %d", planet->gas[j]);
	for (j = 0; j < 4; j++)
	    fprintf (ascii_file, " %d", planet->gas_percent[j]);
	fprintf (ascii_file, " %d", planet->diameter);
	fprintf (ascii_file, " %d", planet->gravity);
	fprintf (ascii_file, " %d", planet->mining_difficulty);
	fprintf (ascii_file, " %d", planet->econ_efficiency);
	fprintf (ascii_file, " %d", planet->md_increase);
	fprintf (ascii_file, " %ld\n", planet->message);
    }

    /* Do species data. */
    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	species_number = species_index + 1;

	if (! data_in_memory[species_index])
	{
	    printf ("Skipping species #%d.\n", species_number);
	    continue;
	}

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];
	ship_base = ship_data[species_number - 1];

	printf ("Doing species #%d, SP %s...\n", species_number, species->name);
	fprintf (ascii_file, "%d\n", species_number);
	fprintf (ascii_file, "%s\n", species->name);
	fprintf (ascii_file, "%s\n", species->govt_name);
	fprintf (ascii_file, "%s\n", species->govt_type);
	fprintf (ascii_file, " %d", species->x);
	fprintf (ascii_file, " %d", species->y);
	fprintf (ascii_file, " %d", species->z);
	fprintf (ascii_file, " %d", species->pn);
	fprintf (ascii_file, " %d", species->required_gas);
	fprintf (ascii_file, " %d", species->required_gas_min);
	fprintf (ascii_file, " %d", species->required_gas_max);
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %d", species->neutral_gas[j]);
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %d", species->poison_gas[j]);
	fprintf (ascii_file, " %d", species->auto_orders);
	fprintf (ascii_file, " %d", species->num_namplas);
	fprintf (ascii_file, " %d\n", species->num_ships);
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %d", species->tech_level[j]);
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %d", species->init_tech_level[j]);
	fprintf (ascii_file, "\n");
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %d", species->tech_knowledge[j]);
	for (j = 0; j < 6; j++)
	    fprintf (ascii_file, " %ld", species->tech_eps[j]);
	fprintf (ascii_file, " %ld", species->econ_units);
	fprintf (ascii_file, " %ld", species->hp_original_base);
	fprintf (ascii_file, " %ld", species->fleet_cost);
	fprintf (ascii_file, " %ld\n", species->fleet_percent_cost);
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	    fprintf (ascii_file, " %ld", species->contact[j]);
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	    fprintf (ascii_file, " %ld", species->ally[j]);
	for (j = 0; j < NUM_CONTACT_WORDS; j++)
	    fprintf (ascii_file, " %ld", species->enemy[j]);

	fprintf (ascii_file, "\n");

	/* Do namplas for this species. */
	for (i = 0; i < species->num_namplas; i++)
	{
	    nampla = nampla_base + i;

	    fprintf (ascii_file, "%d\n", i+1);
	    fprintf (ascii_file, "%s\n", nampla->name);
	    fprintf (ascii_file, " %d", nampla->x);
	    fprintf (ascii_file, " %d", nampla->y);
	    fprintf (ascii_file, " %d", nampla->z);
	    fprintf (ascii_file, " %d", nampla->pn);
	    fprintf (ascii_file, " %d", nampla->status);
	    fprintf (ascii_file, " %d", nampla->hiding);
	    fprintf (ascii_file, " %d", nampla->hidden);
	    fprintf (ascii_file, " %d", nampla->planet_index);
	    fprintf (ascii_file, " %d", nampla->siege_eff);
	    fprintf (ascii_file, " %d", nampla->shipyards);
	    fprintf (ascii_file, " %d", nampla->IUs_needed);
	    fprintf (ascii_file, " %d", nampla->AUs_needed);
	    fprintf (ascii_file, " %d", nampla->auto_IUs);
	    fprintf (ascii_file, " %d", nampla->auto_AUs);
	    fprintf (ascii_file, " %d", nampla->IUs_to_install);
	    fprintf (ascii_file, " %d", nampla->AUs_to_install);
	    fprintf (ascii_file, " %ld", nampla->mi_base);
	    fprintf (ascii_file, " %ld", nampla->ma_base);
	    fprintf (ascii_file, " %ld\n", nampla->pop_units);
	    for (j = 0; j < MAX_ITEMS; j++)
		fprintf (ascii_file, " %ld", nampla->item_quantity[j]);
	    fprintf (ascii_file, " %ld", nampla->use_on_ambush);
	    fprintf (ascii_file, " %ld\n", nampla->message);
	}

	/* Do ships for this species. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;

	    fprintf (ascii_file, "%d\n", i+1);
	    fprintf (ascii_file, "%s\n", ship->name);
	    fprintf (ascii_file, " %d", ship->x);
	    fprintf (ascii_file, " %d", ship->y);
	    fprintf (ascii_file, " %d", ship->z);
	    fprintf (ascii_file, " %d", ship->pn);
	    fprintf (ascii_file, " %d", ship->status);
	    fprintf (ascii_file, " %d", ship->type);
	    fprintf (ascii_file, " %d", ship->dest_x);
	    fprintf (ascii_file, " %d", ship->dest_y);
	    fprintf (ascii_file, " %d", ship->dest_z);
	    fprintf (ascii_file, " %d", ship->just_jumped);
	    fprintf (ascii_file, " %d", ship->arrived_via_wormhole);
	    fprintf (ascii_file, " %d", ship->class);
	    fprintf (ascii_file, " %d\n", ship->tonnage);
	    for (j = 0; j < MAX_ITEMS; j++)
		fprintf (ascii_file, " %d", ship->item_quantity[j]);
	    fprintf (ascii_file, " %d", ship->age);
	    fprintf (ascii_file, " %d", ship->loading_point);
	    fprintf (ascii_file, " %d", ship->unloading_point);
	    fprintf (ascii_file, " %d\n", ship->remaining_cost);
	}
    }

    fclose (ascii_file);

    free_species_data ();
    free (planet_base);
    free (star_base);

    exit (0);
}
