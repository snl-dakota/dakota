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
#include <fstream>
#include <utilib/_math.h>
#ifndef UTILIB_DISABLE_COMMONIO

#undef WARNINGS
#undef DEBUG
#undef UTILIB_VALIDATING
#include <utilib/CommonIO.h>
#include <utilib/mpiUtil.h>
#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif
#include <cxxtest/TestSuite.h>

namespace utilib {

#ifndef UTILIB_DISABLE_COMMONIO
int test_commonio(int argc,char** argv)
{
#ifdef UTILIB_HAVE_MPI
uMPI::init(&argc,&argv,MPI_COMM_WORLD);
#endif

/********************* Comment out ************
#if 1
#ifdef UTILIB_HAVE_MPI
int rank,size;
MPI_Comm_rank(MPI_COMM_WORLD,&rank);
MPI_Comm_size(MPI_COMM_WORLD,&size);

cout << "Basic Data: " << rank << " " << size << endl;
cout << "uMPI Data:  " << uMPI::rank << " " << uMPI::size << " " << uMPI::running() << endl;
int foo=-1;
cin >> foo;
cout << "FOO=" << foo << endl;
#endif
#endif
******************** End comment out ************/

   CommonIO::begin();
   ucout << "First line of text" << std::endl;
   ucout << Flush;
   CommonIO::end();

   CommonIO::begin();
   CommonIO::begin_tagging();
   ucout << "Second line of text" << std::endl;
   ucout << Flush;
   CommonIO::end_tagging();
   CommonIO::end();

   CommonIO::begin();
   CommonIO::begin_tagging();
   ucout << "Third line of text" << std::endl;
   ucout << Flush;
   double x = 123.4567;
   ucout << pscientific(x) << std::endl;
   for(int i=1; i<=4; i++)
     {
       ucout.precision(i);

       std::cout << utilib::CommonIO::MapCout->precision() << " " << std::cout.precision() << " " << (&ucout)->precision() << std::endl;
       ucout << "UCOUT: " << pscientific(x) << std::endl << Flush;
       ucout << "UCOUT: " << pscientific(x) << std::endl << Flush;
     }
   for(int i=1; i<=4; i++) {
       std::cout.precision(i);
       std::cout <<  "COUT:  " << pscientific(x) << std::endl << std::flush;
     }

   CommonIO::end_tagging();
   CommonIO::end();

#ifdef UTILIB_HAVE_MPI
uMPI::done();
#endif
return 0;
}

#else
int test_commonio(int,char**) {return 0;}
#endif


namespace unittest { class CommonIO; }
class utilib::unittest::Test_CommonIO : public CxxTest::TestSuite
{
public:

    void test1()
    {
    ofstream of("TCommonIO1.out");
    CommonIO::set_streams(&of,0,0);
    of << "TEST 1" << std::endl;
    of.close();
    TS_ASSERT_SAME_FILES("TCommonIO1.out", "TCommonIO1.txt");
    //unlink("Tstring_ops_wordwrap.out");
    }

};

} // namespace utilib
