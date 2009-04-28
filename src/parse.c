
#include "fh.h"


int	end_of_file = FALSE;
int	abbr_type, abbr_index, sub_light, tonnage, just_opened_file;
char	input_abbr[256], input_line[256], original_line[256], *input_line_pointer;
char	original_name[32], upper_name[32];
long	value;
FILE	*input_file;


/* Skip white space and comments. */
skip_junk ()
{
again:

    /* Read next line. */
    input_line_pointer = fgets (input_line, 256, input_file);
    if (input_line_pointer == NULL)
    {
	end_of_file = TRUE;
	return;
    }

    if (just_opened_file)	/* Skip mail header, if any. */
    {
	if (*input_line == '\n') goto again;

	just_opened_file = FALSE;

	if (strncmp (input_line, "From ", 5) == 0)  /* This is a mail header. */
	{
	    while (TRUE)
	    {
		input_line_pointer = fgets (input_line, 256, input_file);
		if (input_line_pointer == NULL)
		{
		    end_of_file = TRUE;		/* Weird. */
		    return;
		}
		if (*input_line == '\n') break;    /* End of header. */
	    }

	    goto again;
	}
    }

    strcpy (original_line, input_line);		/* Make a copy. */

    /* Skip white space and comments. */
    while (TRUE)
    {
	switch (*input_line_pointer)
	{
	    case ';':			/* Semi-colon. */
	    case '\n':			/* Newline. */
	    	goto again;

	    case '\t':			/* Tab. */
	    case ' ':			/* Space. */
	    case ',':			/* Comma. */
		++input_line_pointer;
		continue;

	    default:
		return;
	}
    }
}


skip_whitespace ()
{
    while (TRUE)
    {
	switch (*input_line_pointer)
	{
	    case '\t':			/* Tab. */
	    case ' ':			/* Space. */
	    case ',':			/* Comma. */
		++input_line_pointer;
		break;

	    default:
		return;
	}
    }
}



/* The following "get" routines will return 0 if the item found was not
   of the appropriate type, and 1 or greater if an item of the correct
   type was found. */


/* Get a command and return its index. */
int get_command ()
{
    int		i, cmd_n;
    char	c, cmd_s[4];


    skip_junk();
    if (end_of_file)
	return -1;

    c = *input_line_pointer;
    /* Get first three characters of command word. */
    for (i = 0; i < 3; i++)
    {
	if (! isalpha (c)) return 0;
	cmd_s[i] = toupper (c);
	++input_line_pointer;
	c = *input_line_pointer;
    }
    cmd_s[3] = '\0';

    /* Skip everything after third character of command word. */
    while (1)
    {
	switch (c)
	{
	    case '\t':
	    case '\n':
	    case ' ':
	    case ',':
	    case ';':
		goto find_cmd;

	    default:
		++input_line_pointer;
		c = *input_line_pointer;
	}
    }

  find_cmd:

    /* Find corresponding string in list. */
    cmd_n = UNKNOWN;
    for (i = 1; i < NUM_COMMANDS; i++)
    {
	if (strcmp(cmd_s, command_abbr[i]) == 0)
	{
	    cmd_n = i;
	    break;
	}
    }

    return cmd_n;
}


/* Get a class abbreviation and return TECH_ID, ITEM_CLASS, SHIP_CLASS,
   PLANET_ID, SPECIES_ID or ALLIANCE_ID as appropriate, or UNKNOWN if it
   cannot be identified. Also, set "abbr_type" to this value. If it is
   TECH_ID, ITEM_CLASS or SHIP_CLASS, "abbr_index" will contain the
   abbreviation index. If it is a ship, "tonnage" will contain tonnage/10,000,
   and "sub_light" will be TRUE or FALSE. (Tonnage value returned is based
   ONLY on abbreviation.) */

int get_class_abbr ()
{
    int		i;

    char	*digit_start;


    skip_whitespace();

    abbr_type = UNKNOWN;

    if (! isalnum (*input_line_pointer)) return UNKNOWN;
    input_abbr[0] = toupper(*input_line_pointer);
    ++input_line_pointer;

    if (! isalnum (*input_line_pointer)) return UNKNOWN;
    input_abbr[1] = toupper(*input_line_pointer);
    ++input_line_pointer;

    input_abbr[2] = '\0';

    /* Check for IDs that are followed by one or more digits or letters. */
    i = 2;
    digit_start = input_line_pointer;
    while (isalnum (*input_line_pointer))
    {
	input_abbr[i++] = *input_line_pointer++;
	input_abbr[i] = '\0';
    }

    /* Check tech ID. */
    for (i = 0; i < 6; i++)
    {
	if (strcmp(input_abbr, tech_abbr[i]) == 0)
	{
	    abbr_index = i;
	    abbr_type = TECH_ID;
	    return abbr_type;
	}
    }

    /* Check item abbreviations. */
    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (strcmp(input_abbr, item_abbr[i]) == 0)
	{
	    abbr_index = i;
	    abbr_type = ITEM_CLASS;
	    return abbr_type;
	}
    }

    /* Check ship abbreviations. */
    for (i = 0; i < NUM_SHIP_CLASSES; i++)
    {
	if (strncmp(input_abbr, ship_abbr[i], 2) == 0)
	{
	    input_line_pointer = digit_start;
	    abbr_index = i;
	    tonnage = ship_tonnage[i];
	    if (i == TR)
	    {
		tonnage = 0;
		while (isdigit(*input_line_pointer))
		{
		    tonnage = (10 * tonnage) + (*input_line_pointer - '0');
		    ++input_line_pointer;
		}
	    }

	    if (toupper(*input_line_pointer) == 'S')
	    {
		sub_light = TRUE;
		++input_line_pointer;
	    }
	    else
		sub_light = FALSE;

	    if (isalnum (*input_line_pointer)) break;	/* Garbage. */

	    abbr_type = SHIP_CLASS;
	    return abbr_type;
	}
    }

    /* Check for planet name. */
    if (strcmp(input_abbr, "PL") == 0)
    {
	abbr_type = PLANET_ID;
	return abbr_type;
    }

    /* Check for species name. */
    if (strcmp(input_abbr, "SP") == 0)
    {
	abbr_type = SPECIES_ID;
	return abbr_type;
    }

    abbr_type = UNKNOWN;
    return abbr_type;
}


/* Get a name and copy original version to "original_name" and upper
   case version to "upper_name". Return length of name. */
int get_name ()
{
    int		name_length;

    char	c;


    skip_whitespace();

    name_length = 0;
    while (TRUE)
    {
	c = *input_line_pointer;
	if (c == ';') break;
	++input_line_pointer;
	if (c == ','  ||  c == '\t'  ||  c == '\n')
	    break;
	if (name_length < 31)
	{
	    original_name[name_length] = c;
	    upper_name[name_length] = toupper(c);
	    ++name_length;
	}
    }

    /* Remove any final spaces in name. */
    while (name_length > 0)
    {
	c = original_name[name_length-1];
	if (c != ' ')
	    break;
	--name_length;
    }

    /* Terminate strings. */
    original_name[name_length] = '\0';
    upper_name[name_length] = '\0';

    return name_length;
}


/* Read a long decimal and place its value in 'value'. */
int get_value ()
{
    int		n;


    skip_whitespace();

    n = sscanf (input_line_pointer, "%ld", &value);
    if (n != 1) return 0;	/* Not a numeric value. */

    /* Skip numeric string. */
    ++input_line_pointer;	/* Skip first sign or digit. */
    while (isdigit(*input_line_pointer)) ++input_line_pointer;

    return 1;
}



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
