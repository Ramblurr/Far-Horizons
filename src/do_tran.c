
#include "fh.h"


extern int			abbr_type, abbr_index, species_number,
				first_pass, num_transactions,
				post_arrival_phase;
extern long			value;
extern char			input_line[256], original_line[256],
				*input_line_pointer;
extern FILE			*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_TRANSFER_command ()

{
    int		i, n, item_class, item_count, capacity, transfer_type,
		attempt_during_siege, siege_1_chance, siege_2_chance,
		alien_number, first_try, both_args_present, need_destination;

    char	c, x1, x2, y1, y2, z1, z2, *original_line_pointer, *temp_ptr,
		already_notified[MAX_SPECIES];

    long	num_available, original_count;

    struct nampla_data		*nampla1, *nampla2, *temp_nampla;
    struct ship_data		*ship1, *ship2;


    /* Get number of items to transfer. */
    i = get_value ();

    /* Make sure value is meaningful. */
    if (i == 0  ||  value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid item count in TRANSFER command.\n");
	return;
    }
    original_count = value;
    item_count = value;

    /* Get class of item. */
    item_class = get_class_abbr ();

    if (item_class != ITEM_CLASS)
    {
	/* Players sometimes accidentally use "MI" for "IU"
		or "MA" for "AU". */
	if (item_class == TECH_ID  &&  abbr_index == MI)
	    abbr_index = IU;
	else if (item_class == TECH_ID  &&  abbr_index == MA)
	    abbr_index = AU;
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid item class!\n");
	    return;
	}
    }
    item_class = abbr_index;

    /* Get source of transfer. */
    nampla1 = NULL;
    nampla2 = NULL;
    original_line_pointer = input_line_pointer;
    if (! get_transfer_point ())
    {
	/* Check for missing comma or tab after source name. */
	input_line_pointer = original_line_pointer;
	fix_separator ();
	if (! get_transfer_point ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid source location in TRANSFER command.\n");
	    return;
	}
    }

    /* Test if the order has both a source and a destination.  Sometimes,
	the player will accidentally omit the source if it's "obvious". */
    temp_ptr = input_line_pointer;
    both_args_present = FALSE;
    while (1)
    {
	c = *temp_ptr++;

	if (c == ';'  ||  c == '\n') break;	/* End of order. */

	if (isalpha (c))
	{
	    both_args_present = TRUE;
	    break;
	}
    }

    need_destination = TRUE;

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS)
    {
	ship1 = ship;

	if (ship1->status == UNDER_CONSTRUCTION)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s is still under construction!\n",
				ship_name (ship1));
	    return;
	}

	if (ship1->status == FORCED_JUMP  ||  ship1->status == JUMPED_IN_COMBAT)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	    return;
	}

	x1 = ship1->x;	y1 = ship1->y;	z1 = ship1->z;

	num_available = ship1->item_quantity[item_class];

check_ship_items:

	if (item_count == 0) item_count = num_available;
	if (item_count == 0) return;

	if (num_available < item_count)
	{
	    if (both_args_present)	/* Change item count to "0". */
	    {
		if (num_available == 0)
		{
		    fprintf (log_file, "!!! Order ignored:\n");
		    fprintf (log_file, "!!! %s", original_line);
		    fprintf (log_file, "!!! %s does not have specified item(s)!\n",
				ship_name (ship1));
		    return;
		}

		fprintf (log_file, "! WARNING: %s", original_line);
		fprintf (log_file, "! Ship does not have %d units. Substituting %d for %d!\n",
		    item_count, num_available, item_count);
		item_count = 0;
		goto check_ship_items;
	    }

	    /* Check if ship is at a planet that has the items. If so,
		we'll assume that the planet is the source and the ship is
		the destination. We'll look first for a planet that the
		ship is actually landed on or orbiting. If that fails,
		then we'll look for a planet in the same sector. */

	    first_try = TRUE;

	next_ship_try:

	    nampla1 = nampla_base - 1;
	    for (i = 0; i < species->num_namplas; i++)
	    {
		++nampla1;

		if (nampla1->x != ship1->x) continue;
		if (nampla1->y != ship1->y) continue;
		if (nampla1->z != ship1->z) continue;
		if (first_try)
		{
		    if (nampla1->pn != ship1->pn)
			continue;
		}

		num_available = nampla1->item_quantity[item_class];
		if (num_available < item_count) continue;

		ship = ship1;		/* Destination. */
		transfer_type = 1;	/* Source = planet. */
		abbr_type = SHIP_CLASS;	/* Destination type. */

		need_destination = FALSE;

		goto get_destination;
	    }

	    if (first_try)
	    {
		first_try = FALSE;
		goto next_ship_try;
	    }

	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s does not have specified item(s)!\n",
				ship_name (ship1));
	    return;
	}

	transfer_type = 0;	/* Source = ship. */
    }
    else	/* Source is a planet. */
    {
	nampla1 = nampla;

	x1 = nampla1->x;	y1 = nampla1->y;	z1 = nampla1->z;

	num_available = nampla1->item_quantity[item_class];

check_planet_items:

	if (item_count == 0) item_count = num_available;
	if (item_count == 0) return;

	if (num_available < item_count)
	{
	    if (both_args_present)	/* Change item count to "0". */
	    {
		if (num_available == 0)
		{
		    fprintf (log_file, "!!! Order ignored:\n");
		    fprintf (log_file, "!!! %s", original_line);
		    fprintf (log_file, "!!! PL %s does not have specified item(s)!\n",
				nampla1 -> name);
		    return;
		}

		fprintf (log_file, "! WARNING: %s", original_line);
		fprintf (log_file, "! Planet does not have %d units. Substituting %d for %d!\n",
		    item_count, num_available, item_count);
		item_count = 0;
		goto check_planet_items;
	    }

	    /* Check if another planet in the same sector has the items.
		If so, we'll assume that it is the source and that the
		named planet is the destination. */

	    temp_nampla = nampla_base - 1;
	    for (i = 0; i < species->num_namplas; i++)
	    {
		++temp_nampla;

		if (temp_nampla->x != nampla1->x) continue;
		if (temp_nampla->y != nampla1->y) continue;
		if (temp_nampla->z != nampla1->z) continue;

		num_available = temp_nampla->item_quantity[item_class];
		if (num_available < item_count) continue;

		nampla = nampla1;	/* Destination. */
		nampla1 = temp_nampla;	/* Source. */
		transfer_type = 1;	/* Source = planet. */
		abbr_type = PLANET_ID;	/* Destination type. */

		need_destination = FALSE;

		goto get_destination;
	    }

	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! PL %s does not have specified item(s)!\n",
				nampla1 -> name);
	    return;
	}

	transfer_type = 1;	/* Source = planet. */
    }

get_destination:

    /* Get destination of transfer. */
    if (need_destination)
    {
	if (! get_transfer_point ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid destination location.\n");
	    return;
	}
    }

    /* Make sure everything makes sense. */
    if (abbr_type == SHIP_CLASS)
    {
	ship2 = ship;

	if (ship2->status == UNDER_CONSTRUCTION)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! %s is still under construction!\n",
				ship_name (ship2));
	    return;
	}

	if (ship2->status == FORCED_JUMP  ||  ship2->status == JUMPED_IN_COMBAT)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	    return;
	}

	/* Check if destination ship has sufficient carrying capacity. */
	if (ship2->class == TR)
	    capacity = (10 + ((int) ship2->tonnage / 2)) * (int) ship2->tonnage;
	else if (ship2->class == BA)
	    capacity = 10 * ship2->tonnage;
	else
	    capacity = ship2->tonnage;

	for (i = 0; i < MAX_ITEMS; i++)
	    capacity -= ship2->item_quantity[i] * item_carry_capacity[i];

    do_capacity:

	if (original_count == 0)
	{
	    i = capacity / item_carry_capacity[item_class];
	    if (i < item_count) item_count = i;
	    if (item_count == 0) return;
	}

	if (capacity < item_count * item_carry_capacity[item_class])
	{
	    fprintf (log_file, "! WARNING: %s", original_line);
	    fprintf (log_file, "! %s does not have sufficient carrying capacity!",
				ship_name (ship2));
	    fprintf (log_file, " Changed %d to 0.\n", original_count);
	    original_count = 0;
	    goto do_capacity;
	}

	x2 = ship2->x;	y2 = ship2->y;	z2 = ship2->z;
    }
    else
    {
	nampla2 = nampla;

	x2 = nampla2->x;	y2 = nampla2->y;	z2 = nampla2->z;

	transfer_type |= 2;

	/* If this is the post-arrival phase, then make sure the planet
		is populated. */
	if (post_arrival_phase  &&  ((nampla2->status & POPULATED) == 0))
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Destination planet must be populated for post-arrival TRANSFERs.\n");
	    return;
	}
    }

    /* Check if source and destination are in same system. */
    if (x1 != x2  ||  y1 != y2  ||  z1 != z2)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Source and destination are not at same 'x y z' in TRANSFER command.\n");
	return;
    }

    /* Check for siege. */
    siege_1_chance = 0;
    siege_2_chance = 0;
    if (transfer_type == 3	/* Planet to planet. */
	&&  (nampla1->siege_eff != 0  ||  nampla2->siege_eff != 0))
    {
	if (nampla1->siege_eff >= 0)
	    siege_1_chance = nampla1->siege_eff;
	else
	    siege_1_chance = -nampla1->siege_eff;

	if (nampla2->siege_eff >= 0)
	    siege_2_chance = nampla2->siege_eff;
	else
	    siege_2_chance = -nampla2->siege_eff;

	attempt_during_siege = TRUE;
    }
    else
	attempt_during_siege = FALSE;

    /* Make the transfer and log the result. */
    log_string ("    ");

    if (attempt_during_siege  &&  first_pass)
	log_string ("An attempt will be made to transfer ");

    log_int (item_count);  log_char (' ');  log_string (item_name[item_class]);

    if (attempt_during_siege  &&  first_pass)
    {
	if (item_count > 1) log_char ('s');
	log_char (' ');
    }
    else
    {
	if (item_count > 1)
	    log_string ("s were transferred from ");
	else
	    log_string (" was transferred from ");
    }

    switch (transfer_type)
    {
	case 0:		/* Ship to ship. */
		ship1->item_quantity[item_class] -= item_count;
		ship2->item_quantity[item_class] += item_count;
		log_string (ship_name (ship1));  log_string (" to ");
		log_string (ship_name (ship2));  log_char ('.');
		break;

	case 1:		/* Planet to ship. */
		nampla1->item_quantity[item_class] -= item_count;
		ship2->item_quantity[item_class] += item_count;
		if (item_class == CU)
		{
		    if (nampla1 == nampla_base)
			ship2->loading_point = 9999;	/* Home planet. */
		    else
			ship2->loading_point = (nampla1 - nampla_base);
		}
		log_string ("PL ");  log_string (nampla1->name);
		log_string (" to ");  log_string (ship_name (ship2));
		log_char ('.');
		break;

	case 2:		/* Ship to planet. */
		ship1->item_quantity[item_class] -= item_count;
		nampla2->item_quantity[item_class] += item_count;
		log_string (ship_name (ship1));  log_string (" to PL ");
		log_string (nampla2->name);  log_char ('.');
		break;

	case 3:		/* Planet to planet. */
		nampla1->item_quantity[item_class] -= item_count;
		nampla2->item_quantity[item_class] += item_count;

		log_string ("PL ");  log_string (nampla1->name);
		log_string (" to PL ");  log_string (nampla2->name);
		if (attempt_during_siege) log_string (" despite the siege");
		log_char ('.');

		if (first_pass) break;

		/* Check if either planet is under siege and if transfer
			was detected by the besiegers. */
		if (rnd(100) > siege_1_chance  &&  rnd(100) > siege_2_chance)
		    break;

		log_string (" However, the transfer was detected by the besiegers and the items were destroyed!!!");
		nampla2->item_quantity[item_class] -= item_count;

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
		    transaction[n].value = 4;	/* Transfer of items. */
		    transaction[n].number1 = item_count;
		    transaction[n].number2 = item_class;
		    if (siege_1_chance > siege_2_chance)
		    {
			/* Besieged planet is the source of the transfer. */
			transaction[n].value = 4;
			strcpy (transaction[n].name1, nampla1->name);
			strcpy (transaction[n].name2, nampla2->name);
		    }
		    else
		    {
			/* Besieged planet is the destination of the transfer. */
			transaction[n].value = 5;
			strcpy (transaction[n].name1, nampla2->name);
			strcpy (transaction[n].name2, nampla1->name);
		    }
		    strcpy (transaction[n].name3, species->name);
		    transaction[n].number3 = alien_number;

		    already_notified[alien_number - 1] = TRUE;
		}

		break;

	default:	/* Internal error. */
		fprintf (stderr, "\n\n\tInternal error: transfer type!\n\n");
		exit (-1);
    }

    log_char ('\n');

    if (nampla1 != NULL) check_population (nampla1);
    if (nampla2 != NULL) check_population (nampla2);
}
