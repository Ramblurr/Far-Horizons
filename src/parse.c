
#include "fh.h"


/* The following routine will check that the next argument in the current
   command line is followed by a comma or tab.  If not present, it will
   try to insert a comma in the proper position.  This routine should
   be called only AFTER an error has been detected. */

fix_separator ()

{
    int		n, first_class, fix_made, num_commas;

    char	c, *temp_ptr, *temp2_ptr, *first_comma;


    skip_whitespace ();

    if (isdigit (*input_line_pointer)) return;	/* Nothing can be done. */

    if (strchr (input_line_pointer, ' ') == NULL) return; /* Ditto. */

    fix_made = FALSE;

    /* Look for a ship, planet, or species abbreviation after the first one.
	If it is preceeded by a space, convert the space to a comma. */
    temp_ptr = input_line_pointer;
    first_class = get_class_abbr ();	/* Skip first one but remember what it was. */
    while (1)
    {
	skip_whitespace ();
	temp2_ptr = input_line_pointer - 1;
	if (*input_line_pointer == '\n') break;
	if (*input_line_pointer == ';') break;

	/* The following is to prevent an infinite loop. */
	if (! isalnum (*input_line_pointer))
	{
	    ++input_line_pointer;
	    continue;
	}

	n = get_class_abbr ();
	if (n == SHIP_CLASS  ||  n == PLANET_ID  ||  n == SPECIES_ID)
	{
	    /* Convert space preceeding abbreviation to a comma. */
	    if (*temp2_ptr == ' ')
	    {
		*temp2_ptr = ',';
		fix_made = TRUE;
	    }
	}
    }
    input_line_pointer = temp_ptr;

    if (fix_made) return;

    /* Look for a space followed by a digit. If found, convert the space
	to a comma.  If exactly two or four commas are added, re-convert
	the first one back to a space; e.g. Jump TR1 Seeker,7,99,99,99 or
	Build TR1 Seeker,7,50. */
    num_commas = 0;
    while (1)
    {
	c = *temp_ptr++;

	if (c == '\n') break;
	if (c == ';') break;

	if (c != ' ') continue;
	if (isdigit (*temp_ptr))
	{
	    --temp_ptr;		/* Convert space to a comma. */
	    *temp_ptr = ',';
	    if (num_commas++ == 0) first_comma = temp_ptr;
	    ++temp_ptr;
	    fix_made = TRUE;
	}
    }

    if (fix_made)
    {
	if (num_commas == 2  ||  num_commas == 4) *first_comma = ' ';
	return;
    }

    /* Now's the time for wild guesses. */
    temp_ptr = input_line_pointer;

    /* If first word is a valid abbreviation, put a comma after the
	second word. */
    if (first_class == SHIP_CLASS  ||  first_class == PLANET_ID  ||  first_class == SPECIES_ID)
    {
	temp_ptr = strchr (temp_ptr, ' ') + 1;
	temp_ptr = strchr (temp_ptr, ' ');
	if (temp_ptr != NULL) *temp_ptr = ',';
	return;
    }

    /* First word is not a valid abbreviation.  Put a comma after it. */
    temp_ptr = strchr (temp_ptr, ' ');
    if (temp_ptr != NULL) *temp_ptr = ',';
}
