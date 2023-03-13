#pragma once
#ifndef DAKOTA_ROL_MODEL_INTERFACE_HPP
#define DAKOTA_ROL_MODEL_INTERFACE_HPP


namespace rol_interface {

class ModelInterface { 
public:

  ModelInterface() = delete;
  ModelInterface( Dakota::Model& model ); 
  ~ModelInterface() = default;

  static constexpr short asVal         = 1;
  static constexpr short asValGrad     = 3;
  static constexpr short asValGradHess = 7;

private:

  void update( const RealVector&     x,
                     ROL::UpdateType type,
                     int             iter = -1 );     

 // Ptr<RealVector>   xCurrent, xModelInterface, xTemp;
  Dakota::ActiveSet evalSet;
  Dakota::Model&    dakotaModel;

  bool useDefaultDeriv1 = true;
  bool useDefaultDeriv2 = true;
  bool useCenteredDifferences = false;

  friend class Objective;
  friend class Constraint;
  friend class Hessian;
  friend class Jacobian;

}; // class ModelInterface


} // namespace rol_interface

#endif // DAKOTA_ROL_MODEL_INTERFACE_HPP
