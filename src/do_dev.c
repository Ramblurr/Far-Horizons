
#include "fh.h"


extern int	doing_production;
extern long	value, balance, EU_spending_limit;
extern char	input_line[256], original_line[256], *input_line_pointer;
extern FILE	*log_file;

extern struct planet_data	*planet_base;
extern struct species_data	*species;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship;


do_DEVELOP_command ()
{
    int		i, num_CUs, num_AUs, num_IUs, more_args, load_transport,
		capacity, resort_colony, mining_colony, production_penalty,
		CUs_only;

    char	c, *original_line_pointer, *tp;

    long	n, ni, na, amount_to_spend, original_cost, max_funds_available,
		ls_needed, raw_material_units, production_capacity,
		colony_production, ib, ab, md, denom, reb, specified_max;

    struct planet_data	*colony_planet, *home_planet;
    struct nampla_data	*temp_nampla, *colony_nampla;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Get default spending limit. */
    max_funds_available = species->econ_units;
    if (max_funds_available > EU_spending_limit)
	max_funds_available = EU_spending_limit;
    max_funds_available += balance;

    /* Get specified spending limit, if any. */
    specified_max = -1;
    if (get_value ())
    {
	if (value == 0)
	    max_funds_available = balance;
	else if (value > 0)
	{
	    specified_max = value;
	    if (value <= max_funds_available)
		max_funds_available = value;
	    else
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file,
		    "! Insufficient funds. Substituting %ld for %ld.\n",
		    max_funds_available, value);
		if (max_funds_available == 0) return;
	    }
	}
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Invalid spending limit.\n");
	    return;
	}
    }

    /* See if there are any more arguments. */
    tp = input_line_pointer;
    more_args = FALSE;
    while (c = *tp++)
    {
	if (c == ';'  ||  c == '\n') break;
	if (c == ' '  ||  c == '\t') continue;
	more_args = TRUE;
	break;
    }

    if (! more_args)
    {
	/* Make sure planet is not a healthy home planet. */
	if (nampla->status & HOME_PLANET)
	{
	    reb = species->hp_original_base - (nampla->mi_base + nampla->ma_base);
	    if (reb > 0)
	    {
		/* Home planet is recovering from bombing. */
		if (reb < max_funds_available) max_funds_available = reb;
	    }
	    else
	    {
		fprintf (log_file, "!!! Order ignored:\n");
		fprintf (log_file, "!!! %s", input_line);
		fprintf (log_file, "!!! You can only DEVELOP a home planet if it is recovering from bombing.\n");
		return;
	    }
	}

	/* No arguments. Order is for this planet. */
	num_CUs = nampla->pop_units;
	if (2 * num_CUs > max_funds_available)
	    num_CUs = max_funds_available / 2;
	if (num_CUs <= 0) return;

	colony_planet = planet_base + (long) nampla->planet_index;
	ib = nampla->mi_base + nampla->IUs_to_install;
	ab = nampla->ma_base + nampla->AUs_to_install;
	md = colony_planet->mining_difficulty;

	denom = 100 + md;
	num_AUs =
	    (100 * (num_CUs + ib) - (md * ab) + denom/2) / denom;
	num_IUs = num_CUs - num_AUs;

	if (num_IUs < 0)
	{
	    num_AUs = num_CUs;
	    num_IUs = 0;
	}
	if (num_AUs < 0)
	{
	    num_IUs = num_CUs;
	    num_AUs = 0;
	}

	amount_to_spend = num_CUs + num_AUs + num_IUs;

	if (check_bounced (amount_to_spend))
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Internal error. Please notify GM!\n");
	    return;
	}

	nampla->pop_units -= num_CUs;
	nampla->item_quantity[CU] += num_CUs;
	nampla->item_quantity[IU] += num_IUs;
	nampla->item_quantity[AU] += num_AUs;

	nampla->auto_IUs += num_IUs;
	nampla->auto_AUs += num_AUs;

	start_dev_log (num_CUs, num_IUs, num_AUs);
	log_string (".\n");

	check_population (nampla);

	return;
    }

    /* Get the planet to be developed. */
    temp_nampla = nampla;
    original_line_pointer = input_line_pointer;
    i = get_location ();
    if (! i  ||  nampla == NULL)
    {
	/* Check for missing comma or tab after source name. */
	input_line_pointer = original_line_pointer;
	fix_separator ();
	i = get_location ();
    }
    colony_nampla = nampla;
    nampla = temp_nampla;
    if (! i  ||  colony_nampla == NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in DEVELOP command.\n");
	return;
    }

    /* Make sure planet is not a healthy home planet. */
    if (colony_nampla->status & HOME_PLANET)
    {
	reb = species->hp_original_base - (colony_nampla->mi_base + colony_nampla->ma_base);
	if (reb > 0)
	{
	    /* Home planet is recovering from bombing. */
	    if (reb < max_funds_available) max_funds_available = reb;
	}
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! You can only DEVELOP a home planet if it is recovering from bombing.\n");
	    return;
	}
    }

    /* Determine if its a mining or resort colony, and if it can afford to
	build its own IUs and AUs. Note that we cannot use nampla->status
	because it is not correctly set until the Finish program is run. */

    home_planet = planet_base + (long) nampla_base->planet_index;
    colony_planet = planet_base + (long) colony_nampla->planet_index;
    ls_needed = life_support_needed (species, home_planet, colony_planet);

    ni = colony_nampla->mi_base + colony_nampla->IUs_to_install;
    na = colony_nampla->ma_base + colony_nampla->AUs_to_install;

    if (ni > 0  &&  na == 0)
    {
	colony_production = 0;
	mining_colony = TRUE;
	resort_colony = FALSE;
    }
    else if (na > 0  &&  ni == 0  &&  ls_needed <= 6
	&&  colony_planet->gravity <= home_planet->gravity)
    {
	colony_production = 0;
	resort_colony = TRUE;
	mining_colony = FALSE;
    }
    else
    {
	mining_colony = FALSE;
	resort_colony = FALSE;

	raw_material_units = (10L * (long) species->tech_level[MI] * ni)
		/ (long) colony_planet->mining_difficulty;
	production_capacity = ((long) species->tech_level[MA] * na) / 10L;

	if (ls_needed == 0)
	    production_penalty = 0;
	else
	    production_penalty = (100 * ls_needed) / species->tech_level[LS];

	raw_material_units -= (production_penalty * raw_material_units) / 100;
	production_capacity -= (production_penalty * production_capacity) / 100;

	colony_production = (production_capacity > raw_material_units)
			? raw_material_units : production_capacity;

	colony_production -= colony_nampla->IUs_needed
				+ colony_nampla->AUs_needed;
			/* In case there is more than one DEVELOP order for
				this colony. */
    }

    /* See if there are more arguments. */
    tp = input_line_pointer;
    more_args = FALSE;
    while (c = *tp++)
    {
	if (c == ';'  ||  c == '\n') break;
	if (c == ' '  ||  c == '\t') continue;
	more_args = TRUE;
	break;
    }

    if (more_args)
    {
	load_transport = TRUE;

	/* Get the ship to receive the cargo. */
	if (! get_ship ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship to be loaded does not exist!\n");
	    return;
	}

	if (ship->class == TR)
	    capacity = (10 + ((int) ship->tonnage / 2)) * (int) ship->tonnage;
	else if (ship->class == BA)
	    capacity = 10 * ship->tonnage;
	else
	    capacity = ship->tonnage;

	for (i = 0; i < MAX_ITEMS; i++)
	    capacity -= ship->item_quantity[i] * item_carry_capacity[i];

	if (capacity <= 0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s was already full and could take no more cargo!\n",
		ship_name (ship));
	    return;
	}

	if (capacity > max_funds_available)
	{
	    capacity = max_funds_available;
	    if (max_funds_available != specified_max)
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file, "! Insufficient funds to completely fill %s!\n",
			ship_name (ship));
		fprintf (log_file, "! Will use all remaining funds (= %d).\n",
			capacity);
	    }
	}
    }
    else
    {
	load_transport = FALSE;

	/* No more arguments. Order is for a colony in the same sector as the
	    producing planet. */
	if (nampla->x != colony_nampla->x  ||  nampla->y != colony_nampla->y
		|| nampla->z != colony_nampla->z)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Colony and producing planet are not in the same sector.\n");
	    return;
	}

	num_CUs = nampla->pop_units;
	if (2 * num_CUs > max_funds_available)
	    num_CUs = max_funds_available / 2;
    }

    CUs_only = FALSE;
    if (mining_colony)
    {
	if (load_transport)
	{
	    num_CUs = capacity / 2;
	    if (num_CUs > nampla->pop_units)
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file,
		    "! Insufficient available population! %d CUs are needed",
		    num_CUs);
		num_CUs = nampla->pop_units;
		fprintf (log_file, " to fill ship but only %d can be built.\n",
		    num_CUs);
	    }
	}

	num_AUs = 0;
	num_IUs = num_CUs;
    }
    else if (resort_colony)
    {
	if (load_transport)
	{
	    num_CUs = capacity / 2;
	    if (num_CUs > nampla->pop_units)
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file,
		    "! Insufficient available population! %d CUs are needed",
		    num_CUs);
		num_CUs = nampla->pop_units;
		fprintf (log_file, " to fill ship but only %d can be built.\n",
		    num_CUs);
	    }
	}

	num_IUs = 0;
	num_AUs = num_CUs;
    }
    else
    {
	if (load_transport)
	{
	    if (colony_production >= capacity)
	    {
		/* Colony can build its own IUs and AUs. */
		num_CUs = capacity;
		CUs_only = TRUE;
	    }
	    else
	    {
		/* Build IUs and AUs for the colony. */
		num_CUs = capacity / 2;
	    }

	    if (num_CUs > nampla->pop_units)
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file,
		    "! Insufficient available population! %d CUs are needed",
		    num_CUs);
		num_CUs = nampla->pop_units;
		fprintf (log_file, " to fill ship, but\n!   only %d can be built.\n",
		    num_CUs);
	    }
	}

	colony_planet = planet_base + (long) colony_nampla->planet_index;

	i = 100 + (int) colony_planet->mining_difficulty;
	num_AUs = ((100 * num_CUs) + (i + 1)/2) / i;
	num_IUs = num_CUs - num_AUs;
    }

    if (num_CUs <= 0) return;

    /* Make sure there's enough money to pay for it all. */
    if (load_transport && CUs_only)
	amount_to_spend = num_CUs;
    else
	amount_to_spend = num_CUs + num_IUs + num_AUs;

    if (check_bounced (amount_to_spend))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Internal error. Notify GM!\n");
	return;
    }

    /* Start logging what happened. */
    if (load_transport && CUs_only)
    {
	start_dev_log (num_CUs, 0, 0);
    }
    else
	start_dev_log (num_CUs, num_IUs, num_AUs);

    log_string (" for PL ");    log_string (colony_nampla->name);

    nampla->pop_units -= num_CUs;

    if (load_transport)
    {
	if (CUs_only)
	{
	    colony_nampla->IUs_needed += num_IUs;
	    colony_nampla->AUs_needed += num_AUs;
	}

	if (nampla->x != ship->x  ||  nampla->y != ship->y
		|| nampla->z != ship->z)
	{
	    nampla->item_quantity[CU] += num_CUs;
	    if (! CUs_only)
	    {
		nampla->item_quantity[IU] += num_IUs;
		nampla->item_quantity[AU] += num_AUs;
	    }

	    log_string (" but will remain on the planet's surface because ");
	    log_string (ship_name (ship));
	    log_string (" is not in the same sector.");
	}
	else
	{
	    ship->item_quantity[CU] += num_CUs;
	    if (! CUs_only)
	    {
		ship->item_quantity[IU] += num_IUs;
		ship->item_quantity[AU] += num_AUs;
	    }

	    n = colony_nampla - nampla_base;
	    if (n == 0) n = 9999;	/* Home planet. */
	    ship->unloading_point = n;

	    n = nampla - nampla_base;
	    if (n == 0) n = 9999;	/* Home planet. */
	    ship->loading_point = n;

	    log_string (" and transferred to ");
	    log_string (ship_name (ship));
	}
    }
    else
    {
	colony_nampla->item_quantity[CU] += num_CUs;
	colony_nampla->item_quantity[IU] += num_IUs;
	colony_nampla->item_quantity[AU] += num_AUs;

	colony_nampla->auto_IUs += num_IUs;
	colony_nampla->auto_AUs += num_AUs;

	log_string (" and transferred to PL ");
	log_string (colony_nampla->name);

	check_population (colony_nampla);
    }

    log_string (".\n");
}


start_dev_log (num_CUs, num_IUs, num_AUs)

int	num_CUs, num_IUs, num_AUs;

{
    log_string ("    ");
    log_int (num_CUs);  log_string (" Colonist Unit");
	if (num_CUs != 1) log_char ('s');

    if (num_IUs + num_AUs  ==  0) goto done;

    if (num_IUs > 0)
    {
	if (num_AUs == 0)
	    log_string (" and ");
	else
	    log_string (", ");

	log_int (num_IUs);  log_string (" Colonial Mining Unit");
	    if (num_IUs != 1) log_char ('s');
    }

    if (num_AUs > 0)
    {
	if (num_IUs > 0) log_char (',');

	log_string (" and ");

	log_int (num_AUs);  log_string (" Colonial Manufacturing Unit");
	    if (num_AUs != 1) log_char ('s');
    }

done:

    log_string (" were built");
}
