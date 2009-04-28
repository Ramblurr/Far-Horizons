
#include "fh.h"


extern int			first_pass, doing_production, g_spec_number,
				num_transactions, species_number;
extern char			input_line[256], g_spec_name[32];
extern FILE			*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base;
extern struct ship_data		*ship_base;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_ALLIES_command ()

{
    int		i, n, contact_word_number, contact_bit_number;

    long	cost, contact_mask;

    struct species_data		*alien;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (stderr, "\n\n\tMissing PRODUCTION order!\n\n");
	exit (-1);
    }

    /* Get name of alien species. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in ALLIES command.\n");
	return;
    }

    /* Check if we've met this species. */
    contact_word_number = (g_spec_number - 1) / 32;
    contact_bit_number = (g_spec_number - 1) % 32;
    contact_mask = 1 << contact_bit_number;
    if ((species->contact[contact_word_number] & contact_mask)  ==  0)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You cannot use ALLIES on a species you haven't met.\n");
	return;
    }

    /* Check if sufficient funds are available. */
    cost = 200;
    if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Log the result. */
    log_string ("    A project to determine the potential allies of SP ");
    log_string (g_spec_name);
    log_string (" was funded at a cost of ");
    log_long (cost);
    log_string (".\n");

    if (first_pass) return;

    /* Create an appropriate interspecies transaction. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    n = num_transactions++;
    transaction[n].type = ALLIES_ORDER;
    transaction[n].number1 = species_number;
    transaction[n].number2 = g_spec_number;
    strcpy (transaction[n].name1, species->name);
    strcpy (transaction[n].name2, g_spec_name);
}
