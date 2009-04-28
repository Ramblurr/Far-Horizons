
#define	THIS_IS_MAIN

#include "fh.h"


int	x, y, z, pn, species_number, species_index;

long	value;


struct galaxy_data	galaxy;
struct star_data	*star;
struct planet_data	*planet;
struct species_data	*species;
struct nampla_data	*nampla_base, *nampla;
struct ship_data	*ship_base, *ship;


extern int	num_stars, num_planets, truncate_name, ignore_field_distorters;

extern struct star_data		*star_base;
extern struct planet_data	*planet_base;



main (argc, argv)

int argc;
char *argv[];

{
    int		i, option, stars_modified, planets_modified, species_modified;

    char	answer[16];


    /* Get data. */
    get_galaxy_data ();
    get_star_data ();
    get_planet_data ();
    get_species_data ();

    ignore_field_distorters = TRUE;
    truncate_name = TRUE;
    stars_modified = FALSE;
    planets_modified = FALSE;
    species_modified = FALSE;

again:
    /* Get what to edit. */
    printf ("\n\nEdit what (1=star, 2=planet, 3=species, 4=create star, 8=save, 9=quit)? ");
    if (get_value() != 1) goto again;
    option = value;

    if (option == 1)
    {
	edit_star ();
	stars_modified = TRUE;
	goto again;
    }
    else if (option == 2)
    {
	edit_planet ();
	planets_modified = TRUE;
	goto again;
    }
    else if (option == 3)
    {
	edit_species ();
	species_modified = TRUE;
	goto again;
    }
    else if (option == 4)
    {
	create_star ();
	stars_modified = TRUE;
	planets_modified = TRUE;
	goto again;
    }
    else if (option == 8)
    {
	/* Save data. */
	if (stars_modified) save_star_data ();
	if (planets_modified) save_planet_data ();
	if (species_modified)
	{
	    save_species_data ();
	    do_locations ();	/* Create new locations array. */
	    save_location_data ();
	}

	stars_modified = FALSE;
	planets_modified = FALSE;
	species_modified = FALSE;
    }
    else if (option == 9)
    {
	if (stars_modified || planets_modified || species_modified)
	{
	    printf ("\007\n\n\tWARNING! Data may have been modified!\n");
	    printf ("\n\tDo you really want to exit (y or Y = yes): ");
	    fflush (stdout);
	    fgets (answer, 16, stdin);
	    if (answer[0] != 'y'  &&  answer[0] != 'Y') goto again;
	    printf ("\n");
	}

	free_species_data ();
	free (star_base);
	free (planet_base);
	exit (0);
    }

    goto again;
}



create_star ()
{
    int		i;

    char	*cp;


    if (get_location (FALSE)) goto no_change;

    star = star_base + num_stars;

    cp = (char *) star;
    for (i = 0; i < sizeof (struct star_data); i++)
	*cp++ = 0;

    star->x = x;
    star->y = y;
    star->z = z;
    star->planet_index = num_planets;

    printf ("Enter star type (1=dwarf, 2=degen, 3=main, 4=giant): ");
    if (get_value () < 1) goto no_change;
    star->type = value;

    printf ("Enter star color (1=O, 2=B, 3=A, 4=F, 5=G, 6=K, 7=M): ");
    if (get_value () < 1) goto no_change;
    star->color = value;

    printf ("Enter star size (0-9): ");
    if (get_value () < 1) goto no_change;
    star->size = value;

    printf ("Enter number of planets (1-9) to generate: ");
    if (get_value () < 1) goto no_change;
    i = value;
    star->num_planets = i;

    ++num_stars;

    planet = planet_base + num_planets;

    generate_planets (planet, i);

    num_planets += star->num_planets;

    return;

no_change:

    printf ("\n\tAbort. No changes were made.\n\n");

}



edit_star ()

{
    int		i;


    if (! get_location (FALSE)) return;

    printf ("Current message = %ld: ", star->message);
    i = get_value ();
    if (i == 1) star->message = value;
    else if (i == -1) return;

    printf ("Current worm_here = %d: ", star->worm_here);
    i = get_value ();
    if (i == 1) star->worm_here = value;
    else if (i == -1) return;

    printf ("Current worm_x = %d: ", star->worm_x);
    i = get_value ();
    if (i == 1) star->worm_x = value;
    else if (i == -1) return;

    printf ("Current worm_y = %d: ", star->worm_y);
    i = get_value ();
    if (i == 1) star->worm_y = value;
    else if (i == -1) return;

    printf ("Current worm_z = %d: ", star->worm_z);
    i = get_value ();
    if (i == 1) star->worm_z = value;
    else if (i == -1) return;
}




edit_planet ()

{
    int		i, n, p, num_gases;

    char	answer[64];


    if (! get_location (TRUE)) return;

    printf ("\n               Temp  Press Mining\n");
    printf ("  #  Dia  Grav Class Class  Diff  Atmosphere\n");
    printf (" ------------------------------------------------------------------\n");

    printf ("  %d  %3d  %d.%02d  %2d    %2d    %d.%02d  ",
	pn,
    	planet->diameter,
    	planet->gravity/100,
    	planet->gravity%100,
    	planet->temperature_class,
    	planet->pressure_class,
    	planet->mining_difficulty/100,
    	planet->mining_difficulty%100);

    num_gases = 0;
    for (n = 0; n < 4; n++)
    {
	if (planet->gas_percent[n] > 0)
	{
	    if (num_gases > 0) printf (",");
	    printf ("%s(%d%%)", gas_string[planet->gas[n]],
			planet->gas_percent[n]);
	    ++num_gases;
	}
    }

    if (num_gases == 0) printf ("No atmosphere");

    printf ("\n");

    /* List each planet attribute and let user change it. */
    printf ("\nEnter new value for planet or hit RETURN key to leave as is...\n\n");

    printf ("Current diameter = %d: ", planet->diameter);
    i = get_value ();
    if (i == 1) planet->diameter = value;
    else if (i == -1) return;

    printf ("Current gravity times 100 = %d: ", planet->gravity);
    i = get_value ();
    if (i == 1) planet->gravity = value;
    else if (i == -1) return;

    printf ("Current temperature class = %d: ", planet->temperature_class);
    i = get_value ();
    if (i == 1) planet->temperature_class = value;
    else if (i == -1) return;

    printf ("Current pressure class = %d: ", planet->pressure_class);
    i = get_value ();
    if (i == 1) planet->pressure_class = value;
    else if (i == -1) return;

    printf ("Current mining difficulty times 100 = %d: ", planet->mining_difficulty);
    i = get_value ();
    if (i == 1) planet->mining_difficulty = value;
    else if (i == -1) return;

    printf ("Current md_increase times 100 = %d: ", planet->md_increase);
    i = get_value ();
    if (i == 1) planet->md_increase = value;
    else if (i == -1) return;

    printf ("Current econ_efficiency = %d: ", planet->econ_efficiency);
    i = get_value ();
    if (i == 1) planet->econ_efficiency = value;
    else if (i == -1) return;

    printf ("Current message = %ld: ", planet->message);
    i = get_value ();
    if (i == 1) planet->message = value;
    else if (i == -1) return;

next_gas:
    printf ("\nGases:\n");
    for (i = 1; i <= 13; i++)
	printf (" %d-%s", i, gas_string[i]);
    printf ("\n\n");

    n = 0;
    for (i = 0; i < 4; i++)
    {
	printf ("\t%d - %s = %d\%", i, gas_string[planet->gas[i]],
		planet->gas_percent[i]);
	n += planet->gas_percent[i];
    }

    printf ("\n\nTotal percentage = %d\%.\n", n);

    printf ("\nEnter 'index, new gas number, new percent' or RETURN for as is: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = -1; n = 0;
	sscanf (answer, "%d,%d,%d", &i, &n, &p);
	if (i >= 0)
	{
	    planet->gas[i] = n;
	    planet->gas_percent[i] = p;
	    goto next_gas;
	}
    }
}



edit_species ()

{
    int		n, found, sp_index, species_number, option;


    n = 0;
    printf ("\n");
    for (sp_index = 0; sp_index < galaxy.num_species; sp_index++)
    {
	species_number = sp_index + 1;

	found = data_in_memory[sp_index];
	if (! found) continue;

	printf ("%4d - %-18.18s", species_number, spec_data[sp_index].name);

	if (n++ % 3  ==  2) printf ("\n");
    }
    if (n % 3) printf ("\n");

    printf ("\nEnter species number: ");
    if (get_value() != 1) return;
    species_number = value;

    sp_index = species_number - 1;

    if (! data_in_memory[sp_index])
    {
	printf ("\n\n\tNo such species!\n");
	return;
    }

    data_modified[sp_index] = TRUE;

    species = &spec_data[sp_index];
    nampla_base = namp_data[sp_index];
    ship_base = ship_data[sp_index];

again:
    printf ("\nEdit what (1 = species stats, 2 = nampla, 3 = ship, 9 = return): ");
    if (get_value() != 1) return;
    option = value;

    if (option == 1)
    {
	edit_species_stats ();
	goto again;
    }
    else if (option == 2)
    {
	edit_nampla ();
	goto again;
    }
    else if (option == 3)
    {
	edit_ship ();
	goto again;
    }
    else if (option != 9)
	goto again;
    else
	return;
}


edit_species_stats ()

{
    int		i, n, delete, array_index, bit_number;

    long	bit_mask;

    char	name[32], answer[32];


    /* List each nampla attribute and let user change it. */
    printf ("\nEnter new value for SP %s or hit RETURN key to leave as is...\n\n",
	species->name);

    printf ("Species name is %s: ", species->name);
    get_name (name);
    if (answer[0] == 27) return;
    else if (name[0] != '\0') strcpy (species->name, name);

    printf ("Species government name is %s: ", species->govt_name);
    get_name (name);
    if (answer[0] == 27) return;
    else if (name[0] != '\0') strcpy (species->govt_name, name);

    printf ("Species government type is %s: ", species->govt_type);
    get_name (name);
    if (answer[0] == 27) return;
    else if (name[0] != '\0') strcpy (species->govt_type, name);

    printf ("\nCurrent economic units = %ld: ", species->econ_units);
    i = get_value ();
    if (i == 1) species->econ_units = value;
    else if (i == -1) return;

next_tech_level:
    printf ("\nTech levels:\n\n");
    for (i = 0; i < 6; i++) printf ("\t%d - %s = %d\n", i+1, tech_name[i],
	species->tech_level[i]);

    printf ("\nEnter 'tech number-space-new level' or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = 0; n = 0;
	sscanf (answer, "%d%d", &i, &n);
	if (i != 0)
	{
	    species->tech_level[i-1] = n;
	    goto next_tech_level;
	}
    }


next_tech_knowledge:
    printf ("\nTech knowledges:\n\n");
    for (i = 0; i < 6; i++) printf ("\t%d - %s = %d\n", i+1, tech_name[i],
	species->tech_knowledge[i]);

    printf ("\nEnter 'tech number-space-new knowledge' or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = 0; n = 0;
	sscanf (answer, "%d%d", &i, &n);
	if (i != 0)
	{
	    species->tech_knowledge[i-1] = n;
	    goto next_tech_knowledge;
	}
    }

next_tech_eps:
    printf ("\nTech experience points:\n\n");
    for (i = 0; i < 6; i++) printf ("\t%d - %s = %d\n", i+1, tech_name[i],
	species->tech_eps[i]);

    printf ("\nEnter 'tech number-space-new eps' or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = 0; n = 0;
	sscanf (answer, "%d%d", &i, &n);
	if (i != 0)
	{
	    species->tech_eps[i-1] = n;
	    goto next_tech_eps;
	}
    }

next_contact_mask:
    printf ("\nSpecies contacted:\n\n");
    n = 0;
    for (i = 1; i <= galaxy.num_species; i++)
    {
	if (! data_in_memory[i-1]) continue;

	array_index = (i - 1) / 32;
	bit_number = (i - 1) % 32;
	bit_mask = 1 << bit_number;
	if ((species->contact[array_index] & bit_mask) == 0) continue;

	printf ("%4d - %-18.18s", i, spec_data[i-1].name);

	if (n++ % 3  ==  2) printf ("\n");
    }
    if (n % 3) printf ("\n");

    printf ("\n  Enter negative species number to delete bit, positive number to add bit,\n");
    printf ("    or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	n = 0;
	sscanf (answer, "%d", &n);
	if (n != 0)
	{
	    if (n < 0)
	    {
		delete = TRUE;
		n = -n;
	    }
	    else
		delete = FALSE;

	    array_index = (n - 1) / 32;
	    bit_number = (n - 1) % 32;
	    bit_mask = 1 << bit_number;

	    if (delete)
		species->contact[array_index] &= ~bit_mask;
	    else
		species->contact[array_index] |= bit_mask;

	    goto next_contact_mask;
	}
    }

next_ally_mask:
    printf ("\nSpecies declared ALLY:\n\n");
    n = 0;
    for (i = 1; i <= galaxy.num_species; i++)
    {
	if (! data_in_memory[i-1]) continue;

	array_index = (i - 1) / 32;
	bit_number = (i - 1) % 32;
	bit_mask = 1 << bit_number;
	if ((species->ally[array_index] & bit_mask) == 0) continue;

	printf ("%4d - %-18.18s", i, spec_data[i-1].name);

	if (n++ % 3  ==  2) printf ("\n");
    }
    if (n % 3) printf ("\n");

    printf ("\n  Enter negative species number to delete bit, positive number to add bit,\n");
    printf ("    or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	n = 0;
	sscanf (answer, "%d", &n);
	if (n != 0)
	{
	    if (n < 0)
	    {
		delete = TRUE;
		n = -n;
	    }
	    else
		delete = FALSE;

	    array_index = (n - 1) / 32;
	    bit_number = (n - 1) % 32;
	    bit_mask = 1 << bit_number;

	    if (delete)
		species->ally[array_index] &= ~bit_mask;
	    else
		species->ally[array_index] |= bit_mask;

	    goto next_ally_mask;
	}
    }

next_enemy_mask:
    printf ("\nSpecies declared ENEMY:\n\n");
    n = 0;
    for (i = 1; i <= galaxy.num_species; i++)
    {
	if (! data_in_memory[i-1]) continue;

	array_index = (i - 1) / 32;
	bit_number = (i - 1) % 32;
	bit_mask = 1 << bit_number;
	if ((species->enemy[array_index] & bit_mask) == 0) continue;

	printf ("%4d - %-18.18s", i, spec_data[i-1].name);

	if (n++ % 3  ==  2) printf ("\n");
    }
    if (n % 3) printf ("\n");

    printf ("\n  Enter negative species number to delete bit, positive number to add bit,\n");
    printf ("    or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	n = 0;
	sscanf (answer, "%d", &n);
	if (n != 0)
	{
	    if (n < 0)
	    {
		delete = TRUE;
		n = -n;
	    }
	    else
		delete = FALSE;

	    array_index = (n - 1) / 32;
	    bit_number = (n - 1) % 32;
	    bit_mask = 1 << bit_number;

	    if (delete)
		species->enemy[array_index] &= ~bit_mask;
	    else
		species->enemy[array_index] |= bit_mask;

	    goto next_enemy_mask;
	}
    }
}



edit_nampla ()

{
    int		i, n, nampla_number, index_changed, found;

    char	answer[32];

again:
    printf ("\n");
    n = 0;
    nampla = nampla_base - 1;
    for (i = 0; i < species->num_namplas; i++)
    {
	++nampla;

	if (nampla->pn == 99) continue;

	printf ("%3d - %-32s", i+1, nampla->name);

	if (n++ % 2  ==  1) printf ("\n");
    }
    if (n % 2  ==  1) printf ("\n");

    printf ("\nWhich nampla (0 to create a new one, negative to delete)? ");
    if (get_value() != 1) return;
    nampla_number = value;

    if (nampla_number < 0)
    {
	nampla_number = -nampla_number;
	nampla = nampla_base + nampla_number - 1;

	printf ("\nDo you really want to delete PL %s (y or Y = yes)? ",
		nampla->name);
	fflush (stdout);
	fgets (answer, 16, stdin);
	if (answer[0] == 'y'  ||  answer[0] == 'Y') delete_nampla (nampla);
	return;
    }

    if (nampla_number == 0)
	create_nampla ();
    else
	nampla = nampla_base + nampla_number - 1;

    /* List each nampla attribute and let user change it. */
    printf ("\nEnter new value for PL %s or hit RETURN key to leave as is...\n\n",
	nampla->name);

    index_changed = FALSE;

    x = nampla->x;
    printf ("Current x = %d: ", x);
    i = get_value ();
    if (i == 1) { x = value; index_changed = TRUE; }
    else if (i == -1) return;

    y = nampla->y;
    printf ("Current y = %d: ", y);
    i = get_value ();
    if (i == 1) { y = value; index_changed = TRUE; }
    else if (i == -1) return;

    z = nampla->z;
    printf ("Current z = %d: ", z);
    i = get_value ();
    if (i == 1) { z = value; index_changed = TRUE; }
    else if (i == -1) return;

    pn = nampla->pn;
    printf ("Current pn = %d: ", pn);
    i = get_value ();
    if (i == 1) { pn = value; index_changed = TRUE; }
    else if (i == -1) return;

    if (index_changed)
    {
	found = FALSE;
	star = star_base - 1;
	for (i = 0; i < num_stars; i++)
	{
	    ++star;

	    if (star->x != x) continue;
	    if (star->y != y) continue;
	    if (star->z != z) continue;

	    if (pn > star->num_planets) break;

	    found = TRUE;

	    break;
	}

	if (! found)
	{
	    printf ("\n\tPlanet does not exist!\n\n");
	    return;
	}
	nampla->x = x;
	nampla->y = y;
	nampla->z = z;
	nampla->pn = pn;
	nampla->planet_index = star->planet_index + pn - 1;
    }

    printf ("\nStatus bits:\n\n\tHome planet = 1\t\tColony = 2\n");
    printf ("\tSelf-sufficient = 4\tPopulated = 8\n");
    printf ("\tMining colony = 16\tResort colony = 32\n");
    printf ("\tDisbanded colony = 64\n\n");

    printf ("Current status = %d: ", nampla->status);
    i = get_value ();
    if (i == 1) nampla->status = value;
    else if (i == -1) return;

    printf ("\nCurrent mining base times 10 = %ld: ", nampla->mi_base);
    i = get_value ();
    if (i == 1) nampla->mi_base = value;
    else if (i == -1) return;

    printf ("\nCurrent manufacturing base times 10 = %ld: ", nampla->ma_base);
    i = get_value ();
    if (i == 1) nampla->ma_base = value;
    else if (i == -1) return;

    printf ("\nCurrent IUs to install = %d: ", nampla->IUs_to_install);
    i = get_value ();
    if (i == 1) nampla->IUs_to_install = value;
    else if (i == -1) return;

    printf ("\nCurrent AUs to install = %d: ", nampla->AUs_to_install);
    i = get_value ();
    if (i == 1) nampla->AUs_to_install = value;
    else if (i == -1) return;

    printf ("\nCurrent population units = %ld: ", nampla->pop_units);
    i = get_value ();
    if (i == 1) nampla->pop_units = value;
    else if (i == -1) return;

    printf ("Current message = %ld: ", nampla->message);
    i = get_value ();
    if (i == 1) nampla->message = value;
    else if (i == -1) return;

    printf ("Current shipyards = %d: ", nampla->shipyards);
    i = get_value ();
    if (i == 1) nampla->shipyards = value;
    else if (i == -1) return;

next_item:
    printf ("\n");
    n = 0;
    for (i = 0; i < MAX_ITEMS; i++)
    {
	printf ("\t%d: %d %ss", i+1, nampla->item_quantity[i], item_abbr[i]);
	if (++n % 4 == 0) printf ("\n");
    }
    if (n % 4) printf ("\n");
    printf ("\nEnter 'item number-space-new quantity' or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = 0; n = 0;
	sscanf (answer, "%d%d", &i, &n);
	if (i != 0)
	{
	    nampla->item_quantity[i-1] = n;
	    goto next_item;
	}
    }

    printf ("\nCurrent hiding value = %d: ", nampla->hiding);
    i = get_value ();
    if (i == 1) nampla->hiding = value;
    else if (i == -1) return;

    printf ("\nCurrent hidden value = %d: ", nampla->hidden);
    i = get_value ();
    if (i == 1) nampla->hidden = value;
    else if (i == -1) return;

    printf ("\nCurrent siege_eff value = %d: ", nampla->siege_eff);
    i = get_value ();
    if (i == 1) nampla->siege_eff = value;
    else if (i == -1) return;

    printf ("\nCurrent use_on_ambush value = %ld: ", nampla->use_on_ambush);
    i = get_value ();
    if (i == 1) nampla->use_on_ambush = value;
    else if (i == -1) return;
}



create_nampla ()

{
    int		i, found, unused_nampla_available, nampla_index;

    char	name[32], upper_nampla_name[32], upper_name[32];

    struct nampla_data	*unused_nampla;


    /* Get planet cordinates. */
    if (! get_location (TRUE)) return;

    /* Get planet name and make an upper case copy. */
    printf ("\nEnter name: ");
    get_name (name);
    if (name[0] == 27  ||  name[0] == '\0') return;
    for (i = 0; i < 32; i++) upper_name[i] = toupper(name[i]);

    /* Search existing namplas for name and location. */
    found = FALSE;
    unused_nampla_available = FALSE;
    nampla = nampla_base - 1;
    for (nampla_index = 0; nampla_index < species->num_namplas; nampla_index++)
    {
	++nampla;

	if (nampla->pn == 99)
	{
	    /* We can re-use this nampla rather than append a new one. */
	    unused_nampla = nampla;
	    unused_nampla_available = TRUE;
	    continue;
	}

	/* Check if a named planet already exists at this location. */
	if (nampla->x == x  &&  nampla->y == y  &&  nampla->z == z
		&&  nampla->pn == pn)
	{
	    printf ("\nThe planet at these coordinates already has a name.\n\n");
	    return;
	}

	/* Make upper case copy of nampla name. */
	for (i = 0; i < 32; i++)
	    upper_nampla_name[i] = toupper(nampla->name[i]);

	/* Compare names. */
	if (strcmp (upper_nampla_name, upper_name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (found)
    {
	printf ("\nPlanet name is already in use.\n\n");
	return;
    }

    /* Add new nampla to database for this species. */
    if (unused_nampla_available)
	nampla = unused_nampla;
    else
    {
	nampla = nampla_base + species->num_namplas;
	species->num_namplas += 1;
	delete_nampla (nampla);		/* Set everything to zero. */
    }

    /* Initialize new nampla. */
    strcpy (nampla->name, name);
    nampla->x = x;
    nampla->y = y;
    nampla->z = z;
    nampla->pn = pn;
    nampla->planet_index = star->planet_index + pn - 1;
    nampla->status = COLONY;
	/* Everything else was set to zero in above call to 'delete_nampla'. */
}



edit_ship ()

{
    int		i, n, ship_number;

    char	answer[32];


again:
    n = 0;
    ship = ship_base - 1;
    for (i = 0; i < species->num_ships; i++)
    {
	++ship;

	if (ship->pn == 99) continue;

	printf ("%3d - %-32s", i+1, ship_name(ship));

	if (n++ % 2  ==  1) printf ("\n");
    }
    if (n % 2  ==  1) printf ("\n");

    printf ("\nWhich ship (0 to create a new one, negative to delete)? ");
    if (get_value() != 1) return;
    ship_number = value;

    if (ship_number < 0)
    {
	ship_number = -ship_number;
	ship = ship_base + ship_number - 1;

	printf ("\nDo you really want to delete %s (y or Y = yes)? ",
		ship_name (ship));
	fflush (stdout);
	fgets (answer, 16, stdin);
	if (answer[0] == 'y'  ||  answer[0] == 'Y') delete_ship (ship);
	return;
    }

    if (ship_number == 0)
	create_ship ();
    else
	ship = ship_base + ship_number - 1;

    /* List each ship attribute and let user change it. */
    printf ("\nEnter new value for %s or hit RETURN key to leave as is...\n\n",
	ship_name(ship));

    printf ("Current x = %d: ", ship->x);
    i = get_value ();
    if (i == 1) ship->x = value;
    else if (i == -1) return;

    printf ("Current y = %d: ", ship->y);
    i = get_value ();
    if (i == 1) ship->y = value;
    else if (i == -1) return;

    printf ("Current z = %d: ", ship->z);
    i = get_value ();
    if (i == 1) ship->z = value;
    else if (i == -1) return;

    printf ("Current pn = %d: ", ship->pn);
    i = get_value ();
    if (i == 1) ship->pn = value;
    else if (i == -1) return;

    printf ("\nStatus values:\n\n\tUnder Construction = 0\tOn Surface = 1\n");
    printf ("\tIn Orbit = 2\t\tIn Deep Space = 3\n");
    printf ("\tJumped in Combat = 4\tForced Jump = 5\n\n");

    printf ("Current status = %d: ", ship->status);
    i = get_value ();
    if (i == 1) ship->status = value;
    else if (i == -1) return;

    printf ("\n\nShip types: FTL = 0, Sub-light = 1, Starbase = 2\n");

    printf ("\nCurrent ship type = %d: ", ship->type);
    i = get_value ();
    if (i == 1) ship->type = value;
    else if (i == -1) return;

    printf ("\n\nShip classes:\n\n");
    for (i = 0; i < NUM_SHIP_CLASSES; i++)
    {
	printf ("\t%d - %s", i, ship_abbr[i]);
	if (i % 5  ==  4) printf ("\n");
    }
    if (i % 5  !=  4) printf ("\n");

    printf ("\nCurrent ship class = %d: ", ship->class);
    i = get_value ();
    if (i == 1) ship->class = value;
    else if (i == -1) return;

    printf ("\nCurrent ship tonnage = %d: ", ship->tonnage);
    i = get_value ();
    if (i == 1) ship->tonnage = value;
    else if (i == -1) return;

    printf ("\nCurrent ship dest_x = %d: ", ship->dest_x);
    i = get_value ();
    if (i == 1) ship->dest_x = value;
    else if (i == -1) return;

    printf ("\nCurrent ship dest_y = %d: ", ship->dest_y);
    i = get_value ();
    if (i == 1) ship->dest_y = value;
    else if (i == -1) return;

    printf ("\nCurrent ship dest_z = %d: ", ship->dest_z);
    i = get_value ();
    if (i == 1) ship->dest_z = value;
    else if (i == -1) return;

next_item:
    printf ("\n");
    n = 0;
    for (i = 0; i < MAX_ITEMS; i++)
    {
	printf ("\t%d: %d %ss", i+1, ship->item_quantity[i], item_abbr[i]);
	if (++n % 4 == 0) printf ("\n");
    }
    if (n % 4) printf ("\n");
    printf ("\nEnter 'item number-space-new quantity' or RETURN to continue: ");
    get_name (answer);
    if (answer[0] == 27) return;
    else if (answer[0] != '\0')
    {
	i = 0; n = 0;
	sscanf (answer, "%d%d", &i, &n);
	if (i != 0)
	{
	    ship->item_quantity[i-1] = n;
	    goto next_item;
	}
    }

    printf ("\nCurrent just_jumped value = %d: ", ship->just_jumped);
    i = get_value ();
    if (i == 1) ship->just_jumped = value;
    else if (i == -1) return;

    printf ("\nCurrent age value = %d: ", ship->age);
    i = get_value ();
    if (i == 1) ship->age = value;
    else if (i == -1) return;

    printf ("\nCurrent remaining_cost value = %d: ", ship->remaining_cost);
    i = get_value ();
    if (i == 1) ship->remaining_cost = value;
    else if (i == -1) return;
}



create_ship ()

{
    int		i, found, unused_ship_available, ship_index;

    char	name[32], upper_ship_name[32], upper_name[32];

    struct ship_data	*unused_ship;


    /* Get ship name and make an upper case copy. */
    printf ("\nEnter name: ");
    get_name (name);
    if (name[0] == 27  ||  name[0] == '\0') return;
    for (i = 0; i < 32; i++) upper_name[i] = toupper(name[i]);

    /* Search existing ships for name and location. */
    found = FALSE;
    unused_ship_available = FALSE;
    ship = ship_base - 1;
    for (ship_index = 0; ship_index < species->num_ships; ship_index++)
    {
	++ship;

	if (ship->pn == 99)
	{
	    /* We can re-use this ship rather than append a new one. */
	    unused_ship = ship;
	    unused_ship_available = TRUE;
	    continue;
	}

	/* Make upper case copy of ship name. */
	for (i = 0; i < 32; i++)
	    upper_ship_name[i] = toupper(ship->name[i]);

	/* Compare names. */
	if (strcmp (upper_ship_name, upper_name) == 0)
	{
	    found = TRUE;
	    break;
	}
    }

    if (found)
    {
	printf ("\nShip name is already in use.\n\n");
	return;
    }

    /* Add new ship to database for this species. */
    if (unused_ship_available)
	ship = unused_ship;
    else
    {
	ship = ship_base + (int) species->num_ships;
	species->num_ships += 1;
	delete_ship (ship);		/* Set everything to zero. */
    }

    /* Initialize new ship. */
    strcpy (ship->name, name);
	/* Everything else was set to zero in above call to 'delete_ship'. */
}



int get_location (need_planet)

int need_planet;

{
    int		i, found;


    printf ("\nEnter x-coordinate: ");
    if (get_value() != 1) return;
    x = value;

    printf ("\nEnter y-coordinate: ");
    if (get_value() != 1) return;
    y = value;

    printf ("\nEnter z-coordinate: ");
    if (get_value() != 1) return;
    z = value;

    if (need_planet)
    {
	printf ("Enter planet number: ");
	if (get_value () != 1) return;
	pn = value;
    }
    else
	pn = 0;

    /* Get pointers to appropriate star and planet. */
    found = 0;
    star = star_base;
    for (i = 0; i < num_stars; i++)
    {
	if (star->x == x  &&  star->y == y  &&  star->z == z)
	{
	    found = 1;
	    break;
	}
	++star;
    }

    if (found == 0)
    {
	printf ("\nThere is currently no star at these coordinates.\n\n");
	return FALSE;
    }

    if (pn > star->num_planets)
    {
	printf ("\nPlanet number is too large for star!\n\n");
	return FALSE;
    }

    /* Get pointer to planet. */
    if (need_planet)
	planet = planet_base + (long) (star->planet_index + pn - 1);

    return TRUE;
}



get_name (name)

char	name[];

{
    int		i;
    char	temp[1024];


    for (i = 0; i < 32; i++) name[i] = 0;

again:
    fflush (stdout);
    fgets (temp, 1024, stdin);
    if (strlen(temp) > 32)
    {
	printf ("\n\tIt's too long! 31 characters max!\n");
	printf ("\nEnter again: ");
	goto again;
    }

    i = 0;
    while (1)
    {
	if (temp[i] == '\n') break;
	name[i] = temp[i];
	++i;
    }

    name[i] = '\0';
}



get_value ()

{
    char	temp[1024];

again:
    fflush (stdout);
    fgets (temp, 1024, stdin);

    if (temp[0] == '\n') return 0;

    if (temp[0] == 27) return -1;	/* Escape character. */

    sscanf (temp, "%ld", &value);

    return 1;
}
