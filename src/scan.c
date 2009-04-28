
#include "fh.h"


int				print_LSN = TRUE;

extern int			num_stars;
extern FILE			*log_file;
extern struct star_data		*star_base;
extern struct planet_data	*planet_base;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base;


scan (x, y, z)

char	x, y, z;

{
    int		i, j, k, n, found, num_gases, ls_needed;

    char	filename[32];

    struct star_data	*star;
    struct planet_data	*planet, *home_planet;
    struct nampla_data	*home_nampla;


    /* Find star. */
    star = star_base;
    found = FALSE;
    for (i = 0; i < num_stars; i++)
    {
	if (star->x == x  &&  star->y == y  &&  star->z == z)
	{
	    found = TRUE;
	    break;
	}
	++star;
    }

    if (! found)
    {
	fprintf (log_file,
	    "Scan Report: There is no star system at x = %d, y = %d, z = %d.\n",
				x, y, z);
	return;
    }

    /* Print data for star, */
    fprintf (log_file, "Coordinates:\tx = %d\ty = %d\tz = %d", x, y, z);
    fprintf (log_file, "\tstellar type = %c%c%c", type_char[star->type],
    	color_char[star->color], size_char[star->size]);

    fprintf (log_file, "   %d planets.\n\n", star->num_planets);

    if (star->worm_here)
	fprintf (log_file,
	    "This star system is the terminus of a natural wormhole.\n\n");

    /* Print header. */
    fprintf (log_file, "               Temp  Press Mining\n");
    fprintf (log_file, "  #  Dia  Grav Class Class  Diff  LSN  Atmosphere\n");
    fprintf (log_file, " ---------------------------------------------------------------------\n");

    /* Check for nova. */
    if (star->num_planets == 0)
    {
	fprintf (log_file, "\n\tThis star is a nova remnant. Any planets it may have once\n");
	fprintf (log_file, "\thad have been blown away.\n\n");
	return;
    }

    /* Print data for each planet. */
    planet = planet_base + (long) star->planet_index;
    if (print_LSN)
    {
	home_nampla = nampla_base;
        home_planet = planet_base + (long) home_nampla->planet_index;
    }

    for (i = 1; i <= star->num_planets; i++)
    {
	/* Get life support tech level needed. */
	if (print_LSN)
	    ls_needed = life_support_needed (species, home_planet, planet);
	else
	    ls_needed = 99;

	fprintf (log_file, "  %d  %3d  %d.%02d  %2d    %2d    %d.%02d %4d  ",
	i,
    	planet->diameter,
    	planet->gravity/100,
    	planet->gravity%100,
    	planet->temperature_class,
    	planet->pressure_class,
    	planet->mining_difficulty/100,
    	planet->mining_difficulty%100,
	ls_needed);

	num_gases = 0;
	for (n = 0; n < 4; n++)
	{
	    if (planet->gas_percent[n] > 0)
	    {
		if (num_gases > 0) fprintf (log_file, ",");
		fprintf (log_file, "%s(%d%%)", gas_string[planet->gas[n]],
			planet->gas_percent[n]);
		++num_gases;
	    }
	}

	if (num_gases == 0) fprintf (log_file, "No atmosphere");

	fprintf (log_file, "\n");
	++planet;
    }

    if (star->message)
    {
	/* There is a message that must be logged whenever this star
		system is scanned. */
	sprintf (filename, "message%ld.txt\0", star->message);
	log_message (filename);
    }

    return;
}
