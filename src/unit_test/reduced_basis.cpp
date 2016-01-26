
#include "ExperimentDataUtils.hpp"
#include "ReducedBasis.hpp"
#include "dakota_tabular_io.hpp"
#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"
#include "dakota_linear_algebra.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp> 
#include <Teuchos_SerialDenseHelpers.hpp>

using namespace Dakota;

namespace {

  const int NUM_ROWS = 100;
  const int NUM_COLS = 53;

  // Create submatrix consisting of the two input parameter columns
  std::pair<RealMatrix, RealMatrix> get_parameter_and_response_submatrices() {
    static RealMatrix param_sub_mat;
    static RealMatrix response_sub_mat;
    if (param_sub_mat.empty() || response_sub_mat.empty())
    {
      const std::string filename = "dakota_tabular_svd.dat";
      std::ifstream in_file;
      TabularIO::open_file(in_file, filename, "unit test test_compute_svd_data");

      RealVectorArray test_data;
      read_fixed_rowsize_data(in_file, test_data, NUM_COLS, true /* row_major */);

      // Convert RealVectorArray --> RealMatrix
      RealMatrix test_mat;
      copy_data(test_data, test_mat);

      //test_mat.print(std::cout);

      // Create submatrix consisting of the two input parameter columns
      param_sub_mat = RealMatrix(Teuchos::Copy, test_mat, test_mat.numRows(), 2, 0, 1);

      // Create submatrix consisting of the 50 response columns
      response_sub_mat = RealMatrix(Teuchos::Copy, test_mat, test_mat.numRows(), 50, 0, 3);
    }

    return std::make_pair(param_sub_mat, response_sub_mat);
  }
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, compute_col_means)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  RealVector column_means;
  compute_column_means(matrix, column_means);
  // --------------- What we are testing

  // Test against Column Means obtained from matlab for the same matrix
  TEST_EQUALITY( column_means.length(), 2 );
  TEST_FLOATING_EQUALITY( column_means[0], 0.299958060862, 1.e-12 )
  TEST_FLOATING_EQUALITY( column_means[1], 0.299814773583, 1.e-12 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, compute_svd)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  RealVector singular_values;
  RealMatrix VTranspose;
  svd(matrix, singular_values, VTranspose);
  // --------------- What we are testing

  //std::cout << "\nSingular values:\n" << std::endl;
  //singular_values.print(std::cout);

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, simple_api)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(matrix);
  reduced_basis.update_svd(false);
  const RealVector & singular_values = reduced_basis.get_singular_values();
  // --------------- What we are testing

  // Test against Singlular Values obtained from matlab for the same matrix
  TEST_EQUALITY( singular_values.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values[0], 4.39718, 1.e-5 )
  TEST_FLOATING_EQUALITY( singular_values[1], 1.14583, 1.e-5 )

  const RealMatrix & U_mat = reduced_basis.get_left_singular_vector();
  const RealMatrix & VT_mat = reduced_basis.get_right_singular_vector_transpose();

  RealMatrix S(2,2);
  S(0,0) = singular_values(0);
  S(1,1) = singular_values(1);

  // Reconstruct matrix using SVD and compare to original (implies original matrix does not get changed).
  RealMatrix reconstructed_mat( matrix.numRows(), matrix.numCols() );
  int ierr = reconstructed_mat.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, U_mat, S, 0.0);
  RealMatrix temp_matrix(reconstructed_mat);
  ierr += reconstructed_mat.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1.0, temp_matrix, VT_mat, 0.0);
  TEST_EQUALITY( ierr, 0 )

  reconstructed_mat -= matrix;
  Real diff = 1.0 + reconstructed_mat.normFrobenius();
  TEST_FLOATING_EQUALITY( diff, 1.0, 1.e-14 )
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, simple_api2)
{
  RealMatrix matrix1 = get_parameter_and_response_submatrices().first;
  RealMatrix matrix2 = get_parameter_and_response_submatrices().first;

  // Center columns about their means
  RealVector column_means;
  compute_column_means(matrix1, column_means);
  RealVector const_vec(matrix1.numRows());
  RealVector col1 = Teuchos::getCol(Teuchos::View, matrix1, 0);
  RealVector col2 = Teuchos::getCol(Teuchos::View, matrix1, 1);
  const_vec.putScalar(column_means(0));
  col1 -= const_vec;
  const_vec.putScalar(column_means(1));
  col2 -= const_vec;

  // --------------- What we are testing
  ReducedBasis reduced_basis;

  reduced_basis.set_matrix(matrix1);
  reduced_basis.update_svd(false);
  const RealVector & singular_values1 = reduced_basis.get_singular_values();

  reduced_basis.set_matrix(matrix2);
  reduced_basis.update_svd(/* true - by default */);
  const RealVector & singular_values2 = reduced_basis.get_singular_values();
  // --------------- What we are testing

  //std::cout << "Original mean-shifted matrix : \n";
  //matrix.print(std::cout);

  //std::cout << "\n\nCorresponding singular values: ["
  //          << singular_values(0) << ", "
  //          << singular_values(1) << "]" << std::endl;
  // Need to test something ...
  TEST_EQUALITY( singular_values1.length(), 2 );
  TEST_FLOATING_EQUALITY( singular_values1[0], 1.161389978, 1.e-6 )
  TEST_FLOATING_EQUALITY( singular_values1[1], 1.145814014, 1.e-6 )
  TEST_EQUALITY( singular_values1.length(), singular_values2.length() );
  for( int i=0; i<singular_values1.length(); ++i )
    TEST_FLOATING_EQUALITY( singular_values1[i], singular_values2[i], 1.e-6 )
}

//----------------------------------------------------------------

#include "DakotaApproximation.hpp"
#include "DataMethod.hpp"

// test construction and evaluation of a GP surrogate from data
// matrices; one approximation per response
TEUCHOS_UNIT_TEST(reduced_basis, gp_surr0)
{
  size_t num_vars = 2, num_samples = 5;
  
  // training data, one sample per row
  RealMatrix doe_vars(num_vars, num_samples);
  RealVector doe_resp(num_samples);
  
  doe_vars(0,0) =  0.0;  doe_vars(1,0) =  0.0; 
  doe_vars(0,1) = -1.0;  doe_vars(1,1) =  1.0; 
  doe_vars(0,2) =  1.0;  doe_vars(1,2) =  1.0; 
  doe_vars(0,3) = -1.0;  doe_vars(1,3) = -1.0; 
  doe_vars(0,4) =  1.0;  doe_vars(1,4) = -1.0; 

  // true response is 2*x1.^2 + 2*x1 + 6*x2 + x2.^3
  doe_resp(0) =   0.0;
  doe_resp(1) =   7.0;
  doe_resp(2) =  11.0;
  doe_resp(3) =  -7.0;
  doe_resp(4) =  -3.0;
  
  // configure the surrogate
  // String approx_type("global_gaussian");  // Dakota GP
  String approx_type("global_kriging");  // Surfpack GP
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = Dakota::QUIET_OUTPUT;
  SharedApproxData shared_approx_data(approx_type, approx_order, num_vars, 
				      data_order, output_level);

  // construct the GP
  Approximation gp_approx(shared_approx_data);
  gp_approx.add(doe_vars, doe_resp);
  gp_approx.build();

  // check the value of the surrogate
  RealVector eval_vars(2);
  eval_vars(0) = -1.0;
  eval_vars(1) =  1.0;
  TEST_FLOATING_EQUALITY(gp_approx.value(eval_vars), 7.0, 1.e-8);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, spectral_sums)
{
  // Use the response submatrix
  RealMatrix matrix = get_parameter_and_response_submatrices().second;

  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(matrix);
  reduced_basis.update_svd();

  RealVector singular_values  = reduced_basis.get_singular_values();

  Real computed_eigen_sum = 0.0;
  Real computed_sval_sum  = 0.0;
  for( int i=0; i<singular_values.length(); ++i ) {
    computed_sval_sum  += singular_values(i);
    computed_eigen_sum += singular_values(i)*singular_values(i);
  }

  Real eigen_sum = reduced_basis.get_eigen_values_sum();
  Real sval_sum  = reduced_basis.get_singular_values_sum();
  
  TEST_FLOATING_EQUALITY(eigen_sum, 86.00739691478532, 1.e-14); // from Matlab eig
  TEST_FLOATING_EQUALITY(computed_eigen_sum, eigen_sum, 1.e-8);
  TEST_FLOATING_EQUALITY(computed_sval_sum, sval_sum, 1.e-8);
}

//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(reduced_basis, truncations)
{
  // Use the response submatrix
  RealMatrix matrix = get_parameter_and_response_submatrices().second;

  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(matrix);
  reduced_basis.update_svd();

  ReducedBasis::Untruncated                trunc1;
  ReducedBasis::NumComponents              trunc2(5);
  ReducedBasis::VarianceExplained          trunc3(0.99);
  ReducedBasis::HeuristicVarianceExplained trunc4(0.99);

  //reduced_basis.get_singular_values(ReducedBasis::Untruncated()).print(std::cout);
  //reduced_basis.get_singular_values(ReducedBasis::VarianceExplained(0.99)).print(std::cout);
  //reduced_basis.get_singular_values(ReducedBasis::HeuristicVarianceExplained(0.99)).print(std::cout);

  RealVector singular_values  = reduced_basis.get_singular_values();
  RealVector singular_values1 = reduced_basis.get_singular_values(trunc1);
  RealVector singular_values2 = reduced_basis.get_singular_values(trunc2);
  RealVector singular_values3 = reduced_basis.get_singular_values(trunc3);
  RealVector singular_values4 = reduced_basis.get_singular_values(trunc4);

  int num_values  = singular_values.length();
  int num_values1 = trunc1.get_num_components(reduced_basis);
  int num_values2 = trunc2.get_num_components(reduced_basis);
  int num_values3 = trunc3.get_num_components(reduced_basis);
  int num_values4 = trunc4.get_num_components(reduced_basis);

  // Test lengths
  TEST_EQUALITY( num_values , 50 );
  TEST_EQUALITY( num_values1, 50 );
  TEST_EQUALITY( num_values2, 5  );
  TEST_EQUALITY( num_values3, 4  );
  TEST_EQUALITY( num_values4, 5  );

  Real ratio3 = 0.0;
  for( int i=0; i<num_values3; ++i )
    ratio3 += singular_values3(i)*singular_values3(i);
  ratio3 /= reduced_basis.get_eigen_values_sum();
  TEST_FLOATING_EQUALITY(ratio3, 0.9994559642736607, 1.e-12);

  Real ratio4 = singular_values(num_values4-1)*singular_values(num_values4-1)/(singular_values(0)*singular_values(0));

  // Create order 1.0 values for comparison
  Real test_val =                 1.0 + ratio4;
  const Real matlab_based_value = 1.0 + 4.602723195500141e-04;
  TEST_FLOATING_EQUALITY(test_val, matlab_based_value, 1.e-12);

}

