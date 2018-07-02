/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#ifdef DAKOTA_HAVE_HDF5

//#include <limits>
#include <string>
//#include <map>

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "BinaryIO_Helper.hpp"

#include <Teuchos_UnitTestHarness.hpp> 

#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
     using namespace H5;
#ifndef H5_NO_STD
     using std::cout;
     using std::endl;
#endif  // H5_NO_STD
#endif

const H5std_string	FILE_NAME("h5tutr_subset.h5");
const H5std_string	DATASET_NAME("IntArray");

using namespace Dakota;

//----------------------------------------------------------------

namespace {


  template<typename T>
  double
  max_diff( const Teuchos::SerialDenseVector<int,T> & vec1,
            const Teuchos::SerialDenseVector<int,T> & vec2 )
  {
    double diff_val = 0.0;
    double diff_max = 0.0;

    for( int i=0; i<vec1.length(); ++i )
    {
      diff_val = std::abs(vec1[i] - vec2[i]);
      if( diff_max < diff_val )
        diff_max = diff_val;
    }

    return diff_max;
  }

  const size_t VEC_SIZE = 5;
  const size_t MAT_ROWS = 3;

} // anonymous namespace

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(hdf5, scalar)
{
  const std::string file_name("hdf5_scalar.h5");

  Real value = 1.23;

  herr_t status;

  // scope within which file write takes place
  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = true;

    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    status = binary_file.store_data_scalar("/SomeScalar", value);
    TEST_ASSERT(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = false;
    Real val_in = 0;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    // WJB: see hack (hdr file) to make a single val look like a vec of len==1
    status = binary_file.read_data("/SomeScalar", val_in);
    TEST_ASSERT(status >= 0);
    TEST_ASSERT(val_in == value);
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(hdf5, realvec)
{
  const std::string file_name("hdf5_basic_realvec.h5");
  const std::string ds_name("/SomeRealVectorData");

  RealVector vec_out(VEC_SIZE);
  vec_out.random();

  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = true;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    herr_t status = binary_file.store_data(ds_name, vec_out);
    TEST_ASSERT(status >= 0);
  }

  // Now read back in and test correctness
  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = false;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    RealVector test_vec;
    binary_file.read_data(ds_name, test_vec);

    TEST_EQUALITY( test_vec.length(), vec_out.length() );
    double diff = max_diff( vec_out, test_vec );
    TEST_COMPARE( diff, <, 1.e-15 );
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(hdf5, stdvec)
{
  const std::string file_name("hdf5_basic_stdvec.h5");
  const std::string ds_name("/SomeStdVectorData");

  RealVector rvec(VEC_SIZE);
  rvec.random();

  std::vector<Real> vec_out(VEC_SIZE);
  int i = 0;
  for( auto & e : vec_out ) 
    e = rvec[i++];

  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = true;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    herr_t status = binary_file.store_data_array(ds_name, vec_out);
    TEST_ASSERT(status >= 0);
  }

  // Now read back in and test correctness
  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = false;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist, write_file);

    std::vector<Real> test_vec;
    binary_file.read_data(ds_name, test_vec);

    TEST_EQUALITY( test_vec.size(), vec_out.size() );
    TEST_COMPARE_ARRAYS( vec_out, test_vec );
  }
}

//----------------------------------------------------------------

#endif
