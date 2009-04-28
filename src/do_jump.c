
#include "fh.h"


int				using_alien_portal, other_species_number,
				jump_portal_gv;

char				jump_portal_name[128];

short				jump_portal_units, jump_portal_age;

struct species_data		*other_species;
struct ship_data		*alien_portal;


extern int			x, y, z, pn, first_pass, ship_index, abbr_type,
				abbr_index, species_number, num_transactions;

extern char			input_line[256], original_line[256],
				upper_name[32], *input_line_pointer,
				*ship_name();

extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct star_data		*star;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship, *ship_base;
extern struct trans_data	transaction[MAX_TRANSACTIONS];



do_JUMP_command (jumped_in_combat, using_jump_portal)

int	jumped_in_combat, using_jump_portal;

{
    int		i, n, found, max_xyz, temp_x, temp_y, temp_z, difference,
		status, mishap_gv;

    long	mishap_chance, success_chance;

    char	temp_string[32], *original_line_pointer;

    short	mishap_age;

    struct ship_data	*jump_portal;


    /* Set default status at end of jump. */
    status = IN_DEEP_SPACE;

    /* Check if this ship jumped in combat. */
    if (jumped_in_combat)
    {
	x = ship->dest_x;  y = ship->dest_y;  z = ship->dest_z;  pn = 0;
	using_jump_portal = FALSE;
	nampla = NULL;
	goto do_jump;
    }

    /* Get ship making the jump. */
    original_line_pointer = input_line_pointer;
    found = get_ship ();
    if (! found)
    {
	input_line_pointer = original_line_pointer;
	fix_separator ();	/* Check for missing comma or tab. */
	found = get_ship ();	/* Try again. */
	if (! found)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid ship name in JUMP or PJUMP command.\n");
	    return;
	}
    }

    /* Make sure ship is not salvage of a disbanded colony. */
    if (disbanded_ship (ship))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! This ship is salvage of a disbanded colony!\n");
	return;
    }

    /* Check if this ship withdrew or was was forced to jump from combat.
	If so, ignore specified coordinates and use those provided by the
	combat program. */
    if (ship->status == FORCED_JUMP  ||  ship->status == JUMPED_IN_COMBAT)
    {
	x = ship->dest_x;  y = ship->dest_y;  z = ship->dest_z;  pn = 0;
	jumped_in_combat = TRUE;
	using_jump_portal = FALSE;
	nampla = NULL;
	goto do_jump;
    }

    /* Make sure ship can jump. */
    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s is still under construction!\n",
				ship_name (ship));
	return;
    }

    if (ship->type == STARBASE
	||  (! using_jump_portal  &&  ship->type == SUB_LIGHT))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s cannot make an interstellar jump!\n",
				ship_name (ship));
	return;
    }

    /* Check if JUMP, MOVE, or WORMHOLE was already done for this ship. */
    if (ship->just_jumped)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s already jumped or moved this turn!\n",
		ship_name (ship));
	return;
    }

    /* Get the destination. */
    original_line_pointer = input_line_pointer;
    found = get_location();
    if (! found)
    {
	if (using_jump_portal)
	{
	    input_line_pointer = original_line_pointer;
	    fix_separator ();	/* Check for missing comma or tab. */
	    found = get_location();	/* Try again. */
	}

	if (! found)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid destination in JUMP or PJUMP command.\n");
	    return;
	}
    }

    /* Set status to IN_ORBIT if destination is a planet. */
    if (pn > 0) status = IN_ORBIT;

    /* Check if a jump portal is being used. */
    if (using_jump_portal)
    {
	found = get_jump_portal ();
	if (! found)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Invalid starbase name in PJUMP command.\n");
	    return;
	}
    }

    /* If using a jump portal, make sure that starbase has sufficient number
	of jump portal units. */
    if (using_jump_portal)
    {
	if (jump_portal_units < ship->tonnage)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Starbase does not have enough Jump Portal Units!\n");
	    return;
	}
    }

do_jump:

    if (x == ship->x  &&  y == ship->y  &&  z == ship->z)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %s was already at specified x,y,z.\n",
			ship_name (ship));
	return;
    }

    /* Set flags to show that ship jumped this turn. */
    ship->just_jumped = TRUE;

    /* Calculate basic mishap probability. */
    if (using_jump_portal)
    {
	mishap_age = jump_portal_age;
	mishap_gv = jump_portal_gv;
    }
    else
    {
	mishap_age = ship->age;
	mishap_gv = species->tech_level[GV];
    }
    mishap_chance = ( 100L * (long) ( ((x - ship->x) * (x - ship->x))
				+     ((y - ship->y) * (y - ship->y))
				+     ((z - ship->z) * (z - ship->z)) ) )
			/ (long) mishap_gv;

    if (mishap_chance > 10000L)
    {
	mishap_chance = 10000L;
	goto start_jump;
    }

    /* Add aging effect. */
    if (mishap_age > 0)
    {
	success_chance = 10000L - mishap_chance;
	success_chance -= (2L * (long) mishap_age * success_chance)/100L;
	if (success_chance < 0) success_chance = 0;
	mishap_chance = 10000L - success_chance;
    }


start_jump:

    log_string ("    ");
    log_string (ship_name (ship));
    log_string (" will try to jump to ");

    if (nampla == NULL)
    {
	log_int (x);  log_char (' ');
	log_int (y);  log_char (' ');
	log_int (z);
    }
    else
    {
	log_string ("PL ");
	log_string (nampla->name);
    }

    if (using_jump_portal)
    {
	log_string (" via jump portal ");
	log_string (jump_portal_name);

	if (using_alien_portal  &&  ! first_pass)
	{
	    /* Define this transaction. */
	    if (num_transactions == MAX_TRANSACTIONS)
	    {
		fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		exit (-1);
	    }

	    n = num_transactions++;
	    transaction[n].type = ALIEN_JUMP_PORTAL_USAGE;
	    transaction[n].number1 = other_species_number;
	    strcpy (transaction[n].name1, species->name);
	    strcpy (transaction[n].name2, ship_name(ship));
	    strcpy (transaction[n].name3, ship_name(alien_portal));
	}
    }

    sprintf (temp_string, " (%ld.%02ld%%).\n", mishap_chance/100L,
	mishap_chance%100L);
    log_string (temp_string);

jump_again:

    if (first_pass  ||  (rnd(10000) > mishap_chance))
    {
	ship->x = x;	ship->y = y;	ship->z = z;	ship->pn = pn;
	ship->status = status;

	if (! first_pass) star_visited (x, y, z);

	return;
    }

    /* Ship had a mishap. Check if it has any fail-safe jump units. */
    if (ship->item_quantity[FS] > 0)
    {
	if (num_transactions == MAX_TRANSACTIONS)
	{
	    fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
	    exit (-1);
	}

	n = num_transactions++;
	transaction[n].type = SHIP_MISHAP;
	transaction[n].value = 4;	/* Use of one FS. */
	transaction[n].number1 = species_number;
	strcpy (transaction[n].name1, ship_name(ship));

	ship->item_quantity[FS] -= 1;
	goto jump_again;
    }

    /* If ship was forced to jump, and it reached this point, then it
	self-destructed. */
    if (ship->status == FORCED_JUMP) goto self_destruct;

    /* Check if ship self-destructed or just mis-jumped. */
    if (rnd(10000) > mishap_chance)
    {
	/* Calculate mis-jump location. */
	max_xyz = 2 * galaxy.radius  -  1;

    try_again:
	temp_x = -1;
	difference = (ship->x > x) ? (ship->x - x) : (x - ship->x);
	difference = (2L * mishap_chance * difference)/10000L;
	if (difference < 3) difference = 3;
	while  (temp_x < 0  ||  temp_x > max_xyz)
	    temp_x = x - rnd(difference) + rnd(difference);

	temp_y = -1;
	difference = (ship->y > y) ? (ship->y - y) : (y - ship->y);
	difference = (2L * mishap_chance * difference)/10000L;
	if (difference < 3) difference = 3;
	while  (temp_y < 0  ||  temp_y > max_xyz)
	    temp_y = y - rnd(difference) + rnd(difference);

	temp_z = -1;
	difference = (ship->z > z) ? (ship->z - z) : (z - ship->z);
	difference = (2L * mishap_chance * difference)/10000L;
	if (difference < 3) difference = 3;
	while  (temp_z < 0  ||  temp_z > max_xyz)
	    temp_z = z - rnd(difference) + rnd(difference);

	if (x == temp_x  &&  y == temp_y  &&  z == temp_z)
	    goto try_again;

	if (num_transactions == MAX_TRANSACTIONS)
	{
	    fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
	    exit (-1);
	}

	n = num_transactions++;
	transaction[n].type = SHIP_MISHAP;
	transaction[n].value = 3;	/* Mis-jump. */
	transaction[n].number1 = species_number;
	strcpy (transaction[n].name1, ship_name(ship));
	transaction[n].x = temp_x;
	transaction[n].y = temp_y;
	transaction[n].z = temp_z;

	ship->x = temp_x;	ship->y = temp_y;	ship->z = temp_z;
	ship->pn = 0;

	ship->status = IN_DEEP_SPACE;

	star_visited (temp_x, temp_y, temp_z);

	return;
    }

self_destruct:

    /* Ship self-destructed. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS in do_int.c!\n\n");
	exit (-1);
    }

    n = num_transactions++;
    transaction[n].type = SHIP_MISHAP;
    transaction[n].value = 2;	/* Self-destruction. */
    transaction[n].number1 = species_number;
    strcpy (transaction[n].name1, ship_name(ship));

    delete_ship (ship);
}



int get_jump_portal ()

{
    int		i, j, k, found, array_index, bit_number;

    long	bit_mask;

    char	start_x, start_y, start_z, upper_ship_name[32],
		*original_line_pointer;

    struct species_data		*original_species;

    struct ship_data		*temp_ship, *portal, *original_ship,
				*original_ship_base;


    /* See if specified starbase is owned by the current species. */
    original_line_pointer = input_line_pointer;
    temp_ship = ship;
    found = get_ship();
    portal = ship;
    ship = temp_ship;
    using_alien_portal = FALSE;

    if (found)
    {
	if (portal->type != STARBASE) return FALSE;
	jump_portal_age = portal->age;
	jump_portal_gv = species->tech_level[GV];
	jump_portal_units = portal->item_quantity[JP];
	strcpy (jump_portal_name, ship_name (portal));
	return TRUE;
    }

    start_x = ship->x;
    start_y = ship->y;
    start_z = ship->z;

    if (abbr_type != SHIP_CLASS) goto check_for_bad_spelling;
    if (abbr_index != BA) goto check_for_bad_spelling;

    /* It IS the name of a starbase.  See if another species has given
	permission to use their starbase. */

    for (other_species_number = 1; other_species_number <= galaxy.num_species;
	other_species_number++)
    {
	if (! data_in_memory[other_species_number - 1]) continue;

	if (other_species_number == species_number) continue;

	other_species = &spec_data[other_species_number - 1];

	found = FALSE;

	/* Check if other species has declared this species as an ally. */
	array_index = (species_number - 1) / 32;
	bit_number = (species_number - 1) % 32;
	bit_mask = 1 << bit_number;
	if ((other_species->ally[array_index] & bit_mask) == 0) continue;

	/* See if other species has a starbase with the specified name at
		the start location. */
	alien_portal = ship_data[other_species_number - 1] - 1;
	for (j = 0; j < other_species->num_ships; j++)
	{
	    ++alien_portal;

	    if (alien_portal->type != STARBASE) continue;
	    if (alien_portal->x != start_x) continue;
	    if (alien_portal->y != start_y) continue;
	    if (alien_portal->z != start_z) continue;
	    if (alien_portal->pn == 99) continue;

	    /* Make upper case copy of ship name. */
	    for (k = 0; k < 32; k++)
		upper_ship_name[k] = toupper(alien_portal->name[k]);

	    /* Compare names. */
	    if (strcmp (upper_ship_name, upper_name) == 0)
	    {
		found = TRUE;
		break;
	    }
	}

	if (found)
	{
	    jump_portal_units = alien_portal->item_quantity[JP];
	    jump_portal_age = alien_portal->age;
	    jump_portal_gv = other_species->tech_level[GV];
	    strcpy (jump_portal_name, ship_name (alien_portal));
	    strcat (jump_portal_name, " owned by SP ");
	    strcat (jump_portal_name, other_species->name);

	    using_alien_portal = TRUE;

	    break;
	}
    }

    if (found) return TRUE;

check_for_bad_spelling:

    /* Try again, but allow spelling errors. */
    original_ship = ship;
    original_ship_base = ship_base;
    original_species = species;

    for (other_species_number = 1; other_species_number <= galaxy.num_species;
	other_species_number++)
    {
	if (! data_in_memory[other_species_number - 1]) continue;

	if (other_species_number == species_number) continue;

	species = &spec_data[other_species_number - 1];

	/* Check if other species has declared this species as an ally. */
	array_index = (species_number - 1) / 32;
	bit_number = (species_number - 1) % 32;
	bit_mask = 1 << bit_number;
	if ((species->ally[array_index] & bit_mask) == 0) continue;

	input_line_pointer = original_line_pointer;

	ship_base = ship_data[other_species_number - 1];

	found = get_ship ();

	if (found)
	{
	    found = FALSE;

	    if (ship->type != STARBASE) continue;
	    if (ship->x != start_x) continue;
	    if (ship->y != start_y) continue;
	    if (ship->z != start_z) continue;
	    if (ship->pn == 99) continue;

	    found = TRUE;
	    break;
	}
    }

    if (found)
    {
	jump_portal_units = ship->item_quantity[JP];
	jump_portal_age = ship->age;
	jump_portal_gv = species->tech_level[GV];
	strcpy (jump_portal_name, ship_name (ship));
	strcat (jump_portal_name, " owned by SP ");
	strcat (jump_portal_name, species->name);

	using_alien_portal = TRUE;
    }

    species = original_species;
    ship = original_ship;
    ship_base = original_ship_base;

    return found;
}
