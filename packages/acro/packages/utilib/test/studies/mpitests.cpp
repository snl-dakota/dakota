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

#include <string.h>
#include <iostream>
#include <fstream>

#include <utilib_config.h>
#include <utilib/mpiUtil.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::streambuf;
using std::stringstream;

   int test_mpitest(int argc, char** argv);
   int test_mpistream(int argc, char** argv);


#ifndef UTILIB_HAVE_MPI
int main()
{
   cout << "UTILIB not configured with MPI support; cannot run tests."
             << endl;
   return 1;
}

#else // UTILIB_HAVE_MPI

int main(int argc, char** argv)
{
   // Step # 1: configure & initialise MPI
   utilib::uMPI::init(&argc,&argv);
   int rank = utilib::uMPI::rank;
   if ( ! utilib::uMPI::running() )
   {
      cout << "Error: uMPI::init() failed to initialize MPI" << endl;
      return 1;
   }

   char* tmp = 0;
   //
   // This ignores all arguments after the first.
   //
   if (argc > 1) {
      tmp = argv[1];
      argv++;
      argc--;
   }

   if ( tmp == NULL )
   {
      if ( rank == 0 )
         cout << "Error: unspecified test." << endl;
      utilib::uMPI::done();
      return 1;
   }

   string test = tmp;
   if ((test.size() > 1 ) && ( test[0] == '"' ) && ( *test.rbegin() == '"' ))
   {
      test.erase(0,1);
      test.erase(test.size()-1, 1);
   }

   // Redirect cout/cerr on all non-root ranks to files.
   streambuf *save_cerr = cerr.rdbuf();
   streambuf *save_cout = cout.rdbuf();
   fstream log_cerr;
   fstream log_cout;
   if ( rank > 0 )
   {
      stringstream ss;
      ss << tmp << "." << rank << ".";
      log_cerr.open((ss.str() + "err").c_str(), fstream::out);
      log_cout.open((ss.str() + "out").c_str(), fstream::out);

      // Redirect stream buffers
      if (log_cerr.is_open())
         cerr.rdbuf(log_cerr.rdbuf());
      else
         cerr << "Error opening error file (rank " << rank << ")" << endl;
      if (log_cout.is_open())
         cout.rdbuf(log_cout.rdbuf());
      else
         cerr << "Error opening output file (rank " << rank << ")" << endl;
   }

   int status=-1;

   try {

      if ( test == "mpitest" )
         status = test_mpitest(argc,argv);

      if ( test == "mpistream" )
         status = test_mpistream(argc,argv);

      if ((status == -1) && ( rank == 0 ))
         cout << "ERROR: unknown test \"" << tmp << "\"" << endl;

   } // end of main try block...
   catch ( std::exception &e )
   { 
      if ( rank == 0 )
         cout << "Caught exception: " << e.what() << endl; 
   }
   catch ( ... )
   { 
      if ( rank == 0 )
         cout << "Caught unknown exception." << endl; 
   }

   // Restore cerr's stream buffer before terminating
   if (log_cerr.is_open())
      cerr.rdbuf(save_cerr);
   if (log_cout.is_open())
      cout.rdbuf(save_cout);

   // Finallize MPI
   utilib::uMPI::done();

   return status;
}

#endif // UTILIB_HAVE_MPI
