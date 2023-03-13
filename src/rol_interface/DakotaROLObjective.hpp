#pragma once
#ifndef DAKOTA_ROL_OBJECTIVE_HPP
#define DAKOTA_ROL_OBJECTIVE_HPP

namespace rol_interface {

class Objective : public ROL::Objective<Real> {
public:

  Objective() = delete;
  Objective( const ROL::Ptr<ModelInterface>& model_interface );
    
  virtual ~Objective() = default;

  void update( const RealVector&     x, 
                     ROL::UpdateType type,
                     int             iter = -1 ) override;

  Real value( const RealVector& x,
                    Real&       tol ) override;

  void gradient(       RealVector& g,
                 const RealVector& x,
                       Real&       tol ) override;

  void hessVec(        RealVector& hv,
                 const RealVector& v,
                 const RealVector& x,
                       Real&       tol ) override;

  void invHessVec(        RealVector& ihv,
                    const RealVector& v,
                    const RealVector& x,
                          Real&       tol ) override;

  inline ROL::Ptr<ROL::Vector<Real>> make_opt_vector() noexcept {
    return ROL::makePtr<ROL::TeuchosVector<int,Real>>(modelInterface->dakotaModel.cv())
  }

private:

  ROL::Ptr<ModelInterface> modelInterface;
  ROL::Ptr<Hessian> hessOp;

}; // class Objective


} // namespace rol_interface


#endif // DAKOTA_ROL_OBJECTIVE_HPP
