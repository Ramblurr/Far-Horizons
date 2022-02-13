
#include "fh.h"




extern struct galaxy_data	galaxy;

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


