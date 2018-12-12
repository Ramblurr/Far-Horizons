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
import os, tempfile, subprocess, sys, shutil, glob
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

    turn = fhutils.run(bin_dir, "TurnNumber").strip()

    print("Will now perform save for turn %s" % (turn))

    os.chdir(data_dir)

    backup_path = data_dir+"/backup"
    reports_path = data_dir+"/reports"
    stats_path = data_dir+"/reports/stats"
    turn_path = data_dir+"/backup/turn%s" %(turn)
    if not os.path.isdir(backup_path):
        os.makedirs(backup_path)

    if not os.path.isdir(reports_path):
        os.makedirs(reports_path)

    if not os.path.isdir(stats_path):
        os.makedirs(stats_path)

    if not os.path.isdir(turn_path):
        os.makedirs(turn_path)

    print("Moving sp*.ord files to backup/ ...")
    for order in glob.glob(data_dir+"/sp*.ord"):
        fname = os.path.basename(order)
        os.rename(order, "%s/%s" % (turn_path, fname))

    print("Moving *.dat files to backup/ ...")
    for dat in glob.glob(data_dir+"/*.dat"):
        fname = os.path.basename(dat)
        shutil.copyfile(dat, "%s/%s" % (turn_path, fname))

    for hs in glob.glob(data_dir+"/HS*"):
        fname = os.path.basename(hs)
        os.rename(hs, "%s/%s" % (turn_path, fname))

    print("Moving all report files to reports/ ...")
    for rpt in glob.glob(data_dir+"/*.rpt.*"):
        fname = os.path.basename(rpt)
        dest = "%s/%s" % (reports_path, fname)
        os.rename(rpt, dest)

    stats = fhutils.run(bin_dir, "Stats").strip()
    stats_fname = "%s/stats.t%s" % (stats_path, turn)
    print("Writing stats for turn %s to %s" % (turn, stats_fname))
    with open(stats_fname, 'w') as f:
        f.write(stats)

    print("Deleting temporary files...")
    interspecies_path = data_dir + "/interspecies.dat"
    if os.path.isfile(interspecies_path):
        os.remove(interspecies_path)
    for log in glob.glob(data_dir+"/*.log"):
        os.remove(log)

    print("Done!")


if __name__ == "__main__":
    main(sys.argv[1:])
