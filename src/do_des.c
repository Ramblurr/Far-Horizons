
#include "fh.h"


extern int			first_pass, correct_spelling_required;
extern char			input_line[256];
extern FILE			*log_file;
extern struct species_data	*species;
extern struct ship_data		*ship;


do_DESTROY_command ()
{
    int		found;


    /* Get the ship. */
    correct_spelling_required = TRUE;
    found = get_ship ();
    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid ship or starbase name in DESTROY command.\n");
	return;
    }

    /* Log result. */
    log_string ("    ");
    log_string (ship_name (ship));

    if (first_pass)
    {
	log_string (" will be destroyed.\n");
	return;
    }

    log_string (" was destroyed.\n");

    delete_ship (ship);
}

