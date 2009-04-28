
#include <stdio.h>


/* Look-up table for ship defensive/offensive power uses ship->tonnage
   as an index. Each value is equal to 100 * (ship->tonnage)^1.2. The
   'power' subroutine uses recursion to calculate values for tonnages
   over 100. */
short	ship_power[101] =	{0,	/* Zeroth element not used. */
	  100,   230,   374,   528,   690,   859,  1033,  1213,  1397,  1585,
	 1777,  1973,  2171,  2373,  2578,  2786,  2996,  3209,  3424,  3641,
	 3861,  4082,  4306,  4532,  4759,  4988,  5220,  5452,  5687,  5923,
	 6161,  6400,  6641,  6883,  7127,  7372,  7618,  7866,  8115,  8365,
	 8617,  8870,  9124,  9379,  9635,  9893, 10151, 10411, 10672, 10934,
	11197, 11461, 11725, 11991, 12258, 12526, 12795, 13065, 13336, 13608,
	13880, 14154, 14428, 14703, 14979, 15256, 15534, 15813, 16092, 16373,
	16654, 16936, 17218, 17502, 17786, 18071, 18356, 18643, 18930, 19218,
	19507, 19796, 20086, 20377, 20668, 20960, 21253, 21547, 21841, 22136,
	22431, 22727, 23024, 23321, 23619, 23918, 24217, 24517, 24818, 25119 };

long power (tonnage)

short	tonnage;

{
    long	result;
    short	t1, t2;

    if (tonnage > 4068)
    {
	fprintf (stderr,
	    "\n\n\tLong integer overflow will occur in call to 'power(tonnage)'!\n");
	fprintf (stderr, "\t\tActual call is power(%d).\n\n", tonnage);
	exit (-1);
    }

    if (tonnage <= 100)
	result = ship_power[tonnage];
    else
    {
	/* Tonnage is not in table. Break it up into two halves and get
	   approximate result = 1.149 * (x1 + x2), using recursion if
	   necessary. */
	t1 = tonnage/2; t2 = tonnage - t1;
	result = 1149L * (power(t1) + power(t2)) / 1000L;
    }

    return result;
}


extern char	input_line[];

extern FILE	*log_file;

battle_error (species_number)

int	species_number;

{
    fprintf (log_file, "!!! Order ignored:\n");
    fprintf (log_file, "!!! %s", input_line);
    fprintf (log_file, "!!! Missing BATTLE command!\n");
    return;
}


bad_species ()
{
    fprintf (log_file, "!!! Order ignored:\n");
    fprintf (log_file, "!!! %s", input_line);
    fprintf (log_file, "!!! Invalid species name!\n");
    return;
}



bad_argument ()
{
    fprintf (log_file, "!!! Order ignored:\n");
    fprintf (log_file, "!!! %s", input_line);
    fprintf (log_file, "!!! Invalid argument in command.\n");
    return;
}



bad_coordinates ()
{
    fprintf (log_file, "!!! Order ignored:\n");
    fprintf (log_file, "!!! %s", input_line);
    fprintf (log_file, "!!! Invalid coordinates in command.\n");
    return;
}
