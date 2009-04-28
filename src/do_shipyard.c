
#include "fh.h"


extern int			doing_production, first_pass, abbr_index,
				shipyard_built;
extern long			value, balance;
extern char			input_line[256];
extern FILE			*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla;


do_SHIPYARD_command ()
{
    long	cost;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Make sure this is not a mining or resort colony. */
    if ((nampla->status & MINING_COLONY)  ||  (nampla->status & RESORT_COLONY))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You may not build shipyards on a mining or resort colony!\n");
	return;
    }

    /* Check if planet has already built a shipyard. */
    if (shipyard_built)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Only one shipyard can be built per planet per turn!\n");
	return;
    }

    /* Check if sufficient funds are available. */
    cost = 10 * species->tech_level[MA];
    if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    ++nampla->shipyards;

    shipyard_built = TRUE;

    /* Log transaction. */
    log_string ("    Spent ");  log_long (cost);
    log_string (" to increase shipyard capacity by 1.\n");
}
