
#include <stdio.h>

main (argc, argv)

int argc;
char *argv[];

{
    int		i, bit_number, word_number, species_number;

    long	mask, bit_mask;


    /* Check for valid command line. */
    if (argc == 1)
    {
	fprintf (stderr, "\n    Usage: Mask species_number species_number ...\n\n");
	exit (-1);
    }

    mask = 0;
    for (i = 1; i < argc; i++)
    {
	species_number = atoi (argv[i]);
	word_number = (species_number - 1) / 32; 
	bit_number = (species_number - 1) % 32;
	bit_mask = 1 << bit_number;
	printf ("Species number = %d, word number = %d, bit number = %d, bit mask = %lx\n",
		species_number, word_number, bit_number, bit_mask);
	mask |= bit_mask;
    }

    printf ("\nFinal single-word bit mask = %lx\n", mask);
}
