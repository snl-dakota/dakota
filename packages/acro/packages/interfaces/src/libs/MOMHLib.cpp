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

//
// MOMHLib.cpp
//

#include <acro_config.h>
#ifdef ACRO_USING_SCOLIB
#ifdef ACRO_USING_MOMHLIB
#include <interfaces/MOMHLib.h>
#include <colin/SolverMngr.h>
#include <scolib/DomainOpsMixedInteger.h>
#include <scolib/DomainInfoMixedInteger.h>

namespace interfaces {


namespace {

///
/// This is a specialization of the MOMHLibProblem class that
/// contains data that is handy to have around for mixed-integer applications
///
class Problem : public MOMHLibProblem<colin::MO_UMINLP0_problem>
{
public:

   ///
   Problem() 
      : properties(), 
        mi_ops(properties) 
   {}

   ///
   virtual ~Problem() {}

   ///
   virtual void reset()
	{
	MOMHLibProblem<colin::MO_UMINLP0_problem>::reset();
        rlower = problem->real_lower_bounds;
        rupper = problem->real_upper_bounds;
        ilower = problem->int_lower_bounds;
        iupper = problem->int_upper_bounds;
	utilib::MixedIntVars tmp;
	mi_ops.initialize(problem,100,tmp,1.0,1.0);
	mi_ops.reset();
	}

   ///
   void apply_xover(utilib::MixedIntVars& p1, utilib::MixedIntVars& p2, utilib::MixedIntVars& c)
	{ mi_ops.apply_xover(p1,dummy,p2,dummy,c,dummy); }

   ///
   void apply_mutation(utilib::MixedIntVars& p1)
	{ mi_ops.apply_mutation(p1,dummy,-1); }

   ///
   utilib::BasicArray<colin::real> rlower;

   ///
   utilib::BasicArray<colin::real> rupper;

   ///
   utilib::BasicArray<int> ilower;

   ///
   utilib::BasicArray<int> iupper;

   ///
   scolib::DomainInfoMixedInteger dummy;

   // NB: This is clearly broken: The problem should never be held as a
   // static instance, and because it is, there is no way to properly
   // initialize the problem properties so that they appear as part of
   // the solver's PropertyDict.  As a result -- just to get things
   // compiling, we will have a separate PropertyDict here.

   ///
   utilib::PropertyDict properties;

   ///
   scolib::DomainOpsMixedInteger<scolib::DomainInfoMixedInteger> mi_ops;
};

class Solution;

typedef MOMHLib<Problem,Solution,colin::MO_UMINLP0_problem> solver;

//
// This class is defined here to provide a concrete implementation 
// for MOMHLib with mixed-integer variables.
//
class Solution: public TMOMHSolution
{
public:

   /// Constructor
   Solution()
	{
	resize();
	}

   /// Constructor
   Solution(const Solution& solution)
   { 
	resize();
	point << solution.point;
	}

   /// Construct new random solution
   Solution(TScalarizingFunctionType type,
            TPoint& point,
            TWeightVector weights,
            TNondominatedSet& NondominatedSet)
	{
	resize();
	}

   /// Constructs new solution by recombination of point1 and point2
   /// and calls RestoreFeasibility
   Solution(Solution& point1, Solution& point2,
            TScalarizingFunctionType type,
            TPoint& rpoint,
            TWeightVector weights,
            TNondominatedSet& NondominatedSet)
	: TMOMHSolution(point1,point2,type,rpoint,weights,NondominatedSet)
	{
	resize();
	solver::momhlib_problem.apply_xover(point1.point,point2.point,point);
	//RestoreFeasibility(rpoint);
	}

   /// Constructs new solution by recombination of point1 and point2
   Solution(Solution& point1, Solution& point2)
	: TMOMHSolution(point1,point2)
	{
	resize();
	solver::momhlib_problem.apply_xover(point1.point,point2.point,point);
	//RestoreFeasibility(*this);
	WeightVector = GetRandomWeightVector();
	}

   /// Destructor
   virtual ~Solution() {}

   ///
   void resize()
	{
	point.resize( solver::momhlib_problem.problem->num_binary_vars,
		      solver::momhlib_problem.problem->num_int_vars,
		      solver::momhlib_problem.problem->num_real_vars );
	saved_point.resize( solver::momhlib_problem.problem->num_binary_vars,
		      solver::momhlib_problem.problem->num_int_vars,
		      solver::momhlib_problem.problem->num_real_vars );
	}

   ///
   ostream& Save(ostream& Stream)
   {
      TMOMHSolution::Save(Stream);
      Stream << point;
      return Stream;
   }

   ///
   void FindLocalMove()
	{
	SaveObjectiveValues();
	saved_point << point;
	solver::momhlib_problem.apply_mutation(point);
	//RestoreFeasibility(*this);
	}

   ///
   void RejectLocalMove()
	{
	RestoreObjectiveValues();
	point << saved_point; 
	}

   ///
   void Mutate()
	{
	solver::momhlib_problem.apply_mutation(point);
	//RestoreFeasibility(*this);
	}

   ///
   //void LocalSearch(TPoint&);

   ///
   utilib::MixedIntVars point;

   ///
   utilib::MixedIntVars saved_point;

};

typedef interfaces::MOMHLib<Problem,Solution,colin::MO_UMINLP0_problem> momhlib_solver_t;

class momh_psa : public momhlib_solver_t
{ public: momh_psa() : momhlib_solver_t() { initialize("PSA"); } };

class momh_mosa : public momhlib_solver_t
{ public: momh_mosa() : momhlib_solver_t() { initialize("MOSA"); } };

class momh_smosa : public momhlib_solver_t
{ public: momh_smosa() : momhlib_solver_t() { initialize("SMOSA"); } };

class momh_mogls : public momhlib_solver_t
{ public: momh_mogls() : momhlib_solver_t() { initialize("MOGLS"); } };

class momh_immogls : public momhlib_solver_t
{ public: momh_immogls() : momhlib_solver_t() { initialize("IMMOGLS"); } };

class momh_pma : public momhlib_solver_t
{ public: momh_pma() : momhlib_solver_t() { initialize("PMA"); } };

class momh_momsls : public momhlib_solver_t
{ public: momh_momsls() : momhlib_solver_t() { initialize("MOMSLS"); } };

class momh_spea : public momhlib_solver_t
{ public: momh_spea() : momhlib_solver_t() { initialize("SPEA"); } };

class momh_nsgaii : public momhlib_solver_t
{ public: momh_nsgaii() : momhlib_solver_t() { initialize("NSGAII"); } };

class momh_nsgaiic : public momhlib_solver_t
{ public: momh_nsgaiic() : momhlib_solver_t() { initialize("NSGAIIC"); } };

class momh_nsga : public momhlib_solver_t
{ public: momh_nsga() : momhlib_solver_t() { initialize("NSGA"); } };

}

REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_psa,"momh:PSA","momh:psa", "The MOMHLib PSA optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_mosa,"momh:MOSA","momh:mosa", "The MOMHLib MOSA optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_smosa,"momh:SMOSA","momh:smosa", "The MOMHLib SMOSA optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_mogls,"momh:MOGLS","momh:mogls", "The MOMHLib MOGLS optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_immogls,"momh:IMMOGLS","momh:immogls", "The MOMHLib IMMOGLS optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_pma,"momh:PMA","momh:pma", "The MOMHLib PMA optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_momsls,"momh:MOMSLS","momh:momsls", "The MOMHLib MOMSLS optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_nsgaii,"momh:NSGAII","momh:nsgaii", "The MOMHLib NSGAII optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_nsgaiic,"momh:NSGAIIC","momh:nsgaiic", "The MOMHLib NSGAIIC optimizer")
REGISTER_COLIN_SOLVER_WITH_ALIAS(momh_nsga,"momh:NSGA","momh:nsga", "The MOMHLib NSGA optimizer")

}

#endif
#endif
