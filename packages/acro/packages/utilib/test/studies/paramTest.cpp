/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <utilib_config.h>

#if 0
#ifndef UTILIB_HAVE_STD
#include <iostream.h>
#else
#include <iostream>
#endif
#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

#ifdef UTILIB_HAVE_MPI
#include <utilib/mpiUtil.h>
#endif
#include <utilib/CommonIO.h>
#include <utilib/paramTable.h>


using namespace utilib;

class c1
{
public:
  CLASS_PARAMETER(param1);
  CLASS_PARAMETER(param2);
  void snortate();
};


CLASS_PARAMETER_DEF(c1,param1,0,50,100);
CLASS_PARAMETER_DEF(c1,param2,0,60,100);

GLOBAL_PARAMETER_DEF(param3,0,1,1);


int test_paramtest(int argc,char** argv)
{
#ifdef UTILIB_HAVE_MPI
  uMPI::init(&argc,&argv);
  ucout << parameter::howMany() << " parameters defined on PN";
  ucout << uMPI::rank << ".\n";
  parameter::readAllSerial(argc,argv,1,2);
  ucout << '[' << uMPI::rank << "] " << "param1 = " << c1::param1() << '\n';
  ucout << '[' << uMPI::rank << "] " << "param2 = " << c1::param2() << '\n';
  ucout << '[' << uMPI::rank << "] " << "param3 = " << param3() << '\n';
#endif
  c1 object;
  object.snortate();
#ifdef UTILIB_HAVE_MPI
  uMPI::done();
#endif
return 0;
}


void c1::snortate()
{
  ucout << "From within C1, param1 = " << param1() << '\n';
}

#else

int test_paramtest(int,char**)
{
return 0;
}

#endif
