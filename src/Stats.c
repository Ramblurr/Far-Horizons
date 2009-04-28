
#define THIS_IS_MAIN

#include "fh.h"


int			species_number;

long			power ();

char			input_line[128];	/* Not actually used. */


struct galaxy_data	galaxy;
struct planet_data	*planet, *home_planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern struct planet_data	*planet_base;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, m, n, nampla_index, ship_index, num_ships, avg_tech_level,
		all_tech_level[6], n_species, n_warships, n_starbases,
		n_transports, avg_pop_pl, n_pop_pl, ls_needed, num_yards,
		production_penalty, fleet_percent_cost, num_pop_planets,
		min_starbases, max_starbases, min_warships, max_warships,
		min_transports, max_transports, min_tech_level[6],
		max_tech_level[6], min_pop_pl, max_pop_pl, ntr, nba, nwa,
		n_yards, min_yards, max_yards, avg_yards;

    long	total_production, raw_material_units, production_capacity,
		total_tonnage, total_offensive_power, total_defensive_power,
		avg_production, all_production, avg_warship_tons,
		all_warship_tons, avg_starbase_tons, all_starbase_tons,
		avg_transport_tons, all_transport_tons, n1, n2, n3,
		min_production, max_production;

    short	tons;


    /* Check for valid command line. */
    if (argc != 1)
    {
	fprintf (stderr, "\n\tUsage: Stats\n\n");
	exit (0);
    }

    /* Get all necessary data. */
    get_galaxy_data ();
    get_planet_data ();
    get_species_data ();

    /* Initialize data. */
    n_species = 0;
    all_production = 0;
    min_production = 1000000000;
    max_production = 0;
    all_warship_tons = 0;
    all_starbase_tons = 0;
    all_transport_tons = 0;
    n_warships = 0;
    min_warships = 32000;
    max_warships = 0;
    n_starbases = 0;
    min_starbases = 32000;
    max_starbases = 0;
    n_transports = 0;
    min_transports = 32000;
    max_transports = 0;
    n_pop_pl = 0;
    min_pop_pl = 32000;
    max_pop_pl = 0;
    n_yards = 0;
    min_yards = 32000;
    max_yards = 0;
    for (i = 0; i < 6; i++)
    {
	all_tech_level[i] = 0;
	min_tech_level[i] = 32000;
	max_tech_level[i] = 0;
    }

    /* Print header. */
    printf ("SP Species               Tech Levels        Total  Num Num  Num  Offen.  Defen.\n");
    printf (" # Name             MI  MA  ML  GV  LS  BI  Prod.  Pls Shps Yrds  Power   Power\n");
    printf ("-------------------------------------------------------------------------------\n");

    /* Main loop. For each species, take appropriate action. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	if (! data_in_memory[species_number - 1]) continue;

	++n_species;

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];
	ship_base = ship_data[species_number - 1];

	/* Get fleet maintenance cost. */
	fleet_percent_cost = species->fleet_percent_cost;

	if (fleet_percent_cost > 10000) fleet_percent_cost = 10000;

	/* Print species data. */
	printf ("%2d", species_number);
	printf (" %-15.15s", species->name);

	for (i = 0; i < 6; i++)
	{
	    printf ("%4d", species->tech_level[i]);
	    all_tech_level[i] += (int) species->tech_level[i];
	    if (species->tech_level[i] < min_tech_level[i])
		min_tech_level[i] = species->tech_level[i];
	    if (species->tech_level[i] > max_tech_level[i])
		max_tech_level[i] = species->tech_level[i];
	}

	/* Get stats for namplas. */
	total_production = 0;
	total_defensive_power = 0;
	num_yards = 0;
	num_pop_planets = 0;
	home_planet = planet_base + (int) nampla_base->planet_index;
	nampla = nampla_base - 1;
	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;

	    num_yards += nampla->shipyards;
	    n_yards += nampla->shipyards;

	    planet = planet_base + (int) nampla->planet_index;

	    raw_material_units =
		(10L * (long) species->tech_level[MI] * (long) nampla->mi_base)
			/ (long) planet->mining_difficulty;

	    production_capacity =
		((long) species->tech_level[MA] * (long) nampla->ma_base) / 10L;

	    ls_needed = life_support_needed (species, home_planet, planet);

	    if (ls_needed == 0)
		production_penalty = 0;
	    else
		production_penalty = (100 * ls_needed) / species->tech_level[LS];

	    raw_material_units
		-= (production_penalty * raw_material_units) / 100;

	    raw_material_units
		= (((long) planet->econ_efficiency * raw_material_units) + 50) / 100;

	    production_capacity
		-= (production_penalty * production_capacity) / 100;

	    production_capacity
		= (((long) planet->econ_efficiency * production_capacity) + 50) / 100;

	    if (nampla->status & MINING_COLONY)
		n1 = (2 * raw_material_units) / 3;
	    else if (nampla->status & RESORT_COLONY)
		n1 = (2 * production_capacity) / 3;
	    else
		n1 = (production_capacity > raw_material_units)
			? raw_material_units : production_capacity;

	    n2 = ((fleet_percent_cost * n1) + 5000) / 10000;
	    n3 = n1 - n2;
	    total_production += n3;

	    tons = nampla->item_quantity[PD]/200;
	    if (tons < 1  &&  nampla->item_quantity[PD] > 0) tons = 1;
	    total_defensive_power += power (tons);

	    if (nampla->status & POPULATED)
	    {
		++n_pop_pl;
		++num_pop_planets;
	    }
	}

	printf ("%7ld%4d", total_production, num_pop_planets);

	if (total_production < min_production) min_production = total_production;
	if (total_production > max_production) max_production = total_production;

	if (num_pop_planets < min_pop_pl) min_pop_pl = num_pop_planets;
	if (num_pop_planets > max_pop_pl) max_pop_pl = num_pop_planets;

	if (num_yards < min_yards) min_yards = num_yards;
	if (num_yards > max_yards) max_yards = num_yards;

	all_production += total_production;

	/* Get stats for ships. */
	num_ships = 0;
	ntr = 0;  nba = 0;  nwa = 0;
	total_tonnage = 0;
	total_offensive_power = 0;
	ship = ship_base - 1;
	for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;

	    if (ship->status == UNDER_CONSTRUCTION) continue;

	    ++num_ships;
	    total_tonnage += (long) ship->tonnage;

	    if (ship->type == STARBASE)
	    {
		total_defensive_power += power (ship->tonnage);
		all_starbase_tons += (long) ship->tonnage;
		++n_starbases;  ++nba;
	    }
	    else if (ship->class == TR)
	    {
		all_transport_tons += (long) ship->tonnage;
		++n_transports;  ++ntr;
	    }
	    else
	    {
		if (ship->type == SUB_LIGHT)
		    total_defensive_power += power (ship->tonnage);
		else
		    total_offensive_power += power (ship->tonnage);
		all_warship_tons += (long) ship->tonnage;
		++n_warships;  ++nwa;
	    }
	}

	if (nwa < min_warships ) min_warships = nwa;
	if (nwa > max_warships ) max_warships = nwa;

	if (nba < min_starbases ) min_starbases = nba;
	if (nba > max_starbases ) max_starbases = nba;

	if (ntr < min_transports ) min_transports = ntr;
	if (ntr > max_transports ) max_transports = ntr;

	total_offensive_power +=
		((long) species->tech_level[ML] * total_offensive_power) / 50;

	total_defensive_power +=
		((long) species->tech_level[ML] * total_defensive_power) / 50;

	if (species->tech_level[ML] == 0)
	{
	    total_defensive_power = 0;
	    total_offensive_power = 0;
	}

	total_offensive_power /= 10;
	total_defensive_power /= 10;

	printf ("%5d", num_ships);
	printf ("%5d", num_yards);
	printf ("%8ld%8ld\n", total_offensive_power, total_defensive_power);
    }

    m = n_species / 2;
    printf ("\n");
    for (i = 0; i < 6; i++)
    {
	avg_tech_level = (all_tech_level[i] + m) / n_species;
	printf ("Average %s tech level = %d (min = %d, max = %d)\n",
	    tech_name[i], avg_tech_level, min_tech_level[i], max_tech_level[i]);
    }

    i = ((10* n_warships) + m) / n_species;
    printf ("\nAverage number of warships per species = %d.%d (min = %d, max = %d)\n",
	i/10, i%10, min_warships, max_warships);

    if (n_warships == 0) n_warships = 1;
    avg_warship_tons = (10000L * all_warship_tons) / n_warships;
    avg_warship_tons = 1000L * ((avg_warship_tons + 500L) / 1000L);
    printf ("Average warship size = %s tons\n", commas (avg_warship_tons));

    avg_warship_tons = (10000L * all_warship_tons) / n_species;
    avg_warship_tons = 1000L * ((avg_warship_tons + 500L) / 1000L);
    printf ("Average total warship tonnage per species = %s tons\n",
	commas (avg_warship_tons));

    i = ((10 * n_starbases) + m) / n_species;
    printf ("\nAverage number of starbases per species = %d.%d (min = %d, max = %d)\n",
	i/10, i%10, min_starbases, max_starbases);

    if (n_starbases == 0) n_starbases = 1;
    avg_starbase_tons = (10000L * all_starbase_tons) / n_starbases;
    avg_starbase_tons = 1000L * ((avg_starbase_tons + 500L) / 1000L);
    printf ("Average starbase size = %s tons\n", commas (avg_starbase_tons));

    avg_starbase_tons = (10000L * all_starbase_tons) / n_species;
    avg_starbase_tons = 1000L * ((avg_starbase_tons + 500L) / 1000L);
    printf ("Average total starbase tonnage per species = %s tons\n",
	commas (avg_starbase_tons));

    i = ((10 * n_transports) + m) / n_species;
    printf ("\nAverage number of transports per species = %d.%d (min = %d, max = %d)\n",
	i/10, i%10, min_transports, max_transports);

    if (n_transports == 0) n_transports = 1;
    avg_transport_tons = (10000L * all_transport_tons) / n_transports;
    avg_transport_tons = 1000L * ((avg_transport_tons + 500L) / 1000L);
    printf ("Average transport size = %s tons\n", commas (avg_transport_tons));

    avg_transport_tons = (10000L * all_transport_tons) / n_species;
    avg_transport_tons = 1000L * ((avg_transport_tons + 500L) / 1000L);
    printf ("Average total transport tonnage per species = %s tons\n",
	commas (avg_transport_tons));

    avg_yards = ((10 * n_yards) + m) / n_species;
    printf ("\nAverage number of shipyards per species = %d.%d (min = %d, max = %d)\n",
	avg_yards/10, avg_yards%10, min_yards, max_yards);

    avg_pop_pl = ((10 * n_pop_pl) + m) / n_species;
    printf ("\nAverage number of populated planets per species = %d.%d (min = %d, max = %d)\n",
	avg_pop_pl/10, avg_pop_pl%10, min_pop_pl, max_pop_pl);

    avg_production = (all_production + m) / n_species;
    printf ("Average total production per species = %ld (min = %ld, max = %ld)\n",
	avg_production, min_production, max_production);
}
