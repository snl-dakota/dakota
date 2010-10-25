/*
  This header file contains the lookup table used for converting between
  FCIs (font characterization integers) and font names for the standard
  35 type 1 fonts.

  Copyright (C) 2005  Alan W. Irwin

  This file is part of PLplot.
  
  PLplot is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Library Public License as published
  by the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  PLplot is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with PLplot; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/* There are no good choices for script fonts for Type 1 so I just
 * defaulted to the Times variants in that case. */

/* There is only one Type 1 symbol font in the standard 35 so use
 * it for all variations. */

/* N.B. When updating this table by hand be sure to keep it in 
 * ascending order in fci! 
 */
#define N_Type1Lookup 30
const FCI_to_FontName_Table Type1Lookup[N_Type1Lookup] = {
     {0x10000000, "Helvetica"},
     {0x10000001, "Times-Roman"},
     {0x10000002, "Courier"},
     {0x10000003, "Times-Roman"},
     {0x10000004, "Symbol"},
     {0x10000010, "Helvetica-Oblique"},
     {0x10000011, "Times-Italic"},
     {0x10000012, "Courier-Oblique"},
     {0x10000013, "Times-Italic"},
     {0x10000014, "Symbol"},
     {0x10000020, "Helvetica-Oblique"},
     {0x10000021, "Times-Italic"},
     {0x10000022, "Courier-Oblique"},
     {0x10000023, "Times-Italic"},
     {0x10000024, "Symbol"},
     {0x10000100, "Helvetica-Bold"},
     {0x10000101, "Times-Bold"},
     {0x10000102, "Courier-Bold"},
     {0x10000103, "Times-Bold"},
     {0x10000104, "Symbol"},
     {0x10000110, "Helvetica-BoldOblique"},
     {0x10000111, "Times-BoldItalic"},
     {0x10000112, "Courier-BoldOblique"},
     {0x10000113, "Times-BoldItalic"},
     {0x10000114, "Symbol"},
     {0x10000120, "Helvetica-BoldOblique"},
     {0x10000121, "Times-BoldItalic"},
     {0x10000122, "Courier-BoldOblique"},
     {0x10000123, "Times-BoldItalic"},
     {0x10000124, "Symbol"}
};
