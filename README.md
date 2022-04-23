# FAR HORIZONS

*This README.md file was last updated by Raven Zachary (raven@rinzai.com) on 1 May 2021.*
FAR HORIZONS is a strategic role-playing game of galactic exploration, trade, diplomacy, and conquest. The first and
second editions were designed for play by postal mail. Later editions were designed for play by electronic mail. The
seventh edition rules and source code were released in 1999 by [Rick Morneau](http://rickmor.x10.mx).

    Far Horizons is a play-by-email strategy game that I designed several
    years ago.  In this game, each player plays the role of an intelligent
    species that can build starships, explore the galaxy, start new colonies,
    meet other species, go to war, and so on.  Far Horizons is a rich and
    realistic simulation, and the rules are considerably longer and more
    complex than most similar games.  A typical game can last anywhere from
    6 months to a year.  Far Horizons is not an open-ended game, so you may
    not start a new species at any time.
    
    I am not currently running a Far Horizons game, and I have no plans to do
    so in the foreseeable future.  However, if you would like to run your own
    game, a complete source code distribution (written in ANSI C and including
    the rules) is available here.

This repository contains the original source code with various bug fixes and improvements. There have been *no changes
to the game mechanics*, and there are no immediate plans to make any.

If you find a change to the game mechanics, please file a bug report on the
[FAR HORIZONS Issue Tracker](https://github.com/Ramblurr/Far-Horizons/issues).

# Rules

You can find the original 7th edition ASCII rules at [doc/rules](doc/rules).

You can also build a PDF and HTML versions of these if you have [pandoc](https://pandoc.org/) installed.

    cd doc/manual
    make
    
A PDF version of the manual has also been included in the doc folder.    

# Change Notes

Our changes are focused on

* bug fixes
* removing compiler warnings
* simplify the build process
* making the data files portable between 32- and 64-bit x86 machines
* creating a single executable to reduce the dependencies on Python and Perl to run game turns

The shell scripts used to compile have been replaced with a single CMakefile. This should allow the engine to build on
Linux, Windows and Mac. Please file a bug report on the
[FAR HORIZONS Issue Tracker](https://github.com/Ramblurr/Far-Horizons/issues)
if you have problems with the build on any of these systems.

We refactored the source and added a common entry point for all the commands. Those changes are covered in later
sections of this document.

The major change to the engine has been the data structures. The original code performed some minor miracles to support
16- and 32-bit systems. We've changed to that support 32- and 64-bit systems.

The internal structures have been cleaned up to remove unused fields.

New structures were created for the external data formats (the binary and JSON data files). The `get` and `save`
functions were updated to convert between the internal and external structures when fetching and saving data.
(A few of the internal structures had new fields added to help with the conversion.)

We replaced the commands used to edit data (`AsciiToBinary`, `BinaryToAscii`, and `Edit`). Gamemasters must convert the
binary data to JSON, edit the JSON data directly, and then convert back to binary. We apologize for the inconvenience of
the extra steps, but it simplifies the code and testing.

Again, we have not intentionally changed any of the game mechanics. If you find a change it is very likely a bug. Please
file a report on the [FAR HORIZONS Issue Tracker](https://github.com/Ramblurr/Far-Horizons/issues).

# Building

You can build with CMake or use one of the Makefile scripts.

Building with CMake requires version 3.5 or newer. We highly recommend installing `ninja-build` along with CMake.

If you're on a Linux machine and have checked out the `main` branch, it should be something like:

    ~$ mkdir -p src

    ~$ cd src
    
    ~/src$ cmake --version
      cmake version 3.5.1
    
    ~/src$ git clone https://github.com/Ramblurr/Far-Horizons.git

    ~/src$ cd Far-Horizons

    ~/src/Far-Horizons$ git remote -v
      origin  git@github.com:Ramblurr/Far-Horizons.git (fetch)
      origin  git@github.com:Ramblurr/Far-Horizons.git (push)
    
    ~/src/Far-Horizons$ cd build
    
    ~/src/Far-Horizons/build$ cmake ..
      -- The C compiler identification is GNU 5.4.0
      -- Check for working C compiler: /usr/bin/cc
      -- Check for working C compiler: /usr/bin/cc -- works
      -- Detecting C compiler ABI info
      -- Detecting C compiler ABI info - done
      -- Detecting C compile features
      -- Detecting C compile features - done
      -- Looking for sqrt in m
      -- Looking for sqrt in m - found
      -- Configuring done
      -- Generating done
      -- Build files have been written to: ~/src/Far-Horizons/build
    
    ~/src/Far-Horizons/build$ cd ..
    
    ~/src/Far-Horizons$ cmake --build build

All the resulting binaries are in `build/`.

Compiling is never easy. Please feel free to reach out to the GitHub site if you have any questions.

# Game Mastering

The game has scripts to help initialize and run a game. See [tools/README.md](tools/README.md).

## Running by Hand

### Example: Creating a New Galaxy

```bash
mkdir gamma
cp examples/farhorizons.cfg gamma/
cp examples/noorders.txt gamma/
cp examples/species.cfg gamma/
cd gamma
FH_SEED=$RANDOM ../build/fh create galaxy --less-crowded --species=18
FH_SEED=$RANDOM ../build/fh show galaxy
FH_SEED=$RANDOM ../build/fh create home-system-templates
FH_SEED=$RANDOM ../build/fh create species --config=species.cfg
FH_SEED=$RANDOM ../build/fh finish
FH_SEED=$RANDOM ../build/fh report
FH_SEED=$RANDOM ../build/fh stats
```

### Example: Running a Turn

```bash
FH_SEED=$RANDOM ../build/fh turn
FH_SEED=$RANDOM ../build/fhorders
FH_SEED=$RANDOM ../build/fh locations
FH_SEED=$RANDOM ../build/fhcombat
FH_SEED=$RANDOM ../build/fh pre-departure
FH_SEED=$RANDOM ../build/fh jump
FH_SEED=$RANDOM ../build/fh production
FH_SEED=$RANDOM ../build/fh post-arrival
FH_SEED=$RANDOM ../build/fh locations
FH_SEED=$RANDOM ../build/fhcombat --strike
FH_SEED=$RANDOM ../build/fh finish
FH_SEED=$RANDOM ../build/fh report
FH_SEED=$RANDOM ../build/fh stats
```

## Create Galaxy

The `fh create-galaxy` command initializes a new game by creating three files:

* `galaxy.dat`, which contains the parameters for the galaxy
* `stars.dat`, which contains data for all the systems in the galaxy
* `planets.dat`, which contains data for all the planets in the galaxy

The command accepts the following options:

* --species=integer, required, defines the number of species
* --stars=integer, optional
* --radius=integer, optional
* --less-crowded, optional
* --suggest-values, optional

The number of species is used to determine the number of stars in the galaxy. The number of stars is used to determine
the radius. As a game master, you can specify the values, or let the program determine them. You can also use
the `--suggest-values` flag to display suggested values based on the number of species.

The `--less-crowded` flag increases the number of stars by about 50%.
(It has no effect if you specify the number of stars yourself.)

Increasing the number of stars tends to slow the pace of the game since it will take longer for species to encounter
each other.

NB: `fh create-galaxy` replaces `NewGalaxy`.

## Show Galaxy

The `fh show galaxy` command displays a very crude ASCII map of the galaxy.

The map shows the locations of ideal home worlds, colonies, and other star systems.

NB: `fh show galaxy` replaces `ShowGalaxy`.

## Create Home System Templates

The `fh create-home-systems` commands creates a set of templates for home systems. The templates are
named `homesystemN.dat` where `N` is the number of planets in the template.
(The number ranges from 3 to 9). The command ensures that one planet in the system template is "earth-like" and will be
a good starting point for a species.

When a species is added to the game, the template is used to updated their home system.

NB: `fh create-home-systems` replaces `MakeHomes`.

# List Galaxy

The `fh list galaxy` command lists all the systems in the galaxy.

The command accepts the following options:

* --planets=bool, optional, default false, reports on planet details
* --wormholes=bool, optional, default false, reports on wormhole details

NB: `fh list galaxy` replaces `ListGalaxy`.

## Make Home System

The command accepts the following options:

* --radius=integer, optional, default 10, minimum distance between home systems
* --system=integer,integer,integer, optional, use the system located at x,y,z
* --force, optional, overrides the check for radius and existing home system

If the `--radius` option is given, this command searches for find a system that has at least 3 planets, is not already a
home system, and is at least the minimum distance from any other home system. If it cannot, it reports an error and
terminates.

If the `--system` option is given, this command verifies that the system has at least 3 planets and is not already a
home system. If not, it reports an error and terminates.

If both `--radius` and `--system` are specified, the command will verify that the system has at least 3 planets, is not
already a home system, and is at least the minimum distance from any other home system. If not, it reports an error and
terminates.

The system is converted to one suitable for a home system by loading the appropriate template (based on the number of
planets in the system). The values for the planets are adjusted by small random amounts to provide some variation.

If there are no problems, the `planets.dat` file is updated with the new data.

NB: `fh update home-system` replaces `MakeHomeSystem` and `MakeHomeSystemAuto`.

## Add Species

NB: `?` replaces `AddSpeciesAuto`.

## Process Combat Commands

NB: `fh combat` replaces `Combat`.

## Process Pre-Departure Commands

NB: `fh pre-departure` replaces `PreDep`.

## Process Jump Commands

NB: `fh jump` replaces `Jump`.

## Process Production Commands

NB: `fh production` replaces `Production`.

## Process Post-Arrival Commands

NB: `fh post-arrival` replaces `PostArrival`.

## Process Strike Commands

NB: `fh combat --strike` replaces `Strike`.

## Finish Turn

NB: `fh finish` replaces `Finish`.

## Generate Turn Reports

NB: `fh report` replaces `Report`.

# License

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

All source code in this repository is licensed under the GPL v3.

The original ANSI C source code and rules are Copyright (c) 1999 by Richard A. Morneau.

# Notes

* [LtanHonor's Gmail Updates](https://github.com/LtanHonor/Far-Horizons/blob/develop/tools/turn_send.py)

## CMake

There's a helpful page on CMake at
[HSF Training](https://hsf-training.github.io/hsf-training-cmake-webpage/02-building/index.html).

## Links

* [PBM List](http://www.pbm.com/~lindahl/pbm_list/)
* [PBeM Magazine](http://www.pbm.com/~lindahl/pbem_magazine.html)
* [Galaxy Design](http://www.pbm.com/~lindahl/pbem_articles/galaxy.design)
* [WarpWar Game](http://www.contrib.andrew.cmu.edu/usr/gc00/reviews/warpwar.html)

