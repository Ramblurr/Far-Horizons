#!/usr/bin/python2
# Utility to read player registrations from a google spreadsheet
# Authentication credentials goes in 'gmail.cfg'

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
try:
	config = ConfigParser.ConfigParser()
	config.read('gmail.cfg')

	user = config.get("main", "user")
	doc_name = config.get("main", "spreadsheet")
	password = config.get("main", "password")
except ConfigParser.Error:
	print "Error parsing gmail.cfg file"
	sys.exit(1)

email_template_fail = """Hi there,
Your player registration for Far Horizons has been received, however some problems were detected.

The following errors were detected in your registration:
%s
Feel free to resubmit your registration. If you have any questions just reply to this email.

Best,
Casey
Gamemaster
"""

email_template_success = """Hi there,

Your player registration for Far Horizons has been received and approved. 

Species Name:      %s
Home Planet Name:  %s  
Government Name:   %s
Government Type:   %s

While waiting for the game to begin feel free to browse over the game manual again and familiarize yourself with the various orders. Also, check out the "Hints for Beginner's" section on the website [1].

I will notify you when the game begins. If you have any questions just reply to this email.

Best,
Casey
Game Master

[1] http://fh.binaryelysium.com/index.php?title=Beginner's_Guide
"""

def sendMail(subject, recipient, text, *attachmentFilePaths):
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

def check_length(string):
	if  7 <= len(string)  <= 31:
		return True
	return False

def main():
	# Connect to Google
	gd_client = gdata.spreadsheet.service.SpreadsheetsService()
	gd_client.email = user
	gd_client.password = password
	gd_client.source = 'payne.org-example-1'
	gd_client.ProgrammaticLogin()

	q = gdata.spreadsheet.service.DocumentQuery()
	q['title'] = doc_name
	q['title-exact'] = 'true'
	feed = gd_client.GetSpreadsheetsFeed(query=q)
	spreadsheet_id = feed.entry[0].id.text.rsplit('/',1)[1]
	feed = gd_client.GetWorksheetsFeed(spreadsheet_id)
	worksheet_id = feed.entry[0].id.text.rsplit('/',1)[1]

	rows = gd_client.GetListFeed(spreadsheet_id, worksheet_id).entry

	messages = {}
	for idx, row in enumerate(rows):
		if row.custom["validated"].text == "Yes":
			continue
		recipient = row.custom["email"].text
		if len(recipient.strip()) == 0:
			continue
		print "Validating %s - %s" % (row.custom["speciesname"].text, row.custom["email"].text)
		error_msg = ""
		
		if not check_length(row.custom["speciesname"].text):
			error_msg += "* Species name must be between 7 and 31 character.\n"
		if not check_length(row.custom["governmentname"].text):
			error_msg += "* Government name must be between 7 and 31 character.\n"
		if not check_length(row.custom["homeplanetname"].text):
			error_msg += "* Homeplanet name must be between 7 and 31 character.\n"
		if not check_length(row.custom["governmenttype"].text):
			error_msg += "* Government type must be between 7 and 31 character.\n"
		bio = 0
		grav = 0
		mil = 0
		life = 0
		converted = True
		try:
			bio = int(row.custom["biology"].text)
		except ValueError:
			error_msg += "* Biology tech value must be a number\n"
			converted = False
		try:
			grav = int(row.custom["gravitics"].text)
		except ValueError:
			error_msg += "* Gravitics tech value must be a number\n"
			converted = False
		try:
			mil = int(row.custom["military"].text)
		except ValueError:
			error_msg += "* Military tech value must be a number\n"
			converted = False
		try:
			life = int(row.custom["lifesupport"].text)
		except ValueError:
			error_msg += "* Life Support tech value must be a number\n"
			converted = False
			
		if converted:
			total = bio + grav + mil + life
			if total != 15:
				error_msg += "* The total number of points allocated to technologies must be equal to 15\n"
		
		if len(error_msg) > 0:
			messages[recipient] = email_template_fail % (error_msg)
		else:
			messages[recipient] = email_template_success % (row.custom["speciesname"].text, row.custom["homeplanetname"].text, row.custom["governmentname"].text, row.custom["governmenttype"].text)
			d = { "validated" : "Yes" }
			entry = gd_client.UpdateRow( row, d)
			
	#for email,msg in messages.iteritems():
		#sendMail("FH Player Registration", email, msg)
	print "All done"


if __name__ == "__main__":
	main()