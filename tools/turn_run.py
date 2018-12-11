#!/usr/bin/env python
"""
    Usage: run_turn.py [-h] -c config.yml -d

    -h, --help      print this message
    -c config.yml   use a particular config file
    -d, --discard   discard the exisiting turn processing and start over

    This script will create a temporary directory in /tmp and copy
    all Far Horizons files to it that are needed for processing
    orders.  Next, the main FH programs will be run.  If you are
    happy with the results, you can then run script "turn_save.py" to
    save them.

"""
import fhutils
import os, tempfile, subprocess, sys, shutil
import getopt

def main(argv):
    config_file = ''
    discard = False
    try:
        opts, args = getopt.getopt(argv, "hc:d", ["help", "config=","discard"])
    except getopt.GetoptError:
        print(__doc__)
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
        elif opt in ("-c", "--config"):
            config_file = arg
        elif opt in ("-d", "--discard"):
            discard = True

    if config_file:
        config = fhutils.GameConfig(config_file)
    else:
        config = fhutils.GameConfig()
    game = config.gameslist[0] # for now we only support a single game
    game_name = game['name']
    data_dir = game['datadir']
    bin_dir = config.bindir

    if not os.path.isdir(data_dir):
        print("Sorry data directory %s does not exist." % (data_dir))
        sys.exit(2)

    if not os.path.isdir(bin_dir):
        print("Sorry bin directory %s does not exist." % (bin_dir))
        sys.exit(2)
    os.chdir(data_dir)
    if os.path.isfile("interspecies.dat"):
        print("interspecies.dat present. Have you forgotten to fhclean?")
        sys.exit(2)
    try:
        tempdir = game['tmpdir']
        if tempdir != '' and not discard:
            print("Temp directory exists (%s). Do you need to save? Maybe you need --discard?" % (game['tmpdir']))
            sys.exit(2)
        elif not os.path.isdir(tempdir) and discard:
            config.write_tmpdir(game_name, "")
        elif tempdir and discard:
            shutil.rmtree(tempdir)
    except KeyError:
        pass

    tempdir = tempfile.mkdtemp("fhtest")
    config.write_tmpdir(game_name, tempdir)
    print("Using tempdir %s" % (tempdir))
    print("Copying all needed files to /tmp/fhtest.dir/ ...")
    os.system("cp -p *.ord %s" % (tempdir))
    os.system("cp -p *.dat %s" % (tempdir))
    os.system("cp -p *.txt %s" % (tempdir))
    os.system("cp -p *.msg %s" % (tempdir))
    os.system("cp -p *.log %s" % (tempdir))

    os.chdir(tempdir)

    t = fhutils.run(bin_dir, "TurnNumber").strip()
    if t != '0':
        print("Running NoOrders...")
        fhutils.run(bin_dir, "NoOrders")
    else:
        print("Turn 1 hasn't happened yet, running Locations")
        fhutils.run(bin_dir, "Locations")

    print("Running Combat...")
    ret = fhutils.run(bin_dir, "Combat")

    print("Running PreDeparture...")
    fhutils.run(bin_dir, "PreDeparture")

    print("Running Jump...")
    fhutils.run(bin_dir, "Jump")

    print("Running Production...")
    fhutils.run(bin_dir, "Production")

    print("Running PostArrival...")
    fhutils.run(bin_dir, "PostArrival")

    print("Running Locations...")
    fhutils.run(bin_dir, "Locations")

    print("Running Strike...")
    fhutils.run(bin_dir, "Strike")

    print("Running Finish...")
    fhutils.run(bin_dir, "Finish")

    print("Running Report...")
    fhutils.run(bin_dir, "Report")

    print("Success! Verify turn results: %s" % (tempdir))

if __name__ == "__main__":
    main(sys.argv[1:])
