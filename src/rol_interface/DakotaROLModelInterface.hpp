#pragma once
#ifndef DAKOTA_ROL_MODEL_INTERFACE_HPP
#define DAKOTA_ROL_MODEL_INTERFACE_HPP

namespace rol_interface {

class ModelInterface {
 public:
  ModelInterface(Optimizer* opt);
  ModelInterface() = delete;
  virtual ~ModelInterface() = default;

  static constexpr short asVal = 1;
  static constexpr short asValGrad = 3;
  static constexpr short asValGradHess = 7;

  void set_default_parameters(Optimizer* opt);

  void update(const Dakota::RealVector& x, ROL::UpdateType type, int iter = -1);

  void set_dimension(Objective* obj);

  // ModelInterface is a Visitor type object for Constraint classes
  void set_dimensions(LinearInequalityConstraint* con);
  void set_dimensions(LinearEqualityConstraint* con);
  void set_dimensions(NonlinearInequalityConstraint* con);
  void set_dimensions(NonlinearEqualityConstraint* con);

  /// Only set value data for nonlinear constraints as
  /// linear constraint values are computed by multiplying
  /// a vector by the Jacobian
  void set_value(NonlinearInequalityConstraint* con);
  void set_value(NonlinearEqualityConstraint* con);

  void set_jacobian(LinearInequalityConstraint* con);
  void set_jacobian(LinearEqualityConstraint* con);
  void set_jacobian(NonlinearInequalityConstraint* con);
  void set_jacobian(NonlinearEqualityConstraint* con);

  void set_equality_target(LinearEqualityConstraint* con);
  void set_equality_target(NonlinearEqualityConstraint* con);

  // Access to Dakota::Model::Respose
  const Dakota::RealVector& get_values() const noexcept;
  const Dakota::RealMatrix& get_gradients() const noexcept;
  const Dakota::RealSymMatrixArray& get_hessians() const noexcept;

  /// provide access to BLAS for use by objective and constraints
  inline Teuchos::BLAS<int, Dakota::Real>& get_blas() { return blasLib; }

  /// provide access to LAPACK for use by objective and constraints
  inline Teuchos::LAPACK<int, Dakota::Real>& get_lapack() { return lapackLib; }

  inline bool has_first_derivatives() const { return !useDefaultDeriv1; }
  inline bool has_second_derivatives() const { return !useDefaultDeriv2; }

 private:
  Optimizer* optPtr;

  // Ptr<RealVector>   xCurrent, xModelInterface, xTemp;
  Dakota::ActiveSet evalSet;
  Dakota::Model& dakotaModel;
  Dakota::RealVector prevX;

  Teuchos::BLAS<int, Dakota::Real> blasLib;
  Teuchos::LAPACK<int, Dakota::Real> lapackLib;

  bool useDefaultDeriv1 = true;
  bool useDefaultDeriv2 = true;
  bool useCenteredDifferences = false;
};  // class ModelInterface

}  // namespace rol_interface

#endif  // DAKOTA_ROL_MODEL_INTERFACE_HPP
