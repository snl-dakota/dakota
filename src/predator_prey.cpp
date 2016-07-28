#include "predator_prey.hpp"
#include "MathTools.hpp"
#include "LinearAlgebra.hpp"

namespace Dakota {

PredatorPreyModel::PredatorPreyModel()
{

  set_time( 10., 1. / 100. );

  phiT_.size( 3 );
  phiT_[2] = 1.;
  initialConditions_.sizeUninitialized( 3 );
  initialConditions_[0] = 0.7; 
  initialConditions_[1] = 0.5; 
  initialConditions_[2] = 0.2;
};

PredatorPreyModel::~PredatorPreyModel(){};

void PredatorPreyModel::set_initial_conditions( RealVector& u0 )
{
  if ( u0.length() != 3 )
    throw( std::runtime_error( "Must specify 3 initial conditions" ) );

  initialConditions_.sizeUninitialized( u0.length() );
  initialConditions_.assign( u0 );
}

void PredatorPreyModel::set_time( Real final_time, Real dt )
{
  dt_ = dt;
  finalTime_ = final_time;
  N_ = ( finalTime_ / dt_ ) + 1;
  Pecos::linspace( time_, 0., finalTime_, N_ );

  dtVector_.sizeUninitialized( N_-1 );
  for ( int n = 0 ; n < N_-1; n++ )
    dtVector_[n] = time_[n+1] - time_[n];

  r1_.shapeUninitialized( 3, N_-1 );
  r2_.shapeUninitialized( 3, N_-1 ); 
  rr_.shapeUninitialized( 3, N_-1 );
  fhat_.shapeUninitialized( 3, N_-1 ); 
  ahat_.shapeUninitialized( 3, N_-1 );

  fwdNDOF_ = 3 * N_;
  adjNDOF_ = 3 * N_; 
}

void PredatorPreyModel::get_num_dof( IntVector& ndof )
{
  ndof.sizeUninitialized( 2 );
  ndof[0] = fwdNDOF_; 
  ndof[1] = adjNDOF_;
}

Real PredatorPreyModel::compute_error_estimate( const RealVector &x, 
						const RealVector &fwd_sol, 
						const RealVector &adj_sol )
{
  /*
  int num_quadrature_points = 5;
  Real quadrature_points[] = {0.04691007703067,0.2307653449471,
			      0.50000000000000,0.76923465505284,
			      0.95308992296933};
  Real quadrature_weights[] = {0.11846344252810,0.23931433524968,
			       0.28444444444445,0.23931433524968,
			       0.11846344252810};
  */
  int num_quadrature_points = 2;
  Real quadrature_points[] = {0.21132487,0.78867513};
  Real quadrature_weights[] = {0.5,0.5};

  Real r[3];
  r[0] = x[0]; r[1] = x[1];  r[2] = x[2];
  
  RealMatrix alpha( 3, 3, false );
  alpha(0,0) = 1.;   alpha(0,1) = x[3]; alpha(0,2) = x[4]; 
  alpha(1,0) = x[5]; alpha(1,1) = 1.;   alpha(1,2) = x[6]; 
  alpha(2,0) = x[7]; alpha(2,1) = x[8]; alpha(2,2) = 1.; 

  RealMatrix f1(  Teuchos::View, fwd_sol.values(), 3, 3, N_-1 ),
    f2(  Teuchos::View, fwd_sol.values() + 3, 3, 3, N_-1 ),
    a1(  Teuchos::View, adj_sol.values(), 3, 3, N_-1 ),
    a2(  Teuchos::View, adj_sol.values() + 3, 3, 3, N_-1 );

  N_ = time_.length();
  if ( fwd_sol.length() != 3 * N_ )
    throw( std::runtime_error( "time_ and fwd_sol are inconsistent" ) );
  if ( adj_sol.length() != 3 * N_ )
    throw( std::runtime_error( "time_ and adj_sol are inconsistent" ) );

  Real *fhat_ptr = fhat_.values(), *ahat_ptr = ahat_.values(), 
    *r1_ptr = r1_.values(), *r2_ptr = r2_.values(), *f1_ptr = f1.values(),
    *f2_ptr = f2.values(), *a1_ptr = a1.values(), *a2_ptr = a2.values(),
    *rr_ptr = rr_.values();
  
  Real I = 0;
  for ( int i = 0; i < num_quadrature_points; i++ )
    {
      for ( int k = 0; k < N_-1; k ++ )
	{
	  for ( int j = 0; j < 3; j ++ )
	    {
	      int idx = 3 * k +j;
	      fhat_ptr[idx] = quadrature_points[i] * f1_ptr[idx] + ( 1. - quadrature_points[i] ) * f2_ptr[idx];
	      ahat_ptr[idx] = quadrature_points[i] * a1_ptr[idx] + ( 1. - quadrature_points[i] ) * a2_ptr[idx];
	      r1_ptr[idx] = fhat_ptr[idx] * r[j];
	      r2_ptr[idx] = 1.;
	    }
	}
      r2_.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, -1.0, alpha,fhat_,1.0);
      for ( int k = 0; k < N_-1; k++ )
	{
	  Real sum = 0.;
	  for ( int j = 0; j < 3; j ++ )
	    {
	      int idx = 3 * k + j;
	      rr_ptr[idx] = r1_ptr[idx] * r2_ptr[idx];
	      sum += dtVector_[k] * ( (f2_ptr[idx] - f1_ptr[idx]) /dtVector_[k] -
	  			      rr_ptr[idx] ) * ahat_ptr[idx];
	    }
	  I -= quadrature_weights[i] * sum;
	}
    }

  return I;  
}

void PredatorPreyModel::compute_error_estimates( 
const RealMatrix &coordinates, 
const RealMatrix &forward_solutions,
const RealMatrix &adjoint_solutions,
RealMatrix &error_estimates )
{
  int num_dims = coordinates.numRows(), num_pts = coordinates.numCols();
  error_estimates.shapeUninitialized( num_pts, 1 );
  for ( int k = 0; k < num_pts; k++ )
    {
      RealVector coordinate( Teuchos::View, const_cast<Real*>(coordinates[k]),
			     num_dims );
      RealVector forward_solution( fwdNDOF_, false );
      RealVector adjoint_solution( adjNDOF_, false );
      for ( int i = 0 ; i < fwdNDOF_; i++ )
	forward_solution[i] = forward_solutions(k,i);
      for ( int i = 0 ; i < adjNDOF_; i++ )
	adjoint_solution[i] = adjoint_solutions(k,i);
      error_estimates(k,0) = compute_error_estimate( coordinate, 
						     forward_solution, 
						     adjoint_solution );
    }
}

void PredatorPreyModel::evaluate( const RealVector &x, 
				  RealVector &result )
{
  dtVector_.sizeUninitialized( N_-1 );
  for ( int n = 0 ; n < N_-1; n++ )
    dtVector_[n] = time_[n+1] - time_[n];

  int num_dims = x.length();
  result.sizeUninitialized( 3 );


  RealMatrix sol( 3, N_, false );
  RealMatrix asol( 3, N_, false );

  RealVector r( 3, false );
  r[0] = x[0]; r[1] = x[1];  r[2] = x[2];
  
  RealMatrix alpha( 3, 3, false );
  alpha(0,0) = 1.;   alpha(0,1) = x[3]; alpha(0,2) = x[4]; 
  alpha(1,0) = x[5]; alpha(1,1) = 1.;   alpha(1,2) = x[6]; 
  alpha(2,0) = x[7]; alpha(2,1) = x[8]; alpha(2,2) = 1.; 

  sol(0,0) = initialConditions_[0]; sol(1,0) = initialConditions_[1];
  sol(2,0) = initialConditions_[2];

  int prog = 0;
  Real ct = 0.;
  RealVector ov( 3, false );
  ov = 1.;

  RealVector csol( 3, false ), psol( 3, false ), t1( 3, false ), t2( 3, false ),
    res( 3, false ), j1( 3, false ), ppsol( 3, false );

  //Real intwt = 3. / 2. / dt_;
  Real intwt = 1. / dt_;
  RealMatrix Jt( 3, 3 ), J1( 3, 3 ), J2( 3, 3, false ), J( 3, 3, false );
  Jt(0,0) = intwt; Jt(1,1) = intwt; Jt(2,2) = intwt;

  std::cout.precision(16);
  while ( prog < N_-1 )
    {
      ct += dt_;
      csol[0] = sol(0,prog); csol[1] = sol(1,prog); csol[2] = sol(2,prog);
      psol[0] = sol(0,prog); psol[1] = sol(1,prog); psol[2] = sol(2,prog);
      if ( prog > 0 )
	{ 
	  //ppsol[0] = sol(0,prog-1); ppsol[1] = sol(1,prog-1); 
	  //ppsol[2] = sol(2,prog-1);
	  ppsol[0] = sol(0,prog); ppsol[1] = sol(1,prog); 
	  ppsol[2] = sol(2,prog);
	}
      else
	{
	  ppsol[0] = sol(0,prog); ppsol[1] = sol(1,prog); ppsol[2] = sol(2,prog);
	}
      psol[0] = ( 4. * psol[0] - ppsol[0] ) / 3.;
      psol[1] = ( 4. * psol[1] - ppsol[1] ) / 3.;
      psol[2] = ( 4. * psol[2] - ppsol[2] ) / 3.;
      t1[0] = r[0] * csol[0]; t1[1] = r[1] * csol[1]; t1[2] = r[2] * csol[2];
      t2 = ov;
      t2.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, -1.0, alpha, csol, 1.0);
      res[0] = (csol[0] - psol[0] ) * intwt -t1[0] * t2[0];
      res[1] = (csol[1] - psol[1] ) * intwt -t1[1] * t2[1];
      res[2] = (csol[2] - psol[2] ) * intwt -t1[2] * t2[2];
      Real res_norm = res.normFrobenius();
      while ( res_norm > 1e-12 )
	{
	  j1[0] = r[0] * t2[0]; j1[1] = r[1] * t2[1]; j1[2] = r[2] * t2[2];
	  J1(0,0) = j1[0]; J1(1,1) = j1[1]; J1(2,2) = j1[2];
	  for ( int j = 0 ; j < 3; j++ )
	    {
	      for ( int i = 0 ; i < 3; i++ )
		{
		  J2(i,j) = - t1[i] * alpha(i,j);
		  J(i,j) = -( Jt(i,j) - J1(i,j) - J2(i,j) );
		}
	    }
	  
	  RealMatrix dsol;
	  //qr_solve( J, res, dsol );
	  Pecos::lu_solve( J, res, dsol, false, Teuchos::NO_TRANS );
	  //int rank;
	  //RealVector singular_values;
	  //svd_solve( J, res, dsol, singular_values, rank );
	  //if ( rank != 3 ) throw( std::runtime_error("rank") );
	  csol[0] += dsol(0,0); csol[1] += dsol(1,0); csol[2] += dsol(2,0);
	  t1[0] = r[0] * csol[0]; t1[1] = r[1] * csol[1]; t1[2] = r[2] * csol[2];
	  t2 = ov;
	  t2.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, -1.0, alpha, csol, 
		       1.0);
	  res[0] = (csol[0] - psol[0] ) * intwt -t1[0] * t2[0];
	  res[1] = (csol[1] - psol[1] ) * intwt -t1[1] * t2[1];
	  res[2] = (csol[2] - psol[2] ) * intwt -t1[2] * t2[2];
	  res_norm = res.normFrobenius();
	}
      sol(0,prog+1) = csol[0]; sol(1,prog+1) = csol[1]; sol(2,prog+1) = csol[2];
      prog++;
    }

  ct = dt_;
  asol(0,N_-1) = phiT_[0]; asol(1,N_-1) = phiT_[1]; asol(2,N_-1) = phiT_[2];
  prog = 0;
  while ( ct < finalTime_ )
    {
      ct += dt_;
      csol[0] = 0.5 * ( sol(0,N_-1-prog) + sol(0,N_-2-prog) );
      csol[1] = 0.5 * ( sol(1,N_-1-prog) + sol(1,N_-2-prog) );
      csol[2] = 0.5 * ( sol(2,N_-1-prog) + sol(2,N_-2-prog) );
      psol[0] = asol(0,N_-1-prog); 
      psol[1] = asol(1,N_-1-prog); 
      psol[2] = asol(2,N_-1-prog);
      if ( prog > 0 )
	{ 
	  //ppsol[0] = asol(0,N_-prog); ppsol[1] = asol(1,N_-prog); 
	  //ppsol[2] = asol(2,N_-prog);
	  ppsol[0] = asol(0,N_-prog-1); ppsol[1] = asol(1,N_-prog-1); 
	  ppsol[2] = asol(2,N_-prog-1);
	}
      else
	{
	  ppsol[0] = asol(0,N_-prog-1); ppsol[1] = asol(1,N_-prog-1); 
	  ppsol[2] = asol(2,N_-prog-1);
	}
      psol[0] = ( 4. * psol[0] - ppsol[0] ) / 3.;
      psol[1] = ( 4. * psol[1] - ppsol[1] ) / 3.;
      psol[2] = ( 4. * psol[2] - ppsol[2] ) / 3.;
      t1[0] = r[0] * csol[0]; t1[1] = r[1] * csol[1]; t1[2] = r[2] * csol[2];
      t2 = ov;
      t2.multiply( Teuchos::NO_TRANS, Teuchos::NO_TRANS, -1.0, alpha, csol, 1.0);
      psol *= intwt;
      j1[0] = r[0] * t2[0]; j1[1] = r[1] * t2[1]; j1[2] = r[2] * t2[2];
      J1(0,0) = j1[0]; J1(1,1) = j1[1]; J1(2,2) = j1[2];
      for ( int j = 0 ; j < 3; j++ )
	{
	  for ( int i = 0 ; i < 3; i++ )
	    {
	      J2(j,i) = - t1[j] * alpha(j,i);
	      J(i,j) = ( Jt(i,j) - J1(j,i) - J2(j,i) );
	    }
	}
      RealMatrix dsol;
      //qr_solve( J, psol, dsol );
      Pecos::lu_solve( J, psol, dsol , false, Teuchos::NO_TRANS  );
      //int rank;
      //RealVector singular_values;
      //svd_solve( J, psol, dsol, singular_values, rank );
      asol(0,N_-prog-2) = dsol(0,0);
      asol(1,N_-prog-2) = dsol(1,0);
      asol(2,N_-prog-2) = dsol(2,0);
      prog++;
    }
  asol(0,0) = 0.; asol(1,0) = 0.; asol(2,0) = 0.;
  

  //adjoint is computed for sol2, (N_-1) QOI
  result[0] = sol(2,N_-1);
  result[1] = sol(1,N_-1);
  result[2] = sol(0,N_-1);

  //The following computes error estimate can activate if we want
  //RealVector sol_flat( Teuchos::Copy, sol.values(), fwdNDOF_ ),
    //  asol_flat( Teuchos::Copy, asol.values(), adjNDOF_ );
  //result[1] = compute_error_estimate( x, sol_flat, asol_flat );
}

} // namepsace Dakota
