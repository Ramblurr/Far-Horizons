#!/usr/bin/env python
"""

 The following script will auto-generate galaxy using data piped via stdio
 Player data is expected in the following format(one line per player):

 email,sp_name,home_planet_name,gov_name,gov_type,ML,GV,LS,BI

 See example.csv for example

 Execute from game directory: game_setup.py < ../example.csv

"""

import fhutils
import os, tempfile, subprocess, sys, shutil, csv
import getopt

def main(argv):
    config_file = None
    discard = False
    try:
        opts, args = getopt.getopt(argv, "hc:", ["help", "config="])
    except getopt.GetoptError:
        print(__doc__)
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
        elif opt in ("-c", "--config"):
            config_file = arg

    if config_file:
        config = fhutils.GameConfig(config_file)
    else:
        config = fhutils.GameConfig()
    game = config.gameslist[0] # for now we only support a single game
    game_name = game['name']
    data_dir = game['datadir']
    bin_dir = config.bindir
    os.chdir(data_dir)

    reader = csv.reader(sys.stdin, delimiter=',')
    data = []
    for row in reader:
        data.append(row)

    num_species = len(data)

    print("%s species defined." % (num_species));

    print("Executing NewGalaxy ")

    # Galaxy size is calculated based on default star density and fixed number of stars per race
    # Adjust it here to create less crowded galaxy, players are asking for that
    adjusted_num_species = str(int(( num_species * 3) / 2))

    fhutils.run(bin_dir, "NewGalaxy", [adjusted_num_species])

    print("Executing MakeHomes")
    fhutils.run(bin_dir, "MakeHomes")

    print("Executing ListGalaxy")

    output = fhutils.run(bin_dir, "ListGalaxy", ["-p"])

    star_list = [s for s in output.splitlines() if s] # strips empty lines from output
    uniq_list = list(set(star_list))  # uniques the list
    if len(star_list) != len(uniq_list):
        print("Galaxy contains duplicate stars!!!")
        sys.exit(1)

    curr_sp_number = 1

    try:
        fh_names = open("fh_names", "w")
        fh_names.truncate()
    except IOError:
        print("cannot open fh_names")
        sys.exit(1)

    for col in data:
        email, sp_name, home_planet, gov_name, gov_type, ML, GV, LS, BI = col

        print("\t Executing HomeSystemAuto (%s) " % (curr_sp_number))
        output = fhutils.run(bin_dir, "HomeSystemAuto", ["12"])
        x,y,z,n = output.split(" ")

        print("\t Executing AddSpecies (%s) " % (curr_sp_number))
        arg = [ str(curr_sp_number), sp_name, home_planet, gov_name, gov_type, x, y, z, n, ML, GV, LS, BI]
        output = fhutils.run(bin_dir, "AddSpeciesAuto", arg)

        try:
            fh_names.write("%02d\n%s\n%s\n" % (curr_sp_number, sp_name, email))
        except IOError:
            print("cannot write to fh_names")
            sys.exit(1)

        curr_sp_number += 1
    fh_names.close()

    print("\t Preparing Turn 1: executing Finish")
    fhutils.run(bin_dir, "Finish")
    print("\t Reporting Turn 1: executing Report")
    fhutils.run(bin_dir, "Report")
    print("DONE")

if __name__ == "__main__":
    main(sys.argv[1:])
