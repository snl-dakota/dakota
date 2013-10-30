/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- class:       Implementation code for DakotaBinStream
//- Description: A replacement for the RWbistream class.  This class reads
//-              and writes binary files by extending the basic iostream
//-		 Note : If RPC/XDR not supported, routines output data 
//-              in native binary.
//-
//- Owner:       Mario Alleva
//- Version: $Id: 

#include "DakotaBinStream.hpp"
#include "dakota_global_defs.hpp"


namespace Dakota {

/** Constructor which takes a char* filename.  Calls the base 
    class open method with the filename and no other arguments.
    Also allocates the xdr stream.   */
BiStream::BiStream(const char *s)
{ 
  // call open method
  open(s); 
  // create xdr stream
#ifndef NO_XDR
  dak_xdrmem_create(&xdrInBuf, (char *)inBuf, sizeof(inBuf),XDR_DECODE);
#endif
}


/** Constructor which takes a char* filename and int flags.  
    Calls the base class open method with the filename and 
    flags as arguments.  Also allocates xdr stream.    */
BiStream::BiStream(const char *s, std::ios_base::openmode mode)
{ 
  // call open method with flags
  std::ifstream::open(s, mode);

#ifndef NO_XDR
  // create xdr stream
  dak_xdrmem_create(&xdrInBuf, (char *)inBuf, sizeof(inBuf),XDR_DECODE);
#endif
}


/** Default constructor, allocates xdr stream , but does not call the open
    method.  The open method must be called before stream can be read.  */
BiStream::BiStream()
{
  // create xdr stream
#ifndef NO_XDR
  dak_xdrmem_create(&xdrInBuf, (char *)inBuf, sizeof(inBuf),XDR_DECODE);
#endif
}


/** Destructor, destroys the xdr stream allocated in constructor */
BiStream::~BiStream()
{
  // destroy xdr stream
#ifndef NO_XDR
  dak_xdr_destroy(&xdrInBuf);
#endif
}


// input operators  

/**  The std::string input operator must first read both the xdr buffer size
     and the size of the string written.  Once these are read it can then
     read and convert the std::string correctly. */
BiStream& BiStream::operator>>(std::string& ds)
{
#ifndef NO_XDR
  /* convert message structure to local representation */
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;

  // The reason for the (sizeof(int) *2) is that we are
  // reading in two integers before the string (size,xdrStringSize)
  if (read((char*)inBuf, sizeof(int) * 2) == 0)
    return *this; // if EOF return *this

  // convert to integers
  int size;
  int xdr_string_size;
  dak_xdr_int(&xdrInBuf, &size);
  dak_xdr_int(&xdrInBuf, &xdr_string_size);

  // reset xdr stream position
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  /* read and convert message structure to local representation */
  
  if (read((char*)inBuf, xdr_string_size) == 0)
    return *this;

  // read into a tmp array, add null terminator and then assign to std::string
  char* tmp = (char *)std::malloc(size + 1);
  dak_xdr_string(&xdrInBuf, (char **) &tmp, size);
  tmp[size] = '\0';       // Add the null terminator
  ds = tmp; 
  //Cout << "debug: std::string = " << ds << std::endl;
  std::free(tmp);
#else
  // do native binary reads
  // read in size of string
  int size;
  if (read((char*)&size, sizeof(int)) == 0)
    return *this; // if EOF return *this
  // read in string
  char* tmp = (char *)std::malloc(size + 1);
  if (read((char*)tmp, size) == 0)
    return *this;
  tmp[size] = '\0';       // Add the null terminator
  ds = tmp; 
  std::free(tmp);
  //Cout << "debug: std::string = " << ds << std::endl;
#endif

  return *this;
}


BiStream& BiStream::operator>>(char& c)
{    
#ifndef NO_XDR
  // Calculate the size of a xdr char buffer by doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  char tc;
  dak_xdr_char(&xdrInBuf, (char *) &tc);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf,xdr_buffer_size) == 0)
    return *this;  
  // convert to char
  dak_xdr_char(&xdrInBuf, &c);
  //Cout << "debug:  char in = " << c << std::endl;
#else
  // Do native binary read of char
  if (read((char*) &c,sizeof(char) ) == 0)
    return *this;  
  //Cout << "debug:  char in = " << c << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(int& i)
{
#ifndef NO_XDR
  // Calculate the size of a xdr int buffer by doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  int ti;
  dak_xdr_int(&xdrInBuf, (int *) &ti);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to integers
  dak_xdr_int(&xdrInBuf, &i);
  //Cout << "debug:  int in = " << i << std::endl;
#else
  // Do native binary read of int
  if (read((char*)&i, sizeof(int)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug:  int in = " << i << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(short& s)
{
#ifndef NO_XDR
  // Calculate the size of an xdr short buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  short ts;
  dak_xdr_short(&xdrInBuf, (short *) &ts);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to short
  dak_xdr_short(&xdrInBuf, &s);
  //Cout << "debug:  short in = " << s << std::endl;
#else
  // Do native binary read of short
  if (read((char*)&s, sizeof(short)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug:  short in = " << s << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(bool& b)
{
#ifndef NO_XDR
  // Calculate the size of an xdr bool buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  bool_t tb;
  dak_xdr_bool(&xdrInBuf, (bool_t *) &tb);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to bool_t
  bool_t tmp;
  dak_xdr_bool(&xdrInBuf, &tmp);
  // convert to bool
  b = (tmp) ? true : false;
  //Cout << "debug:  bool in = " << b << std::endl;
#else
  // Do native binary read of bool
  if (read((char*)&b, sizeof(bool)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug:  bool in = " << b << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(double& d)
{
#ifndef NO_XDR
  // Calculate the size of an xdr double buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  double td;
  dak_xdr_double(&xdrInBuf, (double *) &td);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to double
  dak_xdr_double(&xdrInBuf, &d);
  //Cout << "debug:  double in = " << d << std::endl;
#else
  // Do native binary read of double
  if (read((char*)&d, sizeof(double)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug:  double in = " << d << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(float& f)
{
#ifndef NO_XDR
  // Calculate the size of an xdr float buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  float tf;
  dak_xdr_float(&xdrInBuf, (float *) &tf);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to float
  dak_xdr_float(&xdrInBuf, &f);
  //Cout << " debug : float in = " << f << std::endl;
#else
  // Do native binary read of float
  if (read((char*)&f, sizeof(float)) == 0)
    return *this;  // if EOF return *this
  //Cout << " debug : float in = " << f << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(long& l)
{
#ifndef NO_XDR
  // Calculate the size of an xdr long buffer by first doing a conversion
  //   NOTE that for 64 bit software, the xdr representation of a long
  //   and a u_long, is 32 bits. 
  //   All existing protocols expect only 32-bit data.
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  long tl;
  dak_xdr_long(&xdrInBuf, (long *) &tl);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;

  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to long
  dak_xdr_long(&xdrInBuf, &l);
  //Cout << " debug:long in = " << l << std::endl;
#else
  // Do native binary read of long
  if (read((char*)&l, sizeof(long)) == 0)
    return *this;  // if EOF return *this
  //Cout << " debug:long in = " << l << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(unsigned char& c)
{
#ifndef NO_XDR
  // Calculate the size of an xdr uchar buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  unsigned char tc;
  dak_xdr_u_char(&xdrInBuf, (unsigned char *) &tc);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to uchar
  dak_xdr_u_char(&xdrInBuf, &c);

  //Cout << "debug: uchar in = " << c << std::endl;
#else
  // Do native binary read of unsigned char
  if (read((char*)&c, sizeof(unsigned char)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug: uchar in = " << c << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator >> (unsigned int& i)
{
#ifndef NO_XDR
   // Calculate the size of an xdr uint buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  unsigned int ti;
  dak_xdr_u_int(&xdrInBuf, (unsigned int *) &ti);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to uint
  dak_xdr_u_int(&xdrInBuf, &i);
  //Cout << "debug: uint in = " << i << std::endl;
#else
  // Do native binary read of unsigned int
  if (read((char*)&i, sizeof(unsigned int)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug: uint in = " << i << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(unsigned short& s)
{
#ifndef NO_XDR
  // Calculate the size of an xdr unsigned short buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  unsigned short ts;
  dak_xdr_u_short(&xdrInBuf, (unsigned short *) &ts);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);

  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  //  Note : xdr stores shorts as integers

  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size ) == 0)
    return *this;  // if EOF return *this

  // convert to ushort
  dak_xdr_u_short(&xdrInBuf, &s);
  //Cout << "debug: ushort in = " << s << std::endl;
#else
  // Do native binary read of unsigned short
  if (read((char*)&s, sizeof(unsigned short)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug: ushort in = " << s << std::endl;
#endif
  return *this;
}


BiStream& BiStream::operator>>(unsigned long& l)
{
#ifndef NO_XDR
  // Calculate the size of an xdr u_long buffer by first doing a conversion
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  unsigned long tl;
  dak_xdr_u_long(&xdrInBuf, (unsigned long *) &tl);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrInBuf);


  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrInBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;

  //   NOTE that for 64 bit software, the xdr representation of a long
  //   and a u_long, is 32 bits.
  //   All existing protocols expect only 32-bit data.

  // read in initial buffer using calculated buffer size
  if (read((char*)inBuf, xdr_buffer_size) == 0)
    return *this;  // if EOF return *this

  // convert to ulong
  dak_xdr_u_long(&xdrInBuf, &l);
  //Cout << "debug: ulong in = " << l << std::endl;
#else
  // Do native binary read of unsigned long
  if (read((char*)&l, sizeof(unsigned long)) == 0)
    return *this;  // if EOF return *this
  //Cout << "debug: ulong in = " << l << std::endl;
#endif
  return *this;
}


// Binary Output Stream Methods

/** Constructor, takes char * filename as argument. Calls base class open
    method with filename and no other arguments.  Also allocates xdr stream */
BoStream::BoStream(const char *s)
{ 
  open(s); 
#ifndef NO_XDR
  dak_xdrmem_create(&xdrOutBuf, (char *)outBuf, sizeof(outBuf),XDR_ENCODE);
#endif
}


/** Constructor, takes char * filename and int flags as arguments. 
    Calls base class open method with filename and flags as arguments.
    Also allocates xdr stream.  Note : If no rpc/xdr support xdr calls
    are \#ifdef'd out.  */
BoStream::BoStream(const char *s, std::ios_base::openmode mode)
{ 
  open(s, mode);
#ifndef NO_XDR
  dak_xdrmem_create(&xdrOutBuf, (char *)outBuf, sizeof(outBuf),XDR_ENCODE);
#endif
}


/** Default constructor allocates the xdr stream but does not call the open()
    method.  The open() method must be called before stream can be written to.*/
BoStream::BoStream()
{
#ifndef NO_XDR
  dak_xdrmem_create(&xdrOutBuf, (char *)outBuf, sizeof(outBuf),XDR_ENCODE);
#endif
}


BoStream::~BoStream() throw()
{
  // destroy xdr stream
#ifndef NO_XDR
  dak_xdr_destroy(&xdrOutBuf);
#endif
}


/** The std::string operator<< must first write the xdr buffer size and
    the original string size to the stream.  The input operator needs this
    information to be able to correctly read and convert the std::string. */
BoStream& BoStream::operator<<(const std::string& ds)
{
#ifndef NO_XDR
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;

  // determine size of string after converted to xdr
  int N = ds.size();
  char *tmp = (char *) ds.data(); // pointer to char data
  dak_xdr_string(&xdrOutBuf, (char **) &tmp, sizeof(char)* N);
  int xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);

  // reset xdr stream
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;

  // convert ints
  dak_xdr_int(&xdrOutBuf, &N);
  dak_xdr_int(&xdrOutBuf, &xdr_buffer_size);

  // convert string
  dak_xdr_string(&xdrOutBuf, (char **) &tmp, sizeof(char)* N);

  // get total xdr buffer size
  xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);

  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
#else
  // Do native binary writes.
  // write size of string.
  int N = ds.size();
  write((char *)&N, sizeof(int));
  // write string data
  write((char *)ds.data(), N);
#endif
  return *this;
}


BoStream& BoStream::operator<<(const char& c)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert 
  dak_xdr_char(&xdrOutBuf, (char *)&c);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: char out = " << c << std::endl;
#else
  write((char *)&c, sizeof(char));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const int& i)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert 
  dak_xdr_int(&xdrOutBuf, (int *) &i);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: int out = " << i << std::endl;
#else
  // Do native binary write of int
  write((char *)&i, sizeof(int));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const short& s)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_short(&xdrOutBuf, (short *) &s);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: short out = " << s << std::endl;
#else
  // Do native binary write of short
  write((char *)&s, sizeof(short));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const bool& b)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl ;
  // convert
  bool_t tmp = (b) ? 1 : 0;
  dak_xdr_bool(&xdrOutBuf, (bool_t *) &tmp);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: bool out = " << b << std::endl;
#else
  // Do native binary write of bool
  write((char *)&b, sizeof(bool));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const double& d)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_double(&xdrOutBuf, (double *) &d);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: double out = " << d << std::endl;
#else
  // Do native binary write of double
  write((char *)&d, sizeof(double));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const long& l)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_long(&xdrOutBuf, (long *) &l);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: long out = " << l <<  std::endl;
#else
  // Do native binary write of long
  write((char *)&l, sizeof(long));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const float& f)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_float(&xdrOutBuf, (float *) &f);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: float out = " << f << std::endl;
#else
  // Do native binary write of float
  write((char *)&f, sizeof(float));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const unsigned char& c)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_u_char(&xdrOutBuf, (unsigned char *) &c);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: uchar out = " << c << std::endl;
#else
  // Do native binary write of usigned char
  write((char *)&c, sizeof(unsigned char));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const unsigned int& i)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_u_int(&xdrOutBuf, (unsigned int *) &i);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: uint out = " << i << std::endl;
#else
  // Do native binary write of usigned int
  write((char *)&i, sizeof(unsigned int));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const unsigned short& s)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_u_short(&xdrOutBuf, (unsigned short *) &s);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: ushort out = " << s << std::endl;
#else
  // Do native binary write of usigned short
  write((char*)&s, sizeof(unsigned short));
#endif
  return *this;
}


BoStream& BoStream::operator<<(const unsigned long& l)
{
#ifndef NO_XDR
  // set xdr stream position to zero
  if (! dak_xdr_setpos(&xdrOutBuf, 0))
    Cerr << "xdr_setpos failed" << std::endl;
  // convert
  dak_xdr_u_long(&xdrOutBuf, (unsigned long *) &l);
  // get total xdr buffer size
  size_t xdr_buffer_size = dak_xdr_getpos(&xdrOutBuf);
  // write binary data to file
  write((char *)&outBuf, xdr_buffer_size);
  //Cout << "debug: ulong out = " << l << std::endl;
#else
  // Do native binary write of unsigned long
  write((char *)&l, sizeof(unsigned long));
#endif
  return *this;
}

} // namespace Dakota
