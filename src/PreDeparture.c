
#define THIS_IS_MAIN

#include "fh.h"


int	post_arrival_phase = FALSE;

int	x, y, z, pn, nampla_index, ship_index, first_pass, species_number,
	species_index;

int	test_mode, verbose_mode;

extern int			star_data_modified, planet_data_modified;
extern struct star_data		*star_base;
extern struct planet_data	*planet_base;

struct galaxy_data	galaxy;
struct star_data	*star;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;


extern int		end_of_file, truncate_name, log_stdout,
			ignore_field_distorters, just_opened_file;
extern char		input_line[256], *input_line_pointer;
extern unsigned long	last_random;
extern FILE		*input_file, *log_file;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, n, found, num_species, sp_num[MAX_SPECIES], sp_index,
		command, do_all_species;

    char	filename[32], keyword[4];


    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) rnd(10);

    /* Get commonly used data. */
    get_galaxy_data ();
    get_transaction_data ();

    ignore_field_distorters = TRUE;

    /* Check arguments. If an argument is -p, then do two passes. In the
	first pass, display results and prompt the GM, allowing him to
	abort if necessary before saving results to disk. If an argument
	is -t, then set test mode. All other arguments must be species
	numbers. If no species numbers are specified, then do all species. */
    num_species = 0;
    first_pass = FALSE;
    test_mode = FALSE;
    verbose_mode = FALSE;
    for (i = 1; i < argc; i++)
    {
	if (strcmp (argv[i], "-p") == 0)
	    first_pass = TRUE;
	else if (strcmp (argv[i], "-t") == 0)
	    test_mode = TRUE;
	else if (strcmp (argv[i], "-v") == 0)
	    verbose_mode = TRUE;
	else
	{
	    n = atoi (argv[i]);
	    if (n < 1  ||  n > galaxy.num_species)
	    {
		fprintf (stderr,
		    "\n    '%s' is not a valid argument!\n", argv[i]);
		exit (-1);
	    }
	    sp_num[num_species++] = n;
	}
    }

    if (num_species == 0)
    {
	num_species = galaxy.num_species;
	for (i = 0; i < num_species; i++)
	    sp_num[i] = i+1;
	do_all_species = TRUE;
    }
    else
	do_all_species = FALSE;

    /* Two passes through all orders will be done. The first pass will
	check for errors and abort if any are found. Results will be written
	to disk only on the second pass. */

start_pass:

    if (first_pass) printf ("\nStarting first pass...\n\n");

    get_species_data ();
    get_star_data ();
    get_planet_data ();

    /* Main loop. For each species, take appropriate action. */
    for (sp_index = 0; sp_index < num_species; sp_index++)
    {
	species_number = sp_num[sp_index];
	species_index = species_number - 1;

	found = data_in_memory[species_index];
	if (! found)
	{
	    if (do_all_species)
	    {
		if (first_pass) printf ("\n    Skipping species #%d.\n", species_number);
		continue;
	    }
	    else
	    {
		fprintf (stderr, "\n    Cannot get data for species #%d!\n",
			species_number);
		exit (-1);
	    }
	}

	species = &spec_data[species_index];
	nampla_base = namp_data[species_index];
	ship_base = ship_data[species_index];

	/* Open orders file for this species. */
	sprintf (filename, "sp%02d.ord\0", species_number);
	input_file = fopen (filename, "r");
	if (input_file == NULL)
	{
	    if (do_all_species)
	    {
		if (first_pass) printf ("\n    No orders for species #%d.\n", species_number);
		continue;
	    }
	    else
	    {
		fprintf (stderr, "\n\tCannot open '%s' for reading!\n\n", filename);
		exit (-1);
	    }
	}

	end_of_file = FALSE;

	just_opened_file = TRUE;	/* Tell parse.c to skip mail header,
						if any. */
find_start:

	/* Search for START PRE-DEPARTURE order. */
	found = FALSE;
	while (! found)
	{
	    command = get_command();
	    if (command == MESSAGE)
	    {
		/* Skip MESSAGE text. It may contain a line that starts
			with "start". */
		while (TRUE)
		{
		    command = get_command();
		    if (command < 0)
		    {
			fprintf (stderr,
			    "WARNING: Unterminated MESSAGE command in file %s!\n",
			    filename);
			break;
		    }

		    if (command == ZZZ) goto find_start;
		}
	    }

	    if (command < 0)
		break;		/* End of file. */

	    if (command != START)
		continue;

	    /* Get the first three letters of the keyword and convert to
		upper case. */
	    skip_whitespace();
	    for (i = 0; i < 3; i++)
	    {
		keyword[i] = toupper (*input_line_pointer);
		++input_line_pointer;
	    }
	    keyword[3] = '\0';

	    if (strcmp(keyword, "PRE") == 0) found = TRUE;
	}

	if (! found)
	{
	    if (first_pass) printf ("\nNo pre-departure orders for species #%d, SP %s.\n",
		species_number, species->name);
	    goto done_orders;
	}

	/* Open log file. Use stdout for first pass. */
	log_stdout = FALSE;  /* We will control value of log_file from here. */
	if (first_pass)
	{
	    log_file = stdout;
	}
	else
	{
	    /* Open log file for appending. */
	    sprintf (filename, "sp%02d.log\0", species_number);
	    log_file = fopen (filename, "a");
	    if (log_file == NULL)
	    {
		fprintf (stderr, "\n\tCannot open '%s' for appending!\n\n", filename);
		exit (-1);
	    }
	    log_string ("\nPre-departure orders:\n");
	}

	/* Handle predeparture orders for this species. */
	do_predeparture_orders ();

	data_modified[species_index] = TRUE;

	if (first_pass) goto done_orders;

	/* If this is the second pass, close the log file. */
	if (! first_pass) fclose (log_file);

done_orders:

	fclose (input_file);
    }

    if (first_pass)
    {
	printf ("\nFinal chance to abort safely!\n");
	gamemaster_abort_option ();
	first_pass = FALSE;
	free_species_data ();
	free (star_base);	/* In case data was modified. */

	printf ("\nStarting second pass...\n\n");

	goto start_pass;
    }

    save_species_data ();
    save_transaction_data ();
    if (star_data_modified) save_star_data ();
    if (planet_data_modified) save_planet_data ();
    free_species_data ();
    free (star_base);
    free (planet_base);
    exit (0);
}



do_predeparture_orders ()

{
    int		i, command, old_test_mode;


    if (first_pass)
	printf ("\nStart of pre-departure orders for species #%d, SP %s...\n",
	    species_number, species->name);

    truncate_name = TRUE;	/* For these commands, do not display age
				   or landed/orbital status of ships. */

    while (TRUE)
    {
	command = get_command();

	if (command == 0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Unknown or missing command.\n");
	    continue;
	}

	if (end_of_file || command == END)
	{
	    if (first_pass)
		printf ("End of pre-departure orders for species #%d, SP %s.\n",
		    species_number, species->name);

	    if (first_pass) gamemaster_abort_option ();

	    break;			/* END for this species. */
	}

	switch (command)
	{
	    case ALLY:
		do_ALLY_command ();
		break;

	    case BASE:
		do_BASE_command ();
		break;

	    case DEEP:
		do_DEEP_command ();
		break;

	    case DESTROY:
		do_DESTROY_command ();
		break;

	    case DISBAND:
		do_DISBAND_command ();
		break;

	    case ENEMY:
		do_ENEMY_command ();
		break;

	    case INSTALL:
		do_INSTALL_command ();
		break;

	    case LAND:
		do_LAND_command ();
		break;

	    case MESSAGE:
		do_MESSAGE_command ();
		break;

	    case NAME:
		do_NAME_command ();
		break;

	    case NEUTRAL:
		do_NEUTRAL_command ();
		break;

	    case ORBIT:
		do_ORBIT_command ();
		break;

	    case REPAIR:
		do_REPAIR_command ();
		break;

	    case SCAN:
		/* Scan is okay in test mode for pre-departure. */
		old_test_mode = test_mode;
		test_mode = FALSE;
		do_SCAN_command ();
		test_mode = old_test_mode;
		break;

	    case SEND:
		do_SEND_command ();
		break;

	    case TRANSFER:
		do_TRANSFER_command ();
		break;

	    case UNLOAD:
		do_UNLOAD_command ();
		break;

	    default:
		fprintf (log_file, "!!! Order ignored:\n");
		fprintf (log_file, "!!! %s", input_line);
		fprintf (log_file, "!!! Invalid pre-departure command.\n");
	}
    }
}
