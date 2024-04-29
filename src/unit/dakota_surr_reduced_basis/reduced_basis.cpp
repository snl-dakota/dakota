/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "ExperimentDataUtils.hpp"
#include "ReducedBasis.hpp"
#include "dakota_tabular_io.hpp"
#include "dakota_data_util.hpp"
#include "dakota_data_io.hpp"
#include "dakota_linear_algebra.hpp"
#include "util_windows.hpp"

#include <string>

#define BOOST_TEST_MODULE dakota_reduced_basis
#include <boost/test/included/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(test_reduced_basis_compute_col_means)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  RealVector column_means;
  compute_col_means(matrix, column_means);
  // --------------- What we are testing

  // Test against Column Means obtained from matlab for the same matrix
  BOOST_CHECK( column_means.length() == 2 );
  BOOST_CHECK_CLOSE( column_means[0], 0.299958060862, 1.e-10 );
  BOOST_CHECK_CLOSE( column_means[1], 0.299814773583, 1.e-10 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_sort_vectors)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  RealVector sorted_vector;
  IntVector  sorted_indices;

  for( int i=0; i<matrix.numCols(); ++i )
  {
    const RealVector & unsrt_vec = Teuchos::getCol(Teuchos::View, matrix, i);

    // starting vector should be unsorted
    BOOST_CHECK( !std::is_sorted(unsrt_vec.values(), unsrt_vec.values()+unsrt_vec.length()) );

    // --------------- What we are testing
    sort_vector(Teuchos::getCol(Teuchos::View, matrix, i), sorted_vector, sorted_indices);
    //sorted_indices.print(std::cout);
    //sorted_vector.print(std::cout);
    // --------------- What we are testing

    // sorted vector should be ... well, sorted
    BOOST_CHECK( std::is_sorted(sorted_vector.values(), sorted_vector.values()+sorted_vector.length()) );
    // but indices reflecting the sorting order should not be sorted
    BOOST_CHECK( !std::is_sorted(sorted_indices.values(), sorted_indices.values()+sorted_indices.length()) );
  }
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_sort_matrix_cols)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  RealMatrix sorted_matrix;
  IntMatrix  sorted_indices;

  // --------------- What we are testing
  sort_matrix_columns(matrix, sorted_matrix, sorted_indices);
  // --------------- What we are testing

  // Correctness checks
  for( int i=0; i<matrix.numCols(); ++i )
  {
    const RealVector & unsrt_vec    = Teuchos::getCol(Teuchos::View,        matrix,  i);
    const RealVector & sorted_vec   = Teuchos::getCol(Teuchos::View, sorted_matrix,  i);
    const IntVector  & sort_indices = Teuchos::getCol(Teuchos::View, sorted_indices, i);

    // starting column vector should be unsorted
    BOOST_CHECK( !std::is_sorted(unsrt_vec.values(), unsrt_vec.values()+unsrt_vec.length()) );
    // sorted matrix column should be ... well, sorted
    BOOST_CHECK( std::is_sorted(sorted_vec.values(), sorted_vec.values()+sorted_vec.length()) );
    // but indices reflecting the sorting order should not be sorted
    BOOST_CHECK( !std::is_sorted(sort_indices.values(), sort_indices.values()+sort_indices.length()) );
  }
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_compute_svd)
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
  BOOST_CHECK( singular_values.length() == 2 );
  BOOST_CHECK_CLOSE( singular_values[0], 4.39718, 1.e-3 );
  BOOST_CHECK_CLOSE( singular_values[1], 1.14583, 1.e-3 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_simple_api)
{
  RealMatrix matrix = get_parameter_and_response_submatrices().first;

  // --------------- What we are testing
  ReducedBasis reduced_basis;
  reduced_basis.set_matrix(matrix);
  reduced_basis.update_svd(false);
  const RealVector & singular_values = reduced_basis.get_singular_values();
  // --------------- What we are testing

  // Test against Singlular Values obtained from matlab for the same matrix
  BOOST_CHECK( singular_values.length() == 2 );
  BOOST_CHECK_CLOSE( singular_values[0], 4.39718, 1.e-3 );
  BOOST_CHECK_CLOSE( singular_values[1], 1.14583, 1.e-3 );

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
  BOOST_CHECK( ierr == 0 );

  reconstructed_mat -= matrix;
  Real diff = 1.0 + reconstructed_mat.normFrobenius();
  BOOST_CHECK_CLOSE( diff, 1.0, 1.e-12 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_simple_api2)
{
  RealMatrix matrix1 = get_parameter_and_response_submatrices().first;
  RealMatrix matrix2 = get_parameter_and_response_submatrices().first;

  // Center columns about their means
  RealVector column_means;
  compute_col_means(matrix1, column_means);
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
  BOOST_CHECK( singular_values1.length() == 2 );
  BOOST_CHECK_CLOSE( singular_values1[0], 1.161389978, 1.e-4 );
  BOOST_CHECK_CLOSE( singular_values1[1], 1.145814014, 1.e-4 );
  BOOST_CHECK( singular_values1.length() == singular_values2.length() );
  for( int i=0; i<singular_values1.length(); ++i )
    BOOST_CHECK_CLOSE( singular_values1[i], singular_values2[i], 1.e-4 );
}

//----------------------------------------------------------------

#include "DakotaApproximation.hpp"
#include "DataMethod.hpp"

// test construction and evaluation of a GP surrogate from data
// matrices; one approximation per response
BOOST_AUTO_TEST_CASE(test_reduced_basis_gp_surr0)
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
  gp_approx.add_array(doe_vars, false, doe_resp, false); // shallow copies
  gp_approx.build();

  // check the value of the surrogate
  RealVector eval_vars(2);
  eval_vars(0) = -1.0;
  eval_vars(1) =  1.0;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), 7.0, 1.e-6);
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_spectral_sums)
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
  
  BOOST_CHECK_CLOSE(eigen_sum, 86.00739691478532, 1.e-12); // from Matlab eig
  BOOST_CHECK_CLOSE(computed_eigen_sum, eigen_sum, 1.e-6);
  BOOST_CHECK_CLOSE(computed_sval_sum, sval_sum, 1.e-6);
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reduced_basis_truncations)
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
  BOOST_CHECK( num_values  == 50 );
  BOOST_CHECK( num_values1 == 50 );
  BOOST_CHECK( num_values2 == 5  );
  BOOST_CHECK( num_values3 == 4  );
  BOOST_CHECK( num_values4 == 5  );

  Real ratio3 = 0.0;
  for( int i=0; i<num_values3; ++i )
    ratio3 += singular_values3(i)*singular_values3(i);
  ratio3 /= reduced_basis.get_eigen_values_sum();
  BOOST_CHECK_CLOSE(ratio3, 0.9994559642736607, 1.e-10);

  Real ratio4 = singular_values(num_values4-1)*singular_values(num_values4-1)/(singular_values(0)*singular_values(0));

  // Create order 1.0 values for comparison
  Real test_val =                 1.0 + ratio4;
  const Real matlab_based_value = 1.0 + 4.602723195500141e-04;
  BOOST_CHECK_CLOSE(test_val, matlab_based_value, 1.e-10);

}

//----------------------------------------------------------------

#ifdef HAVE_DAKOTA_SURROGATES

#include "DakotaSurrogatesGP.hpp"
#include "SurrogatesGaussianProcess.hpp"


// test construction and evaluation of a GP surrogate from data
// matrices; one approximation per response
BOOST_AUTO_TEST_CASE(test_reduced_basis_gp_surr_module0)
{
  // Set things up to correspond to the unit test in
  // src/surrogates/unit/gp_approximation_ts.cpp

  size_t num_vars = 1, num_samples = 7;
  
  // training data
  RealMatrix vars(num_vars, num_samples);
  RealVector resp(num_samples);
  
  vars(0,0) = 0.05536604;
  vars(0,1) = 0.28730518;
  vars(0,2) = 0.30391231;
  vars(0,3) = 0.40768703;
  vars(0,4) = 0.45035059;
  vars(0,5) = 0.52639952;
  vars(0,6) = 0.78853488;

  resp(0) = -0.15149429;
  resp(1) = -0.19689361;
  resp(2) = -0.17323105;
  resp(3) = -0.02379026;
  resp(4) =  0.02013445;
  resp(5) =  0.05011702;
  resp(6) = -0.11678312;
  

  // configure the surrogate
  String approx_type("global_exp_gauss_proc");  // Tom's new GP from surrogate module
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = Dakota::QUIET_OUTPUT;
  SharedApproxData shared_approx_data(approx_type, approx_order, num_vars, 
				      data_order, output_level);

  // construct the GP
  Approximation gp_approx(shared_approx_data);
  gp_approx.add_array(vars, false, resp, false); // shallow copies
  auto gp_derived = 
    std::static_pointer_cast<SurrogatesGPApprox>(gp_approx.approx_rep());
  auto& plist = gp_derived->getSurrogateOpts();
  plist.sublist("Nugget").set("fixed nugget", 1.0e-12);
  // need to override lightweight ctor defaults to get historical behavior
  plist.sublist("Nugget").set("estimate nugget", false);
  plist.set("standardize response", false);
  gp_approx.build();

  // check the value of the surrogate
  // Note the tolerance here had to be relaxed from 1.e-8 used in the original
  // unit test mentioned above.
  RealVector eval_vars(num_vars);  // Dakota::Approximation only supports single eval pt
  eval_vars(0) = 0.4;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), -0.0333528, 1.e-4);
  eval_vars(0) = 1.0;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), -0.0506785, 1.e-4);
}

//----------------------------------------------------------------


// test construction and evaluation of a multi-var GP surrogate from data
// matrices
BOOST_AUTO_TEST_CASE(test_reduced_basis_gp_surr_module1)
{
  // Set things up to correspond to the unit test in
  // src/surrogates/unit/gp_approximation_ts.cpp

  size_t num_vars = 2, num_samples = 64;
  
  // training data
  RealMatrix vars(num_vars, num_samples);
  RealVector resp(num_samples);
  

  // These are taken from the corresponding unit test data (read from a file)
  vars(0,0 ) = -0.261288;   vars(1,0 ) =   0.814009;     resp(0 ) = 0.882224;
  vars(0,1 ) = -0.982836;   vars(1,1 ) =    -1.0054;     resp(1 ) =  1.03762;
  vars(0,2 ) =  -1.78722;   vars(1,2 ) =   0.179717;     resp(2 ) = 0.511201;
  vars(0,3 ) =  0.365971;   vars(1,3 ) =     1.5384;     resp(3 ) = 0.674006;
  vars(0,4 ) =   1.26108;   vars(1,4 ) =  -0.106859;     resp(4 ) = 0.781579;
  vars(0,5 ) = -0.799412;   vars(1,5 ) =   0.309589;     resp(5 ) = 0.881064;
  vars(0,6 ) =   1.96176;   vars(1,6 ) =   -1.17439;     resp(6 ) = 0.391393;
  vars(0,7 ) =  -1.44696;   vars(1,7 ) = -0.0151629;     resp(7 ) = 0.698452;
  vars(0,8 ) = -0.674207;   vars(1,8 ) =   -1.47158;     resp(8 ) = 0.821806;
  vars(0,9 ) =  0.241617;   vars(1,9 ) =   0.111534;     resp(9 ) = 0.705623;
  vars(0,10) =  0.376523;   vars(1,10) =    1.61501;     resp(10) = 0.618657;
  vars(0,11) =   1.46547;   vars(1,11) =    1.70874;     resp(11) = 0.494227;
  vars(0,12) =  -1.16948;   vars(1,12) =   -1.27764;     resp(12) = 0.932841;
  vars(0,13) = -0.609541;   vars(1,13) =    1.64353;     resp(13) = 0.638165;
  vars(0,14) =  0.267278;   vars(1,14) =   -1.54597;     resp(14) = 0.679864;
  vars(0,15) =  -1.68987;   vars(1,15) =   -1.11799;     resp(15) = 0.684212;
  vars(0,16) =   1.90157;   vars(1,16) =   -1.65917;     resp(16) = 0.314571;
  vars(0,17) =    1.6058;   vars(1,17) =   0.484434;     resp(17) = 0.654061;
  vars(0,18) =   1.20726;   vars(1,18) =   -1.20618;     resp(18) = 0.953061;
  vars(0,19) = -0.396448;   vars(1,19) =   -1.85506;     resp(19) =  0.49583;
  vars(0,20) =   1.06894;   vars(1,20) =  -0.968341;     resp(20) =  1.04835;
  vars(0,21) =  0.933847;   vars(1,21) =  -0.764871;     resp(21) =  1.04701;
  vars(0,22) =  -1.52112;   vars(1,22) =    1.14615;     resp(22) = 0.809662;
  vars(0,23) =   1.00061;   vars(1,23) =  -0.463758;     resp(23) = 0.948945;
  vars(0,24) = -0.691818;   vars(1,24) =   0.429882;     resp(24) = 0.902627;
  vars(0,25) = 0.0298571;   vars(1,25) =   -1.36792;     resp(25) = 0.736349;
  vars(0,26) =  -1.11944;   vars(1,26) =   -1.61333;     resp(26) = 0.741096;
  vars(0,27) =  -1.59686;   vars(1,27) =   0.526729;     resp(27) = 0.718751;
  vars(0,28) = -0.230589;   vars(1,28) =   0.603084;     resp(28) = 0.827726;
  vars(0,29) =    -1.209;   vars(1,29) =    1.02397;     resp(29) =  1.00943;
  vars(0,30) = .00584672;   vars(1,30) =  -0.613008;     resp(30) = 0.785638;
  vars(0,31) =   1.37895;   vars(1,31) =    1.06479;     resp(31) =  0.90232;
  vars(0,32) = -0.461749;   vars(1,32) =    1.32706;     resp(32) = 0.830706;
  vars(0,33) =  0.455753;   vars(1,33) =   0.773958;     resp(33) = 0.955765;
  vars(0,34) =   1.84068;   vars(1,34) =  -0.703802;     resp(34) =  0.48843;
  vars(0,35) = -0.865699;   vars(1,35) =   -1.75609;     resp(35) = 0.643893;
  vars(0,36) =  0.629012;   vars(1,36) =    1.42598;     resp(36) = 0.835565;
  vars(0,37) =  0.554402;   vars(1,37) =    1.96013;     resp(37) = 0.384589;
  vars(0,38) =   1.74852;   vars(1,38) =    1.83723;     resp(38) = 0.285401;
  vars(0,39) = -0.350333;   vars(1,39) =    1.75192;     resp(39) = 0.499127;
  vars(0,40) =  0.135358;   vars(1,40) =   -1.38036;     resp(40) =  0.74222;
  vars(0,41) =  -1.98395;   vars(1,41) =   -0.35309;     resp(41) = 0.403777;
  vars(0,42) = 0.0852846;   vars(1,42) =      0.911;     resp(42) = 0.857664;
  vars(0,43) =  -1.43059;   vars(1,43) =  -0.647597;     resp(43) = 0.840353;
  vars(0,44) =  -1.27827;   vars(1,44) =  -0.243277;     resp(44) = 0.799552;
  vars(0,45) = -0.936306;   vars(1,45) =   -1.90444;     resp(45) = 0.530518;
  vars(0,46) =  0.599286;   vars(1,46) =   0.012148;     resp(46) = 0.801873;
  vars(0,47) =  -1.04401;   vars(1,47) =   0.247164;     resp(47) = 0.867282;
  vars(0,48) =  -0.51166;   vars(1,48) =    1.88134;     resp(48) = 0.428026;
  vars(0,49) =  -1.89359;   vars(1,49) =  -0.928296;     resp(49) = 0.538805;
  vars(0,50) =  -1.37463;   vars(1,50) =  -0.816837;     resp(50) = 0.906656;
  vars(0,51) =   1.79545;   vars(1,51) =  -0.506636;     resp(51) = 0.493815;
  vars(0,52) =  -1.63354;   vars(1,52) =   0.997715;     resp(52) = 0.756147;
  vars(0,53) =  0.690646;   vars(1,53) =  -0.134874;     resp(53) = 0.833873;
  vars(0,54) =   1.35672;   vars(1,54) =   0.370003;     resp(54) = 0.798756;
  vars(0,55) =   1.18706;   vars(1,55) =  -0.423612;     resp(55) =  0.88706;
  vars(0,56) =  0.983463;   vars(1,56) =   -1.98578;     resp(56) = 0.479354;
  vars(0,57) = -0.178183;   vars(1,57) =    1.26393;     resp(57) = 0.789909;
  vars(0,58) =  0.809034;   vars(1,58) =   0.718326;     resp(58) =  1.05574;
  vars(0,59) =  0.116768;   vars(1,59) =    1.44083;     resp(59) =  0.68804;
  vars(0,60) =  -1.87266;   vars(1,60) =  -0.257825;     resp(60) = 0.461967;
  vars(0,61) =    1.5438;   vars(1,61) =    1.23605;     resp(61) = 0.722748;
  vars(0,62) =  0.870909;   vars(1,62) =   0.683049;     resp(62) =  1.05277;
  vars(0,63) =   1.67871;   vars(1,63) =   -1.69746;     resp(63) = 0.430116;

  // configure the surrogate
  String approx_type("global_exp_gauss_proc");  // Tom's new GP from surrogate module
  UShortArray approx_order;
  short data_order = 1;  // assume only function values
  short output_level = Dakota::QUIET_OUTPUT;
  SharedApproxData shared_approx_data(approx_type, approx_order, num_vars, 
				      data_order, output_level);

  // construct the GP
  Approximation gp_approx(shared_approx_data);
  gp_approx.add_array(vars, false, resp, false); // shallow copies
  SurrogatesGPApprox& gp_derived =
    *std::dynamic_pointer_cast<SurrogatesGPApprox>(gp_approx.approx_rep());
  auto& plist = gp_derived.getSurrogateOpts();
  plist.sublist("Nugget").set("fixed nugget", 1.0e-10);
  // need to override lightweight ctor defaults to get historical behavior
  plist.sublist("Nugget").set("estimate nugget", false);
  plist.sublist("Trend").set("estimate trend", false);
  plist.set("gp seed", 42);
  plist.set("standardize response", false);
  gp_approx.build();

  // check the value of the surrogate
  RealVector eval_vars(2);

  const double tol = 1.e-5;

  eval_vars(0) = 0.20;  eval_vars(1) = 0.45;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), 0.782031, 100.0*tol);

  eval_vars(0) = -0.30;  eval_vars(1) = -0.70;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), 0.847901, 100.0*tol);

  eval_vars(0) = 0.40;  eval_vars(1) = -0.10;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), 0.744622, 100.0*tol);

  eval_vars(0) = -0.25;  eval_vars(1) = 0.33;
  BOOST_CHECK_CLOSE(gp_approx.value(eval_vars), 0.745461, 100.0*tol);
}

#endif

