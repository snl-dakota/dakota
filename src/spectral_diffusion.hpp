#ifndef SPECTRAL_DIFFUSION_HPP
#define SPECTRAL_DIFFUSION_HPP

#include "dakota_data_types.hpp"

namespace Dakota {

void kronecker_product_2d( const RealMatrix &matrix1, const RealMatrix &matrix2,
			   RealMatrix &result );

class SpectralDiffusionModel
{

private:

  int order_;
  int numQOI_;
  String kernel_;
  RealVector singularValues_;
  RealMatrix leftSingularVectors_;
  RealVector boundaryConditions_;
  RealVector physicalDomainLimits_;

  RealMatrix derivativeMatrix_;
  RealMatrix collocationPoints_;

public:
  
  SpectralDiffusionModel();

  ~SpectralDiffusionModel();

  void initialize( int order, String kernel, const RealVector &bndry_conds,
		   const RealVector &domain_limits );

  void form_collocation_matrix( const RealVector &diagonal,
				RealMatrix &collocation_matrix ) const;

  void apply_boundary_conditions( RealMatrix &collocation_matrix, 
				  RealVector &forcing ) const;

  void solve( const RealVector &diffusivity, const RealVector &forcing_in, 
	      RealVector &result ) const;

  void run( const RealVector &sample, RealVector &result ) const;

  void evaluate( const RealVector &sample, RealVector &result );

  void interpolate( const RealVector &mesh_values,
		    const RealVector &interp_samples,
		    RealVector &result ) const;

  double integrate( const RealVector &mesh_values ) const;

  virtual void qoi_functional( const RealVector &solution, 
			       RealVector &result ) const;

  virtual void diffusivity_function( const RealVector &sample, 
				     const RealMatrix &mesh_points,
				     RealVector &result ) const;
  
  virtual void forcing_function( const RealVector &sample, 
				 const RealMatrix &mesh_points,
				 RealVector &result) const;  

  void set_num_qoi( int num_qoi );

  void get_collocation_points(RealMatrix &result ) const;

  void get_derivative_matrix( RealMatrix &result ) const;
};


} //namespace dakota


#endif // SPECTRAL_DIFFUSION_HPP
