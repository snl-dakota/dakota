/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  BinaryIO_Helper (for HDF5)
//-               Inspired by Wesley Fan / SCEPTRE team's SimpleNetcdfFile
//- Version: $Id$

#ifndef BINARY_IO_HELPER_HPP
#define BINARY_IO_HELPER_HPP

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"

//#include <boost/filesystem/operations.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/multi_array.hpp>

#include "hdf5.h"
#include "hdf5_hl.h"

#include <iostream>
#include <vector>
#include <string>

// WJB - ToDo: eliminate using directives in header files
using namespace boost::assign;


namespace Dakota
{

class BinaryStream_OpenFailure   {};
class BinaryStream_CreateFailure {};
class BinaryStream_CloseFailure  {};

//class BinaryStream_CreateDataSpaceFailure {};
class BinaryStream_StoreDataFailure    {};
class BinaryStream_GetDataFailure      {};
	
class BinaryStream_InvalidDataSpace    {};
//class BinaryStream_InvalidVariable   {};

	
template <typename T>
struct NativeDataTypes {};

template <>
struct NativeDataTypes<int>
{
  static hid_t datatype() { return H5T_NATIVE_INT; }
};

template <>
struct NativeDataTypes<double>
{
  static hid_t datatype() { return H5T_NATIVE_DOUBLE; }
};

struct DerivedStringType128
{
  static const size_t length() { return 128; }

  //static hid_t datatype()      { return H5T_STRING; }
  static hid_t datatype()
  { 
    // WJB:  is it a problem if the return val is incremented by each invocation?
    hid_t hidStrExtraBytes = H5Tcopy(H5T_C_S1);
    H5Tset_size(hidStrExtraBytes, 128);
    //H5Tset_strpad(hidStrExtraBytes, H5T_STR_NULLPAD);
    return hidStrExtraBytes;
  }

  class ExceedsMaxLengthException {};
};


class SimpleBinaryStream
{
public:

  /// File-based storage constructor
  SimpleBinaryStream(const  std::string& stream_file_name,
                     bool   file_stream_exist = true,
                     bool   read_only         = true,
                     size_t max_str_len       = DerivedStringType128::length(),
                     bool   exit_on_error     = true) :
    fileName(stream_file_name), binStreamId(),
    streamIsIncore(false), maxStringLength(max_str_len),
    exitOnError(exit_on_error), errorStatus()
  {
    // WJB - ToDo: split-out into .cpp file
    if ( file_stream_exist ) {
      if ( read_only ) {
        binStreamId = H5Fopen(fileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
      }
      else {
        binStreamId = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
      }

      if ( binStreamId < 0 ) {
        if ( exitOnError )
          throw BinaryStream_OpenFailure();
        //else  // WJB: don't really know what to do here?? but NeedToMv ON
          //errorStatus = status;

        // WJB:  don't see any viable "recovery" logic here, so should simplify code
      }
    }
    else {
      binStreamId = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
                               H5P_DEFAULT );
      if ( binStreamId < 0 ) {
        if ( exitOnError )
          throw BinaryStream_CreateFailure();
        //else // WJB: is there a way to "set" a GENERIC fileCreation ErrStatus?
          //errorStatus = status;
      }
    }

    if ( !errorStatus ) {
      // Use a derived HDF data type to support more typical Dakota string len
      if ( maxStringLength != DerivedStringType128::length() && exitOnError )
        throw BinaryStream_CreateFailure();
    }

    /* WJB: "Requirement" (BMA) to store the maxStrLen in the binary File?
    if ( !read_only && !errorStatus ) {
        //hid_t derivedH5T_StrSize; // store as a dataspace ??
        //
        if ( !addDim("MaxStringLength", maxStringLength, ds_id) ) {
          ;
        }
    } */
  }

#if 0
  /// Incore storage constructor (WJB: BAD - interface looks like default constructor)
  // perhaps no default is_incore param AND add a strParam for the initial group??
  SimpleBinaryStream(bool stream_is_incore    = true,
                     bool incore_stream_exist = true,
                     bool read_only           = true,
                     size_t max_str_len       = DerivedStringType128::length(),
                     bool exit_on_error       = true) :
    fileName(std::string()), binStreamId(),
    streamIsIncore(stream_is_incore), maxStringLength(max_str_len),
    errorStatus()
  { // WJB - ToDo: use a private method to "share" with file-based constructor }
#endif

  /// destructor
  ~SimpleBinaryStream()
  {
    if ( H5Fclose(binStreamId) < 0 )
      throw BinaryStream_CloseFailure();
  }


  // WJB: will a client to to query? -- hid_t binary_stream_id() const { return binStreamId; }

  //
  //- Heading:  Data storage methods (write HDF5)
  //
  
  /// Strings are weird in HDF5
  herr_t store_data(const std::string& dset_name,
                    const std::string& val) const
  {
    // Not really able to do std:;string, so leverage a DERIVED HDF5 string type
    if ( val.length() >= DerivedStringType128::length() && exitOnError )
      throw DerivedStringType128::ExceedsMaxLengthException();

    std::vector<hsize_t> dims;
    dims += 1;  // string is a 1D dataspace

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       1, dims.data(), DerivedStringType128::datatype(),
                       val.c_str() );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  /// ToDo: String2DArray

  /// StringArray
  herr_t store_data(const std::string& dset_name,
                    const StringArray& buf) const
  {
    if ( (buf.empty() || buf[0].length() >= DerivedStringType128::length())
          && exitOnError )
      throw DerivedStringType128::ExceedsMaxLengthException();

    const size_t num_strings = buf.size();
    std::vector<hsize_t> dims;

    // 1D since using a HDF5 derived Type within the container
    dims += num_strings;

    //return store_data<std::string, 1>( dset_name, dims, buf.data() );
    return store_data<1>( dset_name, dims, buf );
  }


  template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const T& val) const
  {
    /* WJB: no time time to dig-deeper, think of the data as an array of len==1
    hsize_t dims[1]={1};    
    herr_t ret_val = H5LTmake_dataset_<TYPE>( binStreamId, dset_name.c_str(),
                       1, dims, &val );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
		  
    return ret_val; */

    // tmp store value as the 0th entry in a vec prior to writing 1D dataspace
    std::vector<T> buf; buf.reserve(1); buf.push_back(val);

    std::vector<hsize_t> dims;
    dims += buf.size();

    return store_data<T,1>( dset_name, dims, buf.data() );
  }


  template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<T>& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    std::vector<hsize_t> dims;
    dims += buf.size(); // std::vector<T> is 1D dataspace

    return store_data<T,1>( dset_name, dims, buf.data() );
  }


  herr_t store_data(const std::string& dset_name,
                    const RealVector& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    std::vector<hsize_t> dims;
    dims += buf.length(); // RealVector is 1D dataspace

    return store_data<double,1>( dset_name, dims, buf.values() );
  }


  // should parameterize on ScalarType -- template <typename T>
  // that way, same func for ints, doubles,...
  herr_t store_data(const std::string& dset_name,
                    const RealMatrix& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    std::vector<hsize_t> dims;
    dims += buf.numRows(), buf.numCols(); // Matrix is 2D

    return store_data<double,2>( dset_name, dims, buf.values() );
  }


  herr_t store_data(const std::string& dset_name,
                    const RealVectorArray& buf) const
  {
    if ( buf.size() == 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    std::vector<hsize_t> dims;
    dims += buf.size(), buf[0].length(); // VectorArray is 2D

    // RECALL:  teuchos is a C++ library, but has fortran layout
    // WJB:     look into a Boost MultiArray here instead!
    RealMatrix tmp( dims[0], dims[1] );
    for(int i=0; i<dims[0]; ++i)
      for(int j=0; j<dims[1]; ++j)
        tmp(i, j) = buf[i][j];

    return store_data<double,2>( dset_name, dims, tmp.values() );
  }


  //
  //- Heading:  Data retrieval methods (read HDF5)
  //

  template <typename T, size_t DIM>
  herr_t read_data(const std::string& dset_name,
                   std::vector<T>& buf) const
  {
    //dbg_progress(binStreamId);
    // WJB: how to know what to size the dims vector??
    // H5LTget_dataset_ndims
    //      hardwire to have some success for now
    std::vector<hsize_t> dims( DIM, hsize_t(1) ); // see "accumulate" 9 lines down
    
    herr_t ret_val = H5LTget_dataset_info( binStreamId, dset_name.c_str(),
                       &dims[0], NULL, NULL );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    // WJB: need an stl alg similar to accumulate here
    buf.resize( dims[0]*dims[1] );

    ret_val = H5LTread_dataset( binStreamId, dset_name.c_str(),
                NativeDataTypes<T>::datatype(), &buf[0] );

    //output_status(ret_val);
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    return ret_val;
  }


  template <typename T>
  herr_t read_data(const std::string& dset_name, T& val) const
  {
#if 0
  //herr_t read_data(const std::string& dset_name, T* buf) const
    // WJB - ToDo:  As with singleVal WRITE step, need to NOT resort to "hack"
    //              of considering the val as the 0th entry in a std::vector

    std::vector<T> tmp_buf( 1, T(0) );

    herr_t ret_val = read_data<T,1>( dset_name.c_str(), tmp_buf );
    *val = tmp_buf[0];
    return ret_val;
#else

    return H5LTread_dataset( binStreamId, dset_name.c_str(),
             NativeDataTypes<T>::datatype(), &val);
#endif
  }


  herr_t read_data(const std::string& dset_name, std::string& buf) const
  {
    buf.reserve(maxStringLength);
    herr_t ret_val = H5LTread_dataset_string( binStreamId, dset_name.c_str(),
                       &buf[0] );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    return ret_val;
  }


private:

  template <size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const std::vector<std::string>& buf) const
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

#if 0
    // WJB: use a for loop and delegate each entry the to single string store?
    boost::multi_array<std::string, 1> tmp_buf( boost::extents[num_strings] );
    /* boost::multi_array<unsigned char, 2> tmp_buf(
      boost::extents[num_strings][DerivedStringType128::length()] ); */

    std::cout << "chk2ndString inArray: " << buf[1].c_str() << std::endl;

    for(int i=0; i<num_strings; ++i)
      tmp_buf[i] = buf[i];

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       dims.size(), dims.data(),
                       DerivedStringType128::datatype(), tmp_buf[0].c_str() );
                       //DerivedStringType128::datatype(), &buf[0] );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
#endif

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), DerivedStringType128::datatype(),
                       buf.data()->c_str() );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const T* buf) const
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), NativeDataTypes<T>::datatype(),
                       buf );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  //
  //- Heading:  Debugging utility methods
  //
  void output_status(herr_t status) const
  {
    std::cout << "BinaryDF return ErrStat (POSITIVE_val) " << status << std::endl;
  }


  //
  //- Heading: Data
  //

  /// file name of binary file stream - empty string if in-core
  std::string fileName;

  /// Binary stream ID
  hid_t binStreamId;

  /// Toggle for storage - default is false - true means store data in-core
  bool streamIsIncore; // WJB: enforce an empty string for fileName in this case

  /// Max string length for storing std::string as a type derived from H5T_C_S1
  const size_t maxStringLength;

  /// Toggle for exit vs continue execution - default is true
  bool exitOnError;

  /// In-class caching of the error status code (mostly for debugging)
  herr_t errorStatus;


  // WJB:  consider boost::ublas::bounded_vector type instead
  //static std::vector<hsize_t, 1> staticVectorDims;
  //static std::vector<hsize_t, 2> staticMatrixDims;
  //static std::vector<hsize_t, 3> static3DBufDims;

  //
  //- Heading:  DISABLED constructors and assignment operator
  //

  /// default constructor
  SimpleBinaryStream();
  /// copy constructor
  SimpleBinaryStream(const SimpleBinaryStream&);

  /// assignment operator
  SimpleBinaryStream& operator=(const SimpleBinaryStream&);

};


} // namespace Dakota

#endif // BINARY_IO_HELPER_HPP

