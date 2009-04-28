
#include "fh.h"


extern int			abbr_type, abbr_index, species_number,
				species_index, ship_index, num_stars;
extern long			value;
extern char			input_line[256], original_line[256], original_name[32],
				upper_name[32], *input_line_pointer;

extern FILE			*log_file;
extern struct star_data		*star_base, *star;
extern struct species_data	*species;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship, *ship_base;


do_BASE_command ()

{
    int		i, n, found, su_count, original_count, item_class, name_length,
		unused_ship_available, new_tonnage, max_tonnage, new_starbase,
		source_is_a_planet, age_new;

    char	x, y, z, pn, upper_ship_name[32], *original_line_pointer;

    struct nampla_data		*source_nampla;
    struct ship_data		*source_ship, *starbase, *unused_ship;


    /* Get number of starbase units to use. */
    i = get_value ();
    if (i == 0)
	value = 0;
    else
    {
	/* Make sure value is meaningful. */
	if (value < 0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid SU count in BASE command.\n");
	    return;
	}
    }
    su_count = value;
    original_count = su_count;

    /* Get source of starbase units. */
    original_line_pointer = input_line_pointer;
    if (! get_transfer_point ())
    {
	input_line_pointer = original_line_pointer;
	fix_separator ();	/* Check for missing comma or tab. */
	if (! get_transfer_point ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid source location in BASE command.\n");
	    return;
	}
    }

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS)
    {
	source_is_a_planet = FALSE;
	source_ship = ship;

	if (source_ship->status == UNDER_CONSTRUCTION)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s is still under construction!\n",
				ship_name (source_ship));
	    return;
	}

	if (source_ship->status == FORCED_JUMP
	  ||  source_ship->status == JUMPED_IN_COMBAT)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	    return;
	}

	if (su_count == 0) su_count = source_ship->item_quantity[SU];
	if (su_count == 0) return;
	if (source_ship->item_quantity[SU] < su_count)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s does not enough starbase units!\n",
				ship_name (source_ship));
	    return;
	}

	x = source_ship->x;	y = source_ship->y;	z = source_ship->z;
	pn = source_ship->pn;
    }
    else	/* Source is a planet. */
    {
	source_is_a_planet = TRUE;
	source_nampla = nampla;

	if (su_count == 0) su_count = source_nampla->item_quantity[SU];
	if (su_count == 0) return;
	if (source_nampla->item_quantity[SU] < su_count)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! PL %s does not have enough starbase units!\n",
				source_nampla -> name);
	    return;
	}

	x = source_nampla->x;	y = source_nampla->y;	z = source_nampla->z;
	pn = source_nampla->pn;
    }

    /* Get starbase name. */
    if (get_class_abbr () != SHIP_CLASS  ||  abbr_index != BA)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid starbase name.\n");
	return;
    }
    name_length = get_name ();

    /* Search all ships for name. */
    found = FALSE;
    ship = ship_base - 1;
    unused_ship_available = FALSE;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	++ship;

	if (ship->pn == 99)
	{
	    unused_ship_available = TRUE;
	    unused_ship = ship;
	    continue;
	}

	/* Make upper case copy of ship name. */
	for (i = 0; i < 32; i++)
	    upper_ship_name[i] = toupper(ship->name[i]);

	/* Compare names. */
	if (strcmp (upper_ship_name, upper_name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (found)
    {
	if (ship->type != STARBASE)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship name already in use.\n");
	    return;
	}

	if (ship->x != x  ||  ship->y != y  ||  ship->z != z)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Starbase units and starbase are not at same X Y Z.\n");
	    return;
	}
	starbase = ship;
	new_starbase = FALSE;
    }
    else
    {
	if (unused_ship_available)
	    starbase = unused_ship;
	else
	{
	    /* Make sure we have enough memory for new starbase. */
	    if (num_new_ships[species_index] == NUM_EXTRA_SHIPS)
	    {
		fprintf (stderr, "\n\n\tInsufficient memory for new starbase!\n\n");
		exit (-1);
	    }
	    ++num_new_ships[species_index];
	    starbase = ship_base + (int) species->num_ships;
	    ++species->num_ships;
	    delete_ship (starbase);		/* Initialize everything to zero. */
	}

	/* Initialize non-zero data for new ship. */
	strcpy (starbase->name, original_name);
	starbase->x = x;
	starbase->y = y;
	starbase->z = z;
	starbase->pn = pn;
	if (pn == 0)
	    starbase->status = IN_DEEP_SPACE;
	else
	    starbase->status = IN_ORBIT;
	starbase->type = STARBASE;
	starbase->class = BA;
	starbase->tonnage = 0;
	starbase->age = -1;
	starbase->remaining_cost = 0;

	/* Everything else was set to zero in above call to 'delete_ship'. */

	new_starbase = TRUE;
    }

    /* Make sure that starbase is not being built in the deep space section
	of a star system .*/
    if (starbase->pn == 0)
    {
	star = star_base - 1;
	for (i = 0; i < num_stars; i++)
	{
	    ++star;

	    if (star->x != x) continue;
	    if (star->y != y) continue;
	    if (star->z != z) continue;

	    if (star->num_planets < 1) break;

	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Starbase cannot be built in deep space if there are planets available!\n");
	    if (new_starbase) delete_ship (starbase);
	    return;
	}
    }

    /* Make sure species can build a starbase of this size. */
    max_tonnage = species->tech_level[MA] / 2;
    new_tonnage = starbase->tonnage + su_count;
    if (new_tonnage > max_tonnage  &&  original_count == 0)
    {
	su_count = max_tonnage - starbase->tonnage;
	if (su_count < 1)
	{
	    if (new_starbase) delete_ship (starbase);
	    return;
	}
	new_tonnage = starbase->tonnage + su_count;
    }

    if (new_tonnage > max_tonnage)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Maximum allowable tonnage exceeded.\n");
	if (new_starbase) delete_ship (starbase);
	return;
    }

    /* Finish up and log results. */
    log_string ("    ");
    if (starbase->tonnage == 0)
    {
	log_string (ship_name (starbase));
	log_string (" was constructed.\n");
    }
    else
    {
	starbase->age =		/* Weighted average. */
	    ((starbase->age * starbase->tonnage) - su_count)
			/new_tonnage;
	log_string ("Size of ");  log_string (ship_name (starbase));
	log_string (" was increased to ");
	log_string (commas (10000L * (long) new_tonnage));
	log_string (" tons.\n");
    }

    starbase->tonnage = new_tonnage;

    if (source_is_a_planet)
	source_nampla->item_quantity[SU] -= su_count;
    else
	source_ship->item_quantity[SU] -= su_count;
}
