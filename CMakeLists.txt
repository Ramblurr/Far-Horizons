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

#cmake_minimum_required(VERSION 3.21)
cmake_minimum_required(VERSION 3.5)
project(far_horizons C)
include_directories(src)

set(CMAKE_C_STANDARD 99)

include(CheckLibraryExists)

CHECK_LIBRARY_EXISTS(m sqrt "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(USE_LIB_M m)
endif (HAVE_LIB_M)

add_executable(fh
        src/fh.c src/fh.h
        src/cfgfile.c src/cfgfile.h
        src/commandvars.c src/commandvars.h
        src/create.c src/create.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/export.c src/export.h
        src/galaxy.c src/galaxy.h
        src/galaxyio.c src/galaxyio.h
        src/item.c src/item.h
        src/list.c src/list.h
        src/location.c src/location.h
        src/locationio.c src/locationio.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/report.c src/report.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/scan.c src/scan.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/stats.c src/stats.h
        src/transactionio.c src/transactionio.h
        src/turn.c src/turn.h
        src/update.c src/update.h)

target_link_libraries(fh ${USE_LIB_M})

add_executable(fhorders
        src/fhorders.c
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxyio.c src/galaxyio.h
        src/item.c src/item.h
        src/jumpvars.c src/jumpvars.h
        src/location.c src/location.h
        src/locationio.c src/locationio.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/no_orders.c src/no_orders.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h)

target_link_libraries(fhorders ${USE_LIB_M})

add_executable(fhcombat
        src/fhcombat.c
        src/combat.c src/combat.h
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxy.c src/galaxy.h
        src/galaxyio.c src/galaxyio.h
        src/galaxyvars.c src/galaxyvars.h
        src/item.c src/item.h
        src/jumpvars.c src/jumpvars.h
        src/location.c src/location.h
        src/locationio.c src/locationio.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transaction.h src/transaction.c
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhcombat ${USE_LIB_M})

add_executable(fhpredep
        src/fhpredep.c
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/dev_log.c src/dev_log.h
        src/do.c src/do.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxy.c src/galaxy.h
        src/galaxyio.c src/galaxyio.h
        src/galaxyvars.c src/galaxyvars.h
        src/intercept.c src/intercept.h
        src/item.c src/item.h
        src/jumpvars.c src/jumpvars.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/money.c src/money.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/predeparture.h src/predeparture.c
        src/productionvars.c src/productionvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transaction.h src/transaction.c
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhpredep ${USE_LIB_M})

add_executable(fhjump
        src/fhjump.c
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/dev_log.c src/dev_log.h
        src/do.c src/do.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxy.c src/galaxy.h
        src/galaxyio.c src/galaxyio.h
        src/galaxyvars.c src/galaxyvars.h
        src/intercept.c src/intercept.h
        src/item.c src/item.h
        src/jump.c src/jump.h
        src/jumpvars.c src/jumpvars.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/money.c src/money.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/productionvars.c src/productionvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transaction.h src/transaction.c
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhjump ${USE_LIB_M})

add_executable(fhprod
        src/fhprod.c
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/dev_log.c src/dev_log.h
        src/do.c src/do.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxyio.c src/galaxyio.h
        src/intercept.c src/intercept.h
        src/item.c src/item.h
        src/jumpvars.c src/jumpvars.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/money.c src/money.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/production.c src/production.h
        src/productionvars.c src/productionvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhprod ${USE_LIB_M})

add_executable(fhpost
        src/fhpost.c
        src/command.c src/command.h
        src/commandvars.c src/commandvars.h
        src/dev_log.c src/dev_log.h
        src/do.c src/do.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxyio.c src/galaxyio.h
        src/intercept.c src/intercept.h
        src/item.c src/item.h
        src/jumpvars.c src/jumpvars.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/money.c src/money.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/ordervars.c src/ordervars.h
        src/planet.c src/planet.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/postarrival.h src/postarrival.c
        src/productionvars.c src/productionvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhpost ${USE_LIB_M})

add_executable(fhfinish
        src/fhfinish.c
        src/commandvars.c src/commandvars.h
        src/engine.c src/engine.h
        src/enginevars.c src/enginevars.h
        src/galaxyio.c src/galaxyio.h
        src/item.c src/item.h
        src/location.c src/location.h
        src/locationio.c src/locationio.h
        src/locationvars.c src/locationvars.h
        src/log.c src/log.h
        src/logvars.c src/logvars.h
        src/nampla.c src/nampla.h
        src/namplaio.c src/namplaio.h
        src/namplavars.c src/namplavars.h
        src/planetio.c src/planetio.h
        src/planetvars.c src/planetvars.h
        src/ship.c src/ship.h
        src/shipio.c src/shipio.h
        src/shipvars.c src/shipvars.h
        src/species.c src/species.h
        src/speciesio.c src/speciesio.h
        src/speciesvars.c src/speciesvars.h
        src/star.c src/star.h
        src/stario.c src/stario.h
        src/starvars.c src/starvars.h
        src/transactionio.c src/transactionio.h)

target_link_libraries(fhfinish ${USE_LIB_M})
