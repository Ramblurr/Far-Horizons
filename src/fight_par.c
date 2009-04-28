
/* The following routine will fill "act" with ship and nampla data necessary
   for an action; i.e., number of shots per round, damage done per shot,
   total shield power, etc. Note that this routine always restores shields
   completely. It is assumed that a sufficient number of rounds passes
   between actions of a battle to completely regenerate shields. 

   The routine will return TRUE if the action can take place, otherwise
   FALSE.
*/


#include "fh.h"
#include "combat.h"

extern int			deep_space_defense, attacking_ML, defending_ML;

extern struct species_data	*c_species[MAX_SPECIES];
extern struct nampla_data	*c_nampla[MAX_SPECIES];
extern struct ship_data		*c_ship[MAX_SPECIES];

long	power ();


int fighting_params (option, location, bat, act)

char			option, location;
struct battle_data	*bat;
struct action_data	*act;

{
    char	x, y, z, pn;

    int		i, j, found, type, num_sp, unit_index, species_index,
		ship_index, nampla_index, sp1, sp2, use_this_ship, n_shots,
		engage_option, engage_location, attacking_ships_here,
		defending_ships_here, attacking_pds_here, defending_pds_here,
		num_fighting_units;

    short	tons;

    long	ml, ls, unit_power, offensive_power, defensive_power;

    struct ship_data	*sh;
    struct nampla_data	*nam;


    /* Add fighting units to "act" arrays. At the same time, check if
	a fight of the current option type will occur at the current
	location. */
    num_fighting_units = 0;
    x = bat->x;
    y = bat->y;
    z = bat->z;
    attacking_ML = 0;
    defending_ML = 0;
    attacking_ships_here = FALSE;
    defending_ships_here = FALSE;
    attacking_pds_here = FALSE;
    defending_pds_here = FALSE;
    deep_space_defense = FALSE;
    num_sp = bat->num_species_here;

    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	/* Check which ships can take part in fight. */
	sh = c_ship[species_index] - 1;
	for (ship_index = 0; ship_index < c_species[species_index]->num_ships; ship_index++)
	{
	    ++sh;
	    use_this_ship = FALSE;

	    if (sh->x != x) continue;
	    if (sh->y != y) continue;
	    if (sh->z != z) continue;
	    if (sh->pn == 99) continue;
	    if (sh->age > 49) continue;
	    if (sh->status == UNDER_CONSTRUCTION) continue;
	    if (sh->status == FORCED_JUMP) continue;
	    if (sh->status == JUMPED_IN_COMBAT) continue;
	    if (sh->class == TR  &&  sh->pn != location
		&&  option != GERM_WARFARE) continue;
	    if (disbanded_ship (species_index, sh)) continue;
	    if (option == SIEGE  ||  option == PLANET_BOMBARDMENT)
		if (sh->special == NON_COMBATANT) continue;

	    for (i = 0; i < bat->num_engage_options[species_index]; i++)
	    {
		engage_option = bat->engage_option[species_index][i];
		engage_location = bat->engage_planet[species_index][i];

		switch (engage_option)
		{
		    case DEFENSE_IN_PLACE:
			if (sh->pn != location) break;
			defending_ships_here = TRUE;
			use_this_ship = TRUE;
			break;

		    case DEEP_SPACE_DEFENSE:
			if (option != DEEP_SPACE_FIGHT) break;
			if (sh->class == BA  &&  sh->pn != 0) break;
			defending_ships_here = TRUE;
			use_this_ship = TRUE;
			deep_space_defense = TRUE;
			if (c_species[species_index]->tech_level[ML] > defending_ML)
			    defending_ML = c_species[species_index]->tech_level[ML];
			break;

		    case PLANET_DEFENSE:
			if (location != engage_location) break;
			if (sh->class == BA  &&  sh->pn != location) break;
			defending_ships_here = TRUE;
			use_this_ship = TRUE;
			break;

		    case DEEP_SPACE_FIGHT:
			if (option != DEEP_SPACE_FIGHT) break;
			if (sh->class == BA  &&  sh->pn != 0) break;
			if (c_species[species_index]->tech_level[ML] > defending_ML)
			    defending_ML = c_species[species_index]->tech_level[ML];
			defending_ships_here = TRUE;
			attacking_ships_here = TRUE;
			use_this_ship = TRUE;
			break;

		    case PLANET_ATTACK:
		    case PLANET_BOMBARDMENT:
		    case GERM_WARFARE:
		    case SIEGE:
			if (sh->class == BA  &&  sh->pn != location) break;
			if (sh->class == TR  &&  option == SIEGE) break;
			if (option == DEEP_SPACE_FIGHT)
			{
			    /* There are two possibilities here: 1. outsiders
				are attacking locals, or 2. locals are attacking
				locals. If (1), we want outsiders to first fight
				in deep space. If (2), locals will not first
				fight in deep space (unless other explicit
				orders were given). The case is (2) if current
				species has a planet here. */

			    found = FALSE;
			    for (nampla_index = 0; nampla_index < c_species[species_index]->num_namplas; nampla_index++)
			    {
				nam = c_nampla[species_index] + nampla_index;

				if (nam->x != x) continue;
				if (nam->y != y) continue;
				if (nam->z != z) continue;
				if ((nam->status & POPULATED) == 0) continue;

				found = TRUE;
				break;
			    }

			    if (! found)
			    {
				attacking_ships_here = TRUE;
				use_this_ship = TRUE;
				if (c_species[species_index]->tech_level[ML] > attacking_ML)
				    attacking_ML = c_species[species_index]->tech_level[ML];
				break;
			    }
			}
			if (option != engage_option
				&&  option != PLANET_ATTACK) break;
			if (location != engage_location) break;
			attacking_ships_here = TRUE;
			use_this_ship = TRUE;
			break;

		    default:
			fprintf (stderr, "\n\n\tInternal error #1 in fight_par.c - invalid engage option!\n\n");
			exit (-1);
		}
	    }

	  add_ship:
	    if (use_this_ship)
	    {
		/* Add data for this ship to action array. */
		act->fighting_species_index[num_fighting_units] = species_index;
		act->unit_type[num_fighting_units] = SHIP;
		act->fighting_unit[num_fighting_units] = (char *) sh;
		act->original_age_or_PDs[num_fighting_units] = sh->age;
		++num_fighting_units;
	    }
	}

	/* Check which namplas can take part in fight. */
	nam = c_nampla[species_index] - 1;
	for (nampla_index = 0; nampla_index < c_species[species_index]->num_namplas; nampla_index++)
	{
	    ++nam;

	    if (nam->x != x) continue;
	    if (nam->y != y) continue;
	    if (nam->z != z) continue;
	    if (nam->pn != location) continue;
	    if ((nam->status & POPULATED) == 0) continue;
	    if (nam->status & DISBANDED_COLONY) continue;

	    /* This planet has been targeted for some kind of attack. In
		most cases, one species will attack a planet inhabited by
		another species. However, it is also possible for two or
		more species to have colonies on the SAME planet, and for
		one to attack the other. */

	    for (i = 0; i < bat->num_engage_options[species_index]; i++)
	    {
		engage_option = bat->engage_option[species_index][i];
		engage_location = bat->engage_planet[species_index][i];
		if (engage_location != location) continue;

		switch (engage_option)
		{
		    case DEFENSE_IN_PLACE:
		    case DEEP_SPACE_DEFENSE:
		    case PLANET_DEFENSE:
		    case DEEP_SPACE_FIGHT:
			break;

		    case PLANET_ATTACK:
		    case PLANET_BOMBARDMENT:
		    case GERM_WARFARE:
		    case SIEGE:
			if (option != engage_option
				&&  option != PLANET_ATTACK) break;
			if (nam->item_quantity[PD] > 0)
			    attacking_pds_here = TRUE;
			break;

		    default:
			fprintf (stderr, "\n\n\tInternal error #2 in fight_par.c - invalid engage option!\n\n");
			exit (-1);
		}
	    }

	    if (nam->item_quantity[PD] > 0) defending_pds_here = TRUE;

	    /* Add data for this nampla to action array. */
	    act->fighting_species_index[num_fighting_units] = species_index;
	    act->unit_type[num_fighting_units] = NAMPLA;
	    act->fighting_unit[num_fighting_units] = (char *) nam;
	    act->original_age_or_PDs[num_fighting_units] = nam->item_quantity[PD];
	    ++num_fighting_units;
	}
    }

    /* Depending on option, see if the right combination of combatants
	are present. */
    switch (option)
    {
	case DEEP_SPACE_FIGHT:
	    if (!attacking_ships_here  ||  !defending_ships_here) return FALSE;
	    break;

	case PLANET_ATTACK:
	case PLANET_BOMBARDMENT:
	    if (!attacking_ships_here  &&  !attacking_pds_here) return FALSE;
	    break;

	case SIEGE:
	case GERM_WARFARE:
	    if (!attacking_ships_here) return FALSE;
	    break;

	default:
	    fprintf (stderr, "\n\n\tInternal error #3 in fight_par.c - invalid engage option!\n\n");
	    exit (-1);
    }

    /* There is at least one attacker and one defender here. See if they
	are enemies. */
    for (i = 0; i < num_fighting_units; i++)
    {
	sp1 = act->fighting_species_index[i];
	for (j = 0; j < num_fighting_units; j++)
	{
	    sp2 = act->fighting_species_index[j];
	    if (bat->enemy_mine[sp1][sp2]) goto next_step;
	}
    }

    return FALSE;

next_step:

    act->num_units_fighting = num_fighting_units;

    /* Determine number of shots, shield power and weapons power for
	all combatants. */
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++)
    {
	type = act->unit_type[unit_index];
	if (type == SHIP)
	{
	    sh = (struct ship_data *) act->fighting_unit[unit_index];
	    tons = sh->tonnage;
	}
	else
	{
	    nam = (struct nampla_data *) act->fighting_unit[unit_index];
	    tons = nam->item_quantity[PD]/200;
	    if (tons < 1  &&  nam->item_quantity[PD] > 0) tons = 1;
	}

	species_index = act->fighting_species_index[unit_index];

	unit_power = power (tons);
	offensive_power = unit_power;
	defensive_power = unit_power;

	if (type == SHIP)
	{
	    if (sh->class == TR)
	    {
		/* Transports are not designed for combat. */
		offensive_power /= 10;
		defensive_power /= 10;
	    }
	    else if (sh->class != BA)
	    {
		/* Add auxiliary shield generator contribution, if any. */
		tons = 5;
		for (i = SG1; i <= SG9; i++)
		{
		    if (sh->item_quantity[i] > 0)
			defensive_power +=
				(long) sh->item_quantity[i] * power (tons);
		    tons += 5;
		}

		/* Add auxiliary gun unit contribution, if any. */
		tons = 5;
		for (i = GU1; i <= GU9; i++)
		{
		    if (sh->item_quantity[i] > 0)
			offensive_power +=
				(long) sh->item_quantity[i] * power (tons);
		    tons += 5;
		}
	    }

	    /* Adjust for ship aging. */
	    offensive_power -= ((long) sh->age * offensive_power) / 50;
	    defensive_power -= ((long) sh->age * defensive_power) / 50;
	}

	/* Adjust values for tech levels. */
	ml = c_species[species_index]->tech_level[ML];
	ls = c_species[species_index]->tech_level[LS];
	offensive_power += (ml * offensive_power) / 50;
	defensive_power += (ls * defensive_power) / 50;

	/* Adjust values if this species is hijacking anyone. */
	if (bat->hijacker[species_index]  &&  (option == DEEP_SPACE_FIGHT
		||  option == PLANET_ATTACK))
	{
	    offensive_power /= 4;
	    defensive_power /= 4;
	}

	/* Get number of shots per round. */
	n_shots = (offensive_power / 1500) + 1;
	if (ml == 0  ||  offensive_power == 0) n_shots = 0;
	if (n_shots > 5) n_shots = 5;
	act->num_shots[unit_index] = n_shots;
	act->shots_left[unit_index] = n_shots;

	/* Get damage per shot. */
	if (n_shots > 0)
	    act->weapon_damage[unit_index] = (2 * offensive_power) / n_shots;
	else
	    act->weapon_damage[unit_index] = 0;

	/* Do defensive shields. */
	act->shield_strength[unit_index] = defensive_power;
	if (type == SHIP)
	{
	    /* Adjust for results of previous action, if any. "dest_y"
		contains the percentage of shields that remained at end
		of last action. */
	    defensive_power = ((long) sh->dest_y * defensive_power) / 100L;
	}
	act->shield_strength_left[unit_index] = defensive_power;

	/* Set bomb damage to zero in case this is planet bombardment or
		germ warfare. */
	act->bomb_damage[unit_index] = 0;

	/* Set flag for individual unit if species can be surprised. */
	if (bat->can_be_surprised[species_index])
	    act->surprised[unit_index] = TRUE;
	else
	    act->surprised[unit_index] = FALSE;
    }

    return TRUE;	/* There will be a fight here. */
}



int disbanded_ship (species_index, sh)

int			species_index;
struct ship_data	*sh;

{
    int				nampla_index;

    struct nampla_data		*nam;

    nam = c_nampla[species_index] - 1;
    for (nampla_index = 0; nampla_index < c_species[species_index]->num_namplas; nampla_index++)
    {
	++nam;

	if (nam->x != sh->x) continue;
	if (nam->y != sh->y) continue;
	if (nam->z != sh->z) continue;
	if (nam->pn != sh->pn) continue;
	if ((nam->status & DISBANDED_COLONY) == 0) continue;
	if (sh->type != STARBASE  &&  sh->status == IN_ORBIT) continue;

	/* This ship is either on the surface of a disbanded colony or is
		a starbase orbiting a disbanded colony. */
	return TRUE;
    }

    return FALSE;
}
