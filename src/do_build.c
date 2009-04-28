
#include "fh.h"



extern int	nampla_index, ship_index, doing_production, tonnage, sub_light,
		abbr_index, first_pass, species_number, species_index,
		num_transactions, g_spec_number, abbr_type, shipyard_capacity;
extern long	value, balance, EU_spending_limit;
extern char	input_line[256], original_line[256], original_name[32],
		upper_name[32], *input_line_pointer, *ship_name();
extern FILE	*log_file;

extern struct species_data	*species;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship_base, *ship;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_BUILD_command (continuing_construction, interspecies_construction)

int	continuing_construction, interspecies_construction;

{
    int		i, n, class, critical_tech, found, name_length,
		siege_effectiveness, cost_given, new_ship, max_tonnage,
		tonnage_increase, alien_number, cargo_on_board,
		unused_nampla_available, unused_ship_available, capacity,
		pop_check_needed, contact_word_number, contact_bit_number,
		already_notified[MAX_SPECIES];

    char	upper_ship_name[32], *commas(), *src, *dest,
		*original_line_pointer;

    long	cost, cost_argument, unit_cost, num_items, pop_reduction,
		premium, total_cost, original_num_items, contact_mask,
		max_funds_available;

    struct species_data		*recipient_species;
    struct nampla_data		*recipient_nampla, *unused_nampla,
				*destination_nampla, *temp_nampla;
    struct ship_data		*recipient_ship, *unused_ship;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Get ready if planet is under siege. */
    if (nampla->siege_eff < 0)
	siege_effectiveness = -nampla->siege_eff;
    else
	siege_effectiveness = nampla->siege_eff;

    /* Get species name and make appropriate tests if this is an interspecies
	construction order. */
    if (interspecies_construction)
    {
	original_line_pointer = input_line_pointer;
	if (! get_species_name ())
	{
	    /* Check for missing comma or tab after species name. */
	    input_line_pointer = original_line_pointer;
	    fix_separator ();
	    if (! get_species_name ())
	    {
		fprintf (log_file, "!!! Order ignored:\n");
		fprintf (log_file, "!!! %s", original_line);
		fprintf (log_file, "!!! Invalid species name.\n");
		return;
	    }
	}
	recipient_species = &spec_data[g_spec_number - 1];

	if (species->tech_level[MA] < 25)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! MA tech level must be at least 25 to do interspecies construction.\n");
	    return;
	}

	/* Check if we've met this species and make sure it is not an enemy. */
	contact_word_number = (g_spec_number - 1) / 32;
	contact_bit_number = (g_spec_number - 1) % 32;
	contact_mask = 1 << contact_bit_number;
	if ((species->contact[contact_word_number] & contact_mask)  ==  0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! You can't do interspecies construction for a species you haven't met.\n");
	    return;
	}
	if (species->enemy[contact_word_number] & contact_mask)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! You can't do interspecies construction for an ENEMY.\n");
	    return;
	}
    }

    /* Get number of items to build. */
    i = get_value ();

    if (i == 0)
	goto build_ship;	/* Not an item. */
    num_items = value;
    original_num_items = value;

    /* Get class of item. */
    class = get_class_abbr ();

    if (class != ITEM_CLASS  ||  abbr_index == RM)
    {
	/* Players sometimes accidentally use "MI" for "IU"
		or "MA" for "AU". */
	if (class == TECH_ID  &&  abbr_index == MI)
	    abbr_index = IU;
	else if (class == TECH_ID  &&  abbr_index == MA)
	    abbr_index = AU;
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid item class.\n");
	    return;
	}
    }
    class = abbr_index;

    if (interspecies_construction)
    {
	if (class == PD  ||  class == CU)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! You cannot build CUs or PDs for another species.\n");
	    return;
	}
    }

    /* Make sure species knows how to build this item. */
    critical_tech = item_critical_tech[class];
    if (species->tech_level[critical_tech] < item_tech_requirment[class])
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Insufficient tech level to build item.\n");
	return;
    }

    /* Get cost of item. */
    if (class == TP)	/* Terraforming plant. */
	unit_cost = item_cost[class] / species->tech_level[critical_tech];
    else
	unit_cost = item_cost[class];

    if (num_items == 0) num_items = balance / unit_cost;
    if (num_items == 0) return;

    /* Make sure item count is meaningful. */
    if (num_items < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Meaningless item count.\n");
	return;
    }

    /* Make sure there is enough available population. */
    pop_reduction = 0;
    if (class == CU  ||  class == PD)
    {
	if (nampla->pop_units < num_items)
	{
	    if (original_num_items == 0)
	    {
		num_items = nampla->pop_units;
		if (num_items == 0) return;
	    }
	    else
	    {
		if (nampla->pop_units > 0)
		{
		    fprintf (log_file, "! WARNING: %s", original_line);
		    fprintf (log_file,
			"! Insufficient available population units. Substituting %ld for %ld.\n",
			nampla->pop_units, num_items);
		    num_items = nampla->pop_units;
		}
		else
		{
		    fprintf (log_file, "!!! Order ignored:\n");
		    fprintf (log_file, "!!! %s", original_line);
		    fprintf (log_file, "!!! Insufficient available population units.\n");
		    return;
		}
	    }
	}
	pop_reduction = num_items;
    }

    /* Calculate total cost and see if planet has enough money. */
do_cost:
    cost = num_items * unit_cost;
    if (interspecies_construction)
	premium = (cost + 9) / 10;
    else
	premium = 0;

    cost += premium;

    if (check_bounced (cost))
    {
	if (interspecies_construction  &&  original_num_items == 0)
	{
	    --num_items;
	    if (num_items < 1) return;
	    goto do_cost;
	}

	max_funds_available = species->econ_units;
	if (max_funds_available > EU_spending_limit)
	    max_funds_available = EU_spending_limit;
	max_funds_available += balance;

	num_items = max_funds_available / unit_cost;
	if (interspecies_construction) num_items -= (num_items + 9) / 10;

	if (num_items > 0)
	{
	    fprintf (log_file, "! WARNING: %s", original_line);
	    fprintf (log_file, "! Insufficient funds. Substituting %ld for %ld.\n",
		num_items, original_num_items);
	    goto do_cost;
	}
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	    return;
	}
    }

    /* Update planet inventory. */
    nampla->item_quantity[class] += num_items;
    nampla->pop_units -= pop_reduction;

    /* Log what was produced. */
    log_string ("    ");  log_long (num_items);
    log_char (' ');  log_string (item_name[class]);

    if (num_items > 1)
	log_string ("s were");
    else
	log_string (" was");

    if (first_pass  &&  class == PD  &&  siege_effectiveness > 0)
    {
	log_string (" scheduled for production despite the siege.\n");
	return;
    }
    else
    {
	log_string (" produced");
	if (interspecies_construction)
	{
	    log_string (" for SP ");
	    log_string (recipient_species->name);
	}
    }

    if (unit_cost != 1  ||  premium != 0)
    {
	log_string (" at a cost of ");
	log_long (cost);
    }

    /* Check if planet is under siege and if production of planetary
	defenses was detected. */
    if (class == PD  &&  rnd(100) <= siege_effectiveness)
    {
	log_string (". However, they were detected and destroyed by the besiegers!!!\n");
	nampla->item_quantity[PD] = 0;

	/* Make sure we don't notify the same species more than once. */
	for (i = 0; i < MAX_SPECIES; i++) already_notified[i] = FALSE;

	for (i = 0; i < num_transactions; i++)
	{
	    /* Find out who is besieging this planet. */
	    if (transaction[i].type != BESIEGE_PLANET) continue;
	    if (transaction[i].x != nampla->x) continue;
	    if (transaction[i].y != nampla->y) continue;
	    if (transaction[i].z != nampla->z) continue;
	    if (transaction[i].pn != nampla->pn) continue;
	    if (transaction[i].number2 != species_number) continue;

	    alien_number = transaction[i].number1;

	    if (already_notified[alien_number - 1]) continue;

	    /* Define a 'detection' transaction. */
	    if (num_transactions == MAX_TRANSACTIONS)
	    {
		fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		exit (-1);
	    }

	    n = num_transactions++;
	    transaction[n].type = DETECTION_DURING_SIEGE;
	    transaction[n].value = 3;	/* Construction of PDs. */
	    strcpy (transaction[n].name1, nampla->name);
	    strcpy (transaction[n].name3, species->name);
	    transaction[n].number3 = alien_number;

	    already_notified[alien_number - 1] = TRUE;
	}
	return;
    }

    if (! interspecies_construction)
    {
	/* Get destination of transfer, if any. */
	pop_check_needed = FALSE;
	temp_nampla = nampla;
	found = get_transfer_point ();
	destination_nampla = nampla;
	nampla = temp_nampla;
	if (! found) goto done_transfer;

	if (abbr_type == SHIP_CLASS)	/* Destination is 'ship'. */
	{
	    if (ship->x != nampla->x
		|| ship->y != nampla->y
		|| ship->z != nampla->z
		|| ship->status == UNDER_CONSTRUCTION) goto done_transfer;

	    if (ship->class == TR)
		capacity = (10 + ((int) ship->tonnage / 2)) * (int) ship->tonnage;
	    else if (ship->class == BA)
		capacity = 10 * ship->tonnage;
	    else
		capacity = ship->tonnage;

	    for (i = 0; i < MAX_ITEMS; i++)
		capacity -= ship->item_quantity[i] * item_carry_capacity[i];

	    n = num_items;
	    if (num_items * item_carry_capacity[class]  >  capacity)
		num_items = capacity / item_carry_capacity[class];

	    ship->item_quantity[class] += num_items;
	    nampla->item_quantity[class] -= num_items;
	    log_string (" and ");
	    if (n > num_items)
	    {
		log_long (num_items);
		log_string (" of them ");
	    }
	    if (num_items == 1)
		log_string ("was");
	    else
		log_string ("were");
	    log_string (" transferred to ");
	    log_string (ship_name(ship));

	    if (class == CU  &&  num_items > 0)
	    {
		if (nampla == nampla_base)
		    ship->loading_point = 9999;	/* Home planet. */
		else
		    ship->loading_point = (nampla - nampla_base);
	    }
	}
	else	/* Destination is 'destination_nampla'. */
	{
	    if (destination_nampla->x != nampla->x
		|| destination_nampla->y != nampla->y
		|| destination_nampla->z != nampla->z) goto done_transfer;

	    if (nampla->siege_eff != 0) goto done_transfer;
	    if (destination_nampla->siege_eff != 0) goto done_transfer;

	    destination_nampla->item_quantity[class] += num_items;
	    nampla->item_quantity[class] -= num_items;
	    log_string (" and transferred to PL ");
	    log_string (destination_nampla->name);
	    pop_check_needed = TRUE;
	}

    done_transfer:

	log_string (".\n");

	if (pop_check_needed) check_population (destination_nampla);

	return;
    }

    log_string (".\n");

    /* Check if recipient species has a nampla at this location. */
    found = FALSE;
    unused_nampla_available = FALSE;
    recipient_nampla = namp_data[g_spec_number - 1] - 1;
    for (i = 0; i < recipient_species->num_namplas; i++)
    {
	++recipient_nampla;

	if (recipient_nampla->pn == 99)
	{
	    unused_nampla = recipient_nampla;
	    unused_nampla_available = TRUE;
	}

	if (recipient_nampla->x != nampla->x) continue;
	if (recipient_nampla->y != nampla->y) continue;
	if (recipient_nampla->z != nampla->z) continue;
	if (recipient_nampla->pn != nampla->pn) continue;

	found = TRUE;
	break;
    }

    if (! found)
    {
	/* Add new nampla to database for the recipient species. */
	if (unused_nampla_available)
	    recipient_nampla = unused_nampla;
	else
	{
	    ++num_new_namplas[species_index];
	    if (num_new_namplas[species_index] > NUM_EXTRA_NAMPLAS)
	    {
		fprintf (stderr, "\n\n\tInsufficient memory for new planet name in do_build.c!\n");
		exit (-1);
	    }
	    recipient_nampla = namp_data[g_spec_number - 1]
		+ recipient_species->num_namplas;
	    recipient_species->num_namplas += 1;
	    delete_nampla (recipient_nampla);	/* Set everything to zero. */
	}

	/* Initialize new nampla. */
	strcpy (recipient_nampla->name, nampla->name);
	recipient_nampla->x = nampla->x;
	recipient_nampla->y = nampla->y;
	recipient_nampla->z = nampla->z;
	recipient_nampla->pn = nampla->pn;
	recipient_nampla->planet_index = nampla->planet_index;
	recipient_nampla->status = COLONY;
    }

    /* Transfer the goods. */
    nampla->item_quantity[class] -= num_items;
    recipient_nampla->item_quantity[class] += num_items;
    data_modified[g_spec_number - 1] = TRUE;

    if (first_pass) return;

    /* Define transaction so that recipient will be notified. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    n = num_transactions++;
    transaction[n].type = INTERSPECIES_CONSTRUCTION;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = 1;	/* Items, not ships. */
    transaction[n].number1 = num_items;
    transaction[n].number2 = class;
    transaction[n].number3 = cost;
    strcpy (transaction[n].name1, species->name);
    strcpy (transaction[n].name2, recipient_nampla->name);

    return;


build_ship:

    original_line_pointer = input_line_pointer;
    if (continuing_construction)
    {
	found = get_ship ();
	if (! found)
	{
	    /* Check for missing comma or tab after ship name. */
	    input_line_pointer = original_line_pointer;
	    fix_separator ();
	    found = get_ship ();
	}

	if (found) goto check_ship;
	input_line_pointer = original_line_pointer;
    }

    class = get_class_abbr ();

    if (class != SHIP_CLASS  ||  tonnage < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid ship class.\n");
	return;
    }
    class = abbr_index;

    /* Get ship name. */
    name_length = get_name ();
    if (name_length < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid ship name.\n");
	return;
    }

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

check_ship:

    if (found)
    {
	/* Check if BUILD was accidentally used instead of CONTINUE. */
	if ((ship->status == UNDER_CONSTRUCTION  ||  ship->type == STARBASE)
		&&  ship->x == nampla->x  &&  ship->y == nampla->y
		&&  ship->z == nampla->z  &&  ship->pn == nampla->pn)
			continuing_construction = TRUE;

	if ((ship->status != UNDER_CONSTRUCTION  &&  ship->type != STARBASE)
		|| (! continuing_construction))
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship name already in use.\n");
	    return;
	}

	new_ship = FALSE;
    }
    else
    {
	/* If CONTINUE command was used, the player probably mis-spelled
	    the name. */
	if (continuing_construction)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid ship name.\n");
	    return;
	}

	if (unused_ship_available)
	    ship = unused_ship;
	else
	{
	    /* Make sure we have enough memory for new ship. */
	    if (num_new_ships[species_index] >= NUM_EXTRA_SHIPS)
	    {
		if (num_new_ships[species_index] == 9999) return;

		fprintf (log_file, "!!! Order ignored:\n");
		fprintf (log_file, "!!! %s", original_line);
		fprintf (log_file,
		    "!!! You cannot build more than %d ships per turn!\n",
		    NUM_EXTRA_SHIPS);
		num_new_ships[species_index] = 9999;
		return;
	    }
	    new_ship = TRUE;
	    ship = ship_base + (int) species->num_ships;
	    delete_ship (ship);		/* Initialize everything to zero. */
	}

	/* Initialize non-zero data for new ship. */
	strcpy (ship->name, original_name);
	ship->x = nampla->x;
	ship->y = nampla->y;
	ship->z = nampla->z;
	ship->pn = nampla->pn;
	ship->status = UNDER_CONSTRUCTION;
	if (class == BA)
	{
	    ship->type = STARBASE;
	    ship->status = IN_ORBIT;
	}
	else if (sub_light)
	    ship->type = SUB_LIGHT;
	else
	    ship->type = FTL;
	ship->class = class;
	ship->age = -1;
	if (ship->type != STARBASE) ship->tonnage = tonnage;
	ship->remaining_cost = ship_cost[class];
	if (ship->class == TR)
	    ship->remaining_cost = ship_cost[TR] * tonnage;
	if (ship->type == SUB_LIGHT)
	    ship->remaining_cost = (3L * (long) ship->remaining_cost) / 4L;
	ship->just_jumped = FALSE;

	/* Everything else was set to zero in above call to 'delete_ship'. */
    }

    /* Check if amount to spend was specified. */
    cost_given = get_value ();
    cost = value;
    cost_argument = value;

    if (cost_given)
    {
	if (interspecies_construction  &&  (ship->type != STARBASE))
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Amount to spend may not be specified.\n");
	    return;
	}

	if (cost == 0)
	{
	    cost = balance;
	    if (ship->type == STARBASE)
	    {
		if (cost % ship_cost[BA]  !=  0)
		    cost = ship_cost[BA] * (cost / ship_cost[BA]);
	    }
	    if (cost < 1)
	    {
		if (new_ship) delete_ship (ship);
		return;
	    }
	}

	if (cost < 1)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Amount specified is meaningless.\n");
	    if (new_ship) delete_ship (ship);
	    return;
	}

	if (ship->type == STARBASE)
	{
	    if (cost % ship_cost[BA]  !=  0)
	    {
		fprintf (log_file, "!!! Order ignored:\n");
		fprintf (log_file, "!!! %s", original_line);
		fprintf (log_file, "!!! Amount spent on starbase must be multiple of %d.\n",
			ship_cost[BA]);
		if (new_ship) delete_ship (ship);
		return;
	    }
	}
    }
    else
    {
	if (ship->type == STARBASE)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Amount to spend MUST be specified for starbase.\n");
	    if (new_ship) delete_ship (ship);
	    return;
	}

	cost = ship->remaining_cost;
    }

    /* Make sure species can build a ship of this size. */
    max_tonnage = species->tech_level[MA] / 2;
    if (ship->type == STARBASE)
    {
	tonnage_increase = cost/(long) ship_cost[BA];
	tonnage = ship->tonnage + tonnage_increase;
	if (tonnage > max_tonnage  &&  cost_argument == 0)
	{
	    tonnage_increase = max_tonnage - ship->tonnage;
	    if (tonnage_increase < 1) return;
	    tonnage = ship->tonnage + tonnage_increase;
	    cost = tonnage_increase * (int) ship_cost[BA];
	}
    }

    if (tonnage > max_tonnage)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Maximum allowable tonnage exceeded.\n");
	if (new_ship) delete_ship (ship);
	return;
    }

    /* Make sure species has gravitics technology if this is an FTL ship. */
    if (ship->type == FTL  &&  species->tech_level[GV] < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Gravitics tech needed to build FTL ship!\n");
	if (new_ship) delete_ship (ship);
	return;
    }

    /* Make sure amount specified is not an overpayment. */
    if (ship->type != STARBASE  &&  cost > ship->remaining_cost)
	cost = ship->remaining_cost;

    /* Make sure planet has sufficient shipyards. */
    if (shipyard_capacity < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Shipyard capacity exceeded!\n");
	if (new_ship) delete_ship (ship);
	return;
    }

    /* Make sure there is enough money to pay for it. */
    premium = 0;
    if (interspecies_construction)
    {
	if (ship->class == TR  ||  ship->type == STARBASE)
	    total_cost = ship_cost[ship->class] * tonnage;
	else
	    total_cost = ship_cost[ship->class];

	if (ship->type == SUB_LIGHT)
	    total_cost = (3 * total_cost) / 4;

	premium = total_cost / 10;
	if (total_cost % 10) ++premium;
    }

    if (check_bounced (cost + premium))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	if (new_ship) delete_ship (ship);
	return;
    }

    --shipyard_capacity;

    /* Test if this is a starbase and if planet is under siege. */
    if (ship->type == STARBASE  &&  siege_effectiveness > 0)
    {
	log_string ("    Your attempt to build ");
	log_string (ship_name (ship));
	log_string (" was detected by the besiegers and the starbase was destroyed!!!\n");

	/* Make sure we don't notify the same species more than once. */
	for (i = 0; i < MAX_SPECIES; i++) already_notified[i] = FALSE;

	for (i = 0; i < num_transactions; i++)
	{
	    /* Find out who is besieging this planet. */
	    if (transaction[i].type != BESIEGE_PLANET) continue;
	    if (transaction[i].x != nampla->x) continue;
	    if (transaction[i].y != nampla->y) continue;
	    if (transaction[i].z != nampla->z) continue;
	    if (transaction[i].pn != nampla->pn) continue;
	    if (transaction[i].number2 != species_number) continue;

	    alien_number = transaction[i].number1;

	    if (already_notified[alien_number - 1]) continue;

	    /* Define a 'detection' transaction. */
	    if (num_transactions == MAX_TRANSACTIONS)
	    {
		fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		exit (-1);
	    }

	    n = num_transactions++;
	    transaction[n].type = DETECTION_DURING_SIEGE;
	    transaction[n].value = 2;	/* Construction of ship/starbase. */
	    strcpy (transaction[n].name1, nampla->name);
	    strcpy (transaction[n].name2, ship_name(ship));
	    strcpy (transaction[n].name3, species->name);
	    transaction[n].number3 = alien_number;

	    already_notified[alien_number - 1] = TRUE;
	}

	delete_ship (ship);

	return;
    }

    /* Finish up and log results. */
    log_string ("    ");
    if (ship->type == STARBASE)
    {
	if (ship->tonnage == 0)
	{
	    log_string (ship_name (ship));
	    log_string (" was constructed");
	}
	else
	{
	    ship->age =		/* Weighted average. */
		((ship->age * ship->tonnage) - tonnage_increase)
			/tonnage;
	    log_string ("Size of ");  log_string (ship_name (ship));
	    log_string (" was increased to ");
	    log_string (commas (10000L * (long) tonnage));
	    log_string (" tons");
	}

	ship->tonnage = tonnage;
    }
    else
    {
	ship->remaining_cost -= cost;
	if (ship->remaining_cost == 0)
	{
	    ship->status = ON_SURFACE;	/* Construction is complete. */
	    if (continuing_construction)
	    {
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string ("An attempt will be made to finish construction on ");
		else
		    log_string ("Construction finished on ");
		log_string (ship_name (ship));
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string (" despite the siege");
	    }
	    else
	    {
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string ("An attempt will be made to construct ");
		log_string (ship_name (ship));
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string (" despite the siege");
		else
		    log_string (" was constructed");
	    }
	}
	else
	{
	    if (continuing_construction)
	    {
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string ("An attempt will be made to continue construction on ");
		else
		    log_string ("Construction continued on ");
		log_string (ship_name (ship));
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string (" despite the siege");
	    }
	    else
	    {
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string ("An attempt will be made to start construction on ");
		else
		    log_string ("Construction started on ");
		log_string (ship_name (ship));
		if (first_pass  &&  siege_effectiveness > 0)
		    log_string (" despite the siege");
	    }
	}
    }
    log_string (" at a cost of ");    log_long (cost + premium);

    if (interspecies_construction)
    {
	log_string (" for SP ");
	log_string (recipient_species->name);
    }

    log_char ('.');

    if (new_ship  &&  (! unused_ship_available))
    {
	++num_new_ships[species_index];
	++species->num_ships;
    }

    /* Check if planet is under siege and if construction was detected. */
    if (! first_pass  &&  rnd(100) <= siege_effectiveness)
    {
	log_string (" However, the work was detected by the besiegers and the ship was destroyed!!!");

	/* Make sure we don't notify the same species more than once. */
	for (i = 0; i < MAX_SPECIES; i++) already_notified[i] = FALSE;

	for (i = 0; i < num_transactions; i++)
	{
	    /* Find out who is besieging this planet. */
	    if (transaction[i].type != BESIEGE_PLANET) continue;
	    if (transaction[i].x != nampla->x) continue;
	    if (transaction[i].y != nampla->y) continue;
	    if (transaction[i].z != nampla->z) continue;
	    if (transaction[i].pn != nampla->pn) continue;
	    if (transaction[i].number2 != species_number) continue;

	    alien_number = transaction[i].number1;

	    if (already_notified[alien_number - 1]) continue;

	    /* Define a 'detection' transaction. */
	    if (num_transactions == MAX_TRANSACTIONS)
	    {
		fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		exit (-1);
	    }

	    n = num_transactions++;
	    transaction[n].type = DETECTION_DURING_SIEGE;
	    transaction[n].value = 2;	/* Construction of ship/starbase. */
	    strcpy (transaction[n].name1, nampla->name);
	    strcpy (transaction[n].name2, ship_name(ship));
	    strcpy (transaction[n].name3, species->name);
	    transaction[n].number3 = alien_number;

	    already_notified[alien_number - 1] = TRUE;
	}

	/* Remove ship from inventory. */
	delete_ship (ship);
    }

    log_char ('\n');

    if (! interspecies_construction) return;

    /* Transfer any cargo on the ship to the planet. */
    cargo_on_board = FALSE;
    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (ship->item_quantity[i] > 0)
	{
	    nampla->item_quantity[i] += ship->item_quantity[i];
	    ship->item_quantity[i] = 0;
	    cargo_on_board = TRUE;
	}
    }
    if (cargo_on_board)
	log_string ("      Forgotten cargo on the ship was first transferred to the planet.\n");

    /* Transfer the ship to the recipient species. */
    unused_ship_available = FALSE;
    recipient_ship = ship_data[g_spec_number - 1];
    for (i = 0; i < recipient_species->num_ships; i++)
    {
	if (recipient_ship->pn == 99)
	{
	    unused_ship_available = TRUE;
	    break;
	}

	++recipient_ship;
    }

    if (! unused_ship_available)
    {
	/* Make sure we have enough memory for new ship. */
	if (num_new_ships[g_spec_number - 1] == NUM_EXTRA_SHIPS)
	{
	    fprintf (stderr, "\n\n\tInsufficient memory for new recipient ship!\n\n");
	    exit (-1);
	}
	recipient_ship = ship_data[g_spec_number - 1]
		+ (int) recipient_species->num_ships;
	++recipient_species->num_ships;
	++num_new_ships[g_spec_number - 1];
    }

    /* Copy donor ship to recipient ship. */
    src = (char *) ship;
    dest = (char *) recipient_ship;
    for (i = 0; i < sizeof (struct ship_data); i++)
	*dest++ = *src++;

    recipient_ship->status = IN_ORBIT;

    data_modified[g_spec_number - 1] = TRUE;

    /* Delete donor ship. */
    delete_ship (ship);

    if (first_pass) return;

    /* Define transaction so that recipient will be notified. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    n = num_transactions++;
    transaction[n].type = INTERSPECIES_CONSTRUCTION;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = 2;	/* Ship, not items. */
    transaction[n].number3 = total_cost + premium;
    strcpy (transaction[n].name1, species->name);
    strcpy (transaction[n].name2, ship_name (recipient_ship));
}
