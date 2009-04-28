
#include "fh.h"


extern int	ship_index, doing_production, correct_spelling_required,
		abbr_index;
extern long	value, raw_material_units, balance, EU_spending_limit;
extern char	input_line[256];
extern FILE	*log_file;

extern struct species_data	*species;
extern struct nampla_data	*nampla;
extern struct ship_data		*ship, *ship_base;


do_RECYCLE_command ()
{
    int		i, class, cargo;

    long	recycle_value, original_cost, units_available;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Get number of items to recycle. */
    i = get_value ();

    if (i == 0)
	goto recycle_ship;	/* Not an item. */

    /* Get class of item. */
    class = get_class_abbr ();

    if (class != ITEM_CLASS)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid item class in RECYCLE command.\n");
	return;
    }
    class = abbr_index;

    /* Make sure value is meaningful. */
    if (value == 0) value = nampla->item_quantity[class];
    if (value == 0) return;
    if (value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid item count in RECYCLE command.\n");
	return;
    }

    /* Make sure that items exist. */
    units_available = nampla->item_quantity[class];
    if (value > units_available)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Attempt to recycle more items than are available.\n");
	return;
    }

    /* Determine recycle value. */
    if (class == TP)
	recycle_value = (value * item_cost[class])
		/ (2L * (long) species->tech_level[BI]);
    else if (class == RM)
	recycle_value = value / 5L;
    else
	recycle_value = (value * item_cost[class]) / 2L;

    /* Update inventories. */
    nampla->item_quantity[class] -= value;
    if (class == PD  ||  class == CU) nampla->pop_units += value;
    species->econ_units += recycle_value;
    if (nampla->status & HOME_PLANET) EU_spending_limit += recycle_value;

    /* Log what was recycled. */
    log_string ("    ");  log_long (value);  log_char (' ');
    log_string (item_name[class]);

    if (value > 1)
	log_string ("s were");
    else
	log_string (" was");

    log_string (" recycled, generating ");  log_long (recycle_value);
    log_string (" economic units.\n");

    return;


recycle_ship:

    correct_spelling_required = TRUE;
    if (! get_ship ())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship to be recycled does not exist.\n");
	return;
    }

    /* Make sure it didn't just jump. */
    if (ship->just_jumped)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship just jumped and is still in transit.\n");
	return;
    }

    /* Make sure item is at producing planet. */
    if (ship->x != nampla->x  ||  ship->y != nampla->y
	||  ship->z != nampla->z  ||  ship->pn != nampla->pn)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship is not at the production planet.\n");
	return;
    }

    /* Calculate recycled value. */
    if (ship->class == TR  ||  ship->type == STARBASE)
	original_cost = ship_cost[ship->class] * ship->tonnage;
    else
	original_cost = ship_cost[ship->class];

    if (ship->type == SUB_LIGHT)
	original_cost = (3 * original_cost) / 4;

    if (ship->status == UNDER_CONSTRUCTION)
	recycle_value = (original_cost - (long) ship->remaining_cost) / 2;
    else
	recycle_value = (3 * original_cost * (60 - (long) ship->age)) / 200;

    species->econ_units += recycle_value;
    if (nampla->status & HOME_PLANET) EU_spending_limit += recycle_value;

    /* Log what was recycled. */
    log_string ("    ");  log_string (ship_name (ship));
    log_string (" was recycled, generating ");  log_long (recycle_value);
    log_string (" economic units");

    /* Transfer cargo, if any, from ship to planet. */
    cargo = FALSE;
    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (ship->item_quantity[i] > 0)
	{
	    nampla->item_quantity[i] += ship->item_quantity[i];
	    cargo = TRUE;
	}
    }

    if (cargo)
    {
	log_string (". Cargo onboard ");
	log_string (ship_name (ship));
	log_string (" was first transferred to PL ");
	log_string (nampla->name);
    }

    log_string (".\n");

    /* Remove ship from inventory. */
    delete_ship (ship);
}
