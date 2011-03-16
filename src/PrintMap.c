/*
  Star Mapping Program:  Version 2.3   26 September, 1993 
  This source code and the accompanying documentation are
  copyright 1991 by David Mar. Permission is granted to
  distribute them freely so long as no modifications are made.
  Any questions, suggestions or bug reports may be forwarded to
    mar@physics.su.oz.au

  Version history:
  2.0    9 December, 1991  first version with 3D maps option
  2.1   25 November, 1992  <sys/types> added to fix some compiler problems
  2.2   ??  added new planetary orbits option, partially complete
  2.3   26 September, 1993  fixed bug with -n option in 3D map
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

/*  uncomment the following line (ONLY) if using US letter paper  */
 #define USPAPER

#ifndef USPAPER
   /*  next two lines are for A4 paper  */
#define XOFFSET  298
#define YOFFSET  421
#else
   /*  next two lines are for US letter paper  */
#define XOFFSET  306
#define YOFFSET  396
#endif

#define TRUE      -1
#define FALSE      0
#define LINELEN   80
#define NAMELEN   40
#define TYPELEN   10
#define MAXSTARS 900
#define NOTFOUND  -1
#define LEFT       0
#define RIGHT      1
#define COS5       0.996194698092
#define SIN5       0.087155742748
#define EYE       10
#define COLWID   250
#define TPSA     369.4
#define TPSB     130.6
#define PSMAX    250

#define OUT_OF_BOUNDS (psx<-130 || psx>370 || psy<-250 || psy>250)
#define XLABEL   (flag->x?'y':flag->y?'z':'x')
#define YLABEL   (flag->x?'z':flag->y?'x':'y')
#define ZLABEL   (flag->x?'x':flag->y?'y':'z')
#define HCOORD   (flag->x?current->z:flag->y?current->x:current->y)
#define HMAX     (flag->x?lim->plotzmax:flag->y?lim->plotxmax:lim->plotymax)
#define HMIN     (flag->x?lim->plotzmin:flag->y?lim->plotxmin:lim->plotymin)
#define VCOORD   (flag->x?current->y:flag->y?current->z:current->x)
#define VMAX     (flag->x?lim->plotymax:flag->y?lim->plotzmax:lim->plotxmax)
#define VMIN     (flag->x?lim->plotymin:flag->y?lim->plotzmin:lim->plotxmin)

#define SQR(a)   ((a)*(a))
#define OUT(a)   fprintf(outfile, "%s\n", a)
#define MAX(a,b) ((a)>(b)?(a):(b))
#define NEWSTAR  (STARINFO*) malloc(sizeof(STARINFO))
#define NEWPLAN  (PLANINFO*) malloc(sizeof(PLANINFO))

typedef struct pinfo {
   float      orbit;         /* million km  */
   float      eccentricity;
   float      inclination;   /* degrees     */
   float      diameter;      /* thousand km */
   float      axial;         /* degrees     */
   float      rotation;      /* days        */
   char       name[NAMELEN];
   struct pinfo   *next;
   struct pinfo   *moon;
} PLANINFO;

typedef struct sinfo {
   float      x;
   float      y;
   float      z;
   char       name[NAMELEN];
   char       type[TYPELEN];
   float      mass;
   struct sinfo   *next;
   struct pinfo   *planet;
} STARINFO;

typedef struct {
   int   x, y, z;
   int   p, t, r;
   int   s, n, c;
   int   d, o;
   int   l, L, h;
   int   g;
} FLAGINFO;

typedef struct {
   float   xmin, xmax;
   float   ymin, ymax;
   float   zmin, zmax;
   int     plotxmin, plotxmax;
   int     plotymin, plotymax;
   int     plotzmin, plotzmax;
   int     mapwidth;
   int     planeheight;
} LIMINFO;

void
header(outfile)
FILE   *outfile;
{
   OUT("%!");
   OUT("% Postscript output from Star Mapping Program");
   OUT("% Copyright 1991 David Mar == mar@astrop.physics.su.OZ.AU");
   OUT("/roman {/Times-Roman findfont exch scalefont setfont} bind def");
   OUT("/greek {/Symbol findfont exch scalefont setfont} bind def");
   OUT("/bold {/Times-Bold findfont exch scalefont setfont} bind def");
   OUT("/centre {dup stringwidth pop 2 div neg 0 rmoveto} bind def");
   OUT("/right {dup stringwidth pop neg 0 rmoveto} bind def");
   OUT("/drawlino { 1 add /ury exch def 1 add /urx exch def");
   OUT("1 sub /lly exch def 1 sub /llx exch def");
   OUT("llx lly moveto urx lly lineto urx ury lineto llx ury lineto");
   OUT("closepath fill } bind def");
   OUT("/blob { 0 360 arc fill } bind def");
   OUT("/str 8 string def");
   OUT("9 roman");
   fprintf(outfile, "%d %d translate\n", XOFFSET, YOFFSET);
   OUT("90 rotate");
}

void
trailer(outfile)
FILE   *outfile;
{
   OUT("showpage");
   OUT("% End of postscript output from Star Mapping Program");
}

void
emitps(outfile, current, side)
FILE       *outfile;
STARINFO   *current;
int        side;
{
   float   psx, psy, psr;
   float   perspy, perspz;

   perspy = current->y * EYE / (EYE - current->x);
   perspz = current->z * EYE / (EYE - current->x);
   psx = -175 + (side * 350) + (perspy * 150);
   psy = (perspz * 150) - 75;
   psr = 2 + current->x;
   fprintf(outfile, "newpath %f %f %f blob\n", psx, psy, psr);
}

void
rotate(current)
STARINFO   *current;
{
   float   temp;

   temp = COS5 * current->x - SIN5 * current->y;
   current->y = SIN5 * current->x + COS5 * current->y;
   current->x = temp;
}

void
dostar(current, outfile)
STARINFO   *current;
FILE       *outfile;
{
   emitps(outfile, current, LEFT);
   rotate(current);
   emitps(outfile, current, RIGHT);
}

void
encode(head, outfile)
STARINFO  *head;
FILE      *outfile;
{
   STARINFO   *current = head;

   while (current) {
      dostar(current, outfile);
      current = current->next;
   }
}

void
emittext(outfile, text, size, psx, psy, lino)
FILE   *outfile;
char   *text;
int    size;
float  psx, psy;
int    lino;
{
   char   *ptext = text;

   fprintf(outfile, "%f %f moveto %d roman\n", psx, psy, size);
   if (lino) {
      OUT("gsave 1 setgray");
      fputc('(', outfile);
      do {
         while (*ptext && *ptext != '{')
            fputc(*ptext++, outfile);
         if (*ptext == '{') {
            fprintf(outfile, ") true charpath\n%d greek (", size);
            while (*++ptext && *ptext != '}')
               fputc(*ptext, outfile);
            fprintf(outfile, ") true charpath\n%d roman (", size);
         }
      } while (*ptext++);
      OUT(") true charpath pathbbox");
      OUT("drawlino grestore");
   }
   fputc('(', outfile);
   do {
      while (*text && *text != '{')
         fputc(*text++, outfile);
      if (*text == '{') {
         fprintf(outfile, ") show\n%d greek (", size);
         while (*++text && *text != '}')
            fputc(*text, outfile);
         fprintf(outfile, ") show\n%d roman (", size);
      }
   } while (*text++);
   OUT(") show");
}

void
calcgrid(lim, flag, gridsize)
LIMINFO    *lim;
FLAGINFO   *flag;
int        *gridsize;
{
   float   mapsize;
   int     reduction;
   int     extrax;
   int     tmin, tmax;
   int     xsize, ysize, zsize;

   if (flag->x) {
      tmin = lim->xmin;
      tmax = lim->xmax;
      lim->xmin = lim->ymin;
      lim->xmax = lim->ymax;
      lim->ymin = lim->zmin;
      lim->ymax = lim->zmax;
      lim->zmin = tmin;
      lim->zmax = tmax;
   }
   else if (flag->y) {
      tmin = lim->ymin;
      tmax = lim->ymax;
      lim->ymin = lim->xmin;
      lim->ymax = lim->xmax;
      lim->xmin = lim->zmin;
      lim->xmax = lim->zmax;
      lim->zmin = tmin;
      lim->zmax = tmax;
   }
   if (flag->l || flag->L)
      mapsize = lim->mapwidth;
   else if (flag->L)
      mapsize = MAX(lim->xmax - lim->xmin,
                 MAX(lim->ymax - lim->ymin, lim->zmax - lim->zmin));
   else
      mapsize = MAX(lim->xmax - lim->xmin, lim->ymax - lim->ymin);
   mapsize /= 20;
   reduction = 1;
   while (mapsize >= 10) {
      mapsize /= 10;
      reduction *= 10;
   }
   *gridsize = mapsize<=1?1:mapsize<=2?2:mapsize<=5?5:10;
   *gridsize *= reduction;

   if (flag->l || flag->L) {
      lim->plotxmax = lim->plotxmin + lim->mapwidth;
      lim->plotymax = lim->plotymin + lim->mapwidth;
      lim->plotzmax = lim->plotzmin + lim->mapwidth;
   }
   else {
      lim->plotxmin = *gridsize * (floor(lim->xmin / *gridsize));
      lim->plotxmax = *gridsize * (ceil(lim->xmax / *gridsize));
      lim->plotymin = *gridsize * (floor(lim->ymin / *gridsize));
      lim->plotymax = *gridsize * (ceil(lim->ymax / *gridsize));
      lim->plotzmin = *gridsize * (floor(lim->zmin / *gridsize));
      lim->plotzmax = *gridsize * (ceil(lim->zmax / *gridsize));
      if (!flag->t) {
         extrax = (lim->plotxmax - lim->plotxmin) -
                  (lim->plotymax - lim->plotymin);
         if (extrax > 0) {
            if ((extrax / *gridsize) % 2) {
               extrax -= *gridsize;
               lim->plotymax += *gridsize;
            }
            lim->plotymin -= (extrax / 2);
            lim->plotymax += (extrax / 2);
         }
         else {
            if ((extrax / *gridsize) % 2) {
               extrax += *gridsize;
               lim->plotxmax += *gridsize;
            }
            lim->plotxmin += (extrax / 2);
            lim->plotxmax -= (extrax / 2);
         }
         lim->mapwidth = (lim->plotxmax - lim->plotxmin);
      }
      else {
         xsize = lim->plotxmax - lim->plotxmin;
         ysize = lim->plotymax - lim->plotymin;
         zsize = lim->plotzmax - lim->plotzmin;
         if (xsize == MAX(xsize, MAX(ysize, zsize))) {
            if ((xsize - ysize) % 2) {
               ysize += *gridsize;
               lim->plotymax += *gridsize;
            }
            lim->plotymin -= (xsize - ysize) / 2;
            lim->plotymax += (xsize - ysize) / 2;
            if ((xsize - zsize) % 2) {
               zsize += *gridsize;
               lim->plotzmax += *gridsize;
            }
            lim->plotzmin -= (xsize - zsize) / 2;
            lim->plotzmax += (xsize - zsize) / 2;
            lim->mapwidth = xsize;
         }
         else if (ysize == MAX(xsize, MAX(ysize, zsize))) {
            if ((ysize - xsize) % 2) {
               xsize += *gridsize;
               lim->plotxmax += *gridsize;
            }
            lim->plotxmin -= (ysize - xsize) / 2;
            lim->plotxmax += (ysize - xsize) / 2;
            if ((ysize - zsize) % 2) {
               zsize += *gridsize;
               lim->plotzmax += *gridsize;
            }
            lim->plotzmin -= (ysize - zsize) / 2;
            lim->plotzmax += (ysize - zsize) / 2;
            lim->mapwidth = ysize;
         }
         else {
            if ((zsize - xsize) % 2) {
               xsize += *gridsize;
               lim->plotxmax += *gridsize;
            }
            lim->plotxmin -= (zsize - xsize) / 2;
            lim->plotxmax += (zsize - xsize) / 2;
            if ((zsize - ysize) % 2) {
               ysize += *gridsize;
               lim->plotymax += *gridsize;
            }
            lim->plotymin -= (zsize - ysize) / 2;
            lim->plotymax += (zsize - ysize) / 2;
            lim->mapwidth = zsize;
         }
      }
   }

   if (!flag->h)
      if (lim->plotzmin <= 0 && lim->plotzmax >= 0)
         lim->planeheight = 0;
      else
         lim->planeheight = lim->plotzmin;
}

void
drawkey(outfile)
FILE    *outfile;
{
   static  char  spectype[] = "OBAFGKM";
   int     i, keyy;
   float   psr;

   OUT("-345 220 moveto 120 0 rlineto 0 -190 rlineto -120 0 rlineto");
   OUT("closepath fill");
   OUT("gsave 1 setgray -350 225 moveto 120 0 rlineto 0 -190 rlineto");
   OUT("-120 0 rlineto closepath fill grestore");
   OUT("-350 225 moveto 120 0 rlineto 0 -190 rlineto");
   OUT("-120 0 rlineto closepath stroke");
   OUT("-325 200 moveto 9 bold (Spectral Type Key) show 9 roman");
   OUT("/hh (O) true charpath flattenpath pathbbox");
   OUT("exch pop sub neg exch pop 2 div def");
   for (i=0, keyy=180, psr=3.5; i<7; keyy-=20, psr-=0.5, i++) {
      fprintf(outfile, "newpath -310 %d hh add %f blob\n", keyy, psr);
      fprintf(outfile, "-290 %d moveto (%c) show\n", keyy, spectype[i]);
   }
}

void
drawgrid3D(outfile, lim, flag, gridsize)
FILE       *outfile;
LIMINFO    *lim;
FLAGINFO   *flag;
int        *gridsize;
{
   int     xbegin, ybegin;
   float   psxbegin, psybegin;
   float   numgrids;
   float   psgridsize;
   float   psplaneheight;

   xbegin = *gridsize * ceil( (float)lim->plotxmin / (float)*gridsize);
   ybegin = *gridsize * ceil( (float)lim->plotymin / (float)*gridsize);
   numgrids = (float)lim->mapwidth / (float)*gridsize;
   psgridsize = TPSA / numgrids;
   psplaneheight = (lim->planeheight - lim->plotzmin) * TPSA / lim->mapwidth
                    - 250;
   psxbegin = (xbegin - lim->plotxmin) * psgridsize / *gridsize - 130;
   psybegin = (ybegin - lim->plotymin) * (TPSB / numgrids) / *gridsize
               + psplaneheight;

   fprintf(outfile, "/grid %d def\n", *gridsize);
   fprintf(outfile, "/xlab %d def\n", xbegin);
   fprintf(outfile, "/ylab %d def\n", ybegin);
   OUT("5 roman");
   fprintf(outfile, "%f %f %f { /x exch def\n", psxbegin, psgridsize,
           (TPSA - 130));
   fprintf(outfile, "x %f moveto\n", psplaneheight);
   if (!flag->g) {
      OUT("gsave [1 2] 0 setdash 0.001 setlinewidth");
      fprintf(outfile, "%f %f rlineto stroke grestore\n", TPSB, TPSB);
   }
   else {
      OUT("gsave 0.001 setlinewidth 3 3 rlineto stroke");
      fprintf(outfile, "x %f add %f moveto -3 -3 rlineto stroke grestore\n",
              TPSB, (psplaneheight + TPSB));
   }
   fprintf(outfile, "x %f moveto xlab str cvs centre show\n",
           (psplaneheight - 6));
   OUT("/xlab xlab grid add def } for");
   psgridsize = TPSB / numgrids;          /*  Trust me on this one  */
   fprintf(outfile, "%f %f %f { /y exch def\n", psybegin, psgridsize,
           (psplaneheight + TPSB));
   fprintf(outfile, "-130 %f sub y add y moveto\n", psplaneheight);
   if (!flag->g) {
      OUT("gsave [1 2] 0 setdash 0.001 setlinewidth");
      fprintf(outfile, "%f 0 rlineto stroke grestore\n", TPSA);
   }
   else {
      OUT("gsave 0.001 setlinewidth 5 0 rlineto stroke");
      fprintf(outfile, "-130 %f sub y add y moveto\n", psplaneheight+TPSA);
      OUT("-5 0 rlineto stroke grestore");
   }
   fprintf(outfile, "-135 %f sub y add y moveto ylab str cvs right show\n",
           psplaneheight);
   OUT("/ylab ylab grid add def } for");
   fprintf(outfile, "gsave 0.48 setlinewidth -130 %f moveto\n", psplaneheight);
   fprintf(outfile, "%f 0 rlineto %f dup rlineto\n", TPSA, TPSB);
   fprintf(outfile, "%f 0 rlineto closepath stroke grestore\n", -TPSA);

   fprintf(outfile, "9 bold %f %f moveto (%c) centre show\n",
           TPSA/2 - 130, psplaneheight - 15, XLABEL);
   fprintf(outfile, "-80 %f moveto (%c) right show\n",
           psplaneheight + TPSB/2, YLABEL);
   OUT("-345 -100 moveto");
   fprintf(outfile, "(Reference plane at %c = %d) show 9 roman\n",
           ZLABEL, lim->planeheight);
}

void
drawgrid(outfile, lim, flag, gridsize)
FILE       *outfile;
LIMINFO    *lim;
FLAGINFO   *flag;
int        *gridsize;
{
   int     xbegin, ybegin;
   float   psxbegin, psybegin;
   float   numgrids;
   float   psgridsize;

   xbegin = *gridsize * ceil( (float)lim->plotxmin / (float)*gridsize);
   ybegin = *gridsize * ceil( (float)lim->plotymin / (float)*gridsize);
   numgrids = (float)lim->mapwidth / (float)*gridsize;
   psgridsize = 500.0 / numgrids;
   psxbegin = (xbegin - lim->plotxmin) * psgridsize / *gridsize - 130;
   psybegin = (ybegin - lim->plotymin) * psgridsize / *gridsize - 250;

   fprintf(outfile, "/grid %d def\n", *gridsize);
   fprintf(outfile, "/xlab %d def\n", xbegin);
   fprintf(outfile, "/ylab %d def\n", ybegin);
   fprintf(outfile, "%f %f 370 { /x exch def\n", psxbegin, psgridsize);
   if (!flag->g) {
      OUT("x -250 moveto gsave [1 2] 0 setdash 0.001 setlinewidth");
      OUT("0 500 rlineto stroke grestore");
   }
   else {
      OUT("x -250 moveto gsave 0.001 setlinewidth 0 5 rlineto stroke");
      OUT("x 250 moveto 0 -5 rlineto stroke grestore");
   }
   OUT("x -260 moveto xlab str cvs centre show");
   OUT("/xlab xlab grid add def } for");
   fprintf(outfile, "%f %f 250 { /y exch def\n", psybegin, psgridsize);
   if (!flag->g) {
      OUT("-130 y moveto gsave [1 2] 0 setdash 0.001 setlinewidth");
      OUT("500 0 rlineto stroke grestore");
   }
   else {
      OUT("-130 y moveto gsave 0.001 setlinewidth 5 0 rlineto stroke");
      OUT("370 y moveto -5 0 rlineto stroke grestore");
   }
   OUT("-135 y moveto ylab str cvs right show");
   OUT("/ylab ylab grid add def } for");
   OUT("gsave 0.48 setlinewidth -130 -250 moveto 0 500 rlineto");
   OUT("500 0 rlineto 0 -500 rlineto closepath stroke grestore");

   fprintf(outfile, "9 bold 120 -270 moveto (%c) centre show\n", XLABEL);
   fprintf(outfile, "-150 0 moveto (%c) right show 9 roman\n", YLABEL);
}

void
getcoords(current, flag, psx, psy, psz, psr)
STARINFO   *current;
FLAGINFO   *flag;
float      *psx, *psy, *psz, *psr;
{
   if (flag->x) {
      *psx = current->y;
      *psy = current->z;
      *psz = current->x;
   }
   else if (flag->y) {
      *psx = current->z;
      *psy = current->x;
      *psz = current->y;
   }
   else {
      *psx = current->x;
      *psy = current->y;
      *psz = current->z;
   }
   switch (toupper(*(current->type))) {
      case 'O' : *psr = 3.5; break;
      case 'B' : *psr = 3.0; break;
      case 'A' : *psr = 2.5; break;
      case 'F' : *psr = 2.0; break;
      case 'G' : *psr = 1.5; break;
      case 'K' : *psr = 1.0; break;
      default  : *psr = 0.5; break;
   }
}

void
datapage(head, outfile)
FILE       *outfile;
STARINFO   *head;
{
   int     i, keyy;
   int     column;
   int     cpos;
   STARINFO   *current = head;

   for (i = column = 0; current; column++) {
      if (i % 102 == 0) {
         OUT("showpage");
         OUT("9 roman");
         fprintf(outfile, "%d %d translate\n", XOFFSET, YOFFSET);
         OUT("90 rotate 350 260 moveto");
         fprintf(outfile, "9 bold (Page ) show %d str cvs show 9 roman\n",
                 (column/3)+1);
      }
      for (keyy=250; keyy>-250 && current; keyy-=15, i++) {
         cpos = (column % 3) * COLWID;
         fprintf(outfile, "%d %d moveto\n", -375+cpos, keyy);
         fprintf(outfile, "((%-4.2f, %-4.2f, %-4.2f) ) show\n",
                 current->x, current->y, current->z);
         emittext(outfile, current->name, 9, (float)(-275+cpos),
                  (float)keyy, FALSE);
         fprintf(outfile, "%d %d moveto\n", -200+cpos, keyy);
         fprintf(outfile, "(%s) show\n", current->type);
         current = current->next;
      }
   }
}

void
drawstars3D(head, outfile, lim, flag)
STARINFO   *head;
FILE       *outfile;
LIMINFO    *lim;
FLAGINFO   *flag;
{
   float   psx, psy, psz, psr;
   float   xoff, yoff, zoff, poff;
   STARINFO   *current = head;

   while (current) {
      getcoords(current, flag, &psx, &psy, &psz, &psr);
      xoff = TPSA * ((psx - lim->plotxmin) / lim->mapwidth);
      yoff = TPSB * ((psy - lim->plotymin) / lim->mapwidth);
      zoff = TPSA * ((psz - lim->plotzmin) / lim->mapwidth);
      poff = TPSA * ((psz - lim->planeheight) / lim->mapwidth);
      psx = -130 + xoff + yoff;
      psy = -250 + zoff + yoff;
      if OUT_OF_BOUNDS {
         current = current->next;
         continue;
      }
      if ((HCOORD > HMAX) || (HCOORD < HMIN)) {
         current = current->next;
         continue;
      }
      if ((VCOORD > VMAX) || (VCOORD < VMIN)) {
         current = current->next;
         continue;
      }
      fprintf(outfile, "newpath %f %f %f blob\n", psx, psy, psr);
      OUT("gsave 0.001 setlinewidth");
      fprintf(outfile, "%f %f moveto 0 %f rlineto stroke\n", psx, psy, -poff);
      if (flag->r)
         fprintf(outfile, "%f %f moveto %f 0 rlineto stroke\n",
                 psx, psy, -xoff);
      if (flag->n)
         emittext(outfile, current->name, 5, psx, (psy+6), TRUE);
      OUT("grestore");
      current = current->next;
   }
}

void
drawstars(head, outfile, lim, flag)
STARINFO   *head;
FILE       *outfile;
LIMINFO    *lim;
FLAGINFO   *flag;
{
   float   psx, psy, psz, psr;
   char    zstr[10];
   STARINFO   *current = head;

   while (current) {
      getcoords(current, flag, &psx, &psy, &psz, &psr);
      psx = -130 + 500 * ((psx - lim->plotxmin) / lim->mapwidth);
      psy = -250 + 500 * ((psy - lim->plotymin) / lim->mapwidth);
      if OUT_OF_BOUNDS {
         current = current->next;
         continue;
      }
      if (flag->n)
         emittext(outfile, current->name, 5, psx, (psy+6), TRUE);
      if (flag->s)
         emittext(outfile, current->type, 5, (psx+7), (psy-1), TRUE);
      if (flag->c) {
         sprintf(zstr, "%-4.2f", psz);
         emittext(outfile, zstr, 5, (psx+7), (psy-8), TRUE);
      }
      fprintf(outfile, "newpath %f %f %f blob\n", psx, psy, psr);
      current = current->next;
   }
}

void
getlims(head, lim)
STARINFO  *head;
LIMINFO   *lim;
{
   STARINFO   *current = head;

   lim->xmin = lim->xmax = current->x;
   lim->ymin = lim->ymax = current->y;
   lim->zmin = lim->zmax = current->z;
   current = current->next;
   while (current) {
      lim->xmin = (lim->xmin<current->x) ? lim->xmin : current->x;
      lim->ymin = (lim->ymin<current->y) ? lim->ymin : current->y;
      lim->zmin = (lim->zmin<current->z) ? lim->zmin : current->z;

      lim->xmax = (lim->xmax>current->x) ? lim->xmax : current->x;
      lim->ymax = (lim->ymax>current->y) ? lim->ymax : current->y;
      lim->zmax = (lim->zmax>current->z) ? lim->zmax : current->z;
      current = current->next;
   }
}

void
normalise(head, lim)
STARINFO   *head;
LIMINFO    *lim;
{
   float   xcent, ycent, zcent;
   float   dist, maxdist;
   STARINFO   *current = head;

   xcent = (lim->xmin + lim->xmax) / 2;
   ycent = (lim->ymin + lim->ymax) / 2;
   zcent = (lim->zmin + lim->zmax) / 2;
   while (current) {
      current->x -= xcent;
      current->y -= ycent;
      current->z -= zcent;
      current = current->next;
   }
   maxdist = 0;
   while (current) {
      dist = sqrt(SQR(current->x) + SQR(current->y) + SQR(current->z));
      maxdist = MAX(maxdist, dist);
      current = current->next;
   }
   while (current) {
      current->x /= maxdist;
      current->y /= maxdist;
      current->z /= maxdist;
      current = current->next;
   }
}

void
doorbits(head, outfile)
STARINFO  *head;
FILE      *outfile;
{
   STARINFO  *current = head;
   PLANINFO  *plan;
   float     maxmin, maxmax;
   float     psconv, pssunoff, pscentre;
   float     xfactor, yfactor;

   header(outfile);
   while (current) {
      if (current->planet) {
         OUT("0 setlinewidth");
         plan = current->planet;
         maxmin = (1 - plan->eccentricity) * plan->orbit;
         maxmax = (1 + plan->eccentricity) * plan->orbit;
         plan = plan->next;
         while (plan) {
            maxmin = MAX(maxmin, ((1 - plan->eccentricity) * plan->orbit));
            maxmax = MAX(maxmax, ((1 + plan->eccentricity) * plan->orbit));
            plan = plan->next;
         }
         psconv = PSMAX / ((maxmax + maxmin) / 2);
         pssunoff = psconv * (maxmax - (maxmin + maxmax) / 2);
         plan = current->planet;
         while (plan) {
            pscentre = pssunoff - psconv * (plan->eccentricity * plan->orbit);
            fprintf(outfile, "gsave %f 0 translate\n", pscentre);
            xfactor = plan->orbit * psconv;
            yfactor = xfactor * sqrt(1 - SQR(plan->eccentricity));
            fprintf(outfile, "%f %f scale\n", xfactor, yfactor);
            OUT("newpath 0 0 1 0 360 arc stroke grestore");
            plan = plan->next;
         }
      }
      current = current->next;
   }
   trailer(outfile);
}

void
doflat(head, outfile, lim, flag)
STARINFO  *head;
FILE      *outfile;
LIMINFO   *lim;
FLAGINFO  *flag;
{
   int    gridsize;

   header(outfile);
   drawkey(outfile);
   getlims(head, lim);
   calcgrid(lim, flag, &gridsize);
   drawgrid(outfile, lim, flag, &gridsize);
   drawstars(head, outfile, lim, flag);
   if (!flag->d)
      datapage(head, outfile);
   trailer(outfile);
}

void
do3D(head, outfile, lim, flag)
STARINFO  *head;
FILE      *outfile;
LIMINFO   *lim;
FLAGINFO  *flag;
{
   int    gridsize;

   header(outfile);
   drawkey(outfile);
   getlims(head, lim);
   calcgrid(lim, flag, &gridsize);
   drawgrid3D(outfile, lim, flag, &gridsize);
   drawstars3D(head, outfile, lim, flag);
   if (!flag->d)
      datapage(head, outfile);
   trailer(outfile);
}

void
dopersp(head, outfile, lim)
STARINFO  *head;
FILE      *outfile;
LIMINFO   *lim;
{
   header(outfile);
   OUT("0 100 moveto");
   OUT("0 -350 rlineto 350 0 rlineto 0 350 rlineto");
   OUT("-700 0 rlineto 0 -350 rlineto 350 0 rlineto");
   OUT("stroke");
   OUT("-350 250 moveto (Perspective plot) show");
   getlims(head, lim);
   normalise(head, lim);
   encode(head, outfile);
   trailer(outfile);
}

void
getargs(argc, argv, filename, lim, flag)
int        argc;
char       *argv[];
char       *filename;
LIMINFO    *lim;
FLAGINFO   *flag;
{
   char    c;
   char    *progname = argv[0];

   flag->x = flag->y = flag->z = flag->p = flag->s = flag->n = flag->d = 0;
   flag->c = flag->l = flag->g = flag->t = flag->r = flag->L = flag->h = 0;
   flag->o = 0;

   while (--argc > 0  && (**++argv) == '-') {
      while (c = *++*argv)
         switch (c) {
            case 'x': flag->x = 1; break;
            case 'y': flag->y = 1; break;
            case 'z': flag->z = 1; break;
            case 'p': flag->p = 1; break;
            case 's': flag->s = 1; break;
            case 'c': flag->c = 1; break;
            case 'n': flag->n = 1; break;
            case 'd': flag->d = 1; break;
            case 'g': flag->g = 1; break;
            case 't': flag->t = 1; break;
            case 'o': flag->o = 1; break;
            case 'r': flag->r = 1;
                      printf("Hang on, I've only got two hours a day to\n");
                      printf("work on this. The r option will be\n");
                      printf("implemented in the next release.\n\n");
                      break;
            case 'h':
               flag->h = 1;
               if (!sscanf((argc--, *++argv), "%d", &(lim->planeheight))) {
                  printf("%s -h: invalid planeheight (%s)\n\n",progname,*argv);
                  exit(1);
               }
               while (*(*argv+1))   /* Don't ask,     */
                  *++*argv;         /* just DON'T ask */
               break;
            case 'l':
               flag->l = 1;
               if (!sscanf((argc--, *++argv), "%d", &(lim->plotxmin))) {
                  printf("%s -l: invalid blh (%s)\n\n", progname, *argv);
                  exit(1);
               }
               if (!sscanf((argc--, *++argv), "%d", &(lim->plotymin))) {
                  printf("%s -l: invalid blv (%s)\n\n", progname, *argv);
                  exit(1);
               }
               if (!sscanf((argc--, *++argv), "%d", &(lim->mapwidth))) {
                  printf("%s -l: invalid map width (%s)\n\n",
                          progname, *argv);
                  exit(1);
               }
               if (lim->mapwidth <= 0) {
                  printf("%s -l: invalid map width (%s)\n\n",
                          progname, *argv);
                  exit(1);
               }
               while (*(*argv+1))   /* Don't ask,     */
                  *++*argv;         /* just DON'T ask */
               break;
            case 'L':
               flag->L = 1;
               if (!sscanf((argc--, *++argv), "%d", &(lim->plotxmin))) {
                  printf("%s -L: invalid xmin (%s)\n\n", progname, *argv);
                  exit(1);
               }
               if (!sscanf((argc--, *++argv), "%d", &(lim->plotymin))) {
                  printf("%s -L: invalid ymin (%s)\n\n", progname, *argv);
                  exit(1);
               }
               if (!sscanf((argc--, *++argv), "%d", &(lim->plotzmin))) {
                  printf("%s -L: invalid zmin (%s)\n\n", progname, *argv);
                  exit(1);
               }
               if (!sscanf((argc--, *++argv), "%d", &(lim->mapwidth))) {
                  printf("%s -L: invalid map width (%s)\n\n",
                          progname, *argv);
                  exit(1);
               }
               if (lim->mapwidth <= 0) {
                  printf("%s -L: invalid map width (%s)\n\n",
                          progname, *argv);
                  exit(1);
               }
               while (*(*argv+1))   /* Don't ask,     */
                  *++*argv;         /* just DON'T ask */
               break;
            default:
               printf("%s: illegal option -%c\n\n", progname, c);
               exit(1);
               break;
         }
   }
   if (argc)
      strcpy(filename, *argv);
   else {
      printf("Usage: %s [-xyzt | -p] [-sncdgro]", progname);
      printf(" [-l blh blv mw | -L xmin ymin zmin mw]\n");
      printf("           [-h planeheight] filename\n\n");
      printf("   -x or -yz  : collapse x-axis (2D map)\n");
      printf("   -y or -xz  : collapse y-axis (2D map)\n");
      printf("   -z or -xy  : collapse z-axis (2D map) (default)\n");
      printf("   -p or -xyz : stereo-pair output\n");
      printf("   -t         : three dimensional map\n");
      printf("   -s         : display spectral type on map\n");
      printf("   -n         : display name on map\n");
      printf("   -c         : display collapsed coordinate on map\n");
      printf("   -d         : suppress data file page/s\n");
      printf("   -g         : suppress map grid lines\n");
      printf("   -r         : display vertical reference plane");
      printf(" in 3D plot\n");
      printf("   -o         : produce planetary orbit plots\n");
      printf("   -l         : set limits for 2D map:");
      printf(" blh = bottom left horizontal coord,\n");
      printf("                blv = bottom left vertical coord,");
      printf(" mw = map width (>0)\n");
      printf("   -L         : set limits for 3D map:");
      printf(" xmin,ymin,zmin = min x,y,z coords,\n");
      printf("                mw = map width (>0)\n");
      printf("   -h         : set reference plane height in 3D map\n");
      printf("\n");
      exit(1);
   }
   if (flag->x && flag->y && flag->z)
      flag->p = 1, flag->x = flag->y = flag->z = 0;
   else if (flag->y && flag->z)
      flag->x = 1, flag->y = flag->z = 0;
   else if (flag->x && flag->z)
      flag->y = 1, flag->x = flag->z = 0;
   else if (flag->x && flag->y)
      flag->z = 1, flag->x = flag->y = 0;
   if (!(flag->p || flag->x || flag->y || flag->z))
      flag->z = 1, flag->x = flag->y = 0;
   if (flag->L)
      flag->t = 1;
   if ((flag->p && (flag->x || flag->y || flag->z || flag->t)) ||
       (flag->l && flag->t)) {
      printf("Fatal conflict between arguments\n");
      printf("Knight - Queen's Bishop 4, checkmate (N - QB4++)\n\n");
      exit(1);
   }
}

void
getdata(head, infile)
STARINFO  **head;
FILE      *infile;
{
   int       linenum;
   char      line[LINELEN];
   STARINFO  *temp, *tailstar;
   PLANINFO  *plan, *tailplan, *tailmoon;

   tailstar = NULL;
   tailplan = NULL;
   tailmoon = NULL;
   linenum = 1;

   while (fgets(line, LINELEN, infile) != NULL) {
      switch (tolower(*line)) {
         case '/' : break;
         case 'p' :
            if (!tailstar) {
               printf("No star before first planet!\n");
               exit(1);
            }
            plan = NEWPLAN;
            if (sscanf(line, "p %f , %f , %f , %f , %f , %f , %[^.].",
                &plan->orbit, &plan->eccentricity, &plan->inclination,
                &plan->diameter, &plan->axial, &plan->rotation,
                plan->name) != 7) {
               printf("Illegal data format in line %d\n",linenum);
               free(plan);
            }
            else {
               plan->next = NULL;
               plan->moon = NULL;
               if (tailstar->planet)
                  tailplan->next = plan;
               else
                  tailstar->planet = plan;
               tailplan = plan;
               tailmoon = NULL;
            }
            break;
         case 'm' :
            if (!tailstar) {
               printf("No star before first moon!\n");
               exit(1);
            }
            if (!tailplan) {
               printf("No planet before moon!\n");
               exit(1);
            }
            plan = NEWPLAN;
            if (sscanf(line, "m %f , %f , %f , %f , %f , %f , %[^.].",
                &plan->orbit, &plan->eccentricity, &plan->inclination,
                &plan->diameter, &plan->axial, &plan->rotation,
                plan->name) != 7) {
               printf("Illegal data format in line %d\n",linenum);
               free(plan);
            }
            else {
               plan->next = NULL;
               if (tailplan->moon)
                  tailmoon->next = plan;
               else
                  tailplan->moon = plan;
               tailmoon = plan;
            }
            break;
         default:
            temp = NEWSTAR;
            if (sscanf(line, "%f, %f, %f, %[^,], %[^.].",
                &temp->x, &temp->y, &temp->z, temp->name, temp->type) != 5) {
               printf("Illegal data format in line %d\n",linenum);
               free(temp);
            }
            else {
               temp->next = NULL;
               temp->planet = NULL;
               if (*head)
                  tailstar->next = temp;
               else
                  *head = temp;
               tailstar = temp;
               tailplan = NULL;
               tailmoon = NULL;
            }
            break;
      }
      linenum++;
   }
   fclose(infile);
}
   
int
main(argc, argv)
int    argc;
char   *argv[];
{
   char      filename[NAMELEN];
   char      line[LINELEN];
   struct    stat stbuf;
   FILE      *infile, *outfile;
   STARINFO  *head;
   LIMINFO   lim;
   FLAGINFO  flag;

   getargs(argc, argv, filename, &lim, &flag);

   if ((infile = fopen(filename,"r"))==NULL) {
      printf("Cannot find file %s\n",filename);
      exit(1);
   }
   strcat(filename,".ps");

   head = NULL;
   getdata(&head, infile);

   if (!head) {
      printf("The data file is empty\n");
      exit(1);
   }

   /*if (stat(filename, &stbuf) != NOTFOUND) {
      printf("Clobber existing %s? ",filename);
      if (tolower(*gets(line)) != 'y')
         exit(1);
   }*/ /* ALWAYS OVER WRITE!*/
   if ((outfile = fopen(filename,"w")) == NULL) {
      printf("Cannot open output file %s\n",filename);
      exit(1);
   }
   if (flag.p)
      dopersp(head, outfile, &lim);
   else if (flag.t)
      do3D(head, outfile, &lim, &flag);
   else if (flag.o)
      doorbits(head, outfile);
   else
      doflat(head, outfile, &lim, &flag);
   fclose(outfile);
   return 0;
}
