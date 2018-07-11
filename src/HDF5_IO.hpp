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
#include <cmath>
#include <string>
#include <vector>


namespace Dakota
{

//----------------------------------------------------------------

  // Some free functions to try to consolidate data type specs
  PredType h5_dtype( const Real & )
    { return PredType::IEEE_F64LE; }

  PredType h5_dtype( const int & )
    { return PredType::IEEE_F64LE; }

//----------------------------------------------------------------

  class HDF5IOHelper
  {
    public:

      HDF5IOHelper(const std::string& file_name) :
        fileName(file_name)
        {
          filePtr = new H5File(fileName, H5F_ACC_TRUNC);
        }

      //----------------------------------------------------------------

      /// destructor
      ~HDF5IOHelper()
      {
        filePtr->close();
      }

      //----------------------------------------------------------------

      template <typename T>
        void store_scalar_data(const std::string& dset_name, const T& val) const
        {
          DataSpace dataspace = DataSpace();

          // Assume dset_name is syntactically correct - will need some utils - RWH
          DataSet dataset(filePtr->createDataSet( dset_name, h5_dtype(val), dataspace) );

          dataset.write(&val, h5_dtype(val));

          return;
        }

      template <typename T>
        void read_data(const std::string& dset_name, T& val) const
        {
          // WIP ...
          return;
        }

      //----------------------------------------------------------------

      template <typename T>
        void store_vector_data(const std::string& dset_name, const std::vector<T>& array) const
        {
          hsize_t dims[1];
          dims[0] = array.size();
          DataSpace dataspace = DataSpace(1, dims);

          // Assume dset_name is syntactically correct - will need some utils - RWH
          DataSet dataset(filePtr->createDataSet( dset_name, h5_dtype(array[0]), dataspace) );

          dataset.write(array.data(), h5_dtype(array[0]));

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


    protected:

      std::string fileName;

      H5File* filePtr;
  };

} // namespace Dakota

#endif // HDF5_IO_HELPER_HPP

