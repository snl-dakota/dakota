*   _______________________________________________________________________
*
*   DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
*   Copyright 2014 Sandia Corporation.
*   This software is distributed under the GNU Lesser General Public License.
*   For more information, see the README file in the top Dakota directory.
*   _______________________________________________________________________
*
c234567
*
* rosenbrock_bb.f is a "black-box" FORTRAN77 program that evaluates the
* Rosenbrock function (see Gill, Murray, Wright; 1981; p. 95). The input
* values x1 and x2 are read in from an ASCII text input file named
* "ros.in". The output data for f, g1, and g2 (all functions of x1, x2)),
* where f is Rosenbrock's function and g1 and g2 are the derivatives w.r.t.
* x1 and x2, respectively, are written to an output file named "ros.out". 
*
* The UNIX script named "simulator_script" performs all of the steps needed
* to pass data from DAKOTA's "params.in" file to "ros.in", run this program
* ("rosenbrock_bb"), and extract data from "ros.out" and pass it to
* DAKOTA's "results.out" file.
*
* Tony Giunta, 07 February 2002
* (yes, FORTRAN77 is still useful in the 21st century)
*
* Brian M. Adams, 18 February 2008: incorporated Tony's gradient calculations
*
******************************************************
*
* variable declarations
*
      integer i
      double precision x1,x2,rosen,g1,g2

* 
* open input and output files
*
      open(unit=21,file='ros.in')
      open(unit=22,file='ros.out')

*
* skip over all of the comments and pseudo-finite element text
* in the "ros.in" input file
*
      do 10 i = 1,26
         read (21,*)
   10 continue

*
* use a formatted read to get the values of x1 and x2
*
      read(21,1000) x1
      read(21,1000) x2

*
* compute the Rosenbrock function and derivatives for x1 and x2
* (to be thorough, gradient calculation should depend on active set)
*
      rosen = 100.*(x2-x1*x1)*(x2-x1*x1) + (1.-x1)*(1.-x1)
      g1 = -400.*x1*(x2-x1*x1) - 2.*(1-x1)
      g2 = 200.*(x2-x1*x1)

*
* write out the values of x1, x2 and rosen in a format
* that allows the UNIX script "simulator_script" to
* extract the value of Rosenbrock's function
*
      write(22,*)
      write(22,*)"Beginning execution of model: Rosenbrock black box"
      write(22,*)"Set up complete."
      write(22,*)"Reading nodes."
      write(22,*)"Reading elements."
      write(22,*)"Reading materials."
      write(22,*)"Checking connectivity...OK"
      write(22,*)"*****************************************************"
      write(22,*)
      write(22,1010) x1
      write(22,1020) x2
      write(22,*)
      write(22,*)"Computing solution...Done"
      write(22,*)"*****************************************************"
      write(22,1030) rosen
      write(22,1040) g1,g2
      write(22,*)

*
* close I/O files
*
      close(21)
      close(22)

* 
* format block
*
 1000 format(10x,E23.16)
 1010 format(1x,'Input value for x1 =',1x,E23.16)
 1020 format(1x,'Input value for x2 =',1x,E23.16)
 1030 format(1x,'Function value =',1x,E23.16)
 1040 format(1x,'Function gradient =',1x,
     +          '[ ',E23.16,1x,E23.16,' ]')

*
* terminate
*
      stop
      end
