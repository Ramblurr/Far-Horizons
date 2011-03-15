import gdata.docs
import gdata.docs.service
import gdata.spreadsheet.service
import re, os, string, ConfigParser, sys
import smtplib
import mimetypes
from email.MIMEMultipart import MIMEMultipart
from email.MIMEBase import MIMEBase
from email.MIMEText import MIMEText
from email.MIMEAudio import MIMEAudio
from email.MIMEImage import MIMEImage
from email.Encoders import encode_base64

user = ""
password = ""
doc_name = ""

gd_client = None

def load_config():
    try:
        config = ConfigParser.ConfigParser()
        config.read('gmail.cfg')
        global user, doc_name, password
        user = config.get("main", "user")
        doc_name = config.get("main", "spreadsheet")
        password = config.get("main", "password")
    except ConfigParser.Error:
        print "Error parsing gmail.cfg file"
        sys.exit(1)

def send_mail(subject, recipient, text, *attachmentFilePaths):
    msg = MIMEMultipart()
    msg['From'] = user
    msg['To'] = recipient
    msg['Subject'] = subject
    msg.attach(MIMEText(text))
    for attachmentFilePath in attachmentFilePaths:
        msg.attach(getAttachment(attachmentFilePath))
        mailServer = smtplib.SMTP('smtp.gmail.com', 587)
        mailServer.ehlo()
        mailServer.starttls()
        mailServer.ehlo()
        mailServer.login(user, password)
        mailServer.sendmail(user, recipient, msg.as_string())
        mailServer.close()
    print('Sent email to %s' % recipient)

def gd_connect():
    load_config()
    global user, doc_name, password, gd_client
    # Connect to Google
    gd_client = gdata.spreadsheet.service.SpreadsheetsService()
    gd_client.email = user
    gd_client.password = password
    gd_client.source = 'payne.org-example-1'
    gd_client.ProgrammaticLogin()

def get_registrations():
    global gd_client
    if gd_client is None:
        gd_connect()
    q = gdata.spreadsheet.service.DocumentQuery()
    q['title'] = doc_name
    q['title-exact'] = 'true'
    feed = gd_client.GetSpreadsheetsFeed(query=q)
    spreadsheet_id = feed.entry[0].id.text.rsplit('/',1)[1]
    feed = gd_client.GetWorksheetsFeed(spreadsheet_id)
    worksheet_id = feed.entry[0].id.text.rsplit('/',1)[1]
    rows = gd_client.GetListFeed(spreadsheet_id, worksheet_id).entry
    return rows

def update_row(row, dict):
    global gd_client
    entry = gd_client.UpdateRow( row, dict)
    if isinstance(entry, gdata.spreadsheet.SpreadsheetsList):
        return True
    else:
        return False