
/* This program will create an ASCII map of the galaxy and write the result
   to file "galaxy.map".  It will paginate the output based on how many
   columns your printer or display can handle.  Pages are separated by
   formfeed characters. If you do not like the results, you can always
   edit the output file with any text editor. */

#define THIS_IS_MAIN

#include "fh.h"


int	star_here[MAX_DIAMETER][MAX_DIAMETER];

struct galaxy_data	galaxy;

extern int	num_stars;

extern struct star_data	*star_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, n, x, y, z, line, x_count, n_columns, x_increment,
		page_count, page, left_x, galactic_diameter, star_index;

    FILE	*outfile;

    struct star_data	*star;


    /* Check for valid command line. */
    if (argc != 1)
    {
	fprintf (stderr, "\n\tUsage: MapGalaxy\n\n");
	fprintf (stderr, "\tResults will be written to file galaxy.map\n\n");
	exit (-1);
    }

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();

    galactic_diameter = 2 * galaxy.radius;

    /* Determine number of pages that will be needed to contain the
	complete map. */
    printf ("\nHow many columns (eg. 80 or 132) can your printer or display handle? ");
    fflush (stdout);
    scanf ("%d", &n_columns);

    x_increment = (n_columns - 4) / 6;	/* 4 columns for left margin
					   plus 6 per star. */
    page_count = (2 * galaxy.radius  +  x_increment  -  1) / x_increment;

    printf ("\nI will generate %d page(s).\n\n", page_count);

    /* For each star, set corresponding element of star_here[] to index
	into star array. */
    for (x = 0; x < galactic_diameter; x++)	/* Initialize array. */
     for (y = 0; y < galactic_diameter; y++)
	star_here[x][y] = -1;

    star = star_base;
    for (star_index = 0; star_index < num_stars; star_index++)
    {
	x = star->x;
	y = star->y;
	star_here[x][y] = star_index;
	++star;
    }

    /* Create output file. */
    outfile = fopen ("galaxy.map", "w");
    if (outfile == NULL)
    {
	fprintf (stderr, "\n\tCannot create file galaxy.map!\n");
	exit (-1);
    }

    /* Outermost loop will count pages. */
    left_x = 0;
    for (page = 1; page <= page_count; page++)
    {
	/* Next-to-outermost loop will control y-coordinates. */
	for (y = 2*galaxy.radius - 1; y >= 0; y--)
	{
	    /* Next-to-innermost loop will count the 4 lines that make up
		each star box. Fifth and sixth lines are generated only at
		the very bottom of the page. */
	    for (line = 1; line <= 6; line++)
	    {
		x = left_x;

		/* Do left margin of first page. */
		if (x == 0  &&  page == 1)
		{
		    switch (line)
		    {
			case 1: fprintf (outfile, "   -"); break;
			case 2: fprintf (outfile, "   |"); break;
			case 3: fprintf (outfile, "%2d |", y); break;
			case 4: if (n_columns < 100) fprintf (outfile, "   |");
				break;
			case 5: if (y == 0) fprintf (outfile, " Y -"); break;
			case 6: if (y == 0) fprintf (outfile, "  X "); break;
		    }
		}

		/* Innermost loop will control x-coordinate. */
		for (x_count = 1; x_count <= x_increment; x_count++)
		{
		    if (x == galactic_diameter) break;

		    star_index = star_here[x][y];
		    star = (struct star_data *) star_base;
		    if (star_index > 0) star += star_index;

		    switch (line)
		    {
			case 1:
			    fprintf (outfile, "------");
			    break;

			case 2:
			    if (star_index >= 0)
			    {
				z =star->z;
				if (z < 10)
				    fprintf (outfile, "%3d  |", z);
				else
				    fprintf (outfile, "%4d |", z);
			    }
			    else
				fprintf (outfile, "     |");
			    break;

			case 3:
			    if (star_index >= 0)
				fprintf (outfile, " %c%c%c |",
				    type_char[star->type],
				    color_char[star->color],
				    size_char[star->size]);
			    else
				fprintf (outfile, "     |");
			    break;

			case 4:
			    if (n_columns < 100) fprintf (outfile, "     |");
			    break;

			case 5:
			    if (y == 0) fprintf (outfile, "------");
			    break;

			case 6:
			    if (y == 0) fprintf (outfile, "  %2d  ", x);
			    break;
		    }

		    ++x;
		}

		if ( (line < 4)  ||  (line == 4  &&  n_columns < 100) )
		    fprintf (outfile, "\n");	/* End of line. */

		if (y == 0  &&  line == 5) fprintf (outfile, "\n");
	    }
	}

	fprintf (outfile, "\n\f");	/* Formfeed character. */
	left_x += x_increment;
    }

    /* Clean up and exit. */
    fclose (outfile);
    exit (0);
}
