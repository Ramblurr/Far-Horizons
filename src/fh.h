
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "engine.h"






#include "galaxy.h"



/* Star types. */
#define	DWARF		1
#define	DEGENERATE	2
#define	MAIN_SEQUENCE	3
#define	GIANT		4

/* Star Colors. */
#define	BLUE		1
#define	BLUE_WHITE	2
#define	WHITE		3
#define	YELLOW_WHITE	4
#define	YELLOW		5
#define	ORANGE		6
#define	RED		7

struct star_data
{
    char	x,y,z;		/* Coordinates. */
    char	type;		/* Dwarf, degenerate, main sequence or giant. */
    char	color;		/* Star color. Blue, blue-white, etc. */
    char	size;		/* Star size, from 0 thru 9 inclusive. */
    char	num_planets;	/* Number of usable planets in star system. */
    char	home_system;	/* TRUE if this is a good potential home system. */
    char	worm_here;	/* TRUE if wormhole entry/exit. */
    char	worm_x, worm_y, worm_z;
    short	reserved1;	/* Reserved for future use. Zero for now. */
    short	reserved2;	/* Reserved for future use. Zero for now. */
    short	planet_index;	/* Index (starting at zero) into the file
				   "planets.dat" of the first planet in the
				   star system. */
    long	message;	/* Message associated with this star system,
					if any. */
    long	visited_by[NUM_CONTACT_WORDS];
				/* A bit is set if corresponding species has
					been here. */
    long	reserved3;	/* Reserved for future use. Zero for now. */
    long	reserved4;	/* Reserved for future use. Zero for now. */
    long	reserved5;	/* Reserved for future use. Zero for now. */
};


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

/* Tech level ids. */
#define	MI	0	/* Mining tech level. */
#define	MA	1	/* Manufacturing tech level. */
#define	ML	2	/* Military tech level. */
#define	GV	3	/* Gravitics tech level. */
#define	LS	4	/* Life Support tech level. */
#define	BI	5	/* Biology tech level. */

#include "species.h"

#include "item.h"



#include "nampla.h"


#include "ship.h"

/* Interspecies transactions. */

#define MAX_TRANSACTIONS		1000

#define	EU_TRANSFER			1
#define MESSAGE_TO_SPECIES		2
#define BESIEGE_PLANET			3
#define SIEGE_EU_TRANSFER		4
#define TECH_TRANSFER			5
#define DETECTION_DURING_SIEGE		6
#define SHIP_MISHAP			7
#define ASSIMILATION			8
#define	INTERSPECIES_CONSTRUCTION	9
#define TELESCOPE_DETECTION		10
#define ALIEN_JUMP_PORTAL_USAGE		11
#define	KNOWLEDGE_TRANSFER		12
#define	LANDING_REQUEST			13
#define	LOOTING_EU_TRANSFER		14
#define ALLIES_ORDER            15

struct trans_data
{
    int		type;		/* Transaction type. */
    short	donor, recipient;
    long	value;		/* Value of transaction. */
    char	x, y, z, pn;	/* Location associated with transaction. */
    long	number1;	/* Other items associated with transaction.*/
    char	name1[40];
    long	number2;
    char	name2[40];
    long	number3;
    char	name3[40];
};


/* Command codes. */
#define	UNDEFINED	0
#define ALLY		1
#define AMBUSH		2
#define	ATTACK		3
#define	AUTO		4
#define BASE		5
#define	BATTLE		6
#define	BUILD		7
#define	CONTINUE	8
#define	DEEP		9
#define DESTROY		10
#define	DEVELOP		11
#define DISBAND		12
#define	END		13
#define ENEMY		14
#define	ENGAGE		15
#define	ESTIMATE	16
#define	HAVEN		17
#define HIDE		18
#define	HIJACK		19
#define IBUILD		20
#define ICONTINUE	21
#define	INSTALL		22
#define INTERCEPT	23
#define	JUMP		24
#define	LAND		25
#define	MESSAGE		26
#define	MOVE		27
#define	NAME		28
#define NEUTRAL		29
#define	ORBIT		30
#define PJUMP		31
#define	PRODUCTION	32
#define	RECYCLE		33
#define REPAIR		34
#define	RESEARCH	35
#define	SCAN		36
#define	SEND		37
#define	SHIPYARD	38
#define	START		39
#define SUMMARY		40
#define	SURRENDER	41
#define TARGET		42
#define	TEACH		43
#define	TECH		44
#define TELESCOPE	45
#define TERRAFORM	46
#define	TRANSFER	47
#define UNLOAD		48
#define	UPGRADE		49
#define	VISITED		50
#define	WITHDRAW	51
#define WORMHOLE	52
#define	ZZZ		53

#define	NUM_COMMANDS	ZZZ+1

/* Constants needed for parsing. */
#define	UNKNOWN		0
#define	TECH_ID		1
#define	ITEM_CLASS	2
#define	SHIP_CLASS	3
#define	PLANET_ID	4
#define	SPECIES_ID	5



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

    char
	tech_abbr[6][4] =
	{
		"MI",
		"MA",
		"ML",
		"GV",
		"LS",
		"BI"
	};

    char
	tech_name[6][16] =
	{
		"Mining",
		"Manufacturing",
		"Military",
		"Gravitics",
		"Life Support",
		"Biology"
	};



    char
	item_name[MAX_ITEMS][32] =
	{
		"Raw Material Unit",
		"Planetary Defense Unit",
		"Starbase Unit",
		"Damage Repair Unit",
		"Colonist Unit",
		"Colonial Mining Unit",
		"Colonial Manufacturing Unit",
		"Fail-Safe Jump Unit",
		"Jump Portal Unit",
		"Forced Misjump Unit",
		"Forced Jump Unit",
		"Gravitic Telescope Unit",
		"Field Distortion Unit",
		"Terraforming Plant",
		"Germ Warfare Bomb",
		"Mark-1 Shield Generator",
		"Mark-2 Shield Generator",
		"Mark-3 Shield Generator",
		"Mark-4 Shield Generator",
		"Mark-5 Shield Generator",
		"Mark-6 Shield Generator",
		"Mark-7 Shield Generator",
		"Mark-8 Shield Generator",
		"Mark-9 Shield Generator",
		"Mark-1 Gun Unit",
		"Mark-2 Gun Unit",
		"Mark-3 Gun Unit",
		"Mark-4 Gun Unit",
		"Mark-5 Gun Unit",
		"Mark-6 Gun Unit",
		"Mark-7 Gun Unit",
		"Mark-8 Gun Unit",
		"Mark-9 Gun Unit",
		"X1 Unit",
		"X2 Unit",
		"X3 Unit",
		"X4 Unit",
		"X5 Unit",
	};

    char
	item_abbr[MAX_ITEMS][4] =
	{
		"RM",	"PD",	"SU",	"DR",	"CU",	"IU",	"AU",	"FS",
		"JP",	"FM",	"FJ",	"GT",	"FD",	"TP",	"GW",	"SG1",
		"SG2",	"SG3",	"SG4",	"SG5",	"SG6",	"SG7",	"SG8",	"SG9",
		"GU1",	"GU2",	"GU3",	"GU4",	"GU5",	"GU6",	"GU7",	"GU8",
		"GU9",	"X1",	"X2",	"X3",	"X4",	"X5"
	};

    long
	item_cost[MAX_ITEMS] =
	{
		1,	1,	110,	50,	1,	1,	1,	25,
		100,	100,	125,	500,	50,	50000,	1000,	250,
		500,	750,	1000,	1250,	1500,	1750,	2000,	2250,
		250,	500,	750,	1000,	1250,	1500,	1750,	2000,
		2250,	9999,	9999,	9999,	9999,	9999
	};

    short
	item_carry_capacity[MAX_ITEMS] =
	{
		1,	3,	20,	1,	1,	1,	1,	1,
		10,	5,	5,	20,	1,	100,	100,	5,
		10,	15,	20,	25,	30,	35,	40,	45,
		5,	10,	15,	20,	25,	30,	35,	40,
		45,	9999,	9999,	9999,	9999,	9999
	};

    char
	item_critical_tech[MAX_ITEMS] =
	{
		MI,	ML,	MA,	MA,	LS,	MI,	MA,	GV,
		GV,	GV,	GV,	GV,	LS,	BI,	BI,	LS,
		LS,	LS,	LS,	LS,	LS,	LS,	LS,	LS,
		ML,	ML,	ML,	ML,	ML,	ML,	ML,	ML,
		ML,	99,	99,	99,	99,	99
	};

    short
	item_tech_requirment[MAX_ITEMS] =
	{
		1,	1,	20,	30,	1,	1,	1,	20,
		25,	30,	40,	50,	20,	40,	50,	10,
		20,	30,	40,	50,	60,	70,	80,	90,
		10,	20,	30,	40,	50,	60,	70,	80,
		90,	999,	999,	999,	999,	999
	};

    char
	ship_abbr[NUM_SHIP_CLASSES][4] =
	{
		"PB",	"CT",	"ES",	"FF",	"DD",	"CL",	"CS",
		"CA",	"CC",	"BC",	"BS",	"DN",	"SD",	"BM",
		"BW",	"BR",	"BA",	"TR"
	};

    char
	ship_type[3][2] = {"", "S", "S"};

    short
	ship_tonnage[NUM_SHIP_CLASSES] =
	{
		1,	2,	5,	10,	15,	20,	25,
		30,	35,	40,	45,	50,	55,	60,
		65,	70,	1,	1
	};

    short
	ship_cost[NUM_SHIP_CLASSES] =
	{
		100,	200,	500,	1000,	1500,	2000,	2500,
		3000,	3500,	4000,	4500,	5000,	5500,	6000,
		6500,	7000,	100,	100
	};

    char
	command_abbr[NUM_COMMANDS][4] =
	{
		"   ", "ALL", "AMB", "ATT", "AUT", "BAS", "BAT", "BUI", "CON",
		"DEE", "DES", "DEV", "DIS", "END", "ENE", "ENG", "EST", "HAV",
		"HID", "HIJ", "IBU", "ICO", "INS", "INT", "JUM", "LAN", "MES",
		"MOV", "NAM", "NEU", "ORB", "PJU", "PRO", "REC", "REP", "RES",
		"SCA", "SEN", "SHI", "STA", "SUM", "SUR", "TAR", "TEA", "TEC",
		"TEL", "TER", "TRA", "UNL", "UPG", "VIS", "WIT", "WOR", "ZZZ"
	};

    char
	command_name[NUM_COMMANDS][16] =
	{
		"Undefined", "Ally", "Ambush", "Attack", "Auto", "Base",
		"Battle", "Build", "Continue", "Deep", "Destroy", "Develop",
		"Disband", "End", "Enemy", "Engage", "Estimate", "Haven",
		"Hide", "Hijack", "Ibuild", "Icontinue", "Install", "Intercept",
		"Jump", "Land", "Message", "Move", "Name", "Neutral", "Orbit",
		"Pjump", "Production", "Recycle", "Repair", "Research", "Scan",
		"Send", "Shipyard", "Start", "Summary", "Surrender", "Target",
		"Teach", "Tech", "Telescope", "Terraform", "Transfer", "Unload",
		"Upgrade", "Visited", "Withdraw", "Wormhole", "ZZZ"
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
