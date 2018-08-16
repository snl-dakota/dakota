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

// We are mixing C and C++ APIs here with an eye to eventually adaopt one or the other - RWH
#include "H5Cpp.h"      // C++ API
#include "hdf5.h"       // C   API
#include "hdf5_hl.h"    // C   H5Lite API

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
	inline H5::DataType h5_dtype( const Real & )
	{ return H5::PredType::IEEE_F64LE; }

	inline H5::DataType h5_dtype( const int & )
	{ return H5::PredType::IEEE_F64LE; }

	inline H5::DataType h5_dtype( const char * )
	{
		H5::StrType str_type(0, H5T_VARIABLE);
		str_type.setCset(H5T_CSET_UTF8);  // set character encoding to UTF-8
		return str_type;
	}

	//----------------------------------------------------------------

	class HDF5IOHelper
	{

    public:

	HDF5IOHelper(const std::string& file_name, bool overwrite = false) :
       	fileName(file_name)
	{

		// create or open a file
		//H5::Exception::dontPrint();
		if( overwrite )
			filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));

		try {
			filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_RDWR));
		} catch(const H5::FileIException&) {
			filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));
		}

		// Initialize global Link Creation Property List to enocde all link (group, dataset) names
		// in UTF-8
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
		H5::DataSet dataset(create_dataset(*filePtr, dset_name, h5_dtype(val), dataspace) );

		dataset.write(&val, h5_dtype(val));
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
		dataset.read(&val, h5_dtype(val));

		return;
	}

	//----------------------------------------------------------------

	template <typename T>
	void store_vector_data(const std::string& dset_name, const std::vector<T>& array) const
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
          // Calling resize on a SerialDenseVector is potentially wasteful if it is initially non-zero
          // length because it copies the existing data to the new buffer. There's probably a better
          // design that would avoid this issue without duplicating a lot of code for std::vector and 
          // SDV.
          array.resize(dims[0]);
          dataset.read(&array[0], h5_dtype(array[0]));
          return;
	}

	template <typename T> void store_vector_data(
		const std::string & dset_name,
		const Teuchos::SerialDenseVector<int,T> & vec )
	{
		store_vector_data(dset_name, &vec[0], vec.length());
		return;
	}

	//------------------------------------------------------------------

	void attach_scale(
		const String& dset_name, const String& scale_name,
		const String& label, const int& dim) const;

	bool exists(const String location_name) const;

	bool is_scale(const H5::DataSet dset) const;

	H5::Group create_groups(const std::string& name, bool includes_dset=true) const;

        // JAS: The current code in ResultsDBHDF5 calls store_vector_data and then
        // attach_scale. This pair of functions may be unneeded.
	H5::DataSet create_dimension_scale (
		const H5::H5Location &loc, std::vector<int> dim_sizes, H5::DataType type,
		std::string label, H5::DSetCreatPropList plist ) const;

	H5::DataSet create_1D_dimension_scale (
		const H5::H5Location &loc, int size, H5::DataType type,
		std::string label, H5::DSetCreatPropList plist ) const;

	H5::DataSet create_dataset(
		const H5::H5Location &loc, const std::string &name,
		const H5::DataType &type, const H5::DataSpace &space,
                const H5::DSetCreatPropList &plist = H5::DSetCreatPropList()) const;

        // Define globally available custom property lists
        // JAS: These probably should not be public. The point of this class is to
        // encapsulate these kinds of low-level details.
        H5::LinkCreatPropList linkCreatePL;
        H5::DSetCreatPropList datasetCompactPL;
	H5::DSetCreatPropList datasetContiguousPL;

	protected:

	std::string fileName;

	std::shared_ptr<H5::H5File> filePtr;

  template<typename T>
  void store_vector_data(const String &dset_name, const T *data, const int &len) const {
      hsize_t dims[1];
      dims[0] = len;
      H5::DataSpace dataspace = H5::DataSpace(1, dims);
      H5::DataType datatype = h5_dtype(*data);
      // Assume dset_name is syntactically correct - will need some utils - RWH
      create_groups(dset_name);
      H5::DataSet dataset(create_dataset(*filePtr, dset_name, datatype, dataspace) );
      dataset.write(data, datatype);
      return;
	}


	}; // class HDF5IOHelper
} // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

