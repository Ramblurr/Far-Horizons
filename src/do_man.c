
#include "fh.h"


extern int			nampla_index, doing_production;
extern long			value, balance;
extern char			input_line[256], manufacturing_done[1000];
extern FILE			*log_file;
extern struct nampla_data	*nampla;


do_MANUFACTURING_command ()
{
    int		status;

    long	cost, increment, old_base, original_value;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (stderr, "\n\n\tMissing PRODUCTION order!\n\n");
	exit (-1);
    }

    /* Check if MANUFACTURING order was already executed for this planet. */
    if (manufacturing_done[nampla_index])
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Only one MANUFACTURING command allowed per planet.\n");
	return;
    }

    /* Get amount of increment. */
    status = get_value ();
    if (status == 0  ||  value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid or missing amount.\n");
	return;
    }
    original_value = value;

    old_base = nampla->ma_base;
    if (value == 0)
    {
	increment = (3 * old_base)/100;
	if (increment > balance) increment = balance;
	if (increment > nampla->pop_units) increment = nampla->pop_units;
	if (increment == 0) return;
    }
    else
	increment = value;

    /* Check if amount requested exceeds 3% growth limit. */
    if (increment > (3 * old_base)/100)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Specified increase exceeds 3%% growth limit.\n");
	return;
    }

    /* Check if sufficient funds are available. */
    cost = increment;
    if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* If the planet is not self-sufficient, make sure there is enough
	available population. */
    if ((nampla->status & SELF_SUFFICIENT) == 0)
    {
	if (nampla->pop_units < increment)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Insufficient available population units.\n");
	    return;
	}
	nampla->pop_units -= increment;
    }

    /* Increase base and log transaction. */
    nampla->ma_base += increment;
    log_string ("    Manufacturing base was increased by ");
    log_long (increment/10); log_char ('.'); log_long (increment%10);
    log_string (" (from ");
    log_long (old_base/10); log_char ('.'); log_long (old_base%10);
    log_string (" to ");
    log_long (nampla->ma_base/10); log_char ('.'); log_long (nampla->ma_base%10);
    log_string (").\n");

    manufacturing_done[nampla_index] = TRUE;
}
