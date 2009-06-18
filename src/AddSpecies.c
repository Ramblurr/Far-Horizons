
/* This program will allow the gamemaster to add a new species to the
   game. */


#define THIS_IS_MAIN

#include "fh.h"

int			species_number;

struct galaxy_data	galaxy;
struct species_data	*species;
struct nampla_data	*nampla_base;

extern int		num_stars, num_planets, print_LSN;
extern unsigned long	last_random;
extern FILE		*log_file;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, j, x, y, z, pn, temp, found, percent, species_fd,
		good_gas[14], num_neutral, req_gas, galaxy_fd,
		species_array_index, species_bit_number;

    long	n, species_bit_mask;

    char	c, filename[16], *cp;

    struct star_data	*star;
    struct planet_data	*home_planet;
    struct species_data	spec;
    struct nampla_data	home_nampla;


    species = &spec;

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) rnd(10);

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();

    /* Check for valid command line. */
    if (argc == 1)
	species_number = ++galaxy.num_species;
    else if (argc == 2)
	species_number = atoi (argv[1]);
    else
    {
	fprintf (stderr, "\n  Usage: AddSpecies [n]\n");
	fprintf (stderr, "    where 'n' is the optional species number.\n\n");
	exit (0);
    }

    /* Initialize all bytes of record to zero. */
    cp = (char *) &spec;
    for (i = 0; i < sizeof (struct species_data); i++)
	*cp++ = 0;

    /* Get info about new species. */
get_name_again:
    printf ("\nEnter name of species #%d: ", species_number);
    get_name (spec.name);
    j = strlen (spec.name);
    if (j < 5) {
      printf("\n\n\tERROR!  Species '%s' name too short (min 5 chars required)\n", spec.name);
      goto get_name_again;
    }
    for (i = 0; i < j; i++)
    {
	c = spec.name[i];
	if (c == '$'  ||  c == '!'  ||  c == '"')
	{
	    printf ("\n\n\tERROR!  Invalid character '%c' in species name!\n",
		c);
	    goto get_name_again;
	}
    }

    delete_nampla (&home_nampla);		/* Set everything to zero. */

    printf ("Enter name of home planet: ");
    get_name (home_nampla.name);

    printf ("Enter name of government: ");
    get_name (spec.govt_name);

    printf ("Enter type of government: ");
    get_name (spec.govt_type);

get_xyz:
    printf ("\nEnter x-coordinate: "); fflush (stdout);
    scanf ("%d", &x); spec.x = x; home_nampla.x = x;

    printf ("Enter y-coordinate: "); fflush (stdout);
    scanf ("%d", &y); spec.y = y; home_nampla.y = y;

    printf ("Enter z-coordinate: "); fflush (stdout);
    scanf ("%d", &z); spec.z = z; home_nampla.z = z;

    /* Get pointers to appropriate star and planet. */
    found = 0;
    star = star_base;
    for (i = 0; i < num_stars; i++)
    {
	if (star->x == x  &&  star->y == y  &&  star->z == z)
	{
	    found = 1;
	    break;
	}
	++star;
    }

    if (found == 0)
    {
	printf ("\n\tThere is no star at these coordinates! Try again.\n");
	goto get_xyz;
    }

    printf ("\nScan of star system:\n\n");
    print_LSN = FALSE;
    log_file = stdout;
    scan (x, y, z);

    printf ("\nEnter planet number: "); fflush (stdout);
    scanf ("%d", &pn); spec.pn = pn; home_nampla.pn = pn;

    if (pn > star->num_planets)
    {
	printf ("\n\tPlanet number is too large for star! Try again!\n");
	goto get_xyz;
    }

    /* Get pointer to planet. */
    home_nampla.planet_index = star->planet_index + pn - 1;
    home_planet = planet_base + (long) home_nampla.planet_index;

    /* Get player-specified tech levels. */
    printf ("\n");
    for (i = ML; i <= BI; i++)
    {
      get_tl:
	printf ("Enter %s tech level: ", tech_name[i]); fflush (stdout);
	scanf ("%d", &j); if (j < 0) goto get_tl;
	spec.tech_level[i] = j;
	spec.tech_knowledge[i] = j;
	spec.init_tech_level[i] = j;
	spec.tech_eps[i] = 0;
    }

    /* Check tech levels. */
    n = 0;
    for (i = ML; i <= BI; i++) n += spec.tech_level[i];
    if (n > 15) printf ("\n\tWarning! ML + GV + LS + BI is greater than 15!\n\n");

    /* Mining and manufacturing are each 10. */
    spec.tech_level[MI] = 10;
    spec.tech_level[MA] = 10;

    /* Initialize other tech stuff. */
    for (i = MI; i <= BI; i++)
    {
	j = spec.tech_level[i];
	spec.tech_knowledge[i] = j;
	spec.init_tech_level[i] = j;
	spec.tech_eps[i] = 0;
    }

    /* Get required gas. */
  get_gas:
    printf ("\n\n");
    for (i = 1; i <= 13; i++)
    {
	printf ("   %2d - %3s", i, gas_string[i]);
	if (i == 7) printf ("\n");
    }

    printf ("\n\n    The home planet has the following gases:\n\t");
    for (i = 0; i < 4; i++)
    {
	if (home_planet->gas[i] == 0) break;
	printf (" %s(%d%%) ",	gas_string[home_planet->gas[i]],
				home_planet->gas_percent[i]);
    }

    printf ("\n\nEnter number of gas required by species: "); fflush (stdout);
    scanf ("%d", &req_gas); if (req_gas < 1  ||  req_gas > 13) goto get_gas;
    spec.required_gas = req_gas;

    /* While checking if selection is valid, start determining neutral
	gases. */
    found = 0;
    num_neutral = 0;
    for (i = 1; i <= 13; i++) good_gas[i] = 0;
    for (i = 0; i < 4; i++)
    {
	if (home_planet->gas[i] == req_gas)
	{
	    percent = home_planet->gas_percent[i];
	    found = 1;
	}
	if (home_planet->gas[i] > 0)
	{
	    good_gas[home_planet->gas[i]] = 1; /* All home planet gases are either
					     required or neutral. */
	    ++num_neutral;
	}
    }
    if (found == 0)
    {
	printf ("\n\tPlanet does not have %s!\n", gas_string[req_gas]);
	goto get_gas;
    }

    temp = percent/2;
    if (temp < 1)
	spec.required_gas_min = 1;
    else
	spec.required_gas_min = temp;

    temp = 2*percent;
    if (temp < 20) temp += 20;
    if (temp > 100) temp = 100;
    spec.required_gas_max = temp;

    /* Do neutral gases. Start with the good_gas array and add neutral
	gases until there are exactly seven of them. One of the seven
	gases will be the required gas. Helium must always be neutral
	since it is a noble gas. Also, this game is biased towards oxygen
	breathers, so make H2O neutral also. */
    if (good_gas[HE] == 0)
    {
	good_gas[HE] = 1;
	++num_neutral;
    }
    if (good_gas[H2O] == 0)
    {
	good_gas[H2O] = 1;
	++num_neutral;
    }
    while (num_neutral < 7)
    {
	i = rnd(13);
	if (good_gas[i] == 0)
	{
	    good_gas[i] = 1;
	    ++num_neutral;
	}
    }

    n = 0;
    for (i = 1; i <= 13; i++)
    {
	if (good_gas[i] > 0  &&  i != req_gas)
	{
	    spec.neutral_gas[n] = i;
	    ++n;
	}
    }

    /* Do poison gases. */
    n = 0;
    for (i = 1; i <= 13; i++)
    {
	if (good_gas[i] == 0)
	{
	    spec.poison_gas[n] = i;
	    ++n;
	}
    }

    /* Do mining and manufacturing bases of home planet. Initial mining
	and production capacity will be 25 times sum of MI and MA plus
	a small random amount. Mining and manufacturing base will be
	reverse-calculated from the capacity. */
    i = spec.tech_level[MI] + spec.tech_level[MA];
    n = (25 * i)  +  rnd(i) + rnd(i) + rnd(i);
    home_nampla.mi_base =
	(n * (long) home_planet->mining_difficulty)
		/ (10L * (long) spec.tech_level[MI]);
    home_nampla.ma_base = (10L * n) / (long) spec.tech_level[MA];

    /* Initialize contact/ally/enemy masks. */
    for (i = 0; i < NUM_CONTACT_WORDS; i++)
    {
	spec.contact[i] = 0;
	spec.ally[i] = 0;
	spec.enemy[i] = 0;
    }

    /* Fill out the rest. */
    spec.num_namplas = 1;	/* Just the home planet for now ("nampla"
				   means "named planet"). */
    spec.num_ships = 0;

    home_nampla.status = HOME_PLANET | POPULATED;
    home_nampla.pop_units = HP_AVAILABLE_POP;
    home_nampla.shipyards = 1;
    /* Everything else was initialized to zero in the earlier call to
	'delete_nampla'. */

    /* Print summary. */
    printf ("\n  Summary for species #%d:\n", species_number);

    printf ("\tName of species: %s\n", spec.name);
    printf ("\tName of home planet: %s\n", home_nampla.name);
    printf ("\t\tCoordinates: %d %d %d #%d\n", spec.x,
	spec.y, spec.z, spec.pn);
    printf ("\tName of government: %s\n", spec.govt_name);
    printf ("\tType of government: %s\n\n", spec.govt_type);

    printf ("\tTech levels: ");
    for (i = 0; i < 6; i++)
    {
	printf ("%s = %d", tech_name[i], spec.tech_level[i]);
	if (i == 2)
	    printf ("\n\t             ");
	else if (i < 5)
	    printf (",  ");
    }

    printf ("\n\n\tFor this species, the required gas is %s (%d%%-%d%%).\n",
	gas_string[spec.required_gas],
	spec.required_gas_min, spec.required_gas_max);

    printf ("\tGases neutral to species:");
    for (i = 0; i < 6; i++)
	printf (" %s ", gas_string[spec.neutral_gas[i]]);

    printf ("\n\tGases poisonous to species:");
    for (i = 0; i < 6; i++)
	printf (" %s ", gas_string[spec.poison_gas[i]]);

    printf ("\n\n\tInitial mining base = %d.%d. Initial manufacturing base = %d.%d.\n",
	home_nampla.mi_base/10, home_nampla.mi_base%10,
	home_nampla.ma_base/10, home_nampla.ma_base%10);
    printf ("\tIn the first turn, %d raw material units will be produced,\n",
	(10 * spec.tech_level[MI] * home_nampla.mi_base)
		/home_planet->mining_difficulty);
    printf ("\tand the total production capacity will be %d.\n\n",
	(spec.tech_level[MA] * home_nampla.ma_base)/10);


    /* Give gamemaster one last chance to change his mind. */
    gamemaster_abort_option ();


    /* Update galaxy file. */
    galaxy_fd = creat ("galaxy.dat", 0600);
    if (galaxy_fd < 0)
    {
	fprintf (stderr, "\n  Cannot create new version of file galaxy.dat!\n");
	exit (-1);
    }

    n = write (galaxy_fd, &galaxy, sizeof (struct galaxy_data));
    if (n != sizeof (struct galaxy_data))
    {
	fprintf (stderr, "\n\tCannot write data to file 'galaxy.dat'!\n\n");
	exit (-1);
    }
    close (galaxy_fd);

    /* Set visited_by bit in star data. */
    species_array_index = (species_number - 1) / 32;
    species_bit_number = (species_number - 1) % 32;
    species_bit_mask = 1 << species_bit_number;
    star->visited_by[species_array_index] |= species_bit_mask;
    save_star_data ();

    /* Create species file. */
    sprintf (filename, "sp%02d.dat\0", species_number);

    species_fd = creat (filename, 0600);
    if (species_fd < 0)
    {
	fprintf (stderr, "\n  Cannot create file %s!\n", filename);
	exit (-1);
    }

    n = write (species_fd, &spec, sizeof(spec));
    if (n != sizeof(spec))
    {
	fprintf (stderr, "\n  Cannot write species data to file!\n");
	exit (-1);
    }

    n = write (species_fd, &home_nampla, sizeof(home_nampla));
    if (n != sizeof(home_nampla))
    {
	fprintf (stderr, "\n  Cannot write home nampla data to file!\n");
	exit (-1);
    }
    close (species_fd);

    /* Create log file for first turn. Write home star system data to it. */
    sprintf (filename, "sp%02d.log\0", species_number);
    log_file = fopen (filename, "w");
    if (log_file == NULL)
    {
	fprintf (stderr, "\n\tCannot open '%s' for writing!\n\n", filename);
	exit (-1);
    }

    fprintf (log_file, "\nScan of home star system for SP %s:\n\n",
				spec.name);
    print_LSN = TRUE;
    nampla_base = &home_nampla;
    scan (home_nampla.x, home_nampla.y, home_nampla.z);

    fclose (log_file);

    exit (0);
}

get_name (name)

char	name[];

{
    int		i;
    char	temp[1024];

again:
    fflush (stdout);
    fgets (temp, 1024, stdin);
    if (strlen(temp) > 32)
    {
	printf ("\n\tIt's too long! 31 characters max!\n");
	printf ("\nEnter again: ");
	goto again;
    }

    i = 0;
    while (1)
    {
	if (temp[i] == '\n') break;
	name[i] = temp[i];
	++i;
    }

    name[i] = '\0';
}
