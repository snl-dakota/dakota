*   _______________________________________________________________________
*
*   DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
*   Copyright 2014 Sandia Corporation.
*   This software is distributed under the GNU Lesser General Public License.
*   For more information, see the README file in the top Dakota directory.
*   _______________________________________________________________________
*
      program container
c*****************************************************************      
c*****************************************************************      
      integer num_fns,num_vars,req(1:2)
      double precision fval(1:2),D,H
      character*80 infile,outfile,instr
      character*25 valtag(1:2)
      double precision PI /3.14159265358979/

c     get the input and output file names from the command line
c     using the fortran 77 library routine getarg
      call getarg(1,infile)
      call getarg(2,outfile)

c*************************************
c     read the input data from DAKOTA	
c*************************************
      open(11,FILE=infile,STATUS='OLD')

c     get the number of variables
      read(11,*)num_vars,instr

c     get the values of the variables and the associated tag names
      read(11,*)H,instr
      read(11,*)D,instr

c     get the number of functions
      read(11,*)num_fns,instr

c     get the evaluation type request for the associated function number
      do 10 i=1,num_fns
        read(11,*)req(i),instr
 10   continue

      close(11)


c**********************************************************
c     compute the objective function and constraint values
c**********************************************************
      if(req(1).eq.1) fval(1)=0.644*PI*D**2+1.05*PI*D*H
      if(req(2).eq.1) fval(2)=0.25*PI*H*D**2-63.525

c******************************************
c     write the response output for DAKOTA
c******************************************
      valtag(1)='area'
      valtag(2)='volume_constraint'

      open(11,FILE=outfile,STATUS='UNKNOWN')

      do 20 i=1,num_fns
        if(req(i).eq.1) then
          write(11,'(E22.15,1X,A)'),fval(i),valtag(i)
        endif
 20   continue
 
      close(11)

      end


