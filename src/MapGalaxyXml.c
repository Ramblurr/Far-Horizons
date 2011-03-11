
/* This program will output the map of the galaxy to the file "galaxy.xml"*/

#define THIS_IS_MAIN

#include "fh.h"

#include <stdio.h>
#include <string.h>
#include <libxml2/libxml/encoding.h>
#include <libxml2/libxml/xmlwriter.h>


struct galaxy_data  galaxy;

extern int  num_stars;

extern struct star_data *star_base;
#define MY_ENCODING "ISO-8859-1"


main (argc, argv)

int argc;
char *argv[];

{
    int rc, galactic_diameter, star_index;

    struct star_data    *star;
    xmlTextWriterPtr writer;
    char buffer[256];

    /* Check for valid command line. */
    if (argc != 1)
    {
        fprintf (stderr, "\n\tUsage: MapGalaxy\n\n");
        fprintf (stderr, "\tResults will be written to file galaxy.xml\n\n");
        exit (-1);
    }

    writer = xmlNewTextWriterFilename("galaxy.xml", 0);
    if (writer == NULL) {
        printf("Error creating the xml writer\n");
        return;
    }

    /* Get all the raw data. */
    get_galaxy_data ();
    get_star_data ();

    galactic_diameter = 2 * galaxy.radius;
    
    rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
    rc = xmlTextWriterStartElement(writer, BAD_CAST "galaxy");
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "version", BAD_CAST "1.0");
    snprintf(buffer, 5, "%d", galaxy.radius);
    rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "radius", BAD_CAST buffer);
    rc = xmlTextWriterStartElement(writer, BAD_CAST "systems");
   
    star = star_base;
    for (star_index = 0; star_index < num_stars; star_index++)
    {
        rc = xmlTextWriterStartElement(writer, BAD_CAST "system");
        /* write pos */
        rc = xmlTextWriterStartElement(writer, BAD_CAST "pos");
        snprintf(buffer, 5, "%d", star->x);
        rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "x", BAD_CAST buffer);
        snprintf(buffer, 5, "%d", star->y);
        rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "y", BAD_CAST buffer);
        snprintf(buffer, 5, "%d", star->z);
        rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "z", BAD_CAST buffer);
        rc = xmlTextWriterEndElement(writer);
        
        /* write color */
        rc = xmlTextWriterStartElement(writer, BAD_CAST "color");
        rc = xmlTextWriterWriteFormatString(writer, "%c", color_char[star->color]);
        rc = xmlTextWriterEndElement(writer);
        /* write type */
        rc = xmlTextWriterStartElement(writer, BAD_CAST "type");
        rc = xmlTextWriterWriteFormatString(writer, "%c", type_char[star->type]);
        rc = xmlTextWriterEndElement(writer);
        /* write size*/
        rc = xmlTextWriterStartElement(writer, BAD_CAST "size");
        rc = xmlTextWriterWriteFormatString(writer, "%c", size_char[star->size]);
        rc = xmlTextWriterEndElement(writer);
        
        /* close the system element */
        rc = xmlTextWriterEndElement(writer);
        ++star;
    }


    /* Clean up and exit. */
    rc = xmlTextWriterEndDocument(writer);
    xmlFreeTextWriter(writer);
//     fclose (outfile);
    exit (0);
}
