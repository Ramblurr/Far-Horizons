
#define THIS_IS_MAIN

#include "fh.h"


int			species_number, species_index, data_was_modified,
			num_arguments, abbr_type, abbr_index, sub_light,
			tonnage, next_arg;

char			**argument, *ship_name(), input_abbr[32];


struct galaxy_data	galaxy;
struct star_data	*star;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;

extern int		num_stars, num_planets;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    num_arguments = argc;
    argument = argv;

    data_was_modified = FALSE;

    /* Find out what we're dealing with. */
    if (argc > 2  &&  isdigit(argv[1][0])  &&  ! isdigit(argv[2][0]))
	set_species ();
    else if (argc > 4  &&  isdigit(argv[1][0])  &&  isdigit(argv[2][0])
		&&  isdigit(argv[3][0])  &&  ! isdigit(argv[4][0]))
	set_star ();
    else if (argc > 5  &&  isdigit(argv[1][0])  &&  isdigit(argv[2][0])
		&&  isdigit(argv[3][0])  &&  isdigit(argv[4][0]))
	set_planet ();

    fprintf (stderr, "\n\tUsage:\n\t\tSet n args... for species\n\t\tSet n n n args... for star\n\t\tSet n n n n args... for planet.\n\n");
    exit (-1);
}

set_species()

{
    long	n;


    species_number = atoi (argument[1]);
    species_index = species_number - 1;

    get_species ();

    next_arg = 2;

next_item:

    get_class_abbr (argument[next_arg++]);
    if (abbr_type == PLANET_ID)
	set_nampla ();
    else if (abbr_type == SHIP_CLASS)
	set_ship ();
    else if (abbr_type == TECH_ID)
    {
	printf ("SP %s, %s tech level = %d", species->name,
	    tech_name[abbr_index], species->tech_level[abbr_index]);

	if (next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		species->tech_level[abbr_index] = n;

		data_was_modified = TRUE;
	    }
	}

	printf (".\n");
    }
    else if (strcmp (input_abbr, "EU") == 0)
    {
	printf ("SP %s has %ld economic units", species->name,
		species->econ_units);

	if (next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		species->econ_units = n;

		data_was_modified = TRUE;
	    }
	}

	printf (".\n");
    }
    else if (strcmp (input_abbr, "HP") == 0)
    {
	printf ("SP %s hp_original_base = %ld", species->name,
		species->hp_original_base);

	if (next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		species->hp_original_base = n;

		data_was_modified = TRUE;
	    }
	}

	printf (".\n");
    }
    else
    {
	fprintf (stderr, "\n\tInvalid argument: '%s'!\n\n", argument[--next_arg]);
	exit (-1);
    }

    if (next_arg < num_arguments) goto next_item;

    if (data_was_modified) save_species ();

    free (nampla_base);
    if (species->num_ships > 0) free (ship_base);

    exit (0);
}



set_nampla ()

{
    int		i, found;

    long	n;


    found = FALSE;
    for (i = 0; i < species->num_namplas; i++)
    {
	nampla = nampla_base + i;

	if (strcasecmp (argument[next_arg], nampla->name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (! found)
    {
	fprintf (stderr, "\n\tInvalid planet name: '%s'!\n\n", argument[next_arg]);
	exit (-1);
    }

    ++next_arg;

again:

    get_class_abbr (argument[next_arg]);
    switch (abbr_type)
    {
	case ITEM_CLASS:

	    printf ("SP %s, PL %s, %ld %ss", species->name, nampla->name,
		nampla->item_quantity[abbr_index], item_name[abbr_index]);

	    if (++next_arg < num_arguments)
	    {
		if (isdigit (argument[next_arg][0]))
		{
		    n = atol (argument[next_arg++]);
		    printf (", changed to %ld", n);

		    nampla->item_quantity[abbr_index] = n;

		    data_was_modified = TRUE;
		}
	    }

	    break;


	default:

	    if (strcasecmp (argument[next_arg], "x") == 0)
	    {
		printf ("SP %s, PL %s, x = %d", species->name, nampla->name,
		    nampla->x);

		++next_arg;

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "y") == 0)
	    {
		printf ("SP %s, PL %s, y = %d", species->name, nampla->name,
		    nampla->y);

		++next_arg;

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "z") == 0)
	    {
		printf ("SP %s, PL %s, z = %d", species->name, nampla->name,
		    nampla->z);

		++next_arg;

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "pn") == 0)
	    {
		printf ("SP %s, PL %s, pn = %d", species->name, nampla->name,
		    nampla->pn);

		++next_arg;

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "sy") == 0)
	    {
		printf ("SP %s, PL %s, %d shipyards", species->name, nampla->name,
		    nampla->shipyards);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			nampla->shipyards = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "ib") == 0)
	    {
		printf ("SP %s, PL %s, mining base = %d", species->name, nampla->name,
		    nampla->mi_base);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			nampla->mi_base = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "ab") == 0)
	    {
		printf ("SP %s, PL %s, manufacturing base = %d", species->name, nampla->name,
		    nampla->ma_base);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			nampla->ma_base = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "ap") == 0)
	    {
		printf ("SP %s, PL %s, available population = %d", species->name, nampla->name,
		    nampla->pop_units);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			nampla->pop_units = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else
		return;
    }

    printf (".\n");

    if (next_arg < num_arguments) goto again;

    return;
}



set_ship ()

{
    int		i, found;

    long	n;


    found = FALSE;
    for (i = 0; i < species->num_ships; i++)
    {
	ship = ship_base + i;

	if (strcasecmp (argument[next_arg], ship->name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (! found)
    {
	fprintf (stderr, "\n\tInvalid ship name: '%s'!\n\n", argument[next_arg]);
	exit (-1);
    }

    ++next_arg;

again:

    get_class_abbr (argument[next_arg]);
    switch (abbr_type)
    {
	case ITEM_CLASS:

	    printf ("SP %s, %s, %ld %ss", species->name, ship_name (ship),
		ship->item_quantity[abbr_index], item_name[abbr_index]);

	    if (++next_arg < num_arguments)
	    {
		if (isdigit (argument[next_arg][0]))
		{
		    n = atol (argument[next_arg++]);
		    printf (", changed to %ld", n);

		    ship->item_quantity[abbr_index] = n;

		    data_was_modified = TRUE;
		}
	    }

	    break;


	default:

	    if (strcasecmp (argument[next_arg], "x") == 0)
	    {
		printf ("SP %s, %s, x = %d", species->name, ship_name(ship),
		    ship->x);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->x = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "y") == 0)
	    {
		printf ("SP %s, %s, y = %d", species->name, ship_name(ship),
		    ship->y);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->y = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "z") == 0)
	    {
		printf ("SP %s, %s, z = %d", species->name, ship_name(ship),
		    ship->z);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->z = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "pn") == 0)
	    {
		printf ("SP %s, %s, pn = %d", species->name, ship_name(ship),
		    ship->pn);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->pn = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "lp") == 0)
	    {
		printf ("SP %s, %s, loading_point = %d", species->name,
		    ship_name(ship), ship->loading_point);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->loading_point = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "up") == 0)
	    {
		printf ("SP %s, %s, unloading_point = %d", species->name,
		    ship_name(ship), ship->unloading_point);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->unloading_point = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else if (strcasecmp (argument[next_arg], "ag") == 0)
	    {
		printf ("SP %s, %s, age = %d turns", species->name, ship_name(ship),
		    ship->age);

		if (++next_arg < num_arguments)
		{
		    if (isdigit (argument[next_arg][0]))
		    {
			n = atol (argument[next_arg++]);
			printf (", changed to %ld", n);

			ship->age = n;

			data_was_modified = TRUE;
		    }
		}

		break;
	    }
	    else
		return;
    }

    printf (".\n");

    if (next_arg < num_arguments) goto again;

    return;
}



set_star()

{
    int		i, x, y, z, found;

    long	n;


    x = atoi (argument[1]);
    y = atoi (argument[2]);
    z = atoi (argument[3]);

    get_star_data ();

    /* Get star. Check if planet exists. */
    found = FALSE;
    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	if (star->x != x) continue;
	if (star->y != y) continue;
	if (star->z != z) continue;

	found = TRUE;

	break;
    }

    if (! found )
    {
	fprintf (stderr, "\n\tThere is no star at %d %d %d!\n\n",
	    x, y, z);
	exit (-1);
    }

    next_arg = 4;

next_item:

    if (strcasecmp (argument[next_arg], "np") == 0)
    {
	printf ("For star at %d %d %d, num planets = %d",
		x, y, z, star->num_planets);

	++next_arg;
    }
    else
    {
	fprintf (stderr, "\n\tInvalid argument: '%s'!\n\n", argument[next_arg]);
	exit (-1);
    }

    printf (".\n");

    if (next_arg < num_arguments) goto next_item;

    if (data_was_modified) save_star_data ();

    free (star_base);

    exit (0);
}



set_planet()

{
    int		i, x, y, z, pn, found;

    long	n;


    x = atoi (argument[1]);
    y = atoi (argument[2]);
    z = atoi (argument[3]);
    pn = atoi (argument[4]);

    get_star_data ();
    get_planet_data ();

    /* Get star. Check if planet exists. */
    found = FALSE;
    for (i = 0; i < num_stars; i++)
    {
	star = star_base + i;

	if (star->x != x) continue;
	if (star->y != y) continue;
	if (star->z != z) continue;

	if (pn > star->num_planets) break;

	found = TRUE;

	break;
    }

    if (! found )
    {
	fprintf (stderr, "\n\tThere is no planet at %d %d %d %d!\n\n",
	    x, y, z, pn);
	exit (-1);
    }

    planet = planet_base + star->planet_index + pn - 1;

    next_arg = 5;

next_item:

    if (strcasecmp (argument[next_arg], "md") == 0)
    {
	printf ("For planet #%d at %d %d %d, mining difficulty = %d",
		pn, x, y, z, planet->mining_difficulty);

	if (++next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		planet->mining_difficulty = n;

		data_was_modified = TRUE;
	    }
	}
    }
    else if (strcasecmp (argument[next_arg], "ee") == 0)
    {
	printf ("For planet #%d at %d %d %d, economic efficiency = %d",
		pn, x, y, z, planet->econ_efficiency);

	if (++next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		planet->econ_efficiency = n;

		data_was_modified = TRUE;
	    }
	}
    }
    else if (strcasecmp (argument[next_arg], "pc") == 0)
    {
	printf ("For planet #%d at %d %d %d, pressure class = %d",
		pn, x, y, z, planet->pressure_class );

	if (++next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		planet->pressure_class = n;

		data_was_modified = TRUE;
	    }
	}
    }
    else if (strcasecmp (argument[next_arg], "tc") == 0)
    {
	printf ("For planet #%d at %d %d %d, temperature class = %d",
		pn, x, y, z, planet->temperature_class );

	if (++next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		planet->temperature_class = n;

		data_was_modified = TRUE;
	    }
	}
    }
    else if (strcasecmp (argument[next_arg], "gv") == 0)
    {
	printf ("For planet #%d at %d %d %d, gravity = %d",
		pn, x, y, z, planet->gravity);

	if (++next_arg < num_arguments)
	{
	    if (isdigit (argument[next_arg][0]))
	    {
		n = atol (argument[next_arg++]);
		printf (", changed to %ld", n);

		planet->gravity = n;

		data_was_modified = TRUE;
	    }
	}
    }
    else
    {
	fprintf (stderr, "\n\tInvalid argument: '%s'!\n\n", argument[next_arg]);
	exit (-1);
    }

    printf (".\n");

    if (next_arg < num_arguments) goto next_item;

    if (data_was_modified) save_planet_data ();

    free (planet_base);

    exit (0);
}



get_species ()

{
    int		species_fd;

    long	n, num_bytes;

    char	filename[16];


    species = &spec_data[species_index];

    /* Open the species data file. */
    sprintf (filename, "sp%02d.dat\0", species_number);
    species_fd = open (filename, 0);
    if (species_fd < 0)
    {
	fprintf (stderr, "\n\tInvalid species number!\n\n");
	exit (-1);
    }

    /* Read in species data. */
    num_bytes = read (species_fd, species, sizeof(struct species_data));
    if (num_bytes != sizeof(struct species_data))
    {
	fprintf (stderr, "\n\tCannot read species record in file '%s'!\n\n",
	    filename);
	exit (-1);
    }

    /* Allocate enough memory for all namplas. */
    num_bytes = ((long) species->num_namplas) * sizeof (struct nampla_data);
    nampla_base = (struct nampla_data *) malloc (num_bytes);
    if (nampla_base == NULL)
    {
	fprintf (stderr, "\nCannot allocate enough memory for nampla data!\n\n");
	exit (-1);
    }

    /* Read it all into memory. */
    num_bytes = (long) species->num_namplas * sizeof (struct nampla_data);
    n = read (species_fd, nampla_base, num_bytes);
    if (n != num_bytes)
    {
	fprintf (stderr, "\nCannot read nampla data into memory!\n\n");
	exit (-1);
    }

    if (species->num_ships > 0)
    {
	/* Allocate enough memory for all ships. */
	num_bytes = ((long) species->num_ships) * sizeof (struct ship_data);
	ship_base = (struct ship_data *) malloc (num_bytes);
	if (ship_base == NULL)
	{
	    fprintf (stderr, "\nCannot allocate enough memory for ship data!\n\n");
	    exit (-1);
	}

	/* Read it all into memory. */
	num_bytes = (long) species->num_ships * sizeof (struct ship_data);
	n = read (species_fd, ship_base, num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\nCannot read ship data into memory!\n\n");
	    exit (-1);
	}
    }

    close (species_fd);
}



save_species ()

{
    int		species_fd;

    long	n, num_bytes;

    char	filename[16];


    /* Create the new species data file. */
    sprintf (filename, "sp%02d.dat\0", species_number);
    species_fd = creat (filename, 0600);
    if (species_fd < 0)
    {
	fprintf (stderr, "\n  Cannot create new version of file '%s'!\n",
				filename);
	exit (-1);
    }

    /* Write species data. */
    num_bytes = write (species_fd, species, sizeof(struct species_data));
    if (num_bytes != sizeof(struct species_data))
    {
	fprintf (stderr, "\n\tCannot write species record to file '%s'!\n\n",
	    filename);
	exit (-1);
    }

    /* Write nampla data. */
    num_bytes = (long) species->num_namplas * sizeof (struct nampla_data);
    n = write (species_fd, nampla_base, num_bytes);
    if (n != num_bytes)
    {
	fprintf (stderr, "\nCannot write nampla data to disk!\n\n");
	exit (-1);
    }

    if (species->num_ships > 0)
    {
	/* Write ship data. */
	num_bytes = (long) species->num_ships * sizeof (struct ship_data);
	n = write (species_fd, ship_base, num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\nCannot write ship data to disk!\n\n");
	    exit (-1);
	}
    }

    close (species_fd);
}



/* Get a class abbreviation and return TECH_ID, ITEM_CLASS, SHIP_CLASS,
   PLANET_ID, SPECIES_ID or ALLIANCE_ID as appropriate, or UNKNOWN if it
   cannot be identified. Also, set "abbr_type" to this value. If it is
   TECH_ID, ITEM_CLASS or SHIP_CLASS, "abbr_index" will contain the
   abbreviation index. If it is a ship, "tonnage" will contain tonnage/10,000,
   and "sub_light" will be TRUE or FALSE. (Tonnage value returned is based
   ONLY on abbreviation.) */

int get_class_abbr (arg)

char	*arg;

{
    int		i;

    char	*digit_start;


    abbr_type = UNKNOWN;

    if (! isalnum (*arg)) return UNKNOWN;
    input_abbr[0] = toupper(*arg);
    ++arg;

    if (! isalnum (*arg)) return UNKNOWN;
    input_abbr[1] = toupper(*arg);
    ++arg;

    input_abbr[2] = '\0';

    /* Check for IDs that are followed by one or more digits or letters. */
    i = 2;
    digit_start = arg;
    while (isalnum (*arg))
    {
	input_abbr[i++] = *arg++;
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
	    arg = digit_start;
	    abbr_index = i;
	    tonnage = ship_tonnage[i];
	    if (i == TR)
	    {
		tonnage = 0;
		while (isdigit(*arg))
		{
		    tonnage = (10 * tonnage) + (*arg - '0');
		    ++arg;
		}
	    }

	    if (toupper(*arg) == 'S')
	    {
		sub_light = TRUE;
		++arg;
	    }
	    else
		sub_light = FALSE;

	    if (isalnum (*arg)) break;	/* Garbage. */

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
    if (ship->pn  ==  0) status = IN_DEEP_SPACE;	/* For combat only. */
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
