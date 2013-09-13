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

// WJB - ToDo: move this header file into Dakota source
#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"

//#include <boost/filesystem/operations.hpp>
#include <boost//assign/std/vector.hpp>

#include "hdf5.h"
#include "hdf5_hl.h"

#include <iostream>
#include <vector>
#include <string>

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
struct BuiltinDataTypes {};

template <>
struct BuiltinDataTypes<int>
{
  //static hid_t h5_type() { return H5T_NATIVE_INT; }
  static hid_t datatype() { return H5T_NATIVE_INT; }
};

template <>
struct BuiltinDataTypes<double>
{
  static hid_t datatype() { return H5T_NATIVE_DOUBLE; }
};


class SimpleBinaryStream
{
private:

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

  /// Toggle for exit vs continue execution - default is true
  bool exitOnError;

  /// In-class caching of the error status code (mostly for debugging)
  herr_t errorStatus;

public:

  /// File-based storage constructor
  SimpleBinaryStream(const std::string& stream_file_name,
                     bool file_stream_exist = true,
                     bool read_only         = true,
                     bool exit_on_error     = true) :
    fileName(stream_file_name), binStreamId(),
    streamIsIncore(false), exitOnError(exit_on_error),
    errorStatus()
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
      binStreamId = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
                              H5P_DEFAULT);
      if ( binStreamId < 0 ) {
        if ( exitOnError )
          throw BinaryStream_CreateFailure();
        //else // WJB: is there a way to "set" a GENERIC fileCreation ErrStatus?
          //errorStatus = status;
      }
    }

    // WJB: may be needed when I start tackling string data in HDF5
    //if ( !read_only && !errorStatus ) {
        int ds_id;
        /*
        if ( !addDim("MaxStringLength", maxStringLength, ds_id) ) {
          if ( exitOnError ) {
            throw BinaryStream_CreateDataSpaceFailure();
          }
          else
            errorStatus = status;
        } */
    //}
  }

#if 0
  /// Incore storage constructor (WJB: BAD - interface looks like default constructor)
  // perhaps no default is_incore param AND add a strParam for the initial group??
  SimpleBinaryStream(bool stream_is_incore    = true,
                     bool incore_stream_exist = true,
                     bool read_only           = true,
                     bool exit_on_error       = true) :
    fileName(std::string()), binStreamId(),
    streamIsIncore(stream_is_incore), exitOnError(exit_on_error),
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

  template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const T& val)
  {
    /* WJB: no time time to dig-deeper, think of the data as an array of len==1
    hsize_t dims[1]={1};    
    herr_t ret_val = H5LTmake_dataset_double( binStreamId, dset_name.c_str(),
                       1, dims, &val );
		  
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
		  
    return ret_val; */

    std::vector<hsize_t> dims(1, 1); // wrong constructor??
    std::vector<double>  buf(1, val);

    return store_data<T, 1>(dset_name, dims, buf);
  }


  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const T* buf)
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), BuiltinDataTypes<T>::datatype(),
                       buf );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const std::vector<T>& buf)
  {
#if 0
    // WJB: temporarily keep old impl available till I am confident
    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), BuiltinDataTypes<T>::datatype(),
                       buf.data() );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
#endif
    return store_data<T, DIM>(dset_name, dims, buf.data());
  }

  // should parameterize on ScalarType -- template <typename T>
  // that way, same func for ints, doubles,...
  herr_t store_data(const std::string& dset_name,
                    const RealMatrix& buf)
  {
    std::vector<hsize_t> dims; // Matrix is 2D
    dims += buf.numRows(), buf.numCols();

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       dims.size(), &dims[0],
                       BuiltinDataTypes<double>::datatype(),
                       &buf[0][0] );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }

  herr_t store_data(const std::string& dset_name,
                    const RealVectorArray& buf)
  {
    std::vector<hsize_t> dims; // VectorArray is 2D
    dims += buf.size(), buf[0].length();

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       dims.size(), &dims[0],
                       BuiltinDataTypes<double>::datatype(),
                       &buf[0] );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  template <typename T, size_t DIM>
  herr_t read_data(const std::string& dset_name,
                   std::vector<T>& buf)
  {
    //dbg_progress(binStreamId);
    // WJB: how to know what to size the dims vector??
    // H5LTget_dataset_ndims
    //      hardwire to have some success for now
    std::vector<hsize_t> dims(DIM);
    
    herr_t ret_val = H5LTget_dataset_info( binStreamId, dset_name.c_str(),
                       &dims[0], NULL, NULL );
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    // WJB: need an stl alg similar to accumulate here
    buf.resize( dims[0]*dims[1] );

    ret_val = H5LTread_dataset( binStreamId, dset_name.c_str(),
                BuiltinDataTypes<T>::datatype(), &buf[0] );

    //output_status(ret_val);
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    return ret_val;
  }


private:

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

