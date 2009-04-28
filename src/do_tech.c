
#include "fh.h"


extern int			abbr_index, first_pass, species_number,
				g_spec_number, num_transactions;
extern char			input_line[256], g_spec_name[32];
extern long			value;
extern FILE			*log_file;
extern struct species_data	*species;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_TECH_command ()

{
    int		i, tech, contact_word_number, contact_bit_number,
		max_level_specified, max_tech_level, max_cost_specified,
		need_technology;

    long	contact_mask, max_cost;



    /* See if a maximum cost was specified. */
    max_cost_specified = get_value ();
    if (max_cost_specified)
	max_cost = value;
    else
	max_cost = 0;

    /* Get technology. */
    if (get_class_abbr() != TECH_ID)
	need_technology = TRUE;		/* Sometimes players accidentally
					reverse the arguments. */
    else
    {
	need_technology = FALSE;
	tech = abbr_index;
    }

    /* See if a maximum tech level was specified. */
    max_level_specified = get_value ();
    max_tech_level = value;

    /* Get species to transfer tech to. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in TECH command.\n");
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
	fprintf (log_file, "!!! You can't transfer tech to a species you haven't met.\n");
	return;
    }
    if (species->enemy[contact_word_number] & contact_mask)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You can't transfer tech to an ENEMY.\n");
	return;
    }

    /* Get the technology now if it wasn't obtained above. */
    if (need_technology)
    {
	if (get_class_abbr() != TECH_ID)
	{
	    fprintf (log_file, "!!! Order ignored:\n");
	    fprintf (log_file, "!!! %s", input_line);
	    fprintf (log_file, "!!! Invalid or missing technology!\n");
	    return;
	}
	tech = abbr_index;
    }

    /* Make sure there isn't already a transfer of the same technology from
	the same donor species to the same recipient species. */
    for (i = 0; i < num_transactions; i++)
    {
	if (transaction[i].type != TECH_TRANSFER) continue;
	if (transaction[i].value != tech) continue;
	if (transaction[i].number1 != species_number) continue;
	if (transaction[i].number2 != g_spec_number) continue;

	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You can't transfer the same tech to the same species more than once!\n");
	return;
    }

    /* Log the result. */
    log_string ("    Will attempt to transfer ");
    log_string (tech_name[tech]);
    log_string (" technology to SP ");
    log_string (g_spec_name);
    log_string (".\n");

    if (first_pass) return;

    /* Define this transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    i = num_transactions++;
    transaction[i].type = TECH_TRANSFER;
    transaction[i].donor = species_number;
    transaction[i].recipient = g_spec_number;
    transaction[i].value = tech;
    strcpy (transaction[i].name1, species->name);
    transaction[i].number1 = max_cost;
    strcpy (transaction[i].name2, g_spec_name);
    if (max_level_specified  &&  (max_tech_level < species->tech_level[tech]))
	transaction[i].number3 = max_tech_level;
    else
	transaction[i].number3 = species->tech_level[tech];
}
