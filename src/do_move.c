
#include "fh.h"


extern int			x, y, z, first_pass;
extern char			input_line[256], original_line[256],
				*input_line_pointer;
extern FILE			*log_file;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship;



do_MOVE_command ()
{
    int		i, n, found;

    char	*original_line_pointer;


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
	    fprintf (log_file, "!!! Invalid ship name in MOVE command.\n");
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

    /* Check if JUMP or MOVE was already done for this ship. */
    if (ship->just_jumped)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s already jumped or moved this turn!\n",
		ship_name (ship));
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
    found = get_location ();
    if (! found  ||  nampla != NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! You may not use a planet name in MOVE command.\n");
	return;
    }

    /* Check if deltas are acceptable. */
    i = x - ship->x; if (i < 0) n = -i; else n = i;
    i = y - ship->y; if (i < 0) n += -i; else n += i;
    i = z - ship->z; if (i < 0) n += -i; else n += i;
    if (n > 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Destination is too far in MOVE command.\n");
	return;
    }

    /* Move the ship. */
    ship->x = x;
    ship->y = y;
    ship->z = z;
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;
    ship->just_jumped = 50;

    if (! first_pass) star_visited (x, y, z);

    /* Log result. */
    log_string ("    ");
    log_string (ship_name (ship));
    if (first_pass)
	log_string (" will move to sector ");
    else
	log_string (" moved to sector ");
    log_int (x);  log_char (' ');  log_int (y);  log_char (' ');  log_int (z);
    log_string (".\n");
}
