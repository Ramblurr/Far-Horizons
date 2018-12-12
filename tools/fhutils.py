import re, os, string, sys, subprocess, itertools
import smtplib
import mimetypes
import yaml
from dateutil import rrule
from dateutil.parser import parse
from dateutil import tz
import dateutil
import datetime

def run(bindir, tool, args = []):
    path = ["%s/%s" %(bindir, tool)]
    try:
        if len(args):
            path.extend(args)
        out = subprocess.check_output(path).decode("utf-8")
        return out
    except subprocess.CalledProcessError:
        print("Error detected in program %s!" % tool)
        sys.exit(1)

def natatime(itr, fillvalue=None, n=2):
    """
    get values from an iterator n at a time
    http://stackoverflow.com/questions/1528711/reading-lines-2-at-a-time/1528769#1528769
    """
    return itertools.zip_longest(*(iter(itr),)*n, fillvalue=fillvalue)

class GameConfig(object):

    def __init__(self, config_file="farhorizons.yml"):
        self.config_file = config_file
        self.load_config()

    def load_config(self):
        try:
            with open(self.config_file, 'r') as f:
                self.config = yaml.load(f)
            self.user = self.config['googleaccount']['user']
            self.doc_name = self.config['googleaccount']['spreadsheet']
            self.password = self.config['googleaccount']['password']

            self.bindir = self.config['bindir']
            self.gameslist = []
            for game in self.config['games']:
                d = dict()
                d['name'] = game
                d['stub'] = self.config[game]['stub']
                d['datadir'] = self.config[game]['datadir']
                d['timezone'] = self.config[game]['timezone']
                add_default_tz = lambda x, tzinfo: x.replace(tzinfo=x.tzinfo or tzinfo)
                zone = tz.gettz(d['timezone'])
                d['zone'] = zone
                do_parse = lambda x: add_default_tz(parse(x), zone)

                weekdays = tuple([do_parse(x).weekday() for x in self.config[game]['deadlines']])
                hours = tuple([do_parse(x).hour for x in self.config[game]['deadlines']])
                minutes = tuple([do_parse(x).minute for x in self.config[game]['deadlines']])
                d['deadline'] = rrule.rrule(rrule.WEEKLY, byweekday=weekdays,byhour=hours,byminute=minutes,dtstart=datetime.datetime.now(zone))

                if 'tmpdir' in self.config[game]:
                    d['tmpdir'] = self.config[game]['tmpdir']
                self.gameslist.append( d )


        except yaml.YAMLError as exc:
            print("Error parsing %s file" % (self.config_file))
            print(exc)
            sys.exit(1)

    def registrations(self):
        """Return registrations from the spreadsheet"""
        pass

    def save(self):
        with open(self.config_file, 'w') as f:
            yaml.dump(self.config, f, default_flow_style=False)

    def write_tmpdir(self, game, tmpdir):
        """
        Saves the tmpdir as a value under game.
        """
        if not game in self.config:
            return
        self.config[game]['tmpdir'] = tmpdir
        self.save()

class Game(object):
    def __init__(self):
        self.players = []
        with open('fh_names') as f:
            for num,name,email in natatime(f,'',3):
                self.players.append({'num':num.strip(), 'name':name.strip(), 'email':email.strip().lower()})
