# Far Horizons Game Engine
# Copyright (C) 2022 Michael D Henderson
# Copyright (C) 2021 Raven Zachary
# Copyright (C) 2019 Casey Link, Adam Piggott
# Copyright (C) 1999 Richard A. Morneau
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.21)
project(far_horizons C)
include_directories(src)

set(CMAKE_C_STANDARD 99)

add_executable(fhturn
        src/galaxy.h src/galaxy.c
        src/fhturn.c)

add_executable(fhloc
        src/engine.h
        src/galaxy.h src/galaxy.c
        src/planet.h src/planet.c
        src/species.h src/species.c
        src/nampla.h src/nampla.c
        src/ship.h src/ship.c
        src/item.h src/item.c
        src/location.h src/location.c
        src/fhloc.c)

add_executable(fhorders
        src/engine.h src/engine.c
        src/galaxy.h src/galaxy.c
        src/star.h src/star.c
        src/planet.h src/planet.c
        src/species.h src/species.c
        src/nampla.h src/nampla.c
        src/ship.h src/ship.c
        src/location.h src/location.c
        src/no_orders.h src/no_orders.c
        src/NoOrders.c)