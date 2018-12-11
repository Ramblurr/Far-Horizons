#!/usr/bin/env python
"""
    Usage: turn_send.py [-h] -c config.yml [-s num]
    
    -h, --help      print this message
    -c config.yml   use a particular config file
    -t              perform a test run, do not send any email.
    -s species num  only do one species number

    This script will read file fh_names and mail report files to the
    appropriate players.  File deadline.msg will be inserted at the
    head of each report file before mailing.

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

DO NOT send HTML formatted email!
You can also attach a "orders.txt" or "orders.ord" PLAIN TEXT file to your email.

I generally download orders within half an hour of the above deadline.  If they
have not arrived by that time, or if the Subject line is not correctly
formatted, then I will not be able to process them.
"""

start_msg = """
The FH:TA game of Galaxy Alpha has begun!

Your Far Horizons %s first turn packet is attached. It contains your first 
turn report, the galaxy in several different formats, and the revised 
game policies (please read them).

%s

As always, if there any problems, or you have a question, then shoot me an email.

Best,
Casey
Gamemaster
"""

message = """
Your Far Horizons %s turn results are attached.

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
    file_name = None
    subject = None
    try:                                
        opts, args = getopt.getopt(argv, "hc:ts:f:u:", ["help", "config=","test","species","file","subject"])
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
        elif opt in ("-f", "--file"):
            file_name = arg
        elif opt in ("-u", "--subject"):
            subject = arg
    if (file_name != None and subject == None) or (file_name == None and subject != None):
        print("if you specify file_name, you must specify subject.");
        sys.exit(1)

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
    next_deadline = deadline_rule.after(datetime.now(dateutil.tz.tzutc()))
    est = zoneinfo.gettz('America/New_York')
    pst = zoneinfo.gettz('America/Los_Angeles')
    day = next_deadline.strftime("%A %B %d")
    time = next_deadline.strftime("%H:%M (%I:%M %p) %Z")
    time += "\n= " + next_deadline.astimezone(est).strftime("%I:%M %p %Z")
    time += "\n= " + next_deadline.astimezone(pst).strftime("%I:%M %p %Z")

    deadline_msg = deadline_msg %(day, time,game_stub, game_stub)
    msg = start_msg %(game_name, deadline_msg) if turn == "1" else message %(game_name, deadline_msg)
    for player in players:
        if species_num != None and species_num != player['num']:
            print("skipping %s - %s" %(player['num'], player['name']))
            continue
        if player['email'] == "player_dropped":
            print("skipping dropped player %s - %s" %(player['num'], player['name']))
            continue
        if file_name != None:
            report = "%s/%s" %(data_dir, file_name)
        elif turn == "1":
            report = "%s/sp%s.zip" %(data_dir, player['num'])
            subject ="FH %s Game Start - %s" % (game_stub, player['name'])
        else:
            report = "%s/sp%s.rpt.t%s" %(data_dir, player['num'], turn)
            subject = "FH %s Turn Results - %s turn %s" % (game_stub, player['name'], turn)
        if not test_flag:
            print("Mailing %s to %s (sp %s)" %(report, player['email'], player['name']))
            config.send_mail(subject, player['email'], msg, report)
        else:
            print("Writing .test file")
            with open(report+".test", "w") as f:
                f.write("To: %s\n" %( player['email']))
                f.write("Subject: %s\n" %( subject))
                f.write(msg)
                f.write("Attached: %s\n"  % (report))

if __name__ == "__main__":
    main(sys.argv[1:])