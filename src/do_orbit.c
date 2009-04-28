
#include "fh.h"


extern int			first_pass, num_stars;
extern long			value;
extern char			input_line[256], original_line[256],
				*input_line_pointer;
extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct star_data		*star_base, *star;
extern struct species_data	*species;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship;


do_ORBIT_command ()
{
    int		i, found, specified_planet_number;

    char	*original_line_pointer;


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
	    fprintf (log_file, "!!! Invalid ship name in ORBIT command.\n");
	    return;
	}
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

    /* Make sure this ship didn't just arrive via a MOVE command. */
    if (ship->just_jumped == 50)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! ORBIT not allowed immediately after a MOVE!\n");
	return;
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship (ship))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! This ship is salvage of a disbanded colony!\n");
	return;
    }

    /* Get the planet. */
    specified_planet_number = get_value ();

get_planet:

    if (specified_planet_number)
    {
	found = FALSE;
	specified_planet_number = value;
	for (i = 0; i < num_stars; i++)
	{
	    star = star_base + i;

	    if (star->x != ship->x) continue;
	    if (star->y != ship->y) continue;
	    if (star->z != ship->z) continue;

	    if (specified_planet_number >= 1
		&&  specified_planet_number <= star->num_planets)
			found = TRUE;

	    break;
	}

	if (! found)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid planet in ORBIT command.\n");
	    return;
	}

	ship->pn = specified_planet_number;

	goto finish_up;
    }

    found = get_location ();
    if (! found  ||  nampla == NULL)
    {
	if (ship->status == IN_ORBIT  || ship->status == ON_SURFACE)
	{
	    /* Player forgot to specify planet. Use the one it's already at. */
	    specified_planet_number = ship->pn;
	    value = specified_planet_number;
	    goto get_planet;
	}

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Invalid or missing planet in ORBIT command.\n");
	return;
    }

    /* Make sure the ship and the planet are in the same star system. */
    if (ship->x != nampla->x  ||  ship->y != nampla->y  ||  ship->z != nampla->z)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship and planet are not in the same sector.\n");
	return;
    }

    /* Move the ship. */
    ship->pn = nampla->pn;

finish_up:

    ship->status = IN_ORBIT;

    /* If a planet number is being used, see if it has a name.  If so,
	use the name. */
    if (specified_planet_number)
    {
	for (i = 0; i < species->num_namplas; i++)
	{
	    nampla = nampla_base + i;

	    if (nampla->x != ship->x) continue;
	    if (nampla->y != ship->y) continue;
	    if (nampla->z != ship->z) continue;
	    if (nampla->pn != specified_planet_number) continue;

	    specified_planet_number = 0;
	    break;
	}
    }

    /* Log result. */
    log_string ("    ");
    log_string (ship_name (ship));
    if (first_pass)
	log_string (" will enter orbit around ");
    else
	log_string (" entered orbit around ");

    if (specified_planet_number)
    {
	log_string ("planet number ");
	log_int (specified_planet_number);
    }
    else
    {
	log_string ("PL ");
	log_string (nampla->name);
    }

    log_string (".\n");
}
