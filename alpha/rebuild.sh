#!/bin/bash
tput clear

cd /Volumes/habi/Software/mdhender/Far-Horizons/alpha || exit 2

FH_EXE=../cmake-build-debug/fh
if [ ! -x "${FH_EXE}" ]; then
  echo "error: can't find '${FH_EXE}'"
  exit 2
fi

echo " info: purging old files..."
for file in *.dat *.json *.log *.map *.txt sp*.rpt.t*
do
  [ -f "${file}" ] && {
    rm -f "${file}"
  }
done
echo " info: purging old test directories..."
rm -rf tst??? || exit 2

echo
echo " info: running tests..."

export FH_SEED=90123
${FH_EXE} create galaxy --less-crowded --species=18 || exit 2
turn_number=$(${FH_EXE} list turn_number)
if [ "${turn_number}" -ne "0" ]; then
  echo "error: turn_number: want '0': got '${turn_number}'"
  exit 2
fi
radius=$(${FH_EXE} list radius)
if [ "${radius}" -ne "25" ]; then
  echo "error: radius: want '25': got '${radius}'"
  exit 2
fi
d_num_species=$(${FH_EXE} list d_num_species)
if [ "${d_num_species}" -ne "18" ]; then
  echo "error: d_num_species: want '18': got '${d_num_species}'"
  exit 2
fi
num_species=$(${FH_EXE} list num_species)
if [ "${num_species}" -ne "0" ]; then
  echo "error: num_species: want '0': got '${num_species}'"
  exit 2
fi
num_stars=$(${FH_EXE} list num_stars)
if [ "${num_stars}" -ne "162" ]; then
  echo "error: num_stars: want '162': got '${num_stars}'"
  exit 2
fi
num_planets=$(${FH_EXE} list num_planets)
if [ "${num_planets}" -ne "606" ]; then
  echo "error: num_planets: want '606: got '${num_planets}'"
  exit 2
fi
num_wormholes=$(${FH_EXE} list num_wormholes)
if [ "${num_wormholes}" -ne "0" ]; then
  echo "error: num_wormholes: want '7': got '${num_wormholes}'"
  exit 2
fi
mkdir tst001 || exit 2
mv galaxy.dat planets.dat stars.dat tst001/ || exit 2

${FH_EXE} create galaxy --less-crowded --species=18 >/dev/null || exit 2
for file in galaxy.dat planets.dat stars.dat; do
  cknew=$(cksum ${file})
  cktst=$(cd tst001 && cksum ${file})
  if [ "${cknew}" != "${cktst}" ]; then
    echo "error: cksum ${file} tst001/${file} does not match"
    exit 2
  fi
done

# test the home systems
${FH_EXE} create home-system-templates || exit 2
mkdir tst002 || exit 2
mv galaxy.dat planets.dat stars.dat homesystem?.dat tst002/ || exit 2

${FH_EXE} create galaxy --less-crowded --species=18 >/dev/null || exit 2
${FH_EXE} create home-system-templates >/dev/null || exit 2

for file in homesystem?.dat; do
  cknew=$(cksum ${file})
  cktst=$(cd tst002 && cksum ${file})
  if [ "${cknew}" != "${cktst}" ]; then
    echo "error: cksum ${file} tst002/${file} does not match"
    exit 2
  fi
done

# add the five test species
num_species=$(${FH_EXE} list num_species)
if [ "${num_species}" -ne "0" ]; then
  echo "error: num_species: want '0': got '${num_species}'"
  exit 2
fi
${FH_EXE} create species --config=species.cfg > /dev/null || exit 2
num_species=$(${FH_EXE} list num_species)
if [ "${num_species}" -ne "5" ]; then
  echo "error: num_species: want '5': got '${num_species}'"
  exit 2
fi

${FH_EXE} finish || exit 2

turn_number=$(${FH_EXE} list turn_number)
if [ "${turn_number}" -ne "1" ]; then
  echo "error: turn_number: want '1': got '${turn_number}'"
  exit 2
fi

rm -f *.sexpr

cp inp001/sp??.ord .

${FH_EXE} production || exit 2
${FH_EXE} export json >/dev/null || exit 2
for file in species.001.json species.001.planets.json species.001.ships.json; do
  cknew=$(cksum ${file})
  cktst=$(cd inp001 && cksum ${file})
  if [ "${cknew}" != "${cktst}" ]; then
    echo "error: cksum ${file} inp001/${file} does not match"
    exit 2
  fi
done
${FH_EXE} import json >/dev/null || exit 2
${FH_EXE} export json >/dev/null || exit 2
for file in species.001.json species.001.planets.json species.001.ships.json; do
  cknew=$(cksum ${file})
  cktst=$(cd inp001 && cksum ${file})
  if [ "${cknew}" != "${cktst}" ]; then
    echo "error: cksum ${file} inp001/${file} does not match"
    exit 2
  fi
done

exit 0

