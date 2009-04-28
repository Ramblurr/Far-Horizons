
#include "fh.h"
#include "combat.h"



extern int	num_transactions;
extern char	x_attacked_y[MAX_SPECIES][MAX_SPECIES];

extern struct species_data	*c_species[MAX_SPECIES];
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_siege (bat, act)

struct battle_data	*bat;
struct action_data	*act;

{
    int		a, d, i, attacker_index, defender_index,
		attacking_species_number, defending_species_number;

    struct nampla_data	*defending_nampla;
    struct ship_data	*attacking_ship;
    struct species_data	*defending_species, *attacking_species;


    for (defender_index = 0; defender_index < act->num_units_fighting;
		defender_index++)
    {
	if (act->unit_type[defender_index] == BESIEGED_NAMPLA)
	{
	    defending_nampla =
		(struct nampla_data *) act->fighting_unit[defender_index];

	    defending_nampla->siege_eff = TRUE;

	    d = act->fighting_species_index[defender_index];
	    defending_species = c_species[d];
	    defending_species_number = bat->spec_num[d];

	    for (attacker_index = 0; attacker_index < act->num_units_fighting;
			attacker_index++)
	    {
		if (act->unit_type[attacker_index] == SHIP)
		{
		    attacking_ship =
			(struct ship_data *) act->fighting_unit[attacker_index];

		    a = act->fighting_species_index[attacker_index];

		    if (x_attacked_y[a][d])
		    {
			attacking_species = c_species[a];
			attacking_species_number = bat->spec_num[a];

			/* Check if there's enough memory for a new
				interspecies transaction. */
			if (num_transactions == MAX_TRANSACTIONS)
			{
			    fprintf (stderr, "\nRan out of memory! MAX_TRANSACTIONS is too small!\n\n");
			    exit (-1);
			}
			i = num_transactions++;

			/* Define this transaction. */
			transaction[i].type = BESIEGE_PLANET;
			transaction[i].x = defending_nampla->x;
			transaction[i].y = defending_nampla->y;
			transaction[i].z = defending_nampla->z;
			transaction[i].pn = defending_nampla->pn;
			transaction[i].number1 = attacking_species_number;
			strcpy (transaction[i].name1, attacking_species->name);
			transaction[i].number2 = defending_species_number;
			strcpy (transaction[i].name2, defending_species->name);
			strcpy (transaction[i].name3, attacking_ship->name);
		    }
		}
	    }
	}
    }

    log_string ("      Only those ships that actually remain in the system will take part in the siege.\n");
}
