#include "DakotaROLInterface.hpp"
#include "model_utils.hpp"

namespace rol_interface {

void Constraint::evaluateIfNeeded(const ROL::Vector<Dakota::Real>& x,
                                  short request_values)
{
  isLinear.receive([&](auto is_linear) {
    if constexpr (!is_linear) {
      const auto& x_dakota =
        as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(x));

      if (auto* optimizer = Dakota::ROLOptimizer::active_instance())
        optimizer->evaluate_model_if_needed(dakotaModel, x_dakota, request_values);
      else {
        Dakota::ModelUtils::continuous_variables(dakotaModel, x_dakota);

        Dakota::ActiveSet eval_set(dakotaModel.current_response().active_set());
        eval_set.request_values(request_values);
        dakotaModel.evaluate(eval_set);
      }

      copy_response_data(request_values);
    }
  });
}

void Constraint::copy_response_data(short request_values) {
  const_pointer val_ptr{nullptr}, jac_ptr{nullptr}, target_ptr{nullptr};
  std::ptrdiff_t val_offset{0}, jac_offset{0};

  isLinear.receive([&](auto is_linear) {
    if constexpr( is_linear ) {
      isEquality.receive([&](auto is_equality) {
        if constexpr( is_equality ) {
          numCon = Dakota::ModelUtils::num_linear_eq_constraints(dakotaModel);
          jac_ptr = Dakota::ModelUtils::linear_eq_constraint_coeffs(dakotaModel).values();
          target_ptr = Dakota::ModelUtils::linear_eq_constraint_targets(dakotaModel).values();
        } else {
          numCon = Dakota::ModelUtils::num_linear_ineq_constraints(dakotaModel);
          jac_ptr = Dakota::ModelUtils::linear_ineq_constraint_coeffs(dakotaModel).values();
        }
      });
    } else { // Nonlinear Constraint
      const auto& resp = dakotaModel.current_response();
      val_ptr = resp.function_values().values();
      jac_ptr = resp.function_gradients().values();

      isEquality.receive([&](auto is_equality) {
        if constexpr( is_equality ) {
          numCon = Dakota::ModelUtils::num_nonlinear_eq_constraints(dakotaModel);
          target_ptr = Dakota::ModelUtils::nonlinear_eq_constraint_targets(dakotaModel).values();
          std::size_t numIneq = Dakota::ModelUtils::num_nonlinear_ineq_constraints(dakotaModel);
          val_offset = 1 + numIneq;
        } else {
          numCon = Dakota::ModelUtils::num_nonlinear_ineq_constraints(dakotaModel);
          val_offset = 1;
        }
      });

      jac_offset = val_offset * static_cast<int>(numOpt);

      hessianView.clear();
      hasHessian.receive([&,this](auto has_hessian) {
        if constexpr( has_hessian ) {
          if (!(request_values & 4))
            return;

          std::size_t offset = static_cast<std::size_t>(val_offset);
          for(std::size_t i=0; i<numCon; ++i) {
            hessianView.push_back(resp.function_hessian(i+offset));
          }
        }
      });
    }
  });

  isLinear.receive([&,this](auto is_linear) {
    if constexpr( !is_linear ) {
      valueCopy.sizeUninitialized(static_cast<int>(numCon));
      for (std::size_t i = 0; i < numCon; ++i) {
        valueCopy[static_cast<int>(i)] = val_ptr[val_offset + i];
      }
    }
  });

  isEquality.receive([&,this](auto is_equality){
    if constexpr( is_equality ) {
      targetView = Dakota::RealVector(Teuchos::View,
                                      const_cast<pointer>(target_ptr),
                                      static_cast<int>(numCon));
    }
  });

  hasJacobian.receive([&,this](auto has_jacobian){
    if constexpr( has_jacobian ) {
      if (!(request_values & 2))
        return;

      isLinear.receive([&,this](auto is_linear) {
        if constexpr( is_linear ) {
          jacobianCopy.shapeUninitialized(static_cast<int>(numCon), static_cast<int>(numOpt));
          for (std::size_t row = 0; row < numCon; ++row) {
            for (std::size_t col = 0; col < numOpt; ++col) {
              jacobianCopy(static_cast<int>(row), static_cast<int>(col)) =
                jac_ptr[row + col * numCon];
            }
          }
        } else {
          const auto& gradient_matrix = dakotaModel.current_response().function_gradients();
          jacobianCopy.shapeUninitialized(static_cast<int>(numCon), static_cast<int>(numOpt));
          for (std::size_t conIdx = 0; conIdx < numCon; ++conIdx) {
            for (std::size_t varIdx = 0; varIdx < numOpt; ++varIdx) {
              jacobianCopy(static_cast<int>(conIdx), static_cast<int>(varIdx)) =
                gradient_matrix(static_cast<int>(varIdx), static_cast<int>(val_offset + conIdx));
            }
          }
        }
      });
    }
  });
}


Constraint::Constraint( BoolDispatch   IsLinear,
                        BoolDispatch   IsEquality,
                        BoolDispatch   HasJacobian,
                        BoolDispatch   HasHessian,
                        Dakota::Model& model )
: dakotaModel{model},
  numOpt{Dakota::ModelUtils::cv(model)},
  numCon{0},
  valueCopy(1, true),
  jacobianCopy(1, 1, true),
  isLinear{IsLinear},
  isEquality{IsEquality},
  hasJacobian{HasJacobian},
  hasHessian{HasHessian}  {
  isLinear.receive([&,this](auto is_linear) {
    if constexpr( is_linear ) {
      isEquality.receive([&,this](auto is_equality) {
        if constexpr( is_equality ) {
          numCon = Dakota::ModelUtils::num_linear_eq_constraints(dakotaModel);
        } else {
          numCon = Dakota::ModelUtils::num_linear_ineq_constraints(dakotaModel);
        }
      });
    } else {
      isEquality.receive([&,this](auto is_equality) {
        if constexpr( is_equality ) {
          numCon = Dakota::ModelUtils::num_nonlinear_eq_constraints(dakotaModel);
        } else {
          numCon = Dakota::ModelUtils::num_nonlinear_ineq_constraints(dakotaModel);
        }
      });
    }
  });
  valueCopy.sizeUninitialized(static_cast<int>(numCon));
  jacobianCopy.shapeUninitialized(static_cast<int>(numCon), static_cast<int>(numOpt));
}


void Constraint::update( const ROL::Vector<Dakota::Real>& x,
                               ROL::UpdateType            type,
                               int                        iter ) {
  std::ignore = iter;
  if (type == ROL::UpdateType::Temp || type == ROL::UpdateType::Trial || type == ROL::UpdateType::Initial) {
    short request_values = 7;
    if (dakotaModel.gradient_type() == "numerical" && dakotaModel.method_source() == "vendor")
      request_values = 1;
    else if (dakotaModel.hessian_type() == "none")
      request_values = 3;

    evaluateIfNeeded(x, request_values);
  }
}

void Constraint::value(       ROL::Vector<Dakota::Real>& c,
                        const ROL::Vector<Dakota::Real>& x,
                              Dakota::Real&              tol ) {
  evaluateIfNeeded(x, 1);
  isLinear.receive([&,this]( auto is_true ) {
    if constexpr( is_true ) {
      applyJacobian(c,x,x,tol);
    } else {
      auto& c_vector = as_dakota_vector(c);
      c_vector = valueCopy;
    }
  });

  isEquality.receive([&,this]( auto is_true ) {
    if constexpr( is_true ) {
      auto& c_vector = as_dakota_vector(c);
      c_vector -= targetView;
    }
  });
}

void Constraint::applyJacobian(       ROL::Vector<Dakota::Real>& jv,
                                const ROL::Vector<Dakota::Real>& v,
                                const ROL::Vector<Dakota::Real>& x,
                                      Dakota::Real&              tol ) {

  hasJacobian.receive([&,this]( auto is_true ) {
    if constexpr( is_true ) {
      evaluateIfNeeded(x, 3);
      const auto& v_vector  = as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(v));
      auto& jv_vector = as_dakota_vector(jv);

      int err_code = jv_vector.multiply(Teuchos::NO_TRANS,
                                        Teuchos::NO_TRANS,
                                        1,
                                        jacobianCopy,
                                        v_vector,
                                        0);
      TEUCHOS_ASSERT_EQUALITY(err_code,0);
    }
    else {
      ROL::Constraint<Dakota::Real>::applyJacobian(jv,v,x,tol);
    }
  });
}

void Constraint::applyAdjointJacobian(       ROL::Vector<Dakota::Real>& ajv,
                                       const ROL::Vector<Dakota::Real>& v,
                                       const ROL::Vector<Dakota::Real>& x,
                                             Dakota::Real&              tol ) {

  hasJacobian.receive([&,this]( auto is_true ) {
    if constexpr( is_true ) {
      evaluateIfNeeded(x, 3);
      const auto& v_vector  = as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(v));
      auto& ajv_vector = as_dakota_vector(ajv);
      int err_code = ajv_vector.multiply(Teuchos::TRANS,
                                         Teuchos::NO_TRANS,
                                         1,
                                         jacobianCopy,
                                         v_vector,
                                         0);
      TEUCHOS_ASSERT_EQUALITY(err_code,0);
    }
    else {
      ROL::Constraint<Dakota::Real>::applyAdjointJacobian(ajv,v,x,tol);
    }
  });
}


void Constraint::applyAdjointHessian(       ROL::Vector<Dakota::Real>& ahuv,
                                      const ROL::Vector<Dakota::Real>& u,
                                      const ROL::Vector<Dakota::Real>& v,
                                      const ROL::Vector<Dakota::Real>& x,
                                            Dakota::Real&              tol ) {
  isLinear.receive([&,this]( auto is_linear ) {
    if constexpr(is_linear) {
      ahuv.zero();
    } else {
      hasHessian.receive([&,this]( auto has_hessian ) {
        if constexpr ( has_hessian ) {
          evaluateIfNeeded(x, 7);
          auto& ahuv_vector    = as_dakota_vector(ahuv);
          const auto& v_vector = as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(v));
          const auto& u_vector = as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(u));
          for(std::size_t i = 0; i < numCon; ++i) {
            int err_code = ahuv_vector.multiply(Teuchos::LEFT_SIDE,
                                                u_vector[i],
                                                hessianView.at(i),
                                                v_vector,
                                                static_cast<Dakota::Real>(i>0));
            TEUCHOS_ASSERT_EQUALITY(err_code,0);
          }
        } else {
          ROL::Constraint<Dakota::Real>::applyAdjointHessian(ahuv,u,v,x,tol);
        }
      });
    }
  });
}

ConstraintSet Constraint::createSetFromModel( Dakota::Model& model ) {

  auto grad_type  = model.gradient_type();
  auto hess_type  = model.hessian_type();
  auto method_src = model.method_source();

  BoolDispatch have_jacobian{( grad_type == "analytic" || grad_type == "mixed" ||
                            ( grad_type == "numerical" && method_src == "dakota" ))};

  BoolDispatch have_hessian{hess_type != "none"};

  ConstraintSet constraints;

  if( Dakota::ModelUtils::num_linear_eq_constraints(model) ) {
    constexpr BoolDispatch is_linear{true}, is_equality{true};
    constraints.linearEquality = ROL::makePtr<Constraint>(is_linear,
                                                          is_equality,
                                                          have_jacobian,
                                                          have_hessian,
                                                          model);
  }
  if( Dakota::ModelUtils::num_linear_ineq_constraints(model) ) {
    constexpr BoolDispatch is_linear{true}, is_equality{false};
    constraints.linearInequality = ROL::makePtr<Constraint>(is_linear,
                                                            is_equality,
                                                            have_jacobian,
                                                            have_hessian,
                                                            model);
  }
  if( Dakota::ModelUtils::num_nonlinear_eq_constraints(model) ) {
    constexpr BoolDispatch is_linear{false}, is_equality{true};
    constraints.nonlinearEquality = ROL::makePtr<Constraint>(is_linear,
                                                             is_equality,
                                                             have_jacobian,
                                                             have_hessian,
                                                             model);
  }
  if( Dakota::ModelUtils::num_nonlinear_ineq_constraints(model) ) {
    constexpr BoolDispatch is_linear{false}, is_equality{false};
    constraints.nonlinearInequality = ROL::makePtr<Constraint>(is_linear,
                                                               is_equality,
                                                               have_jacobian,
                                                               have_hessian,
                                                               model);
  }
  return constraints;
}

} // namespace rol_interface
