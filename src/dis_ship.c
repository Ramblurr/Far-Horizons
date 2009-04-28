
#include "fh.h"


extern struct species_data	*species;
extern struct nampla_data	*nampla_base;



int disbanded_ship (ship)

struct ship_data	*ship;

{
    int				nampla_index;

    struct nampla_data		*nampla;

    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
    {
	++nampla;

	if (nampla->x != ship->x) continue;
	if (nampla->y != ship->y) continue;
	if (nampla->z != ship->z) continue;
	if (nampla->pn != ship->pn) continue;
	if ((nampla->status & DISBANDED_COLONY) == 0) continue;
	if (ship->type != STARBASE  &&  ship->status == IN_ORBIT) continue;

	/* This ship is either on the surface of a disbanded colony or is
		a starbase orbiting a disbanded colony. */
	return TRUE;
    }

    return FALSE;
}
