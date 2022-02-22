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

#include <string.h>
#include "engine.h"
#include "planet.h"
#include "speciesvars.h"


void fix_gases(struct planet_data *pl) {
    int i, j, total, left, add_neutral;
    long n;

    total = 0;
    for (i = 0; i < 4; i++) {
        total += pl->gas_percent[i];
    }

    if (total == 100) {
        return;
    }

    left = 100 - total;

    /* If we have at least one gas that is not the required gas, then we simply need to adjust existing gases.
     * Otherwise, we have to add a neutral gas. */
    add_neutral = TRUE;
    for (i = 0; i < 4; i++) {
        if (pl->gas_percent[i] == 0) {
            continue;
        }
        if (pl->gas[i] == species->required_gas) {
            continue;
        }
        add_neutral = FALSE;
        break;
    }

    if (add_neutral) {
        goto add_neutral_gas;
    }

    /* Randomly modify existing non-required gases until total percentage is exactly 100. */
    for (; left != 0;) {
        i = rnd(4) - 1;
        if (pl->gas_percent[i] == 0) {
            continue;
        }
        if (pl->gas[i] == species->required_gas) {
            continue;
        }
        if (left > 0) {
            if (left > 2) {
                j = rnd(left);
            } else {
                j = left;
            }

            pl->gas_percent[i] += j;
            left -= j;
        } else {
            if (-left > 2) {
                j = rnd(-left);
            } else {
                j = -left;
            }

            if (j < pl->gas_percent[i]) {
                pl->gas_percent[i] -= j;
                left += j;
            }
        }
    }

    return;

    add_neutral_gas:

    /* If we reach this point, there is either no atmosphere or it contains only the required gas.
     * In either case, add a random neutral gas. */
    for (i = 0; i < 4; i++) {
        if (pl->gas_percent[i] > 0) {
            continue;
        }
        j = rnd(6) - 1;
        pl->gas[i] = species->neutral_gas[j];
        pl->gas_percent[i] = left;

        break;
    }
}


// generate_planets creates planets and inserts them into the planet_data array.
void generate_planets(struct planet_data *first_planet, int num_planets) {
    /* Values for the planets of Earth's solar system will be used as starting values.
     * Diameters are in thousands of kilometers.
     * The zeroth element of each array is a placeholder and is not used.
     * The fifth element corresponds to the asteroid belt, and is pure fantasy on my part.
     * I omitted Pluto because it is probably a captured planet, rather than an original member of our solar system. */
    static int start_diameter[10] = {0, 5, 12, 13, 7, 20, 143, 121, 51, 49};
    static int start_temp_class[10] = {0, 29, 27, 11, 9, 8, 6, 5, 5, 3};

    int diameter[10];
    int g[10];
    int gas[10][5];
    int gas_percent[10][5];
    int mining_dif;
    int mining_difficulty[10];
    int pressure_class[10];
    int temperature_class[10];

    /* Main loop. Generate one planet at a time. */
    for (int planet_number = 1; planet_number <= num_planets; planet_number++) {
        /* Start with diameters, temperature classes and pressure classes based on the planets in Earth's solar system. */
        int baseValue;
        if (num_planets <= 3) {
            // nudge towards the Earth-like zone
            baseValue = 2 * planet_number + 1;
        } else {
            baseValue = (9 * planet_number) / num_planets;
        }
        int dia = start_diameter[baseValue];
        int tc = start_temp_class[baseValue];

        /* Randomize the diameter. */
        int die_size = dia / 4;
        if (die_size < 2) {
            die_size = 2;
        }
        for (int i = 1; i <= 4; i++) {
            int roll = rnd(die_size);
            if (rnd(100) > 50) {
                dia = dia + roll;
            } else {
                dia = dia - roll;
            }
        }
        /* Minimum allowable diameter is 3,000 km.
         * Note that the maximum diameter we can generate is 283,000 km. */
        for (; dia < 3;) {
            dia += rnd(4);
        }
        diameter[planet_number] = dia;

        /* If diameter is greater than 40,000 km, assume the planet is a gas giant. */
        int gas_giant;
        if (dia > 40) {
            gas_giant = TRUE;
        } else {
            gas_giant = FALSE;
        }

        /* Density will depend on whether the planet is a gas giant.
         * Again ignoring Pluto, densities range from 0.7 to 1.6 times the
         * density of water for the gas giants, and from 3.9 to 5.5 for the
         * others. We will expand this range slightly and use 100 times the
         * actual density so that we can use integer arithmetic. */
        int density;
        if (gas_giant == TRUE) {
            /* Final values from 0.60 through 1.70 (scaled to 60 thru 170). */
            density = 58 + rnd(56) + rnd(56);
        } else {
            /* Final values from 3.70 through 5.70 (scaled to 370 thru 570). */
            density = 368 + rnd(101) + rnd(101);
        }

        /* Gravitational acceleration is proportional to the mass divided by the radius-squared.
         * The radius is proportional to the diameter, and the mass is proportional to the
         * density times the radius-cubed. The net result is that "g" is proportional to
         * the density times the diameter. Our value for "g" will be a multiple of Earth
         * gravity, and will be further multiplied by 100 to allow us to use integer arithmetic.
         *
         * The factor 72 ensures that "g" will be 100 for Earth (density=550 and diameter=13). */
        int grav = (density * diameter[planet_number]) / 72;
        g[planet_number] = grav;

        /* Randomize the temperature class obtained earlier. */
        die_size = tc / 4;
        if (die_size < 2) {
            die_size = 2;
        }
        int n_rolls = rnd(3) + rnd(3) + rnd(3);
        for (int i = 1; i <= n_rolls; i++) {
            if (rnd(100) > 50) {
                tc = tc + rnd(die_size);
            } else {
                tc = tc - rnd(die_size);
            }
        }

        if (gas_giant == TRUE) {
            for (; tc < 3;) {
                tc += rnd(2);
            }
            for (; tc > 7;) {
                tc -= rnd(2);
            }
        } else {
            for (; tc < 1;) {
                tc += rnd(3);
            }
            for (; tc > 30;) {
                tc -= rnd(3);
            }
        }

        /* Sometimes, planets close to the sun in star systems with less than four planets are too cold.
         * Warm them up a little. */
        if (num_planets < 4 && planet_number < 3) {
            for (; tc < 12;) {
                tc += rnd(4);
            }
        }
        /* Make sure that planets farther from the sun are not warmer than planets closer to the sun. */
        if (planet_number > 1 && temperature_class[planet_number - 1] < tc) {
            tc = temperature_class[planet_number - 1];
        }
        temperature_class[planet_number] = tc;

        /* Pressure class depends primarily on gravity.
         * Calculate an approximate value and randomize it. */
        int pc = g[planet_number] / 10;
        die_size = pc / 4;
        if (die_size < 2) {
            die_size = 2;
        }
        n_rolls = rnd(3) + rnd(3) + rnd(3);
        for (int i = 1; i <= n_rolls; i++) {
            if (rnd(100) > 50) {
                pc = pc + rnd(die_size);
            } else {
                pc = pc - rnd(die_size);
            }
        }
        if (gas_giant) {
            for (; pc < 11;) {
                pc += rnd(3);
            }
            for (; pc > 29;) {
                pc -= rnd(3);
            }
        } else {
            for (; pc < 0;) {
                pc += rnd(3);
            }
            for (; pc > 12;) {
                pc -= rnd(3);
            }
        }
        if (grav < 10) {
            /* Planet's gravity is too low to retain an atmosphere. */
            pc = 0;
        } else if (tc < 2 || tc > 27) {
            /* Planets outside this temperature range have no atmosphere. */
            pc = 0;
        }
        pressure_class[planet_number] = pc;

        /* Generate gases, if any, in the atmosphere. */
        for (int i = 1; i <= 4; i++) {
            /* Initialize. */
            gas[planet_number][i] = 0;
            gas_percent[planet_number][i] = 0;
        }
        if (pc != 0) {
            /* Convert planet's temperature class to a value between 1 and 9.
             * We will use it as the start index into the list of 13 potential gases. */
            int first_gas = 100 * tc / 225;
            if (first_gas < 1) {
                first_gas = 1;
            } else if (first_gas > 9) {
                first_gas = 9;
            }

            /* The following algorithm is something I tweaked until it worked well. */
            int num_gases_wanted = (rnd(4) + rnd(4)) / 2;
            int num_gases_found = 0;
            int gas_quantity = 0;
            for (; num_gases_found == 0;) {
                for (int i = first_gas; i <= first_gas + 4; i++) {
                    if (num_gases_wanted == num_gases_found) {
                        break;
                    }
                    if (i == HE) {
                        /* Treat Helium specially. */
                        if (rnd(3) > 1) {
                            /* Don't want too many He planets. */
                            continue;
                        } else if (tc > 5) {
                            /* Too hot for helium. */
                            continue;
                        }
                        num_gases_found++;
                        gas[planet_number][num_gases_found] = HE;
                        gas_percent[planet_number][num_gases_found] = rnd(20);
                    } else {
                        /* Not Helium. */
                        if (rnd(3) == 3) {
                            continue;
                        }
                        num_gases_found++;
                        gas[planet_number][num_gases_found] = i;
                        if (i == O2) {
                            /* Oxygen is self-limiting. */
                            gas_percent[planet_number][num_gases_found] = rnd(50);
                        } else {
                            gas_percent[planet_number][num_gases_found] = rnd(100);
                        }
                    }
                    gas_quantity += gas_percent[planet_number][num_gases_found];
                }
            }

            /* Now convert gas quantities to percentages. */
            int total_percent = 0;
            for (int i = 1; i <= num_gases_found; i++) {
                gas_percent[planet_number][i] = 100 * gas_percent[planet_number][i] / gas_quantity;
                total_percent += gas_percent[planet_number][i];
            }
            /* Give leftover to first gas. */
            gas_percent[planet_number][1] += 100 - total_percent;
        }

        /* Get mining difficulty.
         * Basically, mining difficulty is proportional to planetary diameter
         * with randomization and an occasional big surprise.
         * Actual values will range between 0.80 and 10.00.
         * Again, the actual value will be multiplied by 100
         * to allow use of integer arithmetic. */
        int mining_dif = 0;
        for (; mining_dif < 40 || mining_dif > 500;) {
            mining_dif = (rnd(3) + rnd(3) + rnd(3) - rnd(4)) * rnd(dia) + rnd(30) + rnd(30);
        }

        /* Fudge factor. */
        mining_dif *= 11;
        mining_dif /= 5;

        mining_difficulty[planet_number] = mining_dif;
    }

    /* Copy planet data to structure. */
    for (int i = 1; i <= num_planets; i++) {
        int planetidx = i - 1;
        planet_data_t *current_planet = first_planet + planetidx;

        /* Initialize all bytes of record to zero. */
        memset(current_planet, 0, sizeof(struct planet_data));

        current_planet->diameter = diameter[i];
        current_planet->gravity = g[i];
        current_planet->mining_difficulty = mining_difficulty[i];
        current_planet->temperature_class = temperature_class[i];
        current_planet->pressure_class = pressure_class[i];
        current_planet->special = 0;

        for (int n = 0; n < 4; n++) {
            current_planet->gas[n] = gas[i][n + 1];
            current_planet->gas_percent[n] = gas_percent[i][n + 1];
        }
    }
}
