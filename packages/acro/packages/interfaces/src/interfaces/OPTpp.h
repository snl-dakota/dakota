/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file OPTpp.h
 *
 * Defines the interfaces::Cobyla class.
 */

#ifndef interfaces_OPTpp_h
#define interfaces_OPTpp_h

#include <acro_config.h>
#ifdef ACRO_USING_OPTPP
#include <colin/Solver.h>
#include <colin/CommonOptions.h>
#if 0 // disable OPT++ interface
// Consider conditional inclusion of Teuchos headers (can't forward declare)?
#include <Teuchos_SerialDenseVector.hpp>
#include <Teuchos_SerialSymDenseMatrix.hpp>
#endif // disable OPT++ interface

namespace interfaces
{

#if 0 // disable OPT++ interface

typedef Teuchos::SerialDenseVector<int, double> TeuchosVector;
typedef Teuchos::SerialSymDenseMatrix<int,double> TeuchosSymMatrix;

/** A base class for NLP0 optimizers for OPT++
  */
class OptppNLP0 : public colin::Solver<colin::NLP0_problem>
{
public:

   /// Constructor
   OptppNLP0();

   ///
   ~OptppNLP0();

private:

   ///
   OptppNLP0(const OptppNLP0&) : nlf(0) {}

   ///
   OptppNLP0& operator=(const OptppNLP0&) {return *this;}

   ///
   void reset_OptppNLP0();

protected:

   ///
   class NLF0;

   ///
   NLF0* nlf;

};


/** A base class for NLP1 optimizers for OPT++
  */
class OptppNLP1 : public colin::Solver<colin::NLP1_problem>
{
public:

   /// Constructor
   OptppNLP1();

   ///
   ~OptppNLP1();

private:

   ///
   OptppNLP1(const OptppNLP1&) {}

   ///
   OptppNLP1& operator=(const OptppNLP1&) {return *this;}

   ///
   void reset_OptppNLP1();

protected:

   ///
   class NLF1;

   ///
   NLF1* nlf;

};


/** A base class for NLP2 optimizers for OPT++
  */
class OptppNLP2 : public colin::Solver<colin::NLP2_problem>
{
public:

   /// Constructor
   OptppNLP2();

   ///
   ~OptppNLP2();

private:

   ///
   OptppNLP2(const OptppNLP2&) {}

   ///
   OptppNLP2& operator=(const OptppNLP2&) {return *this;}

   ///
   void reset_OptppNLP2();

protected:

   ///
   class NLF2;

   ///
   NLF2* nlf;

};


class NLFOptions
{
public:

   NLFOptions(utilib::PropertyDict& properties)
   {
      is_expensive = false;
      properties.declare
         ( "expensive", 
           "Determines whether or not function is expensive, in which "
           "case a value-based line-search is used instead of a "
           "gradient-based line-search",
           utilib::Privileged_Property(is_expensive) );
      //option.add("expensive", is_expensive,
      //           "Determines whether or not function is expensive, in which case a value-based line-search is used instead of a gradient-based line-search");

      mode_override = false;
      properties.declare
         ( "mode-override", 
           "Allow the function, gradient, and Hessian to be computed "
           "all in a single evaluation request",
           utilib::Privileged_Property(mode_override) );
      //option.add("mode-override", mode_override,
      //           "Allow the function, gradient, and Hessian to be computed all in a single evaluation request");

      finitediff = "forward";
      properties.declare
         ( "gradient-fd", 
           "Set finite-difference approach for gradients: {forward, "
           "backward, central}.  This option only applies to FDNLF1 "
           "optimizers",
           utilib::Privileged_Property(finitediff) );
      //option.add("gradient-fd", finitediff,
      //           "Set finite-difference approach for gradients: forward, backward, central.  This option only applies to FDNLF1 optimizers");

      properties.declare
         ( "fd-scaling", 
           "Set finite-difference scaling for all variables.  This "
           "option only to FDNLF1 optimizers.",
           utilib::Privileged_Property(fcn_accuracy) );
      //option.add("fd-scaling", fcn_accuracy,
      //           "Set finite-difference scaling for all variables.  This option only to FDNLF1 optimizers.");

      spec_grad = "none";
      properties.declare
         ( "spec-gradients", 
           "Perform speculative gradient operations in parallel execution: "
           "{no, spec1, or spec2}. This option only applies to "
           "FDNLF1 optimizers",
           utilib::Privileged_Property(spec_grad) );
      //option.add("spec-gradients", spec_grad,
      //           "Perform speculative gradient operations in parallel execution: no, spec1, or spec2. This option only applies to FDNLF1 optimizers");
   }

   bool is_expensive;
   bool mode_override;
   std::string finitediff;
   std::vector<double> fcn_accuracy;
   std::string spec_grad;

   template <class OptT, class NLFT, class ProblemT>
   void set_options(OptT* /*opt*/, NLFT* nlf, ProblemT& problem)
   {
      nlf->setIsExpensive(is_expensive);
      nlf->setModeOverride(mode_override);
      if (finitediff == "forward")
         nlf->setDerivOption(OPTPP::ForwardDiff);
      else if (finitediff == "backward")
         nlf->setDerivOption(OPTPP::BackwardDiff);
      else if (finitediff == "central")
         nlf->setDerivOption(OPTPP::CentralDiff);
      else
         EXCEPTION_MNGR(std::runtime_error, ucout << "ERROR: unexpected value for 'gradient-fd' option: " << finitediff << " Valid values are 'forward', 'backward' and 'central'.");
      if (fcn_accuracy.size() == 0)
      {
         fcn_accuracy.resize(problem->num_real_vars);
         fcn_accuracy << DBL_EPSILON;
      }
      else if ( problem->num_real_vars != fcn_accuracy.size() )
         EXCEPTION_MNGR(std::runtime_error, ucout << "ERROR: the value of the 'fd-scaling' option is an array with " << fcn_accuracy.size() << " values, but there are " << problem->num_real_vars << " decision variables.");
      TeuchosVector tmp;
      utilib::TypeManager()->lexical_cast(fcn_accuracy, tmp);
      nlf->setFcnAccrcy(tmp);
      if (spec_grad == "none")
         nlf->setSpecOption(OPTPP::NoSpec);
      else if (spec_grad == "spec1")
         nlf->setSpecOption(OPTPP::Spec1);
      else if (spec_grad == "spec2")
         nlf->setSpecOption(OPTPP::Spec2);
      else
         EXCEPTION_MNGR(std::runtime_error, ucout << "ERROR: unexpected value for 'spec-gradient' option: " << spec_grad << " Valid values are 'none', 'spec1' and 'spec2'.");

   }

};

class GenericOptions : public colin::CommonOptions
{
public:

   GenericOptions(utilib::Privileged_PropertyDict& properties)
         : colin::CommonOptions(properties)
   {
      double mcheps = DBL_EPSILON;

      max_step_size = 1.0e+3;
      properties.declare
         ( "max-step-size", 
           "The maximum step length allowed",
           utilib::Privileged_Property(max_step_size) );
      //option.add("max-step-size", max_step_size,
      //           "The maximum step length allowed");

      min_step_size = sqrt(mcheps);
      properties.declare
         ( "min-step-size", 
           "The minimum step length allowed",
           utilib::Privileged_Property(min_step_size) );
      //option.add("min-step-size", min_step_size,
      //           "The minimum step length allowed");

      step_tol = sqrt(mcheps);
      properties.declare
         ( "step-tolerance", 
           "The termination criteria based on step length",
           utilib::Privileged_Property(step_tol) );
      //option.add("step-tolerance", step_tol,
      //           "The termination criteria based on step length");

      fcn_tol = sqrt(mcheps);
      properties.declare
         ( "fn-tolerance", 
           "The termination criteria based on change in function value",
           utilib::Privileged_Property(fcn_tol) );
      //option.add("fn-tolerance", fcn_tol,
      //           "The termination criteria based on change in function value");

      con_tol = sqrt(mcheps);
      properties.declare
         ( "feasibility-tolerance", 
           "The tolerance for constraint feasibility",
           utilib::Privileged_Property(con_tol) );
      //option.add("feasibility-tolerance", con_tol,
      //           "The tolerance for constraint feasibility");

      grad_tol = pow(mcheps, 1.0 / 3.0);
      properties.declare
         ( "gradient-tolerance", 
           "The termination criteria based on gradient norm",
           utilib::Privileged_Property(grad_tol) );
      //option.add("gradient-tolerance", grad_tol,
      //           "The termination criteria based on gradient norm");

      linesearch_tol = 1.0e-4; 
      properties.declare
         ( "linesearch-tolerance", 
           "The tolerance used for sufficient decrease in line search",
           utilib::Privileged_Property(linesearch_tol) );
      //option.add("linesearch-tolerance", linesearch_tol,
      //           "The tolerance used for sufficient decrease in line search");

      max_backiter = 5;
      properties.declare
         ( "max_backiter", 
           "Maximum number of backtrack iterations allowed",
           utilib::Privileged_Property(max_backiter) );
      //option.add("max_backiter", max_backiter,
      //           "Maximum number of backtrack iterations allowed");

      max_neval = 1000;

      sx << 1.0;
      properties.declare
         ( "var_scale", 
           "Variable scaling",
           utilib::Privileged_Property(sx) );
      //option.add("var_scale", sx,
      //           "Variable scaling");

      properties.privilegedGet("debug").unset_readonly();
      properties.privilegedGet("max_neval").unset_readonly();
      properties.privilegedGet("max_iters").unset_readonly();
      //option.enable("debug");
      //option.enable("max_neval");
      //option.enable("max_iters");
   }

   double max_step_size;
   double min_step_size;
   double step_tol;
   double fcn_tol;
   double con_tol;
   double grad_tol;
   double linesearch_tol;
   int max_backiter;
   std::vector<double> sx;


   template <class OptT, class NLFT, class ProblemT>
   void set_options(OptT* opt, NLFT* /*nlf*/, ProblemT& /*problem*/)
   {
      if (debug)
         opt->setDebug();
      opt->setMaxStep(max_step_size);
      opt->setMinStep(min_step_size);
      opt->setStepTol(step_tol);
      opt->setLineSearchTol(linesearch_tol);
      opt->setMaxIter(max_iters);
      opt->setMaxBacktrackIter(max_backiter);
      opt->setMaxFeval(max_neval);
      TeuchosVector tmp;
      utilib::TypeManager()->lexical_cast(sx, tmp);
      opt->setXScale(tmp);
      opt->setOutputFile(std::cout);
   }

};


class NewtonOptions
{
public:

   NewtonOptions(utilib::PropertyDict& properties)
   {
      strategy = "TrustRegion";
      properties.declare
         ( "search-strategy", 
           "Set the globalization strategy: "
           "{LineSearch, TrustRegion, or TrustPDS}",
           utilib::Privileged_Property(strategy) );
      //option.add("search-strategy", strategy,
      //           "Set the globalization strategy: LineSearch, TrustRegion, or TrustPDS");

      TR_size = 0.0;
      properties.declare
         ( "tr-size", 
           "The size of the trust region.  If this is set to zero, "
           "then at runtime the trust region is initialized to the "
           "gradient multiplier * norm of gradient",
           utilib::Privileged_Property(TR_size) );
      //option.add("tr-size", TR_size,
      //           "The size of the trust region.  If this is set to zero, then at runtime the trust  region is initialized to the gradient multiplier * norm of gradient");

      gradMult = 0.1;
      properties.declare
         ( "gradient-multiplier", 
           "Multiplier for scaling gradient norm to set trust region size",
           utilib::Privileged_Property(gradMult) );
      //option.add("gradient-multiplier", gradMult,
      //           "Multiplier for scaling gradient norm to set trust region size");

      searchSize = 64;
      properties.declare
         ( "trustPDS-size", 
           "Number of pattern points for TrustPDS",
           utilib::Privileged_Property(searchSize) );
      //option.add("trustPDS-size", searchSize,
      //           "Number of pattern points for TrustPDS");
   }

   std::string strategy;
   double TR_size;
   double gradMult;
   unsigned int searchSize;

   template <class OptT, class NLFT, class ProblemT>
   void set_options(OptT* opt, NLFT* /*nlf*/, ProblemT& problem)
   {
      if ( problem->enforcing_domain_bounds )
         opt->setSearchStrategy(OPTPP::LineSearch);
      else
      {
         if (strategy == "TrustRegion")
            opt->setSearchStrategy(OPTPP::TrustRegion);
         else if (strategy == "LineSearch")
            opt->setSearchStrategy(OPTPP::LineSearch);
         else if (strategy == "TrustPDS")
            opt->setSearchStrategy(OPTPP::TrustPDS);
         else
            EXCEPTION_MNGR(std::runtime_error, "Unexpected value for option 'search-        strategy': '" << strategy << "'. Expected values are TrustRegion, LineSearch and       TrustPDS");
      }
      opt->setTRSize(TR_size);
      opt->setGradMult(gradMult);
      opt->setSearchSize(searchSize);
   }

};

/** An interface to the OPTpp Newton solver with NLP1
  */
class OptppNewton1 : public OptppNLP1, public NLFOptions, public GenericOptions, public NewtonOptions
{
public:

   /// Constructor
   OptppNewton1();

   ///
   void optimize();

   ///
   bool using_fd_hessian;

protected:

   ///
   std::string define_solver_type() const
      { return "optpp:newton_nlp1"; }

private:

   ///
   //void reset_OptppNewton1();

};


/** An interface to the OPTpp Newton solver with NLP2
  */
class OptppNewton2 : public OptppNLP2, public NLFOptions, public GenericOptions, public NewtonOptions
{
public:

   /// Constructor
   OptppNewton2();

   ///
   void optimize();

protected:

   ///
   std::string define_solver_type() const
      { return "optpp:newton_nlp2"; }

private:

   ///
   //void reset_OptppNewton2();

};


/** An interface to the OPTpp NIPS solver with NLP1
  */
class OptppNIPS1 : public OptppNLP1, public NLFOptions, public GenericOptions
{
public:

   /// Constructor
   OptppNIPS1();

   ///
   void optimize();

   ///
   bool using_fd_hessian;

   std::string MeritFcn;
   double sigmin;
   double taumin;

protected:

   template <class OptT>
   void set_options(OptT* opt)
   {
      if (MeritFcn == "NormFmu")
         opt->setMeritFcn(OPTPP::NormFmu);
      else if (MeritFcn == "ArgaezTapia")
         opt->setMeritFcn(OPTPP::ArgaezTapia);
      else if (MeritFcn == "VanShanno")
         opt->setMeritFcn(OPTPP::VanShanno);
      else
         EXCEPTION_MNGR(std::runtime_error, "Unknown value for option 'merit-fn': '" << MeritFcn << "'. Expected NormFmu, ArgaezTapia or VanShanno");
      if (sigmin > 0.0)
         opt->setCenteringParameter(sigmin);
      if (taumin > 0.0)
         opt->setStepLengthToBdry(sigmin);
   }

   ///
   std::string define_solver_type() const
   { return "optpp:nips_nlp1"; }

private:

   ///
   //void reset_OptppNIPS1();

};


/** An interface to the OPTpp NIPS solver with NLP2
  */
class OptppNIPS2 : public OptppNLP2, public NLFOptions, public GenericOptions
{
public:

   /// Constructor
   OptppNIPS2();

   ///
   void optimize();

   std::string MeritFcn;
   double sigmin;
   double taumin;

protected:

   template <class OptT>
   void set_options(OptT* opt)
   {
      if (MeritFcn == "NormFmu")
         opt->setMeritFcn(OPTPP::NormFmu);
      else if (MeritFcn == "ArgaezTapia")
         opt->setMeritFcn(OPTPP::ArgaezTapia);
      else if (MeritFcn == "VanShanno")
         opt->setMeritFcn(OPTPP::VanShanno);
      else
         EXCEPTION_MNGR(std::runtime_error, "Unknown value for option 'merit-fn': '" << MeritFcn << "'. Expected NormFmu, ArgaezTapia or VanShanno");
      if (sigmin > 0.0)
         opt->setCenteringParameter(sigmin);
      if (taumin > 0.0)
         opt->setStepLengthToBdry(sigmin);
   }

   ///
   std::string define_solver_type() const
   { return "optpp:nips_nlp2"; }

private:

   ///
   //void reset_OptppNIPS2();

};


/** An interface to the OPTpp CG solver with NLP1
  */
class OptppCG : public OptppNLP1, public NLFOptions, public GenericOptions
{
public:

   /// Constructor
   OptppCG();

   ///
   void optimize();

protected:

   ///
   std::string define_solver_type() const
      { return "optpp:cg"; }

private:

   ///
   //void reset_OptppCG();

};


/** An interface to the OPTpp LBFGS solver with NLP1
  */
class OptppLBFGS : public OptppNLP1, public NLFOptions, public GenericOptions
{
public:

   /// Constructor
   OptppLBFGS();

   ///
   void optimize();

protected:

   ///
   std::string define_solver_type() const
      { return "optpp:lbfgs"; }

private:

   ///
   //void reset_OptppLBFGS();

};


/** An interface to the OPTpp PDS solver with NLP0
  */
class OptppPDS : public OptppNLP0, public GenericOptions
{
public:

   /// Constructor
   OptppPDS();

   ///
   void optimize();

   int search_scheme_size;
   int simplex_type;
   std::vector<double> vscales;
   //currently unsupported
   //NEWMAT::Matrix simplex_matrix;
   std::string scheme_filename;

protected:

   ///
   std::string define_solver_type() const
      { return "optpp:pds"; }

private:

   ///
   //void reset_OptppPDS();

};

#endif // disable OPT++ interface

} // namespace interfaces
#endif

#endif
