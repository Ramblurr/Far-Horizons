#!/usr/bin/python2
# Utility to read player registrations from a google spreadsheet
# and save them in players.csv
#
# After signups are closed and all the registrations are verified (see
# signups_verify.py) run this script to download and save the registrations
# to a CSV file in the format:
# email, species, home_planet, gov_name, gov_type, mil, grav, life, bio

import fhutils

def main():
    rows = fhutils.get_registrations()
    messages = {}
    playerlist = []
    for idx, row in enumerate(rows):
         # email,sp_name,,home_planet_name,gov_name,gov_type,ML,GV,LS,BI
        email = row.custom["email"].text
        if row.custom["validated"].text != "Yes":
            print "%s not validated" % (email)
            continue
        species = row.custom["speciesname"].text
        gov_name = row.custom["governmentname"].text
        home_planet = row.custom["homeplanetname"].text
        gov_type = row.custom["governmenttype"].text
        bio = row.custom["biology"].text
        grav = row.custom["gravitics"].text
        mil = row.custom["military"].text
        life = row.custom["lifesupport"].text

        csv_row = "%s,%s,%s,%s,%s,%s,%s,%s,%s\n" % (email, species, home_planet, gov_name, gov_type, mil, grav, life, bio)
        playerlist.append(csv_row)

    csv = open('players.csv',"w")
    csv.writelines(playerlist)
    csv.close

if __name__ == "__main__":
    main()
