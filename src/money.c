
#include "fh.h"


long	balance, raw_material_units, production_capacity, EU_spending_limit;


extern FILE	*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla;


int check_bounced (amount_needed)

long	amount_needed;

{
    long	take_from_EUs, limiting_balance;


    /* Check if we have sufficient funds for this purchase. */
    if (amount_needed > balance)
    {
	take_from_EUs = amount_needed - balance;

	if (take_from_EUs <= EU_spending_limit
	  &&  take_from_EUs <= species->econ_units)
	{
	    species->econ_units -= take_from_EUs;
	    EU_spending_limit -= take_from_EUs;
	    balance = amount_needed;
	}
	else
	    return TRUE;
    }

    /* Reduce various balances appropriately. */
    if (raw_material_units >= amount_needed)
    {
	if (production_capacity >= amount_needed)
	{
	    /* Enough of both. */
	    raw_material_units -= amount_needed;
	    production_capacity -= amount_needed;
	}
	else
	{
	    /* Enough RMs but not enough PC. */
	    raw_material_units -= production_capacity;
	    production_capacity = 0;
	}
    }
    else
    {
	if (production_capacity >= amount_needed)
	{
	    /* Enough PC but not enough RMs. */
	    production_capacity -= raw_material_units;
	    raw_material_units = 0;
	}
	else
	{
	    /* Not enough RMs or PC. */
	    limiting_balance = (raw_material_units > production_capacity)
				? production_capacity : raw_material_units;
	    raw_material_units -= limiting_balance;
	    production_capacity -= limiting_balance;
	}
    }

    balance -= amount_needed;

    return FALSE;
}



transfer_balance ()
{
    long	limiting_amount;


    /* Log end of production. Do not print ending balance for mining
	or resort colonies. */
    limiting_amount = 0;
    fprintf (log_file, "  End of production on PL %s.", nampla->name);
    if ( ! (nampla->status & (MINING_COLONY | RESORT_COLONY)) )
    {
	limiting_amount = (raw_material_units > production_capacity)
			? production_capacity : raw_material_units;
	fprintf (log_file, " (Ending balance is %ld.)", limiting_amount);
    }
    fprintf (log_file, "\n");

    /* Convert unused balance to economic units. */
    species->econ_units += limiting_amount;
    raw_material_units -= limiting_amount;

    /* Carry over unused raw material units into next turn. */
    nampla->item_quantity[RM] += raw_material_units;

    balance = 0;
}
