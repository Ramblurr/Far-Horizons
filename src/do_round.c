
#include "fh.h"
#include "combat.h"


long	power ();

char	x_attacked_y[MAX_SPECIES][MAX_SPECIES];

short	germ_bombs_used[MAX_SPECIES][MAX_SPECIES];


extern int			log_summary, ignore_field_distorters,
				log_to_file;
extern char			field_distorted[MAX_SPECIES];
extern struct species_data	*c_species[MAX_SPECIES];


/* The following routine will return TRUE if a round of combat actually
   occurred. Otherwise, it will return false. */

int do_round (option, round_number, bat, act)

char			option;
int			round_number;
struct battle_data	*bat;
struct action_data	*act;

{
    int		i, j, n, unit_index, combat_occurred, total_shots,
		attacker_index, defender_index, found, chance_to_hit,
		attacker_ml, attacker_gv, defender_ml, target_index[MAX_SHIPS],
		num_targets, header_printed, num_sp, fj_chance, shields_up,
		FDs_were_destroyed, di[3], start_unit, current_species,
		this_is_a_hijacking;

    long	aux_shield_power, units_destroyed, tons, percent_decrease,
		damage_done, damage_to_ship, damage_to_shields, op1, op2,
		original_cost, recycle_value, economic_units;

    char	attacker_name[64], defender_name[64];

    struct species_data		*attacking_species, *defending_species;
    struct ship_data		*sh, *attacking_ship, *defending_ship;
    struct nampla_data		*attacking_nampla, *defending_nampla;


    /* Clear out x_attacked_y and germ_bombs_used arrays.  They will be used
	to log who bombed who, or how many GWs were used. */
    num_sp = bat->num_species_here;
    for (i = 0; i < num_sp; i++)
    {
	for (j = 0; j < num_sp; j++)
	{
	    x_attacked_y[i][j] = FALSE;
	    germ_bombs_used[i][j] = 0;
	}
    }

    /* If a species has ONLY non-combatants left, then let them fight. */
    start_unit = 0;
    total_shots = 0;
    current_species = act->fighting_species_index[0];
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++)
    {
	if (act->fighting_species_index[unit_index] != current_species)
	{
	    if (total_shots == 0)
	    {
		/* Convert all non-combatants, if any, to combatants. */
		for (i = start_unit; i < unit_index; i++)
		{
		    if (act->unit_type[i] == SHIP)
		    {
			sh = (struct ship_data *) act->fighting_unit[i];
			sh->special = 0;
		    }
		}
	    }
	    start_unit = unit_index;
	    total_shots = 0;
	}

	n = act->num_shots[unit_index];
	if (act->surprised[unit_index]) n = 0;
	if (act->unit_type[unit_index] == SHIP)
	{
	    sh = (struct ship_data *) act->fighting_unit[unit_index];
	    if (sh->special == NON_COMBATANT) n = 0;
	}
	total_shots += n;
    }

    /* Determine total number of shots for all species present. */
    total_shots = 0;
    for (unit_index = 0; unit_index < act->num_units_fighting; unit_index++)
    {
	n = act->num_shots[unit_index];
	if (act->surprised[unit_index]) n = 0;
	if (act->unit_type[unit_index] == SHIP)
	{
	    sh = (struct ship_data *) act->fighting_unit[unit_index];
	    if (sh->special == NON_COMBATANT) n = 0;
	}
	act->shots_left[unit_index] = n;
	total_shots += n;
    }

    /* Handle all shots. */
    header_printed = FALSE;
    combat_occurred = FALSE;
    while (total_shots > 0)
    {
        /* check to make sure we arent in infinite loop
         * that usually happens when there are shots remaining
         * but the side with the shots has no more ships left*/
        for( i = 0; i < act->num_units_fighting; ++i)
        {
            attacking_ship = (struct ship_data *) act->fighting_unit[i];
            if (attacking_ship->age > 49 ||
                attacking_ship->status == FORCED_JUMP ||
                attacking_ship->status == JUMPED_IN_COMBAT ||
                (attacking_ship->special == NON_COMBATANT  &&
                    option != GERM_WARFARE) )
            {
                total_shots -= act->shots_left[i];
                act->shots_left[i] = 0;
            }
        }
	/* Determine who fires next. */
	attacker_index = rnd(act->num_units_fighting) - 1;
	if (act->unit_type[attacker_index] == SHIP)
	{
	    attacking_ship = (struct ship_data *) act->fighting_unit[attacker_index];
	    i = act->fighting_species_index[attacker_index];
	    ignore_field_distorters = ! field_distorted[i];
	    sprintf (attacker_name, "%s\0", ship_name (attacking_ship));
	    ignore_field_distorters = FALSE;

	    /* Check if ship can fight. */
	    if (attacking_ship->age > 49) continue;
	    if (attacking_ship->status == FORCED_JUMP) continue;
	    if (attacking_ship->status == JUMPED_IN_COMBAT) continue;
	    if (attacking_ship->special == NON_COMBATANT  &&
		option != GERM_WARFARE) continue;
	}
	else
	{
	    attacking_nampla = (struct nampla_data *) act->fighting_unit[attacker_index];
	    sprintf (attacker_name, "PL %s\0", attacking_nampla->name);

	    /* Check if planet still has defenses. */
	    if (attacking_nampla->item_quantity[PD] == 0) continue;
	}

	/* Make sure attacker is not someone who is being taken by surprise
		this round. */
	if (act->surprised[attacker_index]) continue;

	/* Find an enemy. */
	num_targets = 0;
	i = act->fighting_species_index[attacker_index];
	attacker_ml = c_species[i]->tech_level[ML];
	attacker_gv = c_species[i]->tech_level[GV];
	for (defender_index = 0; defender_index < act->num_units_fighting; defender_index++)
	{
	    j = act->fighting_species_index[defender_index];
	    if (! bat->enemy_mine[i][j]) continue;

	    if (act->unit_type[defender_index] == SHIP)
	    {
		defending_ship =
		    (struct ship_data *) act->fighting_unit[defender_index];

		if (defending_ship->age > 49) continue;  /* Already destroyed. */
		if (defending_ship->status == FORCED_JUMP) continue;
		if (defending_ship->status == JUMPED_IN_COMBAT) continue;
		if (defending_ship->special == NON_COMBATANT) continue;
	    }
	    else
	    {
		defending_nampla =
		    (struct nampla_data *) act->fighting_unit[defender_index];

		if (defending_nampla->item_quantity[PD] == 0
			&&  option == PLANET_ATTACK) continue;
	    }

	    target_index[num_targets] = defender_index;
	    ++num_targets;
	}

	if (num_targets == 0)	/* Attacker has no enemies left. */
	{
	    total_shots -= act->shots_left[attacker_index];
	    act->shots_left[attacker_index] = 0;
	    continue;
	}

	/* Randomly choose a target. Choose the toughest of four. */
	defender_index = target_index[rnd(num_targets) - 1];
	op1 = (long) act->num_shots[defender_index]
		* act->weapon_damage[defender_index];
	di[0] = target_index[rnd(num_targets) - 1];
	di[1] = target_index[rnd(num_targets) - 1];
	di[2] = target_index[rnd(num_targets) - 1];
	for (i = 0; i < 3; i++)
	{
	    op2 = (long) act->num_shots[di[i]] * act->weapon_damage[di[i]];
	    if (op2 > op1)
	    {
		defender_index = di[i];
		op1 = op2;
	    }
	}

	j = act->fighting_species_index[defender_index];
	defender_ml = c_species[j]->tech_level[ML];

	if (act->unit_type[defender_index] == SHIP)
	{
	    defending_ship =
		(struct ship_data *) act->fighting_unit[defender_index];
	    ignore_field_distorters = ! field_distorted[j];
	    sprintf (defender_name, "%s\0", ship_name (defending_ship));
	    ignore_field_distorters = FALSE;
	}
	else
	{
	    defending_nampla =
		(struct nampla_data *) act->fighting_unit[defender_index];
	    sprintf (defender_name, "PL %s\0", defending_nampla->name);
	}

	/* Print round number. */
	if (! header_printed)
	{
	    log_string ("      Now doing round ");
	    log_int (round_number);
	    log_string (":\n");
	    header_printed = TRUE;
	}
		int attackerGvMl = attacker_gv + attacker_ml;
		if (attackerGvMl <= 0) {
			attackerGvMl = 1;
		}
	/* Check if attacker has any forced jump units. The attacker will
		place more emphasis on the use of these devices if he
		emphasizes gravitics technology over military technology. */
		fj_chance = 50 * attacker_gv / attackerGvMl;
	if (rnd(100) < fj_chance
		&&  act->unit_type[attacker_index] == SHIP
		&&  act->unit_type[defender_index] == SHIP)
	{
	    if (forced_jump_units_used (attacker_index, defender_index,
					&total_shots, bat, act))
	    {
		combat_occurred = TRUE;
		continue;
	    }
	}

	if (act->shots_left[attacker_index] == 0) continue;

	/* Since transports generally avoid combat, there is only a 10%
	   chance that they will be targetted, unless they are being
	   explicitly targetted. */
	i = act->fighting_species_index[attacker_index];
	j = act->fighting_species_index[defender_index];
	if (act->unit_type[defender_index] == SHIP
		&&  defending_ship->class == TR
		&&  bat->special_target[i] != TARGET_TRANSPORTS
		&&  rnd(10) != 5) continue;

	/* If a special target has been specified, then there is a 75%
	   chance that it will be attacked if it is available. */
	if (bat->special_target[i]  &&  rnd(100) < 76)
	{
	    if (bat->special_target[i] == TARGET_PDS)
	    {
		if (act->unit_type[defender_index] != SHIP)
			goto fire;
		else
			continue;
	    }

	    if (act->unit_type[defender_index] != SHIP) continue;

	    if (bat->special_target[i] == TARGET_STARBASES
		&&  defending_ship->class != BA) continue;
	    if (bat->special_target[i] == TARGET_TRANSPORTS
		&&  defending_ship->class != TR) continue;
	    if (bat->special_target[i] == TARGET_WARSHIPS)
	    {
		if (defending_ship->class == TR) continue;
		if (defending_ship->class == BA) continue;
	    }
	}

fire:
	/* Update counts. */
	--act->shots_left[attacker_index];
	--total_shots;

	/* Since transports generally avoid combat, there is only a 10%
	   chance that they will attack. */
	if (act->unit_type[attacker_index] == SHIP
		&&  attacking_ship->class == TR
		&&  option != GERM_WARFARE
		&&  rnd(10) != 5) continue;

	/* Fire! */
	combat_occurred = TRUE;
	log_string ("        ");  log_string (attacker_name);
	log_string (" fires on ");  log_string (defender_name);
	if (act->unit_type[defender_index] == NAMPLA)
	    log_string (" defenses");

		int combinedMl = attacker_ml + defender_ml;
		if (combinedMl <= 0) {
			combinedMl = 1;
		}
	/* Get hit probability. The basic chance to hit is 1.5 times
	   attackers ML over the sum of attacker's and defender's ML.
	   Double this value if defender is surprised. */
		chance_to_hit = (150 * attacker_ml) / combinedMl;
	if (act->surprised[defender_index])
	{
	    chance_to_hit *= 2;
	    shields_up = FALSE;
	}
	else
	    shields_up = TRUE;

	/* If defending ship is field-distorted, chance-to-hit is
		reduced by 25%. */
	j = act->fighting_species_index[defender_index];
	if (act->unit_type[defender_index] == SHIP
	    &&  field_distorted[j]
	    &&  defending_ship->item_quantity[FD] == defending_ship->tonnage)
		chance_to_hit = (3 * chance_to_hit) / 4;

	if (chance_to_hit > 98) chance_to_hit = 98;
	if (chance_to_hit < 2) chance_to_hit = 2;

	/* Adjust for age. */
	if (act->unit_type[attacker_index] == SHIP)
	    chance_to_hit  -=
		(2 * attacking_ship->age * chance_to_hit)/100;

	/* Calculate damage that shot will do if it hits. */
	damage_done = act->weapon_damage[attacker_index];
	damage_done += ((26-rnd(51)) * damage_done) / 100;

	/* Take care of attempted annihilation and sieges. */
	if (option == PLANET_BOMBARDMENT  ||  option == GERM_WARFARE
		|| option == SIEGE)
	{
	    /* Indicate the action that was attempted against this nampla. */
	    if (option == SIEGE)
		act->unit_type[defender_index] = BESIEGED_NAMPLA;
	    else
		act->unit_type[defender_index] = GENOCIDE_NAMPLA;

	    /* Indicate who attacked who. */
	    i = act->fighting_species_index[attacker_index];
	    j = act->fighting_species_index[defender_index];
	    x_attacked_y[i][j] = TRUE;

	    /* Update bombardment damage. */
	    if (option == PLANET_BOMBARDMENT)
		act->bomb_damage[defender_index] += damage_done;
	    else if (option == GERM_WARFARE)
	    {
		if (act->unit_type[attacker_index] == SHIP)
		{
		    germ_bombs_used[i][j] += attacking_ship->item_quantity[GW];
		    attacking_ship->item_quantity[GW] = 0;
		}
		else
		{
		    germ_bombs_used[i][j] += attacking_nampla->item_quantity[GW];
		    attacking_nampla->item_quantity[GW] = 0;
		}
	    }

	    continue;
	}

	/* Check if shot hit. */
	if (rnd(100) <= chance_to_hit)
	    log_string (" and hits!\n");
	else
	{
	    log_string (" and misses!\n");
	    continue;
	}

	/* Subtract damage from defender's shields, if they're up. */
	damage_to_ship = 0;
	if (shields_up)
	{
	    if (act->unit_type[defender_index] == SHIP)
	    {
		damage_to_shields =
			((long) defending_ship->dest_y * damage_done) / 100;
		damage_to_ship = damage_done - damage_to_shields;
		act->shield_strength_left[defender_index] -= damage_to_shields;

		/* Calculate percentage of shields left. */
				if (act->shield_strength_left[defender_index] > 0) {
					long int defenderShieldStrength =
							act->shield_strength[defender_index];
					if (defenderShieldStrength <= 0) {
						defenderShieldStrength = 1;
					}
		    defending_ship->dest_y =
			(100L * act->shield_strength_left[defender_index])
							/ defenderShieldStrength;
				} else
		    defending_ship->dest_y = 0;
	    }
	    else  /* Planetary defenses. */
		act->shield_strength_left[defender_index] -= damage_done;
	}

	/* See if it got through shields. */
	units_destroyed = 0;
	percent_decrease = 0;
	if (! shields_up  ||  act->shield_strength_left[defender_index] < 0
		||  damage_to_ship > 0)
	{
	    /* Get net damage to ship or PDs. */
	    if (shields_up)
	    {
		if (act->unit_type[defender_index] == SHIP)
		{
		    /* Total damage to ship is direct damage plus damage
			that shields could not absorb. */
		    damage_done = damage_to_ship;
		    if (act->shield_strength_left[defender_index] < 0)
			damage_done -=
			    act->shield_strength_left[defender_index];
		}
		else
		    damage_done = -act->shield_strength_left[defender_index];
	    }

			long defenderShieldStrength = act->shield_strength[defender_index];
			if (defenderShieldStrength <= 0) {
				defenderShieldStrength = 1;
			}

			percent_decrease = (50L * damage_done) / defenderShieldStrength;
			
	    percent_decrease += ((rnd(51) - 26) * percent_decrease) / 100;
	    if (percent_decrease > 100) percent_decrease = 100;

	    if (act->unit_type[defender_index] == SHIP)
	    {
		defending_ship->age += percent_decrease/2;
		units_destroyed = (defending_ship->age > 49);
	    }
	    else
	    {
		units_destroyed = (percent_decrease
			* act->original_age_or_PDs[defender_index]) / 100L;
		if (units_destroyed > defending_nampla->item_quantity[PD])
		    units_destroyed = defending_nampla->item_quantity[PD];
		if (units_destroyed < 1) units_destroyed = 1;
		defending_nampla->item_quantity[PD] -= units_destroyed;
	    }

	    if (act->shield_strength_left[defender_index] < 0)
		act->shield_strength_left[defender_index] = 0;
	}

	/* See if this is a hijacking. */
	i = act->fighting_species_index[attacker_index];
	j = act->fighting_species_index[defender_index];
	if (bat->enemy_mine[i][j] == 2  &&  (option == DEEP_SPACE_FIGHT
		|| option == PLANET_ATTACK))
	    this_is_a_hijacking = TRUE;
	else
	    this_is_a_hijacking = FALSE;

	attacking_species = c_species[i];
	defending_species = c_species[j];

	/* Report if anything was destroyed. */
	FDs_were_destroyed = FALSE;
	if (units_destroyed)
	{
	    if (act->unit_type[defender_index] == SHIP)
	    {
		log_summary = TRUE;
		log_string ("        ");
		log_string (defender_name);
		if (this_is_a_hijacking)
		{
		    log_string (" was successfully hijacked and will generate ");

		    if (defending_ship->class == TR  ||  defending_ship->type == STARBASE)
			original_cost = ship_cost[defending_ship->class] * defending_ship->tonnage;
		    else
			original_cost = ship_cost[defending_ship->class];

		    if (defending_ship->type == SUB_LIGHT)
			original_cost = (3 * original_cost) / 4;

		    if (defending_ship->status == UNDER_CONSTRUCTION)
			recycle_value =
			    (original_cost - (long) defending_ship->remaining_cost) / 2;
		    else
			recycle_value =
			    (3 * original_cost * (60 - act->original_age_or_PDs[defender_index])) / 200;

		    economic_units = recycle_value;

		    for (i = 0; i < MAX_ITEMS; i++)
		    {
			j = defending_ship->item_quantity[i];
			if (j > 0)
			{
							if (i == TP) {
								long int techLevel_2x =
										2L
												* (long) defending_species->tech_level[BI];
								if (techLevel_2x <= 0) {
									techLevel_2x = 1;
								}
								recycle_value = (j * item_cost[i])
										/ techLevel_2x;
							}
			    else if (i == RM)
				recycle_value = j / 5;
			    else
				recycle_value = (j * item_cost[i]) / 2;

			    economic_units += recycle_value;
			}
		    }

		    attacking_species->econ_units += economic_units;

		    log_long (economic_units);
		    log_string (" economic units for the hijackers.\n");
		}
		else
		    log_string (" was destroyed.\n");

		for (i = 0; i < MAX_ITEMS; i++)
		{
		    if (defending_ship->item_quantity[i] > 0)
		    {
			/* If this is a hijacking of a field-distorted ship,
			    we want the true name of the hijacked species to
			    be announced, but we don't want any cargo to be
			    destroyed. */
			if (i == FD) FDs_were_destroyed = TRUE;
			if (! this_is_a_hijacking)
			    defending_ship->item_quantity[FD] = 0;
		    }
		}
		log_to_file = FALSE;
		if (this_is_a_hijacking)
		    log_string ("          The hijacker was ");
		else
		    log_string ("          The killing blow was delivered by ");
		log_string (attacker_name);
		log_string (".\n");
		log_to_file = TRUE;
		log_summary = FALSE;

		total_shots -= act->shots_left[defender_index];
		act->shots_left[defender_index] = 0;
		act->num_shots[defender_index] = 0;
	    }
	    else
	    {
		log_summary = TRUE;
		log_string ("        ");  log_int (units_destroyed);
		if (units_destroyed > 1)
		    log_string (" PDs on PL ");
		else
		    log_string (" PD on PL ");
		log_string (defending_nampla->name);
		if (units_destroyed > 1)
		    log_string (" were destroyed by ");
		else
		    log_string (" was destroyed by ");

		log_string (attacker_name);
		log_string (".\n");

		if (defending_nampla->item_quantity[PD] == 0)
		{
		    total_shots -= act->shots_left[defender_index];
		    act->shots_left[defender_index] = 0;
		    act->num_shots[defender_index] = 0;
		    log_string ("        All planetary defenses have been destroyed on ");
		    log_string (defender_name);
		    log_string ("!\n");
		}
		log_summary = FALSE;
	    }
	}
	else if (percent_decrease > 0  &&  ! this_is_a_hijacking
		&&  act->unit_type[defender_index] == SHIP)
	{
	    /* See if anything carried by the ship was also destroyed. */
	    for (i = 0; i < MAX_ITEMS; i++)
	    {
		j = defending_ship->item_quantity[i];
		if (j > 0)
		{
		    j = (percent_decrease * j) / 100;
		    if (j > 0)
		    {
			defending_ship->item_quantity[i] -= j;
			if (i == FD) FDs_were_destroyed = TRUE;
		    }
		}
	    }
	}

	j = act->fighting_species_index[defender_index];
	if (FDs_were_destroyed  &&  field_distorted[j]
		&&  defending_ship->dest_x == 0)
	{
	    /* Reveal the true name of the ship and the owning species. */
	    log_summary = TRUE;
	    if (this_is_a_hijacking)
		log_string ("        Hijacking of ");
	    else
		log_string ("        Damage to ");
	    log_string (defender_name);
	    log_string (" caused collapse of distortion field. Real name of ship is ");
	    log_string (ship_name (defending_ship));
	    log_string (" owned by SP ");
	    log_string (defending_species->name);
	    log_string (".\n");
	    log_summary = FALSE;
	    defending_ship->dest_x = 127;	/* Ship is now exposed. */
	}
    }

    /* No more surprises. */
    for (i = 0; i < act->num_units_fighting; i++)
	act->surprised[i] = FALSE;

    return combat_occurred;
}
