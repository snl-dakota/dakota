/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DakotaBinStream
//- Description: A replacement for the RWbistream class.  This class reads
//-              and writes binary files by extending the basic iostream
//-		 if RPC/xdr is not supported then comment #ifdef out all 
//-		 xdr specific code and output in native binary format.
//- Owner:       
//- Version: $Id: 


#ifndef DAKOTA_BINSTREAM_H
#define DAKOTA_BINSTREAM_H

#include "system_defs.h"
#ifndef NO_XDR
#include "dak_xdr_interface.h"
#endif

namespace Dakota {

/// The binary input stream class.  Overloads the >> operator for all
/// data types

/** The Dakota::BiStream class is a  binary input class which overloads the >>
    operator for all standard data types (int, char, float, etc). The class 
    relies on the methods within the ifstream base class.
    The Dakota::BiStream class inherits from the ifstream class.
    If available, the class utilize rpc/xdr to construct machine independent 
    binary files.  These Dakota restart files can be moved from host to host.
    The motivation to develop these classes was to replace the Rogue
    wave classes which Dakota historically used for binary I/O.  */

class BiStream : public virtual std::ifstream
{
public:

  //
  //- Heading: Constructors, destructor
  //

  /// Default constructor, need to open
  BiStream();
  /// Constructor takes name of input file
  BiStream(const char *s);
  /// Constructor takes name of input file, mode
  BiStream(const char *s, std::ios_base::openmode mode);

  /// Destructor, calls xdr_destroy to delete xdr stream
  ~BiStream();

  //
  //- Heading: operators
  //

  /// Binary Input stream operator>>
  BiStream& operator>>(std::string& ds);
  /// Input operator, reads char* from binary stream BiStream
  BiStream& operator>>(char* s);
  /// Input operator, reads char from binary stream BiStream
  BiStream& operator>>(char& c);
  /// Input operator, reads int* from binary stream BiStream
  BiStream& operator>>(int& i);
  /// Input operator, reads long from binary stream BiStream
  BiStream& operator>>(long& l);
  /// Input operator, reads short from binary stream BiStream
  BiStream& operator>>(short& s);
  /// Input operator, reads bool from binary stream BiStream
  BiStream& operator>>(bool& b);
  /// Input operator, reads double from binary stream BiStream
  BiStream& operator>>(double& d);
  /// Input operator, reads float from binary stream BiStream
  BiStream& operator>>(float& f);
  /// Input operator, reads unsigned char* from binary stream BiStream
  BiStream& operator>>(unsigned char& c);
  /// Input operator, reads unsigned int from binary stream BiStream
  BiStream& operator>>(unsigned int& i);
  /// Input operator, reads unsigned long from binary stream BiStream
  BiStream& operator>>(unsigned long& l);
  /// Input operator, reads unsigned short from binary stream BiStream
  BiStream& operator>>(unsigned short& s);

private:

  //
  //- Heading : Private data members
  //

  /// XDR input stream buffer
#ifndef NO_XDR
  XDR xdrInBuf;
  /// Buffer to hold data as it is read in
  char inBuf[MAX_NETOBJ_SZ];
#endif
};


/// The binary output stream class.  Overloads the << operator for all
/// data types

/** The Dakota::BoStream class is a binary output classes which
    overloads the << operator for all standard data types (int, char,
    float, etc). The class relies on the built in write methods
    within the ostream base classes.  Dakota::BoStream inherits
    from the ofstream class.  The motivation to develop this class was
    to replace the Rogue wave class which Dakota historically used for
    binary I/O.  If available, the class utilize rpc/xdr to construct machine 
    independent binary files. These Dakota restart files can be moved 
    between hosts. */

class BoStream : public virtual std::ofstream
{
public:

  //
  //- Heading: Constructors, destructor
  //

  /// Default constructor, need to open
  BoStream();
  /// Constructor takes name of input file
  BoStream(const char *s);
  /// Constructor takes name of input file, mode
  BoStream(const char *s, std::ios_base::openmode mode);

  /// Destructor, calls xdr_destroy to delete xdr stream
  ~BoStream() throw();

  //
  //- Heading: operators 
  //

  /// Binary Output stream operator<<
  BoStream& operator<<(const std::string& ds);
  /// Output operator, writes char* TO binary stream BoStream
  BoStream& operator<<(const char* s);
  /// Output operator, writes char to binary stream BoStream
  BoStream& operator<<(const char& c);
  /// Output operator, writes int to binary stream BoStream
  BoStream& operator<<(const int& i);
  /// Output operator, writes long to binary stream BoStream
  BoStream& operator<<(const long& l);
  /// Output operator, writes short to binary stream BoStream
  BoStream& operator<<(const short& s);
  /// Output operator, writes bool to binary stream BoStream
  BoStream& operator<<(const bool& b);
  /// Output operator, writes double to binary stream BoStream
  BoStream& operator<<(const double& d);
  /// Output operator, writes float to binary stream BoStream
  BoStream& operator<<(const float& f);
  /// Output operator, writes unsigned char to binary stream BoStream
  BoStream& operator<<(const unsigned char& c);
  /// Output operator, writes unsigned int to binary stream BoStream
  BoStream& operator<<(const unsigned int& i);
  /// Output operator, writes unsigned long to binary stream BoStream
  BoStream& operator<<(const unsigned long& l);
  /// Output operator, writes unsigned short to binary stream BoStream
  BoStream& operator<<(const unsigned short& s);

private:

  //
  //- Heading : Private data members
  //

  /// XDR output stream buffer
#ifndef NO_XDR
  XDR xdrOutBuf;
  /// Buffer to hold converted data before it is written
  char outBuf[MAX_NETOBJ_SZ];
#endif
};


/// Read an array from BiStream, s
template <class ArrayT>
inline void array_read(BiStream& s, ArrayT& v)
{
  typename ArrayT::size_type len;
  s >> len;
  v.resize(len);
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s >> v[i];
}


/// Write an array to BoStream, s
template <class ArrayT>
inline void array_write(BoStream& s, const ArrayT& v)
{
  typename ArrayT::size_type len = v.size();
  s << len;
  for (register typename ArrayT::size_type i=0; i<len; ++i)
    s << v[i];
}


/// global BiStream extraction operator for generic "array" container
template <class ArrayT>
inline BiStream& operator>>(BiStream& s, ArrayT& data)
{ array_read(s, data); return s; }

/// global BoStream insertion operator for generic "array" container
template <class ArrayT>
inline BoStream& operator<<(BoStream& s, const ArrayT& data)
{ array_write(s, data); return s; }

} // namespace Dakota

#endif
