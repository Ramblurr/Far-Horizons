
#include "fh.h"


extern int			doing_production, first_pass, abbr_index;
extern long			value, balance;
extern char			input_line[256];
extern FILE			*log_file;
extern struct nampla_data	*nampla;


do_AMBUSH_command ()
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

    /* Get amount to spend. */
    status = get_value ();
    if (status == 0  ||  value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid or missing amount.\n");
	return;
    }
    if (value == 0) value = balance;
    if (value == 0) return;
    cost = value;

    /* Check if planet is under siege. */
    if (nampla->siege_eff != 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Besieged planet cannot ambush!\n");
	return;
    }

    /* Check if sufficient funds are available. */
    if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Increment amount spent on ambush. */
    nampla->use_on_ambush += cost;

    /* Log transaction. */
    log_string ("    Spent ");
    log_long (cost);
    log_string (" in preparation for an ambush.\n");
}
