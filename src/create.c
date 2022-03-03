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
#include <string.h>
#include "create.h"
#include "galaxy.h"
#include "galaxyio.h"
#include "planetio.h"
#include "stario.h"


int createGalaxyCommand(int argc, char *argv[]);

int createHomeSystemTemplatesCommand(int argc, char *argv[]);

int createSpeciesCommand(int argc, char *argv[]);


int createCommand(int argc, char *argv[]) {
    const char *cmdName = argv[0];
    get_galaxy_data();
    get_star_data();
    get_planet_data();
    for (int i = 1; i < argc; i++) {
        // fprintf(stderr, "fh: %s: argc %2d argv '%s'\n", cmdName, i, argv[i]);
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr, "fh: usage: create (galaxy | home-system-templates | species)\n");
            return 2;
        } else if (strcmp(opt, "galaxy") == 0) {
            return createGalaxyCommand(argc - i, argv + i);
        } else if (strcmp(opt, "home-system-templates") == 0) {
            return createHomeSystemTemplatesCommand(argc - i, argv + i);
        } else if (strcmp(opt, "species") == 0) {
            return createSpeciesCommand(argc - i, argv + i);
        } else {
            fprintf(stderr, "fh: %s: unknown option '%s'\n", cmdName, opt);
            return 2;
        }
    }
    return 0;
}


int createGalaxyCommand(int argc, char *argv[]) {
    int desiredNumSpecies = 0;
    int desiredNumStars = 0;
    int galacticRadius = 0;
    int lessCrowded = FALSE;
    int suggestValues = FALSE;

    for (int i = 1; i < argc; i++) {
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr,
                    "fh: usage: create galaxy --species=integer [--stars=integer] [--radius=integer] [--suggest-values]\n");
            return 2;
        } else if (strcmp(opt, "--less-crowded") == 0) {
            lessCrowded = TRUE;
        } else if (strcmp(opt, "--radius") == 0 && val != NULL) {
            galacticRadius = atoi(val);
            if (galacticRadius < MIN_RADIUS || galacticRadius > MAX_RADIUS) {
                fprintf(stderr, "error: radius must be between %d and %d parsecs.\n", MIN_RADIUS, MAX_RADIUS);
                return 2;
            }
        } else if (strcmp(opt, "--species") == 0 && val != NULL) {
            desiredNumSpecies = atoi(val);
            if (desiredNumSpecies < MIN_SPECIES || desiredNumSpecies > MAX_SPECIES) {
                fprintf(stderr, "error: species must be between %d and %d.\n", MIN_SPECIES, MAX_SPECIES);
                return 2;
            }
        } else if (strcmp(opt, "--stars") == 0 && val != NULL) {
            desiredNumStars = atoi(val);
            if (desiredNumStars < MIN_STARS || desiredNumStars > MAX_STARS) {
                fprintf(stderr, "error: stars must be between %d and %d.\n", MIN_STARS, MAX_STARS);
                return 2;
            }
        } else if (strcmp(opt, "--suggest-values") == 0) {
            suggestValues = TRUE;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    if (desiredNumSpecies == 0) {
        fprintf(stderr, "error: you must supply the desired number of species.\n");
        return 2;
    }

    if (desiredNumStars == 0 || suggestValues != FALSE) {
        if (lessCrowded == FALSE) {
            desiredNumStars = (desiredNumSpecies * STANDARD_NUMBER_OF_STAR_SYSTEMS) / STANDARD_NUMBER_OF_SPECIES;
        } else {
            // bump the number of stars by 50% to make it take longer to encounter other species.
            desiredNumStars =
                    (3 * desiredNumSpecies * STANDARD_NUMBER_OF_STAR_SYSTEMS) / (2 * STANDARD_NUMBER_OF_SPECIES);
        }
        if (desiredNumStars > MAX_STARS) {
            fprintf(stderr, "error: calculation results in a number greater than %d stars.\n", MAX_STARS);
            return 2;
        }
    }

    if (galacticRadius == 0 || suggestValues != FALSE) {
        long minVolume = desiredNumStars
                         * STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS * STANDARD_GALACTIC_RADIUS
                         / STANDARD_NUMBER_OF_STAR_SYSTEMS;
        for (galacticRadius = MIN_RADIUS; galacticRadius * galacticRadius * galacticRadius < minVolume;) {
            galacticRadius++;
        }
        if (galacticRadius > MAX_RADIUS) {
            fprintf(stderr, "error: calculation results in a radius greater than %d parsecs.\n", MAX_RADIUS);
            return 2;
        }
    }

    if (suggestValues != FALSE) {
        printf(" info: for %d species, a %sgalaxy needs about %d star systems.\n",
               desiredNumSpecies, lessCrowded == FALSE ? "" : "less crowded ", desiredNumStars);
        printf(" info: for %d stars, the galaxy should have a radius of about %d parsecs.\n",
               desiredNumStars, galacticRadius);
        return 0;
    }

    return createGalaxy(galacticRadius, desiredNumStars, desiredNumSpecies);
}


int createHomeSystemTemplatesCommand(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr,
                    "fh: usage: create home-system-templates...\n");
            return 2;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return createHomeSystemTemplates();
}


int createSpeciesCommand(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *opt = argv[i];
        char *val = NULL;
        for (val = opt; *val != 0; val++) {
            if (*val == '=') {
                *val = 0;
                val++;
                break;
            }
        }
        if (*val == 0) {
            val = NULL;
        }
        if (strcmp(opt, "--help") == 0 || strcmp(opt, "-h") == 0 || strcmp(opt, "-?") == 0) {
            fprintf(stderr,
                    "fh: usage: create species...\n");
            return 2;
        } else {
            fprintf(stderr, "error: unknown option '%s'\n", opt);
            return 2;
        }
    }

    return 0;
}
