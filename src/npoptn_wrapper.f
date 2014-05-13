*   _______________________________________________________________________
*
*   DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
*   Copyright 2014 Sandia Corporation.
*   This software is distributed under the GNU Lesser General Public License.
*   For more information, see the README file in the top Dakota directory.
*   _______________________________________________________________________
*
C This Fortran wrapper circumvents problems with implicit string sizes
C in f77.

C --------------------------
C Wrapper for NPSOL's npoptn
C --------------------------
      subroutine npoptn2( string )

C Fix the string size and always call npoptn2 from C++ with a string of
C length 72
      character*72 string

C Since calling from F77 now, the implicit string size passing should work
      call npoptn( string )

      end
