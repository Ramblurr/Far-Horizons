FAR HORIZONS
============

This README is available on GitHub at [Ramblurr/Far-Horizons](https://github.com/Ramblurr/Far-Horizons/blob/main/README.md).

FAR HORIZONS is a strategic play-by-email (PBeM) game of galactic exploration,
trade, diplomacy, and conquest. The first and second editions were designed for
play by postal mail. Later editions were designed for play by electronic mail.
The seventh edition rules and source code were released in 1999 by [Rick
Morneau](http://rickmor.x10.mx).

The code for editions 1 - 6 are not available, and likely never existed on the
WWW.

The original 7th edition codebase as posted by Rick is available at [his
website](http://rickmor.x10.mx), but in the event that that link dies it is
also [available here on
github](https://github.com/Ramblurr/Far-Horizons/releases/tag/v7).

This repository contains the original source code with various bug fixes and
improvements. There have been *no (intentional) changes to the
game mechanics*, and there are no immediate plans to make any.

CHANGES
-------

Updates to this codebase have been made sporadically over the years. The
project was imported into git in 2009 by slawcok. slawcok was part of a polish
contigent of FH players.

Since then the following folks have contributed to the codebase:

* slawcok
* rozenfeld.piotr
* Casey Link
* mjoyner
* Raven Zachary
* Michael D Henderson

If you're interested in what changes have been made, see the git history.

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

All source code in this repository is licenced under the GPL v2.

The original ANSI C source code and rules are Copyright 1999 by Richard A.
Morneau.
