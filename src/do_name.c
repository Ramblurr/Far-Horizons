
#include "fh.h"



extern int			x, y, z, pn, num_stars, nampla_index,
				species_index;
extern char			input_line[256], upper_name[32],
				original_name[32], *input_line_pointer;
extern FILE			*log_file;
extern struct species_data	*species;
extern struct star_data		*star;
extern struct planet_data	*planet_base;
extern struct nampla_data	*nampla_base, *nampla;


do_NAME_command ()
{
    int		i, found, name_length, unused_nampla_available;

    char	upper_nampla_name[32], *original_line_pointer;

    struct planet_data		*planet;
    struct nampla_data		*unused_nampla;


    /* Get x y z coordinates. */
    found = get_location ();
    if (! found  ||  nampla != NULL  ||  pn == 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid coordinates in NAME command.\n");
	return;
    }

    /* Get planet abbreviation. */
    skip_whitespace ();
    original_line_pointer = input_line_pointer;
    if (get_class_abbr () != PLANET_ID)
    {
	/* Check if PL was mispelled (i.e, "PT" or "PN"). Otherwise
		assume that it was accidentally omitted. */
	if (tolower(*original_line_pointer) != 'p'
		||  isalnum (*(original_line_pointer+2)))
			input_line_pointer = original_line_pointer;
    }

    /* Get planet name. */
    name_length = get_name ();
    if (name_length < 1)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid planet name in NAME command.\n");
	return;
    }

    /* Search existing namplas for name and location. */
    found = FALSE;
    unused_nampla_available = FALSE;
    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
    {
	++nampla;

	if (nampla->pn == 99)
	{
	    /* We can re-use this nampla rather than append a new one. */
	    unused_nampla = nampla;
	    unused_nampla_available = TRUE;
	    continue;
	}

	/* Check if a named planet already exists at this location. */
	if (nampla->x == x  &&  nampla->y == y  &&  nampla->z == z
		&&  nampla->pn == pn)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! The planet at these coordinates already has a name.\n");
	    return;
	}

	/* Make upper case copy of nampla name. */
	for (i = 0; i < 32; i++)
	    upper_nampla_name[i] = toupper(nampla->name[i]);

	/* Compare names. */
	if (strcmp (upper_nampla_name, upper_name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Planet in NAME command already exists.\n");
	return;
    }

    /* Add new nampla to database for this species. */
    if (unused_nampla_available)
	nampla = unused_nampla;
    else
    {
	++num_new_namplas[species_index];
	if (num_new_namplas[species_index] > NUM_EXTRA_NAMPLAS)
	{
	    fprintf (stderr, "\n\n\tInsufficient memory for new planet name:\n");
	    fprintf (stderr, "\n\t%s\n", input_line);
	    exit (-1);
	}
	nampla = nampla_base + species->num_namplas;
	species->num_namplas += 1;
	delete_nampla (nampla);		/* Set everything to zero. */
    }

    /* Initialize new nampla. */
    strcpy (nampla->name, original_name);
    nampla->x = x;
    nampla->y = y;
    nampla->z = z;
    nampla->pn = pn;
    nampla->status = COLONY;
    nampla->planet_index = star->planet_index + pn - 1;
    planet = planet_base + (long) nampla->planet_index;
    nampla->message = planet->message;

	/* Everything else was set to zero in above call to 'delete_nampla'. */

    /* Mark sector as having been visited. */
    star_visited (x, y, z);

    /* Log result. */
    log_string ("    Named PL ");  log_string (nampla->name);
    log_string (" at ");  log_int (nampla->x);  log_char (' ');
    log_int (nampla->y);  log_char (' ');  log_int (nampla->z);
    log_string (", planet #");  log_int (nampla->pn);
    log_string (".\n");
}
