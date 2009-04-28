/* This program will generate files HS3 through HS9, which will be used later
	by the HomeSystem program to create home star systems. */


#define THIS_IS_MAIN

#include "fh.h"


int			potential_home_system, species_number;

struct galaxy_data	galaxy;

extern unsigned long	last_random;


main (argc, argv)

int argc;
char *argv[];

{
    int		i, earth_like, home_system_fd, num_planets;

    long	n, num_bytes;

    char	filename[32];

    struct planet_data	*planet_base, *planet, planets[9];


    planet_base = &planets[0];

    /* Seed random number generator. */
    last_random = time(NULL);
    n = rnd(100) + rnd(200) + rnd(300);
    for (i = 0; i < n; i++) rnd(10);

    earth_like = TRUE;
    for (num_planets = 3; num_planets < 10; num_planets++)
    {
	sprintf (filename, "HS%d\0", num_planets);
	printf ("Now doing file '%s'...\n", filename);

	potential_home_system = FALSE;

	while (! potential_home_system)
	    generate_planets (planet_base, num_planets, earth_like);

	home_system_fd = creat (filename, 0600);
	if (home_system_fd < 0)
	{
	    fprintf (stderr, "\n\tCannot create file '%s'!\n\n", filename);
	    exit (-1);
	}

	num_bytes = num_planets * sizeof (struct planet_data);
	n = write (home_system_fd, planet_base, num_bytes);
	if (n != num_bytes)
	{
	    fprintf (stderr, "\n\tCannot write home system data to file '%s'!\n\n",
	    	filename);
	    exit (-1);
	}

	close (home_system_fd);
    }

    exit (0);
}



/* Generate planets. */

int	start_diameter[10]   = {0,  5, 12, 13, 7, 20, 143, 121, 51, 49};
int	start_temp_class[10] = {0, 29, 27, 11, 9, 8,   6,   5,  5,  3};
	/* Values for the planets of Earth's solar system will be used
	   as starting values. Diameters are in thousands of kilometers.
	   The zeroth element of each array is a placeholder and is not
	   used. The fifth element corresponds to the asteroid belt, and
	   is pure fantasy on my part. I omitted Pluto because it is probably
	   a captured planet, rather than an original member of our solar
	   system. */

generate_planets (first_planet, num_planets, earth_like)

struct planet_data	*first_planet;

int	num_planets, earth_like;

{
    int		i, j, n, planet_number, dia, diameter[10], gas_giant, die_size,
		density, grav, g[10], tc, temperature_class[10], pc, temp,
		pressure_class[10], n_rolls, mining_dif, mining_difficulty[10],
		gas[10][5], gas_percent[10][5], first_gas, num_gases_wanted,
		num_gases_found, gas_quantity, total_percent, make_earth,
		special[10], potential;

    char	*cp;

    struct planet_data	*home_planet, *current_planet;


    /* Set flag to indicate if this star system requires an earth-like
	planet. If so, we will zero this flag after we use it. */
    make_earth = earth_like;

    /* Main loop. Generate one planet at a time. */
    for (planet_number = 1; planet_number <= num_planets; planet_number++)
    {
	/* Start with diameters, temperature classes and pressure classes
	    based on the planets in Earth's solar system. */
	if (num_planets > 3)
	{
	    i = (9 * planet_number)/num_planets;
	    dia = start_diameter[i];
	    tc = start_temp_class[i];
	}
	else
	{
	    i = 2 * planet_number  +  1;
	    dia = start_diameter[i];
	    tc = start_temp_class[i];
	}

	/* Randomize the diameter. */
	die_size = dia/4; if (die_size < 2) die_size = 2;
	for (i = 1; i <= 4; i++)
	{
	    if (rnd(100) > 50)
		dia = dia + rnd(die_size);
	    else
		dia = dia - rnd(die_size);
	}

	/* Minimum allowable diameter is 3,000 km. Note that the
	   maximum diameter we can generate is 283,000 km. */
	while (dia < 3) dia += rnd(4);

	diameter[planet_number] = dia;

	/* If diameter is greater than 40,000 km, assume the planet
	   is a gas giant. */
	gas_giant = (dia > 40);

	/* Density will depend on whether or not the planet is a gas giant.
	   Again ignoring Pluto, densities range from 0.7 to 1.6 times the
	   density of water for the gas giants, and from 3.9 to 5.5 for the
	   others. We will expand this range slightly and use 100 times the
	   actual density so that we can use integer arithmetic. */
	if (gas_giant)
	    density = 58 + rnd(56) + rnd(56);
		/* Final values from 60 thru 170. */
	else
	    density = 368 + rnd(101) + rnd(101);
		/* Final values from 370 thru 570. */

	/* Gravitational acceleration is proportional to the mass divided
	   by the radius-squared. The radius is proportional to the
	   diameter, and the mass is proportional to the density times the
	   radius-cubed. The net result is that "g" is proportional to
	   the density times the diameter. Our value for "g" will be
	   a multiple of Earth gravity, and will be further multiplied
	   by 100 to allow us to use integer arithmetic. */
	grav = (density * diameter[planet_number]) / 72;
		/* The factor 72 ensures that "g" will be 100 for
		   Earth (density=550 and diameter=13). */
	g[planet_number] = grav;

	/* Randomize the temperature class obtained earlier. */
	die_size = tc/4; if (die_size < 2) die_size = 2;
	n_rolls = rnd(3) + rnd(3) + rnd(3);
	for (i = 1; i <= n_rolls; i++)
	{
	    if (rnd(100) > 50)
		tc = tc + rnd(die_size);
	    else
		tc = tc - rnd(die_size);
	}

	if (gas_giant)
	{
	    while (tc < 3) tc += rnd(2);
	    while (tc > 7) tc -= rnd(2);
	}
	else
	{
	    while (tc <  1) tc += rnd(3);
	    while (tc > 30) tc -= rnd(3);
	}

	/* Sometimes, planets close to the sun in star systems with less
	   than four planets are too cold. Warm them up a little. */
	if (num_planets < 4  &&  planet_number < 3)
	{
	    while (tc < 12) tc += rnd(4);
	}

	/* Make sure that planets farther from the sun are not warmer
	   than planets closer to the sun. */
	if (planet_number > 1)
	{
	    if (temperature_class[planet_number-1] < tc)
		tc = temperature_class[planet_number-1];
	}

	temperature_class[planet_number] = tc;

	/* Check if this planet should be earth-like. If so, discard all
	   of the above and replace with earth-like characteristics. */
	special[planet_number] = 0;
	if (make_earth && (tc <= 11))
	{
	    make_earth = 0;	/* Once only. */

	    /* Initialize 3rd & 4th gases in case they are not used below. */
	    gas[planet_number][3] = 0;
	    gas_percent[planet_number][3] = 0;
	    gas[planet_number][4] = 0;
	    gas_percent[planet_number][4] = 0;

	    diameter[planet_number] = 11 + rnd(3);
	    g[planet_number] = 93 + rnd(11) + rnd(11) + rnd(5);
	    temperature_class[planet_number] = 9 + rnd(3);
	    pressure_class[planet_number] = 8 + rnd(3);
	    mining_difficulty[planet_number] = 208 + rnd(11) + rnd(11);
	    special[planet_number] = 1;	/* Maybe ideal home planet. */

	    i = 1;
	    total_percent = 0;

	    if (rnd(3) == 1)	/* Give it a shot of ammonia. */
	    {
		gas[planet_number][i] = NH3;
		temp = rnd(30);
		gas_percent[planet_number][i] = temp;
		total_percent += temp;
		++i;
	    }
	    n = i++; 	/* Save index for nitrogen. */

	    if (rnd(3) == 1)	/* Give it a shot of carbon dioxide. */
	    {
		gas[planet_number][i] = CO2;
		temp = rnd(30);
		gas_percent[planet_number][i] = temp;
		total_percent += temp;
		++i;
	    }

	    /* Now do oxygen. */
	    gas[planet_number][i] = O2;
	    temp = rnd(20) + 10;
	    gas_percent[planet_number][i] = temp;
	    total_percent += temp;

	    /* Give the rest to nitrogen. */
	    gas[planet_number][n] = N2;
	    gas_percent[planet_number][n] = 100 - total_percent;

	    continue;
	}

	/* Pressure class depends primarily on gravity. Calculate
	   an approximate value and randomize it. */
	pc = g[planet_number]/10;
	die_size = pc/4; if (die_size < 2) die_size = 2;
	n_rolls = rnd(3) + rnd(3) + rnd(3);
	for (i = 1; i <= n_rolls; i++)
	{
	    if (rnd(100) > 50)
		pc = pc + rnd(die_size);
	    else
		pc = pc - rnd(die_size);
	}

	if (gas_giant)
	{
	    while (pc < 11) pc += rnd(3);
	    while (pc > 29) pc -= rnd(3);
	}
	else
	{
	    while (pc <  0) pc += rnd(3);
	    while (pc > 12) pc -= rnd(3);
	}

	if (grav < 10) pc = 0;
	    /* Planet's gravity is too low to retain an atmosphere. */
	if (tc < 2  ||  tc > 27) pc = 0;
	    /* Planets outside this temperature range have no atmosphere. */

	pressure_class[planet_number] = pc;

	/* Generate gases, if any, in the atmosphere. */
	for (i = 1; i <= 4; i++)	/* Initialize. */
	{
	    gas[planet_number][i] = 0;
	    gas_percent[planet_number][i] = 0;
	}
	if (pc == 0) goto done_gases;	/* No atmosphere. */

	/* Convert planet's temperature class to a value between 1 and 9.
	   We will use it as the start index into the list of 13 potential
	   gases. */
	first_gas = 100*tc/225;
	if (first_gas < 1) first_gas = 1;
	if (first_gas > 9) first_gas = 9;

	/* The following algorithm is something I tweaked until it
	   worked well. */
	num_gases_wanted = (rnd(4) + rnd(4))/2;
	num_gases_found = 0;
	gas_quantity = 0;

get_gases:
	for (i = first_gas; i <= first_gas + 4; i++)
	{
	    if (num_gases_wanted == num_gases_found) break;

	    if (i == HE)	/* Treat Helium specially. */
	    {
		if (rnd(3) > 1) continue; /* Don't want too many He planets. */
		if (tc > 5) continue;	  /* Too hot for helium. */
		++num_gases_found;
		gas[planet_number][num_gases_found] = HE;
		temp = rnd(20);
		gas_percent[planet_number][num_gases_found] = temp;
		gas_quantity += temp;
	    }
	    else	/* Not Helium. */
	    {
		if (rnd(3) == 3) continue;
		++num_gases_found;
		gas[planet_number][num_gases_found] = i;
		if (i == O2)
		    temp = rnd(50);	/* Oxygen is self-limiting. */
		else
		    temp = rnd(100);
		gas_percent[planet_number][num_gases_found] = temp;
		gas_quantity += temp;
	    }
	}

	if (num_gases_found == 0) goto get_gases;	/* Try again. */

	/* Now convert gas quantities to percentages. */
	total_percent = 0;
	for (i = 1; i <= num_gases_found; i++)
	{
	    gas_percent[planet_number][i] =
		100 * gas_percent[planet_number][i] / gas_quantity;
	    total_percent += gas_percent[planet_number][i];
	}

	/* Give leftover to first gas. */
	gas_percent[planet_number][1] += 100 - total_percent;

  done_gases:

	/* Get mining difficulty. Basically, mining difficulty is
	   proportional to planetary diameter with randomization and an
	   occasional big surprise. Actual values will range between 0.30
	   and 10.00. Again, the actual value will be multiplied by 100
	   to allow use of integer arithmetic. */
	mining_dif = 0;
	while (mining_dif < 30  || mining_dif > 1000)
	    mining_dif = (rnd(3) + rnd(3) + rnd(3) - rnd(4)) * rnd(dia)
				+ rnd(20) + rnd(20);

	mining_difficulty[planet_number] = mining_dif;
    }

    /* Copy planet data to structure. */
    potential_home_system = FALSE;
    current_planet = first_planet;
    for (i = 1; i <= num_planets; i++)
    {
	/* Initialize all bytes of record to zero. */
	cp = (char *) current_planet;
	for (j = 0; j < sizeof (struct planet_data); j++)
		*cp++ = 0;

	current_planet->diameter = diameter[i];
	current_planet->gravity = g[i];
	current_planet->mining_difficulty = mining_difficulty[i];
	current_planet->temperature_class = temperature_class[i];
	current_planet->pressure_class = pressure_class[i];

	current_planet->special = special[i];
	if (special[i] == 1)
	{
	    home_planet = current_planet;
	    potential_home_system = TRUE;
	}

	for (n = 0; n < 4; n++)
	{
	    current_planet->gas[n] = gas[i][n+1];
	    current_planet->gas_percent[n] = gas_percent[i][n+1];
	}

	++current_planet;
    }

    if (! potential_home_system) return;

    /* If this is a potential home system, make sure it passes certain
	tests. */
    current_planet = first_planet;
    potential = 0;
    for (i = 1; i <= num_planets; i++)
    {
	n = LSN (current_planet, home_planet);

	potential += 20000L /
	    ((long) (n + 3) * (long) (50 + current_planet->mining_difficulty));

	++current_planet;
    }

    if (potential > 53  &&  potential < 57)
	;
    else
	potential_home_system = FALSE;
}



int LSN (current_planet, home_planet)

struct planet_data	*current_planet, *home_planet;

{
    /* This routine provides an approximate LSN for a planet. It assumes
	that oxygen is required and any gas that does not appear on the
	home planet is poisonous. */

    int		i, j, k, ls_needed, poison;

    ls_needed = 0;

    j = current_planet->temperature_class - home_planet->temperature_class;
    if (j < 0) j = -j;
    ls_needed = 2 * j;		/* Temperature class. */

    j = current_planet->pressure_class - home_planet->pressure_class;
    if (j < 0) j = -j;
    ls_needed += 2 * j;		/* Pressure class. */

    /* Check gases. Assume oxygen is not present. */
    ls_needed += 2;
    for (j = 0; j < 4; j++)	/* Check gases on planet. */
    {
	if (current_planet->gas[j] == 0) continue;

	if (current_planet->gas[j] == O2) ls_needed -= 2;

	poison = TRUE;
	for (k = 0; k < 4; k++)	/* Compare with home planet. */
	{
	    if (current_planet->gas[j] == home_planet->gas[k])
	    {
		poison = FALSE;
		break;
	    }
	}
	if (poison) ls_needed += 2;
    }

    return ls_needed;
}
