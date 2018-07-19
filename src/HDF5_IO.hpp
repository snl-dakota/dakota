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
  inline H5::PredType h5_dtype( const Real & )
    { return H5::PredType::IEEE_F64LE; }

  inline H5::PredType h5_dtype( const int & )
    { return H5::PredType::IEEE_F64LE; }

//----------------------------------------------------------------

  class HDF5IOHelper
  {
    public:

      HDF5IOHelper(const std::string& file_name, bool overwrite = false) :
        fileName(file_name)
    {
      // create or open a file
      H5::Exception::dontPrint();

      if( overwrite )
        filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));

      try {
        filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_RDWR));
      } catch(const H5::FileIException&) {
        filePtr = std::shared_ptr<H5::H5File>(new H5::H5File(fileName.c_str(), H5F_ACC_TRUNC));
      }
    }

      //----------------------------------------------------------------

      /// destructor
      ~HDF5IOHelper() { }

      //----------------------------------------------------------------

      template <typename T>
        void store_scalar_data(const std::string& dset_name, const T& val) const
        {
          H5::DataSpace dataspace = H5::DataSpace();

          // Assume dset_name is syntactically correct - will need some utils - RWH
          create_groups(dset_name);
          H5::DataSet dataset(filePtr->createDataSet( dset_name, h5_dtype(val), dataspace) );

          dataset.write(&val, h5_dtype(val));
          dataset.close(); // does this flush the buffer; is it needed ? 

          return;
        }

      template <typename T>
        void read_data(const std::string& dset_name, T& val) const
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
          create_groups(dset_name);
          H5::DataSet dataset(filePtr->createDataSet( dset_name, h5_dtype(array[0]), dataspace) );

          dataset.write(array.data(), h5_dtype(array[0]));

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

      //----------------------------------------------------------------

      template <typename T>
        void store_vector_data(const std::string & dset_name,
                          const Teuchos::SerialDenseVector<int,T> & vec)
        {
          // This is kludgy but gets things moving ...
          // We should avoid a copy, but do we know whether or not SerialDenseMatrix data is
          // contiguous? RWH
          std::vector<T> copy_vec;
          for( int i=0; i<vec.length(); ++i )
            copy_vec.push_back(vec[i]);

          return store_vector_data(dset_name, copy_vec);
        }


      template <typename T>
        void read_data(const std::string& dset_name, Teuchos::SerialDenseVector<int,T> & buf) const
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


    protected:

      std::string fileName;

      std::shared_ptr<H5::H5File> filePtr;

      //----------------------------------------------------------------

      /** Assume we have an absolute path /root/dir/dataset and create
        groups /root/ and /root/dir/ if needed */
      inline void create_groups(const std::string& dset_name) const
      {
        // the first group will be empty due to leading delimiter
        // the last group will be the dataset name
        std::vector<std::string> groups;
        boost::split(groups, dset_name, boost::is_any_of("/"));

        // index instead of pruning first and last or clever iterators
        std::string full_path;
        for( size_t i=1; i<(groups.size()-1); ++i )
        {
          full_path += '/' + groups[i];
          // if doesn't exist, add

          htri_t grpexists = H5Lexists(filePtr->getId(), full_path.c_str(), H5P_DEFAULT);
          if( grpexists == 0 )
          {
            hid_t create_status = H5Gcreate(filePtr->getId(), full_path.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

            if (create_status < 0)
            {
              Cerr << "\nError: Could not create group hierarchy \"" << full_path << "\""
                   << " for HDF5 file \"" << fileName << "\"."
                   << std::endl;
              abort_handler(-1);
            }

            // I think needed to avoid resource leaks:
            H5Gclose(create_status);
          }
          else if (grpexists < 0)
          {
            Cerr << "\nError: Could not query group hierarchy \"" << full_path << "\""
                 << " for HDF5 file \"" << fileName << "\"."
                 << std::endl;
            abort_handler(-1);
          }
        }
      }

  };

} // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

