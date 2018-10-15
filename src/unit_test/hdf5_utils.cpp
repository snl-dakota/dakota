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
#include "HDF5_IO.hpp"

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



TEUCHOS_UNIT_TEST(hdf5_cpp, scalar)
{
  const std::string file_name("hdf5_scalar.h5");
  const std::string ds_name("/SomeScalarData");

  Real value = 1.23;

  // Write data
  {
    HDF5IOHelper h5_io(file_name, /* overwrite */ true);
    h5_io.store_scalar_data(ds_name, value);
  }

  // Read data
  {
    HDF5IOHelper h5_io(file_name);
    h5_io.read_scalar_data(ds_name, value);
  }

  TEST_ASSERT( true );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(hdf5_cpp, realvec)
{
  const std::string file_name("hdf5_basic_realvec.h5");
  const std::string ds_name("/Level_1/level_2/SomeRealVectorData");

  RealVector vec_out(VEC_SIZE);
  vec_out.random();

  // Write data
  {
    HDF5IOHelper h5_io(file_name, /* overwrite */ true);
    h5_io.store_vector_data(ds_name, vec_out);
  }

  // Read data
  RealVector test_vec(VEC_SIZE);
  {
    HDF5IOHelper h5_io(file_name);
    h5_io.read_vector_data(ds_name, test_vec);
  }

  TEST_EQUALITY( test_vec.length(), vec_out.length() );
  double diff = max_diff( vec_out, test_vec );
  TEST_COMPARE( diff, <, 1.e-15 );
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(hdf5_cpp, stdvec)
{
  const std::string file_name("hdf5_basic_stdvec.h5");
  const std::string ds_name("/Level_One/level_two/2/b/SomeStdVectorData");

  RealVector rvec(VEC_SIZE);
  rvec.random();

  std::vector<Real> vec_out(VEC_SIZE);
  int i = 0;
  for( auto & e : vec_out ) 
    e = rvec[i++];

  // Write data
  {
    HDF5IOHelper h5_io(file_name, /* overwrite */ true);
    h5_io.store_vector_data(ds_name, vec_out);
  }

  // Read data
  std::vector<Real> test_vec(VEC_SIZE);
  {
    HDF5IOHelper h5_io(file_name);
    h5_io.read_vector_data(ds_name, test_vec);
  }

  TEST_EQUALITY( test_vec.size(), vec_out.size() );
  TEST_COMPARE_ARRAYS( vec_out, test_vec );
}

//----------------------------------------------------------------

#endif
