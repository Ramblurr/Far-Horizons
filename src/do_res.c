
#include "fh.h"


extern int			doing_production, first_pass, abbr_index;
extern short			sp_tech_level[6];
extern long			value, balance, EU_spending_limit;
extern char			input_line[256];
extern FILE			*log_file;
extern struct species_data	*species;


do_RESEARCH_command ()
{
    int		n, status, tech, initial_level, current_level,
		need_amount_to_spend;

    long	cost, amount_spent, cost_for_one_level, funds_remaining,
		max_funds_available;


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
    need_amount_to_spend = (status == 0);	/* Sometimes players reverse
						   the arguments. */
    /* Get technology. */
    if (get_class_abbr () != TECH_ID)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid or missing technology.\n");
	return;
    }
    tech = abbr_index;

    if (species->tech_knowledge[tech] == 0  &&  sp_tech_level[tech] == 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Zero level can only be raised via TECH or TEACH.\n");
	return;
    }

    /* Get amount to spend if it was not obtained above. */
    if (need_amount_to_spend) status = get_value ();

    if (status == 0  ||  value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid or missing amount to spend!\n");
	return;
    }

do_cost:

    if (value == 0) value = balance;
    if (value == 0) return;
    cost = value;

    /* Check if sufficient funds are available. */
    if (check_bounced (cost))
    {
	max_funds_available = species->econ_units;
	if (max_funds_available > EU_spending_limit)
	    max_funds_available = EU_spending_limit;
	max_funds_available += balance;

	if (max_funds_available > 0)
	{
	    fprintf (log_file, "! WARNING: %s", input_line);
	    fprintf (log_file, "! Insufficient funds. Substituting %ld for %ld.\n",
		max_funds_available, cost);
	    value = max_funds_available;
	    goto do_cost;
	}

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Check if we already have knowledge of this technology. */
    funds_remaining = cost;
    amount_spent = 0;
    initial_level = sp_tech_level[tech];
    current_level = initial_level;
    while (current_level < species->tech_knowledge[tech])
    {
	cost_for_one_level = current_level * current_level;
	cost_for_one_level -= cost_for_one_level/4;	/* 25% discount. */
	if (funds_remaining < cost_for_one_level) break;
	funds_remaining -= cost_for_one_level;
	amount_spent += cost_for_one_level;
	++current_level;
    }

    if (current_level > initial_level)
    {
	log_string ("    Spent ");  log_long (amount_spent);
	log_string (" raising ");  log_string (tech_name[tech]);
	log_string (" tech level from ");  log_int (initial_level);
	log_string (" to ");  log_int (current_level);
	log_string (" using transferred knowledge.\n");

	sp_tech_level[tech] = current_level;
    }

    if (funds_remaining == 0) return;

    /* Increase in experience points is equal to whatever was not spent
	above. */
    species->tech_eps[tech] += funds_remaining;

    /* Log transaction. */
    log_string ("    Spent ");  log_long (funds_remaining);
    log_string (" on ");  log_string (tech_name[tech]);
    log_string (" research.\n");
}
