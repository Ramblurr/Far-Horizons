
#include "fh.h"


extern int			correct_spelling_required;
extern char			*input_line_pointer;
extern struct nampla_data	*nampla;


int get_transfer_point ()
{
    char	*temp_ptr;


    /* Find out if it is a ship or a planet. First try for a correctly
	spelled ship name. */
    temp_ptr = input_line_pointer;
    correct_spelling_required = TRUE;
    if (get_ship ()) return TRUE;

    /* Probably not a ship. See if it's a planet. */
    input_line_pointer = temp_ptr;
    if (get_location ()) return (nampla != NULL);

    /* Now check for an incorrectly spelled ship name. */
    input_line_pointer = temp_ptr;
    if (get_ship ()) return TRUE;

    return FALSE;
}
