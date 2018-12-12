#!/usr/bin/env python
"""

    Creates PDF maps of the galaxy.

"""
import fhutils
import os, sys, tempfile, subprocess, shutil
import getopt

def pretty_star(stellar_code):
    type = stellar_code[0] if len(stellar_code) == 3 else None
    color = stellar_code[1] if len(stellar_code) == 3 else stellar_code[0]
    size = stellar_code[2] if len(stellar_code) == 3 else stellar_code[1]

    if color is "O":
        color_p = "Blue"
    elif color is "B":
        color_p = "Blue-white"
    elif color is "A":
        color_p = "White"
    elif color is "F":
        color_p = "Yellow-white"
    elif color is "G":
        color_p = "Yellow"
    elif color is "K":
        color_p = "Orange"
    elif color is "M":
        color_p = "Red"
    else:
        color_p = ""

    if type is "d":
        type_p = "dwarf"
    elif type is "g":
        type_p = "giant"
    elif type is "D":
        type_p = "degenerate dwarf"
    else:
        type_p = "main sequence"

    return "%s %s" % (color_p, type_p)


def main(argv):
    config_file = None
    discard = False
    try:
        opts, args = getopt.getopt(argv, "hc:", ["help", "config="])
    except getopt.GetoptError:
        print(__doc__)
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            print(__doc__)
            sys.exit(0)
        elif opt in ("-c", "--config"):
            config_file = arg

    if config_file:
        config = fhutils.GameConfig(config_file)
    else:
        config = fhutils.GameConfig()
    game = config.gameslist[0] # for now we only support a single game
    game_name = game['name']
    data_dir = game['datadir']
    bin_dir = config.bindir
    PS2PDF = shutil.which("ps2pdf")
    PDFTK = shutil.which("pdftk")
    PDFUNITE = shutil.which("pdfunite")

    os.chdir(data_dir)

    output = fhutils.run(bin_dir, "ListGalaxy", ["-p"])
    lines = []
    for row in output.splitlines():
        cols = row.split()
        try:
            if cols[0] is "The":
                break
            x = cols[2]
            y = cols[5]
            z = cols[8]
            stellar_type = cols[12]
            name = pretty_star(stellar_type)
            lines.append("%s, %s, %s, %s, %s.\n" % (x, y, z, name, stellar_type))
        except IndexError:
            continue

    fd = tempfile.NamedTemporaryFile(mode='w', delete=False)
    fd.writelines(lines)
    fd.flush()
    os.fsync(fd)
    fhutils.run(bin_dir, "PrintMap", ["-d", "-t", "%s"%(fd.name)])
    subprocess.call(["%s" % (PS2PDF), "-dAutoRotatePages=/None",fd.name+".ps", data_dir+"/galaxy_map_3d.pdf"])
    fhutils.run(bin_dir, "PrintMap", ["-d", "%s"%(fd.name)])
    subprocess.call(["%s" % (PS2PDF), "-dAutoRotatePages=/None",fd.name+".ps", data_dir+"/galaxy_map.pdf"])
    fd.close()

    if PDFTK is not None:
        subprocess.call(["%s" % (PDFTK), data_dir+"/galaxy_map_3d.pdf", data_dir+"/galaxy_map.pdf", "cat", "output", data_dir+"/galaxy.map.pdf"])
    elif PDFUNITE is not None:
        subprocess.call(["%s" % (PDFUNITE), data_dir+"/galaxy_map_3d.pdf", data_dir+"/galaxy_map.pdf", data_dir+"/galaxy.map.pdf"])
    else:
        print("""Error: could not find pdftk nor pdfunite to merge the galaxy map
       pdfs. You should manually merge galaxy_map_3d.pdf and
       galaxy_map_2d.pdf into a single galaxy.map.pdf file before running
       game_packet.py.""")


if __name__ == "__main__":
    main(sys.argv[1:])
