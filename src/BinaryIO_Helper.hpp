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
#include <boost/algorithm/string.hpp>

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
  static hid_t datatype() { return H5T_NATIVE_INT; }
};

template <>
struct NativeDataTypes<double>
{
  static hid_t datatype() { return H5T_NATIVE_DOUBLE; }
};

// WJB - ToDo: verify NO need for fixed lentgh string type by 1/9/2014
#if 1
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
#endif

// Monostate pattern - really more of a "wrapper" around C API here
class H5VariableString
{
  // Nesting within SimpleBinaryStream probably makes sense in this case
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

short H5VariableString::numVStrUses = 0;
//hid_t H5VariableString::varStringType = -1;
hid_t H5VariableString::varStringType = H5VariableString::datatype();


class SimpleBinaryStream
{
public:

  /// File-based storage constructor
  SimpleBinaryStream(const  std::string& file_name = "dak_db_persist.h5",
                     bool   db_is_incore      = true,
                     bool   file_stream_exist = true,
                     bool   read_only         = true,
                     size_t max_str_len       = DerivedStringType128::length(),
                     bool   exit_on_error     = true) :
    fileName(file_name), binStreamId(),
    streamIsIncore(false), maxStringLength(max_str_len),
    exitOnError(exit_on_error), errorStatus()
  {
    // WJB - ToDo: split-out into .cpp file

    //std::cout << "Start: REPEAT old behavior (preDEFAULT incoreConstructor) " << std::endl;

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
  /// Incore storage constructor
  // WJB - ToDo:  add a strParam for the initial group?? read_only for in-core??
  SimpleBinaryStream(bool stream_is_incore,
                     size_t max_str_len = DerivedStringType128::length(),
                     bool exit_on_error = true) :
    fileName("dak_db_persist.h5"), binStreamId(),
    streamIsIncore(stream_is_incore), maxStringLength(max_str_len),
    exitOnError(exit_on_error), errorStatus()
  {
    // CORE driver - as WJB understands it, db data is "flushed" to file at end
    //http://www.mail-archive.com/hdf-forum@hdfgroup.org/msg00660.html
    hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    bool persist = true;
    if ( H5Pset_fapl_core(fapl_id, 4096, persist) && exitOnError)
      throw BinaryStream_CreateFailure();

    binStreamId = H5Fcreate( fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
                             fapl_id );
    if ( binStreamId < 0 ) {
      if ( exitOnError )
        throw BinaryStream_CreateFailure();
      //else // WJB: is there a way to "set" a GENERIC streamCreation ErrStatus?
        //errorStatus = status;
    }

    if ( !errorStatus ) {
      // Use a derived HDF data type to support more typical Dakota string len
      if ( maxStringLength != DerivedStringType128::length() && exitOnError )
        throw BinaryStream_CreateFailure();
    }

    // WJB - ToDo: look into potential resource leak
    //H5Pclose(fapl_id);
  }
#endif

  /// destructor
  ~SimpleBinaryStream()
  {
    // WJB: need to be "clever" and destroy when ALL SimpleBinaryStream objects
    // are no longer in scope (role of the HDF5 client?)
    // versus
    // allow each SimpleBinaryStream to create/destroy the "one" varStringHid
    // that it "owns"
    //H5VariableString::destroy(varStringHid);

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

    create_groups(dset_name);

    std::vector<hsize_t> dims;
    dims += 1;  // string is a 1D dataspace

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       1, dims.data(), DerivedStringType128::datatype(),
                       val.c_str() );

    // BMA: demo of storing with new data type; not sure why need the
    // extra pointer indirection
    const char * c_str = val.c_str();
    const char ** ptr_c_str = &c_str;
    std::string dsetfullname(dset_name);
    dsetfullname += "-store_data_LT";
    //H5VariableString::create vstr;
    herr_t ret_val2 = H5LTmake_dataset(binStreamId, dsetfullname.c_str(), 1, 
				       dims.data(), H5VariableString::datatype(), ptr_c_str);

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
  }


  void check_error_store(herr_t ret_val) const
  {
    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();
  }

  herr_t store_vstr(const std::string& dset_name,
                    const std::string& val) const
  {
    //std::cout << "Start: storing H5VariableString " << std::endl;

    // Demonstrate three approaches for storing a variable length string...
    // These all seem to create the same data in the file
    // In all cases, need to make appropriate H5?close(...) calls

    // Approach 1: only useful if the dataset is one string (LTstring)
    std::string sltstr(dset_name);
    sltstr += "-store_vstr_LTstr";
    herr_t ret_val1 = H5LTmake_dataset_string(binStreamId, sltstr.c_str(),
     					     val.c_str());
    check_error_store(ret_val1);
    // Approach 2: still using the LT interface; won't help with array of string
    std::vector<hsize_t> dims;
    dims += 1;  // string is a 1D dataspace
    //H5VariableString vstr;

    const char * c_str = val.c_str();
    const char ** ptr_c_str = &c_str;

/* WJB: probably don't need to leverage this, but keep for now
    std::string slt(dset_name);
    slt += "-store_vstr_LT";
    herr_t ret_val2 = H5LTmake_dataset(binStreamId, slt.c_str(), 1, 
     				      dims.data(), H5VariableString::datatype(),
    				      ptr_c_str);
    check_error_store(ret_val2);
// WJB: end Approach 2 comment block */
    // Approach 3: using the full interface, so could be leveraged for
    // array of string (1 dimensional dataspace, N elements, each a
    // variable length string)
    std::string sh(dset_name);
    sh += "-store_vstr_D";
    hid_t space = H5Screate_simple (1, dims.data(), NULL);
    hid_t dset = H5Dcreate (binStreamId, sh.c_str(), H5VariableString::datatype(),
    			    space, H5P_DEFAULT, H5P_DEFAULT,
    			    H5P_DEFAULT);
    herr_t ret_val3 = H5Dwrite (dset, H5VariableString::datatype(), H5S_ALL, H5S_ALL, 
				H5P_DEFAULT, ptr_c_str);
    check_error_store(ret_val3);

    //std::cout << "Finish: storing H5VariableString " << std::endl;
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
    htri_t path_valid = H5LTpath_valid(binStreamId, dset_name.c_str(), 1);
    if ( path_valid != 1 && exitOnError )
      throw BinaryStream_InvalidPath();

    buf.reserve(maxStringLength);
    herr_t ret_val = H5LTread_dataset_string(binStreamId, dset_name.c_str(), &buf[0]);

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

    create_groups(dset_name);

    const size_t num_strings = buf.size();

    // WJB - ToDo:  experiment with chunk size and compression (-1==NO) params
    hid_t strings_pt = H5PTcreate_fl( binStreamId, dset_name.c_str(),
                         DerivedStringType128::datatype(),
                         num_strings*DerivedStringType128::length(), -1);

    // Write each string in the array the "packet" table, individually
    // WJB - ToDo:  *should* consider writing entire buffer out in a single call
    //std::cout << "chk 2ndString in Array: " << buf[1].c_str() << std::endl;
    
    herr_t ret_val;
    for(int i=0; i<num_strings; ++i)
      ret_val = H5PTappend( strings_pt, 1, buf[i].c_str() );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    // WJB - ToDo: look into potential resource leak
    //H5PTclose(strings_pt);
    return ret_val;
  }


  template <typename T, size_t DIM>
  herr_t store_data(const std::string& dset_name,
                    const std::vector<hsize_t>& dims,
                    const T* buf) const
  {
    if ( dims.size() != DIM && exitOnError )
      throw BinaryStream_StoreDataFailure();

    create_groups(dset_name);

    herr_t ret_val = H5LTmake_dataset( binStreamId, dset_name.c_str(),
                       DIM, dims.data(), NativeDataTypes<T>::datatype(),
                       buf );

    if ( ret_val < 0 && exitOnError )
      throw BinaryStream_StoreDataFailure();

    return ret_val;
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

  /// file name of binary file stream - empty string if in-core
  std::string fileName;

  /// Binary stream ID
  hid_t binStreamId;

  /// Toggle for storage - default is false - true means store data in-core
  bool streamIsIncore; // WJB: enforce an empty string for fileName in this case

  /// Max string length for storing std::string as a type derived from H5T_C_S1
  const size_t maxStringLength;

  /// hid_t of the variable-length string_type derived from H5T_C_S1
  //  WJB: maybe one variable length string creation PER HDF5 stream object is the sensible way to manage the resource?
  //const hid_t varStringHid; // WJB: "feels redundant" -- refactor ASAP

  /// Toggle for exit vs continue execution - default is true
  bool exitOnError;

  /// In-class caching of the error status code (mostly for debugging)
  herr_t errorStatus;


  // WJB:  consider boost::ublas::bounded_vector type instead
  //static std::vector<hsize_t, 1> staticVectorDims;
  //static std::vector<hsize_t, 2> staticMatrixDims;
  //static std::vector<hsize_t, 3> static3DBufDims;

};


} // namespace Dakota

#endif // BINARY_IO_HELPER_HPP

