
#include "fh.h"


extern int			abbr_type, abbr_index, species_number,
				g_spec_number, first_pass, num_transactions;
extern long			value;
extern char			input_line[256], g_spec_name[32],
				*input_line_pointer;
extern FILE			*log_file;
extern struct species_data	*species;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_SEND_command ()

{
    int		i, n, found, contact_word_number, contact_bit_number;

    char	*temp_pointer;

    long	num_available, contact_mask, item_count;

    struct nampla_data		*nampla1, *nampla2;


    /* Get number of EUs to transfer. */
    i = get_value ();

    /* Make sure value is meaningful. */
    if (i == 0  ||  value < 0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid item count in SEND command.\n");
	return;
    }
    item_count = value;

    num_available = species->econ_units;
    if (item_count == 0) item_count = num_available;
    if (item_count == 0) return;
    if (num_available < item_count)
    {
	if (num_available == 0)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! You do not have any EUs!\n");
	    return;
	}
	fprintf (log_file, "! WARNING: %s", input_line);
	fprintf (log_file, "! You do not have %ld EUs! Substituting %ld for %ld.\n",
	    item_count, num_available, item_count);
	item_count = num_available;
    }

    /* Get destination of transfer. */
    found = get_species_name ();
    if (! found)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in SEND command.\n");
	return;
    }

    /* Check if we've met this species and make sure it is not an enemy. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask)  ==  0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You can't SEND to a species you haven't met.\n");
	return;
    }
    if (species->enemy[contact_word_number] & contact_mask)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You may not SEND economic units to an ENEMY.\n");
	return;
    }

    /* Make the transfer and log the result. */
    log_string ("    ");
    log_long (item_count);  log_string (" economic unit");
    if (item_count > 1)
	log_string ("s were");
    else
	log_string (" was");
    log_string (" sent to SP ");
    log_string (g_spec_name);
    log_string (".\n");
    species->econ_units -= item_count;

    if (first_pass) return;

    /* Define this transaction. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    n = num_transactions++;
    transaction[n].type = EU_TRANSFER;
    transaction[n].donor = species_number;
    transaction[n].recipient = g_spec_number;
    transaction[n].value = item_count;
    strcpy (transaction[n].name1, species->name);
    strcpy (transaction[n].name2, g_spec_name);

    /* Make the transfer to the alien. */
    spec_data[g_spec_number - 1].econ_units += item_count;
    data_modified[g_spec_number - 1] = TRUE;
}
