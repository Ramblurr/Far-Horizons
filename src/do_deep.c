
#include "fh.h"


extern int			first_pass;
extern char			input_line[256], original_line[256],
				*input_line_pointer;
extern FILE			*log_file;
extern struct ship_data		*ship;


do_DEEP_command ()
{
    int		i, found;

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

    if (ship->type == STARBASE)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! DEEP order may not be given for a starbase.\n");
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

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship (ship))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! This ship is salvage of a disbanded colony!\n");
	return;
    }

    /* Move the ship. */
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;

    /* Log result. */
    log_string ("    ");
    log_string (ship_name (ship));
    log_string (" moved into deep space.\n");
}
