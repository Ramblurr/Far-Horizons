#!/usr/bin/env python
"""
    Usage: turn_send.py [-h] -c config.yml [-s num]

    -h, --help      print this message
    -c config.yml   use a particular config file
    -t              perform a test run, do not send any email.
    -s species num  only do one species number

    This script will read file fh_names and mail reminders to every
    species that does not have .ord files in the data directory.

"""
import fhutils
import os, sys, time
import getopt
from datetime import datetime
from dateutil import zoneinfo
from dateutil.relativedelta import *
import dateutil

deadline_msg ="""
Orders for the next turn are due on %s at:
%s

Please format the subject line of your orders as in the following example:
Subject: FH Orders, %s
or
Subject: FH Orders, %s wait

See the game policies for information on 'wait'.

I generally download orders within half an hour of the above deadline.  If they
have not arrived by that time, or if the Subject line is not correctly
formatted, then I will not be able to process them.
"""

message = """

Just a gentle reminder that Far Horizons %s orders are due soon!

%s

As always, if there any problems, or you have a question, then shoot me an email.

Best,
Casey
Gamemaster
"""

def main(argv):
    config_file = None
    test_flag = False
    species_num = None
    try:
        opts, args = getopt.getopt(argv, "hc:ts:", ["help", "config=","test","species"])
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
        elif opt in ("-s", "--species"):
            species_num = arg

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
    global message,deadline_msg, start_msg
    next_deadline = deadline_rule.after(datetime.now(config['zone']))
    est = zoneinfo.gettz('America/New_York')
    pst = zoneinfo.gettz('America/Los_Angeles')
    poland = zoneinfo.gettz('Europe/Warsaw')
    day = next_deadline.strftime("%A %B %d")
    time = next_deadline.strftime("%H:%M (%I:%M %p) %Z")
    time += "\n= %s" % (next_deadline.astimezone(est).strftime("%I:%M %p %Z"))
    time += "\n= %s" % (next_deadline.astimezone(pst).strftime("%I:%M %p %Z"))
    time += "\n= %s" % (next_deadline.astimezone(poland).strftime("%H:%M %Z"))

    deadline_msg = deadline_msg %(day, time,game_stub, game_stub)
    msg = message %(game_name, deadline_msg)
    for player in players:
        if species_num != None and species_num != player['num']:
            print("skipping %s - %s" %(player['num'], player['name']))
            continue
        orders = "%s/sp%s.ord" %(data_dir, player['num'])
        if os.path.isfile(orders):
            print("found orders for %s" %(player['name']))
            continue
        subject = "FH %s Orders Reminder - %s" % (game_stub, player['name'])
        if not test_flag:
            print("Mailing reminder to %s (sp %s)" %(player['email'], player['name']))
            config.send_mail(subject, player['email'], msg)
        else:
            print("Writing .test file")
            with open("sp%s.test"%(player['num']), "w") as f:
                f.write("To: %s\n" %( player['email']))
                f.write("Subject: %s\n" %( subject))
                f.write(msg)

if __name__ == "__main__":
    main(sys.argv[1:])
