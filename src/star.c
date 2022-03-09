// Far Horizons Game Engine
// Copyright (C) 2022 Michael D Henderson
// Copyright (C) 2021 Raven Zachary
// Copyright (C) 2019 Casey Link, Adam Piggott
// Copyright (C) 1999 Richard A. Morneau
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "locationvars.h"
#include "log.h"
#include "logvars.h"
#include "nampla.h"
#include "namplavars.h"
#include "ordervars.h"
#include "planetio.h"
#include "planetvars.h"
#include "star.h"
#include "stario.h"
#include "starvars.h"
#include "species.h"
#include "speciesvars.h"


char color_char[] = " OBAFGKM";

char size_char[] = "0123456789";

char type_char[] = " dD g";


// changeSystemToHomeSystem replaces the planets in a system with ones
// from the related homesystem template. the template flags one of the
// planets in the system as a home planet.
int changeSystemToHomeSystem(star_data_t *star) {
    if (star == NULL) {
        fprintf(stderr, "error: changeSystemToHomeSystem: internal error, star is NULL\n");
        exit(2);
    }

    printf(" info: updating system id %4d at %3d %3d %3d planet index %4d\n",
           star->id, star->x, star->y, star->z, star->planet_index);

    // load the home system template
    char filename[128];
    sprintf(filename, "homesystem%d.dat", star->num_planets);
    printf(" info: loading template '%s'\n", filename);
    planet_data_t *templateSystem = getPlanetData(0, filename);
    if (templateSystem == NULL) {
        fprintf(stderr, "error: changeSystemToHomeSystem: unable to load template '%s'\n", filename);
        exit(2);
    }
    printf(" info: loaded template from '%s'\n", filename);

    // make minor random modifications to the template
    for (int pn = 0; pn < star->num_planets; pn++) {
        planet_data_t *planet = templateSystem + pn;
        if (planet->special == 1) {
            printf(" info: randomizing home world: orbit %d\n", pn + 1);
        } else {
            printf(" info: randomizing planet    : orbit %d\n", pn + 1);
        }
        if (planet->temperature_class > 12) {
            planet->temperature_class -= rnd(3) - 1;
        } else if (planet->temperature_class > 0) {
            planet->temperature_class += rnd(3) - 1;
        }
        if (planet->pressure_class > 12) {
            planet->pressure_class -= rnd(3) - 1;
        } else if (planet->pressure_class > 0) {
            planet->pressure_class += rnd(3) - 1;
        }
        if (planet->gas[2] > 0) {
            int roll = rnd(25) + 10;
            if (planet->gas_percent[2] > 50) {
                planet->gas_percent[1] += roll;
                planet->gas_percent[2] -= roll;
            } else if (planet->gas_percent[1] > 50) {
                planet->gas_percent[1] -= roll;
                planet->gas_percent[2] += roll;
            }
        }
        if (planet->diameter > 12) {
            planet->diameter -= rnd(3) - 1;
        } else if (planet->diameter > 0) {
            planet->diameter += rnd(3) - 1;
        }
        if (planet->gravity > 100) {
            planet->gravity -= rnd(10);
        } else if (planet->gravity > 0) {
            planet->gravity += rnd(10);
        }
        if (planet->mining_difficulty > 100) {
            planet->mining_difficulty -= rnd(10);
        } else if (planet->mining_difficulty > 0) {
            planet->mining_difficulty += rnd(10);
        }
    }

    // copy from the template into the system's planet data
    for (int pn = 0; pn < star->num_planets; pn++) {
        planet_data_t *p = planet_base + star->planet_index + pn;
        planet_data_t *pd = templateSystem + pn;
        p->temperature_class = pd->temperature_class;
        p->pressure_class = pd->pressure_class;
        p->special = pd->special;
        for (int g = 0; g < 4; g++) {
            p->gas[g] = pd->gas[g];
            p->gas_percent[g] = pd->gas_percent[g];
        }
        p->diameter = pd->diameter;
        p->gravity = pd->gravity;
        p->mining_difficulty = pd->mining_difficulty;
        p->econ_efficiency = pd->econ_efficiency;
        p->md_increase = pd->md_increase;
        p->message = pd->message;
        p->isValid = pd->isValid;
    }
    star->home_system = TRUE;

    return 0;
}


void closest_unvisited_star(struct ship_data *ship) {
    int found = FALSE;
    long shx;
    long shy;
    long shz;
    long stx;
    long sty;
    long stz;
    long closest_distance = 999999;
    long temp_distance;
    struct star_data *star;
    struct star_data *closest_star;

    /* Get array index and bit mask. */
    int species_array_index = (species_number - 1) / 32;
    int species_bit_number = (species_number - 1) % 32;
    long species_bit_mask = 1 << species_bit_number;

    shx = ship->x;
    shy = ship->y;
    shz = ship->z;

    x = -1;

    for (int i = 0; i < num_stars; i++) {
        star = star_base + i;

        /* Check if bit is already set. */
        if (star->visited_by[species_array_index] & species_bit_mask) {
            continue;
        }

        stx = star->x;
        sty = star->y;
        stz = star->z;
        temp_distance = ((shx - stx) * (shx - stx)) + ((shy - sty) * (shy - sty)) + ((shz - stz) * (shz - stz));

        if (temp_distance < closest_distance) {
            x = stx;
            y = sty;
            z = stz;
            closest_distance = temp_distance;
            closest_star = star;
            found = TRUE;
        }
    }

    if (found) {
        fprintf(orders_file, "%d %d %d", x, y, z);
        /* So that we don't send more than one ship to the same place. */
        closest_star->visited_by[species_array_index] |= species_bit_mask;
    } else {
        fprintf(orders_file, "???");
    }
}


// closest_unvisited_star_report is just slight different? why?
void closest_unvisited_star_report(struct ship_data *ship, FILE *fp) {
    int i, found, species_array_index, species_bit_number;
    long shx, shy, shz, stx, sty, stz, closest_distance, temp_distance, species_bit_mask;
    struct star_data *star, *closest_star;

    /* Get array index and bit mask. */
    species_array_index = (species_number - 1) / 32;
    species_bit_number = (species_number - 1) % 32;
    species_bit_mask = 1 << species_bit_number;

    shx = ship->x;
    shy = ship->y;
    shz = ship->z;

    x = 9999;
    closest_distance = 999999;

    found = FALSE;
    for (i = 0; i < num_stars; i++) {
        star = star_base + i;

        /* Check if bit is already set. */
        if (star->visited_by[species_array_index] & species_bit_mask) { continue; }

        stx = star->x;
        sty = star->y;
        stz = star->z;

        temp_distance = ((shx - stx) * (shx - stx)) + ((shy - sty) * (shy - sty)) + ((shz - stz) * (shz - stz));

        if (temp_distance < closest_distance) {
            x = stx;
            y = sty;
            z = stz;
            closest_distance = temp_distance;
            closest_star = star;
            found = TRUE;
        }
    }

    if (found) {
        fprintf(fp, "%d %d %d", x, y, z);
        closest_star->visited_by[species_array_index] |= species_bit_mask;
        /* So that we don't send more than one ship to the same place. */
    } else {
        fprintf(fp, "???");
    }
}


double distanceBetween(star_data_t *s1, star_data_t *s2) {
    double dX = s1->x - s2->x;
    double dY = s1->y - s2->y;
    double dZ = s1->z - s2->z;
    return sqrt(dX * dX + dY * dY + dZ * dZ);
}

// findHomeSystemCandidate returns a randomly picked system that has at least 3 planets,
// is not currently a home system, is not a worm_hole endpoint, and is at least the
// minimum distance from any existing home system. it returns NULL if there are no such systems.
star_data_t *findHomeSystemCandidate(int radius) {
    star_data_t **candidates = calloc(num_stars + 1, sizeof(star_data_t *));
    if (candidates == NULL) {
        perror("findHomeSystemCandidate:");
        exit(2);
    }
    int sidx = 0;
    for (int i = 0; i < num_stars; i++) {
        star_data_t *candidate = star_base + i;
        if (candidate->num_planets >= 3 && candidate->home_system == FALSE && candidate->worm_here == FALSE) {
            candidates[sidx++] = candidate;
        }
    }
    if (sidx == 0) {
        // fprintf(stderr, "error: findHomeSystemCandidate: no candidates meet the criteria for home system!\n");
        free(candidates);
        return NULL;
    }
    // pick one at random by shuffling the list, then iterating through it until we find a match.
    // Fisher and Yates shuffle, updated
    // -- To shuffle an array A of n elements (indices 0..n-1):
    //    for i from n−1 downto 1 do
    //        j ← random integer such that 0 ≤ j ≤ i
    //        swap(A[i], A[j])
    for (int i = sidx - 1; i > 0; i--) {
        // rnd(i)         returns 1 ≤ x ≤ i
        // rnd(i + 1)     returns 1 ≤ x ≤ i+1
        // rnd(i + 1) - 1 returns 0 ≤ x ≤ i
        int j = rnd(i + 1) - 1;
        star_data_t *tmp = candidates[j];
        candidates[j] = candidates[i];
        candidates[i] = tmp;
    }

    // return the first system from the list of candidates that meets the minimum distance criteria.
    for (int i = 0; candidates[i] != NULL; i++) {
        if (hasHomeSystemNeighbor(candidates[i], radius) == FALSE) {
            star_data_t *candidate = candidates[i];
            free(candidates);
            return candidate;
        }
    }
    // fprintf(stderr, "error: findHomeSystemCandidate: no candidates meet the criteria for radius of %d!\n", radius);

    free(candidates);
    return NULL;
}


// hasHomeSystemNeighbor returns TRUE if the star has a neighbor within the given radius that is a home system.
int hasHomeSystemNeighbor(star_data_t *star, int radius) {
    int radiusSquared = radius * radius;
    for (int i = 0; i < num_stars; i++) {
        star_data_t *star2 = star_base + i;
        if (star2->home_system == FALSE) {
            continue;
        }
        int dx = star->x - star2->x;
        int dy = star->y - star2->y;
        int dz = star->z - star2->z;
        if (dx * dx + dy * dy + dz * dz <= radiusSquared) {
            return TRUE;
        }
    }
    return FALSE;
}


void scan(int x, int y, int z, int printLSN) {
    int i, j, k, n, found, num_gases, ls_needed;
    char filename[32];
    struct star_data *star;
    struct planet_data *planet, *home_planet;
    struct nampla_data *home_nampla;

    /* Find star. */
    star = star_base;
    found = FALSE;
    for (i = 0; i < num_stars; i++) {
        if (star->x == x && star->y == y && star->z == z) {
            found = TRUE;
            break;
        }
        star++;
    }

    if (!found) {
        fprintf(log_file, "Scan Report: There is no star system at x = %d, y = %d, z = %d.\n", x, y, z);
        return;
    }

    /* Print data for star, */
    fprintf(log_file, "Coordinates:\tx = %d\ty = %d\tz = %d", x, y, z);
    fprintf(log_file, "\tstellar type = %c%c%c", type_char[star->type], color_char[star->color], size_char[star->size]);
    fprintf(log_file, "   %d planets.\n\n", star->num_planets);

    if (star->worm_here) {
        fprintf(log_file, "This star system is the terminus of a natural wormhole.\n\n");
    }

    /* Print header. */
    fprintf(log_file, "               Temp  Press Mining\n");
    fprintf(log_file, "  #  Dia  Grav Class Class  Diff  LSN  Atmosphere\n");
    fprintf(log_file, " ---------------------------------------------------------------------\n");

    /* Check for nova. */
    if (star->num_planets == 0) {
        fprintf(log_file, "\n\tThis star is a nova remnant. Any planets it may have once\n");
        fprintf(log_file, "\thad have been blown away.\n\n");
        return;
    }

    /* Print data for each planet. */
    planet = planet_base + star->planet_index;
    if (printLSN != FALSE) {
        home_nampla = nampla_base;
        home_planet = planet_base + home_nampla->planet_index;
    }

    for (i = 1; i <= star->num_planets; i++) {
        /* Get life support tech level needed. */
        if (printLSN != FALSE) {
            ls_needed = life_support_needed(species, home_planet, planet);
        } else {
            ls_needed = 99;
        }

        fprintf(log_file, "  %d  %3d  %d.%02d  %2d    %2d    %d.%02d %4d  ",
                i,
                planet->diameter,
                planet->gravity / 100, planet->gravity % 100,
                planet->temperature_class,
                planet->pressure_class,
                planet->mining_difficulty / 100, planet->mining_difficulty % 100,
                ls_needed);

        num_gases = 0;
        for (n = 0; n < 4; n++) {
            if (planet->gas_percent[n] > 0) {
                if (num_gases > 0) {
                    fprintf(log_file, ",");
                }
                fprintf(log_file, "%s(%d%%)", gas_string[planet->gas[n]], planet->gas_percent[n]);
                num_gases++;
            }
        }
        if (num_gases == 0) {
            fprintf(log_file, "No atmosphere");
        }
        fprintf(log_file, "\n");

        planet++;
    }

    if (star->message) {
        /* There is a message that must be logged whenever this star system is scanned. */
        sprintf(filename, "message%d.txt", star->message);
        log_message(filename);
    }
}


char star_color(int c) {
    if (0 <= c && c <= 7) {
        return color_char[c];
    }
    return '?';
}


char star_size(int c) {
    if (0 <= c && c <= 9) {
        return size_char[c];
    }
    return '?';
}


char star_type(int c) {
    if (0 <= c && c <= 4) {
        return type_char[c];
    }
    return '?';
}


/* The following routine will check if coordinates x-y-z contain a star and,
 * if so, will set the appropriate bit in the "visited_by" variable for the star.
 * If the star exists, TRUE will be returned; otherwise, FALSE will be returned. */
int star_visited(int x, int y, int z) {
    int found = FALSE;

    /* Get array index and bit mask. */
    int species_array_index = (species_number - 1) / 32;
    int species_bit_number = (species_number - 1) % 32;
    long species_bit_mask = 1 << species_bit_number;

    for (int i = 0; i < num_stars; i++) {
        struct star_data *star = star_base + i;
        if (x != star->x) { continue; }
        if (y != star->y) { continue; }
        if (z != star->z) { continue; }
        found = TRUE;
        /* Check if bit is already set. */
        if (star->visited_by[species_array_index] & species_bit_mask) { break; }
        /* Set the appropriate bit. */
        star->visited_by[species_array_index] |= species_bit_mask;
        star_data_modified = TRUE;
        break;
    }

    return found;
}
