
#include "fh.h"


extern int			first_pass, num_transactions, species_number;
extern long			value;
extern char			input_line[256], original_line[256],
				*input_line_pointer, *ship_name();
extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_LAND_command ()
{
    int		i, n, found, siege_effectiveness, landing_detected, landed,
		alien_number, alien_index, alien_pn, array_index, bit_number,
		requested_alien_landing, alien_here, already_logged;

    long	bit_mask;

    char	*original_line_pointer;

    struct species_data		*alien;
    struct nampla_data		*alien_nampla;


    /* Get the ship. */
    original_line_pointer = input_line_pointer;
    found = get_ship ();
    if (! found)
    {
	/* Check for missing comma or tab after ship name. */
	input_line_pointer = original_line_pointer;
	fix_separator ();
	found = get_ship ();
	if (! found)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid ship name in LAND command.\n");
	    return;
	}
    }

    /* Make sure the ship is not a starbase. */
    if (ship->type == STARBASE)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! A starbase cannot land on a planet!\n");
	return;
    }

    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship is still under construction.\n");
	return;
    }

    if (ship->status == FORCED_JUMP  ||  ship->status == JUMPED_IN_COMBAT)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	return;
    }

    /* Get the planet number, if specified. */
    found = get_value ();

get_planet:

    alien_pn = 0;
    alien_here = FALSE;
    requested_alien_landing = FALSE;
    landed = FALSE;
    if (! found)
    {
	found = get_location ();
	if (! found  ||  nampla == NULL) found = FALSE;
    }
    else
    {
	/* Check if we or another species that has declared us ALLY has
		a colony on this planet. */
	found = FALSE;
	alien_pn = value;
	requested_alien_landing = TRUE;
	array_index = (species_number - 1) / 32;
	bit_number = (species_number - 1) % 32;
	bit_mask = 1 << bit_number;
	for (alien_index = 0; alien_index < galaxy.num_species; alien_index++)
	{
	    if (! data_in_memory[alien_index]) continue;

	    alien = &spec_data[alien_index];
	    alien_nampla = namp_data[alien_index] - 1;
	    for (i = 0; i < alien->num_namplas; i++)
	    {
		++alien_nampla;
		if (ship->x != alien_nampla->x) continue;
		if (ship->y != alien_nampla->y) continue;
		if (ship->z != alien_nampla->z) continue;
		if (alien_pn != alien_nampla->pn) continue;
		if ((alien_nampla->status & POPULATED) == 0) continue;

		if (alien_index  ==  species_number - 1)
		{
		    /* We have a colony here. No permission needed. */
		    nampla = alien_nampla;
		    found = TRUE;
		    alien_here = FALSE;
		    requested_alien_landing = FALSE;
		    goto finish_up;
		}

		alien_here = TRUE;

		if ((alien->ally[array_index] & bit_mask) == 0) continue;

		found = TRUE;
		break;
	    }

	    if (found) break;
	}
    }

finish_up:

    already_logged = FALSE;

    if (requested_alien_landing  &&  alien_here)
    {
	/* Notify the other alien(s). */
	landed = found;
	for (alien_index = 0; alien_index < galaxy.num_species; alien_index++)
	{
	    if (! data_in_memory[alien_index]) continue;

	    if (alien_index  ==  species_number - 1) continue;

	    alien = &spec_data[alien_index];
	    alien_nampla = namp_data[alien_index] - 1;
	    for (i = 0; i < alien->num_namplas; i++)
	    {
		++alien_nampla;
		if (ship->x != alien_nampla->x) continue;
		if (ship->y != alien_nampla->y) continue;
		if (ship->z != alien_nampla->z) continue;
		if (alien_pn != alien_nampla->pn) continue;
		if ((alien_nampla->status & POPULATED) == 0) continue;

		if ((alien->ally[array_index] & bit_mask) != 0)
		    found = TRUE;
		else
		    found = FALSE;

		if (landed  &&  ! found) continue;

		if (landed)
		    log_string ("    ");
		else
		    log_string ("!!! ");

		log_string (ship_name (ship));

		if (landed)
		    log_string (" was granted");
		else
		    log_string (" was denied");
		log_string (" permission to land on PL ");
		log_string (alien_nampla->name);
		log_string (" by SP ");
		log_string (alien->name);
		log_string (".\n");

		already_logged = TRUE;

		nampla = alien_nampla;

		if (first_pass) break;

		/* Define a 'landing request' transaction. */
		if (num_transactions == MAX_TRANSACTIONS)
		{
		    fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		    exit (-1);
		}

		n = num_transactions++;
		transaction[n].type = LANDING_REQUEST;
		transaction[n].value = landed;
		transaction[n].number1 = alien_index + 1;
		strcpy (transaction[n].name1, alien_nampla->name);
		strcpy (transaction[n].name2, ship_name(ship));
		strcpy (transaction[n].name3, species->name);

		break;
	    }
	}

	found = TRUE;
    }

    if (alien_here  &&  ! landed) return;

    if (! found)
    {
	if ((ship->status == IN_ORBIT  ||  ship->status == ON_SURFACE)
		&&  ! requested_alien_landing)
	{
	    /* Player forgot to specify planet. Use the one it's already at. */
	    value = ship->pn;
	    found = TRUE;
	    goto get_planet;
	}

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid or missing planet in LAND command.\n");
	return;
    }

    /* Make sure the ship and the planet are in the same star system. */
    if (ship->x != nampla->x  ||  ship->y != nampla->y
	||  ship->z != nampla->z)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship and planet are not in the same sector.\n");
	return;
    }

    /* Make sure planet is populated. */
    if ((nampla->status & POPULATED) == 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Planet in LAND command is not populated.\n");
	return;
    }

    /* Move the ship. */
    ship->pn = nampla->pn;
    ship->status = ON_SURFACE;

    if (already_logged) return;

    /* If the planet is under siege, the landing may be detected by the
	besiegers. */
    log_string ("    ");
    log_string (ship_name (ship));

    if (nampla->siege_eff != 0)
    {
	if (first_pass)
	{
	    log_string (" will attempt to land on PL ");
	    log_string (nampla->name);
	    log_string (" in spite of the siege");
	}
	else
	{
	    if (nampla->siege_eff < 0)
		siege_effectiveness = -nampla->siege_eff;
	    else
		siege_effectiveness = nampla->siege_eff;

	    landing_detected = FALSE;
	    if (rnd(100) <= siege_effectiveness)
	    {
		landing_detected = TRUE;
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

		    /* Define a 'detection' transaction. */
		    if (num_transactions == MAX_TRANSACTIONS)
		    {
			fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
			exit (-1);
		    }

		    n = num_transactions++;
		    transaction[n].type = DETECTION_DURING_SIEGE;
		    transaction[n].value = 1;	/* Landing. */
		    strcpy (transaction[n].name1, nampla->name);
		    strcpy (transaction[n].name2, ship_name(ship));
		    strcpy (transaction[n].name3, species->name);
		    transaction[n].number3 = alien_number;
		}
	    }

	    if (rnd(100) <= siege_effectiveness)
	    {
		/* Ship doesn't know if it was detected. */
		log_string (" may have been detected by the besiegers when it landed on PL ");
		log_string (nampla->name);
	    }
	    else
	    {
		/* Ship knows whether or not it was detected. */
		if (landing_detected)
		{
		    log_string (" was detected by the besiegers when it landed on PL ");
		    log_string (nampla->name);
		}
		else
		{
		    log_string (" landed on PL ");
		    log_string (nampla->name);
		    log_string (" without being detected by the besiegers");
		}
	    }
	}
    }
    else
    {
	if (first_pass)
	    log_string (" will land on PL ");
	else
	    log_string (" landed on PL ");
	log_string (nampla->name);
    }

    log_string (".\n");
}
