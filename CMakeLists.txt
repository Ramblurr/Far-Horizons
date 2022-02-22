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

add_executable(fh
        src/fh.c src/fh.c
        src/commandvars.h src/commandvars.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxyio.h src/galaxyio.c
        src/item.h src/item.c
        src/location.h src/location.c
        src/locationio.h src/locationio.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/report.h src/report.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/scan.h src/scan.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/stats.h src/stats.c
        src/transactionio.h src/transactionio.c)

add_executable(fhorders
        src/fhorders.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxyio.h src/galaxyio.c
        src/item.h src/item.c
        src/jumpvars.h src/jumpvars.c
        src/location.h src/location.c
        src/locationio.h src/locationio.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/no_orders.h src/no_orders.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c)

add_executable(fhcombat
        src/fhcombat.c
        src/combat.h src/combat.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxy.h src/galaxy.c
        src/galaxyio.h src/galaxyio.c
        src/galaxyvars.h src/galaxyvars.c
        src/item.h src/item.c
        src/jumpvars.h src/jumpvars.c
        src/location.h src/location.c
        src/locationio.h src/locationio.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c
        src/transaction.c src/transaction.h
        src/transactionio.h src/transactionio.c)

add_executable(fhpredep
        src/fhpredep.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/dev_log.h src/dev_log.c
        src/do.h src/do.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxy.h src/galaxy.c
        src/galaxyio.h src/galaxyio.c
        src/galaxyvars.h src/galaxyvars.c
        src/intercept.h src/intercept.c
        src/item.h src/item.c
        src/jumpvars.h src/jumpvars.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/money.h src/money.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/predeparture.c src/predeparture.h
        src/productionvars.h src/productionvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c
        src/transaction.c src/transaction.h
        src/transactionio.h src/transactionio.c)

add_executable(fhjump
        src/fhjump.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/dev_log.h src/dev_log.c
        src/do.h src/do.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxy.h src/galaxy.c
        src/galaxyio.c
        src/galaxyvars.h src/galaxyvars.c
        src/intercept.h src/intercept.c
        src/item.h src/item.c
        src/jump.h src/jump.c
        src/jumpvars.h src/jumpvars.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/money.h src/money.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/productionvars.h src/productionvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c
        src/transaction.c src/transaction.h
        src/transactionio.h src/transactionio.c)

add_executable(fhprod
        src/fhprod.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/dev_log.h src/dev_log.c
        src/do.h src/do.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxyio.c
        src/intercept.h src/intercept.c
        src/item.h src/item.c
        src/jumpvars.h src/jumpvars.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/money.h src/money.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/production.h src/production.c
        src/productionvars.h src/productionvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c
        src/transactionio.h src/transactionio.c)

add_executable(fhpost
        src/fhpost.c
        src/command.h src/command.c
        src/commandvars.h src/commandvars.c
        src/dev_log.h src/dev_log.c
        src/do.h src/do.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxyio.h src/galaxyio.c
        src/intercept.h src/intercept.c
        src/item.h src/item.c
        src/jumpvars.h src/jumpvars.c
        src/locationvars.h src/locationvars.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/money.h src/money.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/ordervars.h src/ordervars.c
        src/planet.h src/planet.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/postarrival.c src/postarrival.h
        src/productionvars.h src/productionvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/star.h src/star.c
        src/stario.h src/stario.c
        src/starvars.h src/starvars.c
        src/transactionio.h src/transactionio.c)

add_executable(fhfinish
        src/fhfinish.c
        src/commandvars.h src/commandvars.c
        src/engine.h src/engine.c
        src/enginevars.h src/enginevars.c
        src/galaxyio.h src/galaxyio.c
        src/item.h src/item.c
        src/location.h src/location.c
        src/locationio.h src/locationio.c
        src/log.h src/log.c
        src/logvars.h src/logvars.c
        src/nampla.h src/nampla.c
        src/namplaio.h src/namplaio.c
        src/namplavars.h src/namplavars.c
        src/planetio.h src/planetio.c
        src/planetvars.h src/planetvars.c
        src/ship.h src/ship.c
        src/shipio.h src/shipio.c
        src/shipvars.h src/shipvars.c
        src/species.h src/species.c
        src/speciesio.h src/speciesio.c
        src/speciesvars.h src/speciesvars.c
        src/transactionio.h src/transactionio.c)
