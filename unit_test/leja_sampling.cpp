#include "dakota_global_defs.hpp"
#include "DakotaResponse.hpp"
#include "DistributionParams.hpp"
#include "BasisPolynomial.hpp"
//#include "SharedPolyApproxData.hpp"
#include "nested_sampling.hpp"
#include "SharedOrthogPolyApproxData.hpp"
// Boost.Test
#include <boost/test/minimal.hpp>

//#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/poisson_distribution.hpp>
#include <boost/random/binomial_distribution.hpp>
#include <boost/random/negative_binomial_distribution.hpp>


namespace Dakota {
namespace TestLejaSampling {

/** \brief Create an AleatoryDistParams object such that all variables are 
 * uniform in [l_bnd, u_bnd]

  Note this function sets specific distribution parameters 
  that are not general. They must coincide with the parameters used
  in generate_samples

 */
Pecos::AleatoryDistParams 
initialize_homgeneous_uniform_aleatory_dist_params( short utype, 
						    int num_vars ){

  RealVector nuv_means;
  RealVector nuv_std_devs;     RealVector nuv_l_bnds;
  RealVector nuv_u_bnds;       RealVector lnuv_means;
  RealVector lnuv_std_devs;    RealVector lnuv_lambdas;
  RealVector lnuv_zetas;       RealVector lnuv_err_facts;
  RealVector lnuv_l_bnds;      RealVector lnuv_u_bnds;
  RealVector uuv_l_bnds;       RealVector uuv_u_bnds;
  RealVector luuv_l_bnds;      RealVector luuv_u_bnds;
  RealVector tuv_modes;        RealVector tuv_l_bnds;
  RealVector tuv_u_bnds;       RealVector euv_betas;
  RealVector beuv_alphas;      RealVector beuv_betas;
  RealVector beuv_l_bnds;      RealVector beuv_u_bnds;
  RealVector gauv_alphas;      RealVector gauv_betas;
  RealVector guuv_alphas;      RealVector guuv_betas;
  RealVector fuv_alphas;       RealVector fuv_betas;
  RealVector wuv_alphas;       RealVector wuv_betas;
  RealRealMapArray hbuv_prs;   RealVector puv_lambdas;
  RealVector biuv_p_per_tr;    IntVector biuv_num_trials; 
  RealVector nbuv_p_per_tr;    IntVector nbuv_num_trials; 
  RealVector geuv_p_per_tr;    IntVector hguv_tot_pop;
  IntVector hguv_sel_pop;      IntVector hguv_num_drawn;
  IntRealMapArray hpuiv_prs;   StringRealMapArray hpusv_prs;
  RealRealMapArray hpurv_prs;  RealSymMatrix uv_corr;

  switch (utype){
  case Pecos::STD_UNIFORM:
    uuv_l_bnds.sizeUninitialized( num_vars );
    uuv_u_bnds.sizeUninitialized( num_vars );
    uuv_l_bnds = -1; uuv_u_bnds = 1;
    break;
  case Pecos::POISSON:
    puv_lambdas.sizeUninitialized( num_vars );
    puv_lambdas = 5;
    break;
  case Pecos::BINOMIAL:
    biuv_p_per_tr.sizeUninitialized( num_vars );
    biuv_num_trials.sizeUninitialized( num_vars );
    biuv_p_per_tr = 0.5;
    biuv_num_trials = 20;
    break;
  case Pecos::NEGATIVE_BINOMIAL:
    nbuv_p_per_tr.sizeUninitialized( num_vars );
    nbuv_num_trials.sizeUninitialized( num_vars );
    nbuv_p_per_tr = 0.5;
    nbuv_num_trials = 20;
    break;
  }
  Pecos::AleatoryDistParams adp(nuv_means,
				nuv_std_devs,     nuv_l_bnds,
				nuv_u_bnds,       lnuv_means,
				lnuv_std_devs,    lnuv_lambdas,
				lnuv_zetas,       lnuv_err_facts,
				lnuv_l_bnds,      lnuv_u_bnds,
				uuv_l_bnds,       uuv_u_bnds,
				luuv_l_bnds,      luuv_u_bnds,
				tuv_modes,        tuv_l_bnds,
				tuv_u_bnds,       euv_betas,
				beuv_alphas,      beuv_betas,
				beuv_l_bnds,      beuv_u_bnds,
				gauv_alphas,      gauv_betas,
				guuv_alphas,      guuv_betas,
				fuv_alphas,       fuv_betas,
				wuv_alphas,       wuv_betas,
				hbuv_prs,         puv_lambdas,
				biuv_p_per_tr,    biuv_num_trials, 
				nbuv_p_per_tr,    nbuv_num_trials, 
				geuv_p_per_tr,    hguv_tot_pop,
				hguv_sel_pop,     hguv_num_drawn,
				hpuiv_prs,        hpusv_prs,
				hpurv_prs,  uv_corr);
  return adp;
  }

  template <typename T>
  void generate_samples( T& rvt, int num_vars, int num_samples, 
			 RealMatrix &samples ){
    samples.shapeUninitialized( num_vars, num_samples );
    for(int i=0; i<num_vars; i++) 
      for(int j=0; j<num_samples; j++) 
	samples(i,j) = rvt();
  }

  template void generate_samples(boost::variate_generator< boost::mt19937, 
				 boost::random::uniform_real_distribution<Real> > &rvt, 
				 int num_vars, int num_samples, 
				 RealMatrix &samples);
  template void generate_samples(boost::variate_generator< boost::mt19937, 
				 boost::random::poisson_distribution<> > &rvt, 
				 int num_vars, int num_samples, 
				 RealMatrix &samples);
  template void generate_samples(boost::variate_generator< boost::mt19937, 
				 boost::random::binomial_distribution<> > &rvt, 
				 int num_vars, int num_samples, 
				 RealMatrix &samples);
  template void generate_samples(boost::variate_generator< boost::mt19937, 
				 boost::random::negative_binomial_distribution<> > &rvt, 
				 int num_vars, int num_samples, 
				 RealMatrix &samples);

  /**  Note this function sets specific distribution parameters 
       that are not general. They must coincide with the parameters used
       in initialize_homgeneous_uniform_aleatory_dist_params
  */
void generate_samples(short utype, int num_vars, int num_candidate_samples, 
		      int seed, RealMatrix &candidate_samples){
    boost::mt19937 gen;
    gen.seed(seed);

    switch (utype){
    case Pecos::STD_UNIFORM:
      {
	boost::random::uniform_real_distribution<Real> un_dist(-1.,1.);
	boost::variate_generator< boost::mt19937, 
	  boost::random::uniform_real_distribution<Real> > un_rvt(gen, un_dist);
	generate_samples( un_rvt, num_vars, num_candidate_samples, 
			  candidate_samples );
	break;
      }
    case Pecos::POISSON:
      {
	Real poisson_lambda = 5;
	boost::poisson_distribution<> po_dist(poisson_lambda);
	boost::variate_generator< boost::mt19937, 
	  boost::poisson_distribution<> > po_rvt(gen, po_dist);
	generate_samples( po_rvt, num_vars, num_candidate_samples, 
			  candidate_samples );
      break;
    }
  case Pecos::BINOMIAL:
    {
      int n=20;
      Real p=0.5;
      boost::random::binomial_distribution<> bi_dist(n,p);
      boost::variate_generator<boost::mt19937, 
	boost::random::binomial_distribution<> > bi_rvt(gen, bi_dist);
      generate_samples( bi_rvt, num_vars, num_candidate_samples, 
			candidate_samples );
      break;
    }
  case Pecos::NEGATIVE_BINOMIAL:
    {
      int n=20;
      Real p=0.5;
      boost::random::negative_binomial_distribution<> nbi_dist(n,p);
      boost::variate_generator<boost::mt19937, 
	boost::random::negative_binomial_distribution<> > 
	nbi_rvt(gen, nbi_dist);
      generate_samples( nbi_rvt, num_vars, num_candidate_samples, 
			candidate_samples );
    }
    break;
  }
}

void test_leja_sequence_helper(short utype, int num_vars, 
			       int num_initial_samples, int num_new_samples,
			       const RealMatrix &candidate_samples){

  // Currently Leja Sampler assumes that the samples are in U space
  // that is the native space of the polynomial. For Legendre polynomials
  // the parameter range is [-1,1]
  Pecos::AleatoryDistParams adp = 
    initialize_homgeneous_uniform_aleatory_dist_params(utype, num_vars);
  ShortArray u_types(num_vars,utype);

  // Build polynomial basis using default basis configuration options
  Pecos::BasisConfigOptions bc_options;
  std::vector<Pecos::BasisPolynomial> poly_basis;
  Pecos::SharedOrthogPolyApproxData::construct_basis(u_types, adp, bc_options, 
						     poly_basis);

  // Initialize sampling object
  LejaSampler sampler;
  sampler.set_precondition(true);
  sampler.set_polynomial_basis(poly_basis);
  
  // Generate initial samples
  RealMatrix initial_samples, empty_matrix;
  // The test only works if we use the same degree at every step.
  int num_total_samples = num_initial_samples + num_new_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  // First time sampler is called there are no samples we wish to reuse
  // so pass in empty matrix
  sampler.Sampler::enrich_samples( num_vars, empty_matrix, num_initial_samples,
				   candidate_samples, initial_samples );

  // Enrich initial_samples
  RealMatrix enriched_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  sampler.Sampler::enrich_samples( num_vars, initial_samples, num_new_samples,
				   candidate_samples, enriched_samples );

  RealMatrix enriched_samples_subset( Teuchos::Copy, enriched_samples, num_vars,
				      num_initial_samples );
  enriched_samples_subset -= initial_samples;
  BOOST_CHECK( enriched_samples_subset.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );

  // Run sampler and generate all points in one go
  RealMatrix combined_samples;
  sampler.set_total_degree_basis_from_num_samples(num_vars, num_total_samples);
  sampler.Sampler::enrich_samples( num_vars, empty_matrix, num_total_samples,
				   candidate_samples, combined_samples );

  // Check samples obtained by running algorithm once are the same
  // as those obtained running the algorithm in steps.
  combined_samples -= enriched_samples;
  BOOST_CHECK( combined_samples.normInf() < 
	       10.*std::numeric_limits<double>::epsilon() );
}

void test_uniform_leja_sequence(){
  int num_vars = 2;
  int num_initial_samples = 10;
  int num_new_samples = 20;
  int num_candidate_samples = 10000;
  int seed = 1;

  int num_utype_tests = 2;
  short utypes[] = { Pecos::STD_UNIFORM, Pecos::POISSON };
  RealMatrix candidate_samples;

  for (int i=0; i< num_utype_tests; i++){
    generate_samples( utypes[i], num_vars, num_candidate_samples, seed,
		      candidate_samples );
    
    test_leja_sequence_helper(utypes[i], num_vars, num_initial_samples, 
			      num_new_samples, candidate_samples );
  }
}

} // end namespace TestFieldCovariance
} // end namespace Dakota

// NOTE: Boost.Test framework provides the main progran driver

//____________________________________________________________________________//

int test_main( int argc, char* argv[] )      // note the name!
{
  using namespace Dakota::TestLejaSampling;

  test_uniform_leja_sequence();

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}
