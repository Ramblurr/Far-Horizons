
#include "fh.h"


int		last_planet_produced = FALSE;

int		shipyard_built, shipyard_capacity;

extern int	nampla_index, doing_production, first_pass, next_nampla_index,
		planet_data_modified, species_number, num_transactions;
extern long	balance, raw_material_units, production_capacity,
		EU_spending_limit;
extern char	production_done[1000], input_line[256], upper_name[32];
extern FILE	*log_file;

extern struct planet_data	*planet_base, *planet, *home_planet;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base, *nampla, *next_nampla;
extern struct ship_data		*ship_base;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_PRODUCTION_command (missing_production_order)

int	missing_production_order;

{
    int		i, j, abbr_type, name_length, found, alien_number, under_siege,
		siege_percent_effectiveness, new_alien, num_siege_ships,
		mining_colony, resort_colony, special_colony, ship_index,
		enemy_on_same_planet, trans_index, production_penalty,
		ls_needed, shipyards_for_this_species;

    char	upper_nampla_name[32];

    long	n, RMs_produced, num_bytes, total_siege_effectiveness,
		siege_effectiveness[MAX_SPECIES+1], EUs_available_for_siege,
		EUs_for_distribution, EUs_for_this_species, total_EUs_stolen,
		special_production, pop_units_here[MAX_SPECIES+1],
		alien_pop_units, total_alien_pop_here, total_besieged_pop,
		ib_for_this_species, ab_for_this_species, total_ib, total_ab,
		total_effective_tonnage;

    struct species_data		*alien;
    struct nampla_data		*alien_nampla_base, *alien_nampla;
    struct ship_data		*alien_ship_base, *alien_ship, *ship;


    if (doing_production)
    {
	/* Terminate production for previous planet. */
	if (last_planet_produced)
	{
	    transfer_balance ();
	    last_planet_produced = FALSE;
	}

	/* Give gamemaster option to abort. */
	if (first_pass) gamemaster_abort_option ();
	log_char ('\n');
    }

    doing_production = TRUE;

    if (missing_production_order)
    {
	nampla = next_nampla;
	nampla_index = next_nampla_index;

	goto got_nampla;
    }

    /* Get PL abbreviation. */
    abbr_type = get_class_abbr ();

    if (abbr_type != PLANET_ID)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in PRODUCTION command.\n");
	return;
    }

    /* Get planet name. */
    name_length = get_name ();

    /* Search all namplas for name. */
    found = FALSE;
    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
    {
	++nampla;

	if (nampla->pn == 99) continue;

	/* Make upper case copy of nampla name. */
	for (i = 0; i < 32; i++)
	    upper_nampla_name[i] = toupper(nampla->name[i]);

	/* Compare names. */
	if (strcmp (upper_nampla_name, upper_name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in PRODUCTION command.\n");
	return;
    }

    /* Check if production was already done for this planet. */
    if (production_done[nampla_index])
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! More than one PRODUCTION command for planet.\n");
	return;
    }
    production_done[nampla_index] = TRUE;

    /* Check if this colony was disbanded. */
    if (nampla->status & DISBANDED_COLONY)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Production orders cannot be given for a disbanded colony!\n");
	return;
    }

got_nampla:

    last_planet_produced = TRUE;
    shipyard_built = FALSE;
    shipyard_capacity = nampla->shipyards;

    /* See if this is a mining or resort colony. */
    mining_colony = FALSE;
    resort_colony = FALSE;
    special_colony = FALSE;
    if (nampla->status & MINING_COLONY)
    {
	mining_colony = TRUE;
	special_colony = TRUE;
    }
    else if (nampla->status & RESORT_COLONY)
    {
	resort_colony = TRUE;
	special_colony = TRUE;
    }

    /* Get planet data for this nampla. */
    planet = planet_base + (long) nampla->planet_index;

    /* Check if fleet maintenance cost is so high that riots ensued. */
    i = 0;
    j = (species->fleet_percent_cost - 10000) / 100;
    if (rnd(100) <= j)
    {
	log_string ("!!! WARNING! Riots on PL ");
	log_string (nampla->name);
	log_string (" due to excessive and unpopular military build-up reduced ");

	if (mining_colony  ||  ! special_colony)
	{
	    log_string ("mining base by ");
	    i = rnd (j);
	    log_int (i);  log_string (" percent ");
	    nampla->mi_base -= (i * nampla->mi_base) / 100;
	}

	if (resort_colony  ||  ! special_colony)
	{
	    if (i) log_string ("and ");
	    log_string ("manufacturing base by ");
	    i = rnd (j);
	    log_int (i);  log_string (" percent");
	    nampla->ma_base -= (i * nampla->ma_base) / 100;
	}
	log_string ("!\n\n");
    }

    /* Calculate "balance" available for spending and create pseudo
	"checking account". */
    ls_needed = life_support_needed (species, home_planet, planet);

    if (ls_needed == 0)
	production_penalty = 0;
    else
	production_penalty = (100 * ls_needed) / species->tech_level[LS];

    RMs_produced =
	(10L * (long) species->tech_level[MI] * (long) nampla->mi_base)
		/ (long) planet->mining_difficulty;
    RMs_produced
	-= (production_penalty * RMs_produced) / 100;
    RMs_produced
	= (((long) planet->econ_efficiency * RMs_produced) + 50) / 100;

    if (special_colony)
	/* RMs just 'sitting' on the planet cannot be converted to EUs on a
		mining colony, and cannot create a 'balance' on a resort
		colony. */
	raw_material_units = 0;
    else
	raw_material_units = RMs_produced + nampla->item_quantity[RM];

    production_capacity =
	((long) species->tech_level[MA] * (long) nampla->ma_base) / 10L;
    production_capacity
	-= (production_penalty * production_capacity) / 100;
    production_capacity
	= (((long) planet->econ_efficiency * production_capacity) + 50) / 100;

    balance = (raw_material_units > production_capacity)
				? production_capacity : raw_material_units;

    if (species->fleet_percent_cost > 10000)
	n = 10000;
    else
	n = species->fleet_percent_cost;

    if (special_colony)
	EU_spending_limit = 0;
    else
    {
	/* Only excess RMs may be recycled. */
	nampla->item_quantity[RM] = raw_material_units - balance;

	balance -= ((n * balance) + 5000) / 10000;
	raw_material_units = balance;
	production_capacity = balance;
	EUs_available_for_siege = balance;
	if (nampla->status & HOME_PLANET)
	{
	    if (species->hp_original_base != 0)  /* HP was bombed. */
		EU_spending_limit = 4 * balance;  /* Factor = 4 + 1 = 5. */
	    else
		EU_spending_limit = species->econ_units;
	}
	else
	    EU_spending_limit = balance;
    }

    /* Log what was done. Balances for mining and resort colonies will always
	be zero and should not be printed. */
    log_string ("  Start of production on PL ");  log_string (nampla->name);
	log_char ('.');
    if (! special_colony)
    {
	log_string (" (Initial balance is ");  log_long (balance);
	    log_string (".)");
    }
    log_char ('\n');

    /* If this IS a mining or resort colony, convert RMs or production capacity
	to EUs. */
    if (mining_colony)
    {
	special_production = (2 * RMs_produced) / 3;
	special_production -= ((n * special_production) + 5000) / 10000;
	log_string ("    Mining colony ");
    }
    else if (resort_colony)
    {
	special_production = (2 * production_capacity) / 3;
	special_production -= ((n * special_production) + 5000) / 10000;
	log_string ("    Resort colony ");
    }

    if (special_colony)
    {
	log_string (nampla->name);  log_string (" generated ");
	log_long (special_production);  log_string (" economic units.\n");

	EUs_available_for_siege = special_production;
	species->econ_units += special_production;

	if (mining_colony && ! first_pass)
	{
	    planet->mining_difficulty += RMs_produced/150;
	    planet_data_modified = TRUE;
	}
    }

    /* Check if this planet is under siege. */
    nampla->siege_eff = 0;
    under_siege = FALSE;
    alien_number = 0;
    num_siege_ships = 0;
    total_siege_effectiveness = 0;
    enemy_on_same_planet = FALSE;
    total_alien_pop_here = 0;
    for (i = 1; i <= MAX_SPECIES; i++)
    {
	siege_effectiveness[i] = 0;
	pop_units_here[i] = 0;
    }

    for (trans_index = 0; trans_index < num_transactions; trans_index++)
    {
	/* Check if this is a siege of this nampla. */
	if (transaction[trans_index].type != BESIEGE_PLANET) continue;
	if (transaction[trans_index].x != nampla->x) continue;
	if (transaction[trans_index].y != nampla->y) continue;
	if (transaction[trans_index].z != nampla->z) continue;
	if (transaction[trans_index].pn != nampla->pn) continue;
	if (transaction[trans_index].number2 != species_number) continue;

	/* Check if alien ship is still in the same star system as the
		planet. */
	if (alien_number != transaction[trans_index].number1)
	{
	    /* First transaction for this alien. */
	    alien_number = transaction[trans_index].number1;
	    if (! data_in_memory[alien_number - 1])
	    {
		fprintf (stderr, "\n\tData for species #%d should be in memory but is not!\n\n",
			alien_number);
		exit (-1);
	    }
	    alien = &spec_data[alien_number - 1];
	    alien_nampla_base = namp_data[alien_number - 1];
	    alien_ship_base = ship_data[alien_number - 1];

	    new_alien = TRUE;
	}

	/* Find the alien ship. */
	found = FALSE;
	alien_ship = alien_ship_base - 1;
	for (i = 0; i < alien->num_ships; i++)
	{
	    ++alien_ship;

	    if (alien_ship->pn == 99) continue;

	    if (strcmp(alien_ship->name, transaction[trans_index].name3) == 0)
	    {
		found = TRUE;
		break;
	    }
	}

	/* Check if alien ship is still at the siege location. */
	if (! found) continue;		/* It must have jumped away and self-
						destructed, or was recycled. */
	if (alien_ship->x != nampla->x) continue;
	if (alien_ship->y != nampla->y) continue;
	if (alien_ship->z != nampla->z) continue;
	if (alien_ship->class == TR) continue;

	/* This nampla is under siege. */
	if (! under_siege)
	{
	    log_string ("\n    WARNING! PL ");  log_string (nampla->name);
	    log_string (" is under siege by the following:\n      ");
	    under_siege = TRUE;
	}

	if (num_siege_ships++ > 0) log_string (", ");
	if (new_alien)
	{
	    log_string (alien->name);  log_char (' ');
	    new_alien = FALSE;

	    /* Check if this alien has a colony on the same planet. */
	    alien_nampla = alien_nampla_base - 1;
	    for (i = 0; i < alien->num_namplas; i++)
	    {
		++alien_nampla;

		if (alien_nampla->x != nampla->x) continue;
		if (alien_nampla->y != nampla->y) continue;
		if (alien_nampla->z != nampla->z) continue;
		if (alien_nampla->pn != nampla->pn) continue;

		/* Enemy population that will count for both detection AND
			assimilation. */
		alien_pop_units = alien_nampla->mi_base
			+ alien_nampla->ma_base
			+ alien_nampla->IUs_to_install
			+ alien_nampla->AUs_to_install;

		/* Any base over 200.0 has only 5% effectiveness. */
		if (alien_pop_units > 2000)
		    alien_pop_units = (alien_pop_units - 2000) / 20  +  2000;

		/* Enemy population that counts ONLY for detection. */
		n = alien_nampla->pop_units
			+ alien_nampla->item_quantity[CU]
			+ alien_nampla->item_quantity[PD];

		if (alien_pop_units > 0)
		{
		    enemy_on_same_planet = TRUE;
		    pop_units_here[alien_number] = alien_pop_units;
		    total_alien_pop_here += alien_pop_units;
		}
		else if (n > 0)
		    enemy_on_same_planet = TRUE;

		if (alien_nampla->item_quantity[PD] == 0) continue;

		log_string ("planetary defenses of PL ");
		log_string (alien_nampla->name);
		log_string (", ");

		n = (4 * alien_nampla->item_quantity[PD]) / 5;
		n = (n * (long) alien->tech_level[ML])
			/ ((long) species->tech_level[ML] + 1);
		total_siege_effectiveness += n;
		siege_effectiveness[alien_number] += n;
	    }
	}
	log_string (ship_name(alien_ship));

	/* Determine the number of planets that this ship is besieging. */
	n = 0;
	for (j = 0; j < num_transactions; j++)
	{
	    if (transaction[j].type != BESIEGE_PLANET) continue;
	    if (transaction[j].number1 != alien_number) continue;
	    if (strcmp (transaction[j].name3, alien_ship->name) != 0) continue;

	    ++n;
	}

	/* Determine the effectiveness of this ship on the siege. */
	if (alien_ship->type == STARBASE)
	    i = alien_ship->tonnage;	/* One quarter of normal ships. */
	else
	    i = 4 * (int) alien_ship->tonnage;

	i = (i * (int) alien->tech_level[ML])
		/ ((int) species->tech_level[ML] + 1);

	i /= n;

	total_siege_effectiveness += i;
	siege_effectiveness[alien_number] += i;
    }

    if (under_siege)
	log_string (".\n");
    else
	return;

    /* Determine percent effectiveness of the siege. */
    total_effective_tonnage = 2500 * total_siege_effectiveness;

    if (nampla->mi_base + nampla->ma_base  ==  0)
	siege_percent_effectiveness = -9999;	/* New colony with nothing
						    installed yet. */
    else
	siege_percent_effectiveness = total_effective_tonnage
	    / ((((long) species->tech_level[MI] * (long) nampla->mi_base)
		+ ((long) species->tech_level[MA] * (long) nampla->ma_base))
		/ 10L);

    if (siege_percent_effectiveness > 95)
	siege_percent_effectiveness = 95;
    else if (siege_percent_effectiveness == -9999)
    {
	log_string ("      However, although planet is populated, it has no economic base.\n\n");
	return;
    }
    else if (siege_percent_effectiveness < 1)
    {
	log_string ("      However, because of the weakness of the siege, it was completely ineffective!\n\n");
	return;
    }

    if (enemy_on_same_planet)
	nampla->siege_eff = -siege_percent_effectiveness;
    else
	nampla->siege_eff = siege_percent_effectiveness;

    log_string ("      The siege is approximately ");
    log_int (siege_percent_effectiveness);
    log_string ("% effective.\n");

    /* Add siege EU transfer(s). */
    EUs_for_distribution
	= (siege_percent_effectiveness * EUs_available_for_siege) / 100;

    total_EUs_stolen = 0;

    for (alien_number = 1; alien_number <= MAX_SPECIES; alien_number++)
    {
	n = siege_effectiveness[alien_number];
	if (n < 1) continue;
	alien = &spec_data[alien_number - 1];
	EUs_for_this_species
	    = (n * EUs_for_distribution) / total_siege_effectiveness;
	if (EUs_for_this_species < 1) continue;
	total_EUs_stolen += EUs_for_this_species;
	log_string ("      ");  log_long (EUs_for_this_species);
	log_string (" economic unit");
	if (EUs_for_this_species > 1)
	    log_string ("s were");
	else
	    log_string (" was");
	log_string (" lost and 25% of the amount was transferred to SP ");
	log_string (alien->name);
	log_string (".\n");

	if (first_pass) continue;

	/* Define this transaction and add to list of transactions. */
	if (num_transactions == MAX_TRANSACTIONS)
	{
	    fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	    exit (-1);
	}

	trans_index = num_transactions++;
	transaction[trans_index].type = SIEGE_EU_TRANSFER;
	transaction[trans_index].donor = species_number;
	transaction[trans_index].recipient = alien_number;
	transaction[trans_index].value = EUs_for_this_species/4;
	transaction[trans_index].x = nampla->x;
	transaction[trans_index].y = nampla->y;
	transaction[trans_index].z = nampla->z;
	transaction[trans_index].number1 = siege_percent_effectiveness;
	strcpy (transaction[trans_index].name1, species->name);
	strcpy (transaction[trans_index].name2, alien->name);
	strcpy (transaction[trans_index].name3, nampla->name);
    }
    log_char ('\n');

    /* Correct balances. */
    if (special_colony)
	species->econ_units -= total_EUs_stolen;
    else
    {
	if (check_bounced (total_EUs_stolen))
	{
	    fprintf (stderr, "\nWARNING! Internal error! Should never reach this point!\n\n");
	    exit (-1);
	}
    }

    if (! enemy_on_same_planet) return;

    /* All ships currently under construction may be detected by the besiegers
	and destroyed. */
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	ship = ship_base + ship_index;

	if (ship->status == UNDER_CONSTRUCTION
		&&  ship->x == nampla->x
		&&  ship->y == nampla->y
		&&  ship->z == nampla->z
		&&  ship->pn == nampla->pn)
	{
	    if (rnd(100) > siege_percent_effectiveness) continue;

	    log_string ("      ");
	    log_string (ship_name (ship));
	    log_string (", under construction when the siege began, was detected by the besiegers and destroyed!\n");
	    if (! first_pass) delete_ship (ship);
	}
    }

    /* Check for assimilation. */
    if (nampla->status & HOME_PLANET) return;
    if (total_alien_pop_here < 1) return;

    total_besieged_pop = nampla->mi_base + nampla->ma_base 
	+ nampla->IUs_to_install + nampla->AUs_to_install;

    /* Any base over 200.0 has only 5% effectiveness. */
    if (total_besieged_pop > 2000)
	total_besieged_pop = (total_besieged_pop - 2000) / 20  +  2000;

    if (total_besieged_pop / total_alien_pop_here  >=  5) return;
    if (siege_percent_effectiveness < 95) return;

    log_string ("      PL ");  log_string (nampla->name);
    log_string (" has become assimilated by the besieging species");
    log_string (" and is no longer under your control.\n\n");

    total_ib = nampla->mi_base;  /* My stupid compiler can't add an int and
					an unsigned short. */
    total_ib += nampla->IUs_to_install;
    total_ab = nampla->ma_base;
    total_ab += nampla->AUs_to_install;

    for (alien_number = 1; alien_number <= MAX_SPECIES; alien_number++)
    {
	n = pop_units_here[alien_number];
	if (n < 1) continue;

	shipyards_for_this_species
	    = (n * nampla->shipyards) / total_alien_pop_here;

	ib_for_this_species
	    = (n * total_ib) / total_alien_pop_here;
	total_ib -= ib_for_this_species;

	ab_for_this_species
	    = (n * total_ab) / total_alien_pop_here;
	total_ab -= ab_for_this_species;

	if (ib_for_this_species == 0  &&  ab_for_this_species == 0) continue;

	if (first_pass) continue;

	/* Define this transaction and add to list of transactions. */
	if (num_transactions == MAX_TRANSACTIONS)
	{
	    fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	    exit (-1);
	}

	trans_index = num_transactions++;
	transaction[trans_index].type = ASSIMILATION;
	transaction[trans_index].value = alien_number;
	transaction[trans_index].x = nampla->x;
	transaction[trans_index].y = nampla->y;
	transaction[trans_index].z = nampla->z;
	transaction[trans_index].pn = nampla->pn;
	transaction[trans_index].number1 = ib_for_this_species/2;
	transaction[trans_index].number2 = ab_for_this_species/2;
	transaction[trans_index].number3 = shipyards_for_this_species;
	strcpy (transaction[trans_index].name1, species->name);
	strcpy (transaction[trans_index].name2, nampla->name);
    }

    /* Erase the original colony. */
    balance = 0;
    EU_spending_limit = 0;
    raw_material_units = 0;
    production_capacity = 0;
    nampla->mi_base = 0;
    nampla->ma_base = 0;
    nampla->IUs_to_install = 0;
    nampla->AUs_to_install = 0;
    nampla->pop_units = 0;
    nampla->siege_eff = 0;
    nampla->status = COLONY;
    nampla->shipyards = 0;
    nampla->hiding = 0;
    nampla->hidden = 0;
    nampla->use_on_ambush = 0;

    for (i = 0; i < MAX_ITEMS; i++) nampla->item_quantity[i] = 0;
}
