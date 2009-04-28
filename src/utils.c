
#include "fh.h"

/* This routine will return a random int between 1 and max, inclusive.
   It uses the so-called "Algorithm M" method, which is a combination
   of the congruential and shift-register methods. */

unsigned long	last_random = 1924085713L;	/* Random seed. */

int rnd (max)

unsigned int	max;

{
	unsigned long	a, b, c, cong_result, shift_result;

	/* For congruential method, multiply previous value by the
	   prime number 16417. */
	a = last_random;
	b = last_random << 5;
	c = last_random << 14;
	cong_result = a + b + c;	/* Effectively multiply by 16417. */

	/* For shift-register method, use shift-right 15 and shift-left 17
	   with no-carry addition (i.e., exclusive-or). */
	a = last_random >> 15;
	shift_result = a ^ last_random;
	a = shift_result << 17;
	shift_result ^= a;

	last_random = cong_result ^ shift_result;

	a = last_random & 0x0000FFFF;

	return (int) ((a * (long) max) >> 16) + 1L;
}



/* Routine "get_species_data" will read in data files for all species,
	"save_species_data" will write all data that has been modified, and
	"free_species_data" will free memory used for all species data. */

/* Additional memory must be allocated for routines that build ships or
   name planets. Here are the default 'extras', which may be changed, if
   necessary, by the main program. */

long	extra_namplas = NUM_EXTRA_NAMPLAS;
long	extra_ships = NUM_EXTRA_SHIPS;

extern struct galaxy_data	galaxy;


get_species_data ()

{
    int		species_fd, species_index;

    long	n, num_bytes;

    char	filename[16];

    struct species_data		*sp;


    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	data_modified[species_index] = FALSE;

	sp = &spec_data[species_index];

	/* Open the species data file. */
	sprintf (filename, "sp%02d.dat\0", species_index + 1);
	species_fd = open (filename, 0);
	if (species_fd < 0)
	{
	    sp->pn = 0;	/* Extinct! */
	    data_in_memory[species_index] = FALSE;
	    continue;
	}

	/* Read in species data. */
	num_bytes = read (species_fd, sp, sizeof(struct species_data));
	if (num_bytes != sizeof(struct species_data))
	{
	    fprintf (stderr, "\n\tCannot read species record in file '%s'!\n\n",
	    	filename);
	    exit (-1);
	}

	/* Allocate enough memory for all namplas. */
	num_bytes = (sp->num_namplas + extra_namplas) * sizeof (struct nampla_data);
	namp_data[species_index] = (struct nampla_data *) malloc (num_bytes);
	if (namp_data[species_index] == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for nampla data!\n\n");
	    exit (-1);
	}

	/* Read it all into memory. */
	num_bytes = (long) sp->num_namplas * sizeof (struct nampla_data);
	n = read (species_fd, namp_data[species_index], num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\nCannot read nampla data into memory!\n\n");
	    exit (-1);
	}

	/* Allocate enough memory for all ships. */
 	num_bytes = (sp->num_ships + extra_ships) * sizeof (struct ship_data);
	ship_data[species_index] = (struct ship_data *) malloc (num_bytes);
	if (ship_data[species_index] == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for ship data!\n\n");
	    exit (-1);
	}

	if (sp->num_ships > 0)
	{
	    /* Read it all into memory. */
	    num_bytes = (long) sp->num_ships * sizeof (struct ship_data);
	    n = read (species_fd, ship_data[species_index], num_bytes);
	    if (n != num_bytes)
	    {
	        fprintf (stderr, "\nCannot read ship data into memory!\n\n");
	        exit (-1);
	    }
	}

	close (species_fd);

	data_in_memory[species_index] = TRUE;
	num_new_namplas[species_index] = 0;
	num_new_ships[species_index] = 0;
    }
}



save_species_data ()

{
    int		species_fd, species_index;

    long	n, num_bytes;

    char	filename[16];

    struct species_data		*sp;


    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (! data_modified[species_index]) continue;

	sp = &spec_data[species_index];

	/* Open the species data file. */
	sprintf (filename, "sp%02d.dat\0", species_index + 1);
	species_fd = creat (filename, 0600);
	if (species_fd < 0)
	{
	    fprintf (stderr, "\n  Cannot create new version of file '%s'!\n",
				filename);
	    exit (-1);
	}

	/* Write species data. */
	num_bytes = write (species_fd, sp, sizeof(struct species_data));
	if (num_bytes != sizeof(struct species_data))
	{
	    fprintf (stderr, "\n\tCannot write species record to file '%s'!\n\n",
	    	filename);
	    exit (-1);
	}

	/* Write nampla data. */
	num_bytes = sp->num_namplas * sizeof (struct nampla_data);
	n = write (species_fd, namp_data[species_index], num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\nCannot write nampla data to file!\n\n");
	    exit (-1);
	}

	if (sp->num_ships > 0)
	{
	    /* Write ship data. */
	    num_bytes = (long) sp->num_ships * sizeof (struct ship_data);
	    n = write (species_fd, ship_data[species_index], num_bytes);
	    if (n != num_bytes)
	    {
	        fprintf (stderr, "\nCannot write ship data to file!\n\n");
	        exit (-1);
	    }
	}

	close (species_fd);

	data_modified[species_index] = FALSE;
    }
}



free_species_data ()

{
    int		species_index;


    for (species_index = 0; species_index < galaxy.num_species; species_index++)
    {
	if (data_in_memory[species_index])
	{
	    free (namp_data[species_index]);

	    if (spec_data[species_index].num_ships > 0)
		free (ship_data[species_index]);

	    data_in_memory[species_index] = FALSE;
	    data_modified[species_index] = FALSE;
	}
    }
}



/* The following two routines will delete a ship or nampla record. */

delete_ship (ship)

struct ship_data	*ship;

{
	int	i;

	char	*cp;


	/* Set all bytes of record to zero. */
	cp = (char *) ship;
	for (i = 0; i < sizeof (struct ship_data); i++)
	    *cp++ = 0;

	ship->pn = 99;
	strcpy (ship->name, "Unused");
}


delete_nampla (nampla)

struct nampla_data	*nampla;

{
	int	i;

	char	*cp;


	/* Set all bytes of record to zero. */
	cp = (char *) nampla;
	for (i = 0; i < sizeof (struct nampla_data); i++)
	    *cp++ = 0;

	nampla->pn = 99;
	strcpy (nampla->name, "Unused");
}




/* This routine is intended to take a long argument and return a pointer
   to a string that has embedded commas to make the string more readable. */

char	result_plus_commas[33];

char *commas (value)

long	value;

{
    int		i, j, n, length, negative;

    char	*ptr, temp[32];

    long	abs_value;


    if (value < 0)
    {
	abs_value = -value;
	negative = TRUE;
    }
    else
    {
	abs_value = value;
	negative = FALSE;
    }

    sprintf (temp, "%ld\0", abs_value);

    length = strlen (temp);

    i = length - 1;
    j = 31;
    result_plus_commas[32] = '\0';
    for (n = 0; n < length; n++)
    {
	result_plus_commas[j--] = temp[i--];
	if (j % 4  ==  0)
	    result_plus_commas[j--] = ',';
    }

    j++;
    if (result_plus_commas[j] == ',') j++;

    if (negative)
	result_plus_commas[--j] = '-';

    return &result_plus_commas[j];
}



/* This routine will return a pointer to a string containing a complete
   ship name, including its orbital/landed status and age. If global
   variable "truncate_name" is TRUE, then orbital/landed status and age
   will not be included. */

int	truncate_name = FALSE;
int	ignore_field_distorters = FALSE;

char	full_ship_id[64];

char *ship_name (ship)

struct ship_data	*ship;

{
    int		effective_age, status, ship_is_distorted;

    char	temp[16];


    if (ship->item_quantity[FD] == ship->tonnage)
	ship_is_distorted = TRUE;
    else
	ship_is_distorted = FALSE;

    if (ship->status == ON_SURFACE) ship_is_distorted = FALSE;

    if (ignore_field_distorters) ship_is_distorted = FALSE;

    if (ship_is_distorted)
    {
	if (ship->class == TR)
	    sprintf (full_ship_id, "%s%d ???\0", ship_abbr[ship->class],
		ship->tonnage);
	else if (ship->class == BA)
	    sprintf (full_ship_id, "BAS ???\0");
	else
	    sprintf (full_ship_id, "%s ???\0", ship_abbr[ship->class]);
    }
    else if (ship->class == TR)
    {
	sprintf (full_ship_id, "%s%d%s %s\0",
		ship_abbr[ship->class], ship->tonnage, ship_type[ship->type],
		ship->name);
    }
    else
    { 
	sprintf (full_ship_id, "%s%s %s\0",
		ship_abbr[ship->class], ship_type[ship->type], ship->name);
    }

    if (truncate_name) return &full_ship_id[0];

    strcat (full_ship_id, " (");

    effective_age = ship->age;
    if (effective_age < 0) effective_age = 0;

    if (! ship_is_distorted)
    {
	if (ship->status != UNDER_CONSTRUCTION)
	{
	    /* Do age. */
	    sprintf (temp, "A%d,\0", effective_age);
	    strcat (full_ship_id, temp);
	}
    }

    status = ship->status;
    switch (status)
    {
	case UNDER_CONSTRUCTION:
		sprintf (temp, "C\0");
		break;
	case IN_ORBIT:
		sprintf (temp, "O%d\0", ship->pn);
		break;
	case ON_SURFACE:
		sprintf (temp, "L%d\0", ship->pn);
		break;
	case IN_DEEP_SPACE:
		sprintf (temp, "D\0");
		break;
	case FORCED_JUMP:
		sprintf (temp, "FJ\0");
		break;
	case JUMPED_IN_COMBAT:
		sprintf (temp, "WD\0");
		break;
	default:
		sprintf (temp, "***???***\0");
		fprintf (stderr, "\n\tWARNING!!!  Internal error in subroutine 'ship_name'\n\n");
    }

    strcat (full_ship_id, temp);

    if (ship->type == STARBASE)
    {
	sprintf (temp, ",%ld tons\0", 10000L * (long) ship->tonnage);
	strcat (full_ship_id, temp);
    }

    strcat (full_ship_id, ")");

    return &full_ship_id[0];
}



/* The following routines will post an item to standard output and to
   an externally defined log file and summary file. */

FILE		*log_file, *summary_file;

int	log_start_of_line = TRUE;
int	log_indentation = 0;
int	log_position = 0;
int	logging_disabled = FALSE;
int	log_to_file = TRUE;
int	log_summary = FALSE;
int	log_stdout = TRUE;

char	log_line[128];


log_char (c)

char	c;

{
    int		i, temp_position;

    char	temp_char;


    if (logging_disabled) return;

    /* Check if current line is getting too long. */
    if ((c == ' ' || c == '\n')  &&  log_position > 77)
    {
	/* Find closest preceeding space. */
	temp_position = log_position - 1;
	while (log_line[temp_position] != ' ') --temp_position;

	/* Write front of line to files. */
	temp_char = log_line[temp_position+1];
	log_line[temp_position] = '\n';
	log_line[temp_position+1] = '\0';
	if (log_to_file) fputs (log_line, log_file);
	if (log_stdout) fputs (log_line, stdout);
	if (log_summary) fputs (log_line, summary_file);
	log_line[temp_position+1] = temp_char;

	/* Copy overflow word to beginning of next line. */
	log_line[log_position] = '\0';
	log_position = log_indentation + 2;
	for (i = 0; i < log_position; i++)
	    log_line[i] = ' ';
	strcpy (&log_line[log_position], &log_line[temp_position+1]);

	log_position = strlen (log_line);

	if (c == ' ')
	{
	    log_line[log_position++] = ' ';
	    return;
	}
    }

    /* Check if line is being manually terminated. */
    if (c == '\n')
    {
	/* Write current line to output. */
	log_line[log_position] = '\n';
	log_line[log_position+1] = '\0';
	if (log_to_file) fputs (log_line, log_file);
	if (log_stdout) fputs (log_line, stdout);
	if (log_summary) fputs (log_line, summary_file);

	/* Set up for next line. */
	log_position = 0;
	log_indentation = 0;
	log_start_of_line = TRUE;

	return;
    }

    /* Save this character. */
    log_line[log_position] = c;
    ++log_position;

    if (log_start_of_line  &&  c == ' ')  /* Determine number of indenting */
	++log_indentation;		  /*  spaces for current line. */
    else
	log_start_of_line = FALSE;
}


log_string (string)

char	string[];

{
    int		i, length;


    if (logging_disabled) return;

    length = strlen (string);
    for (i = 0; i < length; i++)
	log_char (string[i]);
}


log_int (value)

int	value;

{
    char	string[16];


    if (logging_disabled) return;

    sprintf (string, "%d\0", value);
    log_string (string);
}


log_long (value)

long	value;

{
    char	string[16];


    if (logging_disabled) return;

    sprintf (string, "%ld\0", value);
    log_string (string);
}



int			num_locs = 0;

struct sp_loc_data	loc[MAX_LOCATIONS];

get_location_data ()

{
    int		locations_fd;

    long	n, file_size;


    /* Open locations file. */
    locations_fd = open ("locations.dat", 0);
    if (locations_fd < 0)
    {
	fprintf (stderr, "\nCannot open file 'locations.dat' for reading!\n\n");
	exit (-1);
    }

    /* Get size of file. */
    file_size = lseek (locations_fd, 0L, 2);
    num_locs = file_size / sizeof (struct sp_loc_data);

    /* Read it all into memory. */
    lseek (locations_fd, 0L, 0);	/* Rewind first. */
    n = read (locations_fd, loc, file_size);
    if (n != file_size)
    {
	fprintf (stderr, "\nCannot read file 'locations.dat' into memory!\n\n");
	exit (-1);
    }

    close (locations_fd);
}



save_location_data ()

{
    int		locations_fd;

    long	n, num_bytes;


    /* Open file 'locations.dat' for writing. */
    locations_fd = creat ("locations.dat", 0600);
    if (locations_fd < 0)
    {
	fprintf (stderr, "\n\tCannot create file 'locations.dat'!\n\n");
	exit (-1);
    }

    if (num_locs == 0)
    {
	close (locations_fd);
	return;
    }

    /* Write array to disk. */
    num_bytes = (long) num_locs * (long) sizeof(struct sp_loc_data);

    n = write (locations_fd, loc, num_bytes);
    if (n != num_bytes)
    {
	fprintf (stderr, "\n\n\tCannot write to 'locations.dat'!\n\n");
	exit (-1);
    }

    close (locations_fd);
}



/* The following routine provides the 'distorted' species number used to
	identify a species that uses field distortion units. The input
	variable 'species_number' is the same number used in filename
	creation for the species. */

int distorted (species_number)

int	species_number;

{
	int	i, j, n, ls;


	/* We must use the LS tech level at the start of the turn because
	   the distorted species number must be the same throughout the
	   turn, even if the tech level changes during production. */

	ls = spec_data[species_number-1].init_tech_level[LS];

	i = species_number & 0x000F;		/* Lower four bits. */
	j = (species_number >> 4) & 0x000F;	/* Upper four bits. */

	n = (ls % 5 + 3) * (4 * i + j) + (ls % 11 + 7);

	return n;
}

int undistorted (distorted_species_number)

int	distorted_species_number;

{
    int		i, species_number;


    for (i = 0; i < MAX_SPECIES; i++)
    {
	species_number = i + 1;

	if (distorted (species_number) == distorted_species_number)
	    return species_number;
    }

    return 0;	/* Not a legitimate species. */
}



log_message (message_filename)

char	*message_filename;

{
    char	message_line[256];

    FILE	*message_file;


    /* Open message file. */
    message_file = fopen (message_filename, "r");
    if (message_file == NULL)
    {
	fprintf (stderr, "\n\tWARNING! utils.c: cannot open message file '%s'!\n\n", message_filename);
	return;
    }

    /* Copy message to log file. */
    while (fgets(message_line, 256, message_file) != NULL)
	fputs (message_line, log_file);

    fclose (message_file);
}



/* This routine will set or clear the POPULATED bit for a nampla.  It will
   return TRUE if the nampla is populated or FALSE if not. It will also
   check if a message associated with this planet should be logged. */

int check_population (nampla)

struct nampla_data	*nampla;

{
	int	is_now_populated, was_already_populated;

	long	total_pop;

	char	filename[32];


	if (nampla->status & POPULATED)
	    was_already_populated = TRUE;
	else
	    was_already_populated = FALSE;

	total_pop  =  nampla->mi_base
			+ nampla->ma_base
			+ nampla->IUs_to_install
			+ nampla->AUs_to_install
			+ nampla->item_quantity[PD]
			+ nampla->item_quantity[CU]
			+ nampla->pop_units;

	if (total_pop > 0)
	{
		nampla->status |= POPULATED;
		is_now_populated = TRUE;
	}
	else
	{
		nampla->status &= ~( POPULATED | MINING_COLONY
					| RESORT_COLONY );
		is_now_populated = FALSE;
	}

	if (is_now_populated  &&  ! was_already_populated)
	{
	    if (nampla->message)
	    {
		/* There is a message that must be logged whenever this planet
			becomes populated for the first time. */
		sprintf (filename, "message%ld.txt\0", nampla->message);
		log_message (filename);
	    }
	}

	return is_now_populated;
}

/* Get life support tech level needed. */

int life_support_needed (species, home, colony)

struct species_data	*species;
struct planet_data	*home, *colony;

{
    int	i, j, k, ls_needed;


    i = colony->temperature_class - home->temperature_class;
    if (i < 0) i = -i;
    ls_needed = 3 * i;		/* Temperature class. */

    i = colony->pressure_class - home->pressure_class;
    if (i < 0) i = -i;
    ls_needed += 3 * i;		/* Pressure class. */

    /* Check gases. Assume required gas is NOT present. */
    ls_needed += 3;
    for (j = 0; j < 4; j++)	/* Check gases on planet. */
    {
      if (colony->gas_percent[j] == 0) continue;
      for (i = 0; i < 6; i++)	/* Compare with poisonous gases. */
      {
    	if (species->poison_gas[i] == colony->gas[j])
    	    ls_needed += 3;
      }
      if (colony->gas[j] == species->required_gas)
      {
        if (colony->gas_percent[j] >= species->required_gas_min
         && colony->gas_percent[j] <= species->required_gas_max)
    	ls_needed -= 3;
      }
    }

    return ls_needed;
}



check_high_tech_items (tech, old_tech_level, new_tech_level)

int	tech, old_tech_level, new_tech_level;

{
    int		i;


    for (i = 0; i < MAX_ITEMS; i++)
    {
	if (item_critical_tech[i] != tech) continue;
	if (new_tech_level < item_tech_requirment[i]) continue;
	if (old_tech_level >= item_tech_requirment[i]) continue;

	log_string ("  You now have the technology to build ");
	log_string (item_name[i]);
	log_string ("s.\n");
    }

    /* Check for high tech abilities that are not associated with specific
	items. */
    if (tech == MA  &&  old_tech_level < 25  &&  new_tech_level >= 25)
	log_string ("  You now have the technology to do interspecies construction.\n");
}




/* The following routine will return a score indicating how closely two
   strings match.  If the score is exactly 10000, then the strings are
   identical.  Otherwise, the value returned is the number of character
   matches, allowing for accidental transpositions, insertions, and
   deletions.  Excess characters in either string will subtract from
   the score.  Thus, it's possible for a score to be negative.

   In general, if the strings are at least 7 characters each, then you can
   assume the strings are the same if the highest score equals the length of
   the correct string, length-1, or length-2, AND if the score of the next
   best match is less than the highest score.  A non-10000 score will never
   be higher than the length of the correct string. */

int agrep_score (correct_string, unknown_string)

char	*correct_string, *unknown_string;

{
    int	score;

    char	c1, c2, *p1, *p2;


    if (strcmp (correct_string, unknown_string) == 0) return 10000;

    score = 0;
    p1 = correct_string;
    p2 = unknown_string;

    while (1)
    {
	if ((c1 = *p1++) == '\0')
	{
	    score -= strlen (p2);	/* Reduce score by excess characters,
					   if any. */
	    break;
	}

	if ((c2 = *p2++) == '\0')
	{
	    score -= strlen (p1);	/* Reduce score by excess characters,
					   if any. */
	    break;
	}

	if (c1 == c2)
	    ++score;
	else if (c1 == *p2  &&  c2 == *p1)
	{
	    /* Transposed. */
	    score += 2;
	    ++p1;
	    ++p2;	    
	}
	else if (c1 == *p2)
	{
	    /* Unneeded character. */
	    ++score;
	    ++p2;
	}
	else if (c2 == *p1)
	{
	    /* Missing character. */
	    ++score;
	    ++p1;
	}
    }

    return score;
}


extern int			num_stars, species_number, star_data_modified;
extern struct star_data		*star_base;


/* The following routine will check if coordinates x-y-z contain a star and,
   if so, will set the appropriate bit in the "visited_by" variable for the
   star. If the star exists, TRUE will be returned; otherwise, FALSE will
   be returned. */

int star_visited (x, y, z)

int	x, y, z;

{
    int		i, found, species_array_index, species_bit_number;

    long	species_bit_mask;

    struct star_data	*star;


    /* Get array index and bit mask. */
    species_array_index = (species_number - 1) / 32;
    species_bit_number = (species_number - 1) % 32;
    species_bit_mask = 1 << species_bit_number;

    found = FALSE;

    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	if (x != star->x) continue;
	if (y != star->y) continue;
	if (z != star->z) continue;

	found = TRUE;

	/* Check if bit is already set. */
	if (star->visited_by[species_array_index] & species_bit_mask) break;

	/* Set the appropriate bit. */
	star->visited_by[species_array_index] |= species_bit_mask;
	star_data_modified = TRUE;
	break;
    }

    return found;
}
