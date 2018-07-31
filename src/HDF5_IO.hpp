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
	inline H5::DataType h5_dtype( const Real & );
	inline H5::DataType h5_dtype( const int & );
	inline H5::DataType h5_dtype( const char * );

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
		linkCreatePl.setCharEncoding(H5T_CSET_UTF8);      
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
	const void read_data(const std::string& dset_name, T& val)
	{
		htri_t ds_exists = H5Lexists(filePtr->getId(), dset_name.c_str(), H5P_DEFAULT);
		if( !ds_exists )
		{
			Cerr << "\nError: HDF5 file \"" << fileName << "\""
				 << " does not contain data path \"" << dset_name << "\""
				 << std::endl;
			abort_handler(-1);
		}

		H5::DataSet dataset = filePtr->openDataSet(dset_name);
		dataset.read(&val, h5_dtype(val));

		return;
	}

	//----------------------------------------------------------------

	template <typename T>
	void store_vector_data(const std::string& dset_name, const std::vector<T>& array) const
	{
          hsize_t dims[1];
          dims[0] = array.size();
          H5::DataSpace dataspace = H5::DataSpace(1, dims);

          // Assume dset_name is syntactically correct - will need some utils - RWH
          Cerr << "DEBUG Before creating groups for " << dset_name << std::endl;
          create_groups(dset_name);
          Cerr << "DEBUG Groups created for " << dset_name << std::endl;
          Cerr << "DEBUG Before creating dataset for " << dset_name << std::endl;
          H5::DataSet dataset(create_dataset(*filePtr, dset_name, h5_dtype(array[0]), dataspace) );
          Cerr << "DEBUG dataset created for " << dset_name << std::endl;
          Cerr << "DEBUG Before writing dataset for " << dset_name << std::endl;
          dataset.write(array.data(), h5_dtype(array[0]));
          Cerr << "DEBUG dataset written for " << dset_name << std::endl;
          return;
	}

	template <typename T>
	void read_data(const std::string& dset_name, std::vector<T>& array) const
	{
          htri_t ds_exists = H5Lexists(filePtr->getId(), dset_name.c_str(), H5P_DEFAULT);
          if( !ds_exists )
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

          herr_t ret_val = H5LTget_dataset_info( filePtr->getId(), dset_name.c_str(),
              &dims[0], NULL, NULL );

          array.resize(dims[0]);

          dataset.read(&array[0], h5_dtype(array[0]));

          return;
	}

	template <typename T> void store_vector_data(
		const std::string & dset_name,
		const Teuchos::SerialDenseVector<int,T> & vec )
	{
		// This is kludgy but gets things moving ...
		// We should avoid a copy, but do we know whether or not SerialDenseMatrix data is
		// contiguous? RWH
		std::vector<T> copy_vec;
		for( int i=0; i<vec.length(); ++i )
			copy_vec.push_back(vec[i]);

		store_vector_data(dset_name, copy_vec);
		return;
	}

	template <typename T> void read_data(
		const std::string& dset_name, Teuchos::SerialDenseVector<int,T> & buf) const
	{
    	// This is not ideal in that we are copying data - RWH
		std::vector<T> tmp_vec;
		read_data(dset_name, tmp_vec);

		if( buf.length() != (int) tmp_vec.size() )
			buf.sizeUninitialized(tmp_vec.size());
		for( int i=0; i<buf.length(); ++i )
			buf[i] = tmp_vec[i];

		return;
	}

	//------------------------------------------------------------------

	void attach_scale(
		const String& dset_name, const String& scale_name,
		const String& label, const int& dim) const;

	bool exists(const String location_name) const;

	//------------------------------------------------------------------

	protected:

	std::string fileName;

	std::shared_ptr<H5::H5File> filePtr;

	H5::LinkCreatPropList linkCreatePl;

	//----------------------------------------------------------------

	inline void create_groups(const std::string& dset_name) const;

	inline H5::DataSet create_dataset(
		const H5::H5Location &loc, const std::string &name,
		const H5::DataType &type, const H5::DataSpace &space,
		H5::DSetCreatPropList plist = H5::DSetCreatPropList()) const;

	bool is_scale(const H5::DataSet dset) const;

	}; // class HDF5IOHelper
} // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

