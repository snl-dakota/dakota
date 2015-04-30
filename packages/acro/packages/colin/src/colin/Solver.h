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
 * \file Solver.h
 *
 * Defines the colin::Solver class.
 */

#ifndef colin_Solver_h
#define colin_Solver_h

#include <acro_config.h>
#include <ios>
#include <iomanip>
#include <colin/Problem.h>
#include <colin/solver/Base.h>
#include <colin/ApplicationMngr.h>

namespace colin
{

/**
 * A subclass of colin::Solver that defines the concrete data types
 * used to implement an optimizer, using a template specification.
 * This is a convenience class that enables the extension of the Solver
 * API to utilize these concrete types.
 */
template <class ProblemT>
class Solver : public Solver_Base
{
public:

   /**@name General Information */
   //@{
   /// Constructor
   Solver() 
   {}

   /// Virtual destructor
   virtual ~Solver() 
   {}

   ///
   Problem<ProblemT> get_problem() const
   { return problem; }

   ///
   virtual void set_problem(const ApplicationHandle handle)
   {
      try
      {
         ProblemMngr().lexical_cast(handle->get_problem(), problem);
      }
      catch (utilib::bad_lexical_cast& err)
      {
         std::cerr << "Solver::set_problem - Problem casting "
            "problem for use in a concrete solver object." << std::endl;
         throw;
      }
   }

   ///
   virtual ApplicationHandle get_problem_handle() const
   { return problem; }

protected:

   ///
   const EvaluationManager& get_problem_evaluation_manager()
   { return problem->eval_mngr(); }

   /// The problem that this solver optimizes
   Problem<ProblemT> problem;

};

} // namespace colin

#endif
