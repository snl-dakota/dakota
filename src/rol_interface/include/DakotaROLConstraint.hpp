#ifndef DAKOTA_ROL_CONSTRAINT
#define DAKOTA_ROL_CONSTRAINT

namespace rol_interface {

struct ConstraintSet {
  using constraint_ptr = ROL::Ptr<ROL::Constraint<Dakota::Real>>; 
  constraint_ptr linearEquality{ROL::nullPtr};
  constraint_ptr linearInequality{ROL::nullPtr};
  constraint_ptr nonlinearEquality{ROL::nullPtr};
  constraint_ptr nonlinearInequality{ROL::nullPtr};
};


class Constraint : public ROL::Constraint<Dakota::Real> {
public:
  using pointer       =       Dakota::Real*;
  using const_pointer = const Dakota::Real*;
 
  void update( const ROL::Vector<Dakota::Real>& x,
                     ROL::UpdateType            type,
                     int                        iter = -1 ) override final;
                     
  void value(       ROL::Vector<Dakota::Real>& c, 
              const ROL::Vector<Dakota::Real>& x, 
                    Dakota::Real&              tol ) override final;

  void applyJacobian(       ROL::Vector<Dakota::Real>& jv, 
                      const ROL::Vector<Dakota::Real>& v, 
                      const ROL::Vector<Dakota::Real>& x, 
                            Dakota::Real&              tol ) override final;

  void applyAdjointJacobian(       ROL::Vector<Dakota::Real>& jv, 
                             const ROL::Vector<Dakota::Real>& v, 
                             const ROL::Vector<Dakota::Real>& x, 
                                   Dakota::Real&              tol ) override final;
    
  void applyAdjointHessian(       ROL::Vector<Dakota::Real>& ahuv, 
                            const ROL::Vector<Dakota::Real>& u,
                            const ROL::Vector<Dakota::Real>& v,
                            const ROL::Vector<Dakota::Real>& x,
                                  Dakota::Real&              tol ) override final;

  static ConstraintSet createSetFromModel( Dakota::Model& model );

//private:

  Constraint( BoolDispatch   isLinear, 
              BoolDispatch   isEquality,
              BoolDispatch   hasJacobian,
              BoolDispatch   hasHessian,
              Dakota::Model& model );

  Dakota::Model& dakotaModel;
  std::size_t numOpt, numCon;
  Dakota::RealVector valueView, targetView;
  Dakota::RealMatrix jacobianView;
  Dakota::RealSymMatrixArray hessianView;
  BoolDispatch isLinear, isEquality, hasJacobian, hasHessian;
};        

} // namespace rol_interface

#endif // DAKOTA_ROL_CONSTRAINT

