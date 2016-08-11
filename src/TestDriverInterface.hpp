/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TestDriverInterface
//- Description:  Direct interfaces to test drivers and "simple" linked
//-               applications that don't require separate setup and tear-down
//- Owner:        Mike Eldred, Brian Adams
//- Version: $Id$

#ifndef TEST_DRIVER_INTERFACE_H
#define TEST_DRIVER_INTERFACE_H

#include "DirectApplicInterface.hpp"

namespace Dakota {

/** Specialization of DirectApplicInterface to embed algebraic test function
    drivers directly in Dakota */
class TestDriverInterface: public DirectApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  TestDriverInterface(const ProblemDescDB& problem_db); ///< constructor
  ~TestDriverInterface();                               ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const Dakota::String& ac_name);

private:

  //
  //- Heading: Simulators and test functions
  //

  int cantilever();   ///< scaled cantilever test function for optimization
  int mod_cantilever(); ///< unscaled cantilever test function for UQ
  int cyl_head();     ///< the cylinder head constrained optimization test fn
  int multimodal();   ///< multimodal UQ test function
  int log_ratio();    ///< the log_ratio UQ test function

  int short_column(); ///< the short_column UQ/OUU test function
  int lf_short_column(); ///< a low fidelity short_column test function
  int mf_short_column(); ///< alternate short_column formulations for
                         ///< multifidelity or model form studies
  int alternate_short_column_forms(int form); ///< helper fn for alternate forms

  int side_impact_cost(); ///< the side_impact_cost UQ/OUU test function
  int side_impact_perf(); ///< the side_impact_perf UQ/OUU test function

  int rosenbrock();  ///< the Rosenbrock optimization and least squares test fn
  int modified_rosenbrock();  ///< the modified Rosenbrock optimization and
  /// least squares test fn. The modification is the addition of an sin^2
  /// term so that function can not be exactly approximated by a low degree polynomial
  int generalized_rosenbrock(); ///< n-dimensional Rosenbrock (Schittkowski)
  int extended_rosenbrock();    ///< n-dimensional Rosenbrock (Nocedal/Wright)
  int lf_rosenbrock(); ///< a low fidelity version of the Rosenbrock function
  int extra_lf_rosenbrock(); ///< an extra low fidelity version of the Rosenbrock function
  int mf_rosenbrock(); ///< alternate Rosenbrock formulations for
                       ///< multifidelity or model form studies

  int lf_poly_prod(); ///< modified low fidelity Rosenbrock to test SBO with
                      ///< hierarchical approximations
  int poly_prod();    ///< modified low fidelity Rosenbrock to test SBO with
                      ///< hierarchical approximations

  int gerstner(); ///< the isotropic/anisotropic Gerstner test function family
  int scalable_gerstner(); ///< scalable versions of the Gerstner test family

  /// define coefficients needs for genz model
  void get_genz_coefficients( int num_dims, Real factor,
			      int c_type,
			      RealVector &c, RealVector &w );
  int genz(); ///< scalable test functions from the Genz test suite

  int damped_oscillator(); ///< 1d-6d that returns field values (ode solution)
  int steady_state_diffusion_1d(); ///< solve the 1d steady-state diffusion eqn
                                   ///< with uncertain field diffusivity
  int transient_diffusion_1d(); ///< solve the 1d transient diffusion equation
                                ///< with uncertain scalar diffusivity
  int predator_prey(); /// solve a predator prey population dynamics model

  int steel_column_cost(); ///< the steel_column_cost UQ/OUU test function
  int steel_column_perf(); ///< the steel_column_perf UQ/OUU test function

  int sobol_rational();   ///< Sobol SA rational test function
  int sobol_g_function(); ///< Sobol SA discontinuous test function
  int sobol_ishigami();   ///< Sobol SA transcendental test function

  int text_book();     ///< the text_book constrained optimization test function
  int text_book1();    ///< portion of text_book() evaluating the objective fn
  int text_book2();    ///< portion of text_book() evaluating constraint 1
  int text_book3();    ///< portion of text_book() evaluating constraint 2
  int text_book_ouu(); ///< the text_book_ouu OUU test function
  int scalable_text_book(); ///< scalable version of the text_book test function
  int scalable_monomials(); ///< simple monomials for UQ exactness testing

  // multi-objective test functions
  int mogatest1();  ///< MOP2 from Van Veldhuizen, pp. 5-13
  int mogatest2();  ///< MOP2? from Van Veldhuizen, pp. 5-13
  int mogatest3();  ///< Srinivas' from Van Veldhuizen, pp. B-5

  int illumination(); ///< illumination example in Boyd as a general
                      ///< minimization problem
  int barnes();     ///< barnes test for SBO perforamnce from Rodriguez,
                    ///< Perez, Renaud, et al.
  int barnes_lf();  ///< lo-fi barnes test for SBO perforamnce

  // suite of separable test functions TODO add docs

  /// 1D components of herbie function
  void herbie1D(size_t der_mode, Real xc_loc, std::vector<Real> &w_and_ders);
  /// 1D components of smooth_herbie function
  void smooth_herbie1D(size_t der_mode, Real xc_loc,
		       std::vector<Real> &w_and_ders);
  /// 1D components of shubert function
  void shubert1D(size_t der_mode, Real xc_loc, std::vector<Real> &w_and_ders);

  int herbie();        ///< returns the N-D herbie function
  int smooth_herbie(); ///< returns the N-D smooth herbie function
  int shubert();       ///< returns the N-D shubert function
 
  /// Scalable test function for Bayesian methods, to estimate parameters 
  //  and error terms
  int bayes_linear();

  /// utility to combine components of separable fns
  void separable_combine(Real mult_scale_factor, std::vector<Real> & w,
			 std::vector<Real> & d1w, std::vector<Real> & d2w);
  /// Compute Levenshtein distance between v and LEV_REF
  Real levenshtein_distance(const String &v);
  /// Cache results of Levenshtein distance calc for efficiency
  static StringRealMap levenshteinDistanceCache;

#ifdef DAKOTA_SALINAS
  int salinas(); ///< direct interface to the SALINAS structural dynamics code
#endif

#ifdef DAKOTA_MODELCENTER
  int mc_api_run(); ///< direct interface to ModelCenter via API, HKIM 4/3/03
#endif

  // test functions for high dimensional models with active subspace structure

  int aniso_quad_form();     ///< 1-D function using a anisotropic quadratic
                             ///< form

};

} // namespace Dakota

#endif  // TEST_DRIVER_INTERFACE_H
