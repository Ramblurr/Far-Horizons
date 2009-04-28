
#define	MAX_BATTLES		50
	/* Maximum number of battle locations for all players. */

#define	MAX_SHIPS		200
	/* Maximum number of ships at a single battle. */

#define MAX_ENGAGE_OPTIONS	20
	/* Maximum number of engagement options that a player may specify
	   for a single battle. */


struct battle_data
{
    char	x, y, z, num_species_here;
    char	spec_num[MAX_SPECIES];
    char	summary_only[MAX_SPECIES];
    char	transport_withdraw_age[MAX_SPECIES];
    char	warship_withdraw_age[MAX_SPECIES];
    char	fleet_withdraw_percentage[MAX_SPECIES];
    char	haven_x[MAX_SPECIES];
    char	haven_y[MAX_SPECIES];
    char	haven_z[MAX_SPECIES];
    char	special_target[MAX_SPECIES];
    char	hijacker[MAX_SPECIES];
    char	can_be_surprised[MAX_SPECIES];
    char	enemy_mine[MAX_SPECIES][MAX_SPECIES];
    char	num_engage_options[MAX_SPECIES];
    char	engage_option[MAX_SPECIES][MAX_ENGAGE_OPTIONS];
    char	engage_planet[MAX_SPECIES][MAX_ENGAGE_OPTIONS];
    long	ambush_amount[MAX_SPECIES];
};

/* Types of combatants. */
#define SHIP		1
#define NAMPLA		2
#define GENOCIDE_NAMPLA	3
#define BESIEGED_NAMPLA	4

/* Types of special targets. */
#define TARGET_WARSHIPS		1
#define TARGET_TRANSPORTS	2
#define TARGET_STARBASES	3
#define TARGET_PDS		4

/* Types of actions. */
#define DEFENSE_IN_PLACE	0
#define DEEP_SPACE_DEFENSE	1
#define PLANET_DEFENSE		2
#define DEEP_SPACE_FIGHT	3
#define PLANET_ATTACK		4
#define PLANET_BOMBARDMENT	5
#define GERM_WARFARE		6
#define SIEGE			7

/* Special types. */
#define	NON_COMBATANT		1

struct action_data
{
    int		num_units_fighting;
    int		fighting_species_index[MAX_SHIPS];
    int		num_shots[MAX_SHIPS];
    int		shots_left[MAX_SHIPS];
    long	weapon_damage[MAX_SHIPS];
    long	shield_strength[MAX_SHIPS];
    long	shield_strength_left[MAX_SHIPS];
    long	original_age_or_PDs[MAX_SHIPS];
    long	bomb_damage[MAX_SHIPS];
    char	surprised[MAX_SHIPS];
    char	unit_type[MAX_SHIPS];
    char	*fighting_unit[MAX_SHIPS];
};
