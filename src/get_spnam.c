

/* This routine will get a species name and return TRUE if found and if
   it is valid.  It will also set global values "g_species_number" and
   "g_species_name". The algorithm employed allows minor spelling errors,
   as well as accidental deletion of the SP abbreviation. */


#include "fh.h"


int			g_spec_number;
char			g_spec_name[32];

extern int			abbr_type;
extern char			upper_name[32], *input_line_pointer;
extern struct galaxy_data	galaxy;


int get_species_name ()
{
    int		i, n, species_index, best_score, best_species_index,
		next_best_score, first_try, minimum_score, name_length;

    char	sp_name[32], *temp1_ptr, *temp2_ptr;

    struct species_data		*sp;


    g_spec_number = 0;

    /* Save pointers in case of error. */
    temp1_ptr = input_line_pointer;

    get_class_abbr ();

    temp2_ptr = input_line_pointer;

    first_try = TRUE;

again:

    input_line_pointer = temp2_ptr;

    if (abbr_type != SPECIES_ID  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get species name. */
    get_name();

    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (! data_in_memory[species_index]) continue;

	sp = &spec_data[species_index];

	/* Copy name to g_spec_name and convert it to upper case. */
	for (i = 0; i < 31; i++)
	{
	    g_spec_name[i] = sp->name[i];
	    sp_name[i] = toupper(g_spec_name[i]);
	}

	if (strcmp (sp_name, upper_name) == 0)
	{
	    g_spec_number = species_index + 1;
	    abbr_type = SPECIES_ID;
	    return TRUE;
	}
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

    if (abbr_type != SPECIES_ID  &&  ! first_try)
    {
	/* Assume abbreviation was accidentally omitted. */
	input_line_pointer = temp1_ptr;
    }

    /* Get species name. */
    get_name();

    best_score = -9999;
    next_best_score = -9999;
    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (! data_in_memory[species_index]) continue;

	sp = &spec_data[species_index];

	/* Convert name to upper case. */
	for (i = 0; i < 31; i++) sp_name[i] = toupper(sp->name[i]);

	n = agrep_score (sp_name, upper_name);
	if (n > best_score)
	{
	    /* Best match so far. */
	    best_score = n;
	    best_species_index = species_index;
	}
	else if (n > next_best_score)
	    next_best_score = n;
    }

    sp = &spec_data[best_species_index];
    name_length = strlen (sp->name);
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

    /* Copy name to g_spec_name. */
    for (i = 0; i < 31; i++) g_spec_name[i] = sp->name[i];
    g_spec_number = best_species_index + 1;
    abbr_type = SPECIES_ID;
    return TRUE;
}
