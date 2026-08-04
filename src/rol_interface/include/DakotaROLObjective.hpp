#pragma once
#ifndef DAKOTA_ROL_OBJECTIVE_HPP
#define DAKOTA_ROL_OBJECTIVE_HPP

#include "DakotaModel.hpp"

#include "ROL_Objective.hpp"

namespace rol_interface {

class Objective : public ROL::Objective<Dakota::Real> {
public:
  using pointer       =       Dakota::Real*;
  using const_pointer = const Dakota::Real*;

  Objective( BoolDispatch   hasGradient,
             BoolDispatch   hasHessian,
             Dakota::Model& model ); 

  void update( const ROL::Vector<Dakota::Real>& x, 
                     ROL::UpdateType            type,
                     int                        iter = -1 ) override final;

  Dakota::Real value( const ROL::Vector<Dakota::Real>& x,
                            Dakota::Real&              tol ) override final;

  void gradient(       ROL::Vector<Dakota::Real>& g,
                 const ROL::Vector<Dakota::Real>& x,
                       Dakota::Real&              tol ) override final;

  void hessVec(        ROL::Vector<Dakota::Real>& hv,
                 const ROL::Vector<Dakota::Real>& v,
                 const ROL::Vector<Dakota::Real>& x,
                       Dakota::Real&              tol ) override final;

  static ROL::Ptr<ROL::Objective<Dakota::Real>> createFromModel( Dakota::Model& model ); 

private:
  void evaluateIfNeeded(const ROL::Vector<Dakota::Real>& x, short request_values);
  void cacheGradientIfNeeded();

  std::size_t numOpt;
  Dakota::RealVector gradientCopy;  // Stores a COPY of gradient data (not a view)
  BoolDispatch hasGradient, hasHessian;
  Dakota::Model& dakotaModel;
}; // class Objective


} // namespace rol_interface


#endif // DAKOTA_ROL_OBJECTIVE_HPP
