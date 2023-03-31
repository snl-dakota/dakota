#include "DakotaROLInterface.hpp"

namespace rol_interface {

ModelInterface::ModelInterface( Dakota::Model& model ) 
  : dakotaModel(model),
    evalSet(model.current_response().active_set()) { 

  auto grad_type     = dakotaModel.gradient_type();
  auto hess_type     = dakotaModel.hessian_type();
  auto method_src    = dakotaModel.method_source();
  auto interval_type = dakotaModel.interval_type();  

  // initialize ActiveSet request values
  if( grad_type == "numerical" && method_src == "vendor" ) 
    evalSet.request_values(asVal);
  else if( dakotaModel.hessian_type() == "none" ) 
    evalSet.request_values(asValGrad);
  else
    evalSet.request_values(asValGradHess);
  
  if( grad_type == "analytic" || grad_type == "mixed" ||
    ( grad_type == "numerical" && method_src == "dakota" ) ){
    useDefaultDeriv1 = true;
    useDefaultDeriv2 = ( hess_type != "none" ) 
  } 

  useCenteredDifferences = (interval_type == "central");
}
                 

const Dakota::RealMatrix& 
ModelInterface::get_jacobian_matrix( Constraint::Type type ) {
  int num_rows = dakotaModel.cv();
  int num_cols = 0;
  Dakota::Real* const jac_data_ptr;
  ROL::Ptr<Jacobian> jacobian;
  switch(type) {
    case Constraint::Type::LinearInequality: 
      const Dakota::RealMatrix& J = dakotaModel.lin_ineq_coeffs();
      jac_data_ptr = J.values();
      num_cols = dakotaModel.num_linear_ineq_constraints();
    break;
    case Constraint::Type::LinearEquality: 
      const Dakota::RealMatrix& J = dakotaModel.lin_eq_coeffs();
      num_cols = dakotaModel.num_linear_eq_constraints();
      jac_data_ptr = J.values();
    break;
    case Constraint::Type::NonlinearInequality: 
      const Dakota::RealMatrix& gradient_matrix = model.current_response().function_gradients();
      Dakota::Real* const data = gradient_matrix.values();
      int grad_offset = 1; // The first response is for the Objective function gradient
      num_cols = dakotaModel.num_nonlinear_eq_constraints();  
      jac_data_ptr = data + num_rows*grad_offset;
  
    break;
    case Constraint::Type::NonlinearEquality: 
      const Dakota::RealMatrix& gradient_matrix = model.current_response().function_gradients();
      Dakota::Real* const data = gradient_matrix.values();
      // Nonlinear Equality constraints appear after any Nonlinear Inequality constraints
      int grad_offset = 1 + dakotaModel.num_nonlinear_ineq_constraints(); 
      num_cols = dakotaModel.num_nonlinear_eq_constraints();  
      jac_data_ptr = data + num_rows*grad_offset;
    break;
    default:
      throw std::runtime_error("Invalid Constraint::Type");
  }
  jacobian = ROL::makePtr<Jacobian>(jac_data_ptr,num_rows,num_cols);
  return jacobian; 
} 

ModelInterface::update( const Dakota::RealVector& x,
                     UpdateType  type,
                     int         iter ) {
  
//  switch(type) { 
//    case UpdateType::Initial:
//      break;
//
//    case UpdateType::Accept:
//      break;
//
//    case UpdateType::Revert:
//      break;
//
//    case UpdateType::Trial:
//      break;
//
//    case UpdateType::Temp:
//    default:
//      break;
//  }
  evalSet = dakotaModel.current_response().active_set();
  dakotaModel.continuous_variables(x);
  dakotaModel.evaluate(evalSet);
} // ModelInterface::update


} // namespace rol_interface
