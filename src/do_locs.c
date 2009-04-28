
/* This routine will create the "loc" array based on current species' data. */


#include "fh.h"


extern int			species_number, species_index, num_locs;

extern struct galaxy_data	galaxy;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base, *nampla;
extern struct ship_data		*ship_base, *ship;
extern struct sp_loc_data	loc[MAX_LOCATIONS];


do_locations ()
{
    int		i;


    num_locs = 0;

    for (species_number = 1; species_number <= galaxy.num_species; species_number++)
    {
	if (! data_in_memory[species_number - 1]) continue;

	species = &spec_data[species_number - 1];
	nampla_base = namp_data[species_number - 1];
	ship_base = ship_data[species_number - 1];

	nampla = nampla_base - 1;
	for (i = 0; i < species->num_namplas; i++)
	{
	    ++nampla;

	    if (nampla->pn == 99) continue;

	    if (nampla->status & POPULATED)
		add_location (nampla->x, nampla->y, nampla->z);
	}

	ship = ship_base - 1;
	for (i = 0; i < species->num_ships; i++)
	{
	    ++ship;

	    if (ship->pn == 99) continue;
	    if (ship->status == FORCED_JUMP  ||  ship->status == JUMPED_IN_COMBAT) continue;

	    add_location (ship->x, ship->y, ship->z);
	}
    }
}




add_location (x, y, z)

char	x, y, z;

{
    int		i;


    for (i = 0; i < num_locs; i++)
    {
	if (loc[i].x != x) continue;
	if (loc[i].y != y) continue;
	if (loc[i].z != z) continue;
	if (loc[i].s != species_number) continue;

	return;		/* This location is already in list for this species. */
    }

    /* Add new location to the list. */
    loc[num_locs].x = x;
    loc[num_locs].y = y;
    loc[num_locs].z = z;
    loc[num_locs].s = species_number;

    ++num_locs;
    if (num_locs < MAX_LOCATIONS)
	return;

    fprintf (stderr, "\n\n\tInternal error. Overflow of 'loc' arrays!\n\n");
    exit (-1);
}
