#!/usr/bin/env python
"""
    Usage: turn_inject.py [-h] -c config.yml < message to inject

    -h, --help      print this message
    -c config.yml   use a particular config file
    -t              perform a test run, do not send any email.

    This script injects a message into the beginning of the current turn reports.
    It replaces SP_NAME with the corresponding species' name.

"""
import fhutils
import os, sys, time
import getopt


def main(argv):
    config_file = None
    test_flag = False
    try:
        opts, args = getopt.getopt(argv, "hc:t", ["help", "config=","test"])
    except getopt.GetoptError:
        print(__doc__)
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
        elif opt in ("-c", "--config"):
            config_file = arg
        elif opt in ("-t", "--test"):
            test_flag = True


    if config_file:
        config = fhutils.GameConfig(config_file)
    else:
        config = fhutils.GameConfig()
    game = config.gameslist[0] # for now we only support a single game
    game_name = game['name']
    game_stub = game['stub']
    deadline_rule = game['deadline']
    data_dir = game['datadir']
    bin_dir = config.bindir
    players = fhutils.Game().players

    if not os.path.isdir(data_dir):
        print("Sorry data directory %s does not exist." % (data_dir))
        sys.exit(2)

    if not os.path.isdir(bin_dir):
        print("Sorry bin directory %s does not exist." % (bin_dir))
        sys.exit(2)

    turn = fhutils.run(bin_dir, "TurnNumber").strip()

    inject = sys.stdin.read()

    for player in players:
        inject_p = inject.replace("SP_NAME", player['name'])
        report_name = "%s/sp%s.rpt.t%s" %(data_dir, player['num'], turn)
        with open(report_name, 'r') as original: report = original.read()
        if test_flag:
            print(inject_p + "\n\n" + report)
        else:
            with open(report_name, 'w') as modified: modified.write( inject_p + "\n\n" + report)

if __name__ == "__main__":
    main(sys.argv[1:])
