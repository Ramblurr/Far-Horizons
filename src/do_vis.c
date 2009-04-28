
#include "fh.h"


extern int			x, y, z;
extern char			input_line[256];
extern FILE			*log_file;
extern struct nampla_data	*nampla;


do_VISITED_command ()
{
    int	found;


    /* Get x y z coordinates. */
    found = get_location ();
    if (! found  ||  nampla != NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid coordinates in VISITED command.\n");
	return;
    }

    found = star_visited (x, y, z);

    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! There is no star system at these coordinates.\n");
	return;
    }

    /* Log result. */
    log_string ("    The star system at ");
    log_int (x);  log_char (' ');
    log_int (y);  log_char (' ');
    log_int (z);
    log_string (" was marked as visited.\n");
}
