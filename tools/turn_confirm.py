#!/usr/bin/env python
"""
    Usage: turn_save.py [-h] -c config.yml -d 
    
    -h, --help      print this message
    -c config.yml   use a particular config file

    This script will save the results of script "fhtest".  It will
    copy files *.dat, *.ord, and *.rpt.* to the directory where
    "fhtest" started.  It will then cd to that directory.
"""
import fhutils
import os, tempfile, subprocess, sys
import getopt
import shutil

def main(argv):
    config_file = ''
    discard = False
    try:                                
        opts, args = getopt.getopt(argv, "hc:d", ["help", "config="])
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
    tempdir = ''
    try:
        tempdir = game['tmpdir']
        print(tempdir)
        if not os.path.isdir(tempdir):
            raise KeyError
    except KeyError: 
        print("Temp directory doesn't exist for this game. You should use turn_run.py first.")
        sys.exit(2)

    os.chdir(tempdir)

    print("Copying *.dat files to $odn...")
    os.system("cp -p *.dat %s" % (data_dir))
    print("Copying order files to $odn...")
    os.system("cp -p *.ord %s" % (data_dir))
    print("Copying report files to $odn...")
    os.system("cp -p *.rpt.* %s" % (data_dir))

    os.chdir(data_dir)
    print("Turn Complete")
    shutil.rmtree(tempdir)

if __name__ == "__main__":
    main(sys.argv[1:])