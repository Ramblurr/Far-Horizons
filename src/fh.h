
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "engine.h"
#include "galaxy.h"
#include "star.h"




/* Gases in planetary atmospheres. */
#define	H2	1	/* Hydrogen */
#define	CH4	2	/* Methane */
#define	HE	3	/* Helium */
#define	NH3	4	/* Ammonia */
#define	N2	5	/* Nitrogen */
#define	CO2	6	/* Carbon Dioxide */
#define	O2	7	/* Oxygen */
#define	HCL	8	/* Hydrogen Chloride */
#define	CL2	9	/* Chlorine */
#define	F2	10	/* Fluorine */
#define	H2O	11	/* Steam */
#define	SO2	12	/* Sulfur Dioxide */
#define	H2S	13	/* Hydrogen Sulfide */

#include "planet.h"

#include "species.h"
#include "item.h"
#include "nampla.h"
#include "ship.h"
#include "transaction.h"
#include "command.h"

/* Global data used in most or all programs. */

#ifdef THIS_IS_MAIN

    char
	type_char[] = " dD g";
    char
	color_char[] = " OBAFGKM";
    char
	size_char[] = "0123456789";
    char
	gas_string[14][4] =
	{
		"   ",	"H2",	"CH4",	"He",	"NH3",	"N2",	"CO2",
		"O2",	"HCl",	"Cl2",	"F2",	"H2O",	"SO2",	"H2S"
	};

#else

    extern char			type_char[];
    extern char			color_char[];
    extern char			size_char[];
    extern char			gas_string[14][4];
    extern char			tech_abbr[6][4];
    extern char			tech_name[6][16];
    extern int			data_in_memory[MAX_SPECIES];
    extern int			data_modified[MAX_SPECIES];
    extern int			num_new_namplas[MAX_SPECIES];
    extern int			num_new_ships[MAX_SPECIES];
    extern struct species_data	spec_data[MAX_SPECIES];
    extern struct nampla_data	*namp_data[MAX_SPECIES];
    extern struct ship_data	*ship_data[MAX_SPECIES];
    extern char			item_name[MAX_ITEMS][32];
    extern char			item_abbr[MAX_ITEMS][4];
    extern long			item_cost[MAX_ITEMS];
    extern short		item_carry_capacity[MAX_ITEMS];
    extern char			item_critical_tech[MAX_ITEMS];
    extern short		item_tech_requirment[MAX_ITEMS];
    extern char			ship_abbr[NUM_SHIP_CLASSES][4];
    extern char			ship_type[3][2];
    extern short		ship_tonnage[NUM_SHIP_CLASSES];
    extern short		ship_cost[NUM_SHIP_CLASSES];
    extern char			command_abbr[NUM_COMMANDS][4];
    extern char			command_name[NUM_COMMANDS][16];
    
#endif
