FAR HORIZONS
============

*This README.md file was last updated by Raven Zachary (raven@rinzai.com) on 1 May 2021.*

FAR HORIZONS is a strategic play-by-email (PBeM) game of galactic exploration, trade, diplomacy, and conquest. The first and second editions were designed for play by postal mail. Later editions were designed for play by electronic mail. The seventh edition rules and source code were released in 1999 by [Rick Morneau](http://rickmor.x10.mx). The code was then modernized and updated by Casey Link in the 2009-2011 timeframe. Casey also created helpful Python scripts for the GM in the tools/ folder. In 2021, Raven Zachary forked Casey Link's inactive GitHub project and began to make additional updates.

This repository contains the original source code with various bug fixes and improvements, primarily made by Casey Link. There have been *no changes to the game mechanics*, and there are no immediate plans to make any.

RULES
-----

You can find the original 7th edition ASCII rules at [doc/rules](doc/rules).

You can also build a PDF and HTML versions of these if you have [pandoc](https://pandoc.org/) installed.

    cd doc/manual
    make
    
A PDF version of the manual has also been included in the doc folder.    

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
