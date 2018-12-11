#!/usr/bin/env python
"""
 Utility to read player registrations from a google spreadsheet
 and save them in players.csv

 After signups are closed and all the registrations are verified (see
 signups_verify.py) run this script to download and save the registrations
 to a CSV file in the format:
     email, species, home_planet, gov_name, gov_type, mil, grav, life, bio

 Additionally this script creates players_email.csv which is a simple csv in the format:
    species name, email
This CSV can be imported to GMail as a group, useful for sending mass messages
to the players.
"""

import fhutils

def main():
    config = fhutils.GameConfig()
    spreadsheet = config.registrations()
    rows = spreadsheet.get_registrations()
    messages = {}
    playerlist = []
    emaillist = []
    emaillist.append( "Name, Email Address\n" )
    for idx, row in enumerate(rows):
         # email,sp_name,,home_planet_name,gov_name,gov_type,ML,GV,LS,BI
        email = row.custom["email"].text
        if email is None:
            continue
        if row.custom["validated"].text != "Yes":
            print("%s not validated" % (email))
            continue
        species = row.custom["speciesname"].text
        gov_name = row.custom["governmentname"].text
        home_planet = row.custom["homeplanetname"].text
        gov_type = row.custom["governmenttype"].text
        bio = row.custom["biology"].text
        grav = row.custom["gravitics"].text
        mil = row.custom["military"].text
        life = row.custom["lifesupport"].text

        playercsv_row = "%s,%s,%s,%s,%s,%s,%s,%s,%s\n" % (email, species, home_planet, gov_name, gov_type, mil, grav, life, bio)
        playerlist.append(playercsv_row)
        
        emailcsv_row = "%s,%s\n" % (species, email)
        emaillist.append(emailcsv_row)

    player_csv = open('players.csv',"w")
    player_csv.writelines(playerlist)
    player_csv.close
    
    email_csv = open('players_email.csv',"w")
    email_csv.writelines(emaillist)
    email_csv.close

if __name__ == "__main__":
    main()
