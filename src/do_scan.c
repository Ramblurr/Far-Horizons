
#include "fh.h"


extern int			first_pass, test_mode;
extern char			input_line[256];
extern FILE			*log_file;
extern struct ship_data		*ship;


do_SCAN_command ()
{
    int		i, found, x, y, z;


    found = get_ship ();
    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid ship name in SCAN command.\n");
	return;
    }

    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship is still under construction.\n");
	return;
    }

    if (ship->status == FORCED_JUMP  ||  ship->status == JUMPED_IN_COMBAT)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	return;
    }

    /* Log the result. */
    if (first_pass)
    {
	log_string ("    A scan will be done by ");
	log_string (ship_name (ship));
	log_string (".\n");
	return;
    }

    /* Write scan of ship's location to log file. */
    x = ship->x;	y = ship->y;	z = ship->z;

    if (test_mode)
      fprintf (log_file, "\nA scan will be done by %s.\n\n", ship_name (ship));
    else
    {
      fprintf (log_file, "\nScan done by %s:\n\n", ship_name (ship));
      scan (x, y, z);
    }

    fprintf (log_file, "\n");
}
