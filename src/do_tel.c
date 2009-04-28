
#include "fh.h"


#define MAX_OBS_LOCS	5000


extern int			first_pass, species_number, truncate_name,
				num_transactions;
extern char			input_line[256], *ship_name();
extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct ship_data		*ship;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_TELESCOPE_command ()
{
    int		i, n, found, range_in_parsecs, max_range, alien_index,
		alien_number, alien_nampla_index, alien_ship_index,
		location_printed, industry, detection_chance, num_obs_locs,
		alien_name_printed, loc_index, success_chance, something_found;

    long	x, y, z, max_distance, max_distance_squared,
		delta_x, delta_y, delta_z, distance_squared;

    char	planet_type[32], obs_x[MAX_OBS_LOCS], obs_y[MAX_OBS_LOCS],
		obs_z[MAX_OBS_LOCS];

    struct species_data		*alien;
    struct nampla_data		*alien_nampla;
    struct ship_data		*starbase, *alien_ship;


    found = get_ship ();
    if (! found  ||  ship->type != STARBASE)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid starbase name in TELESCOPE command.\n");
	return;
    }
    starbase = ship;

    /* Make sure starbase does not get more than one TELESCOPE order per
	turn. */
    if (starbase->dest_z != 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! A starbase may only be given one TELESCOPE order per turn.\n");
	return;
    }
    starbase->dest_z = 99;

    /* Get range of telescope. */
    range_in_parsecs = starbase->item_quantity[GT] / 2;
    if (range_in_parsecs < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Starbase is not carrying enough gravitic telescope units.\n");
	return;
    }

    /* Log the result. */
    if (first_pass)
    {
	log_string ("    A gravitic telescope at ");
	log_int (starbase->x);  log_char (' ');
	log_int (starbase->y);  log_char (' ');
	log_int (starbase->z);
	log_string (" will be operated by ");
	log_string (ship_name (starbase));
	log_string (".\n");
	return;
    }

    /* Define range parameters. */
    max_range = (int) species->tech_level[GV] / 10;
    if (range_in_parsecs > max_range) range_in_parsecs = max_range;

    x = starbase->x;
    y = starbase->y;
    z = starbase->z;

    max_distance = range_in_parsecs;
    max_distance_squared = max_distance * max_distance;

    /* First pass. Simply create a list of X Y Z locations that have observable
	aliens. */
    num_obs_locs = 0;
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++)
    {
	if (! data_in_memory[alien_index]) continue;

	alien_number = alien_index + 1;
	if (alien_number == species_number) continue;

	alien = &spec_data[alien_index];

	alien_nampla = namp_data[alien_index] - 1;
	for (alien_nampla_index = 0; alien_nampla_index < alien->num_namplas;
		alien_nampla_index++)
	{
	    ++alien_nampla;

	    if ((alien_nampla->status & POPULATED) == 0) continue;

	    delta_x = x - alien_nampla->x;
	    delta_y = y - alien_nampla->y;
	    delta_z = z - alien_nampla->z;
	    distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
		+ (delta_z * delta_z);

	    if (distance_squared == 0) continue;  /* Same loc as telescope. */
	    if (distance_squared > max_distance_squared) continue;

	    found = FALSE;
	    for (i = 0; i < num_obs_locs; i++)
	    {
		if (alien_nampla->x != obs_x[i]) continue;
		if (alien_nampla->y != obs_y[i]) continue;
		if (alien_nampla->z != obs_z[i]) continue;

		found = TRUE;
		break;
	    }
	    if (! found)
	    {
		if (num_obs_locs == MAX_OBS_LOCS)
		{
		    fprintf (stderr, "\n\nInternal error! MAX_OBS_LOCS exceeded in do_tel.c!\n\n");
		    exit (-1);
		}
		obs_x[num_obs_locs] = alien_nampla->x;
		obs_y[num_obs_locs] = alien_nampla->y;
		obs_z[num_obs_locs] = alien_nampla->z;

		++num_obs_locs;
	    }
	}

	alien_ship = ship_data[alien_index] - 1;
	for (alien_ship_index = 0; alien_ship_index < alien->num_ships;
		alien_ship_index++)
	{
	    ++alien_ship;

	    if (alien_ship->status == UNDER_CONSTRUCTION) continue;
	    if (alien_ship->status == ON_SURFACE) continue;
	    if (alien_ship->item_quantity[FD] == alien_ship->tonnage) continue;

	    delta_x = x - alien_ship->x;
	    delta_y = y - alien_ship->y;
	    delta_z = z - alien_ship->z;
	    distance_squared = (delta_x * delta_x) + (delta_y * delta_y)
		+ (delta_z * delta_z);

	    if (distance_squared == 0) continue;  /* Same loc as telescope. */
	    if (distance_squared > max_distance_squared) continue;

	    found = FALSE;
	    for (i = 0; i < num_obs_locs; i++)
	    {
		if (alien_ship->x != obs_x[i]) continue;
		if (alien_ship->y != obs_y[i]) continue;
		if (alien_ship->z != obs_z[i]) continue;

		found = TRUE;
		break;
	    }
	    if (! found)
	    {
		if (num_obs_locs == MAX_OBS_LOCS)
		{
		    fprintf (stderr, "\n\nInternal error! MAX_OBS_LOCS exceeded in do_tel.c!\n\n");
		    exit (-1);
		}
		obs_x[num_obs_locs] = alien_ship->x;
		obs_y[num_obs_locs] = alien_ship->y;
		obs_z[num_obs_locs] = alien_ship->z;

		++num_obs_locs;
	    }
	}
    }

    /* Operate the gravitic telescope. */
    log_string ("\n  Results of operation of gravitic telescope by ");
    log_string (ship_name (starbase));  log_string (" (location = ");
    log_int (starbase->x);  log_char (' ');
    log_int (starbase->y);  log_char (' ');
    log_int (starbase->z);
    log_string (", max range = ");
    log_int (range_in_parsecs);  log_string (" parsecs):\n");

    something_found = FALSE;

    for (loc_index = 0; loc_index < num_obs_locs; loc_index++)
    {
      x = obs_x[loc_index];
      y = obs_y[loc_index];
      z = obs_z[loc_index];

      location_printed = FALSE;

      for (alien_index = 0; alien_index < galaxy.num_species; alien_index++)
      {
	if (! data_in_memory[alien_index]) continue;

	alien_number = alien_index + 1;
	if (alien_number == species_number) continue;

	alien = &spec_data[alien_index];

	alien_name_printed = FALSE;

	alien_nampla = namp_data[alien_index] - 1;
	for (alien_nampla_index = 0; alien_nampla_index < alien->num_namplas;
		alien_nampla_index++)
	{
	    ++alien_nampla;

	    if ((alien_nampla->status & POPULATED) == 0) continue;
	    if (alien_nampla->x != x) continue;
	    if (alien_nampla->y != y) continue;
	    if (alien_nampla->z != z) continue;

	    industry = alien_nampla->mi_base + alien_nampla->ma_base;

	    success_chance = species->tech_level[GV];
	    success_chance += starbase->item_quantity[GT];
	    success_chance += (industry - 500) / 20;
	    if (alien_nampla->hiding  ||  alien_nampla->hidden)
		success_chance /= 10;

	    if (rnd(100) > success_chance) continue;

	    if (industry < 100)
		industry = (industry + 5)/10;
	    else
		industry = ((industry + 50)/100) * 10;

	    if (alien_nampla->status & HOME_PLANET)
		strcpy (planet_type, "Home planet");
	    else if (alien_nampla->status & RESORT_COLONY)
		strcpy (planet_type, "Resort colony");
	    else if (alien_nampla->status & MINING_COLONY)
		strcpy (planet_type, "Mining colony");
	    else
		strcpy (planet_type, "Colony");

	    if (! alien_name_printed)
	    {
		if (! location_printed)
		{
		    fprintf (log_file, "\n    %ld%3ld%3ld:\n", x, y, z);
		    location_printed = TRUE;
		    something_found = TRUE;
		}
		fprintf (log_file, "      SP %s:\n", alien->name);
		alien_name_printed = TRUE;
	    }

	    fprintf (log_file, "\t#%d: %s PL %s (%d)\n",
		alien_nampla->pn, planet_type, alien_nampla->name, industry);
	}

	alien_ship = ship_data[alien_index] - 1;
	for (alien_ship_index = 0; alien_ship_index < alien->num_ships;
		alien_ship_index++)
	{
	    ++alien_ship;

	    if (alien_ship->x != x) continue;
	    if (alien_ship->y != y) continue;
	    if (alien_ship->z != z) continue;
	    if (alien_ship->status == UNDER_CONSTRUCTION) continue;
	    if (alien_ship->status == ON_SURFACE) continue;
	    if (alien_ship->item_quantity[FD] == alien_ship->tonnage) continue;

	    success_chance = species->tech_level[GV];
	    success_chance += starbase->item_quantity[GT];
	    success_chance += alien_ship->tonnage - 10;
	    if (alien_ship->type == STARBASE) success_chance *= 2;
	    if (alien_ship->class == TR)
		success_chance = (3 * success_chance) / 2;
	    if (rnd(100) > success_chance) continue;

	    if (! alien_name_printed)
	    {
		if (! location_printed)
		{
		    fprintf (log_file, "\n    %ld%3ld%3ld:\n", x, y, z);
		    location_printed = TRUE;
		    something_found = TRUE;
		}
		fprintf (log_file, "      SP %s:\n", alien->name);
		alien_name_printed = TRUE;
	    }

	    truncate_name = FALSE;
	    fprintf (log_file, "\t%s", ship_name (alien_ship));
	    truncate_name = TRUE;

	    /* See if alien detected that it is being observed. */
	    if (alien_ship->type == STARBASE)
	    {
		detection_chance = 2 * alien_ship->item_quantity[GT];
		if (detection_chance > 0)
		{
		    fprintf (log_file, " <- %d GTs installed!",
			alien_ship->item_quantity[GT]);
		}
	    }
	    else
		detection_chance = 0;

	    fprintf (log_file, "\n");

	    detection_chance += 2 *
		((int) alien->tech_level[GV] - (int) species->tech_level[GV]);

	    if (rnd(100) > detection_chance) continue;

	    /* Define this transaction. */
	    if (num_transactions == MAX_TRANSACTIONS)
	    {
		fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
		exit (-1);
	    }

	    n = num_transactions++;
	    transaction[n].type = TELESCOPE_DETECTION;
	    transaction[n].x = starbase->x;
	    transaction[n].y = starbase->y;
	    transaction[n].z = starbase->z;
	    transaction[n].number1 = alien_number;
	    strcpy (transaction[n].name1, ship_name (alien_ship));
	}
      }
    }

    if (something_found)
	log_char ('\n');
    else
	log_string ("    No alien ships or planets were detected.\n\n");
}
