#pragma once
#ifndef DAKOTA_ROL_CACHE_HPP
#define DAKOTA_ROL_CACHE_HPP

namespace Dakota {
namespace rol_interface {

class Cache { 
public:

  Cache() = delete;
  Cache( Dakota::Model& model ); 
  ~Cache() = default;

  static constexpr short asVal         = 1;
  static constexpr short asValGrad     = 3;
  static constexpr short asValGradHess = 7;

private:

  void update( const RealVector& x,
                     UpdateType   type,
                     int          iter = -1 );     

 // Ptr<RealVector>   xCurrent, xCache, xTemp;
  Dakota::ActiveSet evalSet;
  Dakota::Model&    dakotaModel;

  bool useDefaultDeriv1 = true;
  bool useDefaultDeriv2 = true;
  bool useCenteredDifferences = false;

  friend class Objective;
  friend class Constraint;
  friend class LinearOperator;

}; // class Cache


} // namespace rol_interface
} // namespace Dakota 

#endif // DAKOTA_ROL_CACHE_HPP
