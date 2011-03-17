#!/usr/bin/python2

import sys, subprocess
from imapclient import IMAPClient
import email, email.utils, email.parser
import fhutils

server = "imap.gmail.com"
port = 993
ssl = True

def main():
    global server, port,ssl
    config = fhutils.GameConfig()
    game = fhutils.Game()
    server = IMAPClient(server, use_uid=True, ssl=ssl)
    server.login(config.user, config.password)
    select_info = server.select_folder('INBOX')
    messages = server.search(['SUBJECT "FH Orders"'])
    response = server.fetch(messages, ['RFC822'])
    for k in response:
        mail = email.message_from_string(response[k]['RFC822'])
        addressor = mail.get("From")
        from_address = email.utils.parseaddr(addressor)[1]
        if 'wait' in mail.get("Subject"):
            wait = True
        else:
            wait = False
        for player in game.players:
            if from_address == player['email']: 
                fd = open("sp%s.ord" %(player['num']), 'w')
                if mail.is_multipart():
                    print "MULTIPART MESSAGE DETECTED!"
                    sys.exit(1)
                orders = mail.get_payload()
                p = subprocess.Popen(["/usr/bin/perl", "/home/ramblurr/src/fh/engine/bash/orders.pl"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
                verify = p.communicate(input=orders)[0]
                subject = "FH Orders recevied"
                if wait:
                    subject += " - wait set"
                else:
                    subject += " - wait not set"
                config.send_mail(subject, from_address, verify)
                
    #print response
    
if __name__ == "__main__":
    main()
