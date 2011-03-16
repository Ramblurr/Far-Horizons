#!/bin/bash
FH_DIR=/home/ramblurr/src/fh/games/ga
BIN_DIR=/home/ramblurr/src/fh/engine/bin
LISTGALAXY=$BIN_DIR/ListGalaxy
STARMAP=$BIN_DIR/PrintMap
PS2PDF=/usr/bin/ps2pdf
#############3
cd $FH_DIR
$LISTGALAXY -p | awk 'BEGIN { OFS = ", "} {print $3,$6,$9,"unnamed",$13"."}' > /tmp/fh.map.raw.$$
num=`wc -l /tmp/fh.map.raw.$$ | awk '{print $1}'` 
head -n $(($num-4)) /tmp/fh.map.raw.$$ > /tmp/fh.map.$$
cd /tmp

#3d map
$STARMAP -t /tmp/fh.map.$$
$PS2PDF -dAutoRotatePages=/None /tmp/fh.map.$$.ps $FH_DIR/galaxy_map_3d.pdf


$STARMAP /tmp/fh.map.$$
$PS2PDF -dAutoRotatePages=/None /tmp/fh.map.$$.ps $FH_DIR/galaxy_map.pdf

rm /tmp/fh.map.*

cd -
