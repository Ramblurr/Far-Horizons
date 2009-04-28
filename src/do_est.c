
#include "fh.h"


extern int			first_pass, doing_production, g_spec_number;
extern char			input_line[256], g_spec_name[32];
extern FILE			*log_file;
extern struct species_data	*species;
extern struct nampla_data	*nampla_base;
extern struct ship_data		*ship_base;


do_ESTIMATE_command ()

{
    int		i, max_error, estimate[6], contact_word_number,
		contact_bit_number;

    long	cost, contact_mask;

    struct species_data		*alien;


    /* Check if this order was preceded by a PRODUCTION order. */
    if (!doing_production)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Missing PRODUCTION order!\n");
	return;
    }

    /* Get name of alien species. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in ESTIMATE command.\n");
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
	fprintf (log_file, "!!! You can't do an estimate of a species you haven't met.\n");
	return;
    }

    /* Check if sufficient funds are available. */
    cost = 25;
    if (check_bounced (cost))
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Insufficient funds to execute order.\n");
	return;
    }

    /* Log the result. */
    if (first_pass)
    {
	log_string ("    An estimate of the technology of SP ");
	log_string (g_spec_name);
	log_string (" was made at a cost of ");
	log_long (cost);
	log_string (".\n");
	return;
    }

    /* Make the estimates. */
    alien = &spec_data[g_spec_number - 1];
    for (i = 0; i < 6; i++)
    {
	max_error = (int) alien->tech_level[i] - (int) species->tech_level[i];
	if (max_error < 1) max_error = 1;
	estimate[i] = (int) alien->tech_level[i] + rnd((2 * max_error) + 1)
		- (max_error + 1);
	if (alien->tech_level[i] == 0) estimate[i] = 0;
	if (estimate[i] < 0) estimate[i] = 0;
    }

    log_string ("    Estimate of the technology of SP ");
    log_string (alien->name);
    log_string (" (government name '");
    log_string (alien->govt_name);
    log_string ("', government type '");
    log_string (alien->govt_type);
    log_string ("'):\n      MI = ");   log_int (estimate[MI]);
    log_string (", MA = ");   log_int (estimate[MA]);
    log_string (", ML = ");   log_int (estimate[ML]);
    log_string (", GV = ");   log_int (estimate[GV]);
    log_string (", LS = ");   log_int (estimate[LS]);
    log_string (", BI = ");   log_int (estimate[BI]);
    log_string (".\n");
}
