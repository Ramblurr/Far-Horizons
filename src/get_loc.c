
/* This routine will assign values to global variables x, y, z, pn, star
   and nampla. If the location is not a named planet, then nampla will be
   set to NULL. If planet is not specified, pn will be set to zero. If
   location is valid, TRUE will be returned, otherwise FALSE will be
   returned. */


#include "fh.h"


extern int			x, y, z, pn, num_stars, abbr_type;
extern long			value;
extern char			upper_name[32], *input_line_pointer;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base, *nampla;
extern struct star_data		*star_base, *star;


int get_location ()
{
    int		i, n, found, temp_nampla_index, first_try, name_length,
		best_score, next_best_score, best_nampla_index,
		minimum_score;

    char	upper_nampla_name[32], *temp1_ptr, *temp2_ptr;

    struct nampla_data	*temp_nampla;


    /* Check first if x, y, z are specified. */
    nampla = NULL;
    skip_whitespace ();

    if (get_value () == 0)
	goto get_planet;
    x = value;

    if (get_value () == 0) return FALSE;
    y = value;

    if (get_value () == 0) return FALSE;
    z = value;

    if (get_value () == 0)
	pn = 0;
    else
	pn = value;

    if (pn == 0) return TRUE;

    /* Get star. Check if planet exists. */
    found = FALSE;
    star = star_base - 1;
    for (i = 0; i < num_stars; i++)
    {
	++star;

	if (star->x != x) continue;
	if (star->y != y) continue;
	if (star->z != z) continue;

	if (pn > star->num_planets)
	  return FALSE;
	else
	  return TRUE;
    }

    return FALSE;


get_planet:

    /* Save pointers in case of error. */
    temp1_ptr = input_line_pointer;

    get_class_abbr ();

    temp2_ptr = input_line_pointer;

    first_try = TRUE;

again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != PLANET_ID  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get planet name. */
    get_name ();

    /* Search all temp_namplas for name. */
    temp_nampla = nampla_base - 1;
    for (temp_nampla_index = 0; temp_nampla_index < species->num_namplas; temp_nampla_index++)
    {
	++temp_nampla;

	if (temp_nampla->pn == 99) continue;

	/* Make upper case copy of temp_nampla name. */
	for (i = 0; i < 32; i++)
	    upper_nampla_name[i] = toupper(temp_nampla->name[i]);

	/* Compare names. */
	if (strcmp (upper_nampla_name, upper_name) == 0) goto done;
    }

    if (first_try)
    {
	first_try = FALSE;
	goto again;
    }


    /* Possibly a spelling error.  Find the best match that is approximately
	the same. */

    first_try = TRUE;

yet_again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != PLANET_ID  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get planet name. */
    get_name ();

    best_score = -9999;
    next_best_score = -9999;
    for (temp_nampla_index = 0; temp_nampla_index < species->num_namplas; temp_nampla_index++)
    {
	temp_nampla = nampla_base + temp_nampla_index;

	if (temp_nampla->pn == 99) continue;

	/* Make upper case copy of temp_nampla name. */
	for (i = 0; i < 32; i++)
	    upper_nampla_name[i] = toupper(temp_nampla->name[i]);

	/* Compare names. */
	n = agrep_score (upper_nampla_name, upper_name);
	if (n > best_score)
	{
	    best_score = n;	/* Best match so far. */
	    best_nampla_index = temp_nampla_index;
	}
	else if (n > next_best_score)
	    next_best_score = n;
    }

    temp_nampla = nampla_base + best_nampla_index;
    name_length = strlen (temp_nampla->name);
    minimum_score = name_length - ((name_length / 7)  + 1);

    if (best_score < minimum_score		/* Score too low. */
	||  name_length < 5			/* No errors allowed. */
	||  best_score == next_best_score)	/* Another name with equal
							score. */
    {
	if (first_try)
	{
	    first_try = FALSE;
	    goto yet_again;
	}
	else 
	    return FALSE;
    }

done:

    abbr_type = PLANET_ID;

    x = temp_nampla->x;
    y = temp_nampla->y;
    z = temp_nampla->z;
    pn = temp_nampla->pn;
    nampla = temp_nampla;

    return TRUE;
}
