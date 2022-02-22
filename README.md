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

Compiling is easy:

    $ cd src/
    $ ./make.all

All the resulting binaries are in bin/

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
