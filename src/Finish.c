
/*
   This program should be run immediately before running the Report program;
   i.e. immediately after the last run of AddSpecies in the very first turn, or
   immediately after running PostArrival on all subsequent turns.  This program
   will create the file 'locations.dat' (via the do_locations subroutine),
   update populations, handle interspecies transactions, and do some other
   housekeeping chores.
*/


#define THIS_IS_MAIN

#include "fh.h"


int			species_number, species_index, header_printed;
int			test_mode, verbose_mode;

struct galaxy_data	galaxy;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern int		log_stdout, num_planets, num_transactions, num_locs;
extern unsigned long	last_random;
extern FILE		*log_file;

extern struct planet_data	*planet_base;
extern struct trans_data	transaction[MAX_TRANSACTIONS];
extern struct sp_loc_data	loc[MAX_LOCATIONS];


main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, n, rec, don, nampla_index, ship_index, ls_needed,
		ls_actual, tech, turn_number, percent_increase, old_tech_level,
		new_tech_level, experience_points, their_level, my_level,
		new_level, orders_received, contact_bit_number,
		contact_word_number, alien_number, galaxy_fd,
		production_penalty, max_tech_level;

    short	ns;

    long	change, total_pop_units, contact_mask, salvage_EUs,
		salvage_value, original_cost, ib, ab, increment, old_base,
		max_cost, actual_cost, one_point_cost, working_pop_units,
		ib_increment, ab_increment, md, growth_factor, denom,
		fleet_maintenance_cost, balance, total_species_production,
		RMs_produced, production_capacity, diff, total, eb,
		*total_econ_base;

    char	filename[32], *dest, *src;

    struct planet_data		*home_planet;
    struct species_data		*donor_species;
    struct nampla_data		*home_nampla;


    /* Check for options, if any. */
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-t") == 0) test_mode = TRUE;
      if (strcmp (argv[i], "-v") == 0) verbose_mode = TRUE;
    }

    /* Seed random number generator. */
    last_random = time(NULL);
    n = 907;
    for (i = 0; i < n; i++) rnd(100);

    /* Get commonly used data. */
    get_galaxy_data ();
    get_planet_data ();
    get_species_data ();
    get_transaction_data ();
    num_locs = 0;

    /* Allocate memory for array "total_econ_base". */
    total = (long) num_planets * sizeof (long);
    total_econ_base = (long *) malloc (total);
    if (total_econ_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for total_econ_base!\n\n");
	exit (-1);
    }

    /* Handle turn number. */
    turn_number = ++galaxy.turn_number;
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

    /* Do mining difficulty increases and initialize total economic base
	for each planet. */
    planet = planet_base;
    for (i = 0; i < num_planets; i++)
    {
	planet->mining_difficulty += planet->md_increase;
	planet->md_increase = 0;

	total_econ_base[i] = 0;

	++planet;
    }

    /* Main loop. For each species, take appropriate action. */
    if (verbose_mode) printf ("\nFinishing up for all species...\n");
    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	if (! data_in_memory[species_number - 1]) continue;

	data_modified[species_number - 1] = TRUE;

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];
	ship_base = ship_data[species_number - 1];

	/* Check if player submitted orders for this turn. */
	sprintf (filename, "sp%02d.ord\0", species_number);
	i = open (filename, 0);
	if (i < 0)
	    orders_received = FALSE;
	else
	{
	    orders_received = TRUE;
	    close (i);
	}
	if (turn_number == 1) orders_received = TRUE;

	/* Display name of species. */
	if (verbose_mode)
	{
	  printf ("  Now doing SP %s...", species->name);
	  if (! orders_received)
	    printf (" WARNING: player did not submit orders this turn!");
	  printf ("\n");
	}

	/* Open log file for appending. */
	sprintf (filename, "sp%02d.log\0", species_number);
	log_file = fopen (filename, "a");
	if (log_file == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
	    exit (-1);
	}
	log_stdout = FALSE;
	header_printed = FALSE;

	if (turn_number == 1) goto check_for_message;

	/* Check if any ships of this species experienced mishaps. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == SHIP_MISHAP
		&&  transaction[i].number1 == species_number)
	    {
		if (! header_printed) print_header();
		log_string ("  !!! ");
		log_string (transaction[i].name1);
		if (transaction[i].value < 3)
		{
		    /* Intercepted or self-destructed. */
		    log_string (" disappeared without a trace, cause unknown!\n");
		}
		else if (transaction[i].value == 3)
		{
		    /* Mis-jumped. */
		    log_string (" mis-jumped to ");
		    log_int (transaction[i].x);  log_char (' ');
		    log_int (transaction[i].y);  log_char (' ');
		    log_int (transaction[i].z);  log_string ("!\n");
		}
		else
		{
		    /* One fail-safe jump unit used. */
		    log_string (" had a jump mishap! A fail-safe jump unit was expended.\n");
		}
	    }
	}

	/* Take care of any disbanded colonies. */
	home_nampla = nampla_base;
	nampla = nampla_base - 1;
	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    ++nampla;

	    if ((nampla->status & DISBANDED_COLONY) == 0) continue;

	    /* Salvage ships on the surface and starbases in orbit. */
	    salvage_EUs = 0;
	    ship = ship_base - 1;
	    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	    {
		++ship;

		if (nampla->x != ship->x) continue;
		if (nampla->y != ship->y) continue;
		if (nampla->z != ship->z) continue;
		if (nampla->pn != ship->pn) continue;
		if (ship->type != STARBASE  &&  ship->status == IN_ORBIT)
		    continue;

		/* Transfer cargo to planet. */
		for (i = 0; i < MAX_ITEMS; i++)
		    nampla->item_quantity[i] += ship->item_quantity[i];

		/* Salvage the ship. */
		if (ship->class == TR  ||  ship->type == STARBASE)
		    original_cost = ship_cost[ship->class] * ship->tonnage;
		else
		    original_cost = ship_cost[ship->class];

		if (ship->type == SUB_LIGHT)
		    original_cost = (3 * original_cost) / 4;

		if (ship->status == UNDER_CONSTRUCTION)
		    salvage_value =
			(original_cost - (long) ship->remaining_cost) / 4;
		else
		    salvage_value =
			(3 * original_cost * (60 - (long) ship->age)) / 400;

		salvage_EUs += salvage_value;

		/* Destroy the ship. */
		delete_ship (ship);
	    }

	    /* Salvage items on the planet. */
	    for (i = 0; i < MAX_ITEMS; i++)
	    {
		if (i == RM)
		    salvage_value = nampla->item_quantity[RM] / 10;
		else if (nampla->item_quantity[i] > 0)
		{
		    original_cost = nampla->item_quantity[i] * item_cost[i];
		    if (i == TP)
		    {
		      if (species->tech_level[BI] > 0)
			original_cost /= (long) species->tech_level[BI];
		      else
			original_cost /= 100;
		    }
		    salvage_value = original_cost / 4; 
		}
		else
		  salvage_value = 0;

		salvage_EUs += salvage_value;
	    }

	    /* Transfer EUs to species. */
	    species->econ_units += salvage_EUs;

	    /* Log what happened. */
	    if (! header_printed) print_header();
	    log_string ("  PL ");
	    log_string (nampla->name);
	    log_string (" was disbanded, generating ");
	    log_long (salvage_EUs);
	    log_string (" economic units in salvage.\n");

	    /* Destroy the colony. */
	    delete_nampla (nampla);
	}

	/* Check if this species is the recipient of a transfer of economic
		units from another species. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].recipient == species_number &&
		(transaction[i].type == EU_TRANSFER
		||  transaction[i].type == SIEGE_EU_TRANSFER
		||  transaction[i].type == LOOTING_EU_TRANSFER))
	    {
		/* Transfer EUs to attacker if this is a siege or looting
		   transfer. If this is a normal transfer, then just log
		   the result since the actual transfer was done when the
		   order was processed. */
		if (transaction[i].type != EU_TRANSFER)
		    species->econ_units += transaction[i].value;

		if (! header_printed) print_header();
		log_string ("  ");
		log_long (transaction[i].value);
		log_string (" economic units were received from SP ");
		log_string (transaction[i].name1);
		if (transaction[i].type == SIEGE_EU_TRANSFER)
		{
		    log_string (" as a result of your successful siege of their PL ");
		    log_string (transaction[i].name3);
		    log_string (". The siege was ");
		    log_long (transaction[i].number1);
		    log_string ("% effective");
		}
		else if (transaction[i].type == LOOTING_EU_TRANSFER)
		{
		    log_string (" as a result of your looting their PL ");
		    log_string (transaction[i].name3);
		}
		log_string (".\n");
	    }
	}

	/* Check if any jump portals of this species were used by aliens. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == ALIEN_JUMP_PORTAL_USAGE
		&&  transaction[i].number1 == species_number)
	    {
		if (! header_printed) print_header();
		log_string ("  ");
		log_string (transaction[i].name1);
		log_char (' ');
		log_string (transaction[i].name2);
		log_string (" used jump portal ");
		log_string (transaction[i].name3);
		log_string (".\n");
	    }
	}

	/* Check if any starbases of this species detected the use of gravitic
		telescopes by aliens. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == TELESCOPE_DETECTION
		&&  transaction[i].number1 == species_number)
	    {
		if (! header_printed) print_header();
		log_string ("! ");
		log_string (transaction[i].name1);
		log_string (" detected the operation of an alien gravitic telescope at x = ");
		log_int (transaction[i].x);
		log_string (", y = ");    log_int (transaction[i].y);
		log_string (", z = ");    log_int (transaction[i].z);
		log_string (".\n");
	    }
	}

	/* Check if this species is the recipient of a tech transfer from
		another species. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == TECH_TRANSFER
		&&  transaction[i].recipient == species_number)
	    {
		rec = transaction[i].recipient - 1;
		don = transaction[i].donor - 1;

		/* Try to transfer technology. */
		if (! header_printed) print_header();
		log_string ("  ");
		tech = transaction[i].value;
		log_string (tech_name[tech]);
		log_string (" tech transfer from SP ");
		log_string (transaction[i].name1);
		their_level = transaction[i].number3;
		my_level = species->tech_level[tech];

		if (their_level <= my_level)
		{
		    log_string (" failed.\n");
		    transaction[i].number1 = -1;
		    continue;
		}

		new_level = my_level;
		max_cost = transaction[i].number1;
		donor_species = &spec_data[don];
		if (max_cost == 0)
		    max_cost = donor_species->econ_units;
		else if (donor_species->econ_units < max_cost)
		    max_cost = donor_species->econ_units;
		actual_cost = 0;
		while (new_level < their_level)
		{
		    one_point_cost = new_level * new_level;
		    one_point_cost -= one_point_cost/4;  /* 25% discount. */
		    if ((actual_cost + one_point_cost) > max_cost) break;
		    actual_cost += one_point_cost;
		    ++new_level;
		}

		if (new_level == my_level)
		{
		    log_string (" failed due to lack of funding.\n");
		    transaction[i].number1 = -2;
		}
		else
		{
		    log_string (" raised your tech level from ");
		    log_int (my_level);
		    log_string (" to ");
		    log_int (new_level);
		    log_string (" at a cost to them of ");
		    log_long (actual_cost);
		    log_string (".\n");
		    transaction[i].number1 = actual_cost;
		    transaction[i].number2 = my_level;
		    transaction[i].number3 = new_level;

		    species->tech_level[tech] = new_level;
		    donor_species->econ_units -= actual_cost;
		}
	    }
	}

	/* Calculate tech level increases. */
	for (tech = MI; tech <= BI; tech++)
	{
	    old_tech_level = species->tech_level[tech];
	    new_tech_level = old_tech_level;

	    experience_points = species->tech_eps[tech];
	    if (experience_points == 0) goto check_random;

	    /* Determine increase as if there were NO randomness in the
		process. */
	    i = experience_points;
	    j = old_tech_level;
	    while (i >= j * j)
	    {
		i -= j * j;
		++j;
	    }

	    /* When extremely large amounts are spent on research, tech
		level increases are sometimes excessive.  Set a limit. */
	    if (old_tech_level > 50)
		max_tech_level = j + 1;
	    else
		max_tech_level = 9999;

	    /* Allocate half of the calculated increase NON-RANDOMLY. */
	    n = (j - old_tech_level) / 2;
	    for (i = 0; i < n; i++)
	    {
		experience_points -= new_tech_level * new_tech_level;
		++new_tech_level;
	    }

	    /* Allocate the rest randomly. */
	    while (experience_points >= new_tech_level)
	    {
		experience_points -= new_tech_level;
		n = new_tech_level;

		/* The chance of success is 1 in n. At this point, n is
		   always at least 1. */

		i = rnd (16*n);
		if (i >= 8*n  &&  i <= 8*n+15)
		    new_tech_level = n + 1;
	    }

	    /* Save unused experience points. */
	    species->tech_eps[tech] = experience_points;

check_random:

	    /* See if any random increase occurred. Odds are 1 in 6. */
	    if (old_tech_level > 0  &&  rnd(6) == 6) ++new_tech_level;

	    if (new_tech_level > max_tech_level)
		new_tech_level = max_tech_level;

	    /* Report result only if tech level went up. */
	    if (new_tech_level > old_tech_level)
	    {
		if (! header_printed) print_header();
		log_string ("  ");
		log_string (tech_name[tech]);
		log_string (" tech level rose from ");
		log_int (old_tech_level);  log_string (" to ");
		log_int (new_tech_level);
		log_string (".\n");

		species->tech_level[tech] = new_tech_level;
	    }
	}

	/* Notify of any new high tech items. */
	for (tech = MI; tech <= BI; tech++)
	{
	    old_tech_level = species->init_tech_level[tech];
	    new_tech_level = species->tech_level[tech];

	    if (new_tech_level > old_tech_level)
		check_high_tech_items (tech, old_tech_level, new_tech_level);

	    species->init_tech_level[tech] = new_tech_level;
	}

	/* Check if this species is the recipient of a knowledge transfer
		from another species. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == KNOWLEDGE_TRANSFER
		&&  transaction[i].recipient == species_number)
	    {
		rec = transaction[i].recipient - 1;
		don = transaction[i].donor - 1;

		/* Try to transfer technology. */
		tech = transaction[i].value;
		their_level = transaction[i].number3;
		my_level = species->tech_level[tech];
		n = species->tech_knowledge[tech];
		if (n > my_level) my_level = n;

		if (their_level <= my_level) continue;

		species->tech_knowledge[tech] = their_level;

		if (! header_printed) print_header();
		log_string ("  SP ");
		log_string (transaction[i].name1);
		log_string (" transferred knowledge of ");
		log_string (tech_name[tech]);
		log_string (" to you up to tech level ");
		log_long (their_level);
		log_string (".\n");
	    }
	}

	/* Loop through each nampla for this species. */
	home_nampla = nampla_base;
	home_planet = planet_base + (long) home_nampla->planet_index;
	nampla = nampla_base - 1;
	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;

	    /* Get planet pointer. */
	    planet = planet_base + (long) nampla->planet_index;

	    /* Clear any amount spent on ambush. */
	    nampla->use_on_ambush = 0;

	    /* Handle HIDE order. */
	    nampla->hidden = nampla->hiding;
	    nampla->hiding = FALSE;

	    /* Check if any IUs or AUs were installed. */
	    if (nampla->IUs_to_install > 0)
	    {
		nampla->mi_base += nampla->IUs_to_install;
		nampla->IUs_to_install = 0;
	    }

	    if (nampla->AUs_to_install > 0)
	    {
		nampla->ma_base += nampla->AUs_to_install;
		nampla->AUs_to_install = 0;
	    }

	    /* Check if another species on the same planet has become
		assimilated. */
	    for (i = 0; i < num_transactions; i++)
	    {
		if (transaction[i].type == ASSIMILATION
		    &&  transaction[i].value == species_number
		    &&  transaction[i].x == nampla->x
		    &&  transaction[i].y == nampla->y
		    &&  transaction[i].z == nampla->z
		    &&  transaction[i].pn == nampla->pn)
		{
		    ib = transaction[i].number1;
		    ab = transaction[i].number2;
		    ns = transaction[i].number3;
		    nampla->mi_base += ib;
		    nampla->ma_base += ab;
		    nampla->shipyards += ns;

		    if (! header_printed) print_header();

		    log_string ("  Assimilation of ");
		    log_string (transaction[i].name1);
		    log_string (" PL ");
		    log_string (transaction[i].name2);
		    log_string (" increased mining base of ");
		    log_string (species->name);
		    log_string (" PL ");
		    log_string (nampla->name);
		    log_string (" by ");
		    log_long (ib/10);  log_char ('.');  log_long (ib%10);
		    log_string (", and manufacturing base by ");
		    log_long (ab/10);  log_char ('.');  log_long (ab%10);
		    if (ns > 0)
		    {
			log_string (". Number of shipyards was also increased by ");
			log_int (ns);
		    }
		    log_string (".\n");
		}
	    }

	    /* Calculate available population for this turn. */
	    nampla->pop_units = 0;

	    eb = nampla->mi_base + nampla->ma_base;
	    total_pop_units = eb + nampla->item_quantity[CU]
		+ nampla->item_quantity[PD];

	    if (nampla->status & HOME_PLANET)
	    {
		if (nampla->status & POPULATED)
		{
		    nampla->pop_units = HP_AVAILABLE_POP;

		    if (species->hp_original_base != 0)  /* HP was bombed. */
		    {
			if (eb >= species->hp_original_base)
			    species->hp_original_base = 0; /* Fully recovered. */
			else
			    nampla->pop_units = (eb * HP_AVAILABLE_POP)
				/ species->hp_original_base;
		    }
		}
	    }
	    else if (nampla->status & POPULATED)
	    {
		/* Get life support tech level needed. */
		ls_needed = life_support_needed (species, home_planet, planet);

		/* Basic percent increase is 10*(1 - ls_needed/ls_actual). */
		ls_actual = species->tech_level[LS];
		percent_increase =
		    10 * (100 - ((100 * ls_needed)/ls_actual));

		if (percent_increase < 0)	/* Colony wiped out! */
		{
		    if (! header_printed) print_header();

		    log_string ("  !!! Life support tech level was too low to support colony on PL ");
		    log_string (nampla->name);
		    log_string (". Colony was destroyed.\n");

		    nampla->status = COLONY;  /* No longer populated or
						self-sufficient. */
		    nampla->mi_base = 0;
		    nampla->ma_base = 0;
		    nampla->pop_units = 0;
		    nampla->item_quantity[PD] = 0;
		    nampla->item_quantity[CU] = 0;
		    nampla->siege_eff = 0;
		}
		else
		{
		    percent_increase /= 100;

		    /* Add a small random variation. */
		    percent_increase +=
			rnd(percent_increase/4) - rnd(percent_increase/4);

		    /* Add bonus for Biology technology. */
		    percent_increase += (int) species->tech_level[BI] / 20;

		    /* Calculate and apply the change. */
		    change = (percent_increase * total_pop_units) / 100;

		    if (nampla->mi_base > 0  &&  nampla->ma_base == 0)
		    {
			nampla->status |= MINING_COLONY;
			change = 0;
		    }
		    else if (nampla->status & MINING_COLONY)
		    {
			/* A former mining colony has been converted to a
				normal colony. */
			nampla->status &= ~MINING_COLONY;
			change = 0;
		    }

		    if (nampla->ma_base > 0  &&  nampla->mi_base == 0
			&&  ls_needed <= 6
			&&  planet->gravity <= home_planet->gravity)
		    {
			nampla->status |= RESORT_COLONY;
			change = 0;
		    }
		    else if (nampla->status & RESORT_COLONY)
		    {
			/* A former resort colony has been converted to a
				normal colony. */
			nampla->status &= ~RESORT_COLONY;
			change = 0;
		    }

		    if (total_pop_units == nampla->item_quantity[PD])
			change = 0;	/* Probably an invasion force. */

		    nampla->pop_units = change;
		}
	    }

	    /* Handle losses due to attrition and update location array if
		planet is still populated. */
	    if (nampla->status & POPULATED)
	    {
		total_pop_units = nampla->pop_units + nampla->mi_base
			+ nampla->ma_base + nampla->item_quantity[CU]
			+ nampla->item_quantity[PD];

		if (total_pop_units > 0  &&  total_pop_units < 50)
		{
		    if (nampla->pop_units > 0)
		    {
			--nampla->pop_units;
			goto do_auto_increases;
		    }
		    else if (nampla->item_quantity[CU] > 0)
		    {
			--nampla->item_quantity[CU];
			if (! header_printed) print_header();
			log_string ("  Number of colonist units on PL ");
			log_string (nampla->name);
			log_string (" was reduced by one unit due to normal attrition.");
		    }
		    else if (nampla->item_quantity[PD] > 0)
		    {
			--nampla->item_quantity[PD];
			if (! header_printed) print_header();
			log_string ("  Number of planetary defense units on PL ");
			log_string (nampla->name);
			log_string (" was reduced by one unit due to normal attrition.");
		    }
		    else if (nampla->ma_base > 0)
		    {
			--nampla->ma_base;
			if (! header_printed) print_header();
			log_string ("  Manufacturing base of PL ");
			log_string (nampla->name);
			log_string (" was reduced by 0.1 due to normal attrition.");
		    }
		    else
		    {
			--nampla->mi_base;
			if (! header_printed) print_header();
			log_string ("  Mining base of PL ");
			log_string (nampla->name);
			log_string (" was reduced by 0.1 due to normal attrition.");
		    }

		    if (total_pop_units == 1)
		    {
			if (! header_printed) print_header();
			log_string (" The colony is dead!");
		    }

		    log_char ('\n');
		}
	    }

do_auto_increases:

	    /* Apply automatic 2% increase to mining and manufacturing bases
		of home planets. */
	    if (nampla->status & HOME_PLANET)
	    {
		growth_factor = 20L;
		ib = nampla->mi_base;
		ab = nampla->ma_base;
		old_base = ib + ab;
		increment = (growth_factor * old_base) / 1000;
		md = planet->mining_difficulty;

		denom = 100 + md;
		ab_increment =
		    (100 * (increment + ib) - (md * ab) + denom/2) / denom;
		ib_increment = increment - ab_increment;

		if (ib_increment < 0)
		{
		    ab_increment = increment;
		    ib_increment = 0;
		}
		if (ab_increment < 0)
		{
		    ib_increment = increment;
		    ab_increment = 0;
		}
		nampla->mi_base += ib_increment;
		nampla->ma_base += ab_increment;
	    }

    check_pop:

	    check_population (nampla);

	    /* Update total economic base for colonies. */
	    if ((nampla->status & HOME_PLANET) == 0)
		total_econ_base[nampla->planet_index] +=
			nampla->mi_base + nampla->ma_base;
	}

	/* Loop through all ships for this species. */
	ship = ship_base - 1;
	for (ship_index = 0; ship_index < species->num_ships; ship_index++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;

	    /* Set flag if ship arrived via a natural wormhole. */
	    if (ship->just_jumped == 99)
		ship->arrived_via_wormhole = TRUE;
	    else
		ship->arrived_via_wormhole = FALSE;

	    /* Clear 'just-jumped' flag. */
	    ship->just_jumped = FALSE;

	    /* Increase age of ship. */
	    if (ship->status != UNDER_CONSTRUCTION)
	    {
		ship->age += 1;
		if (ship->age > 49) ship->age = 49;
	    }
	}

	/* Check if this species has a populated planet that another species
		tried to land on. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == LANDING_REQUEST
		&&  transaction[i].number1 == species_number)
	    {
		if (! header_printed) print_header();
		log_string ("  ");
		log_string (transaction[i].name2);
		log_string (" owned by SP ");
		log_string (transaction[i].name3);
		if (transaction[i].value)
		    log_string (" was granted");
		else
		    log_string (" was denied");
		log_string (" permission to land on PL ");
		log_string (transaction[i].name1);
		log_string (".\n");
	    }
	}

	/* Check if this species is the recipient of interspecies
		construction. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == INTERSPECIES_CONSTRUCTION
		&&  transaction[i].recipient == species_number)
	    {
		/* Simply log the result. */
		if (! header_printed) print_header();
		log_string ("  ");
		if (transaction[i].value == 1)
		{
		    log_long (transaction[i].number1);  log_char (' ');
		    log_string (item_name[transaction[i].number2]);
		    if (transaction[i].number1 == 1)
			log_string (" was");
		    else
			log_string ("s were");
		    log_string (" constructed for you by SP ");
		    log_string (transaction[i].name1);
		    log_string (" on PL ");
		    log_string (transaction[i].name2);
		}
		else
		{
		    log_string (transaction[i].name2);
		    log_string (" was constructed for you by SP ");
		    log_string (transaction[i].name1);
		}
		log_string (".\n");
	    }
	}

	/* Check if this species is besieging another species and detects
		forbidden construction, landings, etc. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == DETECTION_DURING_SIEGE
		&&  transaction[i].number3 == species_number)
	    {
		/* Log what was detected and/or destroyed. */
		if (! header_printed) print_header();
		log_string ("  ");
		log_string ("During the siege of ");
		log_string (transaction[i].name3);
		log_string (" PL ");
		log_string (transaction[i].name1);
		log_string (", your forces detected the ");

		if (transaction[i].value == 1)
		{
		    /* Landing of enemy ship. */
		    log_string ("landing of ");
		    log_string (transaction[i].name2);
		    log_string (" on the planet.\n");
		}
		else if (transaction[i].value == 2)
		{
		    /* Enemy ship or starbase construction. */
		    log_string ("construction of ");
		    log_string (transaction[i].name2);
		    log_string (", but you destroyed it before it");
		    log_string (" could be completed.\n");
		}
		else if (transaction[i].value == 3)
		{
		    /* Enemy PD construction. */
		    log_string ("construction of planetary defenses, but you");
		    log_string (" destroyed them before they could be completed.\n");
		}
		else if (transaction[i].value == 4
			||  transaction[i].value == 5)
		{
		    /* Enemy item construction. */
		    log_string ("transfer of ");
		    log_int (transaction[i].number1);
		    log_char (' ');
		    log_string (item_name[transaction[i].number2]);
		    if (transaction[i].number1 > 1) log_char ('s');
		    if (transaction[i].value == 4)
			log_string (" to PL ");
		    else
			log_string (" from PL ");
		    log_string (transaction[i].name2);
		    log_string (", but you destroyed them in transit.\n");
		}
		else
		{
		    fprintf (stderr, "\n\tInternal error!  Cannot reach this point!\n\n");
		    exit (-1);
		}
	    }
	}

check_for_message:

	/* Check if this species is the recipient of a message from another
		species. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == MESSAGE_TO_SPECIES
		&&  transaction[i].number2 == species_number)
	    {
		if (! header_printed) print_header();
		log_string ("\n  You received the following message from SP ");
		log_string (transaction[i].name1);
		log_string (":\n\n");

		sprintf (filename, "m%d.msg\0", (int) transaction[i].value);

		log_message (filename);

		log_string ("\n  *** End of Message ***\n\n");
	    }
	}

	/* Close log file. */
	fclose (log_file);
    }

    /* Calculate economic efficiency for each planet. */
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

    if (turn_number == 1) goto clean_up;

    /* Go through all species one more time to update alien contact masks,
	report tech transfer results to donors, and calculate fleet
	maintenance costs. */
    if (verbose_mode) printf ("\nNow updating contact masks et al.\n");
    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (! data_in_memory[species_index]) continue;

	species = &spec_data[species_index];
	nampla_base = namp_data[species_index];
	ship_base = ship_data[species_index];
	species_number = species_index + 1;

	home_nampla = nampla_base;
	home_planet = planet_base + (long) home_nampla->planet_index;

	/* Update contact mask in species data if this species has met a
		new alien. */
	for (i = 0; i < num_locs; i++)
	{
	    if (loc[i].s != species_number) continue;

	    for (j = 0; j < num_locs; j++)
	    {
		if (loc[j].s == species_number) continue;
		if (loc[j].x != loc[i].x) continue;
		if (loc[j].y != loc[i].y) continue;
		if (loc[j].z != loc[i].z) continue;

		/* We are in contact with an alien. Make sure it is not
		    hidden from us. */
		alien_number = loc[j].s;
		if (alien_is_visible (loc[j].x, loc[j].y, loc[j].z,
			species_number, alien_number))
		{
		    contact_word_number = (loc[j].s - 1) / 32;
		    contact_bit_number = (loc[j].s - 1) % 32;
		    contact_mask = 1 << contact_bit_number;
		    species->contact[contact_word_number] |= contact_mask;
		}
	    }
	}

	/* Report results of tech transfers to donor species. */
	for (i = 0; i < num_transactions; i++)
	{
	    if (transaction[i].type == TECH_TRANSFER
		&&  transaction[i].donor == species_number)
	    {
		/* Open log file for appending. */
		sprintf (filename, "sp%02d.log\0", species_number);
		log_file = fopen (filename, "a");
		if (log_file == NULL)
		{
		    fprintf (stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
		    exit (-1);
		}
		log_stdout = FALSE;

		log_string ("  ");
		tech = transaction[i].value;
		log_string (tech_name[tech]);
		log_string (" tech transfer to SP ");
		log_string (transaction[i].name2);

		if (transaction[i].number1 < 0)
		{
		    log_string (" failed");
		    if (transaction[i].number1 == -2)
			log_string (" due to lack of funding");
		}
		else
		{
		    log_string (" raised their tech level from ");
		    log_long (transaction[i].number2);
		    log_string (" to ");
		    log_long (transaction[i].number3);
		    log_string (" at a cost to you of ");
		    log_long (transaction[i].number1);
		}

		log_string (".\n");

		fclose (log_file);
	    }
	}

	/* Calculate fleet maintenance cost and its percentage of total
	   production. */
	fleet_maintenance_cost = 0;
	ship = ship_base - 1;
	for (i = 0; i < species->num_ships; i++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;

	    if (ship->class == TR)
		n = 4 * (int) ship->tonnage;
	    else if (ship->class == BA)
		n = 10 * (int) ship->tonnage;
	    else
		n = 20 * (int) ship->tonnage;

	    if (ship->type == SUB_LIGHT)
		n -= (25 * n) / 100;

	    fleet_maintenance_cost += n;
	}

	/* Subtract military discount. */
	i = (int) species->tech_level[ML] / 2;
	fleet_maintenance_cost -= (i * fleet_maintenance_cost) / 100;

	/* Calculate total production. */
	total_species_production = 0;
	nampla = nampla_base - 1;
	for (i = 0; i < species->num_namplas; i++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;
	    if (nampla->status & DISBANDED_COLONY) continue;

	    planet = planet_base + (long) nampla->planet_index;

	    ls_needed = life_support_needed (species, home_planet, planet);

	    if (ls_needed == 0)
		production_penalty = 0;
	    else
		production_penalty = (100 * ls_needed) / species->tech_level[LS];

	    RMs_produced =
		(10L * (long) species->tech_level[MI] * (long) nampla->mi_base)
			/ (long) planet->mining_difficulty;
	    RMs_produced
		-= (production_penalty * RMs_produced) / 100;

	    production_capacity =
		((long) species->tech_level[MA] * (long) nampla->ma_base) / 10L;
	    production_capacity
		-= (production_penalty * production_capacity) / 100;

	    if (nampla->status & MINING_COLONY)
		balance = (2 * RMs_produced) / 3;
	    else if (nampla->status & RESORT_COLONY)
		balance = (2 * production_capacity) / 3;
	    else
	    {
		RMs_produced += nampla->item_quantity[RM];
		balance = (RMs_produced > production_capacity)
				? production_capacity : RMs_produced;
	    }

	    balance = (((long) planet->econ_efficiency * balance) + 50) / 100;

	    total_species_production += balance;
	}

	/* If cost is greater than production, take as much as possible
		from EUs in treasury.*
	if (fleet_maintenance_cost > total_species_production)
	{
	    if (fleet_maintenance_cost > species->econ_units)
	    {
		fleet_maintenance_cost -= species->econ_units;
		species->econ_units = 0;
	    {
	    else
	    {
		species->econ_units -= fleet_maintenance_cost;
		fleet_maintenance_cost = 0;
	    }
	}

	/* Save fleet maintenance results. */
	species->fleet_cost = fleet_maintenance_cost;
	if (total_species_production > 0)
	    species->fleet_percent_cost = (10000 * fleet_maintenance_cost)
			/ total_species_production;
	else
	    species->fleet_percent_cost = 10000;
    }

clean_up:

    /* Clean up and exit. */
    save_planet_data ();
    save_location_data ();
    save_species_data ();
    free_species_data ();
    free (planet_base);
    free (total_econ_base);
    exit (0);
}




print_header ()

{
    log_string ("\nOther events:\n");
    header_printed = TRUE;
}



alien_is_visible (x, y, z, species_number, alien_number)

char	x, y, z;
int	species_number, alien_number;

{

    int		i, j;

    struct species_data		*species, *alien;
    struct nampla_data		*nampla, *alien_nampla;
    struct ship_data		*alien_ship;


    /* Check if the alien has a ship or starbase here that is in orbit or in
	deep space. */
    alien = &spec_data[alien_number - 1];
    alien_ship = ship_data[alien_number - 1] - 1;
    for (i = 0; i < alien->num_ships; i++)
    {
	++alien_ship;

	if (alien_ship->x != x) continue;
	if (alien_ship->y != y) continue;
	if (alien_ship->z != z) continue;
	if (alien_ship->item_quantity[FD] == alien_ship->tonnage) continue;

	if (alien_ship->status == IN_ORBIT
		||  alien_ship->status == IN_DEEP_SPACE)
	    return TRUE;
    }

    /* Check if alien has a planet that is not hidden. */
    alien_nampla = namp_data[alien_number - 1] - 1;
    for (i = 0; i < alien->num_namplas; i++)
    {
	++alien_nampla;

	if (alien_nampla->x != x) continue;
	if (alien_nampla->y != y) continue;
	if (alien_nampla->z != z) continue;
	if ((alien_nampla->status & POPULATED) == 0) continue;

	if (! alien_nampla->hidden) return TRUE;

	/* The colony is hidden. See if we have population on the same
	    planet. */
	species = &spec_data[species_number - 1];
	nampla = namp_data[species_number - 1] - 1;
	for (j = 0; j < species->num_namplas; j++)
	{
	    ++nampla;

	    if (nampla->x != x) continue;
	    if (nampla->y != y) continue;
	    if (nampla->z != z) continue;
	    if (nampla->pn != alien_nampla->pn) continue;
	    if ((nampla->status & POPULATED) == 0) continue;

	    /* We have population on the same planet, so the alien
		cannot hide. */
	    return TRUE;
	}
    }

    return FALSE;
}
