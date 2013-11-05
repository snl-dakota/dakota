/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DakotaBinStream
//- Description: This class reads and writes binary files by extending
//-              the basic iostream and writing in native binary
//-              format with Boost serialization
//- Owner:       Brian Adams
//- Version: $Id$ 

#ifndef DAKOTA_BINSTREAM_H
#define DAKOTA_BINSTREAM_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"  // for Cerr, write_precision
#include "dakota_data_types.hpp"

// forward declarations
namespace boost {
  namespace archive {
    class binary_oarchive;
    class binary_iarchive;
  }
}

namespace Dakota {

/// The binary input stream class.  Overloads the >> operator for all
/// data types

/** The Dakota::BiStream class is a binary input class which overloads
    the >> operator for all standard data types (int, char, float,
    etc). The class relies on the methods within the ifstream base
    class.  The Dakota::BiStream class inherits from the ifstream
    class.  The class utilizes Boost serialization platform-depdendent
    binary files.  TODO: Later we will extend to be portable so Dakota
    restart files can be moved from host to host. */
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

  /// Open a stream after default constructing
  /// Open a stream after default constructing
  void open(const char *filename, 
	    ios_base::openmode mode = ios_base::in | ios_base::binary);


  //
  //- Heading: operators
  //

  /// Input operator, reads string from binary stream BiStream
  BiStream& operator>>(std::string& ds);
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
  /// Input operator, reads unsigned char from binary stream BiStream
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

  /// Binary input archive from which data is read
  boost::archive::binary_iarchive *inputArchive;

};


/// The binary output stream class.  Overloads the << operator for all
/// data types

/** The Dakota::BoStream class is a binary output class which overloads
    the << operator for all standard data types (int, char, float,
    etc). The class relies on the methods within the ofstream base
    class.  The Dakota::BiStream class inherits from the ofstream
    class.  The class utilizes Boost serialization platform-depdendent
    binary files.  TODO: Later we will extend to be portable so Dakota
    restart files can be moved from host to host. */
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

  ~BoStream();

  /// Open a stream after default constructing
  void open(const char *filename, 
	    ios_base::openmode mode = ios_base::out | ios_base::binary);

  //
  //- Heading: operators 
  //

  /// Binary Output stream operator<<
  BoStream& operator<<(const std::string& ds);
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

  /// Binary output archive from which data is read
  boost::archive::binary_oarchive *outputArchive;

};

// TODO: make these handle Teuchos only and use boost serialization
// directly to serialize std types.

/// standard binary stream extraction operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(BiStream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArray& label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() )
    label_array.resize(boost::extents[len]);
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// standard binary stream extraction operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void read_data(BiStream& s,
	       Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
	       StringMultiArrayView label_array)
{
  OrdinalType i, len;
  s >> len;
  if( len != v.length() )
    v.sizeUninitialized(len);
  if( len != label_array.size() ) {
    Cerr << "Error: size of label_array in read_data(BiStream&) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  for (i=0; i<len; ++i)
    s >> v[i] >> label_array[i];
}


/// standard binary stream insertion operator for full
/// SerialDenseVector with labels
template <typename OrdinalType, typename ScalarType>
void write_data(BoStream& s,
		const Teuchos::SerialDenseVector<OrdinalType, ScalarType>& v,
		const StringMultiArray& label_array)
{
  OrdinalType i, len = v.length();
  if (label_array.size() != len) {
    Cerr << "Error: size of label_array in write_data(BoStream) does not "
	 << "equal length of SerialDenseVector." << std::endl;
    abort_handler(-1);
  }
  s << len;
  for (i=0; i<len; ++i)
    s << v[i] << label_array[i];
}


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
