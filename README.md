FAR HORIZONS
============

FAR HORIZONS is a strategic role-playing game of galactic exploration, trade,
diplomacy, and conquest. The first and second editions were designed for play
by postal mail. Later editions were designed for play by electronic mail. The
seventh edition rules and source code were released in 1999 by
[Rick Morneau](http://rickmor.x10.mx).

This repository contains the original source code with various bug fixes and
improvements. There have been *no changes to the game mechanics*, and there are
no immediate plans to make any.

RULES
-----

You can find the original 7th edition ASCII rules at [doc/rules](doc/rules).

You can also build a PDF and HTML versions of these if you have
[pandoc](https://pandoc.org/) installed.

    cd doc/manual
    make

BUILDING
--------

Compiling is never easy, but if you're on the `main` branch, it should be something like:

    ~/src/Far-Horizons$ git remote -v
      origin  git@github.com:mdhender/Far-Horizons.git (fetch)
      origin  git@github.com:mdhender/Far-Horizons.git (push)
    
    ~/src/Far-Horizons$ cmake --version
      cmake version 3.5.1
    
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
      -- Build files have been written to: /home/mdhender/src/Far-Horizons/build
    
    ~/src/Far-Horizons/build$ cd ..
    
    ~/src/Far-Horizons$ cmake --build build

All the resulting binaries are in `build/`.

GAME MASTERING
--------------

See [tools/README.md](tools/README.md).


LICENSE
-------

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

All source code in this repository is licensed under the GPL v3.

The original ANSI C source code and rules are Copyright 1999 by Richard A. Morneau.

# Notes
## Links
* [PBM List](http://www.pbm.com/~lindahl/pbm_list/)
* [PBeM Magazine](http://www.pbm.com/~lindahl/pbem_magazine.html)
* [Galaxy Design](http://www.pbm.com/~lindahl/pbem_articles/galaxy.design)
* [WarpWar Game](http://www.contrib.andrew.cmu.edu/usr/gc00/reviews/warpwar.html)

## CMake
* [Basics](https://hsf-training.github.io/hsf-training-cmake-webpage/02-building/index.html)

    cmake -S . -B build
    cmake -S . -B .
    cmake --build .

Highly recommend installing `ninja-build` along with CMake.
