
#include "fh.h"


extern char			input_line[256];
extern FILE			*log_file;
extern struct nampla_data	*nampla;


do_DISBAND_command ()
{
    int		found;


    /* Get the planet. */
    found = get_location ();
    if (! found  ||  nampla == NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in DISBAND command.\n");
	return;
    }

    /* Make sure planet is not the home planet. */
    if (nampla->status & HOME_PLANET)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You cannot disband your home planet!\n");
	return;
    }

    /* Make sure planet is not under siege. */
    if (nampla->siege_eff)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You cannot disband a planet that is under siege!\n");
	return;
    }

    /* Mark the colony as "disbanded" and convert mining and manufacturing
	base to CUs, IUs, and AUs. */
    nampla->status |= DISBANDED_COLONY;
    nampla->item_quantity[CU] += nampla->mi_base + nampla->ma_base;
    nampla->item_quantity[IU] += nampla->mi_base / 2;
    nampla->item_quantity[AU] += nampla->ma_base / 2;
    nampla->mi_base = 0;
    nampla->ma_base = 0;

    /* Log the event. */
    log_string ("    The colony on PL ");
    log_string (nampla->name);
    log_string (" was ordered to disband.\n");
}

