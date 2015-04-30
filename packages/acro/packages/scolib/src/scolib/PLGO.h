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
 * \file PLGO.h
 *
 * Defines the scolib::PLGO class.
 */

#ifndef scolib_PLGO_h
#define scolib_PLGO_h

#ifdef ACRO_USING_PEBBL
#include <acro_config.h>
#ifdef ACRO_USING_PLGO
#include <pebbl/serialLipshitzian.h>
#include <pebbl/parallelLipshitzian.h>
#include <colin/Solver.h>
#include <colin/real.h>
#include <colin/CommonOptions.h>

namespace scolib {

using utilib::BasicArray;
using utilib::ParameterSet;
using utilib::Ereal;
using colin::real;

class PLGO;


namespace plgo {

///
/// A class that defines an PLGO problem instance
///
class PLGOProblem
{
public:

  ///
  PLGOProblem() : solver(0) {}

  ///
  Ereal<double> operator()(BasicArray<double>& x);

  ///
  colin::Problem<colin::UNLP0_problem> problem;

#if 0
  ///
  colin::AppResponse response;
#endif

  ///
  BasicArray<real> lower;

  ///
  BasicArray<real> upper; 

  ///
  Ereal<double> val;

  ///
  Ereal<double> cval;

  ///
  PLGO* solver;
};


///
/// An abstract class that defines the particular solver
///
class PLGOSolver
{
public:

  /// Destructor
  virtual ~PLGOSolver() {}

  /// Get a ParameterSet object, which can be used to set parameters
  virtual ParameterSet& parameters() = 0;

  /// Reset the solver's state
  virtual void reset() = 0;

  /// Reset the solver's state
  virtual pebbl::branching* base() = 0;

  /// Perform minimization
  virtual void minimize(std::string& termination_info, BasicArray<double>& x, colin::real& val) = 0;

  /// Returns true if this is a serial solver
  virtual bool isSerial() = 0;

  /// Set the function used in the search
  virtual void set_func(PLGOProblem* func) = 0;
};


///
/// PLGO serial solver class
///
class PLGOSerialSolver : public PLGOSolver
{
public:

  /// The serial solver
  pebbl::serialLipshitzian<PLGOProblem> solver;

  ///
  ParameterSet& parameters()
	{ return solver; }

  ///
  void reset()
	{
	int argc=0;
	char** argv=0;
	solver.setup(argc,argv);
	solver.reset();
	}

  /// 
  pebbl::branching* base()
	{ return &solver; }

  ///
  void minimize(std::string& termination_info, BasicArray<double>& x, colin::real& val)
	{
	solver.solve();
	pebbl::arraySolution<double>* soln = static_cast<pebbl::arraySolution<double>*>(solver.incumbent);
	x << soln->array;
	val = soln->value;
        if (solver.abortReason)
	   termination_info = "Error";
        else
	   termination_info = "Successful";
	}

  ///
  bool isSerial()
	{ return true; }

  ///
  void set_func(PLGOProblem* func)
	{ solver.func = func; }
};


///
/// PLGO parallel solver class
///
#if defined(ACRO_HAVE_MPI)
class PLGOParallelSolver : public PLGOSolver
{
public:

  /// The serial solver
  pebbl::parallelLipshitzian<PLGOProblem> solver;

  ///
  ParameterSet& parameters()
	{ return solver; }

  ///
  void reset()
	{
	int argc=0;
	char** argv=0;
	solver.setup(argc,argv);
	solver.reset();
	}

  /// 
  pebbl::branching* base()
	{ return &solver; }

  ///
  void minimize(std::string& termination_info, BasicArray<double>& x, colin::real& val)
	{
	solver.solve();
	pebbl::arraySolution<double>* soln = static_cast<pebbl::arraySolution<double>*>(solver.incumbent);
	x << soln->array;
	val = soln->value;
        if (solver.abortReason)
	   termination_info = "Error";
        else
	   termination_info = "Successful";
	}


  ///
  bool isSerial()
	{ return false; }

  ///
  void set_func(PLGOProblem* func)
	{
#if 0
	 /// WEH - this isn't portable, so I'm commenting it out for now.
	 static_cast<pebbl::serialLipshitzian<PLGOProblem> >(solver).func 
			= func;
#endif
	}
};
#endif


}


/** An interface to the Lipshitzian Global Optimizer defined by PICO
  */
class PLGO : public colin::Solver<colin::NLP0_problem>, colin::CommonOptions
{
public:

  /// Constructor
  PLGO();

  ///
  virtual ~PLGO()
	{ delete solver; }

  ///
  void optimize();

protected:

  ///
  std::string define_solver_type() const
        {return "PLGO";}

  ///
  double step_tolerance;

  ///
  double initial_step;

  ///
  BasicArray<double> x;

  ///
  plgo::PLGOSolver* solver;

  ///
  plgo::PLGOProblem func;

  ///
  void reset_PLGO();

};

inline Ereal<double> plgo::PLGOProblem::operator()(BasicArray<double>& x)
{
colin::real val;
problem->EvalF(solver->eval_mngr(),x,val);
//return response->value<Ereal<double> >(colin::f_info);
return val;
}

} // namespace scolib
#endif


#endif
#endif
