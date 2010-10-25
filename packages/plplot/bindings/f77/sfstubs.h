C      $Id: sfstubs.h 3186 2006-02-15 18:17:33Z slbrow $
C      common blocks required for routines in sfstubs.f
C
C      Copyright (C) 2004  Alan W. Irwin
C
C      This file is part of PLplot.
C
C      PLplot is free software; you can redistribute it and/or modify
C      it under the terms of the GNU General Library Public License as
C      published by the Free Software Foundation; either version 2 of the
C      License, or (at your option) any later version.
C
C      PLplot is distributed in the hope that it will be useful,
C      but WITHOUT ANY WARRANTY; without even the implied warranty of
C      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C      GNU Library General Public License for more details.
C
C      You should have received a copy of the GNU Library General Public
C      License along with PLplot; if not, write to the Free Software
C      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

      integer maxlen
      parameter (maxlen = 320)
      character*(maxlen) string1, string2, string3
      character*(maxlen) string4, string5, string6
      character*(maxlen) string7, string8, string9
      integer s1(80), s2(80), s3(80),
     &  s4(80), s5(80), s6(80), s7(80), s8(80), s9(80)
      equivalence ( s1, string1 ), ( s2, string2 )
      equivalence ( s3, string3 ), ( s4, string4 )
      equivalence ( s5, string5 ), ( s6, string6 )
      equivalence ( s7, string7 ), ( s8, string8 )
      equivalence ( s9, string9 )
      common /zzplstr1/ string1
      common /zzplstr2/ string2
      common /zzplstr3/ string3
      common /zzplstr4/ string4
      common /zzplstr5/ string5
      common /zzplstr6/ string6
      common /zzplstr6/ string7
      common /zzplstr6/ string8
      common /zzplstr6/ string9
