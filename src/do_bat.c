#include "fh.h"
#include "combat.h"


int		first_battle = TRUE;
int		attacking_ML, defending_ML, deep_space_defense,
		ambush_took_place;
char		field_distorted[MAX_SPECIES], append_log[MAX_SPECIES],
		make_enemy[MAX_SPECIES][MAX_SPECIES];


extern int			attacker_here, defender_here, logging_disabled,
				strike_phase, prompt_gm;
extern int			log_summary, num_combat_options;
extern int			truncate_name, num_transactions;
extern int			ignore_field_distorters;
extern char			combat_option[1000], combat_location[1000];
extern char			x_attacked_y[MAX_SPECIES][MAX_SPECIES];
extern FILE			*log_file, *summary_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*c_species[MAX_SPECIES];
extern struct nampla_data	*nampla_base, *c_nampla[MAX_SPECIES];
extern struct ship_data		*ship_base, *c_ship[MAX_SPECIES];



do_battle (bat)

struct battle_data	*bat;

{
    int		i, j, k, species_index, species_number, num_sp, save,
		max_rounds, round_number, battle_here, fight_here,
		unit_index, option_index, current_species, temp_status,
		temp_pn, num_namplas, array_index, bit_number, first_action,
		traitor_number, betrayed_number, betrayal, need_comma,
		TRUE_value, do_withdraw_check_first;

    short	identifiable_units[MAX_SPECIES],
		unidentifiable_units[MAX_SPECIES];

    long	n, bit_mask;

    char	x, y, z, where, option, filename[32], enemy,
		enemy_num[MAX_SPECIES], log_line[256];

    FILE	*combat_log, *species_log;

    struct action_data	act;
    struct nampla_data	*namp, *attacked_nampla;
    struct ship_data	*sh;


    ambush_took_place = FALSE;

    /* Open log file for writing. */
    log_file = fopen ("combat.log", "w");
    if (log_file == NULL)
    {
	fprintf (stderr, "\n\tCannot open 'combat.log' for writing!\n\n");
	exit (-1);
    }

    /* Open summary file for writing. */
    summary_file = fopen ("summary.log", "w");
    if (summary_file == NULL)
    {
	fprintf (stderr, "\n\tCannot open 'summary.log' for writing!\n\n");
	exit (-1);
    }
    log_summary = TRUE;

    /* Get data for all species present at this battle. */
    num_sp = bat->num_species_here;
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	species_number = bat->spec_num[species_index];
	c_species[species_index] = &spec_data[species_number - 1];
	c_nampla[species_index] = namp_data[species_number - 1];
	c_ship[species_index] = ship_data[species_number - 1];
	if (data_in_memory[species_number - 1])
	    data_modified[species_number - 1] = TRUE;
	else
	{
	    fprintf (stderr, "\n\tData for species #%d is needed but is not available!\n\n",
		species_number);
	    exit (-1);
	}

	/* Determine number of identifiable and unidentifiable units present. */
	identifiable_units[species_index] = 0;
	unidentifiable_units[species_index] = 0;

	namp = c_nampla[species_index] - 1;
	for (i = 0; i < c_species[species_index]->num_namplas; i++)
	{
	    ++namp;

	    if (namp->x != bat->x) continue;
	    if (namp->y != bat->y) continue;
	    if (namp->z != bat->z) continue;

	    if (namp->status & POPULATED) ++identifiable_units[species_index];
	}

	sh = c_ship[species_index] - 1;
	for (i = 0; i < c_species[species_index]->num_ships; i++)
	{
	    ++sh;

	    if (sh->x != bat->x) continue;
	    if (sh->y != bat->y) continue;
	    if (sh->z != bat->z) continue;
	    if (sh->status == UNDER_CONSTRUCTION) continue;
	    if (sh->status == JUMPED_IN_COMBAT) continue;
	    if (sh->status == FORCED_JUMP) continue;

	    sh->dest_x = 0;	/* Not yet exposed. */
	    sh->dest_y = 100;	/* Shields at 100%. */

	    if (sh->item_quantity[FD] == sh->tonnage)
		++unidentifiable_units[species_index];
	    else
		++identifiable_units[species_index];
	}

	if (identifiable_units[species_index] > 0
	    ||  unidentifiable_units[species_index] == 0)
		field_distorted[species_index] = FALSE;
	else
		field_distorted[species_index] = TRUE;
    }

    /* Start log of what's happening. */
    if (strike_phase)
	log_string ("\nStrike log:\n");
    else
	log_string ("\nCombat log:\n");
    first_battle = FALSE;

    log_string ("\n  Battle orders were received for sector ");  log_int (bat->x);
    log_string (", ");  log_int (bat->y);  log_string (", ");  log_int (bat->z);
    log_string (". The following species are present:\n\n");

    /* Convert enemy_mine array from a list of species numbers to an array
	of TRUE/FALSE values whose indices are:

			[species_index1][species_index2]

	such that the value will be TRUE if #1 mentioned #2 in an ATTACK
	or HIJACK command.  The actual TRUE value will be 1 for ATTACK or
	2 for HIJACK. */

    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	/* Make copy of list of enemies. */
	for (i = 0; i < MAX_SPECIES; i++)
	{
	    enemy_num[i] = bat->enemy_mine[species_index][i];
	    bat->enemy_mine[species_index][i] = FALSE;
	}

	for (i = 0; i < MAX_SPECIES; i++)
	{
	    enemy = enemy_num[i];
	    if (enemy == 0) break;	/* No more enemies in list. */

	    if (enemy < 0)
	    {
		enemy = -enemy;
		TRUE_value = 2;		/* This is a hijacking. */
	    }
	    else
		TRUE_value = 1;		/* This is a normal attack. */

	    /* Convert absolute species numbers to species indices that
		have been assigned in the current battle. */
	    for (j = 0; j < num_sp; j++)
	    {
		if (enemy == bat->spec_num[j])
		    bat->enemy_mine[species_index][j] = TRUE_value;
	    }
	}
    }

    /* For each species that has been mentioned in an attack order, check
	if it can be surprised. A species can only be surprised if it has
	not given a BATTLE order and if it is being attacked ONLY by one
	or more ALLIES. */
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	j = bat->spec_num[species_index] - 1;
	array_index = j / 32;
	bit_number = j % 32;
	bit_mask = 1 << bit_number;

	for (i = 0; i < num_sp; i++)
	{
	    if (i == species_index) continue;

	    if (! bat->enemy_mine[species_index][i]) continue;

	    if (field_distorted[species_index])
	    {
		/* Attacker is field-distorted. Surprise not possible. */
		bat->can_be_surprised[i] = FALSE;
		continue;
	    }

	    if ((c_species[i]->ally[array_index] & bit_mask))
		betrayal = TRUE;
	    else
		betrayal = FALSE;

	    if (betrayal)
	    {
		/* Someone is being attacked by an ALLY. */
		traitor_number = bat->spec_num[species_index];
		betrayed_number = bat->spec_num[i];
		make_enemy[betrayed_number-1][traitor_number-1] = betrayed_number;
		auto_enemy (traitor_number, betrayed_number);
	    }

	    if (! bat->can_be_surprised[i]) continue;

	    if (! betrayal)	/* At least one attacker is not an ally. */
		bat->can_be_surprised[i] = FALSE;
	}
    }

    /* For each species that has been mentioned in an attack order, see if
	there are other species present that have declared it as an ALLY.
	If so, have the attacker attack the other species and vice-versa. */
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	for (i = 0; i < num_sp; i++)
	{
	    if (i == species_index) continue;

	    if (! bat->enemy_mine[species_index][i]) continue;

	    j = bat->spec_num[i] - 1;
	    array_index = j / 32;
	    bit_number = j % 32;
	    bit_mask = 1 << bit_number;

	    for (k = 0; k < num_sp; k++)
	    {
		if (k == species_index) continue;
		if (k == i) continue;

		if (c_species[k]->ally[array_index] & bit_mask)
		{
		    /* Make sure it's not already set (it may already be set
			for HIJACK and we don't want to accidentally change
			it to ATTACK). */
		    if (! bat->enemy_mine[species_index][k])
			bat->enemy_mine[species_index][k] = TRUE;
		    if (! bat->enemy_mine[k][species_index])
			bat->enemy_mine[k][species_index] = TRUE;
		}
	    }
	}
    }

    /* If a species did not give a battle order and is not the target of an
	attack, set can_be_surprised flag to a special value. */
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	if (! bat->can_be_surprised[species_index]) continue;

	bat->can_be_surprised[species_index] = 55;

	for (i = 0; i < num_sp; i++)
	{
	    if (i == species_index) continue;

	    if (! bat->enemy_mine[i][species_index]) continue;

	    bat->can_be_surprised[species_index] = TRUE;

	    break;
	}
    }

    /* List combatants. */
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	species_number = bat->spec_num[species_index];

	log_string ("    SP ");
	if (field_distorted[species_index])
	    log_int (distorted (species_number));
	else
	    log_string (c_species[species_index]->name);
	if (bat->can_be_surprised[species_index])
	    log_string (" does not appear to be ready for combat.\n");
	else
	    log_string (" is mobilized and ready for combat.\n");
    }

    /* Check if a declared enemy is being ambushed. */
    for (i = 0; i < num_sp; i++)
    {
	namp = c_nampla[i] - 1;
	num_namplas = c_species[i]->num_namplas;
	bat->ambush_amount[i] = 0;
	for (j = 0; j < num_namplas; j++)
	{
	    ++namp;

	    if (namp->x != bat->x) continue;
	    if (namp->y != bat->y) continue;
	    if (namp->z != bat->z) continue;

	    bat->ambush_amount[i] += namp->use_on_ambush;
	}

	if (bat->ambush_amount[i] == 0) continue;

	for (j = 0; j < num_sp; j++)
	    if (bat->enemy_mine[i][j])
		do_ambush (i, bat);
    }

    /* For all species that specified enemies, make the feeling mutual. */
    for (i = 0; i < num_sp; i++)
      for (j = 0; j < num_sp; j++)
	if (bat->enemy_mine[i][j])
	{
	    /* Make sure it's not already set (it may already be set for
		HIJACK and we don't want to accidentally change it to
		ATTACK). */
	    if (! bat->enemy_mine[j][i]) bat->enemy_mine[j][i] = TRUE;
	}

    /* Create a sequential list of combat options. First check if a
	deep space defense has been ordered. If so, then make sure that
	first option is DEEP_SPACE_FIGHT. */
    num_combat_options = 0;
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	for (i = 0; i < bat->num_engage_options[species_index]; i++)
	{
	    option = bat->engage_option[species_index][i];
	    if (option == DEEP_SPACE_DEFENSE)
	    {
		consolidate_option (DEEP_SPACE_FIGHT, 0);
		goto consolidate;
	    }
	}
    }

  consolidate:
    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	for (i = 0; i < bat->num_engage_options[species_index]; i++)
	{
	    option = bat->engage_option[species_index][i];
	    where = bat->engage_planet[species_index][i];
	    consolidate_option (option, where);
	}
    }

    /* If ships are given unconditional withdraw orders, they will always have
	time to escape if fighting occurs first in a different part of the
	sector. The flag "do_withdraw_check_first" will be set only after the
	first round of combat. */
    do_withdraw_check_first = FALSE;

    /* Handle each combat option. */
    battle_here = FALSE;
    first_action = TRUE;
    for (option_index = 0; option_index < num_combat_options; option_index++)
    {
	option = combat_option[option_index];
	where = combat_location[option_index];

	/* Fill action arrays with data about ships taking part in current
	   action. */
	fight_here = fighting_params (option, where, bat, &act);

	/* Check if a fight will take place here. */
	if (! fight_here) continue;

	/* See if anyone is taken by surprise. */
	if (! battle_here)
	{
	    /* Combat is just starting. */
	    for (species_index = 0; species_index < num_sp; ++species_index)
	    {
		species_number = bat->spec_num[species_index];

		if (bat->can_be_surprised[species_index] == 55) continue;

		if (bat->can_be_surprised[species_index])
		{
		    log_string ("\n    SP ");
		    if (field_distorted[species_index])
			log_int (distorted (species_number));
		    else
			log_string (c_species[species_index]->name);
		    log_string (" is taken by surprise!\n");
		}
	    }
	}

	battle_here = TRUE;

	/* Clear out can_be_surprised array. */
	for (i = 0; i < MAX_SPECIES; i++)
	    bat->can_be_surprised[i] = FALSE;

	/* Determine maximum number of rounds. */
	max_rounds = 10000;	/* Something ridiculously large. */
	if (option == DEEP_SPACE_FIGHT  &&  attacking_ML > 0
					&&  defending_ML > 0
					&&  deep_space_defense)
	{
	    /* This is the initial deep space fight and the defender wants the
		fight to remain in deep space for as long as possible. */
	    if (defending_ML > attacking_ML)
		max_rounds = defending_ML - attacking_ML;
	    else
		max_rounds = 1;
	}
	else if (option == PLANET_BOMBARDMENT)
	{
	    /* To determine the effectiveness of the bombardment, we will
		simulate ten rounds of combat and add up the damage. */
	    max_rounds = 10;
	}
	else if (option == GERM_WARFARE  ||  option == SIEGE)
	{
	    /* We just need to see who is attacking whom and get the number
		of germ warfare bombs being used. */
	    max_rounds = 1;
	}

	/* Log start of action. */
	if (where == 0)
	    log_string ("\n    The battle begins in deep space, outside the range of planetary defenses...\n");
	else if (option == PLANET_ATTACK)
	{
	    log_string ("\n    The battle ");
	    if (first_action)
		log_string ("begins");
	    else
		log_string ("moves");
	    log_string (" within range of planet #");
	    log_int (where);
	    log_string ("...\n");
	}
	else if (option == PLANET_BOMBARDMENT)
	{
	    log_string ("\n    Bombardment of planet #");
	    log_int (where);
	    log_string (" begins...\n");
	}
	else if (option == GERM_WARFARE)
	{
	    log_string ("\n    Germ warfare commences against planet #");
	    log_int (where);
	    log_string ("...\n");
	}
	else if (option == SIEGE)
	{
	    log_string ("\n    Siege of planet #");
	    log_int (where);
	    log_string (" is now in effect...\n\n");
	    goto do_combat;
	}

	/* List combatants. */
	truncate_name = FALSE;
	log_string ("\n      Units present:");
	current_species = -1;
	for (unit_index = 0; unit_index < act.num_units_fighting; unit_index++)
	{
	    if (act.fighting_species_index[unit_index] != current_species)
	    {
		/* Display species name. */
		i = act.fighting_species_index[unit_index];
		log_string ("\n        SP ");
		species_number = bat->spec_num[i];
		if (field_distorted[i])
		    log_int (distorted (species_number));
		else
		    log_string (c_species[i]->name);
		log_string (": ");
		current_species = i;
		need_comma = FALSE;
	    }

	    if (act.unit_type[unit_index] == SHIP)
	    {
		sh = (struct ship_data *) act.fighting_unit[unit_index];
		temp_status = sh->status;
		temp_pn = sh->pn;
		if (option == DEEP_SPACE_FIGHT)
		{
		    sh->status = IN_DEEP_SPACE;
		    sh->pn = 0;
		}
		else
		{
		    sh->status = IN_ORBIT;
		    sh->pn = where;
		}
		ignore_field_distorters = ! field_distorted[current_species];
		if (sh->special != NON_COMBATANT)
		{
		    if (need_comma) log_string (", ");
		    log_string (ship_name (sh));
		    need_comma = TRUE;
		}
		ignore_field_distorters = FALSE;
		sh->status = temp_status;
		sh->pn = temp_pn;
	    }
	    else
	    {
		namp = (struct nampla_data *) act.fighting_unit[unit_index];
		if (need_comma) log_string (", ");
		log_string ("PL ");
		log_string (namp->name);
		need_comma = TRUE;
	    }
	}
	log_string ("\n\n");

do_combat:

	/* Long names are not necessary for the rest of the action. */
	truncate_name = TRUE;

	/* Do combat rounds. Stop if maximum count is reached, or if combat
	    does not occur when do_round() is called. */

	round_number = 1;

	log_summary = FALSE;	/* do_round() and the routines that it calls
					will set this for important stuff. */

	if (option == PLANET_BOMBARDMENT  ||  option == GERM_WARFARE
		||  option == SIEGE)
	    logging_disabled = TRUE; /* Disable logging during simulation. */

	while (round_number <= max_rounds)
	{
	    if (do_withdraw_check_first) withdrawal_check (bat, &act);

	    if (! do_round (option, round_number, bat, &act)) break;

	    if (! do_withdraw_check_first) withdrawal_check (bat, &act);

	    do_withdraw_check_first = TRUE;

	    regenerate_shields (&act);

	    ++round_number;
	}

	log_summary = TRUE;
	logging_disabled = FALSE;

	if (round_number == 1)
	{
	    log_string ("      ...But it seems that the attackers had nothing to attack!\n");
	    continue;
	}

	if (option == PLANET_BOMBARDMENT  ||  option == GERM_WARFARE)
	{
	    for (unit_index = 0; unit_index < act.num_units_fighting; unit_index++)
	    {
		if (act.unit_type[unit_index] == GENOCIDE_NAMPLA)
		{
		    attacked_nampla = (struct nampla_data *)
			act.fighting_unit[unit_index];
		    j = act.fighting_species_index[unit_index];
		    for (i = 0; i < num_sp; i++)
		    {
			if (x_attacked_y[i][j])
			{
			    species_number = bat->spec_num[i];
			    log_string ("      SP ");
			    if (field_distorted[i])
				log_int (distorted (species_number));
			    else
				log_string (c_species[i]->name);
			    log_string (" bombards SP ");
			    log_string (c_species[j]->name);
			    log_string (" on PL ");
			    log_string (attacked_nampla->name);
			    log_string (".\n");

			    if (option == GERM_WARFARE)
				do_germ_warfare (i, j, unit_index, bat, &act);
			}
		    }

		    /* Determine results of bombardment. */
		    if (option == PLANET_BOMBARDMENT)
			do_bombardment (unit_index, &act);
		}
	    }
	}
	else if (option == SIEGE)
	    do_siege (bat, &act);

	truncate_name = FALSE;

	first_action = FALSE;
    }

    if (! battle_here)
    {
	if (bat->num_species_here == 1)
	    log_string ("    But there was no one to fight with!\n");
	else if (! ambush_took_place)
	    log_string ("    But no one was willing to throw the first punch!\n");
    }

    /* Close combat log and append it to the log files of all species
	involved in this battle. */
    if (prompt_gm)
	printf ("\n  End of battle in sector %d, %d, %d.\n", bat->x,
		bat->y, bat->z);
    fprintf (log_file, "\n  End of battle in sector %d, %d, %d.\n", bat->x,
	bat->y, bat->z);
    fprintf (summary_file, "\n  End of battle in sector %d, %d, %d.\n",
	bat->x, bat->y, bat->z);
    fclose (log_file);
    fclose (summary_file);

    for (species_index = 0; species_index < num_sp; ++species_index)
    {
	species_number = bat->spec_num[species_index];

	/* Open combat log file for reading. */
	if (bat->summary_only[species_index])
	    combat_log = fopen ("summary.log", "r");
	else
	    combat_log = fopen ("combat.log", "r");

	if (combat_log == NULL)
	{
	    fprintf (stderr, "\n\tCannot open combat log for reading!\n\n");
	    exit (-1);
	}

	/* Open a temporary species log file for appending. */
	sprintf (filename, "sp%02d.temp.log\0", species_number);
	species_log = fopen (filename, "a");
	if (species_log == NULL)
	{
	    fprintf (stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
	    exit (-1);
	}

	/* Copy combat log to temporary species log. */
	while (fgets(log_line, 256, combat_log) != NULL)
		fputs (log_line, species_log);

	fclose (species_log);
	fclose (combat_log);

	append_log[species_number - 1] = TRUE;

	/* Get rid of ships that were destroyed. */
	if (! data_modified[species_number - 1]) continue;
	sh = c_ship[species_index] - 1;
	for (i = 0; i < c_species[species_index]->num_ships; i++)
	{
	    ++sh;

	    if (sh->age < 50) continue;
	    if (sh->pn == 99) continue;
	    if (sh->x != bat->x) continue;
	    if (sh->y != bat->y) continue;
	    if (sh->z != bat->z) continue;
	    if (sh->status == UNDER_CONSTRUCTION) continue;

	    delete_ship (sh);
	}
    }
}



do_ambush (ambushing_species_index, bat)

int			ambushing_species_index;
struct battle_data	*bat;

{
    int		i, j, n, num_sp, ambushed_species_index, num_ships,
		age_increment, species_number, old_truncate_name;

    long	friendly_tonnage, enemy_tonnage;

    struct ship_data		*sh;



    /* Get total ambushing tonnage. */
    friendly_tonnage = 0;
    num_ships = c_species[ambushing_species_index]->num_ships;
    sh = c_ship[ambushing_species_index] - 1;
    for (i = 0; i < num_ships; i++)
    {
	++sh;

	if (sh->pn == 99) continue;
	if (sh->x != bat->x) continue;
	if (sh->y != bat->y) continue;
	if (sh->z != bat->z) continue;
	if (sh->class != TR  &&  sh->class != BA)
	    friendly_tonnage += sh->tonnage;
    }

    /* Determine which species are being ambushed and get total enemy
	tonnage. */
    num_sp = bat->num_species_here;
    enemy_tonnage = 0;
    for (ambushed_species_index = 0; ambushed_species_index < num_sp; ++ambushed_species_index)
    {
	if (! bat->enemy_mine[ambushing_species_index][ambushed_species_index])
	     continue;

	/* This species is being ambushed.  Get total effective tonnage. */
	num_ships = c_species[ambushed_species_index]->num_ships;
	sh = c_ship[ambushed_species_index] - 1;
	for (i = 0; i < num_ships; i++)
	{
	    ++sh;

	    if (sh->pn == 99) continue;
	    if (sh->x != bat->x) continue;
	    if (sh->y != bat->y) continue;
	    if (sh->z != bat->z) continue;
	    if (sh->class == TR)
		enemy_tonnage += sh->tonnage;
	    else
		enemy_tonnage += 10 * sh->tonnage;
	}
    }

    /* Determine the amount of aging that will be added to each ambushed
	ship. */
    if (enemy_tonnage == 0) return;
    age_increment = (10L * bat->ambush_amount[ambushing_species_index])
	/ enemy_tonnage;
    age_increment = (friendly_tonnage * age_increment) / enemy_tonnage;

    ambush_took_place = TRUE;

    if (age_increment < 1)
    {
	log_string ("\n    SP ");
	log_string (c_species[ambushing_species_index]->name);
	log_string (" attempted an ambush, but the ambush was completely ineffective!\n");
	return;
    }
 
    /* Age each ambushed ship. */
    for (ambushed_species_index = 0; ambushed_species_index < num_sp; ++ambushed_species_index)
    {
	if (! bat->enemy_mine[ambushing_species_index][ambushed_species_index])
	     continue;

	log_string ("\n    SP ");
	species_number = bat->spec_num[ambushed_species_index];
	if (field_distorted[ambushed_species_index])
	    log_int (distorted (species_number));
	else
	    log_string (c_species[ambushed_species_index]->name);

	log_string (" was ambushed by SP ");
	log_string (c_species[ambushing_species_index]->name);
	log_string ("!\n");

	num_ships = c_species[ambushed_species_index]->num_ships;
	sh = c_ship[ambushed_species_index] - 1;
	for (i = 0; i < num_ships; i++)
	{
	    ++sh;

	    if (sh->pn == 99) continue;
	    if (sh->x != bat->x) continue;
	    if (sh->y != bat->y) continue;
	    if (sh->z != bat->z) continue;

	    sh->age += age_increment;
	    if (sh->arrived_via_wormhole) sh->age += age_increment;

	    if (sh->age > 49)
	    {
		old_truncate_name = truncate_name;
		truncate_name = TRUE;

		log_string ("      ");
		log_string (ship_name (sh));
		if (field_distorted[ambushed_species_index])
		{
		    log_string (" = ");
		    log_string (c_species[ambushed_species_index]->name);
		    log_char (' ');
		    n = sh->item_quantity[FD];
		    sh->item_quantity[FD] = 0;
		    log_string (ship_name (sh));
		    sh->item_quantity[FD] = n;
		}
		n = 0;
		for (j = 0; j < MAX_ITEMS; j++)
		{
		    if (sh->item_quantity[j] > 0)
		    {
			if (n++ == 0)
			    log_string (" (cargo: ");
			else
			    log_char (',');
			log_int ((int) sh->item_quantity[j]);
			log_char (' ');
			log_string (item_abbr[j]);
		    }
		}
		if (n > 0) log_char (')');

		log_string (" was destroyed in the ambush!\n");

		truncate_name = old_truncate_name;
	    }
	}
    }
}



/* This routine will find all species that have declared alliance with
   both a traitor and betrayed species. It will then set a flag to indicate
   that their allegiance should be changed from ALLY to ENEMY. */

auto_enemy (traitor_species_number, betrayed_species_number)

int	traitor_species_number, betrayed_species_number;

{
    int		traitor_array_index, betrayed_array_index, bit_number,
		species_index;

    long	traitor_bit_mask, betrayed_bit_mask;


    traitor_array_index = (traitor_species_number - 1) / 32;
    bit_number = (traitor_species_number - 1) % 32;
    traitor_bit_mask = 1 << bit_number;

    betrayed_array_index = (betrayed_species_number - 1) / 32;
    bit_number = (betrayed_species_number - 1) % 32;
    betrayed_bit_mask = 1 << bit_number;

    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if ((spec_data[species_index].ally[traitor_array_index]
		& traitor_bit_mask) == 0) continue;
	if ((spec_data[species_index].ally[betrayed_array_index]
		& betrayed_bit_mask) == 0) continue;
	if ((spec_data[species_index].contact[traitor_array_index]
		& traitor_bit_mask) == 0) continue;
	if ((spec_data[species_index].contact[betrayed_array_index]
		& betrayed_bit_mask) == 0) continue;

	make_enemy[species_index][traitor_species_number - 1] = betrayed_species_number;
    }
}
