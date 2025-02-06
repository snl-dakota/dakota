/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef HDF5_IO_HELPER_HPP
#define HDF5_IO_HELPER_HPP

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "dakota_results_types.hpp"

//#include <boost/filesystem/operations.hpp>
//#include <boost/multi_array.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>

// We are mixing C and C++ APIs here with an eye to eventually adopt one or the other - RWH
#include "H5Cpp.h"      // C++ API
#include "hdf5.h"       // C   API
#include "hdf5_hl.h"    // C   H5Lite API
#include "H5Opublic.h"  
#include <iostream>
#include <limits>
#include <memory>
#include <cmath>
#include <string>
#include <vector>


namespace Dakota
{

//----------------------------------------------------------------

// Data written to HDF5 must have source (memory) datatype and
// sink (file) datatype. These free functions return the HDF5
// DataTypes that correspond to the C++ types of data that Dakota
// wants to write out. They enable writing code that is templated
// on those C++ types. The h5_mem_type functions return NATIVE_*
// types that may be platform dependent: a NATIVE_INT may be 32
// bits and little endian on one platform, and 64 bits and big
// endian on another. HDF5 handles those details, promising
// that NATIVE_INT can be used to access a C/C++ int in memory
// on THIS platform. For h5_file_types, we choose a particular
// size and endianness, regardless of the platform, which means
// that all Dakota HDF5 files, regardless of what's native to
// the platform where they were generated, are consistent.

// File Types

/// Return the HDF5 datatype to store a short
inline H5::DataType h5_file_dtype( const short & )
{ return H5::PredType::STD_I16LE; }

/// Return the HDF5 datatype to store a int
inline H5::DataType h5_file_dtype( const int & )
{ return H5::PredType::STD_I32LE; }

//// Return the HDF5 datatype to store a uint
inline H5::DataType h5_file_dtype( const unsigned int & )
{ return H5::PredType::STD_U32LE; }

// Return the HDF5 datatype to store an unsigned long (maybe a size_t)
inline H5::DataType h5_file_dtype( const unsigned long & )
{ return H5::PredType::STD_U64LE; }

// Return the HDF5 datatype to store an unsigned long long (maybe a size_t)
inline H5::DataType h5_file_dtype( const unsigned long long & )
{ return H5::PredType::STD_U64LE; }

/// Return the HDF5 datatype to store a Real
inline H5::DataType h5_file_dtype( const Real & )
{ return H5::PredType::IEEE_F64LE; }

/// Return the HDF5 datatype to store a string
inline H5::DataType h5_file_dtype( const char * )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

/// Overloads for ResultsOutputType (used when creating empty datasets)
inline H5::DataType h5_file_dtype(const ResultsOutputType t) {
  switch(t) {
    case ResultsOutputType::REAL:
      return h5_file_dtype(double(0.0)); break;
    case ResultsOutputType::INTEGER:
      return h5_file_dtype(int(0)); break;
    case ResultsOutputType::UINTEGER:
      return h5_file_dtype(unsigned(0)); break;
    case ResultsOutputType::STRING:
      return h5_file_dtype(""); break;
  }
}

/// Return the HDF5 datatype to store a string
inline H5::DataType h5_file_dtype( const String &)
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

// Memory Types

/// Return the HDF5 datatype to read a Real in memory
inline H5::DataType h5_mem_dtype( const Real & )
{ return H5::PredType::NATIVE_DOUBLE; }

/// Return the HDF5 datatype to read a short in memory
inline H5::DataType h5_mem_dtype( const short & )
{ return H5::PredType::NATIVE_SHORT; }

/// Return the HDF5 datatype to read an int in memory
inline H5::DataType h5_mem_dtype( const int & )
{ return H5::PredType::NATIVE_INT; }

/// Return the HDF5 datatype to read an unsigned int in memory
inline H5::DataType h5_mem_dtype( const unsigned int & )
{ return H5::PredType::NATIVE_UINT; }

/// Return the HDF5 datatype to read an unsigned long (maybe a size_t) in memory
inline H5::DataType h5_mem_dtype( const unsigned long & )
{ return H5::PredType::NATIVE_ULONG; }

/// Return the HDF5 datatype to read an unsigned long long (maybe a
/// size_t) in memory
inline H5::DataType h5_mem_dtype( const unsigned long long & )
{ return H5::PredType::NATIVE_ULLONG; }

/// Return the HDF5 datatype to read a string in memory
inline H5::DataType h5_mem_dtype( const char * )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

/// Return the HDF5 datatype to read a string in memory
inline H5::DataType h5_mem_dtype( const String &)
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

/// Overloads for ResultsOutputType (used when creating empty datasets)
inline H5::DataType h5_mem_dtype(const ResultsOutputType t) {
  switch(t) {
    case ResultsOutputType::REAL:
      return h5_mem_dtype(double(0.0)); break;
    case ResultsOutputType::INTEGER:
      return h5_mem_dtype(int(0)); break;
    case ResultsOutputType::UINTEGER:
      return h5_mem_dtype(unsigned(0)); break;
    case ResultsOutputType::STRING:
      return h5_mem_dtype(""); break;
  }
}

/// Return the length of seeral types
template <typename T>
int length(const std::vector<T> &vec) {
  return vec.size();
}

/// Return the length of an SDV
template <typename T>
int length(const Teuchos::SerialDenseVector<int, T> &vec) {
  return vec.length();
}
/// Return the length of a StringMultiArrayConstView
int length(const StringMultiArrayConstView &vec);

/// Return a vector of pointers to strings
template<typename T>
std::vector<const char *> pointers_to_strings(const T &data) {
  std::vector<const char *> ptrs_to_data(data.size());
  std::transform(data.begin(), data.end(), ptrs_to_data.begin(),
      [](const String &s){return s.c_str();});
  return ptrs_to_data;
}


/**
 * This helper class provides wrapper functions that perform
 * low-level access operations in HDF5 databases.
 *
 * Authors:  J. Adam Stephens, Russell Hooper, Elliott Ridgway
 */

class HDF5IOHelper
{
  /* The names of member functions for reading and writing data follow a loose
   * convention.
   * WRITING
   *  - Create and write a dataset all at once -
   *   store_scalar
   *   store_vector
   *   store_matrix
   *  - Place data into an existing higher dimensional dataset (call create_empty_dataset first)
   *   set_scalar (in a 1D dataset)
   *   set_vector (in a 2D dataset)
   *   set_matrix (in a 3D dataset)
   *   set_vector_matrix (in a 4D dataset)
   *   set_vector_scalar_field (assign a scalar field to a 1D dataset of compound type)
   *   set_vector_vector_field (assign a vector field to a 1D dataset of compound type)
   *  - Append data into a dataset with an unlimited 0th dimension (call create_empty_dataset first)
   *   append_scalar (to a 1D dataset)
   *   append_vector (to a 2D dataset)
   *   append_matrix (to a 3D dataset)
   *   append_vector_matrix (to a 4D dataset)
   * READING
   *  - Read an entire dataset
   *   read_scalar
   *   read_vector
   *   read_matrix
   *  - Read a slice (all diemnsions after 0)
   *   get_scalar (from 1D dataset)
   *   get_vector (from 2D dataset)
   *   get_matrix (from 3D dataset)
   *   get_vector_matrix (from 4D dataset)
   */
  public:

  HDF5IOHelper(const std::string& file_name, bool overwrite = false);

  /// Store scalar data to a data set
  template <typename T>
  void store_scalar(const std::string& dset_name, const T& val);
  /// Store string scalar data to a data set
  void store_scalar(const std::string& dset_name, const String& val);
  /// Store vector (1D) information to a dataset
  template <typename T>
  void store_vector(const std::string& dset_name,
                         const std::vector<T>& array) const;
  /// Store vector (1D) information to a dataset
  template <typename T>
  void store_vector( const std::string & dset_name,
                          const Teuchos::SerialDenseVector<int,T> & vec );
 
  /// Store vector (1D) information to a dataset
  void store_vector(const std::string & dset_name,
                                 const StringMultiArrayConstView &vec );

  /// Store vector (1D) information to a dataset
  void store_vector(const std::string & dset_name,
                                 const SizetMultiArrayConstView &vec );
 
  /// Store matrix (2D) information to a dataset
  template<typename T>
  void store_matrix(const std::string &dset_name, 
      const Teuchos::SerialDenseMatrix<int,T> & matrix, 
      const bool &transpose = false) const; 
  /// Store matrix (2D) information to a dataset
  template<typename T>
  void store_matrix(const std::string &dset_name, 
      const std::vector<T> &buf, const int & num_cols,
      const bool &transpose = false) const;
  /// Store matrix (2D) information to a dataset
  void store_matrix(const std::string &dset_name, 
      const std::vector<String> &buf, const int & num_cols,
      const bool &transpose = false) const;

  /// Set a scalar in a 1D dataset at index using its name
  template<typename T>
  void set_scalar(const String &dset_name, const T &data, const int &index); 
  /// Set a scalar in a 1D dataset at index using the dataset object
  template<typename T>
  void set_scalar(const String &dset_name, H5::DataSet &ds, const T &data, const int &index);
  /// Set a scalar in a 1D dataset at index using the dataset object
  void set_scalar(const String &dset_name, H5::DataSet &ds, const String &data, const int &index);
  
  /// Set a row or column in a 2D dataset at index using its name
  template<typename T>
  void set_vector(const String &dset_name, const T &data, const int &index, const bool &row = true); 
  /// Set a row or column of Strings in a 2D dataset at index using the dataset object
  void set_vector(const String &dset_name, H5::DataSet &ds, const StringMultiArrayConstView &data, 
      const int &index, const bool &row = true);
  /// Set a row or column of Strings in a 2D dataset at index using the dataset object
  void set_vector(const String &dset_name, H5::DataSet &ds, const std::vector<String> &data, 
      const int &index, const bool &row = true);
  /// Set a row or column in a 2D dataset at index using the dataset object
  template<typename T>
  void set_vector(const String &dset_name, H5::DataSet &ds, const T &data, const int &index, const bool &row = true);

  /// Set a matrix in a 3D dataset at the index into the 0th dimension by name. 
  /// Dims of matrix must match those of the trailing dimensions of the dataset.
  template<typename T>
  void set_matrix(const String &dset_name, 
                     const Teuchos::SerialDenseMatrix<int, T> &data,
                     const int &index,
                     const bool &transpose = false);
  /// Set a matrix in a 3D dataset at the index into the 0th dimension using a 
  /// dataset object. Dims of matrix must match those of the trailing dimensions of 
  /// the dataset.
  template<typename T>
  void set_matrix(const String &dset_name, H5::DataSet &ds, 
                     const Teuchos::SerialDenseMatrix<int, T> &data,
                     const int &index,
                     const bool &transpose = false);
  /// Set a 3D slab in a 4D dataset at the index into the 0th dimension by name.
  /// The length of the vector must match the 1st dimension of the dataset, and
  /// the dimensions of the matrices must all match the 2nd and 3rd dimensions.
  template<typename T>
  void set_vector_matrix(const String &dset_name, 
                     const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                     const int &index,
                     const bool &transpose = false);
  /// Set a 3D slab in a 4D dataset at the index into the 0th dimension using a
  /// dataset object.
  /// The length of the vector must match the 1st dimension of the dataset, and
  /// the dimensions of the matrices must all match the 2nd and 3rd dimensions.
  template<typename T>
  void set_vector_matrix(const String &dset_name, H5::DataSet &ds, 
                     const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                     const int &index,
                     const bool &transpose = false);
 
  /// Set a scalar field on all elements of a 1D dataset of compound type using a ds name.
  template<typename T>
  void set_vector_scalar_field(const String &dset_name,
                     const T &data, const String &field_name);
  /// Set a scalar field on all elements of a 1D dataset of compound type using a ds object.
  template<typename T>
  void set_vector_scalar_field(const String &dset_name, H5::DataSet &ds,
                     const std::vector<T> &data, const String &field_name);

  /// Set a vector field on all elements of a 1D dataset of compound type using a ds name.
  template<typename T>
  void set_vector_vector_field(const String &dset_name,
                     const T &data, const size_t length,  const String &field_name);
  /// Set a vector field on all elements of a 1D dataset of compound type using a ds object.
  template<typename T>
  void set_vector_vector_field(const String &dset_name, H5::DataSet &ds,
                     const std::vector<T> &data, const size_t length, const String &field_name);
  /// Set a vector field on all elements of a 1D dataset of compound type using a ds object.
  void set_vector_vector_field(const String &dset_name, H5::DataSet &ds,
                     const std::vector<String> &data, const size_t length, const String &field_name);

  /// Append an empty "layer" to the 0th dimension and return its index
  int append_empty(const String &dset_name);
  
  /// Append a scalar to a 1D dataset
  template<typename T>
  void append_scalar(const String &dset_name, const T&data); 
  /// Append a scalar to a 1D dataset
  void append_scalar(const String &dset_name, const String &data); 
  
  /// Append a vector as a row or column to a 2D dataset
  template<typename T>
  void append_vector(const String &dset_name, const T &data, const bool &row = true);
  /// Append a vector as a row or column to a 2D dataset
  void append_vector(const String &dset_name, const std::vector<String> &data, const bool &row = true);
  /// Append a vector as a row or column to a 2D dataset
  void append_vector(const String &dset_name, const StringMultiArrayConstView &data, const bool &row = true);
  /// Append a SerialDenseMatrix to a 3D dataset. The dataset will be expanded along the 0th
  /// dimension. By default, the shape of the matrix, (nrows, ncols), must match the size of the 1st 
  /// and 2nd dimensions of the dataset. For transpose=true, the reverse must be true. 
  template<typename T>
  void append_matrix(const String &dset_name, 
                     const Teuchos::SerialDenseMatrix<int, T> &data,
                     const bool &transpose = false); 
  /// Append a std::vector of  SerialDenseMatrix's to a 4D dataset. The dataset 
  /// will be expanded along the 0th dimension. By default, the size of the vector
  /// must equal the size of the 1st dimension of the dataset, andthe shape of the 
  /// SDMs (nrows, ncols), must match the sizes of the 2nd 
  /// and 2nd dimensions of the dataset. For transpose=true, the reverse must 
  ///  be true of the SDMs.
  template<typename T>
  void append_vector_matrix(const String &dset_name, 
                     const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                     const bool &transpose = false);

  /// Read scalar data from a dataset
  template <typename T>
  void read_scalar(const std::string& dset_name, T& val);
  /// Read scalar data from a dataset
  void read_scalar(const std::string& dset_name, String& val);
  /// Read vector (1D) information from a dataset
  template <typename T>
  void read_vector(const std::string& dset_name, T& array) const; 
  /// Read a vector of Strings from a dataset
  void read_vector(const std::string& dset_name, StringArray& array) const; 
  /// Read matrix (2D) information from a dataset
  /// Currently this involves a wasteful copy to do the transpose and
  /// is intended only for purposes of testing
  template <typename T>
  void read_matrix(const std::string &dset_name, 
      Teuchos::SerialDenseMatrix<int,T> & matrix, 
      const bool &transpose = false) const; 
  /// Get the matrix (2D) at the index into the 0th dimension of the 3D
  /// dataset at dsetname. 
  /// Currently this involves a wasteful copy to do the transpose and
  /// is intended only for purposes of testing
  template <typename T>
  void get_matrix(const std::string &dset_name, 
      Teuchos::SerialDenseMatrix<int,T> & matrix,
      const int &index, const bool &transpose = false) const;
  /// Read the 3D slice at the index into the 0th dimension of the 4D dataset at ds_name. 
  /// Currently this involves a wasteful copy to do the transpose and
  /// is intended only for purposes of testing
  template <typename T>
  void get_vector_matrix(const std::string &dset_name, 
      std::vector<Teuchos::SerialDenseMatrix<int,T> >& data,
      const int &index, const bool &transpose = false) const;
  /// Report the number of open descriptors of each type; just for debugging
  void report_num_open();
  /// Create an empty dataset. Setting the first element of dims to 0 makes
  /// the dataset unlimited in that dimension. DSs unlimited in other dimensions
  /// currently are unsupported.
  void create_empty_dataset(const String &dset_name, const IntArray &dims, 
                         ResultsOutputType stored_type, int chunk_size=0, 
                         const void *fill_val = NULL);

  /// Create a dataset with compound type
  void create_empty_dataset(const String &dset_name, const IntArray &dims, 
                         const std::vector<VariableParametersField> &fields);
  //------------------------------------------------------------------

  /// attach a dimension scale to a dataset
  void attach_scale(const String& dset_name, const String& scale_name,
                    const String& label, const int& dim) const;

  /// Add an attribute to a group or dataset
  template <typename T>
  void add_attribute(const String &location, const String &label,
                     const T &value); 
  /// Add an attribute to a group or dataset
  void add_attribute(const String &location, const String &label,
                     const String &value); 
  /// Does a group or dataset exist?
  bool exists(const String location_name) const;

  /// Is the dataset a dimenions scale?
  bool is_scale(const H5::DataSet dset) const;

  /// Create a group hierarchy (final token optionally a dataset name)
  H5::Group create_groups(const std::string& name,
                          bool includes_dset=true) const;

  /// Create a dataset with a custom CreatPropList
  H5::DataSet create_dataset( const H5::H5Location &loc,
                              const std::string &name,
                              const H5::DataType &type,
                              const H5::DataSpace &space,
                              const H5::DSetCreatPropList &create_plist =
                                H5::DSetCreatPropList(),
                              const H5::DSetAccPropList &access_plist =
                                H5::DSetAccPropList() ) const;

  /// Create a group
  H5::Group create_group( const H5::H5Location &loc,
                              const std::string &name) const;
  /// Create a soft link
  void create_softlink(const String &link_location, const String &source_location); 
  

  // Define globally available custom property lists
  // JAS: These probably should not be public. The point of this class is to
  // encapsulate these kinds of low-level details.
  
  /// Gobal link creation property list
  H5::LinkCreatPropList linkCreatePL;
  /// Gobal DataSet creation property list for compact datasets
  H5::DSetCreatPropList datasetCompactPL;
  /// Gobal DataSet creation property list for contiguous datasets
  H5::DSetCreatPropList datasetContiguousPL;

  /// Flush cache to file
  void flush() const;
 
  ~HDF5IOHelper() {}; 

  protected:

  /// Name of the HDF5 file
  std::string fileName;

  /// HDF5 file object
  H5::H5File h5File;

  /// create an attribute at the location and return it
  template <typename T>
  H5::Attribute create_attribute(const String &location, const String &label, const T &data);

  /// Store vector data using a pointer to the first element and length
  template<typename T>
  void store_vector(const String &dset_name, const T *data,
                         const int &len) const; 
  /// Store vector of Strings using a pointer to the first element and
  /// length
  void store_vector(const String &dset_name, const String *data,
                         const int &len) const; 
  
  /// Cache open datasets that have unlimited dimension
  /// This is an optimization to prevent eval-related datasets being
  /// repeatedly flushed and reopened, which is very costly
  std::map<String, H5::DataSet> datasetCache;

  //H5::DataSet open_dataset(const String &ds_name);

}; // class HDF5IOHelper


template <typename T>
void HDF5IOHelper::store_scalar(const std::string& dset_name, const T& val)
{
  H5::DataSpace dataspace = H5::DataSpace();
  
  // Assume dset_name is syntactically correct - will need some utils - RWH
  create_groups(dset_name);
  H5::DataSet dataset(create_dataset(
      h5File, dset_name, h5_file_dtype(val), dataspace));

  dataset.write(&val, h5_mem_dtype(val));
      return;
}


/// Store vector (1D) information to a dataset
template <typename T>
void HDF5IOHelper::store_vector(const std::string& dset_name,
                       const std::vector<T>& array) const
{
  store_vector(dset_name, array.data(), array.size());
  return;
}

template <typename T>
void HDF5IOHelper::store_vector( const std::string & dset_name,
                        const Teuchos::SerialDenseVector<int,T> & vec )
{
  store_vector(dset_name, &vec[0], vec.length());
  return;
}

/// Store matrix (2D) information to a dataset
template<typename T>
void HDF5IOHelper::store_matrix(const std::string &dset_name, 
    const Teuchos::SerialDenseMatrix<int,T> & matrix, 
    const bool &transpose) const {

  hsize_t f_dims[2], m_dims[2]; // file and memory dimensions
  H5::DataSpace f_dataspace, m_dataspace; // file and memory DataSpaces
  const int &num_cols = matrix.numCols();
  const int &num_rows = matrix.numRows();

  // Assume dset_name is syntactically correct - will need some utils - RWH
  create_groups(dset_name);
  H5::DataType f_datatype = h5_file_dtype(matrix[0][0]); // file datatype
  H5::DataType m_datatype = h5_mem_dtype(matrix[0][0]);  // memory datatype

  // SerialDenseMatrixes are stored column-major, but HDF5 assumes row-major
  // storage. If transposed storage is requested, we therefore don't need to 
  // do anything special to the dataspace
  if(transpose) {
    m_dims[0] = f_dims[0] = num_cols;
    m_dims[1] = f_dims[1] = num_rows;
    f_dataspace.setExtentSimple(2, f_dims);
    m_dataspace.setExtentSimple(2, m_dims);
    H5::DataSet dataset(
      create_dataset(h5File, dset_name, f_datatype, f_dataspace) );
    dataset.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
  } else {
    // to write an un-tranposed matrix, we use HDF5 hyperslab selections to 
    // sequentially grab rows of the matrix in memory and write them into rows
    // of the dataset in file. This is a little tricky because rows in memory 
    // are non-contiguous due to column-major storage of SDMs.
    // Reminder: To understan what this code is doing, it helps to take out a
    // sheet of paper and draw the file dataset and the matrix, with the matrix
    // transposed because SDMs are stored column-major, and then think about
    // how the data has to move.
    m_dims[1] = f_dims[0] = num_rows;
    m_dims[0] = f_dims[1] = num_cols;
    f_dataspace.setExtentSimple(2, f_dims);
    m_dataspace.setExtentSimple(2, m_dims);
    H5::DataSet dataset(
      create_dataset(h5File, dset_name, f_datatype, f_dataspace) );
    hsize_t m_start[2], f_start[2]; // "start" in the C++ API is "offset" in the C API.
    m_start[0] = f_start[1] = 0;
    hsize_t m_count[2] = {hsize_t(num_cols), 1};
    hsize_t f_count[2] = {1, hsize_t(num_cols)};
    for(int i = 0; i < num_rows; ++i) { // iterate over rows of the matrix; columns of the dataset
      m_start[1] = f_start[0] = i;
      m_dataspace.selectHyperslab(H5S_SELECT_SET, m_count, m_start);
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      dataset.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
    }
  }
  return;
}

/// Store matrix (2D) information to a dataset
template<typename T>
void HDF5IOHelper::store_matrix(const std::string &dset_name, 
    const std::vector<T> &buf, const int & num_cols,
    const bool &transpose) const {
  
  hsize_t f_dims[2], m_dims[2]; // file and memory dimensions
  H5::DataSpace f_dataspace, m_dataspace; // file and memory DataSpaces
  // TODO: ensure buf size is evenly divisible by number of columns
  const int num_rows = buf.size()/num_cols;

  // Assume dset_name is syntactically correct - will need some utils - RWH
  create_groups(dset_name);
  H5::DataType f_datatype = h5_file_dtype(buf[0]); // file datatype
  H5::DataType m_datatype = h5_mem_dtype(buf[0]);  // memory datatype
  
  if(transpose) {
    // to write a tranposed matrix, we use HDF5 hyperslab selections to 
    // sequentially grab rows of the matrix in memory and write them into columns
    // of the dataset in file. 
    m_dims[0] = f_dims[1] = num_rows;
    m_dims[1] = f_dims[0] = num_cols;
    f_dataspace.setExtentSimple(2, f_dims);
    m_dataspace.setExtentSimple(2, m_dims);
    H5::DataSet dataset(
      create_dataset(h5File, dset_name, f_datatype, f_dataspace) );
    hsize_t m_start[2], f_start[2];
    m_start[1] = f_start[0] = 0; // iterate over rows in memory/columns in the file
    hsize_t m_count[2] = {1, hsize_t(num_cols)}; 
    hsize_t f_count[2] = {hsize_t(num_cols), 1};
    for(int i = 0; i < num_rows; ++i) {
      m_start[0] = f_start[1] = i;
      m_dataspace.selectHyperslab(H5S_SELECT_SET, m_count, m_start);
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      dataset.write(buf.data(), m_datatype, m_dataspace, f_dataspace);
    }
  } else {
    m_dims[0] = f_dims[0] = num_rows;
    m_dims[1] = f_dims[1] = num_cols;
    f_dataspace.setExtentSimple(2, f_dims);
    m_dataspace.setExtentSimple(2, m_dims);
    H5::DataSet dataset(
      create_dataset(h5File, dset_name, f_datatype, f_dataspace) );
    dataset.write(buf.data(), m_datatype, m_dataspace, f_dataspace);
  }
  return;
}


template<typename T>
void HDF5IOHelper::set_scalar(const String &dset_name, const T &data, const int &index) {
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_scalar(dset_name, ds_iter->second, data, index);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_scalar(dset_name, ds, data, index);
  }
}

template<typename T>
void HDF5IOHelper::set_scalar(const String &dset_name, H5::DataSet &ds, const T &data, const int &index) {
  // 1. open the dataset
  // 2. discover the rank (must be 1) and dimensions (must be > index)
  // 3. Select the elements in the memory and file dataspaces
  // 4. write
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 1) {
    flush();
    throw std::runtime_error(String("Attempt to insert element into a non-1D datasset ") +
                               dset_name + " failed");
  }
  hsize_t dim;
  f_space.getSimpleExtentDims(&dim);
  if(index < 0 || index >= dim) {
    flush();
    throw std::runtime_error(String("Attempt to insert element into ") + dset_name +
                               " failed; requested index is " + std::to_string(index) +
                               " but must be > 0 and < " + std::to_string(dim));
  }
  hsize_t f_coords = hsize_t(index);
  f_space.selectElements(H5S_SELECT_SET, 1, &f_coords);
  H5::DataSpace m_space;  // memory dataspace (scalar)
  ds.write(&data, h5_mem_dtype(data), m_space, f_space);
}

template<typename T>
void HDF5IOHelper::set_vector(const String &dset_name, const T &data, const int &index, const bool &row) {
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_vector(dset_name, ds_iter->second, data, index, row);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_vector(dset_name, ds, data, index, row);
  }
}

template<typename T>
void HDF5IOHelper::set_vector(const String &dset_name, H5::DataSet &ds, const T &data, const int &index, const bool &row) {
  // 1. open the dataset
  // 2. discover the rank (must be 2) and dimensions
  // 3. Raise an error if the dimensions of the dataset and the data don't match
  // 4. create hyperslabs for the memory and file to write a row or column
  // 5. Write
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 2) {
    flush();
    throw std::runtime_error(String("Attempt to insert row or column into non-2D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t dims[2]; // assume rank == 2
  f_space.getSimpleExtentDims(dims);
  int len = length(data); // length is a free function defined in HDF5_IO.hpp that is 
                          // overloaded/templated to return the length of SDVs and std::vectors
  if(row) {
    if(dims[1] != len) {
      flush();
      throw std::runtime_error(String("Attempt to insert row into  ") + 
                                 dset_name + " failed; length of data is " + 
                                 std::to_string(len) + " and number of DS columns is " + 
                                 std::to_string(dims[1]) );
    } else if(index >= dims[0] || index < 0) {
      flush();
      throw std::runtime_error(String("Attempt to insert row into ") +
                                 dset_name + " failed; requested index is " + 
                                 std::to_string(index) + " but must be > 0 and < " +
                                 std::to_string(dims[0]));
    }
  } else { 
    if(dims[0] != len) {
      flush();
      throw std::runtime_error(String("Attempt to insert column into  ") + 
                                 dset_name + " failed; length of data is " + 
                                 std::to_string(len) + " and number of DS rows is " + 
                                 std::to_string(dims[0]) );
    } else if(index >= dims[1] || index < 0) {
      flush();
      throw std::runtime_error(String("Attempt to insert column into  ") + 
                                 dset_name + " failed; requested index is " + 
                                 std::to_string(index) + " but must be > 0 and < " +
                                 std::to_string(dims[1]));
    }
  }
  hsize_t f_count[2], f_start[2], m_dim[1]; // f_count and f_start are used to index into the
                                            // dataset. m_dim is the dimension of the data in memory
  m_dim[0] = len;
  H5::DataSpace m_space(1, m_dim);  // memory dataspace.
  if(row) {
    f_count[0] = 1; f_count[1] = len;
    f_start[0] = index; f_start[1] = 0;
  } else {
    f_count[0] = len; f_count[1] = 1;
    f_start[0] = 0; f_start[1] = index;
  }
  f_space.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
  ds.write(&data[0], h5_mem_dtype(data[0]), m_space, f_space);
}

template<typename T>
void HDF5IOHelper::set_matrix(const String &dset_name, 
                   const Teuchos::SerialDenseMatrix<int, T> &data,
                   const int &index,
                   const bool &transpose) {
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_matrix(dset_name, ds_iter->second, data, index, transpose);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_matrix(dset_name, ds, data, index, transpose);
  }
}

template<typename T>
void HDF5IOHelper::set_matrix(const String &dset_name, H5::DataSet &ds, 
                   const Teuchos::SerialDenseMatrix<int, T> &data,
                   const int &index,
                   const bool &transpose) {
  // 1. open the dataset
  // 2. discover the rank (must be 3) and dimensions
  // 3. index must be < first dimension
  // 3. Other dimensions of the dataset and the data must match
  // 4. Set up to do a transposed or un-transposed write
  // 5. Write
  const int &num_cols = data.numCols();
  const int &num_rows = data.numRows();

  H5::DataSpace m_dataspace, f_dataspace = ds.getSpace();
  if(f_dataspace.getSimpleExtentNdims() != 3) {
    flush();
    throw std::runtime_error(String("Attempt to insert matrix into non-3D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t f_dims[3], m_dims[2]; // file and memory dimensions
  f_dataspace.getSimpleExtentDims(f_dims);
  if(index >= f_dims[0]) {
    flush();
    throw std::runtime_error(String("Attempt to insert matrix into  ") + 
                             dset_name + " failed; requested index greater than 0th dimension " +
                             "of dataset.");
  }
  if( (transpose && !(f_dims[2] == num_rows && f_dims[1] == num_cols)) ||
      (!transpose && !(f_dims[1] == num_rows && f_dims[2] == num_cols)) ) {
    flush();
    throw std::runtime_error(String("Attempt to insert matrix into  ") + 
                             dset_name + " failed; matrix dimensions do not match " +
                             "dataset dimensions.");

  }
  H5::DataType f_datatype = h5_file_dtype(data[0][0]); // file datatype
  H5::DataType m_datatype = h5_mem_dtype(data[0][0]);  // memory datatype

  if(transpose) {
    // SerialDenseMatrixes are stored column-major, but HDF5 assumes row-major
    // storage. If transposed storage is requested, we therefore don't need to 
    // do anything special to the dataspace
    m_dims[0] = num_rows;
    m_dims[1] = num_cols;
    m_dataspace.setExtentSimple(2, m_dims);
    hsize_t f_start[3] = {hsize_t(index), 0, 0};
    hsize_t f_count[3] = {1, hsize_t(num_cols), hsize_t(num_rows)};
    f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
    ds.write(data.values(), m_datatype, m_dataspace, f_dataspace);
  } else {
    // to write an un-tranposed matrix, we use HDF5 hyperslab selections to 
    // sequentially grab rows of the matrix in memory and write them into rows
    // of the dataset in file. This is a little tricky because rows in memory 
    // are non-contiguous due to column-major storage of SDMs.
    // Reminder: To understan what this code is doing, it helps to take out a
    // sheet of paper and draw the file dataset and the matrix, with the matrix
    // transposed because SDMs are stored column-major, and then think about
    // how the data has to move.
    
    m_dims[1] = num_rows;
    m_dims[0] = num_cols;
    m_dataspace.setExtentSimple(2, m_dims);
    hsize_t m_start[2], f_start[3]; // "start" in the C++ API is "offset" in the C API.
    f_start[0] = index;
    m_start[0] = f_start[2] = 0;
    hsize_t m_count[2] = {hsize_t(num_cols), 1};
    hsize_t f_count[3] = {1, 1, hsize_t(num_cols)};
    for(int i = 0; i < num_rows; ++i) { // iterate over rows of the matrix; columns of the dataset
      m_start[1] = f_start[1] = i;
      m_dataspace.selectHyperslab(H5S_SELECT_SET, m_count, m_start);
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      ds.write(data.values(), m_datatype, m_dataspace, f_dataspace);
    }
  }
} 

template<typename T>
void HDF5IOHelper::set_vector_matrix(const String &dset_name, 
                   const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                   const int &index,
                   const bool &transpose) {
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_vector_matrix(dset_name, ds_iter->second, data, index, transpose);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_vector_matrix(dset_name, ds, data, index, transpose);
  }
}

template<typename T>
void HDF5IOHelper::set_vector_matrix(const String &dset_name, H5::DataSet &ds, 
                   const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                   const int &index,
                   const bool &transpose) {
  // 1. open the dataset
  // 2. discover the rank (must be 4) and dimensions
  // 3. index must be < first dimension
  // 3. Other dimensions of the dataset and the data must match
  // 4. Set up to do a transposed or un-transposed write
  // 5. Write
  const int &num_cols = data[0].numCols();
  const int &num_rows = data[0].numRows();

  H5::DataSpace m_dataspace, f_dataspace = ds.getSpace();
  if(f_dataspace.getSimpleExtentNdims() != 4) {
    flush();
    throw std::runtime_error(String("Attempt to insert vector-matrix into non-4D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t f_dims[4], m_dims[2]; // file and memory dimensions
  f_dataspace.getSimpleExtentDims(f_dims);
  if(index >= f_dims[0]) {
    flush();
    throw std::runtime_error(String("Attempt to insert vector-matrix into  ") + 
                             dset_name + " failed; requested index greater than 0th dimension " +
                             "of dataset.");
  }
  if( (transpose && !(f_dims[3] == num_rows && f_dims[2] == num_cols)) ||
      (!transpose && !(f_dims[2] == num_rows && f_dims[3] == num_cols)) ) {
    flush();
    throw std::runtime_error(String("Attempt to insert vector-matrix into  ") + 
                             dset_name + " failed; matrix dimensions do not match " +
                             "dataset dimensions.");

  }
  H5::DataType f_datatype = h5_file_dtype(data[0](0,0)); // file datatype
  H5::DataType m_datatype = h5_mem_dtype(data[0](0,0));  // memory datatype

  if(transpose) {
    // SerialDenseMatrixes are stored column-major, but HDF5 assumes row-major
    // storage. If transposed storage is requested, we therefore don't need to 
    // do anything special to the dataspace
    m_dims[0] = num_rows;
    m_dims[1] = num_cols;
    m_dataspace.setExtentSimple(2, m_dims);
    hsize_t f_start[4] = {hsize_t(index), 0, 0, 0};
    hsize_t f_count[4] = {1, 1, hsize_t(num_cols), hsize_t(num_rows)};
    for(f_start[1] = 0; f_start[1] < data.size(); ++f_start[1]) {
      const auto & matrix = data[f_start[1]];
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      ds.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
    }
  } else {
    // to write an un-tranposed matrix, we use HDF5 hyperslab selections to 
    // sequentially grab rows of the matrix in memory and write them into rows
    // of the dataset in file. This is a little tricky because rows in memory 
    // are non-contiguous due to column-major storage of SDMs.
    // Reminder: To understan what this code is doing, it helps to take out a
    // sheet of paper and draw the file dataset and the matrix, with the matrix
    // transposed because SDMs are stored column-major, and then think about
    // how the data has to move.
    
    m_dims[1] = num_rows;
    m_dims[0] = num_cols;
    m_dataspace.setExtentSimple(2, m_dims);
    hsize_t m_start[2] = {0,0}; // "start" in the C++ API is "offset" in the C API.
    hsize_t f_start[4] = {hsize_t(index), 0, 0, 0};
    hsize_t m_count[2] = {hsize_t(num_cols), 1};
    hsize_t f_count[4] = {1, 1, 1, hsize_t(num_cols)};
    for(f_start[1] = 0; f_start[1] < data.size(); ++f_start[1]) {
      for(int i = 0; i < num_rows; ++i) { // iterate over rows of the matrix; columns of the dataset
        const auto & matrix = data[f_start[1]];
        m_start[1] = f_start[2] = i;
        m_dataspace.selectHyperslab(H5S_SELECT_SET, m_count, m_start);
        f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
        ds.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
      }
    }
  }
} 

template<typename T>
void HDF5IOHelper::set_vector_scalar_field(const String &dset_name, const T &data, const String &field_name) {
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_vector_scalar_field(dset_name, ds_iter->second, data, field_name);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_vector_scalar_field(dset_name, ds, data, field_name);
  }
}

/// Set a field on all elements of a 1D dataset of compound type using a ds object.
template<typename T>
void HDF5IOHelper::set_vector_scalar_field(const String &dset_name, H5::DataSet &ds,
                     const std::vector<T> &data, const String &field_name) {
  T test_var;
  H5::DataType h5_mem_t = h5_mem_dtype(test_var);
  H5::CompType comp_t(h5_mem_t.getSize());
  comp_t.insertMember(field_name, 0, h5_mem_t);
  ds.write(data.data(), comp_t);
}
 
template<typename T>
void HDF5IOHelper::set_vector_vector_field(const String &dset_name, const T &data, 
    const size_t length, const String &field_name) {
  // the field length could be inferred from the DataType of the DataSet, so it
  // isn't strictly necessary to force the user to provide it. In fact, having
  // separate functions for scalar and vector fields isn't really necessary, because
  // we could detect whether a field is a scalar or array.  But the docs are a little
  // unclear here and I don't want to spend an hour figuring it out.
  auto ds_iter = datasetCache.find(dset_name);
  if( ds_iter != datasetCache.end())
    set_vector_vector_field(dset_name, ds_iter->second, data, length, field_name);
  else {
    H5::DataSet ds = h5File.openDataSet(dset_name);
    set_vector_vector_field(dset_name, ds, data, length, field_name);
  }
}

/// Set a field on all elements of a 1D dataset of compound type using a ds object.
template<typename T>
void HDF5IOHelper::set_vector_vector_field(const String &dset_name, H5::DataSet &ds,
                     const std::vector<T> &data, const size_t length,
                     const String &field_name) {
  T test_var;
  hsize_t dims[1] = {hsize_t(length)}; 
  H5::DataType h5_mem_scalar_t = h5_mem_dtype(test_var);
  H5::ArrayType h5_mem_array_t(h5_mem_scalar_t, 1, dims);
  H5::CompType comp_t(h5_mem_array_t.getSize());
  comp_t.insertMember(field_name, 0, h5_mem_array_t);
  ds.write(data.data(), comp_t);
}
 

template<typename T>
void HDF5IOHelper::append_scalar(const String &dset_name, const T&data) {
  // 1. Open the dataset
  // 2. Discover the rank (must be 1) and current shape
  // 3. Extend the dataset
  // 5. write
  
  H5::DataSet &ds = datasetCache[dset_name];
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 1) {
    flush();
    throw std::runtime_error(String("Attempt to append element to a non-1D datasset ") +
                               dset_name + " failed");
  }
  hsize_t dim, maxdim;
  f_space.getSimpleExtentDims(&dim, &maxdim);
  if(maxdim != H5S_UNLIMITED) {
    flush();
    throw std::runtime_error(String("Attempt to append element to a fixed-sized datasset ") +
                               dset_name + " failed");
  }
  ++dim;
  ds.extend(&dim);
  set_scalar(dset_name, ds, data, dim-1);
}
/// Append a vector as a row or column to a 2D dataset
template<typename T>
void HDF5IOHelper::append_vector(const String &dset_name, const T &data, const bool &row) {
  // 1. open the dataset
  // 2. discover the rank (must be 2) and dimensions
  // 3. Raise an error if the dataset can't be extended
  // 4. Extend
  // 4. Write
  H5::DataSet ds = datasetCache[dset_name];
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 2) {
    flush();
    throw std::runtime_error(String("Attempt to append row or column into non-2D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t dims[2], maxdims[2]; // assume rank == 2
  f_space.getSimpleExtentDims(dims, maxdims);
  int len = length(data); // length is a free function defined in HDF5_IO.hpp that is 
                          // overloaded/templated to return the length of SDVs and std::vectors
  int index;
  if(row) {
     if(maxdims[0] != H5S_UNLIMITED) {
        flush();
        throw std::runtime_error(String("Attempt to append row to  ") + 
                                   dset_name + " failed; dimensions are fixed.");
     }
     index = dims[0]++;
  } else {
    if(maxdims[1] != H5S_UNLIMITED) {
      flush();
      throw std::runtime_error(String("Attempt to append column to  ") + 
                                 dset_name + " failed; dimensions are fixed.");
    }
    index = dims[1]++;      
  }
  ds.extend(dims); 
  set_vector(dset_name, ds, data, index, row); 
}

/// Append a SerialDenseMatrix to a 3D dataset. The dataset will be expanded along the 0th
/// dimension. By default, the shape of the matrix, (nrows, ncols), must match the size of the 1st 
/// and 2nd dimensions of the dataset. For transpose=true, the reverse must be true. 
template<typename T>
void HDF5IOHelper::append_matrix(const String &dset_name, 
                   const Teuchos::SerialDenseMatrix<int, T> &data,
                   const bool &transpose) {
  // 1. open the dataset
  // 2. discover the rank (must be 3) and dimensions
  // 3. Raise an error if the dataset can't be extended
  // 4. Extend
  // 4. Write
  H5::DataSet &ds = datasetCache[dset_name];
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 3) {
    flush();
    throw std::runtime_error(String("Attempt to append matrix to non-3D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t dims[3], maxdims[3]; // assume rank == 3
  f_space.getSimpleExtentDims(dims, maxdims);
  if(maxdims[0] != H5S_UNLIMITED) {
    flush();
    throw std::runtime_error(String("Attempt to append matrix to  ") + 
                                 dset_name + " failed; dimensions are fixed.");
  }
  int expected_nrows = (transpose) ? dims[2] : dims[1];
  int expected_ncols = (transpose) ? dims[1] : dims[2];

  if(data.numRows() != expected_nrows || data.numCols() != expected_ncols) {
      flush();
      throw std::runtime_error(String("Attempt to append matrix to ") +
                               dset_name + " failed; matrix and dataset " +
                               "dimensions do not match");
  }    
  ++dims[0];
  ds.extend(dims);
  // Write. See store_matrix for an example of what to do about the transpose.
  set_matrix(dset_name, ds, data, dims[0]-1, transpose);
}

/// Append a std::vector of  SerialDenseMatrix's to a 4D dataset. The dataset 
/// will be expanded along the 0th dimension. By default, the size of the vector
/// must equal the size of the 1st dimension of the dataset, andthe shape of the 
/// SDMs (nrows, ncols), must match the sizes of the 2nd 
/// and 2nd dimensions of the dataset. For transpose=true, the reverse must 
///  be true of the SDMs.
template<typename T>
void HDF5IOHelper::append_vector_matrix(const String &dset_name, 
                   const std::vector<Teuchos::SerialDenseMatrix<int, T> > &data,
                   const bool &transpose) {
  // 1. open the dataset
  // 2. discover the rank (must be 4) and dimensions
  // 3. Raise an error if the dataset can't be extended
  // 4. Extend
  // 4. Write
  H5::DataSet &ds = datasetCache[dset_name];
  H5::DataSpace f_space = ds.getSpace();
  if(f_space.getSimpleExtentNdims() != 4) {
    flush();
    throw std::runtime_error(String("Attempt to append vector-matrix to non-4D dataset ") + 
                               dset_name + " failed" );
  }
  hsize_t dims[4], maxdims[4];
  f_space.getSimpleExtentDims(dims, maxdims);
  if(maxdims[0] != H5S_UNLIMITED) {
    flush();
    throw std::runtime_error(String("Attempt to append vector-matrix to  ") + 
                                 dset_name + " failed; dimensions are fixed.");
  }
  int expected_nrows = (transpose) ? dims[3] : dims[2];
  int expected_ncols = (transpose) ? dims[2] : dims[3];
  for(const auto &m : data) {
    if(m.numRows() != expected_nrows || m.numCols() != expected_ncols) {
        flush();
        throw std::runtime_error(String("Attempt to append vector-matrix to ") +
                                 dset_name + " failed; matrix and dataset " +
                                 "dimensions do not match");
    }
  }
  ++dims[0];
  ds.extend(dims);
  // Write. See store_matrix for an example of what to do about the transpose.
  set_vector_matrix(dset_name, ds, data, dims[0]-1, transpose);
}

/// Read scalar data from a dataset
template <typename T>
void HDF5IOHelper::read_scalar(const std::string& dset_name, T& val) {
  if( !exists(dset_name) )
  {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\""
         << std::endl;
    abort_handler(-1);
  }
  // JAS: We need some verification here that the dataset is really a scalar.
  H5::DataSet dataset = h5File.openDataSet(dset_name);
  dataset.read(&val, h5_mem_dtype(val));

  return;
}

/// Read vector (1D) information from a dataset
template <typename T>
void HDF5IOHelper::read_vector(const std::string& dset_name, T& array) const 
{
  if( !exists(dset_name) )
  {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\""
         << std::endl;
    abort_handler(-1);
  }

  H5::DataSet dataset = h5File.openDataSet(dset_name);

  // Get dims and size of dataset
  assert( dataset.getSpace().isSimple() );
  int ndims = dataset.getSpace().getSimpleExtentNdims();
  assert( ndims == 1 );
  std::vector<hsize_t> dims( ndims, hsize_t(1) );
  dataset.getSpace().getSimpleExtentDims(dims.data());

  // Calling resize on a SerialDenseVector is potentially wasteful
  // if it is initially non-zero length because it copies the existing
  // data to the new buffer. There's probably a better design that
  // would avoid this issue without duplicating a lot of code for
  // std::vector and SDV.
  array.resize(dims[0]);
  dataset.read(&array[0], h5_mem_dtype(array[0]));
  return;
}





/// Read matrix (2D) information from a dataset
/// Currently this involves a wasteful copy to do the transpose and
/// is intended only for purposes of testing
template <typename T>
void HDF5IOHelper::read_matrix(const std::string &dset_name, 
    Teuchos::SerialDenseMatrix<int,T> & matrix, 
    const bool &transpose) const {
  if( !exists(dset_name) )
  {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\""
         << std::endl;
    abort_handler(-1);
  }

  H5::DataSet dataset = h5File.openDataSet(dset_name);

  // Get dims and size of dataset
  assert( dataset.getSpace().isSimple() );
  int ndims = dataset.getSpace().getSimpleExtentNdims();
  assert( ndims == 2 );
  std::vector<hsize_t> dims(ndims);
  dataset.getSpace().getSimpleExtentDims(dims.data());

  if(transpose) {
    matrix.shape(dims[1], dims[0]);
    dataset.read(&matrix(0,0), h5_mem_dtype(matrix(0,0)));
  } else {
    Teuchos::SerialDenseMatrix<int, T> tmp(dims[1], dims[0]);
    dataset.read(tmp.values(), h5_mem_dtype(tmp(0,0)));
    matrix.shapeUninitialized(dims[0], dims[1]);
    for(int ii = 0; ii < dims[0]; ++ii) 
      for(int jj = 0; jj < dims[1]; ++jj) 
        matrix(ii,jj) = tmp(jj,ii);
    
  }
  return;
}

/// Get the matrix (2D) at the index into the 0th dimension of the 3D
/// dataset at dsetname. 
/// Currently this involves a wasteful copy to do the transpose and
/// is intended only for purposes of testing
template <typename T>
void HDF5IOHelper::get_matrix(const std::string &dset_name, 
    Teuchos::SerialDenseMatrix<int,T> & matrix,
    const int &index, const bool &transpose) const {
  if( !exists(dset_name) )
  {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\""
         << std::endl;
    abort_handler(-1);
  }

  H5::DataSet dataset = h5File.openDataSet(dset_name);
  H5::DataSpace f_dataspace = dataset.getSpace();
  // Get dims and size of dataset
  int ndims = f_dataspace.getSimpleExtentNdims();
  assert( ndims == 3 );
  std::vector<hsize_t> dims(ndims);
  f_dataspace.getSimpleExtentDims(dims.data());
  if(index >= dims[0] || index < 0) {
    flush();
    throw  std::runtime_error(String("Error while getting matrix ") + 
          "slice from 3D dataset " + dset_name + ": index out of range.");
  }
  hsize_t f_start[3] = {hsize_t(index), 0, 0};
  hsize_t f_count[3] = {1, dims[1], dims[2]};
  f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
  // We must create a memory dataspace because if we used the "default",
  // it would be assumed to have the same dimensionality, offset, and count 
  // as the file dataspace
  hsize_t m_dims[2] = {dims[2], dims[1]};
  H5::DataSpace m_dataspace(2, m_dims);
  if(transpose) {
    matrix.shape(dims[2], dims[1]);
    dataset.read(&matrix(0,0), h5_mem_dtype(matrix(0,0)), m_dataspace, f_dataspace);
  } else {
    Teuchos::SerialDenseMatrix<int, T> tmp(dims[2], dims[1]);
    dataset.read(tmp.values(), h5_mem_dtype(tmp(0,0)), m_dataspace, f_dataspace);
    matrix.shapeUninitialized(dims[1], dims[2]);
    for(int ii = 0; ii < dims[1]; ++ii) 
      for(int jj = 0; jj < dims[2]; ++jj) 
        matrix(ii,jj) = tmp(jj,ii);
  }
  return;
}

/// Read the 3D slice at the index into the 0th dimension of the 4D dataset at ds_name. 
/// Currently this involves a wasteful copy to do the transpose and
/// is intended only for purposes of testing
template <typename T>
void HDF5IOHelper::get_vector_matrix(const std::string &dset_name, 
    std::vector<Teuchos::SerialDenseMatrix<int,T> >& data,
    const int &index, const bool &transpose) const {
  if( !exists(dset_name) )
  {
    Cerr << "\nError: HDF5 file \"" << fileName << "\""
         << " does not contain data path \"" << dset_name << "\""
         << std::endl;
    abort_handler(-1);
  }

  H5::DataSet dataset = h5File.openDataSet(dset_name);
  H5::DataSpace f_dataspace = dataset.getSpace();
  // Get dims and size of dataset
  int ndims = f_dataspace.getSimpleExtentNdims();
  assert( ndims == 4 );
  std::vector<hsize_t> dims(ndims);
  f_dataspace.getSimpleExtentDims(dims.data());
  if(index >= dims[0] || index < 0) {
    flush();
    throw  std::runtime_error(String("Error while getting vector-matrix ") + 
          "slice from 4D dataset " + dset_name + ": index out of range.");
  }
  data.resize(dims[1]);
  hsize_t f_start[4] = {hsize_t(index), 0, 0, 0};
  hsize_t f_count[4] = {1, 1, dims[2], dims[3]};
  f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
  // We must create a memory dataspace because if we used the "default",
  // it would be assumed to have the same dimensionality, offset, and count 
  // as the file dataspace
  hsize_t m_dims[2] = {dims[3], dims[2]};
  H5::DataSpace m_dataspace(2, m_dims);
  if(transpose) {
    for(f_start[1] = 0; f_start[1] < dims[1]; ++f_start[1]) {
      auto &matrix = data[f_start[1]];
      matrix.shape(dims[3], dims[2]);
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      dataset.read(matrix.values(), h5_mem_dtype(matrix(0,0)), m_dataspace, f_dataspace);
    }
  } else {
    Teuchos::SerialDenseMatrix<int, T> tmp(dims[3], dims[2]);
    for(f_start[1] = 0; f_start[1] < dims[1]; ++f_start[1]) {
      auto &matrix = data[f_start[1]];
      f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
      dataset.read(tmp.values(), h5_mem_dtype(tmp(0,0)), m_dataspace, f_dataspace);
      matrix.shapeUninitialized(dims[2], dims[3]);
      for(int ii = 0; ii < dims[2]; ++ii) 
        for(int jj = 0; jj < dims[3]; ++jj) 
          matrix(ii,jj) = tmp(jj,ii);
    }
  }
  return;
}

template <typename T>
void HDF5IOHelper::add_attribute(const String &location, const String &label,
                   const T &value) {
  H5::Attribute attr = create_attribute(location, label, value);
  attr.write(h5_mem_dtype(value), &value);
}

/// Store vector data using a pointer to the first element and length
template<typename T>
void HDF5IOHelper::store_vector(const String &dset_name, const T *data,
                       const int &len) const {
  hsize_t dims[1];
  dims[0] = len;
  H5::DataSpace dataspace = H5::DataSpace(1, dims);
  H5::DataType f_datatype = h5_file_dtype(*data);
  H5::DataType m_datatype = h5_mem_dtype(*data);
  // Assume dset_name is syntactically correct - will need some utils - RWH
  create_groups(dset_name);
  H5::DataSet dataset(
    create_dataset(h5File, dset_name, f_datatype, dataspace) );
  dataset.write(data, m_datatype);
  return;
}

template<typename T>
H5::Attribute HDF5IOHelper::create_attribute(const String &location, const String &label,
                                             const T &value) {
  if(! exists(location)) {
    // If it doesn't exist, assume it's a group.
    create_groups(location, false);
  }   

  H5O_type_t type = h5File.childObjType(location.c_str());
  if(type == H5O_TYPE_GROUP) {
    H5::Group group = h5File.openGroup(location);
    return group.createAttribute(label, h5_file_dtype(value), H5::DataSpace());
  } else if(type == H5O_TYPE_DATASET) {
    H5::DataSet dataset = h5File.openDataSet(location);
    return dataset.createAttribute(label, h5_file_dtype(value), H5::DataSpace());
  } else {
    flush();
    throw std::runtime_error(String("HDF5 object at ") + location + " is of unhandled type.");
  }
}



}  // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

