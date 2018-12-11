#!/usr/bin/env python
"""
    This script will create a zip file containing a first turn packet for a player.
"""
import fhutils
import os, sys, tempfile, subprocess
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
    
    # prepare galaxy list
    output = fhutils.run(bin_dir, "ListGalaxy", ["-p"])
    with open("galaxy.list.txt", "w") as f:
        f.write(output)
        
    players = fhutils.Game().players
    for p in players:
        try:
            subprocess.check_call(["zip", "sp%s.zip" % (p['num']), "sp%s.rpt.t1" % (p['num']), "galaxy.map.pdf", "galaxy.map.txt",  "game_policies.pdf", "galaxy.list.txt"])
        except CalledProcessError:
            print("ERROR making zip: sp%s.zip" % (p['num']))

if __name__ == "__main__":
    main(sys.argv[1:])