/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_windows.hpp"

#include "Teuchos_SerialDenseHelpers.hpp"
#define BOOST_TEST_MODULE dakota_teuchos_eigen_conversion
#include <boost/test/included/unit_test.hpp>
#include "util_common.hpp"
#include "util_data_types.hpp"

using MatrixMap = Eigen::Map<Eigen::MatrixXd>;
using VectorMap = Eigen::Map<Eigen::VectorXd>;

using namespace dakota;
using namespace dakota::util;

namespace {

// Test data
MatrixXd create_test_matrix_eigen() {
  MatrixXd test_matrix_eigen(3, 2);
  test_matrix_eigen << 0.2, 0.3, 0.4, 1.0, 3.0, 2.0;
  return test_matrix_eigen;
}

RealMatrix create_test_matrix_teuchos() {
  double test_matrix_teuchos_array[] = {0.2, 0.4, 3.0, 0.3, 1.0, 2.0};
  RealMatrix test_matrix_teuchos(Teuchos::Copy, test_matrix_teuchos_array, 3, 3,
                                 2);
  return test_matrix_teuchos;
}

VectorXd create_test_vector_eigen() {
  VectorXd test_vector_eigen(4);
  test_vector_eigen << 1.0, 2.0, 3.0, 4.0;
  return test_vector_eigen;
}

RealVector create_test_vector_teuchos() {
  double test_vector_teuchos_array[] = {1.0, 2.0, 3.0, 4.0};
  RealVector test_vector_teuchos(Teuchos::Copy, test_vector_teuchos_array, 4);
  return test_vector_teuchos;
}

BOOST_AUTO_TEST_CASE(util_matrix_eigen_to_teuchos) {
  MatrixXd eigen_mat = create_test_matrix_eigen();
  RealMatrix teuchos_mat_gold = create_test_matrix_teuchos();

  int num_rows = eigen_mat.rows();
  int num_cols = eigen_mat.cols();

  RealMatrix teuchos_mat(Teuchos::Copy, eigen_mat.data(), num_rows, num_rows,
                         num_cols);

  /*
  std::cout << "\n";
  teuchos_mat.print(std::cout);
  */

  BOOST_CHECK(matrix_equals(teuchos_mat, teuchos_mat_gold, 1.0e-10));
}

BOOST_AUTO_TEST_CASE(util_matrix_teuchos_to_eigen) {
  RealMatrix teuchos_mat = create_test_matrix_teuchos();
  MatrixXd eigen_mat_gold = create_test_matrix_eigen();

  int num_rows = teuchos_mat.numRows();
  int num_cols = teuchos_mat.numCols();

  MatrixMap eigen_mat(teuchos_mat.values(), num_rows, num_cols);

  /*
  std::cout << "\n";
  std::cout << eigen_mat << std::endl;
  */

  BOOST_CHECK(matrix_equals(eigen_mat, eigen_mat_gold, 1.0e-10));
}

BOOST_AUTO_TEST_CASE(util_vector_eigen_to_teuchos) {
  VectorXd eigen_vec = create_test_vector_eigen();
  RealMatrix teuchos_vec_gold = create_test_vector_teuchos();

  int num_rows = eigen_vec.rows();

  RealVector teuchos_vec(Teuchos::Copy, eigen_vec.data(), num_rows);

  /*
  std::cout << "\n";
  teuchos_vec.print(std::cout);
  */

  BOOST_CHECK(matrix_equals(teuchos_vec, teuchos_vec_gold, 1.0e-10));
}

BOOST_AUTO_TEST_CASE(util_vector_teuchos_to_eigen) {
  RealVector teuchos_vec = create_test_vector_teuchos();
  VectorXd eigen_vec_gold = create_test_vector_eigen();

  int num_rows = teuchos_vec.numRows();

  VectorMap eigen_vec(teuchos_vec.values(), num_rows);

  /*
  std::cout << "\n";
  std::cout << eigen_vec << std::endl;
  */

  BOOST_CHECK(matrix_equals(eigen_vec, eigen_vec_gold, 1.0e-10));
}

}  // namespace
