
#include "fh.h"


extern int			species_number;
extern char			input_line[256];
extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct nampla_data	*nampla, *nampla_base;
extern struct ship_data		*ship;


do_UNLOAD_command ()
{
    int		i, found, item_count, recovering_home_planet, alien_index;

    long	n, reb, current_pop;

    struct nampla_data	*alien_home_nampla;


    /* Get the ship. */
    if (! get_ship ())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid ship name in UNLOAD command.\n");
	return;
    }

    if (ship->status == UNDER_CONSTRUCTION)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship is still under construction.\n");
	return;
    }

    if (ship->status == FORCED_JUMP  ||  ship->status == JUMPED_IN_COMBAT)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship jumped during combat and is still in transit.\n");
	return;
    }

    /* Find which planet the ship is at. */
    found = FALSE;
    nampla = nampla_base - 1;
    for (i = 0; i < species->num_namplas; i++)
    {
	++nampla;
	if (ship->x != nampla->x) continue;
	if (ship->y != nampla->y) continue;
	if (ship->z != nampla->z) continue;
	if (ship->pn != nampla->pn) continue;
	found = TRUE;
	break;
    }

    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Ship is not at a named planet.\n");
	return;
    }

    /* Make sure this is not someone else's populated homeworld. */
    for (alien_index = 0; alien_index < galaxy.num_species; alien_index++)
    {
        if (species_number == alien_index + 1) continue;
	if (! data_in_memory[alien_index]) continue;
	
	alien_home_nampla = namp_data[alien_index];

	if (alien_home_nampla->x != nampla->x) continue;
	if (alien_home_nampla->y != nampla->y) continue;
	if (alien_home_nampla->z != nampla->z) continue;
	if (alien_home_nampla->pn != nampla->pn) continue;
	if ((alien_home_nampla->status & POPULATED) == 0) continue;

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You may not colonize someone else's populated home planet!\n");

	return;
    }

    /* Make sure it's not a healthy home planet. */
    recovering_home_planet = FALSE;
    if (nampla->status & HOME_PLANET)
    {
	n = nampla->mi_base + nampla->ma_base + nampla->IUs_to_install +
		nampla->AUs_to_install;
	reb = species->hp_original_base - n;

	if (reb > 0)
	    recovering_home_planet = TRUE;	/* HP was bombed. */
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Installation not allowed on a healthy home planet!\n");
	    return;
	}
    }

    /* Transfer the items from the ship to the planet. */
    log_string ("    ");

    item_count = ship->item_quantity[CU];
    nampla->item_quantity[CU] += item_count;
    log_int (item_count);    log_char (' ');
    log_string (item_abbr[CU]);    if (item_count != 1) log_char ('s');
    ship->item_quantity[CU] = 0;

    item_count = ship->item_quantity[IU];
    nampla->item_quantity[IU] += item_count;
    log_string (", ");
    log_int (item_count);    log_char (' ');
    log_string (item_abbr[IU]);    if (item_count != 1) log_char ('s');
    ship->item_quantity[IU] = 0;

    item_count = ship->item_quantity[AU];
    nampla->item_quantity[AU] += item_count;
    log_string (", and ");
    log_int (item_count);    log_char (' ');
    log_string (item_abbr[AU]);    if (item_count != 1) log_char ('s');
    ship->item_quantity[AU] = 0;

    log_string (" were transferred from ");
    log_string (ship_name (ship));
    log_string (" to PL ");
    log_string (nampla->name);    log_string (". ");

    /* Do the installation. */
    item_count = nampla->item_quantity[CU];
    if (item_count > nampla->item_quantity[IU])
	item_count = nampla->item_quantity[IU];
    if (recovering_home_planet)
    {
	if (item_count > reb) item_count = reb;
	reb -= item_count;
    }

    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[IU] -= item_count;
    nampla->IUs_to_install += item_count;
    current_pop += item_count;

    log_string ("Installation of ");
    log_int (item_count);    log_char (' ');
    log_string (item_abbr[IU]);    if (item_count != 1) log_char ('s');

    item_count = nampla->item_quantity[CU];
    if (item_count > nampla->item_quantity[AU])
	item_count = nampla->item_quantity[AU];
    if (recovering_home_planet)
    {
	if (item_count > reb) item_count = reb;
	reb -= item_count;
    }

    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[AU] -= item_count;
    nampla->AUs_to_install += item_count;

    log_string (" and ");
    log_int (item_count);    log_char (' ');
    log_string (item_abbr[AU]);    if (item_count != 1) log_char ('s');
    log_string (" began on the planet.\n");

    check_population (nampla);
}
