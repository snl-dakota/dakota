/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "Eigen/Dense"
#include <Teuchos_UnitTestHarness.hpp>
#include "Teuchos_SerialDenseHelpers.hpp"

using Eigen::MatrixXd;
using Eigen::VectorXd;

using MatrixMap = Eigen::Map<MatrixXd>;
using VectorMap = Eigen::Map<VectorXd>;

using RealMatrix = Teuchos::SerialDenseMatrix<int,double>;
using RealVector = Teuchos::SerialDenseVector<int,double>;

namespace dakota {
namespace util {

namespace {

// DTS: Copied from the DataScaler. Should make a separate file
// that contains test support functions.
void error(const std::string msg)
{
  throw(std::runtime_error(msg));
}
bool matrix_equals(const RealMatrix &A, const RealMatrix &B, double tol)
{
  if ( (A.numRows()!=B.numRows()) || (A.numCols()!=B.numCols())){
    std::cout << A.numRows() << "," << A.numCols() << std::endl;
    std::cout << B.numRows() << "," << B.numCols() << std::endl;
    error("matrix_equals() matrices sizes are inconsistent");
  }
  for (int j=0; j<A.numCols(); j++){
    for (int i=0; i<A.numRows(); i++){
      if (std::abs(A(i,j)-B(i,j))>tol)
	    return false;
    }
  }
  return true;
}

bool matrix_equals(const MatrixXd &A, const MatrixXd &B, double tol)
{
  if ( (A.rows()!=B.rows()) || (A.cols()!=B.cols())){
    std::cout << A.rows() << "," << A.cols() << std::endl;
    std::cout << B.rows() << "," << B.cols() << std::endl;
    error("matrix_equals() matrices sizes are inconsistent");
  }
  for (int j=0; j<A.cols(); j++){
    for (int i=0; i<A.rows(); i++){
      if (std::abs(A(i,j)-B(i,j))>tol)
	    return false;
    }
  }
  return true;
}

// Test data
MatrixXd create_test_matrix_eigen() {
  MatrixXd test_matrix_eigen(3,2);
  test_matrix_eigen << 0.2, 0.3,
                       0.4, 1.0,
                       3.0, 2.0;
  return test_matrix_eigen;
}

RealMatrix create_test_matrix_teuchos() {
  double test_matrix_teuchos_array[] = {0.2, 0.4, 3.0, 0.3, 1.0, 2.0};
  RealMatrix test_matrix_teuchos(Teuchos::Copy,
                                 test_matrix_teuchos_array,3,3,2);
  return test_matrix_teuchos;
}

VectorXd create_test_vector_eigen() {
  VectorXd test_vector_eigen(4);
  test_vector_eigen << 1.0, 2.0, 3.0, 4.0;
  return test_vector_eigen;
}

RealVector create_test_vector_teuchos() {
  double test_vector_teuchos_array[] = {1.0, 2.0, 3.0, 4.0};
  RealVector test_vector_teuchos(Teuchos::Copy,
                                 test_vector_teuchos_array,4);
  return test_vector_teuchos;
}


TEUCHOS_UNIT_TEST(util, matrix_eigen_to_teuchos)
{
  MatrixXd eigen_mat = create_test_matrix_eigen();
  RealMatrix teuchos_mat_gold = create_test_matrix_teuchos();

  int num_rows = eigen_mat.rows();
  int num_cols = eigen_mat.cols();

  RealMatrix teuchos_mat(Teuchos::View,eigen_mat.data(),
             num_rows,num_rows,num_cols);

  /*
  std::cout << "\n";
  teuchos_mat.print(std::cout);
  */

  TEST_ASSERT(matrix_equals(teuchos_mat, teuchos_mat_gold, 1.0e-10));

}

TEUCHOS_UNIT_TEST(util, matrix_teuchos_to_eigen)
{

  RealMatrix teuchos_mat = create_test_matrix_teuchos();
  MatrixXd eigen_mat_gold = create_test_matrix_eigen();

  int num_rows = teuchos_mat.numRows();
  int num_cols = teuchos_mat.numCols();

  MatrixMap eigen_mat(teuchos_mat.values(),num_rows,num_cols);

  /*
  std::cout << "\n";
  std::cout << eigen_mat << std::endl;
  */

  TEST_ASSERT(matrix_equals(eigen_mat, eigen_mat_gold, 1.0e-10));

}

TEUCHOS_UNIT_TEST(util, vector_eigen_to_teuchos)
{
  VectorXd eigen_vec = create_test_vector_eigen();
  RealMatrix teuchos_vec_gold = create_test_vector_teuchos();

  int num_rows = eigen_vec.rows();

  RealVector teuchos_vec(Teuchos::View,eigen_vec.data(),num_rows);

  /*
  std::cout << "\n";
  teuchos_vec.print(std::cout);
  */

  TEST_ASSERT(matrix_equals(teuchos_vec, teuchos_vec_gold, 1.0e-10));

}

TEUCHOS_UNIT_TEST(util, vector_teuchos_to_eigen)
{

  RealVector teuchos_vec = create_test_vector_teuchos();
  VectorXd eigen_vec_gold = create_test_vector_eigen();

  int num_rows = teuchos_vec.numRows();

  VectorMap eigen_vec(teuchos_vec.values(),num_rows);

  /*
  std::cout << "\n";
  std::cout << eigen_vec << std::endl;
  */

  TEST_ASSERT(matrix_equals(eigen_vec, eigen_vec_gold, 1.0e-10));

}

}
}  // namespace util
}  // namespace dakota
