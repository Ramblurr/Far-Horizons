
#include "fh.h"
#include "combat.h"


int	num_combat_options;
char	combat_option[1000], combat_location[1000];


consolidate_option (option, location)

char	option, location;

{
    int		i;


    /* Only attack options go in list. */
    if (option < DEEP_SPACE_FIGHT)
	return;

    /* Make sure pre-requisites are already in the list. Bombardment, and
	germ warfare must follow a successful planet attack. */
    if (option > PLANET_ATTACK)
	consolidate_option (PLANET_ATTACK, location);

    /* Check if option and location are already in list. */
    for (i = 0; i < num_combat_options; i++)
    {
	if (option == combat_option[i]  &&  location == combat_location[i])
	    return;
    }

    /* Add new option to list. */
    combat_option[num_combat_options] = option;
    combat_location[num_combat_options] = location;
    ++num_combat_options;
}
