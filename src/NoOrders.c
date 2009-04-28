
/* This program will generate default orders for a species if no explicit
	orders have been provided. */

#define	THIS_IS_MAIN

#include "fh.h"


struct galaxy_data	galaxy;
struct planet_data	*planet, *home_planet;
struct species_data	*species;
struct nampla_data	*nampla_base;
struct ship_data	*ship_base;

int			x, y, z, species_number, species_index;

FILE	*orders_file;

extern int			num_locs, truncate_name, num_stars;
extern unsigned long		last_random;
extern struct sp_loc_data	loc[MAX_LOCATIONS];
extern struct star_data		*star_base;
extern struct planet_data	*planet_base;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, k, ship_index, locations_fd, my_loc_index,
		nampla_index, its_loc_index, tonnage, found, alien_number,
		alien_index, array_index, bit_number, ls_needed,
		production_penalty;

    char	filename[32], *random_name(), message_line[132];

    long	n, nn, raw_material_units, production_capacity, balance,
		current_base, CUs_needed, IUs_needed, AUs_needed, EUs,
		bit_mask;

    FILE	*message_file, *log_file;

    struct species_data		*alien;
    struct nampla_data		*nampla, *home_nampla, *temp_nampla;
    struct ship_data		*ship;
    struct sp_loc_data		*locations_base, *my_loc, *its_loc;


    /* Check for valid command line. */
    if (argc != 1)
    {
	fprintf (stderr, "\n\tUsage: NoOrders\n\n");
	exit (0);
    }

    /* Seed random number generator. */
    last_random = time(NULL);
    j = 907;
    for (i = 0; i < j; i++) rnd(100);

    /* Get all necessary data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_species_data ();
    get_location_data ();

    truncate_name = TRUE;

    /* Major loop. Check each species in the game. */
    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	species_index = species_number - 1;

	/* Check if this species is still in the game. */
	if (! data_in_memory[species_index]) continue;

	/* Check if we have orders. */
	sprintf (filename, "sp%02d.ord\0", species_number);
	i = open (filename, 0);
	if (i >= 0)
	{
	    close (i);
	    continue;
	}

	species = &spec_data[species_index];
	nampla_base = namp_data[species_index];
	ship_base = ship_data[species_index];
	home_nampla = nampla_base;
	home_planet = planet_base + (int) home_nampla->planet_index;

	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;
	    ship->special = 0;
	}

	/* Print message for gamemaster. */
	printf ("Generating orders for species #%02d, SP %s...\n",
		species_number, species->name);

	/* Open message file. */
	sprintf (filename, "noorders.txt\0");
	message_file = fopen (filename, "r");
	if (message_file == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
	    exit (-1);
	}

	/* Open log file. */
	sprintf (filename, "sp%02d.log", species_number);
	log_file = fopen (filename, "a");
	if (log_file == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
	    exit (-1);
	}

	/* Copy message to log file. */
	while (fgets(message_line, 131, message_file) != NULL)
	    fputs (message_line, log_file);

	fclose (message_file);
	fclose (log_file);

 	/* Open orders file for writing. */
	sprintf (filename, "sp%02d.ord", species_number);
	orders_file = fopen (filename, "w");
	if (orders_file == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for writing!\n\n", filename);
	    exit (-1);
	}

	/* Issue PRE-DEPARTURE orders. */
	fprintf (orders_file, "START PRE-DEPARTURE\n");
	fprintf (orders_file, "; Place pre-departure orders here.\n\n");

	for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
	{
	    nampla = nampla_base + nampla_index;
	    if (nampla->pn == 99) continue;

	    /* Generate auto-installs for colonies that were loaded via
		the DEVELOP command. */
	    if (nampla->auto_IUs)
		fprintf (orders_file, "\tInstall\t%d IU\tPL %s\n",
			nampla->auto_IUs, nampla->name);
	    if (nampla->auto_AUs)
		fprintf (orders_file, "\tInstall\t%d AU\tPL %s\n",
			nampla->auto_AUs, nampla->name);
	    if (nampla->auto_IUs  ||  nampla->auto_AUs)
		fprintf (orders_file, "\n");

	    nampla->item_quantity[CU] -= nampla->auto_IUs + nampla->auto_AUs;

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

		fprintf (orders_file, "\tUnload\tTR%d%s %s\n\n", ship->tonnage,
		    ship_type[ship->type], ship->name);

		nampla->item_quantity[CU] = 0;

		ship->special = ship->loading_point;
		n = nampla - nampla_base;
		if (n == 0) n = 9999;
		ship->unloading_point = n;
	    }

	    if (nampla->status & HOME_PLANET) continue;
	    if (nampla->item_quantity[CU] == 0) continue;
	    if (nampla->item_quantity[IU] == 0
		&&  nampla->item_quantity[AU] == 0) continue;

	    if (nampla->item_quantity[IU] > 0)
		fprintf (orders_file, "\tInstall\t0 IU\tPL %s\n", nampla->name);
	    if (nampla->item_quantity[AU] > 0)
		fprintf (orders_file, "\tInstall\t0 AU\tPL %s\n\n", nampla->name);
	}

	fprintf (orders_file, "END\n\n");

	fprintf (orders_file, "START JUMPS\n");
	fprintf (orders_file, "; Place jump orders here.\n\n");

	/* Initialize to make sure ships are not given more than one JUMP order. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;
	    ship->just_jumped = FALSE;
	}

	/* Generate auto-jumps for ships that were loaded via the DEVELOP
	   command or which were UNLOADed because of the AUTO command. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;

	    if (ship->status == JUMPED_IN_COMBAT) continue;
	    if (ship->status == FORCED_JUMP) continue;
	    if (ship->pn == 99) continue;
	    if (ship->just_jumped) continue;

	    j = ship->special;
	    if (j)
	    {
		if (j == 9999) j = 0;	/* Home planet. */
		temp_nampla = nampla_base + j;

		fprintf (orders_file, "\tJump\t%s, PL %s\t; ", ship_name (ship),
		    temp_nampla->name);

		print_mishap_chance (ship, temp_nampla->x, temp_nampla->y,
			temp_nampla->z);

		fprintf (orders_file, "\n\n");

		ship->just_jumped = TRUE;

		continue;
	    }

	    n = ship->unloading_point;
	    if (n)
	    {
		if (n == 9999) n = 0;	/* Home planet. */

		temp_nampla = nampla_base + n;

		if (temp_nampla->x == ship->x  &&  temp_nampla->y == ship->y
			&&  temp_nampla->z == ship->z) continue;

		fprintf (orders_file, "\tJump\t%s, PL %s\t; ", ship_name (ship),
			temp_nampla->name);

		print_mishap_chance (ship, temp_nampla->x, temp_nampla->y,
			temp_nampla->z);

		fprintf (orders_file, "\n\n");

		ship->just_jumped = TRUE;
	    }
	}

	/* Generate JUMP orders for all TR1s. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;
	    if (ship->pn == 99) continue;
	    if (ship->status == UNDER_CONSTRUCTION) continue;
	    if (ship->status == JUMPED_IN_COMBAT) continue;
	    if (ship->status == FORCED_JUMP) continue;
	    if (ship->just_jumped) continue;

	    if (ship->class == TR  &&  ship->tonnage == 1
		&&  ship->type == FTL)
	    {
		fprintf (orders_file, "\tJump\tTR1 %s, ", ship->name);
		closest_unvisited_star (ship);
		fprintf (orders_file,
		    "\n\t\t\t; Age %d, now at %d %d %d, ",
		    ship->age, ship->x, ship->y, ship->z);

		print_mishap_chance (ship, x, y, z);

		ship->dest_x = x;
		ship->dest_y = y;
		ship->dest_z = z;

		fprintf (orders_file, "\n\n");

		ship->just_jumped = TRUE;
	    }
	}

	fprintf (orders_file, "END\n\n");

	fprintf (orders_file, "START PRODUCTION\n");

	/* Generate a PRODUCTION order for each planet that can produce. */
	for (nampla_index = species->num_namplas - 1; nampla_index >= 0;
		nampla_index--)
	{
	    nampla = nampla_base + nampla_index;
	    if (nampla->pn == 99) continue;

	    if (nampla->mi_base == 0  &&  (nampla->status & RESORT_COLONY) == 0) continue;
	    if (nampla->ma_base == 0  &&  (nampla->status & MINING_COLONY) == 0) continue;

	    fprintf (orders_file, "    PRODUCTION PL %s\n", nampla->name);

	    if (nampla->status & MINING_COLONY)
	    {
		fprintf (orders_file,
		    "    ; The above PRODUCTION order is required for this mining colony, even\n");
		fprintf (orders_file,
		    "    ;  if no other production orders are given for it.\n");
	    }
	    else if (nampla->status & RESORT_COLONY)
	    {
		fprintf (orders_file,
		    "    ; The above PRODUCTION order is required for this resort colony, even\n");
		fprintf (orders_file,
		    "    ;  though no other production orders can be given for it.\n");
	    }
	    else
	    {
		fprintf (orders_file,
		    "    ; Place production orders here for planet %s.\n\n",
		    nampla->name);
	    }

	    /* Build IUs and AUs for incoming ships with CUs. */
	    if (nampla->IUs_needed)
		fprintf (orders_file, "\tBuild\t%d IU\n", nampla->IUs_needed);
	    if (nampla->AUs_needed)
		fprintf (orders_file, "\tBuild\t%d AU\n", nampla->AUs_needed);
	    if (nampla->IUs_needed  ||  nampla->AUs_needed)
		fprintf (orders_file, "\n");

	    if (nampla->status & MINING_COLONY) continue;
	    if (nampla->status & RESORT_COLONY) continue;

	    /* See if there are any RMs to recycle. */
	    n = nampla->special / 5;
	    if (n > 0)
		fprintf (orders_file, "\tRecycle\t%d RM\n\n", 5*n);

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

		fprintf (orders_file, "\tDevelop\tPL %s, TR%d%s %s\n\n",
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
		    fprintf (orders_file,
			"\tContinue\t%s, %d\t; Left to pay = %d\n\n",
			ship_name (ship), ship->remaining_cost,
			ship->remaining_cost);

		    continue;
		}

		if (ship->type != STARBASE) continue;

		j = (species->tech_level[MA] / 2) - ship->tonnage;
		if (j < 1) continue;

		fprintf (orders_file,
		    "\tContinue\tBAS %s, %d\t; Current tonnage = %s\n\n",
		    ship->name, 100 * j, commas (10000 * (long) ship->tonnage));
	    }

	    /* Generate DEVELOP command if this is a colony with an
		economic base less than 200. */
	    n = nampla->mi_base + nampla->ma_base + nampla->IUs_needed
			+ nampla->AUs_needed;
	    if ((nampla->status & COLONY)  &&  n < 2000
		&&  nampla->pop_units > 0)
	    {
		if (nampla->pop_units > (2000L-n))
		    nn = 2000L-n;
		else
		    nn = nampla->pop_units;

		fprintf (orders_file, "\tDevelop\t%ld\n\n",
			2L * nn);

		nampla->IUs_needed += nn;
	    }

	    /* For home planets and any colonies that have an economic base of
		at least 200, check if there are other colonized planets in
		the same sector that are not self-sufficient.  If so, DEVELOP
		them. */
	    if (n >= 2000  ||  (nampla->status & HOME_PLANET))
	    {
		for (i = 1; i < species->num_namplas; i++)  /* Skip HP. */
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

		    fprintf (orders_file, "\tDevelop\t%ld\tPL %s\n\n",
			2L * nn, temp_nampla->name);

		    temp_nampla->AUs_needed += nn;
		}
	    }
	}

	fprintf (orders_file, "END\n\n");

	fprintf (orders_file, "START POST-ARRIVAL\n");
	fprintf (orders_file, "; Place post-arrival orders here.\n\n");

	/* Generate an AUTO command. */
	fprintf (orders_file, "\tAuto\n\n");

	/* Generate SCAN orders for all TR1s in sectors that current species
	   does not inhabit. */
	for (i = 0; i < species->num_ships; i++)
	{
	    ship = ship_base + i;

	    if (ship->pn == 99) continue;
	    if (ship->status == UNDER_CONSTRUCTION) continue;
	    if (ship->class != TR) continue;
	    if (ship->tonnage != 1) continue;
	    if (ship->type != FTL) continue;
	    if (ship->dest_x == -1) continue;	/* Not jumping anywhere. */

	    found = FALSE;
	    for (j = 1; j < species->num_namplas; j++) /* Skip home sector. */
	    {
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
	    if (! found) fprintf (orders_file, "\tScan\tTR1 %s\n", ship->name);
	}

	fprintf (orders_file, "END\n\n");

	/* Clean up for this species. */
	fclose (orders_file);
    }

    /* Clean up and exit. */
    free_species_data ();
    exit (0);
}


print_mishap_chance (ship, destx, desty, destz)

struct ship_data	*ship;
int			destx, desty, destz;

{
    int		mishap_GV, mishap_age;

    long	x, y, z, mishap_chance, success_chance;


    if (destx == -1)
    {
	fprintf (orders_file, "Mishap chance = ???");
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

    fprintf (orders_file, "mishap chance = %ld.%02ld%%",
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

    x = -1;
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
	fprintf (orders_file, "%d %d %d", x, y, z);
	closest_star->visited_by[species_array_index] |= species_bit_mask;
	    /* So that we don't send more than one ship to the same place. */
    }
    else
	fprintf (orders_file, "???");

    return;
}
