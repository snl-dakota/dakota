#include "DakotaROLInterface.hpp"


namespace rol_interface {

Objective::Objective( BoolDispatch   HasGradient,
                      BoolDispatch   HasHessian,
                      Dakota::Model& model )
  : numOpt{Dakota::ModelUtils::cv(model)},
    gradientCopy(static_cast<int>(numOpt), true),
    hasGradient{HasGradient},
    hasHessian{HasHessian},
    dakotaModel{model} {
}

void Objective::evaluateIfNeeded(const ROL::Vector<Dakota::Real>& x,
                                 short request_values)
{
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
}

void Objective::cacheGradientIfNeeded()
{
  hasGradient.receive([&,this](auto has_gradient) {
    if constexpr( has_gradient ) {
      const auto& resp = dakotaModel.current_response();
      const_pointer grad_ptr = resp.function_gradients().values();
      for (std::size_t i = 0; i < numOpt; ++i)
        gradientCopy[static_cast<int>(i)] = grad_ptr[i];
    }
  });
}

void Objective::update( const ROL::Vector<Dakota::Real>& x,
                              ROL::UpdateType            type,
                              int                        iter ) {
  std::ignore = iter;
  if (type == ROL::UpdateType::Temp ||
      type == ROL::UpdateType::Trial ||
      type == ROL::UpdateType::Initial) {
    short request_values = 7;
    if (dakotaModel.gradient_type() == "numerical" &&
        dakotaModel.method_source() == "vendor") {
      request_values = 1;
    }
    else if (dakotaModel.hessian_type() == "none") {
      request_values = 3;
    }

    evaluateIfNeeded(x, request_values);
    cacheGradientIfNeeded();
  }
} // Objective::update


Dakota::Real Objective::value( const ROL::Vector<Dakota::Real>& x,
                                     Dakota::Real&              tol ) {
  evaluateIfNeeded(x, 1);
  const auto& resp = dakotaModel.current_response();
  return resp.function_value(0);
}



void Objective::gradient(       ROL::Vector<Dakota::Real>& g,
                          const ROL::Vector<Dakota::Real>& x,
                                Dakota::Real&              tol ) {
  hasGradient.receive([&,this](auto has_gradient) {
    if constexpr( has_gradient ) {
      evaluateIfNeeded(x, 3);
      cacheGradientIfNeeded();
      auto& g_vector = as_dakota_vector(g);
      g_vector = gradientCopy;
    } else {
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
      evaluateIfNeeded(x, 7);
      cacheGradientIfNeeded();

      const auto& resp = dakotaModel.current_response();
      auto& hv_vector = as_dakota_vector(hv);
      const auto& v_vector = as_dakota_vector(v);

      const auto& H = resp.function_hessian(0);

      int err_code = hv_vector.multiply(Teuchos::LEFT_SIDE,
                                        Dakota::Real{1.0},
                                        H,
                                        v_vector,
                                        Dakota::Real{0.0});
      TEUCHOS_ASSERT_EQUALITY(err_code, 0);
    } else {
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
