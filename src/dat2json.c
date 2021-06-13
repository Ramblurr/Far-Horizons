/*
 * Convert binary data files to portable JSON.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fh.h"

extern int num_stars, num_planets;

int species_index, species_number;


struct galaxy_data   galaxy;
struct star_data *   star;
struct planet_data * planet;
struct species_data *species;
struct nampla_data * nampla_base, *nampla;
struct ship_data *   ship_base, *ship;

extern struct star_data *  star_base;
extern struct planet_data *planet_base;

void get_galaxy_data(void);
void get_planet_data(void);
void get_species_data(void);
void get_star_data(void);

const char *itemCode(int j);
char *itoa(int val, char *buffer, int radix);
void jbool(FILE *fp, int indent, const char *name, int val, const char *eol);
void jcoords(FILE *fp, int indent, const char *name, int x, int y, int z, const char *eol);
void jint(FILE *fp, int indent, const char *name, int val, const char *eol);
void jstr(FILE *fp, int indent, const char *name, const char *val, const char *eol);
const char *mkkey(int x, int y, int z, int orbit);
const char *shipClassCode(int j);
const char *shipStatusCode(int j);
const char *shipTypeCode(int j);
char *strreverse(char *s);

int
main(int argc, char *argv[]) {
    int  i, j, ok;
    char buffer[1024];

    FILE *fp;

    // create the output file
    fp = fopen("cluster.json", "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot create output file 'cluster.json'!\n");
        return(2);
    }

    /* Get all binary data. */
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    get_species_data();

    fprintf(fp, "{\n");
    jstr(fp, 1, "version", "7.0.1", ",\n");

    fprintf(fp, "\t\"cluster\": {\n");
    jint(fp, 2, "d_num_species", galaxy.d_num_species, ",\n");
    jint(fp, 2, "num_species", galaxy.num_species, ",\n");
    jint(fp, 2, "radius", galaxy.radius, ",\n");
    jint(fp, 2, "turn_number", galaxy.turn_number, ",\n");
    fprintf(fp, "\t},\n");

    fprintf(fp, "\t\"systems\": [\n");
    for (i = 0; i < num_stars; i++) {
        char *psep = "";
        star = star_base + i;

        fprintf(fp, "\t\t{\n");

        jint(fp, 3, "id", i + 1, ",\n");
        jstr(fp, 3, "key", mkkey(star->x, star->y, star->z, 0), ",\n");
        jcoords(fp, 3, "coords", star->x, star->y, star->z, ",\n");
        jint(fp, 3, "type", star->type, ",\n");
        jint(fp, 3, "color", star->color, ",\n");
        jint(fp, 3, "size", star->size, ",\n");
        if (star->home_system != 0) {
            jbool(fp, 3, "home_system", star->home_system, ",\n");
        }
        if (star->worm_here != 0) {
            jcoords(fp, 3, "worm", star->worm_x, star->worm_y, star->worm_z, ",\n");
        }
        fprintf(fp, "\t\t\t\"planets\": [");
        for (int orbit = 0; orbit < star->num_planets; orbit++) {
            if (orbit != 0) {
                fprintf(fp, ", ");
            }
            fprintf(fp, "%d", star->planet_index + orbit);
        }
        fprintf(fp, "],\n");

        for (j = 0; j < NUM_CONTACT_WORDS; j++) {
            if (star->visited_by[j] != 0) {
                char *vsep = "";
                fprintf(fp, "\t\t\t\"visited_by\": [");
                for (int sp = 1; sp <= MAX_SPECIES; sp++) {
                    /* Get array index and bit mask. */
                    int  species_array_index, species_bit_number;
                    long species_bit_mask;
                    species_array_index = (sp - 1) / 32;
                    species_bit_number  = (sp - 1) % 32;
                    species_bit_mask    = 1 << species_bit_number;
                    /* Check if bit is already set. */
                    if (star->visited_by[species_array_index] & species_bit_mask) {
                        fprintf(fp, "%s\"SP%02d\"", vsep, sp);
                        vsep = ", ";
                    }
                }
                fprintf(fp, "],\n");
                break;
            }
        }
        jint(fp, 3, "message", star->message, "\n");
        fprintf(fp, "\t\t}");
        if (i + 1 < num_stars) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\t],\n");

    fprintf(fp, "\t\"planets\": [\n");
    for (i = 0; i < num_planets; i++) {
        struct planet_data *planet = planet_base + i;
        fprintf(fp, "\t\t{\n");
        jint(fp, 3, "id", i + 1, ",\n");
        jint(fp, 3, "temperature_class", planet->temperature_class, ",\n");
        jint(fp, 3, "pressure_class", planet->pressure_class, ",\n");
        fprintf(fp, "\t\t\t\"gases\": [\n");
        for (j = 0; j < 4; j++) {
            fprintf(fp, "\t\t\t\t{\"gas\": %d, \"percentage\": %d}", planet->gas[j], planet->gas_percent[j]);
            if (j != 3) {
                fprintf(fp, ",");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\t\t\t],\n");
        jint(fp, 3, "diameter", planet->diameter, ",\n");
        jint(fp, 3, "gravity", planet->gravity, ",\n");
        jint(fp, 3, "mining_difficulty", planet->mining_difficulty, ",\n");
        jint(fp, 3, "econ_efficiency", planet->econ_efficiency, ",\n");
        jint(fp, 3, "md_increase", planet->md_increase, ",\n");
        jint(fp, 3, "message", planet->message, ",\n");
        fprintf(fp, "\t\t}");
        if (i + 1 < num_planets) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\t],\n");

    fprintf(fp, "\t\"species\": [\n");
    for (species_index = 0; species_index < galaxy.num_species; species_index++) {
        species_number = species_index + 1;

        if (!data_in_memory[species_index]) {
            fprintf(fp, "\t\t{\"id\": \"SP%02d\"}", species_number);
            jstr(fp, 3, "id", buffer, ",\n");
            if (i + 1 < galaxy.num_species) {
                fprintf(fp, ",");
            }
            fprintf(fp, "\n");
            printf("Skipping species #%d.\n", species_number);
            continue;
        }

        species     = &spec_data[species_number - 1];
        nampla_base = namp_data[species_number - 1];
        ship_base   = ship_data[species_number - 1];

        fprintf(fp, "\t\t{\n");
        sprintf(buffer, "SP%02d", species_number);
        jstr(fp, 3, "id", buffer, ",\n");
        jstr(fp, 3, "name", species->name, ",\n");
        fprintf(fp, "\t\t\t\"government\": {\n");
        jstr(fp, 4, "name", species->govt_name, ",\n");
        jstr(fp, 4, "type", species->govt_type, ",\n");
        fprintf(fp, "\t\t\t},\n");
        fprintf(fp, "\t\t\t\"homeworld\": {\n");
        jstr(fp, 4, "key", mkkey(species->x, species->y, species->z, species->pn), ",\n");
        jcoords(fp, 4, "coords", species->x, species->y, species->z, ",\n");
        jint(fp, 4, "orbit", species->pn, ",\n");
        fprintf(fp, "\t\t\t},\n");
        fprintf(fp, "\t\t\t\"gases\": {\n");
        fprintf(fp, "\t\t\t\t\"required\": [{\"gas\": %d, \"min\": %d, \"max\": %d}],\n", species->required_gas, species->required_gas_min, species->required_gas_max);
        fprintf(fp, "\t\t\t\t\"poison\": [");
        for (j = 0; j < 6; j++) {
            if (j != 0) {
                fprintf(fp, ", ");
            }
            fprintf(fp, "%d", species->poison_gas[j]);
        }
        fprintf(fp, "],\n");
        fprintf(fp, "\t\t\t},\n");
        if (species->auto_orders != 0) {
            jbool(fp, 3, "auto_orders", species->auto_orders, ",\n");
        }
        fprintf(fp, "\t\t\t\"tech\": {\n");
        fprintf(fp, "\t\t\t\t\"MI\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[0], species->init_tech_level[0], species->tech_knowledge[0], species->tech_eps[0]);
        fprintf(fp, "\t\t\t\t\"MA\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[1], species->init_tech_level[1], species->tech_knowledge[1], species->tech_eps[1]);
        fprintf(fp, "\t\t\t\t\"ML\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[2], species->init_tech_level[2], species->tech_knowledge[2], species->tech_eps[2]);
        fprintf(fp, "\t\t\t\t\"GV\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[3], species->init_tech_level[3], species->tech_knowledge[3], species->tech_eps[3]);
        fprintf(fp, "\t\t\t\t\"LS\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[4], species->init_tech_level[4], species->tech_knowledge[4], species->tech_eps[4]);
        fprintf(fp, "\t\t\t\t\"BI\": {\"level\": %d, \"init\": %d, \"knowledge\": %d, \"xp\": %ld},\n", species->tech_level[5], species->init_tech_level[5], species->tech_knowledge[5], species->tech_eps[5]);
        fprintf(fp, "\t\t\t},\n");
        jint(fp, 3, "econ_units", species->econ_units, ",\n");
        jint(fp, 3, "hp_original_base", species->hp_original_base, ",\n");
        jint(fp, 3, "fleet_cost", species->fleet_cost, ",\n");
        jint(fp, 3, "fleet_percent_cost", species->fleet_percent_cost, ",\n");

        for (j = 0; j < NUM_CONTACT_WORDS; j++) {
            if (species->contact[j] != 0) {
                char *vsep = "";
                fprintf(fp, "\t\t\t\"contacts\": [");
                for (int sp = 1; sp <= MAX_SPECIES; sp++) {
                    /* Get array index and bit mask. */
                    int  species_array_index, species_bit_number;
                    long species_bit_mask;
                    species_array_index = (sp - 1) / 32;
                    species_bit_number  = (sp - 1) % 32;
                    species_bit_mask    = 1 << species_bit_number;
                    /* Check if bit is already set. */
                    if (species->contact[species_array_index] & species_bit_mask) {
                        fprintf(fp, "%s\"SP%02d\"", vsep, sp);
                        vsep = ", ";
                    }
                }
                fprintf(fp, "],\n");
                break;
            }
        }
        for (j = 0; j < NUM_CONTACT_WORDS; j++) {
            if (species->ally[j] != 0) {
                char *vsep = "";
                fprintf(fp, "\t\t\t\"allies\": [");
                for (int sp = 1; sp <= MAX_SPECIES; sp++) {
                    /* Get array index and bit mask. */
                    int  species_array_index, species_bit_number;
                    long species_bit_mask;
                    species_array_index = (sp - 1) / 32;
                    species_bit_number  = (sp - 1) % 32;
                    species_bit_mask    = 1 << species_bit_number;
                    /* Check if bit is already set. */
                    if (species->ally[species_array_index] & species_bit_mask) {
                        fprintf(fp, "%s\"SP%02d\"", vsep, sp);
                        vsep = ", ";
                    }
                }
                fprintf(fp, "],\n");
                break;
            }
        }
        for (j = 0; j < NUM_CONTACT_WORDS; j++) {
            if (species->enemy[j] != 0) {
                char *vsep = "";
                fprintf(fp, "\t\t\t\"enemies\": [");
                for (int sp = 1; sp <= MAX_SPECIES; sp++) {
                    /* Get array index and bit mask. */
                    int  species_array_index, species_bit_number;
                    long species_bit_mask;
                    species_array_index = (sp - 1) / 32;
                    species_bit_number  = (sp - 1) % 32;
                    species_bit_mask    = 1 << species_bit_number;
                    /* Check if bit is already set. */
                    if (species->enemy[species_array_index] & species_bit_mask) {
                        fprintf(fp, "%s\"SP%02d\"", vsep, sp);
                        vsep = ", ";
                    }
                }
                fprintf(fp, "],\n");
                break;
            }
        }

        fprintf(fp, "\t\t\t\"namplas\": {\n");
        for (i = 0; i < species->num_namplas; i++) {
            const char *isep = "";
            nampla = nampla_base + i;

            fprintf(fp, "\t\t\t\t\"%s\": {\n", nampla->name);
            jint(fp, 5, "id", i + 1, ",\n");
            jstr(fp, 5, "location", mkkey(nampla->x, nampla->y, nampla->z, nampla->pn), ",\n");
            jcoords(fp, 5, "coords", nampla->x, nampla->y, nampla->z, ",\n");
            jint(fp, 5, "orbit", nampla->pn, ",\n");
            jint(fp, 5, "status", nampla->status, ",\n");
            jint(fp, 5, "hiding", nampla->hiding, ",\n");
            jint(fp, 5, "hidden", nampla->hidden, ",\n");
            jint(fp, 5, "planet_index", nampla->planet_index, ",\n");
            jint(fp, 5, "siege_eff", nampla->siege_eff, ",\n");
            jint(fp, 5, "shipyards", nampla->shipyards, ",\n");
            jint(fp, 5, "IUs_needed", nampla->IUs_needed, ",\n");
            jint(fp, 5, "AUs_needed", nampla->AUs_needed, ",\n");
            jint(fp, 5, "auto_IUs", nampla->auto_IUs, ",\n");
            jint(fp, 5, "auto_AUs", nampla->auto_AUs, ",\n");
            jint(fp, 5, "IUs_to_install", nampla->IUs_to_install, ",\n");
            jint(fp, 5, "AUs_to_install", nampla->AUs_to_install, ",\n");
            jint(fp, 5, "mi_base", nampla->mi_base, ",\n");
            jint(fp, 5, "ma_base", nampla->ma_base, ",\n");
            jint(fp, 5, "pop_units", nampla->pop_units, ",\n");
            jint(fp, 5, "use_on_ambush", nampla->use_on_ambush, ",\n");
            jint(fp, 5, "message", nampla->message, ",\n");
            fprintf(fp, "\t\t\t\t\t\"inventory\": {");
            for (j = 0; j < MAX_ITEMS; j++) {
                if (nampla->item_quantity[j] == 0) {
                    continue;
                }
                fprintf(fp, "%s\"%s\": %ld", isep, itemCode(j), nampla->item_quantity[j]);
                isep = ", ";
            }
            fprintf(fp, "}\n");
            fprintf(fp, "\t\t\t\t}");
            if (i + 1 < species->num_namplas) {
                fprintf(fp, ",");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\t\t\t},\n");

        /* Do ships for this species. */
        fprintf(fp, "\t\t\t\"ships\": {\n");
        for (i = 0; i < species->num_ships; i++) {
            const char *isep = "";
            ship = ship_base + i;

            fprintf(fp, "\t\t\t\t\"%s\": {\n", ship->name);
            jint(fp, 5, "id", i + 1, ",\n");
            jstr(fp, 5, "location", mkkey(ship->x, ship->y, ship->z, ship->pn), ",\n");
            jcoords(fp, 5, "coords", ship->x, ship->y, ship->z, ",\n");
            jint(fp, 5, "orbit", ship->pn, ",\n");
            jstr(fp, 5, "class", shipClassCode(ship->class), ",\n");
            jstr(fp, 5, "type", shipTypeCode(ship->type), ",\n");
            jint(fp, 5, "tonnage", ship->tonnage, ",\n");
            jint(fp, 5, "age", ship->age, ",\n");
            jstr(fp, 5, "status", shipStatusCode(ship->status), ",\n");
            jcoords(fp, 5, "dest", ship->dest_x, ship->dest_y, ship->dest_z, ",\n");
            if (ship->just_jumped != 0) {
                jbool(fp, 5, "just_jumped", ship->just_jumped, ",\n");
            }
            if (ship->arrived_via_wormhole != 0) {
                jbool(fp, 5, "arrived_via_wormhole", ship->arrived_via_wormhole, ",\n");
            }
            jint(fp, 5, "loading_point", ship->loading_point, ",\n");
            jint(fp, 5, "unloading_point", ship->unloading_point, ",\n");
            jint(fp, 5, "remaining_cost", ship->remaining_cost, ",\n");
            jint(fp, 5, "message", 0, ",\n");
            fprintf(fp, "\t\t\t\t\t\"inventory\": {");
            for (j = 0; j < MAX_ITEMS; j++) {
                if (ship->item_quantity[j] == 0) {
                    continue;
                }
                fprintf(fp, "%s\"%s\": %d", isep, itemCode(j), ship->item_quantity[j]);
                isep = ", ";
            }
            fprintf(fp, "}\n");
            fprintf(fp, "\t\t\t\t}");
            if (i + 1 < species->num_ships) {
                fprintf(fp, ",");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\t\t\t}\n");

        fprintf(fp, "\t\t}");
        if (i + 1 < galaxy.num_species) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\t]\n");

    fprintf(fp, "}\n");
    fclose(fp);

    return(0);
}

const char *
itemCode(int j) {
    switch (j) {
    case RM:  return("RM");  /* Raw Material Units. */

    case PD:  return("PD");  /* Planetary Defense Units. */

    case SU:  return("SU");  /* Starbase Units. */

    case DR:  return("DR");  /* Damage Repair Units. */

    case CU:  return("CU");  /* Colonist Units. */

    case IU:  return("IU");  /* Colonial Mining Units. */

    case AU:  return("AU");  /* Colonial Manufacturing Units. */

    case FS:  return("FS");  /* Fail-Safe Jump Units. */

    case JP:  return("JP");  /* Jump Portal Units. */

    case FM:  return("FM");  /* Forced Misjump Units. */

    case FJ:  return("FJ");  /* Forced Jump Units. */

    case GT:  return("GT");  /* Gravitic Telescope Units. */

    case FD:  return("FD");  /* Field Distortion Units. */

    case TP:  return("TP");  /* Terraforming Plants. */

    case GW:  return("GW");  /* Germ Warfare Bombs. */

    case SG1: return("SG1"); /* Mark-1 Auxiliary Shield Generators. */

    case SG2: return("SG2"); /* Mark-2. */

    case SG3: return("SG3"); /* Mark-3. */

    case SG4: return("SG4"); /* Mark-4. */

    case SG5: return("SG5"); /* Mark-5. */

    case SG6: return("SG6"); /* Mark-6. */

    case SG7: return("SG7"); /* Mark-7. */

    case SG8: return("SG8"); /* Mark-8. */

    case SG9: return("SG9"); /* Mark-9. */

    case GU1: return("GU1"); /* Mark-1 Auxiliary Gun Units. */

    case GU2: return("GU2"); /* Mark-2. */

    case GU3: return("GU3"); /* Mark-3. */

    case GU4: return("GU4"); /* Mark-4. */

    case GU5: return("GU5"); /* Mark-5. */

    case GU6: return("GU6"); /* Mark-6. */

    case GU7: return("GU7"); /* Mark-7. */

    case GU8: return("GU8"); /* Mark-8. */

    case GU9: return("GU9"); /* Mark-9. */

    case X1:  return("X1");  /* Unassigned. */

    case X2:  return("X2");  /* Unassigned. */

    case X3:  return("X3");  /* Unassigned. */

    case X4:  return("X4");  /* Unassigned. */

    case X5:  return("X5");  /* Unassigned. */

    default:
        fprintf(stderr, "assert(item_index != %d)\n", j);
        exit(2);
    }
    return("??");
}

// itoa from K&R
char *
itoa(int val, char *buffer, int radix) {
    static const char *digits = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";

    if (val != 0 && 2 <= radix && radix <= 36) {
        int sign = val;
        int len  = 0;
        for ( ; val != 0;) {
            int tmp = val;
            val        /= radix;
            buffer[len] = digits[35 + (tmp - val * radix)];
            len++;
        }
        ;

        if (sign < 0) {
            buffer[len] = '-';
            len++;
        }
        buffer[len] = 0;

        return(strreverse(buffer));
    }

    buffer[0] = '0';
    buffer[1] = 0;
    return(buffer);
}

void
jbool(FILE *fp, int indent, const char *name, int val, const char *eol) {
    for (int i = 0; i < indent; i++) {
        fputc('\t', fp);
    }
    ;
    if (val == 0) {
        fprintf(fp, "\"%s\": false%s", name, eol);
    } else {
        fprintf(fp, "\"%s\": true%s", name, eol);
    }
}

void
jcoords(FILE *fp, int indent, const char *name, int x, int y, int z, const char *eol) {
    for (int i = 0; i < indent; i++) {
        fputc('\t', fp);
    }
    ;
    fprintf(fp, "\"%s\": {\"x\": %d, \"y\": %d, \"z\": %d}%s", name, x, y, z, eol);
}

void
jint(FILE *fp, int indent, const char *name, int val, const char *eol) {
    for (int i = 0; i < indent; i++) {
        fputc('\t', fp);
    }
    ;
    fprintf(fp, "\"%s\": %d%s", name, val, eol);
}

void
jstr(FILE *fp, int indent, const char *name, const char *val, const char *eol) {
    for (int i = 0; i < indent; i++) {
        fputc('\t', fp);
    }
    ;
    if (val == 0) {
        fprintf(fp, "\"%s\": \"\"%s", name, eol);
    } else {
        fprintf(fp, "\"%s\": \"%s\"%s", name, val, eol);
    }
}

const char *
mkkey(int x, int y, int z, int orbit) {
    static char key[64];

    if (orbit == 0) {
        sprintf(key, "%d %d %d", x, y, z);
    } else {
        sprintf(key, "%d %d %d #%d", x, y, z, orbit);
    }
    return(key);
}

/* Ship classes. */
const char *
shipClassCode(int j) {
    switch (j) {
    case PB: return("PB");   /* Picketboat. */

    case CT: return("CT");   /* Corvette. */

    case ES: return("ES");   /* Escort. */

    case DD: return("DD");   /* Destroyer. */

    case FG: return("FG");   /* Frigate. */

    case CL: return("CL");   /* Light Cruiser. */

    case CS: return("CS");   /* Strike Cruiser. */

    case CA: return("CA");   /* Heavy Cruiser. */

    case CC: return("CC");   /* Command Cruiser. */

    case BC: return("BC");   /* Battlecruiser. */

    case BS: return("BS");   /* Battleship. */

    case DN: return("DN");   /* Dreadnought. */

    case SD: return("SD");   /* Super Dreadnought. */

    case BM: return("BM");   /* Battlemoon. */

    case BW: return("BW");   /* Battleworld. */

    case BR: return("BR");   /* Battlestar. */

    case BA: return("BA");   /* Starbase. */

    case TR: return("TR");   /* Transport. */

    default:
        fprintf(stderr, "assert(ship_class != %d)\n", j);
        exit(2);
    }
    return("??");
}

const char *
shipStatusCode(int j) {
    switch (j) {
    case UNDER_CONSTRUCTION: return("UNDER_CONSTRUCTION");

    case ON_SURFACE: return("ON_SURFACE");

    case IN_ORBIT: return("IN_ORBIT");

    case IN_DEEP_SPACE: return("IN_DEEP_SPACE");

    case JUMPED_IN_COMBAT: return("JUMPED_IN_COMBAT");

    case FORCED_JUMP: return("FORCED_JUMP");

    default:
        fprintf(stderr, "assert(ship_status != %d)\n", j);
        exit(2);
    }
    return("??");
}

const char *
shipTypeCode(int j) {
    switch (j) {
    case FTL: return("FTL");

    case SUB_LIGHT: return("SUB_LIGHT");

    case STARBASE: return("STARBASE");

    default:
        fprintf(stderr, "assert(ship_type != %d)\n", j);
        exit(2);
    }
    return("??");
}

// reverse the buffer by swapping from the ends
char *
strreverse(char *s) {
    char *begin = s;

    for (char *end = begin + strlen(begin) - 1; end > begin; end--) {
        char ch = *begin;
        *begin = *end;
        *end   = ch;
        begin++;
    }
    return(s);
}
