/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- class:       Implementation code for DakotaBinStream
//- Description: This class reads and writes binary files using Boost
//-              serialization by extending the basic iostream.
//-              Currently these use native binary formats which may
//-              not be platform portable.
//-
//- Owner:       Brian Adams
//- Version: $Id$ 


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "DakotaBinStream.hpp"

namespace Dakota {

// Binary Output Stream Methods

/** Default constructor. The open method must be called before stream
    can be read.  */
BiStream::BiStream(): inputArchive(NULL)
{
}

BiStream::BiStream(const char *filename): inputArchive(NULL)
{ 
  // call open method
  std::ifstream::open(filename); 
  // open the archive and associate with the stream  
  inputArchive = new boost::archive::binary_iarchive(*this);
}

BiStream::BiStream(const char *filename, std::ios_base::openmode mode): 
  inputArchive(NULL)
{ 
  // call open method with flags
  std::ifstream::open(filename, mode);
  // open the archive and associate with the stream
  inputArchive = new boost::archive::binary_iarchive(*this);
}

BiStream::~BiStream()
{
  if (inputArchive) {
    delete inputArchive;
    inputArchive = NULL;
  }
}

void BiStream::open(const char *filename, ios_base::openmode mode)
{
  if (inputArchive) {
    Cerr << "\nError: binary input stream already open, opening file "
	 << filename << std::endl;
    abort_handler(-1);
  }
  
  // call open method with flags
  std::ifstream::open(filename, mode);

  // open the archive and associate with the stream}
  inputArchive = new boost::archive::binary_iarchive(*this);
}

BiStream& BiStream::operator>>(std::string& ds)
{
  *inputArchive >> ds;
  return *this;
}

BiStream& BiStream::operator>>(char& c)
{    
  *inputArchive >> c;
  return *this;
}

BiStream& BiStream::operator>>(int& i)
{
  *inputArchive >> i;
  return *this;
}

BiStream& BiStream::operator>>(short& s)
{
  *inputArchive >> s;
  return *this;
}

BiStream& BiStream::operator>>(bool& b)
{
  *inputArchive >> b;
  return *this;
}

BiStream& BiStream::operator>>(double& d)
{
  *inputArchive >> d;
  return *this;
}

BiStream& BiStream::operator>>(float& f)
{
  *inputArchive >> f;
  return *this;
}

BiStream& BiStream::operator>>(long& l)
{
  *inputArchive >> l;
  return *this;
}

BiStream& BiStream::operator>>(unsigned char& c)
{
  *inputArchive >> c;
  return *this;
}

BiStream& BiStream::operator >> (unsigned int& i)
{
  *inputArchive >> i;
  return *this;
}

BiStream& BiStream::operator>>(unsigned short& s)
{
  *inputArchive >> s;
  return *this;
}

BiStream& BiStream::operator>>(unsigned long& l)
{
  *inputArchive >> l;
  return *this;
}


// Binary Output Stream Methods

/** Default constructor does not call the open() method.  The open()
    method must be called before stream can be written to.*/
BoStream::BoStream(): outputArchive(NULL)
{
  /* empty ctor */
}

BoStream::BoStream(const char *filename): outputArchive(NULL)
{ 
  // open a stream with the filename
  open(filename); 
  // open the archive and associate with the stream
  outputArchive = new boost::archive::binary_oarchive(*this);
}

BoStream::BoStream(const char *filename, std::ios_base::openmode mode): 
  outputArchive(NULL)
{ 
  open(filename, mode);
  // open the archive and associate with the stream
  outputArchive = new boost::archive::binary_oarchive(*this);
}

BoStream::~BoStream()
{
  if (outputArchive) {
    delete outputArchive;
    outputArchive = NULL;
  }
}

void BoStream::open(const char *filename, ios_base::openmode mode)
{
  if (outputArchive) {
    Cerr << "\nError: binary output stream already open, opening file "
	 << filename << std::endl;
    abort_handler(-1);
  }
  
  // call open method with flags
  std::ofstream::open(filename, mode);

  // open the archive and associate with the stream
  outputArchive = new boost::archive::binary_oarchive(*this);
}

BoStream& BoStream::operator<<(const std::string& ds)
{
  *outputArchive << ds;
  return *this;
}

BoStream& BoStream::operator<<(const char& c)
{
  *outputArchive << c;
  return *this;
}

BoStream& BoStream::operator<<(const int& i)
{
  *outputArchive << i;
  return *this;
}

BoStream& BoStream::operator<<(const short& s)
{
  *outputArchive << s;
  return *this;
}

BoStream& BoStream::operator<<(const bool& b)
{
  *outputArchive << b;
  return *this;
}

BoStream& BoStream::operator<<(const double& d)
{
  *outputArchive << d;
  return *this;
}

BoStream& BoStream::operator<<(const long& l)
{
  *outputArchive << l;
  return *this;
}


BoStream& BoStream::operator<<(const float& f)
{
  *outputArchive << f;
  return *this;
}

BoStream& BoStream::operator<<(const unsigned char& c)
{
  *outputArchive << c;
  return *this;
}

BoStream& BoStream::operator<<(const unsigned int& i)
{
  *outputArchive << i;
  return *this;
}

BoStream& BoStream::operator<<(const unsigned short& s)
{
  *outputArchive << s;
  return *this;
}

BoStream& BoStream::operator<<(const unsigned long& l)
{
  *outputArchive << l;
  return *this;
}

} // namespace Dakota
