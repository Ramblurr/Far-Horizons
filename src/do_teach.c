
#include "fh.h"


extern int			abbr_index, species_number, g_spec_number,
				first_pass, num_transactions;
extern char			input_line[256], g_spec_name[32],
				*input_line_pointer;
extern long			value;
extern FILE			*log_file;
extern struct species_data	*species;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_TEACH_command ()

{
    int		i, tech, contact_word_number, contact_bit_number,
		max_level_specified, need_technology;

    char	*temp_ptr;

    short	max_tech_level;

    long	contact_mask;


    /* Get technology. */
    temp_ptr = input_line_pointer;
    if (get_class_abbr() != TECH_ID)
    {
	need_technology = TRUE;		/* Sometimes players accidentally
					   reverse the arguments. */
	input_line_pointer = temp_ptr;
    }
    else
    {
	need_technology = FALSE;
	tech = abbr_index;
    }

    /* See if a maximum tech level was specified. */
    max_level_specified = get_value ();
    if (max_level_specified)
    {
	max_tech_level = value;
	if (max_tech_level > species->tech_level[tech])
		max_tech_level = species->tech_level[tech];
    }
    else
	max_tech_level = species->tech_level[tech];

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

    /* Get species to transfer knowledge to. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in TEACH command.\n");
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
        fprintf (log_file, "!!! You can't TEACH a species you haven't met.\n");
        return;
    }

    if (species->enemy[contact_word_number] & contact_mask)
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! You can't TEACH an ENEMY.\n");
	return;
    }

    if (first_pass) return;

    /* Define this transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    i = num_transactions++;
    transaction[i].type = KNOWLEDGE_TRANSFER;
    transaction[i].donor = species_number;
    transaction[i].recipient = g_spec_number;
    transaction[i].value = tech;
    strcpy (transaction[i].name1, species->name);
    transaction[i].number3 = max_tech_level;
}
