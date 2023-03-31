#pragma once
#ifndef DAKOTA_ROL_OBJECTIVE_HPP
#define DAKOTA_ROL_OBJECTIVE_HPP

namespace rol_interface {

class Objective : public ROL::Objective<Dakota::Real> {
public:

  Objective() = delete;
  Objective( const ROL::Ptr<ModelInterface>& model_interface );
    
  virtual ~Objective() = default;

  void update( const ROL::Vector<Dakota::Real>& x, 
                     ROL::UpdateType            type,
                     int                        iter = -1 ) override;

  virtual void update( const Dakota::RealVector& x, 
                       ROL::UpdateType           type,
                       int                       iter = -1 ); 

  Dakota::Real value( const ROL::Vector<Dakota::Real>& x,
                            Dakota::Real&              tol ) override;

  virtual Dakota::Real value( const Dakota::RealVector& x,
                                    Dakota::Real&       tol );
          
  void gradient(       ROL::Vector<Dakota::Real>& g,
                 const ROL::Vector<Dakota::Real>& x,
                       Dakota::Real&              tol ) override;

  virtual void gradient(       Dakota::RealVector& g,
                         const Dakota::RealVector& x,
                               Dakota::Real&       tol );

  void hessVec(        ROL::Vector<Dakota::Real>& hv,
                 const ROL::Vector<Dakota::Real>& v,
                 const ROL::Vector<Dakota::Real>& x,
                       Dakota::Real&              tol ) override;

  virtual void hessVec(        Dakota::RealVector& hv,
                         const Dakota::RealVector& v,
                         const Dakota::RealVector& x,
                               Dakota::Real&       tol );

  void invHessVec(        ROL::Vector<Dakota::Real>& ihv,
                    const ROL::Vector<Dakota::Real>& v,
                    const ROL::Vector<Dakota::Real>& x,
                          Dakota::Real&              tol ) override;


  virtual void invHessVec(        Dakota::RealVector& ihv,
                            const Dakota::RealVector& v,
                            const Dakota::RealVector& x,
                                  Dakota::Real&       tol );

  inline auto make_opt_vector() noexcept {
    return make_vector(modelInterface->dakotaModel.cv());
  }

private:

  ROL::Ptr<ModelInterface> modelInterface;
  ROL::Ptr<Hessian> hessOp;

}; // class Objective


} // namespace rol_interface


#endif // DAKOTA_ROL_OBJECTIVE_HPP
