
#include "fh.h"


extern int			abbr_type, g_spec_number;
extern char			input_line[256], g_spec_name[32];
extern FILE			*log_file;
extern struct species_data	*species;


do_ALLY_command ()

{
    int		i, array_index, bit_number;

    long	bit_mask;


    /* Get name of species that is being declared an ally. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid or missing argument in ALLY command.\n");
	return;
    }

    /* Get array index and bit mask. */
    array_index = (g_spec_number - 1) / 32;
    bit_number = (g_spec_number - 1) % 32;
    bit_mask = 1 << bit_number;

    /* Check if we've met this species and make sure it is not an enemy. */
    if ((species->contact[array_index] & bit_mask)  ==  0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You can't declare alliance with a species you haven't met.\n");
	return;
    }

    /* Set/clear the appropriate bit. */
    species->ally[array_index] |= bit_mask;	/* Set ally bit. */
    species->enemy[array_index] &= ~bit_mask;	/* Clear enemy bit. */

    /* Log the result. */
    log_string ("    Alliance was declared with ");
    if (bit_mask == 0)
	log_string ("ALL species");
    else
    {
	log_string ("SP ");
	log_string (g_spec_name);
    }
    log_string (".\n");
}
