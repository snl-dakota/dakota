
#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 

using namespace Dakota;

namespace {

  RealVectorArray 
    create_test_array(int num_entries, int dim = 1, bool rand = false) {
      RealVectorArray test_array(num_entries);

      for( int i = 0; i < num_entries; ++i ) {
        RealVector & test_vec = test_array[i];
        test_vec.sizeUninitialized(dim);
        if( rand )
          test_vec.random();
        // For now, just fill values using counting numbers - user can modify afterward
        else
          for( int i=0; i < test_vec.length(); ++i )
            test_vec(i) = double(i);
      }
      return test_array;
    }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(data_conversion, rva2rm)
{
  const int NROWS = 5;
  const int NCOLS = 3;
  RealVectorArray test_rva = create_test_array(NROWS, NCOLS, true);
  RealMatrix test_rm;

  /////////////////  What we want to test
  copy_data(test_rva, test_rm);
  /////////////////  What we want to test

  // Verify correct dimensions
  TEST_EQUALITY( NROWS, test_rm.numRows() );
  TEST_EQUALITY( NCOLS, test_rm.numCols() );

  // Verify contents of what we wrote and what we read
  for( size_t i=0; i<NROWS; ++i )
    for( int j=0; j<NCOLS; ++j )
    TEST_FLOATING_EQUALITY( test_rva[i][j], test_rm(i,j), 1.e-14 );
}
//----------------------------------------------------------------
