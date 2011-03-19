#!/usr/bin/env python
"""
    Usage: turn_save.py [-h] -c config.yml -d 
    
    -h, --help      print this message
    -c config.yml   use a particular config file

    This utility saves the results of a turn.
    It will copy all *.dat files to directory "backup" and move all
    *.ord files to it.
    It will move all report files to the directory "reports".
    Next, it will create a current statistics report and write
    it to the report directory.
    Finally, it will delete all unneeded temporary files.

"""
import fhutils
import os, tempfile, subprocess, sys, shutil
import getopt

def main(argv):
    config_file = None
    discard = False
    try:                                
        opts, args = getopt.getopt(argv, "hc:", ["help", "config="])
    except getopt.GetoptError:          
        print __doc__                     
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"): 
            print __doc__                     
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

    turn = fhutils.run(bin_dir, "TurnNumber").strip()

    print "Will now perform save for turn %s" % (turn)
    
    os.chdir(data_dir)
    
    if not os.path.isdir(data_dir+"/backup"):
        print "Sorry backup directory %s does not exist." % (data_dir+"/backup")
        sys.exit(1)

    if not os.path.isdir(data_dir+"/reports"):
        print "Sorry reports directory %s does not exist." % (data_dir+"/reports")
        sys.exit(1)
        
    print 'Moving sp*.ord files to backup/ ...'
    os.system("mkdir -p backup/turn%s" %(turn))
    os.system("mv sp*.ord backup/turn%s/" %(turn))
    os.system("cp *.dat backup/turn%s/" %(turn))

    print 'Moving all report files to reports/ ...'
    os.system("mv *.rpt.* %s" %(data_dir+"/reports/"))

    print "Writing statistics for turn %s to reports/stats/stats.t%s..." % (turn, turn)
    os.system("mkdir -p %s" % (data_dir+"reports/stats"))
    os.system("%s/Stats > reports/stats/stats.t%s" %(bin_dir, turn))

    print "Deleting temporary files..."
    os.system("rm -f interspecies.dat")
    os.system("rm -f *.log")

    print "Done!"

if __name__ == "__main__":
    main(sys.argv[1:])    
