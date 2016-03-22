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
C BMA (20160315): Changed to use Fortran 2003 ISO C bindings.
      subroutine npoptn2( string_in ) bind(C)

      use iso_c_binding, only: C_CHAR
      integer, parameter :: num_char = 72
      character (kind=C_CHAR, len=1), dimension (num_char) :: string_in

C Fix the string size and always call npoptn2 from C++ with a string of
C length 72
      character*72 string

C TODO: Could instead iterate until C_NULL_CHAR and allow variable length
      loop_str: do i=1, num_char
        string(i:i) = string_in(i)
      end do loop_str

C Since calling from F77 now, the implicit string size passing should work
      call npoptn( string )

      end
