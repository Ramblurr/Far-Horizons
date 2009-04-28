
#include "fh.h"
#include "combat.h"


extern int	num_transactions;
extern char	field_distorted[MAX_SPECIES];
extern short	germ_bombs_used[MAX_SPECIES][MAX_SPECIES];
extern struct planet_data	*planet_base;
extern struct species_data	*c_species[MAX_SPECIES];
extern struct nampla_data	*c_nampla[MAX_SPECIES];
extern struct ship_data		*c_ship[MAX_SPECIES];
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_germ_warfare (attacking_species, defending_species, defender_index, bat, act)

int			attacking_species, defending_species, defender_index;
struct battle_data	*bat;
struct action_data	*act;

{
    int		i, attacker_BI, defender_BI, success_chance, num_bombs,
		success;

    long	econ_units_from_looting;

    struct planet_data	*planet;
    struct nampla_data	*attacked_nampla;
    struct ship_data	*sh;


    attacker_BI = c_species[attacking_species]->tech_level[BI];
    defender_BI = c_species[defending_species]->tech_level[BI];
    attacked_nampla = (struct nampla_data *) act->fighting_unit[defender_index];
    planet = planet_base + (long) attacked_nampla->planet_index;

    success_chance = 50 + (2 * (attacker_BI - defender_BI));
    success = FALSE;
    num_bombs = germ_bombs_used[attacking_species][defending_species];

    for (i = 0; i < num_bombs; i++)
    {
	if (rnd(100) <= success_chance)
	{
	    success = TRUE;
	    break;
	}
    }

    if (success)
	log_string ("        Unfortunately");
    else
	log_string ("        Fortunately");

    log_string (" for the ");
    log_string (c_species[defending_species]->name);
    log_string (" defenders of PL ");
    log_string (attacked_nampla->name);
    log_string (", the ");
    i = bat->spec_num[attacking_species];
    if (field_distorted[attacking_species])
	log_int (distorted (i));
    else
	log_string (c_species[attacking_species]->name);
    log_string (" attackers ");

    if (! success)
    {
	log_string ("failed");

	if (num_bombs <= 0)
	    log_string (" because they didn't have any germ warfare bombs");

	log_string ("!\n");

	return;
    }

    log_string ("succeeded, using ");
    log_int (num_bombs);
    log_string (" germ warfare bombs. The defenders were wiped out!\n");

    /* Take care of looting. */
    econ_units_from_looting =
	(long) attacked_nampla->mi_base  +  (long) attacked_nampla->ma_base;

    if (attacked_nampla->status & HOME_PLANET)
    {
	if (c_species[defending_species]->hp_original_base < econ_units_from_looting)
	    c_species[defending_species]->hp_original_base = econ_units_from_looting;

	econ_units_from_looting *= 5;
    }

    if (econ_units_from_looting > 0)
    {
	/* Check if there's enough memory for a new interspecies transaction. */
	if (num_transactions == MAX_TRANSACTIONS)
	{
	    fprintf (stderr, "\nRan out of memory! MAX_TRANSACTIONS is too small!\n\n");
	    exit (-1);
	}
	i = num_transactions++;

	/* Define this transaction. */
	transaction[i].type = LOOTING_EU_TRANSFER;
	transaction[i].donor = bat->spec_num[defending_species];
	transaction[i].recipient = bat->spec_num[attacking_species];
	transaction[i].value = econ_units_from_looting;
	strcpy (transaction[i].name1, c_species[defending_species]->name);
	strcpy (transaction[i].name2, c_species[attacking_species]->name);
	strcpy (transaction[i].name3, attacked_nampla->name);
    }

    /* Finish off defenders. */
    attacked_nampla->mi_base = 0;
    attacked_nampla->ma_base = 0;
    attacked_nampla->IUs_to_install = 0;
    attacked_nampla->AUs_to_install = 0;
    attacked_nampla->pop_units = 0;
    attacked_nampla->siege_eff = 0;
    attacked_nampla->shipyards = 0;
    attacked_nampla->hiding = 0;
    attacked_nampla->hidden = 0;
    attacked_nampla->use_on_ambush = 0;

    for (i = 0; i < MAX_ITEMS; i++) attacked_nampla->item_quantity[i] = 0;

    /* Reset status word. */
    if (attacked_nampla->status & HOME_PLANET)
	attacked_nampla->status = HOME_PLANET;
    else
	attacked_nampla->status = COLONY;

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
}
