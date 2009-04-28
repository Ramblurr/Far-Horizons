
#include "fh.h"


extern int			num_stars, first_pass;
extern char			input_line[256], original_line[256],
				*input_line_pointer;
extern FILE			*log_file;
extern struct star_data		*star_base;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship;


do_WORMHOLE_command ()

{
    int		i, found, status;

    char	*original_line_pointer;

    struct star_data	*star;


    /* Get ship making the jump. */
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
	    fprintf (log_file, "!!! Invalid ship name in WORMHOLE command.\n");
	    return;
	}
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship (ship))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! This ship is salvage of a disbanded colony!\n");
	return;
    }

    /* Make sure ship can jump. */
    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s is still under construction!\n",
				ship_name (ship));
	return;
    }

    /* Check if JUMP, MOVE, or WORMHOLE was already done for this ship. */
    if (ship->just_jumped)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s already jumped or moved this turn!\n",
		ship_name (ship));
	return;
    }

    /* Find star. */
    star = star_base;
    found = FALSE;
    for (i = 0; i < num_stars; i++)
    {
	if (star->x == ship->x  &&  star->y == ship->y  &&  star->z == ship->z)
	{
	    found = star->worm_here;
	    break;
	}
	++star;
    }

    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! There is no wormhole at ship's location!\n");
	return;
    }

    /* Get the destination planet, if any. */
    get_location();
    if (nampla != NULL)
    {
	if (nampla->x != star->worm_x  ||  nampla->y != star->worm_y
	  ||  nampla->z != star->worm_z)
	{
	    fprintf (log_file, "!!! WARNING - Destination planet is not at other end of wormhole!\n");
	    nampla = NULL;
	}
    }

    /* Do the jump. */
    log_string ("    ");
    log_string (ship_name (ship));
    log_string (" will jump via natural wormhole at ");
    log_int (ship->x);  log_char (' ');
    log_int (ship->y);  log_char (' ');
    log_int (ship->z);
    ship->pn = 0;
    ship->status = IN_DEEP_SPACE;

    if (nampla != NULL)
    {
	log_string (" to PL ");
	log_string (nampla->name);
	ship->pn = nampla->pn;
	ship->status = IN_ORBIT;
    }
    log_string (".\n");
    ship->x = star->worm_x;
    ship->y = star->worm_y;
    ship->z = star->worm_z;
    ship->just_jumped = 99;	/* 99 indicates that a wormhole was used. */

    if (! first_pass) star_visited (ship->x, ship->y, ship->z);
}
