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


// A very rudimentary test code for pack and unpack of a few datatypes,
// and also reading and writing packed buffers to/from binary files.
#include <utilib/std_headers.h>
#include <utilib/PackBuf.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
using namespace utilib;
#endif

void snarl(const char* what)
{
  cerr << "Error detected unpacking " << what << endl;
  exit(1);
}


int test_packbuf(int argc, char** argv)
{
#ifdef UTILIB_HAVE_MPI
  uMPI::init(&argc,&argv);
#else
  argc=0;
  argv=0;
#endif
  {
    PackBuffer obuf;
    
    bool b = true;
    obuf << b;

    int i = 123456789;
    obuf << i;

    long l = 987654321;
    obuf << l;

    short s = 2468;
    obuf << s;

    float f = 234.5;
    obuf << f;

    double d = 1234567.89012;
    obuf << d;

    const char* cs = "Hello";
    obuf << cs;

    ofstream os("packbuf.dat",ios::out | ios::binary);
    obuf.writeBinary(os);
  }
  
  {
    UnPackBuffer ibuf;
    ifstream is("packbuf.dat",ios::in | ios::binary);
    ibuf.readBinary(is);
    
    bool b = false;
    ibuf >> b;
    if (!b) snarl("bool");

    int i = 0;
    ibuf >> i;
    if (i != 123456789) snarl("int");

    long l = 0;
    ibuf >> l;
    if (l != 987654321) snarl("long");

    short s = 0;
    ibuf >> s;
    if (s != 2468) snarl("short");

    float f = 0;
    ibuf >> f;
    if (f != 234.5) snarl("float");

    double d = 1234567.89012;
    ibuf >> d;
    if (d != 1234567.89012) snarl("double");

    char cs[6];
    ibuf >> cs;
    if (strcmp(cs,"Hello")) snarl("char*");

    if (ibuf.curr() < ibuf.size()) snarl("end of message");
  }

  remove("packbuf.dat");

  cout << "Test completed successfully\n";

#ifdef UTILIB_HAVE_MPI
  uMPI::done();
#endif

  return 0;
}

  
  
    

    
