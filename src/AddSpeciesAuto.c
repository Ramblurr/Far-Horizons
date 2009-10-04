
/* This is a modified non-interactive version of AddSpecies */


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

    /* Initialize all bytes of record to zero. */
    cp = (char *) &spec;
    for (i = 0; i < sizeof (struct species_data); i++)
	*cp++ = 0;
	
    /* Check for valid command line. */
    if (argc != 14)
    {
      fprintf (stderr, "\n  Usage: AddSpeciesAuto <params> \n");
	fprintf (stderr, "    See Auto.pl for parameter details.\n\n");
	exit (-1);
    }

    galaxy.num_species++;
    
    species_number = atoi (argv[1]);

    j = strlen (argv[2]);
    if (j < 5)
    {
        printf("\n\n\tERROR!  Species '%s' name too short (min 5 chars required)\n", argv[2]);
        exit(-1);
    }
    if (j > 31)
    {
        printf("\n\n\tERROR!  Species '%s' name too long (max 31 chars required)\n", argv[2]);
        exit(-1);
    }
    strncpy(&spec.name, argv[2], 31);

    for (i = 0; i < j; i++)
    {
	c = spec.name[i];
	if (c == '$'  ||  c == '!'  ||  c == '"')
	{
	    printf ("\n\n\tERROR!  Invalid character '%c' in species name!\n", c);
            exit(-1);
	}
    }

    delete_nampla (&home_nampla);		/* Set everything to zero. */
    if (strlen(argv[3]) > 31)
    {
        printf("\n\n\tERROR!  Home planet '%s' name too long (max 31 chars required)\n", argv[3]);
        exit(-1);
    }
    strncpy(&home_nampla.name, argv[3], 31);
    
    if (strlen(argv[4]) > 31)
    {
        printf("\n\n\tERROR!  Government '%s' name too long (max 31 chars required)\n", argv[4]);
        exit(-1);
    }
    strncpy(&spec.govt_name, argv[4], 31);
    
    if (strlen(argv[5]) > 31)
    {
        printf("\n\n\tERROR!  Government '%s' type too long (max 31 chars required)\n", argv[5]);
        exit(-1);
    }
    strncpy(&spec.govt_type, argv[5], 31);

    home_nampla.x = spec.x = x = atoi(argv[6]);
    home_nampla.y = spec.y = y = atoi(argv[7]);
    home_nampla.z = spec.z = z = atoi(argv[8]);
    home_nampla.pn = spec.pn = pn = atoi(argv[9]);

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
      printf ("\n\tERROR: There is no star at %d %d %d!\n", x, y, z);
	exit(-1);
    }

    printf ("\nScan of star system:\n\n");
    print_LSN = FALSE;
    log_file = stdout;
    scan (x, y, z);

    if (pn > star->num_planets)
    {
      printf ("\n\tERROR: Planet number (%d) does not exist!\n", pn );
	exit(-1);
    }

    /* Get pointer to planet. */
    home_nampla.planet_index = star->planet_index + pn - 1;
    home_planet = planet_base + (long) home_nampla.planet_index;

    /* Get player-specified tech levels. */
    printf ("\n");
    for (i = ML; i <= BI; i++)
    {
	spec.tech_level[i] = atoi(argv[10 + i - ML]);
    }

    /* Check tech levels. */
    n = 0;
    for (i = ML; i <= BI; i++) n += spec.tech_level[i];
    if (n > 15)
    {
        printf ("\n\tERROR! ML + GV + LS + BI is greater than 15!\n\n");
        exit(-1);
    }

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
    spec.required_gas = req_gas = 7; // Assume O2 is required

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
        printf ("\n\tERROR! Planet does not have %s(%d)!\n", gas_string[req_gas], req_gas);
	exit(-1);
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
