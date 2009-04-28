
#include "fh.h"


extern int			planet_data_modified, first_pass;
extern long			value;
extern char			input_line[256];
extern FILE			*log_file;
extern struct planet_data	*planet_base, *planet;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base, *nampla;


do_TERRAFORM_command ()
{
    int				i, j, ls_needed, num_plants, got_required_gas,
				correct_percentage;

    struct planet_data		*home_planet, *colony_planet;


    /* Get number of TPs to use. */
    if (get_value ())
	num_plants = value;
    else
	num_plants = 0;

    /* Get planet where terraforming is to be done. */
    if (! get_location ()  ||  nampla == NULL)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in TERRAFORM command.\n");
	return;
    }

    /* Make sure planet is not a home planet. */
    if (nampla->status & HOME_PLANET)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Terraforming may not be done on a home planet.\n");
	return;
    }

    /* Find out how many terraforming plants are needed. */
    colony_planet = planet_base + (long) nampla->planet_index;
    home_planet = planet_base + (long) nampla_base->planet_index;

    ls_needed = life_support_needed (species, home_planet, colony_planet);

    if (ls_needed == 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Colony does not need to be terraformed.\n");
	return;
    }

    if (num_plants == 0) num_plants = nampla->item_quantity[TP];
    if (num_plants > ls_needed) num_plants = ls_needed;
    num_plants = num_plants / 3;
    num_plants *= 3;

    if (num_plants < 3)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! At least three TPs are needed to terraform.\n");
	return;
    }

    if (num_plants > nampla->item_quantity[TP])
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! PL %s doesn't have that many TPs!\n",
	    nampla->name);
	return;
    }

    /* Log results. */
    log_string ("    PL ");  log_string (nampla->name);
    log_string (" was terraformed using ");  log_int (num_plants);
    log_string (" Terraforming Unit");
    if (num_plants != 1) log_char ('s');
    log_string (".\n");

    nampla->item_quantity[TP] -= num_plants;
    planet_data_modified = TRUE;

    /* Terraform the planet. */
    while (num_plants > 1)
    {
	got_required_gas = FALSE;
	correct_percentage = FALSE;
	for (j = 0; j < 4; j++)	/* Check gases on planet. */
	{
	    for (i = 0; i < 6; i++)	/* Compare with poisonous gases. */
	    {
		if (colony_planet->gas[j] == species->required_gas)
		{
		    got_required_gas = j + 1;

		    if (colony_planet->gas_percent[j] >= species->required_gas_min
		      && colony_planet->gas_percent[j] <= species->required_gas_max)
			correct_percentage = TRUE;
		}

		if (species->poison_gas[i] == colony_planet->gas[j])
		{
		    colony_planet->gas[j] = 0;
		    colony_planet->gas_percent[j] = 0;

		    /* Make sure percentages add up to 100%. */
		    fix_gases (colony_planet);

		    goto next_change;
		}
	    }
	}

	if (got_required_gas && correct_percentage) goto do_temp;

	j = 0;	/* If all 4 gases are neutral gases, replace the first one. */

	if (got_required_gas)
	    j = got_required_gas - 1;
	else
	{
	    for (i = 0; i < 4; i++)
	    {
		if (colony_planet->gas_percent[i] == 0)
		{
		    j = i;
		    break;
		}
	    }
	}

	colony_planet->gas[j] = species->required_gas;
	i = species->required_gas_max - species->required_gas_min;
	colony_planet->gas_percent[j] = species->required_gas_min + rnd (i);

	/* Make sure percentages add up to 100%. */
	fix_gases (colony_planet);

	goto next_change;

do_temp:

	if (colony_planet->temperature_class != home_planet->temperature_class)
	{
	    if (colony_planet->temperature_class > home_planet->temperature_class)
		--colony_planet->temperature_class;
	    else
		++colony_planet->temperature_class;

	    goto next_change;
	}

	if (colony_planet->pressure_class != home_planet->pressure_class)
	{
	    if (colony_planet->pressure_class > home_planet->pressure_class)
		--colony_planet->pressure_class;
	    else
		++colony_planet->pressure_class;
	}

next_change:

	num_plants -= 3;
    }
}


fix_gases (pl)

struct planet_data	*pl;

{
    int		i, j, total, left, add_neutral;

    long	n;


    total = 0;
    for (i = 0; i < 4; i++) total += pl->gas_percent[i];

    if (total == 100) return;

    left = 100 - total;

    /* If we have at least one gas that is not the required gas, then we
	simply need to adjust existing gases. Otherwise, we have to add a
	neutral gas. */
    add_neutral = TRUE;
    for (i = 0; i < 4; i++)
    {
	if (pl->gas_percent[i] == 0) continue;

	if (pl->gas[i] == species->required_gas) continue;

	add_neutral = FALSE;

	break;
    }

    if (add_neutral) goto add_neutral_gas;

    /* Randomly modify existing non-required gases until total percentage
	is exactly 100. */
    while (left != 0)
    {
	i = rnd(4) - 1;

	if (pl->gas_percent[i] == 0) continue;

	if (pl->gas[i] == species->required_gas) continue;

	if (left > 0)
	{
	    if (left > 2)
		j = rnd(left);
	    else
		j = left;

	    pl->gas_percent[i] += j;
	    left -= j;
	}
	else
	{
	    if (-left > 2)
		j = rnd (-left);
	    else
		j = -left;

	    if (j < pl->gas_percent[i])
	    {
		pl->gas_percent[i] -= j;
		left += j;
	    }
	}
    }

    return;

add_neutral_gas:

    /* If we reach this point, there is either no atmosphere or it contains
	only the required gas.  In either case, add a random neutral gas. */
    for (i = 0; i < 4; i++)
    {
	if (pl->gas_percent[i] > 0) continue;

	j = rnd(6) - 1;
	pl->gas[i] = species->neutral_gas[j];
	pl->gas_percent[i] = left;

	break;
    }
}
