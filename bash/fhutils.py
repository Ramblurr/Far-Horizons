import gdata.docs
import gdata.docs.service
import gdata.spreadsheet.service
import re, os, string, sys, subprocess
import smtplib
import mimetypes
import yaml
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

class GameConfig(object):

    def __init__(self, config_file = 'farhorizons.yml'):
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
                if 'tmpdir' in self.config[game]:
                    d['tmpdir'] = self.config[game]['tmpdir']
                self.gameslist.append( d )
            

        except yaml.YAMLError, exc:
            print "Error parsing %s file" % (config_file)
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
            msg.attach(getAttachment(attachmentFilePath))
        mailServer = smtplib.SMTP('smtp.gmail.com', 587)
        mailServer.ehlo()
        mailServer.starttls()
        mailServer.ehlo()
        mailServer.login(self.user, self.password)
        mailServer.sendmail(self.user, recipient, msg.as_string())
        mailServer.close()
        print('Sent email to %s' % recipient)
        
    def save(self):
        stream = file(self.config_file, 'w')
        yaml.dump(self.config, stream)
        
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
