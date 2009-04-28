
#include "fh.h"


extern int			abbr_index, species_number;
extern char			input_line[256];
extern long			value;
extern FILE			*log_file;
extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct nampla_data	*nampla;


do_INSTALL_command ()
{
    int		i, item_class, item_count, num_available, do_all_units,
		recovering_home_planet, alien_index;

    long	n, current_pop, reb;

    struct nampla_data	*alien_home_nampla;


    /* Get number of items to install. */
    if (get_value ())
	do_all_units = FALSE;
    else
    {
	do_all_units = TRUE;
	item_count = 0;
	item_class = IU;
	goto get_planet;
    }

    /* Make sure value is meaningful. */
    if (value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid item count in INSTALL command.\n");
	return;
    }
    item_count = value;

    /* Get class of item. */
    item_class = get_class_abbr ();
    if (item_class != ITEM_CLASS  ||  (abbr_index != IU  &&  abbr_index != AU))
    {
	/* Players sometimes accidentally use "MI" for "IU"
		or "MA" for "AU". */
	if (item_class == TECH_ID  &&  abbr_index == MI)
	    abbr_index = IU;
	else if (item_class == TECH_ID  &&  abbr_index == MA)
	    abbr_index = AU;
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Invalid item class!\n");
	    return;
	}
    }
    item_class = abbr_index;

get_planet:

    /* Get planet where items are to be installed. */
    if (! get_location ()  ||  nampla == NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in INSTALL command.\n");
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

check_items:

    /* Make sure planet has the specified items. */
    if (item_count == 0)
    {
	item_count = nampla->item_quantity[item_class];

	if (nampla->item_quantity[CU] < item_count)
		item_count = nampla->item_quantity[CU];

	if (item_count == 0)
	{
	    if (do_all_units)
	    {
		item_count = 0;
		item_class = AU;
		do_all_units = FALSE;
		goto check_items;
	    }
	    else
		 return;
	}
    }
    else if (nampla->item_quantity[item_class] < item_count)
    {
	fprintf (log_file, "! WARNING: %s", input_line);
	fprintf (log_file,
	    "! Planet does not have %d %ss. Substituting 0 for %d!\n",
	    item_count, item_abbr[item_class], item_count);
	item_count = 0;
	goto check_items;
    }

    if (recovering_home_planet)
    {
	if (item_count > reb) item_count = reb;
	reb -= item_count;
    }

    /* Make sure planet has enough colonist units. */
    num_available = nampla->item_quantity[CU];
    if (num_available < item_count)
    {
	if (num_available > 0)
	{
	    fprintf (log_file, "! WARNING: %s", input_line);
	    fprintf (log_file, "! Planet does not have %d CUs. Substituting %d for %d!\n",
		item_count, num_available, item_count);
	    item_count = num_available;
	}
	else
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! No colonist units on planet for installation.\n");
	    return;
	}
    }

    /* Start the installation. */
    nampla->item_quantity[CU] -= item_count;
    nampla->item_quantity[item_class] -= item_count;

    if (item_class == IU)
	nampla->IUs_to_install += item_count;
    else
	nampla->AUs_to_install += item_count;

    /* Log result. */
    log_string ("    Installation of ");    log_int (item_count);
    log_char (' ');    log_string (item_name[item_class]);
    if (item_count != 1) log_char ('s');
    log_string (" began on PL ");
    log_string (nampla->name);
    log_string (".\n");

    if (do_all_units)
    {
	item_count = 0;
	item_class = AU;
	do_all_units = FALSE;
	goto check_items;
    }

    check_population (nampla);
}
