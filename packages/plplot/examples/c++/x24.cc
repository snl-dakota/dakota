//
//  Unicode Pace Flag
//
//  Copyright (C) 2005 Rafael Laboissiere
//
//
//  This file is part of PLplot.
//
//  PLplot is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Library Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  PLplot is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public License
//  along with PLplot; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//
// In Debian, run like this:
//
// PLPLOT_FREETYPE_SANS_FONT=/usr/share/fonts/truetype/arphic/bkai00mp.ttf \
// PLPLOT_FREETYPE_SERIF_FONT=/usr/share/fonts/truetype/freefont/FreeSerif.ttf \
// PLPLOT_FREETYPE_MONO_FONT=/usr/share/fonts/truetype/ttf-indic-fonts/lohit_hi.ttf \
// PLPLOT_FREETYPE_SCRIPT_FONT=/usr/share/fonts/truetype/unfonts/UnBatang.ttf \
// PLPLOT_FREETYPE_SYMBOL_FONT=/usr/share/fonts/truetype/ttf-bangla-fonts/JamrulNormal.ttf \
// ./x24 -dev png -drvopt text,smooth=0 -o x24cxx.png
//
// Packages needed:
//
// ttf-arphic-bkai00mp
// ttf-freefont
// ttf-indic-fonts
// ttf-unfonts
// ttf-bangla-fonts
//
//


#include "plstream.h"
#include "plevent.h"

class x24 {

public:
  x24(int, char **);

private:
  plstream *pls;

  static PLINT red[], green[], blue[];

  static PLFLT px[], py[], sx[], sy[];

  static char *peace[];
};

PLINT x24::red[]   = {240, 204, 204, 204,   0,  39, 125};
PLINT x24::green[] = {240,   0, 125, 204, 204,  80,   0};
PLINT x24::blue[]  = {240,   0,   0,   0,   0, 204, 125};

PLFLT x24::px[] = {0.0, 0.0, 1.0, 1.0};
PLFLT x24::py[] = {0.0, 0.16667, 0.16667, 0.0};

PLFLT x24::sx[] = {
  0.16374,
  0.15844,
  0.15255,
  0.17332,
  0.50436,
  0.51721,
  0.49520,
  0.48713,
  0.83976,
  0.81688,
  0.82231,
  0.82647
};

PLFLT x24::sy[] = {
  0.11341,
  0.35481,
  0.61621,
  0.86525,
  0.13249,
  0.37995,
  0.61584,
  0.86303,
  0.13901,
  0.30533,
  0.63165,
  0.87598
};


/* Taken from http://www.columbia.edu/~fdc/pace/ */

char* x24::peace[] = {
  /* Mandarin */
  "#<0x00>和平",
  /* Hindi */
  "#<0x20>शांति",
  /* English */
  "#<0x10>Peace",
  /* Hebrew */
  "#<0x10>םולש",
  /* Russian */
  "#<0x10>Мир",
  /* German */
  "#<0x10>Friede",
  /* Korean */
  "#<0x30>평화",
  /* French */
  "#<0x10>Paix",
  /* Spanish */
  "#<0x10>Paz",
  /* Arabic */
  "#<0x10>مﻼﺳ",
  /* Turkish*/
  "#<0x10>Barış",
  /* Kurdish */
  "#<0x10>Hasîtî",
};

x24::x24(int argc, char *argv[])
{
  int i, j;

  pls = new plstream();

  pls->parseopts (&argc, argv, PL_PARSE_FULL);

  pls->init ();

  pls->adv (0);
  pls->vpor (0.0, 1.0, 0.0, 1.0);
  pls->wind (0.0, 1.0, 0.0, 1.0);
  pls->col0 (0);
  pls->box("", 1.0, 0, "", 1.0, 0);

  pls->scmap0n (7);
  pls->scmap0 (red, green, blue, 7);

  pls->schr (0, 4.0);
  pls->font (1);

  for (i = 0; i < 6; i++) {

    pls->col0 (i + 1);
    pls->fill (4, px, py);

    for (j = 0; j < 4; j++)
      py [j] += 1.0 / 6.0;

  }

  pls->col0 (0);
  for (i = 0; i < 12; i++)
    pls->ptex (sx [i], sy [i], 1.0, 0.0, 0.5, peace [i]);

  delete pls;

}

int main( int argc, char **argv )
{
  x24 *x = new x24( argc, argv );

  delete x;
}
