
#include "fh.h"
#include "combat.h"


long	power ();


extern struct planet_data	*planet_base;
extern struct species_data	*c_species[MAX_SPECIES];
extern struct nampla_data	*c_nampla[MAX_SPECIES];
extern struct ship_data		*c_ship[MAX_SPECIES];


do_bombardment (unit_index, act)

int			unit_index;
struct action_data	*act;

{
    int			i, new_mi, new_ma, defending_species;

    long		n, total_bomb_damage, CS_bomb_damage, new_pop,
			initial_base, total_pop, percent_damage;

    struct planet_data	*planet;
    struct nampla_data	*attacked_nampla;
    struct ship_data	*sh;


    attacked_nampla = (struct nampla_data *) act->fighting_unit[unit_index];
    planet = planet_base + (long) attacked_nampla->planet_index;

    initial_base = attacked_nampla->mi_base + attacked_nampla->ma_base;
    total_pop = initial_base;

    if (attacked_nampla->item_quantity[CU] > 0) total_pop += 1;

    if (total_pop < 1)
    {
	log_string ("        The planet is completely uninhabited. There is nothing to bomb!\n");
	return;
    }

    /* Total damage done by ten strike cruisers (ML = 50) in ten rounds
	is 100 x 4 x the power value for a single ship. To eliminate the
	chance of overflow, the algorithm has been carefully chosen. */

    CS_bomb_damage = 400 * power (ship_tonnage[CS]);
	/* Should be 400 * 4759 = 1,903,600. */

    total_bomb_damage = act->bomb_damage[unit_index];

    /* Keep about 2 significant digits. */
    while (total_bomb_damage > 1000)
    {
	total_bomb_damage /= 10;
	CS_bomb_damage /= 10;
    }

    if (CS_bomb_damage == 0)
	percent_damage = 101;
    else
	percent_damage =
	    ((total_bomb_damage * 250000L) / CS_bomb_damage) / total_pop;

    if (percent_damage > 100) percent_damage = 101;

    new_mi = attacked_nampla->mi_base
	- (percent_damage * attacked_nampla->mi_base)/100;

    new_ma = attacked_nampla->ma_base
	- (percent_damage * attacked_nampla->ma_base)/100;

    new_pop = attacked_nampla->pop_units
	- (percent_damage * attacked_nampla->pop_units)/100;

    if (new_mi == attacked_nampla->mi_base
	&&  new_ma == attacked_nampla->ma_base
	&&  new_pop == attacked_nampla->pop_units)
    {
	log_string ("        Damage due to bombardment was insignificant.\n");
	return;
    }

    defending_species = act->fighting_species_index[unit_index];
    if (attacked_nampla->status & HOME_PLANET)
    {
	n = attacked_nampla->mi_base + attacked_nampla->ma_base;
	if (c_species[defending_species]->hp_original_base < n)
	    c_species[defending_species]->hp_original_base = n;
    }

    if (new_mi <= 0  &&  new_ma <= 0  &&  new_pop <= 0)
    {
	log_string ("        Everyone and everything was completely wiped out!\n");

	attacked_nampla->mi_base = 0;
	attacked_nampla->ma_base = 0;
	attacked_nampla->pop_units = 0;
	attacked_nampla->siege_eff = 0;
	attacked_nampla->shipyards = 0;
	attacked_nampla->hiding = 0;
	attacked_nampla->hidden = 0;
	attacked_nampla->use_on_ambush = 0;

	/* Reset status. */
	if (attacked_nampla->status & HOME_PLANET)
	    attacked_nampla->status = HOME_PLANET;
	else
	    attacked_nampla->status = COLONY;

	for (i = 0; i < MAX_ITEMS; i++)
	    attacked_nampla->item_quantity[i] = 0;

	/* Delete any ships that were under construction on the planet. */
	sh = c_ship[defending_species] - 1;
	for (i = 0; i < c_species[defending_species]->num_ships; i++)
	{
	    ++sh;

	    if (sh->x != attacked_nampla->x) continue;
	    if (sh->y != attacked_nampla->y) continue;
	    if (sh->z != attacked_nampla->z) continue;
	    if (sh->pn != attacked_nampla->pn) continue;

	    delete_ship (sh);
	}

	return;
    }

    log_string ("        Mining base of PL ");
    log_string (attacked_nampla->name);
    log_string (" went from ");
    log_int (attacked_nampla->mi_base / 10);
    log_char ('.');
    log_int (attacked_nampla->mi_base % 10);
    log_string (" to ");
    attacked_nampla->mi_base = new_mi;
    log_int (new_mi / 10);
    log_char ('.');
    log_int (new_mi % 10);
    log_string (".\n");

    log_string ("        Manufacturing base of PL ");
    log_string (attacked_nampla->name);
    log_string (" went from ");
    log_int (attacked_nampla->ma_base / 10);
    log_char ('.');
    log_int (attacked_nampla->ma_base % 10);
    log_string (" to ");
    attacked_nampla->ma_base = new_ma;
    log_int (new_ma / 10);
    log_char ('.');
    log_int (new_ma % 10);
    log_string (".\n");

    attacked_nampla->pop_units = new_pop;

    for (i = 0; i < MAX_ITEMS; i++)
    {
	n = (percent_damage * attacked_nampla->item_quantity[i]) / 100;
	if (n > 0)
	{
	    attacked_nampla->item_quantity[i] -= n;
	    log_string ("        ");    log_long (n);    log_char (' ');
	    log_string (item_name[i]);
	    if (n > 1)
		log_string ("s were");
	    else
		log_string (" was");
	    log_string (" destroyed.\n");
	}
    }

    n = (percent_damage * (long) attacked_nampla->shipyards) / 100;
    if (n > 0)
    {
	attacked_nampla->shipyards -= n;
	log_string ("        ");    log_long (n);
	log_string (" shipyard");
	if (n > 1)
	    log_string ("s were");
	else
	    log_string (" was");
	log_string (" also destroyed.\n");
    }

    check_population (attacked_nampla);
}
