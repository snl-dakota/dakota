#ifndef PREDATOR_PREY_HPP
#define PREDATOR_PREY_HPP

#include "dakota_data_types.hpp"

namespace Dakota {


class PredatorPreyModel
{

private:

  int N_;
  int fwdNDOF_;
  int adjNDOF_;
  Real dt_;
  Real finalTime_;
  RealVector time_;
  RealVector phiT_;
  RealVector initialConditions_;


  // Variables that are reused often. Initialised once to avoid unecessary work
  RealVector dtVector_;
  RealMatrix r1_, r2_, rr_, fhat_, ahat_;
  

public:
  
  PredatorPreyModel();

  ~PredatorPreyModel();

  void set_initial_conditions( RealVector& u0 );

  void set_time(  Real final_time, Real dt );

  void get_num_dof( IntVector& result_0 );

  void evaluate( const RealVector &coordinate, RealVector &result_0 );

  Real compute_error_estimate( const RealVector &coordinate, 
			       const RealVector &forward_solutions, 
			       const RealVector &adjoint_solutions );

  void compute_error_estimates( const RealMatrix &coordinates, 
				const RealMatrix &forward_solutions,
				const RealMatrix &adjoint_solutions,
				RealMatrix &values_out );
};
} // namespace Dakota

#endif // PREDATOR_PREY_HPP
