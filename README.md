FAR HORIZONS
============

FAR HORIZONS is a strategic role-playing game of galactic exploration, trade,
diplomacy, and conquest. The first and second editions were designed for play
by postal mail. Later editions were designed for play by electronic mail. The
seventh edition rules and source code were released in 1999 by [Rick
Morneau](http://rickmor.x10.mx).

This repository contains the original source code with various bug fixes and
improvements. There have been *no changes to the game mechanics*, and there are
no immediate plans to make any.

RULES
-----

* [Rules in HTML](doc/manual/manual.html)
* [Rules in PDF](doc/manual/manual.pdf)

You can find the original 7th edition ASCII rules at [doc/rules](doc/rules).

You can also build a PDF and HTML versions (if you have docker/podman
installed):

    cd doc/manual/latex
    make all

BUILDING
--------

Compiling is easy:

    $ cd src/
    $ ./make.all

All of the resulting binaries are in bin/

GAME MASTERING
--------------

See [tools/README.md](tools/README.md).


LICENSE
-------

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

All source code in this repository is licesned under the GPL v2.

The original ANSI C source code and rules are Copyright 1999 by Richard A. Morneau
