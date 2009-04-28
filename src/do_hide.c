
#include "fh.h"


extern int			doing_production, first_pass, abbr_index;
extern long			value, balance;
extern char			input_line[256];
extern FILE			*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla;


do_HIDE_command ()
{
    int		n, status;

    long	cost;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Make sure this is not a mining colony or home planet. */
    if (nampla->status & HOME_PLANET)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You may not HIDE a home planet.\n");
	return;
    }
    if (nampla->status & RESORT_COLONY)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You may not HIDE a resort colony.\n");
	return;
    }

    /* Check if planet is under siege. */
    if (nampla->siege_eff != 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Besieged planet cannot HIDE!\n");
	return;
    }

    /* Check if sufficient funds are available. */
    cost = (nampla->mi_base + nampla->ma_base) / 10L;
    if (nampla->status & MINING_COLONY)
    {
	if (cost > species->econ_units)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Mining colony does not have sufficient EUs to hide.\n");
	    return;
	}
	else
	    species->econ_units -= cost;
    }
    else if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Set 'hiding' flag. */
    nampla->hiding = TRUE;

    /* Log transaction. */
    log_string ("    Spent ");  log_long (cost);
    log_string (" hiding this colony.\n");
}

