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
//#include <boost/multi_array.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

#include "hdf5.h"
#include "hdf5_hl.h"

#include <iostream>
#include <limits>
#include <cmath>
#include <string>
#include <vector>


namespace Dakota
{

class BinaryStream_OpenFailure   {};
class BinaryStream_CreateFailure {};
class BinaryStream_CloseFailure  {};

class BinaryStream_CreateGroupFailure {};
//class BinaryStream_CreateDataSpaceFailure {};
class BinaryStream_StoreDataFailure    {};
class BinaryStream_GetDataFailure      {};
	
class BinaryStream_InvalidPath         {};
class BinaryStream_InvalidDataSpace    {};
//class BinaryStream_InvalidVariable   {};


template <typename T>
struct NativeDataTypes {};

template <>
struct NativeDataTypes<int>
{
  typedef int PrimitiveType;

  static hid_t datatype() { return H5T_NATIVE_INT; }
  //static PrimitiveType get_fill_value() { return nan(""); }
  static const PrimitiveType get_fill_value()
  { return std::numeric_limits<PrimitiveType>::min(); }
};

template <>
struct NativeDataTypes<double>
{
  typedef double PrimitiveType;

  static hid_t datatype() { return H5T_NATIVE_DOUBLE; }
  static const PrimitiveType get_fill_value() { return nan(""); }
};


// Monostate pattern - really more of a "wrapper" around C API here
class H5VariableString
{
  // Nesting within HDF5BinaryStream probably makes sense in this case
  //friend class HDF5Client; ? should the client be the object allowed to close the hid_t resource for the variable string type?

public:

  static hid_t datatype() { return create(); }
    
private:

  static hid_t create()
  { 
    //if (varStringType == -1) {
    if (numVStrUses == 0) {
      // H5Tcopy not called yet, so do it now (only once)
      varStringType = H5Tcopy(H5T_C_S1);
      H5Tset_size(varStringType, H5T_VARIABLE);
      if (varStringType < 0)
	throw std::string("H5Tcreate error");

    }

    ++numVStrUses;
    return varStringType;
  }

  // not sure if this type is immutable; if so, can't close it
  /// release the type
  static void destroy(hid_t var_str_hid) { 
    assert(var_str_hid == varStringType); // WJB: do away with var_str_hid after some testing
    H5Tclose(varStringType); 
    // typeInitialized = false;
    //assert(numVStrUses == 1); // WJB: do away with numVStrUses after some testing
  }


  // could consider just using varStringType < 0 as uninit 

  /// count the number of initializations (eventually, do away with this cache?)
  static short numVStrUses;
  /// the static type for this string type
  static hid_t varStringType;

  //
  //- Heading:  DISABLED constructors and assignment operator
  //

  /// default constructor
  H5VariableString();
  /// copy constructor
  H5VariableString(const H5VariableString&);

  /// assignment operator
  H5VariableString& operator=(const H5VariableString&);

};


class HDF5BinaryStream
{
public:

  /// Default constructor (evaluate whether sensible - default params??)
  HDF5BinaryStream(const std::string& file_name = "dak_db_persist.h5",
                     bool db_is_incore      = true,
                     bool file_stream_exist = true,
                     bool writable_fstream  = false,
                     bool exit_on_error     = true) :
    fileName(file_name), binStreamId(),
    dbIsIncore(db_is_incore), exitOnError(exit_on_error), errorStatus()
  {
    // WJB - ToDo: split-out into .cpp file
    if ( db_is_incore ) {
      // CORE driver - DB data store is persisted to file at end
      //http://www.mail-archive.com/hdf-forum@hdfgroup.org/msg00660.html

      hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
      bool persist = writable_fstream;
      if ( H5Pset_fapl_core(fapl_id, 4096, persist) && exitOnError)
        throw BinaryStream_CreateFailure();

      binStreamId = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
                               fapl_id );

      if ( binStreamId < 0 ) {
        if ( exitOnError )
          throw BinaryStream_CreateFailure();
        //else
          //errorStatus = status;
      }

      // WJB - ToDo: look into potential resource leak
      //H5Pclose(fapl_id);
    }

    else if ( file_stream_exist ) {
      if ( writable_fstream ) {
        binStreamId = H5Fopen(fileName.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
      }
      else {
        // open file stream as read-only
        binStreamId = H5Fopen(fileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
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

  }


  /// destructor
  ~HDF5BinaryStream()
  {
    // WJB: need to be "clever" and destroy when ALL HDF5BinaryStream objects
    // are no longer in scope (role of the HDF5 client?)
    // versus
    // allow each HDF5BinaryStream to create/destroy the "one" varStringHid
    // that it "owns"
    //H5VariableString::destroy(varStringHid);

    if ( H5Fclose(binStreamId) < 0 )
      throw BinaryStream_CloseFailure();
  }


  // WJB: will a client need to query? -- hid_t binary_stream_id() const { return binStreamId; }

  //
  //- Heading:  Data storage methods (write HDF5)
  //

  void check_error_store(herr_t ret_val) const
  {
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
  }

 
  /// String value stored in HDF5
  herr_t store_data(const std::string& dset_name,
                    const std::string& val) const
  {
    create_groups(dset_name);

    herr_t ret_val = H5LTmake_dataset_string(binStreamId, dset_name.c_str(),
					     val.c_str());
    check_error_store(ret_val);

    return ret_val;
  }

  /// Store a StringArray where each string is variable length (Merge
  /// of h5ex_t_vlstring.c (relies on contiguous string storage) and
  /// h5ex_t_vlen.c (doesn't leverage VL string))
  herr_t store_data(const std::string& dset_name,
                    const StringArray& buf) const
  {
    create_groups(dset_name);

    // store a 1-D array of strings
    int rank = 1;
    std::vector<hsize_t> dims(rank);
    dims[0] = buf.size();
    
    // array of pointers to each null-terminated string
    std::vector<const char *> wdata(buf.size());
    for (size_t i=0; i<buf.size(); ++i)
      wdata[i] = buf[i].c_str();

    herr_t status = 
      H5LTmake_dataset(binStreamId, dset_name.c_str(), rank, dims.data(),
		       H5VariableString::datatype(), wdata.data());

    check_error_store(status);

    return (status);
  }


  template <typename T>
  herr_t store_data(const std::string& dset_name, const T& val) const
  {
    hsize_t dims[1] = {1};    
    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(), 1, dims,
                       NativeDataTypes<T>::datatype(), &val );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
		  
    return ret_val;
  }


  template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<T>& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // std::vector<T> is a 1D dataspace
    return store_data<T,1>( dset_name,
                            boost::assign::list_of( buf.size() ),
                            buf.data() );
  }


  herr_t store_data(const std::string& dset_name,
                    const RealVector& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // RealVector is a 1D dataspace
    return store_data<double,1>( dset_name,
                                 boost::assign::list_of( buf.length() ),
                                 buf.values() );
  }


  /// storage of ragged array of vector<T> - each "row" can be variable length
  /// for example vector< vector<int> >, vector< vector<double> >
  template <typename T>
  herr_t store_vl_data(const std::string& dset_name,
                       const std::vector<std::vector<T> >& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB hmmm -- is this tested? return store_data<T,1>(dset_name, dims, buf);
    // Surprisingly, std::vector< std::vector<T> > is STILL 1D dataspace in HDF5
    return store_data<T,1>( dset_name,
                            boost::assign::list_of( buf.size() ),
                            buf );
  }


  /// RECTANGULAR storage of ragged array of vector<T> - each "row" can be
  /// variable length BUT a hyperslab is selected for writing data, row-by-row
  /// (HDF5's fill_value used to fill empty locations in the rectangular space)
  /// for example vector< vector<int> >, vector< vector<double> >
  template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<std::vector<T> >& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // Create a 2D dataspace sufficient to store first row vector
    std::vector<hsize_t> dims(2);
    dims[0] = buf.size(); dims[1] = buf[0].size();

    std::vector<hsize_t> max_dims = initializeUnlimitedDims<2>();

    // Create the memory space...
    hid_t mem_space = H5Screate_simple( dims.size(), dims.data(),
                        max_dims.data() );

    // Enable chunking using creation properties
    // Chunk size is that of the longest row vector
    size_t num_cols = dims[1];
    for(int i=0; i<buf.size(); ++i) {
      num_cols = std::max( num_cols, buf[i].size() );  
    }

    std::vector<hsize_t> chunk_dims = dims;
    chunk_dims[0] = 1; chunk_dims[1] = num_cols;

    T filler = NativeDataTypes<T>::get_fill_value();
    hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);

    H5Pset_fill_value(cparms, NativeDataTypes<T>::datatype(), &filler);
    H5Pset_layout(cparms, H5D_CHUNKED);
    H5Pset_chunk( cparms, chunk_dims.size(), chunk_dims.data() );

    // Create a new dataset within the DB using cparms creation properties.

    hid_t dataset = H5Dcreate(binStreamId, dset_name.c_str(),
                      NativeDataTypes<T>::datatype(), mem_space, H5P_DEFAULT,
                      cparms, H5P_DEFAULT);

    herr_t status;
    for(int i=0; i<buf.size(); ++i) {
      status = append_data_slab( dset_name, i, buf[i].data(), buf[i].size() );
    }

    // WJB - ToDo: free resources
    H5Sclose(mem_space);

  }


  template <typename T>
  herr_t append_data_slab(const std::string& dset_name,
                          const std::vector<T>& buf) const
  {
    hsize_t last_row = find_last_dataset_record(dset_name);
    return append_data_slab( dset_name, last_row, buf.data(), buf.size() );
  }

  // WJB: nearly a DUPLICATE version of above code for Teuchos row vectors
  //template <typename OrdinalType, typename ScalarType>
  herr_t append_data_slab(const std::string& dset_name,
                          const RealVector& buf) const
  {
    hsize_t last_row = find_last_dataset_record(dset_name);
    return append_data_slab( dset_name, last_row, &buf[0], buf.length() );
  }

  // should parameterize on ScalarType -- template <typename T>
  // that way, same func for ints, doubles,...
  herr_t store_data(const std::string& dset_name,
                    const RealMatrix& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // Matrix is 2D
    return store_data<double,2>( dset_name,
             boost::assign::list_of( buf.numRows() )( buf.numCols() ),
             buf.values() );
  }



  herr_t store_data(const std::string& dset_name,
                    const RealVectorArray& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: much of the "chunking setup" code is nearly identical to the
    // std::vector version, so re-factor for reuse next sprint

    // Create a 2D dataspace sufficient to store first row vector
    std::vector<hsize_t> dims(2);
    dims[0] = buf.size(); dims[1] = buf[0].length(); // WJB: fcnPtr vs PassIn?

    std::vector<hsize_t> max_dims = initializeUnlimitedDims<2>();

    // Create the memory space...
    hid_t mem_space = H5Screate_simple( dims.size(), dims.data(),
                        max_dims.data() );

    // Enable chunking using creation properties
    // Chunk size is that of the longest row vector
    int num_cols = dims[1];
    for(int i=0; i<buf.size(); ++i) {
      num_cols = std::max( num_cols, buf[i].length() ); // WJB: "generic" possible?
    }

    std::vector<hsize_t> chunk_dims = dims;
    chunk_dims[0] = 1; chunk_dims[1] = num_cols;

    hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_layout(cparms, H5D_CHUNKED);
    H5Pset_chunk( cparms, chunk_dims.size(), chunk_dims.data() );

    // Create a new dataset within the DB using cparms creation properties.

    // size() vs length() and 'size_type' vs 'ScalarType' prevents writing a
    // generic algorithm, common to both std::vector and Teuchos vector
    hid_t dataset = H5Dcreate(binStreamId, dset_name.c_str(),
                      NativeDataTypes<double>::datatype(), mem_space,
                      H5P_DEFAULT, cparms, H5P_DEFAULT);

    herr_t status;
    for(int i=0; i<dims[0]; ++i) {
      std::vector<double> tmp( dims[1] );
      for(int j=0; j<dims[1]; ++j)
        tmp[j] = buf[i][j];

      status = append_data_slab( dset_name, i, tmp.data(), buf[i].length() );
    }

    // WJB - ToDo: free resources
    H5Sclose(mem_space);
    //std::cout << std::endl; 
  }


  /// RECTANGULAR storage of array of Teuchos Matrix objects - each "slab" can
  /// have variable Mrows x Ncols, but a hyperslab is selected for writing data,
  /// one matrix "slab" at a time (HDF5's fill_value used to fill empty
  /// locations in the rectangular space).
  /// for example vector<RealMatrix>, vector<IntMatrix>
  //template <typename T>
  herr_t store_data(const std::string& dset_name,
                    const RealMatrixArray& buf) const
  {
    if ( buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: much of the "chunking setup" code is nearly identical to the
    // Teuchos "row" VectorSlab version, so re-factor for reuse THIS sprint

    // Create a 3D dataspace sufficient to store first matrix slab
    std::vector<hsize_t> dims(3);
    dims[0] = buf.size(); dims[1] = buf[0].numRows(); dims[2] = buf[0].numCols();

    std::vector<hsize_t> max_dims = initializeUnlimitedDims<3>();

    // Create the memory space...
    hid_t mem_space = H5Screate_simple( dims.size(), dims.data(),
                        max_dims.data() );

    // Enable chunking using creation properties
    // Chunk size is that of the largest matrix slab
    int slab_size = dims[1]*dims[2];
    for(int i=0; i<buf.size(); ++i) {
      slab_size = std::max( slab_size, buf[i].numRows()*buf[i].numCols() );
    }

    std::vector<hsize_t> chunk_dims = dims;
    chunk_dims[0] = 1; chunk_dims[1] = slab_size/2; chunk_dims[2] = slab_size/2;

    hid_t cparms = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_layout(cparms, H5D_CHUNKED);
    H5Pset_chunk( cparms, chunk_dims.size(), chunk_dims.data() );

    // Create a new dataset within the DB using cparms creation properties.

    // size() vs length() and 'size_type' vs 'ScalarType' prevents writing a
    // generic algorithm
    hid_t dataset = H5Dcreate(binStreamId, dset_name.c_str(),
                      NativeDataTypes<double>::datatype(), mem_space,
                      H5P_DEFAULT, cparms, H5P_DEFAULT);

    herr_t status;
    for(int i=0; i<buf.size(); ++i) {
      //status = append_data_slab( dset_name, i, buf[i].values(),
                                 //buf[i].numRows()*buf[i].numCols() );
      status = append_data_slab<RealMatrixArray::value_type::scalarType>( dset_name, i, buf[0]);
    }

    // WJB - ToDo: free resources
    H5Sclose(mem_space);

  }


  // WJB: nearly a DUPLICATE version of above code for Teuchos Matrix Slabs
  //template <typename OrdinalType, typename ScalarType>
  herr_t append_data_slab(const std::string& dset_name,
                          const RealMatrix& buf) const
  {
    hsize_t last_slab = find_last_dataset_record(dset_name);
    //return append_data_slab( dset_name, last_slab, buf.values(),
                             //buf.numRows()*buf.numCols() );
    return append_data_slab<RealMatrixArray::value_type::scalarType>( dset_name,
             last_slab, buf );
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


  // BMA TODO: extract the common core from these

  herr_t read_data(const std::string& dset_name, std::string& buf) const
  {
    htri_t path_valid = H5LTpath_valid(binStreamId, dset_name.c_str(), 1);
    if ( path_valid != 1 && exitOnError )
      throw BinaryStream_InvalidPath();

    // get the number of dimensions
    int rank  = 0;
    herr_t dim_stat = 
      H5LTget_dataset_ndims(binStreamId, dset_name.c_str(), &rank);
    // strings should have rank 0
    if ( (dim_stat < 0 || rank !=0 ) && exitOnError)
      throw BinaryStream_GetDataFailure();

    // read the dimensions
    size_t      bytes = 0;
    H5T_class_t dt_class;
    hsize_t ndims[1] = {0};
    herr_t size_stat = 
      H5LTget_dataset_info(binStreamId, dset_name.c_str(), ndims, &dt_class, 
			   &bytes);
    if (size_stat < 0 && exitOnError)
      throw BinaryStream_GetDataFailure();

    // make vector of bytes to store string
    std::vector<char> vec_char(bytes);

    // get the string
    herr_t ret_val = H5LTread_dataset_string(binStreamId, dset_name.c_str(),
					       &vec_char[0]);
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_GetDataFailure();

    // set the string
    buf = &vec_char[0];

    return ret_val;
  }

  herr_t read_data(const std::string& dset_name, 
		   StringArray& string_array)
  {
    htri_t path_valid = H5LTpath_valid(binStreamId, dset_name.c_str(), 1);
    if ( path_valid != 1 && exitOnError )
      throw BinaryStream_InvalidPath();

    // get the number of dimensions of the StringArray (should be 1)
    int rank = 0;
    herr_t dim_stat = 
      H5LTget_dataset_ndims(binStreamId, dset_name.c_str(), &rank);
    if ( (dim_stat < 0 || rank != 1 ) && exitOnError)
      throw BinaryStream_GetDataFailure();

    // read the number of strings
    size_t total_bytes = 0;
    H5T_class_t dt_class;
    hsize_t dims[1] = {0};
    herr_t size_stat = 
      H5LTget_dataset_info(binStreamId, dset_name.c_str(), dims, &dt_class, 
			   &total_bytes);
    if (size_stat < 0 && exitOnError)
      throw BinaryStream_GetDataFailure();

    size_t num_str = dims[0];
    string_array.resize(num_str);

    for (size_t i=0; i<num_str; ++i) {
      // TODO: read each string from the HDF5 sharing core with above reader
      ;
    }

  }



private:

  /// storage of ragged array of vector<T> - each "row" can be variable length
  /// for example vector<vector<int>>, vector<vector<real>>,
  /// WJB - ToDo: consider support for vector<IntVector>,vector<RealVector>
  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const std::vector<std::vector<T> >& buf) const
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

    const size_t num_rows = buf.size();

    // WJB - ToDo:  better group "management" (Evans uses a stack)
    create_groups(dset_name);

    std::vector<hvl_t> wdata;

    for (size_t i=0; i<num_rows; ++i) {
      hvl_t row_data;
      row_data.len = buf[i].size();
      row_data.p   = (void*)buf[i].data();
      wdata.push_back(row_data);
    }

    // WJB - ToDo:  evaluate necessity of BOTH filetype AND memtype
    //              (versus re-use of same hid_t for use cases below)
    hid_t filetype = H5Tvlen_create( NativeDataTypes<T>::datatype() );
    hid_t memtype  = H5Tvlen_create( NativeDataTypes<T>::datatype() );

    // Create dataspace.  Setting maximum size to NULL sets the maximum
    hid_t space = H5Screate_simple( 1, dims.data(), NULL );

    // Create the dataset and write the variable-length data to it.
    hid_t dset = H5Dcreate( binStreamId, dset_name.c_str(), filetype, space,
                   H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    herr_t status = H5Dwrite( dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                      wdata.data() );

    if ( status < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: need better resource "management" (invoke close funcs at proper time)
    return (status);
  }


  /// storage of contiguous array of T, potentially as a multi-array with dims
  /// for example vector<int>, vector<real>, IntVector, RealVector, RealMatrix
  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const T* buf) const
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

// WJB: need better group "management" (Evans uses a stack)
    create_groups(dset_name);

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), NativeDataTypes<T>::datatype(),
                       buf );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  /** Check whether a dataset exists (lookup by name) and return the index
      of the last entry; i.e. the dims[0] of its corresponding dataspace
      ("fortran-style" indexing is assumed since easier with HDF5 queries). */
  hsize_t find_last_dataset_record(const std::string& dset_name) const
  {
    htri_t ds_exists = H5Lexists(binStreamId, dset_name.c_str(), H5P_DEFAULT);

    if ( !ds_exists && exitOnError )
      throw BinaryStream_StoreDataFailure();

    int ndims;
    herr_t status = H5LTget_dataset_ndims(binStreamId, dset_name.c_str(),
                      &ndims);

    std::vector<hsize_t> dims( ndims, hsize_t(1) );
    status = H5LTget_dataset_info(binStreamId, dset_name.c_str(), &dims[0],
               NULL, NULL);
    return dims[0];
  }


    // WJB:  consider functor approach once I have more confidence
    //std::for_each( buf.begin(), buf.end(),
      //StoreRowVector(dataset, NativeDataTypes<T>::datatype(), dataspace) );

  template <typename T>
  herr_t append_data_slab(const std::string& dset_name, const size_t row_index,
                          const T* buf, size_t buf_len) const
  {
    htri_t ds_exists = H5Lexists(binStreamId, dset_name.c_str(), H5P_DEFAULT);

    if ( !ds_exists || buf == NULL && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: IF contiguous "record" buf -- extending beyond 2D should be doable
    std::vector<hsize_t> max_dims = initializeUnlimitedDims<2>();
    std::vector<hsize_t> curr_dims(max_dims);

    herr_t status = H5LTget_dataset_info( binStreamId, dset_name.c_str(),
                      &curr_dims[0], NULL, NULL );

    //hsize_t last_index = find_last_dataset_record(dset_name);
    size_t last_index = row_index;

    //if (curr_dims[0] != 1) assert(last_index==curr_dims[0]); // NOT firstRow!

    // Extend the dataset by 1 row with num_cols increased if necessary
    std::vector<hsize_t> dims(max_dims);
    dims[0] = last_index + 1;
    dims[1] = (buf_len > curr_dims[1]) ? buf_len : curr_dims[1];

    hid_t dataset = H5Dopen(binStreamId, dset_name.c_str(), H5P_DEFAULT);
    status = H5Dextend( dataset, dims.data() );

    std::vector<hsize_t> record_dims(max_dims);
    record_dims[0] = 1; record_dims[1] = buf_len;

    /* WJB:  Examples show mem_space re-created
    // Logical, but shouldn't we query for the current mem_space and extend??
    status = H5Sset_extent_simple( mem_space, record_dims.size(),
               record_dims.data(), max_dims.data() );
    // NOT sure I like the re-creation idea, but if it works, does it matter? */
    hid_t mem_space = H5Screate_simple( record_dims.size(), record_dims.data(),
                        max_dims.data() );

    hid_t db_stream_space = H5Dget_space(dataset);

    // Select a row-record hyperslab...
    std::vector<hsize_t> offset(max_dims);
    offset[0] = last_index; offset[1] = 0;

    status = H5Sselect_hyperslab( db_stream_space, H5S_SELECT_SET,
               offset.data(), NULL, record_dims.data(), NULL );

    // Write the row data record to the hyperslab
    status = H5Dwrite( dataset, NativeDataTypes<T>::datatype(), mem_space,
               db_stream_space, H5P_DEFAULT, buf );

    H5Sclose(db_stream_space);
    H5Sclose(mem_space);

    return status;
  }


  template <typename T>
  herr_t append_data_slab(const std::string& dset_name,const size_t outer_index,
                          const RealMatrix& buf) const
  {
    htri_t ds_exists = H5Lexists(binStreamId, dset_name.c_str(), H5P_DEFAULT);

    if ( !ds_exists || buf.empty() && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB: IF contiguous "record" buf - this "3D" version seems doable
    std::vector<hsize_t> max_dims = initializeUnlimitedDims<3>();
    std::vector<hsize_t> curr_dims(max_dims);

    herr_t status = H5LTget_dataset_info( binStreamId, dset_name.c_str(),
                      &curr_dims[0], NULL, NULL );

    //hsize_t last_index = find_last_dataset_record(dset_name);
    size_t last_index = outer_index;

    //if (curr_dims[0] != 1) assert(last_index==curr_dims[0]); // NOT firstMatrixSlice!

    // Extend the dataset by a single matrix-slab with Nrows x Ncols increased
    // if necessary
    std::vector<hsize_t> dims(max_dims);
    dims[0] = last_index + 1;

    //OrdinalType n_rows, n_cols;
    int n_rows = buf.numRows();
    int n_cols = buf.numCols();
    dims[1] = (n_rows > curr_dims[1]) ? n_rows : curr_dims[1];
    dims[2] = (n_cols > curr_dims[2]) ? n_cols : curr_dims[2];

    hid_t dataset = H5Dopen(binStreamId, dset_name.c_str(), H5P_DEFAULT);
    status = H5Dextend( dataset, dims.data() );

    std::vector<hsize_t> record_dims(max_dims); // WJB: slab_dims might be better name now
    record_dims[0] = 1; record_dims[1] = n_rows; record_dims[2] = n_cols;


    hid_t mem_space = H5Screate_simple( record_dims.size(), record_dims.data(),
                        max_dims.data() );

    hid_t db_stream_space = H5Dget_space(dataset);

    // Select a matrix-slab (i.e. HDF5 hyperslab)...
    std::vector<hsize_t> offset(max_dims);
    offset[0] = last_index; offset[1] = 0; offset[2] = 0;

    status = H5Sselect_hyperslab( db_stream_space, H5S_SELECT_SET,
               offset.data(), NULL, record_dims.data(), NULL );

    // Write the matrix data to the hyperslab
    status = H5Dwrite( dataset, NativeDataTypes<T>::datatype(), mem_space,
               db_stream_space, H5P_DEFAULT, buf.values() );

    H5Sclose(db_stream_space);
    H5Sclose(mem_space);

    return status;
  }


  /** Assume we have an absolute path /root/dir/dataset and create
      groups /root/ and /root/dir/ if needed */
  void create_groups(const std::string& dset_name) const {

    // the first group will be empty due to leading delimiter
    // the last group will be the dataset name
    std::vector<std::string> groups;
    boost::split(groups, dset_name, boost::is_any_of("/"));

    // index instead of pruning first and last or clever iterators
    std::string full_path;
    for(size_t i=1; i<(groups.size()-1); ++i) {
      
      full_path += '/' + groups[i];

      // if doesn't exist, add
      //herr_t status = H5Eset_auto(NULL, NULL);
      //      herr_t status = H5Gget_objinfo(binStreamId, full_path.c_str(), 0, NULL);
      // herr_t status = 
      // 	H5Gget_info_by_name(binStreamId, full_path.c_str(), NULL, H5P_DEFAULT)
	;
      //      htri_t grpexists = H5Lexists( hid_t loc_id, const char *name, hid_t lapl_id ) 

      htri_t grpexists = 
	H5Lexists(binStreamId, full_path.c_str(), H5P_DEFAULT);
      if (grpexists == 0) {
	hid_t create_status = 
	  H5Gcreate(binStreamId, full_path.c_str(), 
		    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        if (create_status < 0)
          throw BinaryStream_CreateGroupFailure();

	// I think needed to avoid resource leaks:
	H5Gclose(create_status);
      }
      else if (grpexists < 0) {
        throw BinaryStream_StoreDataFailure();
      }

    }
    

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

  /// File name of binary file stream for persisting DB data store
  std::string fileName;

  /// Binary stream ID
  hid_t binStreamId;

  /// Toggle for storage - default is true, i.e. store DB in-core
  bool dbIsIncore;

  /// hid_t of the variable-length string_type derived from H5T_C_S1
  //  WJB: maybe one variable length string creation PER HDF5 stream object is the sensible way to manage the resource?
  //const hid_t varStringHid; // WJB: "feels redundant" -- refactor ASAP

  /// Toggle for exit vs continue execution - default is true
  bool exitOnError;

  /// In-class caching of the error status code (mostly for debugging)
  herr_t errorStatus;


  // WJB:  consider boost::ublas::bounded_vector type instead
  //static std::vector<hsize_t, 1> staticVectorDims;

//    boost::array<hsize_t, DIM> max_dims_nd;
//    max_dims_nd.fill(H5S_UNLIMITED);

  template <size_t DIM>
  static std::vector<hsize_t> initializeUnlimitedDims()
  {
    const static std::vector<hsize_t> max_dims_nd(DIM, H5S_UNLIMITED);
    return max_dims_nd;
  }

};


// stub so storing vector<RealMatrix> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<RealMatrix>& buf) const;

// stub so storing vector<IntVector> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<IntVector>& buf) const;

// stub so storing vector<RealVector> will compile
template <>
herr_t HDF5BinaryStream::store_data(const std::string& dset_name,
				    const std::vector<RealVector>& buf) const;

// stub so storing vector<vector<string> > will compile
template <>
herr_t HDF5BinaryStream::
store_data(const std::string& dset_name,
	   const std::vector<std::vector<std::string> >& buf) const;


} // namespace Dakota

#endif // BINARY_IO_HELPER_HPP

