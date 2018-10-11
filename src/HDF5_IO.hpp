/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifndef HDF5_IO_HELPER_HPP
#define HDF5_IO_HELPER_HPP

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

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

// Some free functions to try to consolidate data type specs
inline H5::DataType h5_file_dtype( const Real & )
{ return H5::PredType::IEEE_F64LE; }

inline H5::DataType h5_file_dtype( const int & )
{ return H5::PredType::STD_I32LE; }

inline H5::DataType h5_file_dtype( const char * )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

inline H5::DataType h5_file_dtype( const String )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

inline H5::DataType h5_mem_dtype( const Real & )
{ return H5::PredType::NATIVE_DOUBLE; }

inline H5::DataType h5_mem_dtype( const int & )
{ return H5::PredType::NATIVE_INT; }

inline H5::DataType h5_mem_dtype( const char * )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}

inline H5::DataType h5_mem_dtype( const String )
{
  H5::StrType str_type(0, H5T_VARIABLE);
  str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
  return str_type;
}


/**
 * This helper class provides wrapper functions that perform
 * low-level access operations in HDF5 databases.
 *
 * Authors:  J. Adam Stephens, Russell Hooper, Elliott Ridgway
 */

class HDF5IOHelper
{
  public:

  HDF5IOHelper(const std::string& file_name, bool overwrite = false) :
    fileName(file_name)
  {
    // create or open a file
    //H5::Exception::dontPrint();
    if( overwrite ) {
      filePtr = std::shared_ptr<H5::H5File>(
        new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));
    } else {
      try {
        filePtr = std::shared_ptr<H5::H5File>(
          new H5::H5File(fileName.c_str(), H5F_ACC_RDWR));
      } catch(const H5::FileIException&) {
        filePtr = std::shared_ptr<H5::H5File>(
          new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));
      }
    }
    // Initialize global Link Creation Property List to encode all link
    // (group, dataset) names in UTF-8
    linkCreatePL.setCharEncoding(H5T_CSET_UTF8);      
  }

  ~HDF5IOHelper() { }

  //----------------------------------------------------------------

  template <typename T>
  const void store_scalar_data(const std::string& dset_name, const T& val)
  {
    H5::DataSpace dataspace = H5::DataSpace();
    
    // Assume dset_name is syntactically correct - will need some utils - RWH
    create_groups(dset_name);
	H5::DataSet dataset(create_dataset(
      *filePtr, dset_name, h5_file_dtype(val), dataspace));

    dataset.write(&val, h5_mem_dtype(val));
	return;
  }

  template <typename T>
  const void read_scalar_data(const std::string& dset_name, T& val)
  {
    if( !exists(dset_name) )
    {
      Cerr << "\nError: HDF5 file \"" << fileName << "\""
           << " does not contain data path \"" << dset_name << "\""
           << std::endl;
      abort_handler(-1);
    }
    // JAS: We need some verification here that the dataset is really a scalar.
    H5::DataSet dataset = filePtr->openDataSet(dset_name);
    dataset.read(&val, h5_mem_dtype(val));

    return;
  }

  //----------------------------------------------------------------

  template <typename T>
  void store_vector_data(const std::string& dset_name,
                         const std::vector<T>& array) const
  {
    store_vector_data(dset_name, array.data(), array.size());
    return;
  }

  // Use the same reader for std::vector and SerialDenseVector
  template <typename T>
  void read_vector_data(const std::string& dset_name, T& array) const 
  {
    if( !exists(dset_name) )
    {
      Cerr << "\nError: HDF5 file \"" << fileName << "\""
           << " does not contain data path \"" << dset_name << "\""
           << std::endl;
      abort_handler(-1);
    }

    H5::DataSet dataset = filePtr->openDataSet(dset_name);

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

  template <typename T>
  void store_vector_data( const std::string & dset_name,
                          const Teuchos::SerialDenseVector<int,T> & vec )
  {
    store_vector_data(dset_name, &vec[0], vec.length());
    return;
  }
  
  void store_vector_string_data( const std::string & dset_name,
                                 StringMultiArrayConstView vec )
  {
    store_vector_data(dset_name, &vec[0], vec.size());
    return;
  }
  
  /// Write a 2D dataset from a Teuchos::SerialDenseMatrix
  template<typename T>
  void store_matrix_data(const std::string &dset_name, 
      const Teuchos::SerialDenseMatrix<int,T> & matrix, 
      const bool &transpose = false) const {

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
        create_dataset(*filePtr, dset_name, f_datatype, f_dataspace) );
      dataset.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
    } else {
      // to write an un-tranposed matrix, we use HDF5 hyperslab selections to 
      // sequentially grab rows of the matrix in memory and write them into rows
      // of the dataset in file. This is a little tricky because rows in memory 
      // are non-contiguous due to column-major storage of SDMs.
      m_dims[1] = f_dims[0] = num_rows;
      m_dims[0] = f_dims[1] = num_cols;
      f_dataspace.setExtentSimple(2, f_dims);
      m_dataspace.setExtentSimple(2, m_dims);
      H5::DataSet dataset(
        create_dataset(*filePtr, dset_name, f_datatype, f_dataspace) );
      hsize_t m_start[2], f_start[2];
      m_start[0] = f_start[1] = 0;
      hsize_t m_count[2] = {hsize_t(num_cols), 1};
      hsize_t f_count[2] = {1, hsize_t(num_cols)};
      for(int i = 0; i < num_rows; ++i) {
        m_start[1] = f_start[0] = i;
        m_dataspace.selectHyperslab(H5S_SELECT_SET, m_count, m_start);
        f_dataspace.selectHyperslab(H5S_SELECT_SET, f_count, f_start);
        dataset.write(matrix.values(), m_datatype, m_dataspace, f_dataspace);
      }
    }
    return;
  }

  /// Write a 2D dataset from a std::vector
  template<typename T>
  void store_matrix_data(const std::string &dset_name, 
      const std::vector<T> &buf, const int & num_cols,
      const bool &transpose = false) const {
    
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
        create_dataset(*filePtr, dset_name, f_datatype, f_dataspace) );
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
        create_dataset(*filePtr, dset_name, f_datatype, f_dataspace) );
      dataset.write(buf.data(), m_datatype, m_dataspace, f_dataspace);
    }
    return;
  }


  //------------------------------------------------------------------

  void attach_scale(const String& dset_name, const String& scale_name,
                    const String& label, const int& dim) const;

  template <typename T>
  void add_attribute(const String &location, const String &label,
                     const T &value) {

    if(! exists(location)) {
      // If it doesn't exist, assume it's a group.
      create_groups(location, false);
    }   

    H5::DataSpace dataspace;
    H5O_type_t type = filePtr->childObjType(location.c_str());
    if(type == H5O_TYPE_GROUP) {
      Cout << location << " is a group" << std::endl;
      H5::Group group = filePtr->openGroup(location);
      H5::Attribute attr =
        group.createAttribute(label, h5_file_dtype(value), dataspace);
      attr.write(h5_mem_dtype(value), &value);
    } else if(type == H5O_TYPE_DATASET) {
      Cout << location << " is a dataset" << std::endl;
      Cout << "Attribute is (" << label << ", " << value << ")\n";
      H5::DataSet dataset = filePtr->openDataSet(location);
      H5::Attribute attr =
        dataset.createAttribute(label, h5_file_dtype(value), dataspace);
      attr.write(h5_mem_dtype(value), &value);
    } else {
      Cerr << location << " is an HDF5 object of unhandled type." << std::endl;
    }
  }

  bool exists(const String location_name) const;

  bool is_scale(const H5::DataSet dset) const;

  H5::Group create_groups(const std::string& name,
                          bool includes_dset=true) const;

  // JAS: The current code in ResultsDBHDF5 calls store_vector_data and then
  // attach_scale. This pair of functions may be unneeded.
  H5::DataSet create_dimension_scale ( const H5::H5Location &loc,
                                       std::vector<int> dim_sizes,
                                       H5::DataType type,
                                       std::string label,
                                       H5::DSetCreatPropList plist ) const;

  H5::DataSet create_1D_dimension_scale ( const H5::H5Location &loc,
                                          int size, H5::DataType type,
                                          std::string label,
                                          H5::DSetCreatPropList plist ) const;

  H5::DataSet create_dataset( const H5::H5Location &loc,
                              const std::string &name,
                              const H5::DataType &type,
                              const H5::DataSpace &space,
                              const H5::DSetCreatPropList &plist =
                                H5::DSetCreatPropList()) const;

  // Define globally available custom property lists
  // JAS: These probably should not be public. The point of this class is to
  // encapsulate these kinds of low-level details.
  H5::LinkCreatPropList linkCreatePL;
  H5::DSetCreatPropList datasetCompactPL;
  H5::DSetCreatPropList datasetContiguousPL;

  /// Flush cache to file
  void flush();
 

  protected:

  std::string fileName;

  std::shared_ptr<H5::H5File> filePtr;

  template<typename T>
  void store_vector_data(const String &dset_name, const T *data,
                         const int &len) const {
    hsize_t dims[1];
    dims[0] = len;
    H5::DataSpace dataspace = H5::DataSpace(1, dims);
    H5::DataType f_datatype = h5_file_dtype(*data);
    H5::DataType m_datatype = h5_mem_dtype(*data);
    // Assume dset_name is syntactically correct - will need some utils - RWH
    create_groups(dset_name);
    H5::DataSet dataset(
      create_dataset(*filePtr, dset_name, f_datatype, dataspace) );
    dataset.write(data, m_datatype);
    return;
  }
}; // class HDF5IOHelper
}  // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

