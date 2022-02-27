# FAR HORIZONS

FAR HORIZONS is a strategic role-playing game of galactic exploration, trade, diplomacy, and conquest.
The first and second editions were designed for play by postal mail.
Later editions were designed for play by electronic mail.
The seventh edition rules and source code were released in 1999 by [Rick Morneau](http://rickmor.x10.mx).

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

This repository contains the original source code with various bug fixes and improvements.
There have been *no changes to the game mechanics*, and there are no immediate plans to make any.

# Rules

You can find the original 7th edition ASCII rules at [doc/rules](doc/rules).

You can also build a PDF and HTML versions of these if you have [pandoc](https://pandoc.org/) installed.

    cd doc/manual
    make

# Building

You can build with CMake or use one of the Makefile scripts.

Building with CMake requires version 3.5 or newer.
We highly recommend installing `ninja-build` along with CMake.

If you're on a Linux machine and have checked out the `main` branch,
it should be something like:

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

Compiling is never easy.
Please feel free to reach out to the GitHub site if you have any questions.

# Game Mastering

The game has scripts to help initialize and run a game.
See [tools/README.md](tools/README.md).

# License

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

All source code in this repository is licensed under the GPL v3.

The original ANSI C source code and rules are Copyright (c) 1999 by Richard A. Morneau.

# Notes

## CMake

There's a helpful page on CMake at
[HSF Training](https://hsf-training.github.io/hsf-training-cmake-webpage/02-building/index.html).

## Links
* [PBM List](http://www.pbm.com/~lindahl/pbm_list/)
* [PBeM Magazine](http://www.pbm.com/~lindahl/pbem_magazine.html)
* [Galaxy Design](http://www.pbm.com/~lindahl/pbem_articles/galaxy.design)
* [WarpWar Game](http://www.contrib.andrew.cmu.edu/usr/gc00/reviews/warpwar.html)

