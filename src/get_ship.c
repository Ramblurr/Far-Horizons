
/* The following routine will return TRUE and set global variables "ship" and
   "ship_index" if a valid ship designation is found. Otherwise, it will return
   FALSE. The algorithm employed allows minor spelling errors, as well as
   accidental deletion of a ship abbreviation. */


#include "fh.h"


int	correct_spelling_required = FALSE;


extern int			ship_index, abbr_type, abbr_index;
extern char			upper_name[32], *input_line_pointer;
extern struct species_data	*species;
extern struct ship_data		*ship_base, *ship;


int get_ship ()

{
    int		i, n, name_length, best_score, next_best_score, best_ship_index,
		first_try, minimum_score;

    char	upper_ship_name[32], *temp1_ptr, *temp2_ptr;

    struct ship_data	*best_ship = NULL;


    /* Save in case of an error. */
    temp1_ptr = input_line_pointer;

    /* Get ship abbreviation. */
    if (get_class_abbr () == PLANET_ID)
    {
      input_line_pointer = temp1_ptr;
      return FALSE;
    }

    temp2_ptr = input_line_pointer;

    first_try = TRUE;

again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != SHIP_CLASS  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get ship name. */
    name_length = get_name ();

    /* Search all ships for name. */
    ship = ship_base - 1;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	++ship;

	if (ship->pn == 99) continue;

	/* Make upper case copy of ship name. */
	for (i = 0; i < 32; i++) upper_ship_name[i] = toupper(ship->name[i]);

	/* Compare names. */
	if (strcmp (upper_ship_name, upper_name) == 0)
	{
	    abbr_type = SHIP_CLASS;
	    abbr_index = ship->class;
	    correct_spelling_required = FALSE;
	    return TRUE;
	}
    }

    if (first_try)
    {
	first_try = FALSE;
	goto again;
    }

    if (correct_spelling_required)
    {
	correct_spelling_required = FALSE;
	return FALSE;
    }


    /* Possibly a spelling error.  Find the best match that is approximately
	the same. */

    first_try = TRUE;

yet_again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != SHIP_CLASS  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get ship name. */
    name_length = get_name ();

    best_score = -9999;
    next_best_score = -9999;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	ship = ship_base + ship_index;

	if (ship->pn == 99) continue;

	/* Make upper case copy of ship name. */
	for (i = 0; i < 32; i++) upper_ship_name[i] = toupper(ship->name[i]);

	n = agrep_score (upper_ship_name, upper_name);
	if (n > best_score)
	{
	    /* Best match so far. */
	    best_score = n;
	    best_ship = ship;
	    best_ship_index = ship_index;
	}
	else if (n > next_best_score)
	    next_best_score = n;
    }

    if (best_ship == NULL) {
      return FALSE;
    }
    name_length = strlen (best_ship->name);
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
	{
	    correct_spelling_required = FALSE;
	    return FALSE;
	}
    }

    ship = best_ship;
    ship_index = best_ship_index;
    abbr_type = SHIP_CLASS;
    abbr_index = ship->class;
    correct_spelling_required = FALSE;
    return TRUE;
}
