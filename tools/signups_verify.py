#!/usr/bin/env python
# Utility to verify player registrations from a google spreadsheet
# Authentication credentials goes in 'gmail.cfg'
#
# Run this script every time a user registers. It verifies the new registrations
# then sends them a confirmation email. The valid registrations are marked as valid
# in the spreadsheet, so you can safely run it multiple times without spamming
# already validated players.
#
# Also, you probably want to edit the email template messages below

import fhutils



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

While waiting for the game to begin feel free to browse over the game manual again and familiarize yourself with the various orders. If you're new to Far Horizons check out the "Getting Started" section on the website [1].

I will notify you when the game begins. If you have any questions just reply to this email.

Best,
Casey
Game Master

[1] http://fh.binaryelysium.com/Getting_Started
"""



def check_length(string):
    if  7 <= len(string)  <= 31:
        return True
    return False

def main():
    config = fhutils.GameConfig()
    spreadsheet = config.registrations()
    rows = spreadsheet.get_registrations()
    messages = {}
    for idx, row in enumerate(rows):
        if row.custom["validated"].text == "Yes":
            continue
        recipient = row.custom["email"].text
        if recipient is None:
            continue
        if len(recipient.strip()) == 0:
            continue
        print("Validating %s - %s" % (row.custom["speciesname"].text, row.custom["email"].text))
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
        except (TypeError,ValueError):
            error_msg += "* Biology tech value must be a number\n"
            converted = False
        try:
            grav = int(row.custom["gravitics"].text)
        except (TypeError,ValueError):
            error_msg += "* Gravitics tech value must be a number\n"
            converted = False
        try:
            mil = int(row.custom["military"].text)
        except (TypeError,ValueError):
            error_msg += "* Military tech value must be a number\n"
            converted = False
        try:
            life = int(row.custom["lifesupport"].text)
        except (TypeError,ValueError):
            error_msg += "* Life Support tech value must be a number\n"
            converted = False
            
        if converted:
            total = bio + grav + mil + life
            if total != 15:
                error_msg += "* The total number of points allocated to technologies must be equal to 15\n"
        
        if len(error_msg) > 0:
            print("\tErrors found")
            messages[recipient] = email_template_fail % (error_msg)
        else:
            messages[recipient] = email_template_success % (row.custom["speciesname"].text, row.custom["homeplanetname"].text, row.custom["governmentname"].text, row.custom["governmenttype"].text)
            d = {}
            for k in row.custom:
                d[k] = row.custom[k].text
            d["validated"] = "Yes"
            spreadsheet.update_row(row, d)
        
    for email,msg in messages.items():
        config.send_mail("FH Player Registration", email, msg)
    print("All done")


if __name__ == "__main__":
    main()
