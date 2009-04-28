
#include "fh.h"


extern int	doing_production;
extern long	value, balance, EU_spending_limit;
extern char	input_line[256], original_line[256], *input_line_pointer;
extern FILE	*log_file;

extern struct species_data	*species;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship;


do_UPGRADE_command ()
{
    int		age_reduction, value_specified;

    char	*original_line_pointer;

    long	amount_to_spend, original_cost, max_funds_available;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Get the ship to be upgraded. */
    original_line_pointer = input_line_pointer;
    if (! get_ship ())
    {
	/* Check for missing comma or tab after ship name. */
	input_line_pointer = original_line_pointer;
	fix_separator ();
	if (! get_ship ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship to be upgraded does not exist.\n");
	    return;
	}
    }

    /* Make sure it didn't just jump. */
    if (ship->just_jumped)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship just jumped and is still in transit.\n");
	return;
    }

    /* Make sure it's in the same sector as the producing planet. */
    if (ship->x != nampla->x  ||  ship->y != nampla->y
	||  ship->z != nampla->z)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Item to be upgraded is not in the same sector as the production planet.\n");
	return;
    }

    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Item to be upgraded is still under construction.\n");
	return;
    }

    if (ship->age < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship or starbase is too new to upgrade.\n");
	return;
    }

    /* Calculate the original cost of the ship. */
    if (ship->class == TR  ||  ship->type == STARBASE)
	original_cost = ship_cost[ship->class] * ship->tonnage;
    else
	original_cost = ship_cost[ship->class];

    if (ship->type == SUB_LIGHT)
	original_cost = (3 * original_cost) / 4;

    /* Get amount to be spent. */
    if (value_specified = get_value ())
    {
	if (value == 0)
	    amount_to_spend = balance;
	else
	    amount_to_spend = value;

	age_reduction = (40 * amount_to_spend) / original_cost;
    }
    else
	age_reduction = ship->age;

try_again:

    if (age_reduction < 1)
    {
	if (value == 0) return;
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Amount specified is not enough to do an upgrade.\n");
	return;
    }

    if (age_reduction > ship->age) age_reduction = ship->age;

    /* Check if sufficient funds are available. */
    amount_to_spend = ((age_reduction * original_cost) + 39) / 40;
    if (check_bounced (amount_to_spend))
    {
	max_funds_available = species->econ_units;
	if (max_funds_available > EU_spending_limit)
	    max_funds_available = EU_spending_limit;
	max_funds_available += balance;

	if (max_funds_available > 0)
	{
	    if (value_specified)
	    {
		fprintf (log_file, "! WARNING: %s", input_line);
		fprintf (log_file, "! Insufficient funds. Substituting %ld for %ld.\n",
			max_funds_available, value);
	    }
	    amount_to_spend = max_funds_available;
	    age_reduction = (40 * amount_to_spend) / original_cost;
	    goto try_again;
	}

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Log what was upgraded. */
    log_string ("    ");  log_string (ship_name (ship));
    log_string (" was upgraded from age ");
    log_int ((int) ship->age);    log_string (" to age ");
    ship->age -= age_reduction;
    log_int ((int) ship->age);
    log_string (" at a cost of ");    log_long (amount_to_spend);
    log_string (".\n");
}
