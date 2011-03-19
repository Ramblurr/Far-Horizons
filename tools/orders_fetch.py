#!/usr/bin/env python

import sys, subprocess, os
from imapclient import IMAPClient
import email, email.utils, email.parser
import fhutils

server = "imap.gmail.com"
port = 993
ssl = True

def main():
    global server, port,ssl
    config = fhutils.GameConfig()
    data_dir = config.gameslist[0]['datadir']  #only support one game now
    game_stub = config.gameslist[0]['stub']
    try:
       game = fhutils.Game()
    except IOError:
        print "Could not read fh_names"
        sys.exit(2)
    
    if not os.path.isdir(data_dir):
        print "Sorry data directory %s does not exist." % (data_dir)
        sys.exit(2)
    
    server = IMAPClient(server, use_uid=True, ssl=ssl)
    server.login(config.user, config.password)
    select_info = server.select_folder('INBOX')
    messages = server.search(['UNSEEN', 'SUBJECT "FH Orders"'])
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
                orders_file = "%s/sp%s.ord" %(data_dir, player['num'])
                fd = open(orders_file, 'w')
                if mail.is_multipart():
                    print "Multipart Message detected, searching for plain text payload!"
                    payloads = mail.get_payload()
                    try:
                        found = False
                        for loads in payloads:
                            if loads.get_content_type()  == "text/plain":
                                mail = loads
                                found = True
                                break
                        if not found:
                            raise email.errors.MessageError
                    except email.errors.MessageError:
                        print "Could not find text/plain payload for " + from_address
                        sys.exit(1)
                orders = mail.get_payload()
                fd.write(orders)
                fd.close()
                p = subprocess.Popen(["/usr/bin/perl", "/home/ramblurr/src/fh/engine/bash/orders.pl"], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
                verify = p.communicate(input=orders)[0]
                subject = "FH Orders, %s received" % (game_stub)
                if wait:
                    subject += " - wait set"
                else:
                    subject += " - wait not set"
                config.send_mail(subject, from_address, verify, orders_file)
                print "Retrieved orders %s for sp%s - %s - %s" %("[WAIT]" if wait else "", player['num'], player['name'], from_address)
                
if __name__ == "__main__":
    main()
