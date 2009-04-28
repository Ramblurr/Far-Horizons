
/* This program will generate reports for all species in the game and
   write them to separate files. Each report will consist of a log of the
   events of the previous turn, plus status information for the current
   turn. */

#define	THIS_IS_MAIN

#include "fh.h"


int	x, y, z, printing_alien, species_number, fleet_percent_cost;
int	test_mode, verbose_mode;

char	ship_already_listed[5000];

FILE	*report_file;

struct galaxy_data	galaxy;
struct planet_data	*planet, *home_planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla1_base, *nampla2_base;
struct ship_data	*ship_base, *ship1_base, *ship2_base;

extern int			num_locs, log_stdout, ignore_field_distorters,
				truncate_name, num_stars;
extern FILE			*log_file;

extern struct sp_loc_data	loc[MAX_LOCATIONS];
extern struct star_data		*star_base;
extern struct planet_data	*planet_base;

main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, k, ship_index, locations_fd, my_loc_index, its_loc_index,
		industry, turn_number, alien_number, species_fd,
		header_printed, alien_can_hide, do_this_species, sp_index,
		array_index, bit_number, we_have_colony_here, nampla_index,
		we_have_planet_here, found, ls_needed, production_penalty,
		temp_ignore_field_distorters;

    char	filename[32], log_line[256], temp1[16], temp2[128];

    long	n, nn, bit_mask;

    struct species_data		*alien;
    struct nampla_data		*nampla, *alien_nampla, *our_nampla,
				*temp_nampla;
    struct ship_data		*ship, *ship2, *alien_ship;
    struct sp_loc_data		*locations_base, *my_loc, *its_loc;


    /* Check for options, if any. */
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-t") == 0) test_mode = TRUE;
      if (strcmp (argv[i], "-v") == 0) verbose_mode = TRUE;
    }

    /* Get all necessary data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_species_data ();
    get_location_data ();

    turn_number = galaxy.turn_number;

    /* Generate a report for each species. */
    alien_number = 0;	/* Pointers to alien data not yet assigned. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	/* Check if we are doing all species, or just one or more specified
		ones. */
	do_this_species = TRUE;
	if (argc > 1)
	{
	    do_this_species = FALSE;
	    for (i = 1; i < argc; i++)
	    {
		j = atoi(argv[i]);
		if (species_number == j)
		{
		    do_this_species = TRUE;
		    break;
		}
	    }
	}

	if (! do_this_species) continue;

	/* Check if this species is still in the game. */
	if (! data_in_memory[species_number - 1])
	{
	    if (argc == 1)
		continue;	/* This species is no longer in the game. */

	    fprintf (stderr, "\n\tCannot open data file for species #%d!\n\n",
		species_number);
	    exit (-1);
	}

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];
	nampla1_base = nampla_base;
	ship_base = ship_data[species_number - 1];
	ship1_base = ship_base;
	home_planet = planet_base + (long) nampla1_base->planet_index;

	/* Print message for gamemaster. */
	if (verbose_mode)
	  printf ("Generating turn %d report for species #%d, SP %s...\n",
		turn_number, species_number, species->name);

 	/* Open report file for writing. */
	sprintf (filename, "sp%02d.rpt.t%d", species_number, turn_number);
	report_file = fopen (filename, "w");
	if (report_file == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for writing!\n\n", filename);
	    exit (-1);
	}

	/* Copy log file, if any, to output file. */
	sprintf (filename, "sp%02d.log", species_number);
	log_file = fopen (filename, "r");
	if (log_file != NULL)
	{
	    if (turn_number > 1)
		fprintf (report_file, "\n\n\t\t\tEVENT LOG FOR TURN %d\n",
					turn_number - 1);

	    while (fgets(log_line, 256, log_file) != NULL)
		fputs (log_line, report_file);

	    fprintf (report_file, "\n\n");

	    fclose (log_file);
	}

	/* Print header for status report. */
	fprintf (report_file,
		"\n\t\t\t SPECIES STATUS\n\n\t\t\tSTART OF TURN %d\n\n",
				turn_number);

	fprintf (report_file, "Species name: %s\n", species->name);
	fprintf (report_file, "Government name: %s\n", species->govt_name);
	fprintf (report_file, "Government type: %s\n", species->govt_type);

	fprintf (report_file, "\nTech Levels:\n");
	for (i = 0; i < 6; i++)
	{
	    fprintf (report_file, "   %s = %d", tech_name[i],
			species->tech_level[i]);
	    if (species->tech_knowledge[i] > species->tech_level[i])
		fprintf (report_file, "/%d", species->tech_knowledge[i]);
	    fprintf (report_file, "\n");
	}

	fprintf (report_file, "\nAtmospheric Requirement: %d%%-%d%% %s",
			(int) species->required_gas_min,
			(int) species->required_gas_max,
			gas_string[species->required_gas]); 
	fprintf (report_file, "\nNeutral Gases:");
	for (i = 0; i < 6; i++)
	{
	    if (i != 0) fprintf (report_file, ",");
	    fprintf (report_file, " %s", gas_string[species->neutral_gas[i]]);
	}
	fprintf (report_file, "\nPoisonous Gases:");
	for (i = 0; i < 6; i++)
	{
	    if (i != 0) fprintf (report_file, ",");
	    fprintf (report_file, " %s", gas_string[species->poison_gas[i]]);
	}
	fprintf (report_file, "\n");

	/* List fleet maintenance cost and its percentage of total
	   production. */
	fleet_percent_cost = species->fleet_percent_cost;

	fprintf (report_file,
	    "\nFleet maintenance cost = %ld (%d.%02d%% of total production)\n",
	    species->fleet_cost, fleet_percent_cost/100,
	    fleet_percent_cost%100);

	if (fleet_percent_cost > 10000) fleet_percent_cost = 10000;

	/* List species that have been met. */
	n = 0;
	log_file = report_file;		/* Use log utils for this. */
	log_stdout = FALSE;
	header_printed = FALSE;
	for (sp_index = 0; sp_index < galaxy.num_species; sp_index++)
	{
	    if (! data_in_memory[sp_index]) continue;

	    array_index = (sp_index) / 32;
	    bit_number = (sp_index) % 32;
	    bit_mask = 1 << bit_number;
	    if ((species->contact[array_index] & bit_mask) == 0) continue;

	    if (! header_printed)
	    {
		log_string ("\nSpecies met: ");
		header_printed = TRUE;
	    }

	    if (n > 0) log_string (", ");
	    log_string ("SP ");  log_string (spec_data[sp_index].name);
	    ++n;
	}
	if (n > 0) log_char ('\n');

	/* List declared allies. */
	n = 0;
	header_printed = FALSE;
	for (sp_index = 0; sp_index < galaxy.num_species; sp_index++)
	{
	    if (! data_in_memory[sp_index]) continue;

	    array_index = (sp_index) / 32;
	    bit_number = (sp_index) % 32;
	    bit_mask = 1 << bit_number;
	    if ((species->ally[array_index] & bit_mask) == 0) continue;
	    if ((species->contact[array_index] & bit_mask) == 0) continue;

	    if (! header_printed)
	    {
		log_string ("\nAllies: ");
		header_printed = TRUE;
	    }

	    if (n > 0) log_string (", ");
	    log_string ("SP ");  log_string (spec_data[sp_index].name);
	    ++n;
	}
	if (n > 0) log_char ('\n');

	/* List declared enemies that have been met. */
	n = 0;
	header_printed = FALSE;
	for (sp_index = 0; sp_index < galaxy.num_species; sp_index++)
	{
	    if (! data_in_memory[sp_index]) continue;

	    array_index = (sp_index) / 32;
	    bit_number = (sp_index) % 32;
	    bit_mask = 1 << bit_number;
	    if ((species->enemy[array_index] & bit_mask) == 0) continue;
	    if ((species->contact[array_index] & bit_mask) == 0) continue;

	    if (! header_printed)
	    {
		log_string ("\nEnemies: ");
		header_printed = TRUE;
	    }

	    if (n > 0) log_string (", ");
	    log_string ("SP ");  log_string (spec_data[sp_index].name);
	    ++n;
	}
	if (n > 0) log_char ('\n');

	fprintf (report_file, "\nEconomic units = %ld\n", species->econ_units);

	/* Initialize flag. */
	for (i = 0; i < species->num_ships; i++)
	    ship_already_listed[i] = FALSE;

	/* Print report for each producing planet. */
	nampla = nampla1_base - 1;
	for (i = 0; i < species->num_namplas; i++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;
	    if (nampla->mi_base == 0  &&  nampla->ma_base == 0
		&&  (nampla->status & HOME_PLANET) == 0) continue;

	    planet = planet_base + (long) nampla->planet_index;
	    fprintf (report_file,
		"\n\n* * * * * * * * * * * * * * * * * * * * * * * * *\n");
	    do_planet_report (nampla, ship1_base, species);
	}

	/* Give only a one-line listing for other planets. */
	printing_alien = FALSE;
	header_printed = FALSE;
	nampla = nampla1_base - 1;
	for (i = 0; i < species->num_namplas; i++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;
	    if (nampla->mi_base > 0  ||  nampla->ma_base > 0
		||  (nampla->status & HOME_PLANET) != 0) continue;

	    if (! header_printed)
	    {
		fprintf (report_file,
		    "\n\n* * * * * * * * * * * * * * * * * * * * * * * * *\n");
		fprintf (report_file, "\n\nOther planets and ships:\n\n");
		header_printed = TRUE;
	    }
	    fprintf (report_file, "%4d%3d%3d #%d\tPL %s", nampla->x,
		nampla->y, nampla->z, nampla->pn, nampla->name);

	    for (j = 0; j < MAX_ITEMS; j++)
	    {
		if (nampla->item_quantity[j] > 0)
		    fprintf (report_file, ", %d %s",
			nampla->item_quantity[j], item_abbr[j]);
	    }
	    fprintf (report_file, "\n");

	    /* Print any ships at this planet. */
	    ship = ship1_base - 1;
	    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	    {
		++ship;

		if (ship_already_listed[ship_index]) continue;

		if (ship->x != nampla->x) continue;
		if (ship->y != nampla->y) continue;
		if (ship->z != nampla->z) continue;
		if (ship->pn != nampla->pn) continue;

		fprintf (report_file, "\t\t%s", ship_name (ship));
		for (j = 0; j < MAX_ITEMS; j++)
		{
		    if (ship->item_quantity[j] > 0)
			fprintf (report_file, ", %d %s",
			    ship->item_quantity[j], item_abbr[j]);
		}
		fprintf (report_file, "\n");

		ship_already_listed[ship_index] = TRUE;
	    }
	}

	/* Report ships that are not associated with a planet. */
	ship = ship1_base - 1;
	for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	{
	    ++ship;

	    ship->special = 0;

	    if (ship_already_listed[ship_index]) continue;

	    ship_already_listed[ship_index] = TRUE;

	    if (ship->pn == 99) continue;

	    if (! header_printed)
	    {
		fprintf (report_file,
		    "\n\n* * * * * * * * * * * * * * * * * * * * * * * * *\n");
		fprintf (report_file, "\n\nOther planets and ships:\n\n");
		header_printed = TRUE;
	    }

	    if (ship->status == JUMPED_IN_COMBAT  ||  ship->status == FORCED_JUMP)
		fprintf (report_file, "  ?? ?? ??\t%s", ship_name (ship));
	    else if (test_mode && ship->arrived_via_wormhole)
		fprintf (report_file, "  ?? ?? ??\t%s", ship_name (ship));
	    else
		fprintf (report_file, "%4d%3d%3d\t%s",
		    ship->x, ship->y, ship->z, ship_name (ship));

	    for (i = 0; i < MAX_ITEMS; i++)
	    {
		if (ship->item_quantity[i] > 0)
		    fprintf (report_file, ", %d %s",
			ship->item_quantity[i], item_abbr[i]);
	    }
	    fprintf (report_file, "\n");

	    if (ship->status == JUMPED_IN_COMBAT
		||  ship->status == FORCED_JUMP) continue;

 	    if (test_mode && ship->arrived_via_wormhole) continue;

	    /* Print other ships at the same location. */
	    ship2 = ship;
	    for (i = ship_index + 1; i < species->num_ships; i++)
	    {
		++ship2;

		if (ship_already_listed[i]) continue;
		if (ship2->pn == 99) continue;
		if (ship2->x != ship->x) continue;
		if (ship2->y != ship->y) continue;
		if (ship2->z != ship->z) continue;

		fprintf (report_file, "\t\t%s", ship_name (ship2));
		for (j = 0; j < MAX_ITEMS; j++)
		{
		    if (ship2->item_quantity[j] > 0)
			fprintf (report_file, ", %d %s",
			    ship2->item_quantity[j], item_abbr[j]);
		}
		fprintf (report_file, "\n");

		ship_already_listed[i] = TRUE;
	    }
	}

	fprintf (report_file,
	    "\n\n* * * * * * * * * * * * * * * * * * * * * * * * *\n");

	/* Report aliens at locations where current species has inhabited
	   planets or ships. */
	printing_alien = TRUE;
	locations_base = &loc[0];
	my_loc = locations_base - 1;
	for (my_loc_index = 0; my_loc_index < num_locs; my_loc_index++)
	{
	    ++my_loc;
	    if (my_loc->s != species_number) continue;

	    header_printed = FALSE;
	    its_loc = locations_base - 1;
	    for (its_loc_index = 0; its_loc_index < num_locs; its_loc_index++)
	    {
		++its_loc;
		if (its_loc->s == species_number) continue;
		if (my_loc->x != its_loc->x) continue;
		if (my_loc->y != its_loc->y) continue;
		if (my_loc->z != its_loc->z) continue;

		/* There is an alien here. Check if pointers for data for
			this alien have been assigned yet. */
		if (its_loc->s != alien_number)
		{
		    alien_number = its_loc->s;
		    if (! data_in_memory[alien_number - 1])
		    {
			fprintf (stderr, "\n\nWarning! Data for alien #%d is needed but is not in memory!\n\n",
				alien_number);
			continue;
		    }
		    alien = &spec_data[alien_number - 1];
		    nampla2_base = namp_data[alien_number - 1];
		    ship2_base = ship_data[alien_number - 1];
		}

		/* Check if we have a named planet in this system. If so,
			use it when you print the header. */
		we_have_planet_here = FALSE;
		nampla = nampla1_base - 1;
		for (i = 0; i < species->num_namplas; i++)
		{
		    ++nampla;

		    if (nampla->x != my_loc->x) continue;
		    if (nampla->y != my_loc->y) continue;
		    if (nampla->z != my_loc->z) continue;
		    if (nampla->pn == 99) continue;

		    we_have_planet_here = TRUE;
		    our_nampla = nampla;

		    break;
		}

		/* Print all inhabited alien namplas at this location. */
		alien_nampla = nampla2_base - 1;
		for (i = 0; i < alien->num_namplas; i++)
		{
		    ++alien_nampla;

		    if (my_loc->x != alien_nampla->x) continue;
		    if (my_loc->y != alien_nampla->y) continue;
		    if (my_loc->z != alien_nampla->z) continue;
		    if ((alien_nampla->status & POPULATED) == 0) continue;

		    /* Check if current species has a colony on the same
			planet. */
		    we_have_colony_here = FALSE;
		    nampla = nampla1_base - 1;
		    for (j = 0; j < species->num_namplas; j++)
		    {
			++nampla;

			if (alien_nampla->x != nampla->x) continue;
			if (alien_nampla->y != nampla->y) continue;
			if (alien_nampla->z != nampla->z) continue;
			if (alien_nampla->pn != nampla->pn) continue;
			if ((nampla->status & POPULATED) == 0) continue;

			we_have_colony_here = TRUE;

			break;
		    }

		    if (alien_nampla->hidden  &&  ! we_have_colony_here)
			continue;

		    if (! header_printed)
		    {
			fprintf (report_file,
			    "\n\nAliens at x = %d, y = %d, z = %d",
				my_loc->x, my_loc->y, my_loc->z);

			if (we_have_planet_here)
			    fprintf (report_file, " (PL %s star system)",
				our_nampla->name);

			fprintf (report_file, ":\n");
			header_printed = TRUE;
		    }

		    industry = alien_nampla->mi_base + alien_nampla->ma_base;

		    if (alien_nampla->status & MINING_COLONY)
			sprintf (temp1, "%s", "Mining colony");
		    else if (alien_nampla->status & RESORT_COLONY)
			sprintf (temp1, "%s", "Resort colony");
		    else if (alien_nampla->status & HOME_PLANET)
			sprintf (temp1, "%s", "Home planet");
		    else if (industry > 0)
			sprintf (temp1, "%s", "Colony planet");
		    else
			sprintf (temp1, "%s", "Uncolonized planet");

		    sprintf (temp2, "  %s PL %s (pl #%d)", temp1,
			alien_nampla->name, alien_nampla->pn);
		    n = 53 - strlen (temp2);
		    for (j = 0; j < n; j++) strcat (temp2, " ");
		    fprintf (report_file, "%sSP %s\n", temp2, alien->name);

		    j = industry;
		    if (industry < 100)
			industry = (industry + 5)/10;
		    else
			industry = ((industry + 50)/100) * 10;

		    if (j == 0)
			fprintf (report_file,
			    "      (No economic base.)\n");
		    else
			fprintf (report_file,
			    "      (Economic base is approximately %d.)\n",
			    industry);

		    /* If current species has a colony on the same
			planet, report any PDs and any shipyards. */
		    if (we_have_colony_here)
		    {
			if (alien_nampla->item_quantity[PD] == 1)
			    fprintf (report_file,
				"      (There is 1 %s on the planet.)\n",
				item_name[PD]);
			else if (alien_nampla->item_quantity[PD] > 1)
			    fprintf (report_file,
				"      (There are %ld %ss on the planet.)\n",
				alien_nampla->item_quantity[PD],
				item_name[PD]);

			if (alien_nampla->shipyards == 1)
			    fprintf (report_file,
				"      (There is 1 shipyard on the planet.)\n");
			else if (alien_nampla->shipyards > 1)
			    fprintf (report_file,
				"      (There are %d shipyards on the planet.)\n",
				alien_nampla->shipyards);
		    }

		    /* Also report if alien colony is actively hiding. */
		    if (alien_nampla->hidden)
			fprintf (report_file,
			    "      (Colony is actively hiding from alien observation.)\n");
		}

		/* Print all alien ships at this location. */
		alien_ship = ship2_base - 1;
		for (i = 0; i < alien->num_ships; i++)
		{
		    ++alien_ship;

		    if (alien_ship->pn == 99) continue;
		    if (my_loc->x != alien_ship->x) continue;
		    if (my_loc->y != alien_ship->y) continue;
		    if (my_loc->z != alien_ship->z) continue;

		    /* An alien ship cannot hide if it lands on the
			surface of a planet populated by the current
			species. */
		    alien_can_hide = TRUE;
		    nampla = nampla1_base - 1;
		    for (j = 0; j < species->num_namplas; j++)
		    {
			++nampla;

			if (alien_ship->x != nampla->x) continue;
			if (alien_ship->y != nampla->y) continue;
			if (alien_ship->z != nampla->z) continue;
			if (alien_ship->pn != nampla->pn) continue;
			if (nampla->status & POPULATED)
			{
		 	    alien_can_hide = FALSE;
			    break;
			}
		    }

		    if (alien_can_hide  &&  alien_ship->status == ON_SURFACE)
			continue;

		    if (alien_can_hide  &&  alien_ship->status == UNDER_CONSTRUCTION)
			continue;

		    if (! header_printed)
		    {
			fprintf (report_file,
			    "\n\nAliens at x = %d, y = %d, z = %d",
				my_loc->x, my_loc->y, my_loc->z);

			if (we_have_planet_here)
		 	    fprintf (report_file, " (PL %s star system)",
				our_nampla->name);

			fprintf (report_file, ":\n");
			header_printed = TRUE;
		    }

		    print_ship (alien_ship, alien, alien_number);
		}
	    }
	}

	printing_alien = FALSE;

	if (test_mode) goto done_report;

	/* Generate order section. */
	truncate_name = TRUE;
	temp_ignore_field_distorters = ignore_field_distorters;
	ignore_field_distorters = TRUE;

	fprintf (report_file,
	    "\n\n* * * * * * * * * * * * * * * * * * * * * * * * *\n");

	fprintf (report_file,
	    "\n\nORDER SECTION. Remove these two lines and everything above\n");
	fprintf (report_file, "  them, and submit only the orders below.\n\n");

	fprintf (report_file, "START COMBAT\n");
	fprintf (report_file, "; Place combat orders here.\n\n");
	fprintf (report_file, "END\n\n");

	fprintf (report_file, "START PRE-DEPARTURE\n");
	fprintf (report_file, "; Place pre-departure orders here.\n\n");

	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    nampla = nampla_base + nampla_index;
	    if (nampla->pn == 99) continue;

	    /* Generate auto-installs for colonies that were loaded via
		the DEVELOP command. */
	    if (nampla->auto_IUs)
		fprintf (report_file, "\tInstall\t%d IU\tPL %s\n",
			nampla->auto_IUs, nampla->name);
	    if (nampla->auto_AUs)
		fprintf (report_file, "\tInstall\t%d AU\tPL %s\n",
			nampla->auto_AUs, nampla->name);
	    if (nampla->auto_IUs  ||  nampla->auto_AUs)
		fprintf (report_file, "\n");

	    if (! species->auto_orders) continue;

	    /* Generate auto UNLOAD orders for transports at this nampla. */
	    for (j = 0; j < species->num_ships; j++)
	    {
		ship = ship_base + j;
		if (ship->pn == 99) continue;
		if (ship->x != nampla->x) continue;
		if (ship->y != nampla->y) continue;
		if (ship->z != nampla->z) continue;
		if (ship->pn != nampla->pn) continue;
		if (ship->status == JUMPED_IN_COMBAT) continue;
		if (ship->status == FORCED_JUMP) continue;
		if (ship->class != TR) continue;
		if (ship->item_quantity[CU] < 1) continue;

		/* New colonies will never be started automatically unless
		   ship was loaded via a DEVELOP order. */
		if (ship->loading_point != 0)
		{
		    /* Check if transport is at specified unloading point. */
		    n = ship->unloading_point;
		    if (n == nampla_index
			||  (n == 9999  &&  nampla_index == 0))
				goto unload_ship;
		}

		if ((nampla->status & POPULATED) == 0) continue;

		if ((nampla->mi_base + nampla->ma_base) >= 2000) continue;

		if (nampla->x == nampla_base->x
		    &&  nampla->y == nampla_base->y
		    &&  nampla->z == nampla_base->z) continue; /* Home sector. */

unload_ship:

		n = ship->loading_point;
		if (n == 9999) n = 0;	/* Home planet. */
		if (n == nampla_index)
		    continue;	/* Ship was just loaded here. */

		fprintf (report_file, "\tUnload\tTR%d%s %s\n\n", ship->tonnage,
		    ship_type[ship->type], ship->name);

		ship->special = ship->loading_point;
		n = nampla - nampla_base;
		if (n == 0) n = 9999;
		ship->unloading_point = n;
	    }
	}

	fprintf (report_file, "END\n\n");

	fprintf (report_file, "START JUMPS\n");
	fprintf (report_file, "; Place jump orders here.\n\n");

	/* Generate auto-jumps for ships that were loaded via the DEVELOP
	   command or which were UNLOADed because of the AUTO command. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;

	    ship->just_jumped = FALSE;

	    if (ship->pn == 99) continue;
	    if (ship->status == JUMPED_IN_COMBAT) continue;
	    if (ship->status == FORCED_JUMP) continue;

	    j = ship->special;
	    if (j)
	    {
		if (j == 9999) j = 0;	/* Home planet. */
		temp_nampla = nampla_base + j;

		fprintf (report_file, "\tJump\t%s, PL %s\t; Age %d, ",
		    ship_name (ship), temp_nampla->name, ship->age);

		print_mishap_chance (ship, temp_nampla->x, temp_nampla->y,
			temp_nampla->z);

		fprintf (report_file, "\n\n");

		ship->just_jumped = TRUE;

		continue;
	    }

	    n = ship->unloading_point;
	    if (n)
	    {
		if (n == 9999) n = 0;	/* Home planet. */

		temp_nampla = nampla_base + n;

		fprintf (report_file, "\tJump\t%s, PL %s\t; ", ship_name (ship),
			temp_nampla->name);

		print_mishap_chance (ship, temp_nampla->x, temp_nampla->y,
			temp_nampla->z);

		fprintf (report_file, "\n\n");

		ship->just_jumped = TRUE;
	    }
	}

	if (! species->auto_orders) goto jump_end;

	/* Generate JUMP orders for all ships that have not yet been
		given orders. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;
	    if (ship->pn == 99) continue;
	    if (ship->just_jumped) continue;
	    if (ship->status == UNDER_CONSTRUCTION) continue;
	    if (ship->status == JUMPED_IN_COMBAT) continue;
	    if (ship->status == FORCED_JUMP) continue;

	    if (ship->type == FTL)
	    {
		fprintf (report_file, "\tJump\t%s, ", ship_name (ship));
		if (ship->class == TR  &&  ship->tonnage == 1)
		{
		    closest_unvisited_star (ship);
		    fprintf (report_file,
			"\n\t\t\t; Age %d, now at %d %d %d, ",
			ship->age, ship->x, ship->y, ship->z);

		    if (ship->status == IN_ORBIT)
			fprintf (report_file, "O%d, ", ship->pn);
		    else if (ship->status == ON_SURFACE)
			fprintf (report_file, "L%d, ", ship->pn);
		    else
			fprintf (report_file, "D, ");

		    print_mishap_chance (ship, x, y, z);
		}
		else
		{
		    fprintf (report_file,
			"???\t; Age %d, now at %d %d %d",
			ship->age, ship->x, ship->y, ship->z);

		    if (ship->status == IN_ORBIT)
			fprintf (report_file, ", O%d", ship->pn);
		    else if (ship->status == ON_SURFACE)
			fprintf (report_file, ", L%d", ship->pn);
		    else
			fprintf (report_file, ", D");

		    x = 9999;
		}

		fprintf (report_file, "\n");

		/* Save destination so that we can check later if it needs
		   to be scanned. */
		if (x == 9999)
		    ship->dest_x = -1;
		else
		{
		    ship->dest_x = x;
		    ship->dest_y = y;
		    ship->dest_z = z;
		}
	    }
	}

jump_end:
	fprintf (report_file, "END\n\n");

	fprintf (report_file, "START PRODUCTION\n\n");

	fprintf (report_file, ";   Economic units at start of turn = %ld\n\n",
		species->econ_units);

	/* Generate a PRODUCTION order for each planet that can produce. */
	for (nampla_index = species->num_namplas - 1; nampla_index >= 0;
		nampla_index--)
	{
	    nampla = nampla1_base + nampla_index;
	    if (nampla->pn == 99) continue;

	    if (nampla->mi_base == 0  &&  (nampla->status & RESORT_COLONY) == 0) continue;
	    if (nampla->ma_base == 0  &&  (nampla->status & MINING_COLONY) == 0) continue;

	    fprintf (report_file, "    PRODUCTION PL %s\n", nampla->name);

	    if (nampla->status & MINING_COLONY)
	    {
		fprintf (report_file,
		    "    ; The above PRODUCTION order is required for this mining colony, even\n");
		fprintf (report_file,
		    "    ;  if no other production orders are given for it. This mining colony\n");
		fprintf (report_file,
		    "    ;  will generate %ld economic units this turn.\n", nampla->use_on_ambush);
	    }
	    else if (nampla->status & RESORT_COLONY)
	    {
		fprintf (report_file,
		    "    ; The above PRODUCTION order is required for this resort colony, even\n");
		fprintf (report_file,
		    "    ;  though no other production orders can be given for it.  This resort\n");
		fprintf (report_file,
		    "    ;  colony will generate %ld economic units this turn.\n", nampla->use_on_ambush);
	    }
	    else
	    {
		fprintf (report_file,
		    "    ; Place production orders here for planet %s",
		    nampla->name);
		fprintf (report_file, " (sector %d %d %d #%d).\n", nampla->x,
		    nampla->y, nampla->z, nampla->pn);
		fprintf (report_file,
		    "    ;  Avail pop = %ld, shipyards = %d, to spend = %ld",
		    nampla->pop_units, nampla->shipyards, nampla->use_on_ambush);

		n = nampla->use_on_ambush;
		if (nampla->status & HOME_PLANET)
		{
		    if (species->hp_original_base != 0)
			fprintf (report_file, " (max = %ld)", 5*n);
		    else
			fprintf (report_file, " (max = no limit)");
		}
		else
		    fprintf (report_file, " (max = %ld)", 2*n);

		fprintf (report_file, ".\n\n");
	    }

	    /* Build IUs and AUs for incoming ships with CUs. */
	    if (nampla->IUs_needed)
		fprintf (report_file, "\tBuild\t%d IU\n", nampla->IUs_needed);
	    if (nampla->AUs_needed)
		fprintf (report_file, "\tBuild\t%d AU\n", nampla->AUs_needed);
	    if (nampla->IUs_needed  ||  nampla->AUs_needed)
		fprintf (report_file, "\n");

	    if (! species->auto_orders) continue;
	    if (nampla->status & MINING_COLONY) continue;
	    if (nampla->status & RESORT_COLONY) continue;

	    /* See if there are any RMs to recycle. */
	    n = nampla->special / 5;
	    if (n > 0)
		fprintf (report_file, "\tRecycle\t%d RM\n\n", 5*n);

	    /* Generate DEVELOP commands for ships arriving here because of
		AUTO command. */
	    for (i = 0; i < species->num_ships; i++)
	    {
		ship = ship_base + i;
		if (ship->pn == 99) continue;

		k = ship->special;
		if (k == 0) continue;
		if (k == 9999) k = 0;	/* Home planet. */

		if (nampla != nampla_base + k) continue;

		k = ship->unloading_point;
		if (k == 9999) k = 0;
		temp_nampla = nampla_base + k;

		fprintf (report_file, "\tDevelop\tPL %s, TR%d%s %s\n\n",
		    temp_nampla->name, ship->tonnage, ship_type[ship->type],
		    ship->name);
	    }

	    /* Give orders to continue construction of unfinished ships and
		starbases. */
	    for (i = 0; i < species->num_ships; i++)
	    {
		ship = ship_base + i;
		if (ship->pn == 99) continue;

		if (ship->x != nampla->x) continue;
		if (ship->y != nampla->y) continue;
		if (ship->z != nampla->z) continue;
		if (ship->pn != nampla->pn) continue;

		if (ship->status == UNDER_CONSTRUCTION)
		{
		    fprintf (report_file,
			"\tContinue\t%s, %d\t; Left to pay = %d\n\n",
			ship_name (ship), ship->remaining_cost,
			ship->remaining_cost);

		    continue;
		}

		if (ship->type != STARBASE) continue;

		j = (species->tech_level[MA] / 2) - ship->tonnage;
		if (j < 1) continue;

		fprintf (report_file,
		    "\tContinue\tBAS %s, %d\t; Current tonnage = %s\n\n",
		    ship->name, 100 * j, commas (10000 * (long) ship->tonnage));
	    }

	    /* Generate DEVELOP command if this is a colony with an economic
		base less than 200. */
	    n = nampla->mi_base + nampla->ma_base + nampla->IUs_needed
			+ nampla->AUs_needed;
	    nn = nampla->item_quantity[CU];
	    for (i = 0; i < species->num_ships; i++)
	    {
		/* Get CUs on transports at planet. */
		ship = ship_base + i;
		if (ship->x != nampla->x) continue;
		if (ship->y != nampla->y) continue;
		if (ship->z != nampla->z) continue;
		if (ship->pn != nampla->pn) continue;
		nn += ship->item_quantity[CU];
	    }
	    n += nn;
	    if ((nampla->status & COLONY)  &&  n < 2000L
		&&  nampla->pop_units > 0)
	    {
		if (nampla->pop_units > (2000L-n))
		    nn = 2000L-n;
		else
		    nn = nampla->pop_units;

		fprintf (report_file, "\tDevelop\t%ld\n\n", 2L*nn);

		nampla->IUs_needed += nn;
	    }

	    /* For home planets and any colonies that have an economic base of
		at least 200, check if there are other colonized planets in
		the same sector that are not self-sufficient.  If so, DEVELOP
		them. */
	    if (n >= 2000L  ||  (nampla->status & HOME_PLANET))
	    {
		/* Skip home planet. */
		for (i = 1; i < species->num_namplas; i++)
		{
		    if (i == nampla_index) continue;

		    temp_nampla = nampla_base + i;

		    if (temp_nampla->pn == 99) continue;
		    if (temp_nampla->x != nampla->x) continue;
		    if (temp_nampla->y != nampla->y) continue;
		    if (temp_nampla->z != nampla->z) continue;

		    n = temp_nampla->mi_base + temp_nampla->ma_base
			+ temp_nampla->IUs_needed + temp_nampla->AUs_needed;

		    if (n == 0) continue;

		    nn = temp_nampla->item_quantity[IU]
			+ temp_nampla->item_quantity[AU];
		    if (nn > temp_nampla->item_quantity[CU])
			nn = temp_nampla->item_quantity[CU];
		    n += nn;
		    if (n >= 2000L) continue;
		    nn = 2000L - n;

		    if (nn > nampla->pop_units) nn = nampla->pop_units;

		    fprintf (report_file, "\tDevelop\t%ld\tPL %s\n\n",
			2L * nn, temp_nampla->name);

		    temp_nampla->AUs_needed += nn;
		}
	    }
	}

	fprintf (report_file, "END\n\n");

	fprintf (report_file, "START POST-ARRIVAL\n");
	fprintf (report_file, "; Place post-arrival orders here.\n\n");

	if (! species->auto_orders) goto post_end;

	/* Generate an AUTO command. */
	fprintf (report_file, "\tAuto\n\n");

	/* Generate SCAN orders for all TR1s that are jumping to
	   sectors which current species does not inhabit. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;
	    if (ship->pn == 99) continue;
	    if (ship->status == UNDER_CONSTRUCTION) continue;
	    if (ship->class != TR) continue;
	    if (ship->tonnage != 1) continue;
	    if (ship->type != FTL) continue;

	    found = FALSE;
	    for (j = 0; j < species->num_namplas; j++)
	    {
		if (ship->dest_x == -1) break;

		nampla = nampla_base + j;
		if (nampla->pn == 99) continue;
		if (nampla->x != ship->dest_x) continue;
		if (nampla->y != ship->dest_y) continue;
		if (nampla->z != ship->dest_z) continue;

		if (nampla->status & POPULATED)
		{
		    found = TRUE;
		    break;
		}
	    }
	    if (! found) fprintf (report_file, "\tScan\tTR1 %s\n", ship->name);
	}

post_end:
	fprintf (report_file, "END\n\n");

	fprintf (report_file, "START STRIKES\n");
	fprintf (report_file, "; Place strike orders here.\n\n");
	fprintf (report_file, "END\n");

	truncate_name = FALSE;
	ignore_field_distorters = temp_ignore_field_distorters;

  done_report:

	/* Clean up for this species. */
	fclose (report_file);
    }

    /* Clean up and exit. */
    free_species_data ();
    exit (0);
}



do_planet_report (nampla, s_base, species)

struct species_data	*species;
struct nampla_data	*nampla;
struct ship_data	*s_base;

{
    int		i, j, ship_index, header_printed, ls_needed, production_penalty;

    long	n1, n2, n3, raw_material_units, production_capacity,
		available_to_spend, n, ib, ab, current_base, md, denom;

    struct ship_data	*ship;


    /* Print type of planet, name and coordinates. */
    fprintf (report_file, "\n\n");

    if (nampla->status & HOME_PLANET)
	fprintf (report_file, "HOME PLANET");
    else if (nampla->status & MINING_COLONY)
	fprintf (report_file, "MINING COLONY");
    else if (nampla->status & RESORT_COLONY)
	fprintf (report_file, "RESORT COLONY");
    else if (nampla->status & POPULATED)
	fprintf (report_file, "COLONY PLANET");
    else fprintf (report_file, "PLANET");

    fprintf (report_file, ": PL %s", nampla->name);

    fprintf (report_file,
	"\n   Coordinates: x = %d, y = %d, z = %d, planet number %d\n",
	nampla->x, nampla->y, nampla->z, nampla->pn);

    if (nampla->status & HOME_PLANET)
    {
      ib = nampla->mi_base;
      ab = nampla->ma_base;
      current_base = ib + ab;
      if (current_base < species->hp_original_base)
      {
	n = species->hp_original_base - current_base;	/* Number of CUs needed. */

	md = home_planet->mining_difficulty;

	denom = 100 + md;
	j = (100 * (n + ib) - (md * ab) + denom/2) / denom;
	i = n - j;

	if (i < 0)
	{
	    j = n;
	    i = 0;
	}
	if (j < 0)
	{
	    i = n;
	    j = 0;
	}

	fprintf (report_file,
	    "\nWARNING! Home planet has not yet completely recovered from bombardment!\n");
	fprintf (report_file,
	    "         %d IUs and %d AUs will have to be installed for complete recovery.\n",
	    i, j);
      }
    }

    if ( ! (nampla->status & POPULATED) )
	goto do_inventory;

    /* Print available population. */
    if (nampla->status & (MINING_COLONY | RESORT_COLONY))
	;
    else
	fprintf (report_file, "\nAvailable population units = %ld\n",
		nampla->pop_units);

    if (nampla->siege_eff != 0)
    {
	fprintf (report_file,
	  "\nWARNING!  This planet is currently under siege and will remain\n");
	fprintf (report_file,
	  "  under siege until the combat phase of the next turn!\n");
    }

    if (nampla->use_on_ambush > 0)
	fprintf (report_file,
	  "\nIMPORTANT!  This planet has made preparations for an ambush!\n");

    if (nampla->hidden)
	fprintf (report_file,
	  "\nIMPORTANT!  This planet is actively hiding from alien observation!\n");

    /* Print what will be produced this turn. */
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

    fprintf (report_file, "\nProduction penalty = %d%% (LSN = %d)\n",
	production_penalty, ls_needed);

    fprintf (report_file, "\nEconomic efficiency = %d%%\n",
	planet->econ_efficiency);

    raw_material_units
	-= (production_penalty * raw_material_units) / 100;

    raw_material_units
	= (((long) planet->econ_efficiency * raw_material_units) + 50) / 100;

    production_capacity
	-= (production_penalty * production_capacity) / 100;

    production_capacity
	= (((long) planet->econ_efficiency * production_capacity) + 50) / 100;

    if (nampla->mi_base > 0)
    {
	fprintf (report_file, "\nMining base = %d.%d", nampla->mi_base/10,
				nampla->mi_base%10);
	fprintf (report_file, " (MI = %d, MD = %d.%02d)\n",
	    species->tech_level[MI], planet->mining_difficulty/100,
	    planet->mining_difficulty%100);

	/* For mining colonies, print economic units that will be produced. */
	if (nampla->status & MINING_COLONY)
	{
	    n1 = (2 * raw_material_units) / 3;
	    n2 = ((fleet_percent_cost * n1) + 5000) / 10000;
	    n3 = n1 - n2;
	    fprintf (report_file,
		"   This mining colony will generate %ld - %ld = %ld economic units this turn.\n",
		n1, n2, n3);

	    nampla->use_on_ambush = n3;		/* Temporary use only. */
	}
        else
	{
	    fprintf (report_file,
		"   %ld raw material units will be produced this turn.\n",
	    		raw_material_units);
	}
    }

    if (nampla->ma_base > 0)
    {
	if (nampla->status & RESORT_COLONY) fprintf (report_file, "\n");

	fprintf (report_file, "Manufacturing base = %d.%d",
		nampla->ma_base/10, nampla->ma_base%10);
	fprintf (report_file, " (MA = %d)\n", species->tech_level[MA]);

	/* For resort colonies, print economic units that will be produced. */
	if (nampla->status & RESORT_COLONY)
	{
	    n1 = (2 * production_capacity) / 3;
	    n2 = ((fleet_percent_cost * n1) + 5000) / 10000;
	    n3 = n1 - n2;
	    fprintf (report_file,
		"   This resort colony will generate %ld - %ld = %ld economic units this turn.\n",
		n1, n2, n3);

	    nampla->use_on_ambush = n3;		/* Temporary use only. */
	}
        else
	{
	    fprintf (report_file,
		"   Production capacity this turn will be %ld.\n",
			production_capacity);
	}
    }

    if (nampla->item_quantity[RM] > 0)
	fprintf (report_file, "\n%ss (%s,C%d) carried over from last turn = %ld\n",
	    item_name[RM], item_abbr[RM], item_carry_capacity[RM],
	    nampla->item_quantity[RM]);

    /* Print what can be spent this turn. */
    raw_material_units += nampla->item_quantity[RM];
    if (raw_material_units > production_capacity)
    {
	available_to_spend = production_capacity;
	nampla->special = raw_material_units - production_capacity;
	    /* Excess raw material units that may be recycled in AUTO mode. */
    }
    else
    {
	available_to_spend = raw_material_units;
	nampla->special = 0;
    }

    /* Don't print spendable amount for mining and resort colonies. */
    n1 = available_to_spend;
    n2 = ((fleet_percent_cost * n1) + 5000) / 10000;
    n3 = n1 - n2;
    if ( ! (nampla->status & MINING_COLONY)
		&& ! (nampla->status & RESORT_COLONY))
    {
	fprintf (report_file,
	    "\nTotal available for spending this turn = %ld - %ld = %ld\n",
		n1, n2, n3);
	nampla->use_on_ambush = n3;	/* Temporary use only. */

	fprintf (report_file,
	    "\nShipyard capacity = %d\n", nampla->shipyards);
    }

do_inventory:

    header_printed = FALSE;

    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (nampla->item_quantity[i] > 0  &&  i != RM)
	{
	    if (! header_printed)
	    {
		header_printed = TRUE;
		fprintf (report_file, "\nPlanetary inventory:\n");
	    }

	    fprintf (report_file, "   %ss (%s,C%d) = %d",
			item_name[i], item_abbr[i],
			item_carry_capacity[i], nampla->item_quantity[i]);
	    if (i == PD)
		fprintf (report_file, " (warship equivalence = %ld tons)",
			50 * nampla->item_quantity[PD]);
	    fprintf (report_file, "\n");
	}
    }

    /* Print all ships that are under construction on, on the surface of,
	or in orbit around this planet. */
    printing_alien = FALSE;
    header_printed = FALSE;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	ship = s_base + ship_index;

	if (nampla->x != ship->x) continue;
	if (nampla->y != ship->y) continue;
	if (nampla->z != ship->z) continue;
	if (nampla->pn != ship->pn) continue;
	if (ship->class != BA) continue;

	if (! header_printed)
	{
	    fprintf (report_file, "\nShips at PL %s:\n", nampla->name);
	    print_ship_header();
	}
	header_printed = TRUE;

	print_ship (ship, species, species_number);

	ship_already_listed[ship_index] = TRUE;
    }

    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	ship = s_base + ship_index;

	if (nampla->x != ship->x) continue;
	if (nampla->y != ship->y) continue;
	if (nampla->z != ship->z) continue;
	if (nampla->pn != ship->pn) continue;
	if (ship->class != TR) continue;

	if (! header_printed)
	{
	    fprintf (report_file, "\nShips at PL %s:\n", nampla->name);
	    print_ship_header();
	}
	header_printed = TRUE;

	print_ship (ship, species, species_number);

	ship_already_listed[ship_index] = TRUE;
    }

    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	ship = s_base + ship_index;

	if (nampla->x != ship->x) continue;
	if (nampla->y != ship->y) continue;
	if (nampla->z != ship->z) continue;
	if (nampla->pn != ship->pn) continue;
	if (ship_already_listed[ship_index]) continue;

	if (! header_printed)
	{
	    fprintf (report_file, "\nShips at PL %s:\n", nampla->name);
	    print_ship_header();
	}
	header_printed = TRUE;

	print_ship (ship, species, species_number);

	ship_already_listed[ship_index] = TRUE;
    }
}



print_ship_header ()
{
    fprintf (report_file, "  Name                          ");
    if (printing_alien)
	fprintf (report_file, "                     Species\n");
    else
	fprintf (report_file, "                 Cap. Cargo\n");
    fprintf (report_file, " ---------------------------------------");
    fprintf (report_file, "-------------------------------------\n");
}


extern char	full_ship_id[64];

print_ship (ship, species, species_number)

struct species_data	*species;
struct ship_data	*ship;
int			species_number;

{
    int		i, n, length, capacity, need_comma;


    if (printing_alien)
	ignore_field_distorters = FALSE;
    else
	ignore_field_distorters = TRUE;

    fprintf (report_file, "  %s", ship_name (ship));

    length = strlen (full_ship_id);
    if (printing_alien)
	n = 50;
    else
	n = 46;

    for (i = 0; i < (n - length); i++)
	putc (' ', report_file);

    if (ship->class == BA)
	capacity = 10 * (int) ship->tonnage;
    else if (ship->class == TR)
	capacity = (10 + ((int) ship->tonnage / 2)) * (int) ship->tonnage;
    else
	capacity = ship->tonnage;

    if (printing_alien)
	fprintf (report_file, " ");
    else
    {
	fprintf (report_file, "%4d  ", capacity);
	if (ship->status == UNDER_CONSTRUCTION)
	{
	    fprintf (report_file, "Left to pay = %d\n", ship->remaining_cost);
	    return;
	}
    }

    if (printing_alien)
    {
	if (ship->status == ON_SURFACE
		||  ship->item_quantity[FD] != ship->tonnage)
	    fprintf (report_file, "SP %s", species->name);
	else
	    fprintf (report_file, "SP %d", distorted (species_number));
    }
    else
    {
	need_comma = FALSE;
	for (i = 0; i < MAX_ITEMS; i++)
	{
	    if (ship->item_quantity[i] > 0)
	    {
		if (need_comma) putc (',', report_file);
		fprintf (report_file, "%d %s",
		    ship->item_quantity[i], item_abbr[i]);
		need_comma = TRUE;
	    }
	}
    }

    putc ('\n', report_file);
}


print_mishap_chance (ship, destx, desty, destz)

struct ship_data	*ship;
int			destx, desty, destz;

{
    int		mishap_GV, mishap_age;

    long	x, y, z, mishap_chance, success_chance;


    if (destx == 9999)
    {
	fprintf (report_file, "Mishap chance = ???");
	return;
    }

    mishap_GV = species->tech_level[GV];
    mishap_age = ship->age;

    x = destx;
    y = desty;
    z = destz;
    mishap_chance = ( 100 * (
	((x - ship->x) * (x - ship->x))
      + ((y - ship->y) * (y - ship->y))
      + ((z - ship->z) * (z - ship->z))
    	) ) / mishap_GV;

    if (mishap_age > 0  &&  mishap_chance < 10000)
    {
	success_chance = 10000L - mishap_chance;
	success_chance -= (2L * (long) mishap_age * success_chance)/100L;
	mishap_chance = 10000L - success_chance;
    }

    if (mishap_chance > 10000) mishap_chance = 10000;

    fprintf (report_file, "mishap chance = %ld.%02ld%%",
	mishap_chance/100L, mishap_chance%100L);
}


closest_unvisited_star (ship)

struct ship_data	*ship;

{
    int		i, found, species_array_index, species_bit_number;

    long	shx, shy, shz, stx, sty, stz, closest_distance, temp_distance,
		species_bit_mask;

    struct star_data	*star, *closest_star;


    /* Get array index and bit mask. */
    species_array_index = (species_number - 1) / 32;
    species_bit_number = (species_number - 1) % 32;
    species_bit_mask = 1 << species_bit_number;

    shx = ship->x;
    shy = ship->y;
    shz = ship->z;

    x = 9999;
    closest_distance = 999999;

    found = FALSE;
    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	/* Check if bit is already set. */
	if (star->visited_by[species_array_index] & species_bit_mask) continue;

	stx = star->x;
	sty = star->y;
	stz = star->z;

	temp_distance =
		((shx - stx) * (shx - stx))
	      + ((shy - sty) * (shy - sty))
	      + ((shz - stz) * (shz - stz));

	if (temp_distance < closest_distance)
	{
	    x = stx;
	    y = sty;
	    z = stz;
	    closest_distance = temp_distance;
	    closest_star = star;
	    found = TRUE;
	}
    }

    if (found)
    {
	fprintf (report_file, "%d %d %d", x, y, z);
	closest_star->visited_by[species_array_index] |= species_bit_mask;
	    /* So that we don't send more than one ship to the same place. */
    }
    else
	fprintf (report_file, "???");
}
