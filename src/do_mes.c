
#include "fh.h"


extern int			abbr_type, first_pass, species_number,
				g_spec_number, num_transactions, end_of_file;
extern char			input_line[256], g_spec_name[32],
				*input_line_pointer;
extern FILE			*log_file, *input_file;
extern struct species_data	*species;
extern struct trans_data	transaction[MAX_TRANSACTIONS];


do_MESSAGE_command ()

{
    int		i, message_number, message_fd, bad_species,
		unterminated_message;

    char	c1, c2, c3, filename[32];

    FILE	*message_file;


    /* Get destination of message. */
    if (! get_species_name())
    {
	fprintf (log_file, "!!! Order ignored:\n");
	fprintf (log_file, "!!! %s", input_line);
	fprintf (log_file, "!!! Invalid species name in MESSAGE command.\n");
	bad_species = TRUE;
    }
    else
	bad_species = FALSE;

    /* Generate a random number, create a filename with it, and use it to
	store message. */
    if (! first_pass  &&  ! bad_species)
    {
	while (1)
	{
	    /* Generate a random filename. */
	    message_number = rnd (32000);
	    sprintf (filename, "m%d.msg\0", message_number);

	    /* Make sure that this filename is not already in use. */
	    message_fd = open (filename, 0);
	    if (message_fd < 0) break;

	    /* File already exists. Try again. */
	    close (message_fd);
	}

	message_file = fopen (filename, "w");
	if (message_file == NULL)
	{
	    fprintf (stderr,
		"\n\n!!! Cannot open message file '%s' for writing !!!\n\n",
		filename);
	    exit (-1);
	}
    }

    /* Copy message to file. */
    unterminated_message = FALSE;
    while (1)
    {
	/* Read next line. */
	input_line_pointer = fgets (input_line, 256, input_file);
	if (input_line_pointer == NULL)
	{
	    unterminated_message = TRUE;
	    end_of_file = TRUE;
	    break;
	}

	skip_whitespace ();

	c1 = *input_line_pointer++;
	c2 = *input_line_pointer++;
	c3 = *input_line_pointer;

	c1 = toupper (c1);
	c2 = toupper (c2);
	c3 = toupper (c3);

	if (c1 == 'Z'  &&  c2 == 'Z'  &&  c3 == 'Z') break;

	if (! first_pass  &&  ! bad_species) fputs (input_line, message_file);
    }

    if (bad_species) return;

    /* Log the result. */
    log_string ("    A message was sent to SP ");
    log_string (g_spec_name);
    log_string (".\n");

    if (unterminated_message)
    {
	log_string ("  ! WARNING: Message was not properly terminated with ZZZ!");
	log_string (" Any orders that follow the message will be assumed");
	log_string (" to be part of the message and will be ignored!\n");
    }

    if (first_pass) return;

    fclose (message_file);

    /* Define this message transaction and add to list of transactions. */
    if (num_transactions == MAX_TRANSACTIONS)
    {
	fprintf (stderr, "\n\n\tERROR! num_transactions > MAX_TRANSACTIONS!\n\n");
	exit (-1);
    }

    i = num_transactions++;
    transaction[i].type = MESSAGE_TO_SPECIES;
    transaction[i].value = message_number;
    transaction[i].number1 = species_number;
    strcpy (transaction[i].name1, species->name);
    transaction[i].number2 = g_spec_number;
    strcpy (transaction[i].name2, g_spec_name);
}
