
#include "fh.h"


extern long	value;
extern char	input_line[256], original_line[256], *input_line_pointer;
extern FILE	*log_file;


extern struct species_data	*species;
extern struct ship_data		*ship_base, *ship;


do_REPAIR_command ()
{
    int			i, j, n, x, y, z, age_reduction, num_dr_units,
			total_dr_units, dr_units_used, max_age, desired_age;

    char		*original_line_pointer;

    struct ship_data	*damaged_ship;


    /* See if this is a "pool" repair. */
    if (get_value ())
    {
	x = value;
	get_value ();	y = value;
	get_value ();	z = value;

	if (get_value ())
	    desired_age = value;
	else
	    desired_age = 0;

	goto pool_repair;
    }

    /* Get the ship to be repaired. */
    original_line_pointer = input_line_pointer;
    if (! get_ship ())
    {
	/* Check for missing comma or tab after ship name. */
	input_line_pointer = original_line_pointer;
	fix_separator ();
	if (! get_ship ())
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship to be repaired does not exist.\n");
	    return;
	}
    }

    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Item to be repaired is still under construction.\n");
	return;
    }

    if (ship->age < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! Ship or starbase is too new to repair.\n");
	return;
    }

    /* Get number of damage repair units to use. */
    if (get_value ())
    {
	if (value == 0)
	    num_dr_units = ship->item_quantity[DR];
	else
	    num_dr_units = value;

	age_reduction = (16 * num_dr_units) / ship->tonnage;
	if (age_reduction > ship->age)
	{
	    age_reduction = ship->age;
	    n = age_reduction * ship->tonnage;
	    num_dr_units = (n + 15) / 16;
	}
    }
    else
    {
	age_reduction = ship->age;
	n = age_reduction * ship->tonnage;
	num_dr_units = (n + 15) / 16;
    }

    /* Check if sufficient units are available. */
    if (num_dr_units > ship->item_quantity[DR])
    {
	if (ship->item_quantity[DR] == 0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", original_line);
	    fprintf (log_file, "!!! Ship does not have any DRs!\n");
	    return;
	}
	fprintf (log_file, "! WARNING: %s", original_line);
	fprintf (log_file, "! Ship does not have %d DRs. Substituting %d for %d.\n",
	    num_dr_units, ship->item_quantity[DR], num_dr_units);
	num_dr_units = ship->item_quantity[DR];
    }

    /* Check if repair will have any effect. */
    age_reduction = (16 * num_dr_units) / ship->tonnage;
    if (age_reduction < 1)
    {
	if (value == 0) return;
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", original_line);
	fprintf (log_file, "!!! %d DRs is not enough to do a repair.\n",
	    num_dr_units);
	return;
    }

    /* Log what was repaired. */
    log_string ("    ");  log_string (ship_name (ship));
    log_string (" was repaired using ");
    log_int (num_dr_units);  log_char (' ');
    log_string (item_name[DR]);
    if (num_dr_units != 1) log_char ('s');
    log_string (". Age went from ");
    log_int ((int) ship->age);    log_string (" to ");
    ship->age -= age_reduction;
    if (ship->age < 0) ship->age = 0;
    ship->item_quantity[DR] -= num_dr_units;
    log_int ((int) ship->age);
    log_string (".\n");

    return;


pool_repair:

    /* Get total number of DR units available. */
    total_dr_units = 0;
    ship = ship_base - 1;
    for (i = 0; i < species->num_ships; i++)
    {
	++ship;

	if (ship->pn == 99) continue;
	if (ship->x != x) continue;
	if (ship->y != y) continue;
	if (ship->z != z) continue;

	total_dr_units += ship->item_quantity[DR];

	ship->special = 0;
    }

    /* Repair ships, starting with the most heavily damaged. */
    dr_units_used = 0;
    while (total_dr_units > 0)
    {
	/* Find most heavily damaged ship. */
	max_age = 0;
	ship = ship_base - 1;
	for (i = 0; i < species->num_ships; i++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;
	    if (ship->x != x) continue;
	    if (ship->y != y) continue;
	    if (ship->z != z) continue;
	    if (ship->special != 0) continue;
	    if (ship->status == UNDER_CONSTRUCTION) continue;

	    n = ship->age;
	    if (n > max_age)
	    {
		max_age = n;
		damaged_ship = ship;
	    }
	}

	if (max_age == 0) break;

	damaged_ship->special = 99;

	age_reduction = max_age - desired_age;
	n = age_reduction * damaged_ship->tonnage;
	num_dr_units = (n + 15) / 16;

	if (num_dr_units > total_dr_units)
	{
	    num_dr_units = total_dr_units;
	    age_reduction = (16 * num_dr_units) / damaged_ship->tonnage;
	}

	if (age_reduction < 1) continue;  /* This ship is too big. */

	log_string ("    ");  log_string (ship_name (damaged_ship));
	log_string (" was repaired using ");
	log_int (num_dr_units);  log_char (' ');
	log_string (item_name[DR]);
	if (num_dr_units != 1) log_char ('s');
	log_string (". Age went from ");
	log_int ((int) damaged_ship->age);    log_string (" to ");
	damaged_ship->age -= age_reduction;
	if (damaged_ship->age < 0) damaged_ship->age = 0;
	log_int ((int) damaged_ship->age);
	log_string (".\n");

	total_dr_units -= num_dr_units;
	dr_units_used += num_dr_units;
    }

    if (dr_units_used == 0) return;

    /* Subtract units used from ships at the location. */
    ship = ship_base - 1;
    for (i = 0; i < species->num_ships; i++)
    {
	++ship;

	if (ship->pn == 99) continue;
	if (ship->x != x) continue;
	if (ship->y != y) continue;
	if (ship->z != z) continue;

	n = ship->item_quantity[DR];
	if (n < 1) continue;
	if (n > dr_units_used) n = dr_units_used;

	ship->item_quantity[DR] -= n;
	dr_units_used -= n;

	if (dr_units_used == 0) break;
    }
}
