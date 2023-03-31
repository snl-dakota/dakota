#pragma once
#ifndef DAKOTA_ROL_BOUNDS_H
#define DAKOTA_ROL_BOUNDS_H

namespace rol_interface {

class Bounds : public ROL::Bounds<Dakota::Real> {
public:

  using ROL::Bounds<Dakota::Real>::Bounds;
  virtual ~Bounds() = default;

  /// Utility function to dynamically allocate a bound constraint on the optimization vector
  static ROL::Ptr<Bounds> make_continuous_variable( const Dakota::Model& model ) {
    auto n = model.cv();
    auto l = make_vector(n);
    auto u = make_vector(n);
//    auto& l_ref = get_vector(l);
//    auto& u_ref = get_vector(u);
    l->set(model.continuous_lower_bounds());
    u->set(model.continuous_upper_bounds());
    return ROL::makePtr<Bounds>(l,u);
  }

  /// Utility function to dynamically allocate a bound constraint on the linear inequality vector
  static ROL::Ptr<Bounds> make_linear_ineq_constraints( const Dakota::Model& model ) {
    auto n = model.num_linear_ineq_constraints();
    auto l = make_vector(n);
    auto u = make_vector(n);
//    auto& l_ref = get_vector(l);
//    auto& u_ref = get_vector(u);
    // TODO: Cam these values change?
    l->set(model.linear_ineq_constraint_lower_bounds());
    u->set(model.linear_ineq_constraint_upper_bounds());
    return ROL::makePtr<Bounds>(l,u);
  }

  /// Utility function to dynamically allocate a bound constraint on the nonlinear inequality vector
  static ROL::Ptr<Bounds> make_nonlinear_ineq_constraints( const Dakota::Model& model ) {
    auto n = model.num_nonlinear_ineq_constraints();
    auto l = make_vector(n);
    auto u = make_vector(n);
//    auto& l_ref = get_vector(l);
//    auto& u_ref = get_vector(u);
    l->set(model.nonlinear_ineq_constraint_lower_bounds());
    u->set(model.nonlinear_ineq_constraint_upper_bounds());
    return ROL::makePtr<Bounds>(l,u);
  }

};

} // namespace rol_interface

#endif  // DAKOTA_ROL_BOUNDS_H
