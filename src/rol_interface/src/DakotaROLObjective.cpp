#include "DakotaROLInterface.hpp"

namespace rol_interface {

Objective::Objective( BoolDispatch   HasGradient,
                      BoolDispatch   HasHessian,
                      Dakota::Model& model )  
  : numOpt{Dakota::ModelUtils::cv(model)}, 
    hasGradient{HasGradient},
    hasHessian{HasHessian},
    dakotaModel{model} { 
      
  hasGradient.receive([&,this](auto has_gradient) {
    if constexpr( has_gradient ) {
      const auto& resp = dakotaModel.current_response();
      const_pointer grad_ptr = resp.function_gradients().values();
      // Objective gradient is at offset 0 (first function)
      gradientView = Dakota::RealVector(Teuchos::View,
                                       const_cast<pointer>(grad_ptr), 
                                       static_cast<int>(numOpt) /* length */ );
    }
  });
}

void Objective::update( const ROL::Vector<Dakota::Real>& x,
                              ROL::UpdateType            type,
                              int                        iter ) {
  std::ignore = iter;
  if (type == ROL::UpdateType::Temp || type == ROL::UpdateType::Initial) {
    // Update Dakota model with current optimization variables
    const auto& x_dakota = as_dakota_vector(const_cast<ROL::Vector<Dakota::Real>&>(x));
    Dakota::ModelUtils::continuous_variables(dakotaModel, x_dakota);
    
    // Set up active set for evaluation based on what derivatives are needed
    Dakota::ActiveSet eval_set(dakotaModel.current_response().active_set());
    if (dakotaModel.gradient_type() == "numerical" && dakotaModel.method_source() == "vendor") {
      // ROL will handle gradients numerically
      eval_set.request_values(1);  // Function values only
    } else if (dakotaModel.hessian_type() == "none") {
      // Need function values and gradients
      eval_set.request_values(3);  // Function values + gradients
    } else {
      // Need function values, gradients, and Hessians
      eval_set.request_values(7);  // Function values + gradients + Hessians
    }
    
    // Evaluate the Dakota model at the current point
    dakotaModel.evaluate(eval_set);
    
    // Update gradient view if we have gradients
    hasGradient.receive([&,this](auto has_gradient) {
      if constexpr( has_gradient ) {
        const auto& resp = dakotaModel.current_response();
        const_pointer grad_ptr = resp.function_gradients().values();
        // Objective gradient is at offset 0 (first function)
        gradientView = Dakota::RealVector(Teuchos::View,
                                         const_cast<pointer>(grad_ptr), 
                                         static_cast<int>(numOpt) /* length */ );
      }
    });
  }
} // Objective::update


Dakota::Real Objective::value( const ROL::Vector<Dakota::Real>& x, 
                                     Dakota::Real&              tol ) {
  const auto& resp = dakotaModel.current_response();
  return resp.function_value(0);      
}



void Objective::gradient(       ROL::Vector<Dakota::Real>& g,
                          const ROL::Vector<Dakota::Real>& x, 
                                Dakota::Real&              tol ) {
  hasGradient.receive([&,this](auto has_gradient) {
    if constexpr( has_gradient ) {
      auto& g_vector = as_dakota_vector(g);
      g_vector = gradientView;
    } else {
      // Use ROL's finite difference approximation
      ROL::Objective<Dakota::Real>::gradient(g, x, tol);
    }
  });
}


void Objective::hessVec(       ROL::Vector<Dakota::Real>& hv, 
                         const ROL::Vector<Dakota::Real>& v,
                         const ROL::Vector<Dakota::Real>& x,
                               Dakota::Real&              tol ) {
  hasHessian.receive([&,this](auto has_hessian) {
    if constexpr( has_hessian ) {
      const auto& resp = dakotaModel.current_response();
      auto& hv_vector = as_dakota_vector(hv);
      const auto& v_vector = as_dakota_vector(v);
      
      // Objective Hessian is at index 0
      Dakota::RealSymMatrix H(Teuchos::View,
                              resp.function_hessian(0),
                              static_cast<int>(numOpt) /* stride */,
                              static_cast<int>(numOpt) /* size */ );
      
      // hv = H * v
      int err_code = hv_vector.multiply(Teuchos::LEFT_SIDE,
                                        Dakota::Real{1.0},
                                        H,
                                        v_vector,
                                        Dakota::Real{0.0});
      TEUCHOS_ASSERT_EQUALITY(err_code, 0);
    } else {
      // Use ROL's finite difference approximation
      ROL::Objective<Dakota::Real>::hessVec(hv, v, x, tol);
    }
  });
}


ROL::Ptr<ROL::Objective<Dakota::Real>> Objective::createFromModel( Dakota::Model& model ) {
  auto grad_type  = model.gradient_type();
  auto hess_type  = model.hessian_type();
  auto method_src = model.method_source();

  BoolDispatch have_gradient{( grad_type == "analytic" || grad_type == "mixed" ||
                            ( grad_type == "numerical" && method_src == "dakota" ))};

  BoolDispatch have_hessian{hess_type != "none"};

  return ROL::makePtr<Objective>(have_gradient, have_hessian, model);
}

} // namespace rol_interface

