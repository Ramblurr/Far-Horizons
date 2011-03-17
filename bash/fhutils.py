import gdata.docs
import gdata.docs.service
import gdata.spreadsheet.service
import re, os, string, sys, subprocess, itertools
import smtplib
import mimetypes
import yaml
from dateutil import parser
from dateutil import rrule
import dateutil
import datetime
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.MIMEAudio import MIMEAudio
from email.MIMEImage import MIMEImage
from email.Encoders import encode_base64

def run(bindir, tool, args = []):
    path = ["%s/%s" %(bindir, tool)]
    try:
        if len(args):
            path.extend(args)
        out = subprocess.check_output(path)
        return out
    except subprocess.CalledProcessError:
        print "Error detected in program %s!" % tool
        sys.exit(1)

def natatime(itr, fillvalue=None, n=2):
    """
    get values from an iterator n at a time
    http://stackoverflow.com/questions/1528711/reading-lines-2-at-a-time/1528769#1528769
    """
    return itertools.izip_longest(*(iter(itr),)*n, fillvalue=fillvalue)

class GameConfig(object):

    def __init__(self, config_file="farhorizons.yml"):
        self.config_file = config_file
        self.load_config()
        
    def load_config(self):
        try:
            stream = file(self.config_file, 'r')
            self.config = yaml.load(stream)
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
                weekdays = tuple(map(lambda x: parser.parse(x, tzinfos=dateutil.tz.tzutc).weekday(), self.config[game]['deadlines'])) 
                hours = tuple(map(lambda x: parser.parse(x, tzinfos=dateutil.tz.tzutc).hour, self.config[game]['deadlines'])) 
                minutes = tuple(map(lambda x: parser.parse(x, tzinfos=dateutil.tz.tzutc).minute, self.config[game]['deadlines'])) 
                d['deadline'] = rrule.rrule(rrule.WEEKLY, byweekday=weekdays,byhour=hours,byminute=minutes,dtstart=datetime.datetime.now(dateutil.tz.tzutc()))
                    
                if 'tmpdir' in self.config[game]:
                    d['tmpdir'] = self.config[game]['tmpdir']
                self.gameslist.append( d )
            

        except yaml.YAMLError, exc:
            print "Error parsing %s file" % (self.config_file)
            print exc
            sys.exit(1)

    def registrations(self):
        return RegistrationSpreadsheet(self.user, self.password, self.doc_name)

    def send_mail(self, subject, recipient, text, *attachmentFilePaths):
        msg = MIMEMultipart()
        msg['From'] = self.user
        msg['To'] = recipient
        msg['Subject'] = subject
        msg.attach(MIMEText(text))
        for attachmentFilePath in attachmentFilePaths:
            msg.attach(self._getAttachment(attachmentFilePath))
        mailServer = smtplib.SMTP('smtp.gmail.com', 587)
        mailServer.ehlo()
        mailServer.starttls()
        mailServer.ehlo()
        mailServer.login(self.user, self.password)
        mailServer.sendmail(self.user, recipient, msg.as_string())
        mailServer.close()
        #print('Sent email to %s' % recipient)
        
    def _getAttachment(self, attachmentFilePath):
        contentType, encoding = mimetypes.guess_type(attachmentFilePath)

        if contentType is None or encoding is not None:
            contentType = 'application/octet-stream'

        mainType, subType = contentType.split('/', 1)
        file = open(attachmentFilePath, 'rb')

        if mainType == 'text':
            attachment = MIMEText(file.read())
        elif mainType == 'message':
            attachment = email.message_from_file(file)
        elif mainType == 'image':
            attachment = MIMEImage(file.read(),_subType=subType)
        elif mainType == 'audio':
            attachment = MIMEAudio(file.read(),_subType=subType)
        else:
            attachment = MIMEBase(mainType, subType)
        attachment.set_payload(file.read())
        encode_base64(attachment)

        file.close()

        attachment.add_header('Content-Disposition', 'attachment',   filename=os.path.basename(attachmentFilePath))
        return attachment
        
    def save(self):
        stream = file(self.config_file, 'w')
        yaml.dump(self.config, stream, default_flow_style=False)
        
    def write_tmpdir(self, game, tmpdir):
        """
        Saves the tmpdir as a value under game.
        """
        if not game in self.config:
            return
        self.config[game]['tmpdir'] = tmpdir
        self.save()

class RegistrationSpreadsheet(object):
    def __init__(self, user, password, doc_name):
        self.user = user
        self.password = password
        self.doc_name = doc_name
        self.gd_connect()
        
    def gd_connect(self):
        # Connect to Google
        self.gd_client = gdata.spreadsheet.service.SpreadsheetsService()
        self.gd_client.email = self.user
        self.gd_client.password = self.password
        self.gd_client.source = 'payne.org-example-1'
        self.gd_client.ProgrammaticLogin()

    def get_registrations(self):
        if self.gd_client is None:
            gd_connect()
        q = gdata.spreadsheet.service.DocumentQuery()
        q['title'] = self.doc_name
        q['title-exact'] = 'true'
        feed = self.gd_client.GetSpreadsheetsFeed(query=q)
        spreadsheet_id = feed.entry[0].id.text.rsplit('/',1)[1]
        feed = self.gd_client.GetWorksheetsFeed(spreadsheet_id)
        worksheet_id = feed.entry[0].id.text.rsplit('/',1)[1]
        rows = self.gd_client.GetListFeed(spreadsheet_id, worksheet_id).entry
        return rows

    def update_row(self, row, dict):
        entry = self.gd_client.UpdateRow( row, dict)
        if isinstance(entry, gdata.spreadsheet.SpreadsheetsList):
            return True
        else:
            return False

class Game(object):
    def __init__(self):
        self.players = []
        with open('fh_names') as f:
            for num,name,email in natatime(f,'',3):
                self.players.append({'num':num.strip(), 'name':name.strip(), 'email':email.strip()})