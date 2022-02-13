
#include "fh.h"




extern struct galaxy_data	galaxy;



/* The following two routines will delete a ship or nampla record. */


delete_nampla (nampla)

struct nampla_data	*nampla;

{
	int	i;

	char	*cp;


	/* Set all bytes of record to zero. */
	cp = (char *) nampla;
	for (i = 0; i < sizeof (struct nampla_data); i++)
	    *cp++ = 0;

	nampla->pn = 99;
	strcpy (nampla->name, "Unused");
}







/* Get life support tech level needed. */

int life_support_needed (species, home, colony)

struct species_data	*species;
struct planet_data	*home, *colony;

{
    int	i, j, k, ls_needed;


    i = colony->temperature_class - home->temperature_class;
    if (i < 0) i = -i;
    ls_needed = 3 * i;		/* Temperature class. */

    i = colony->pressure_class - home->pressure_class;
    if (i < 0) i = -i;
    ls_needed += 3 * i;		/* Pressure class. */

    /* Check gases. Assume required gas is NOT present. */
    ls_needed += 3;
    for (j = 0; j < 4; j++)	/* Check gases on planet. */
    {
      if (colony->gas_percent[j] == 0) continue;
      for (i = 0; i < 6; i++)	/* Compare with poisonous gases. */
      {
    	if (species->poison_gas[i] == colony->gas[j])
    	    ls_needed += 3;
      }
      if (colony->gas[j] == species->required_gas)
      {
        if (colony->gas_percent[j] >= species->required_gas_min
         && colony->gas_percent[j] <= species->required_gas_max)
    	ls_needed -= 3;
      }
    }

    return ls_needed;
}



check_high_tech_items (tech, old_tech_level, new_tech_level)

int	tech, old_tech_level, new_tech_level;

{
    int		i;


    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (item_critical_tech[i] != tech) continue;
	if (new_tech_level < item_tech_requirment[i]) continue;
	if (old_tech_level >= item_tech_requirment[i]) continue;

	log_string ("  You now have the technology to build ");
	log_string (item_name[i]);
	log_string ("s.\n");
    }

    /* Check for high tech abilities that are not associated with specific
	items. */
    if (tech == MA  &&  old_tech_level < 25  &&  new_tech_level >= 25)
	log_string ("  You now have the technology to do interspecies construction.\n");
}

extern int			num_stars, species_number, star_data_modified;
extern struct star_data		*star_base;


/* The following routine will check if coordinates x-y-z contain a star and,
   if so, will set the appropriate bit in the "visited_by" variable for the
   star. If the star exists, TRUE will be returned; otherwise, FALSE will
   be returned. */

int star_visited (x, y, z)

int	x, y, z;

{
    int		i, found, species_array_index, species_bit_number;

    long	species_bit_mask;

    struct star_data	*star;


    /* Get array index and bit mask. */
    species_array_index = (species_number - 1) / 32;
    species_bit_number = (species_number - 1) % 32;
    species_bit_mask = 1 << species_bit_number;

    found = FALSE;

    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	if (x != star->x) continue;
	if (y != star->y) continue;
	if (z != star->z) continue;

	found = TRUE;

	/* Check if bit is already set. */
	if (star->visited_by[species_array_index] & species_bit_mask) break;

	/* Set the appropriate bit. */
	star->visited_by[species_array_index] |= species_bit_mask;
	star_data_modified = TRUE;
	break;
    }

    return found;
}
