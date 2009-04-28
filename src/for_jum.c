
#include "fh.h"
#include "combat.h"


extern int			log_summary, ignore_field_distorters;
extern char			field_distorted[MAX_SPECIES];
extern FILE			*log_file;
extern struct species_data	*c_species[MAX_SPECIES];
extern struct galaxy_data	galaxy;


/* This routine will return TRUE if forced jump or misjump units are used,
   even if they fail. It will return FALSE if the attacker has none or
   not enough. */

int forced_jump_units_used (attacker_index, defender_index, total_shots,
					bat, act)

int	attacker_index, defender_index, *total_shots;

struct battle_data	*bat;
struct action_data	*act;

{
    int		i, att_sp_index, def_sp_index, attacker_gv, defender_gv,
		type, fj_num, fm_num, number, success_chance, failure;

    char	x, y, z;

    struct ship_data	*attacking_ship, *defending_ship;



    /* Make sure attacking unit is a starbase. */
    attacking_ship = (struct ship_data *) act->fighting_unit[attacker_index];
    if (attacking_ship->type != STARBASE) return FALSE;

    /* See if attacker has any forced jump units. */
    fj_num = attacking_ship->item_quantity[FJ];
    fm_num = attacking_ship->item_quantity[FM];
    if (fj_num == 0  &&  fm_num == 0) return FALSE;

    /* If both types are being carried, choose one randomly. */
    if (fj_num > 0  &&  fm_num > 0)
    {
	if (rnd (2) == 1)
	{
	    type = FJ;
	    number = fj_num;
	}
	else
	{
	    type = FM;
	    number = fm_num;
	}
    }
    else if (fj_num > 0)
    {
	type = FJ;
	number = fj_num;
    }
    else
    {
	type = FM;
	number = fm_num;
    }

    /* Get gravitics tech levels. */
    att_sp_index = act->fighting_species_index[attacker_index];
    attacker_gv = c_species[att_sp_index]->tech_level[GV];

    def_sp_index = act->fighting_species_index[defender_index];
    defender_gv = c_species[def_sp_index]->tech_level[GV];

    /* Check if sufficient units are available. */
    defending_ship = (struct ship_data *) act->fighting_unit[defender_index];
    if (number < defending_ship->tonnage) return FALSE;

    /* Make sure defender is not a starbase. */
    if (defending_ship->type == STARBASE) return FALSE;

    /* Calculate percent chance of success. */
    success_chance = 2 *
	( (number - defending_ship->tonnage) + (attacker_gv - defender_gv) );

    /* See if it worked. */
    failure = rnd (100) > success_chance;

    log_summary = ! failure;

    log_string ("        ");  log_string (ship_name (attacking_ship));
	log_string (" attempts to use ");
	log_string (item_name[type]);
	log_string ("s against ");

    ignore_field_distorters = ! field_distorted[def_sp_index];
	log_string (ship_name (defending_ship));
    ignore_field_distorters = FALSE;

    if (failure)
    {
	log_string (", but fails.\n");
	return TRUE;
    }

    log_string (", and succeeds!\n");
    log_summary = FALSE;

    /* Determine destination. */
    if (type == FM)
    {
	/* Destination is totally random. */
	x = rnd (100) - 1;
	y = rnd (100) - 1;
	z = rnd (100) - 1;
    }
    else
    {
	/* Random location close to battle. */
	i = 3; while (i == 3) i = rnd(5);
	x = bat->x + i - 3;
	if (x < 0) x = 0;

	i = 3; while (i == 3) i = rnd(5);
	y = bat->y + i - 3;
	if (y < 0) y = 0;

	i = 3; while (i == 3) i = rnd(5);
	z = bat->z + i - 3;
	if (z < 0) z = 0;
    }
    defending_ship->dest_x = x;
    defending_ship->dest_y = y;
    defending_ship->dest_z = z;

    /* Make sure this ship can no longer take part in the battle. */
    defending_ship->status = FORCED_JUMP;
    defending_ship->pn = -1;
    *total_shots -= act->shots_left[defender_index];
    act->shots_left[defender_index] = 0;
    act->num_shots[defender_index] = 0;

    return TRUE;
}
