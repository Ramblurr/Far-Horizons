

#include "fh.h"
#include "combat.h"


extern int			truncate_name, ignore_field_distorters;
extern char			field_distorted[MAX_SPECIES];
extern FILE			*log_file, *summary_file;


withdrawal_check (bat, act)

struct battle_data	*bat;
struct action_data	*act;

{
    /* This routine will check all fighting ships and see if any wish to
	withdraw. If so, it will set the ship's status to JUMPED_IN_COMBAT.
	The actual jump will be handled by the Jump program. */

    int		i, old_trunc, ship_index, species_index, percent_loss,
		num_ships_gone[MAX_SPECIES], num_ships_total[MAX_SPECIES];

    char	withdraw_age;

    struct ship_data	*sh;


    for (i = 0; i < MAX_SPECIES; i++)
    {
	num_ships_gone[i] = 0;
	num_ships_total[i] = 0;
    }

    old_trunc = truncate_name;	/* Show age of ship here. */
    truncate_name = FALSE;

    /* Compile statistics and handle individual ships that must leave. */
    for (ship_index = 0; ship_index < act->num_units_fighting; ship_index++)
    {
	if (act->unit_type[ship_index] != SHIP)
	    continue;

	sh = (struct ship_data *) act->fighting_unit[ship_index];
	species_index = act->fighting_species_index[ship_index];
	++num_ships_total[species_index];

	if (sh->status == JUMPED_IN_COMBAT)	/* Already withdrawn. */
	{
		++num_ships_gone[species_index];
		continue;
	}

	if (sh->status == FORCED_JUMP)		/* Forced to leave. */
	{
		++num_ships_gone[species_index];
		continue;
	}

	if (sh->age > 49)			/* Already destroyed. */
	{
		++num_ships_gone[species_index];
		continue;
	}

	if (sh->type != FTL) continue;	/* Ship can't jump. */

	if (sh->class == TR)
	{
	    withdraw_age = bat->transport_withdraw_age[species_index];
	    if (withdraw_age == 0) continue;
		/* Transports will withdraw only when entire fleet withdraws. */
	}
	else
	    withdraw_age = bat->warship_withdraw_age[species_index];

	if (sh->age > withdraw_age)
	{
	    act->num_shots[ship_index] = 0;
	    act->shots_left[ship_index] = 0;
	    sh->pn = 0;

	    ignore_field_distorters = ! field_distorted[species_index];

	    fprintf (log_file, "        %s jumps away from the battle.\n",
		ship_name (sh));
	    fprintf (summary_file, "        %s jumps away from the battle.\n",
		ship_name (sh));

	    ignore_field_distorters = FALSE;

	    sh->dest_x = bat->haven_x[species_index];
	    sh->dest_y = bat->haven_y[species_index];
	    sh->dest_z = bat->haven_z[species_index];

	    sh->status = JUMPED_IN_COMBAT;

	    ++num_ships_gone[species_index];
	}
    }

    /* Now check if a fleet has reached its limit. */
    for (ship_index = 0; ship_index < act->num_units_fighting; ship_index++)
    {
	if (act->unit_type[ship_index] != SHIP)
	    continue;

	sh = (struct ship_data *) act->fighting_unit[ship_index];
	species_index = act->fighting_species_index[ship_index];

	if (sh->type != FTL) continue;			/* Ship can't jump. */
	if (sh->status == JUMPED_IN_COMBAT) continue;	/* Already withdrawn. */
	if (sh->status == FORCED_JUMP) continue;	/* Already gone. */
	if (sh->age > 49) continue;			/* Already destroyed. */

	if (bat->fleet_withdraw_percentage[species_index] == 0)
	    percent_loss = 101;		/* Always withdraw immediately. */
	else
	    percent_loss = (100 * num_ships_gone[species_index])
				/ num_ships_total[species_index];

	if (percent_loss > bat->fleet_withdraw_percentage[species_index])
	{
	    act->num_shots[ship_index] = 0;
	    act->shots_left[ship_index] = 0;
	    sh->pn = 0;

	    ignore_field_distorters = ! field_distorted[species_index];

	    fprintf (log_file, "        %s jumps away from the battle.\n",
		ship_name (sh));
	    fprintf (summary_file, "        %s jumps away from the battle.\n",
		ship_name (sh));

	    ignore_field_distorters = FALSE;

	    sh->dest_x = bat->haven_x[species_index];
	    sh->dest_y = bat->haven_y[species_index];
	    sh->dest_z = bat->haven_z[species_index];

	    sh->status = JUMPED_IN_COMBAT;
	}
    }

    truncate_name = old_trunc;
}
