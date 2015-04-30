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
 * \file GradientDescent.h
 *
 * Defines the colin::GradientDescent class.
 */

#ifndef colin_GradientDescent_h
#define colin_GradientDescent_h

#include <acro_config.h>
#include <utilib/BasicArray.h>
#include <colin/solver/ColinSolver.h>

namespace colin
{

/** A simple derivative-based local search method. */
class GradientDescent
   : public colin::ColinSolver<utilib::BasicArray<double>, UNLP1_problem >
{
public:

   ///
   GradientDescent();

   ///
   void optimize();

protected:

   ///
   std::string define_solver_type() const;

   ///
   void initialize_best_point()
   {
      if ( (initial_point.size() > 0) && 
           ( problem->num_real_vars != initial_point.size() )) 
         EXCEPTION_MNGR(std::runtime_error, 
                        "GradientDescent::initialize_best_point(): "
                        "user-provided best point has " << 
                        initial_point.size() << 
                        " real variables, but the problem has " << 
                        problem->num_real_vars << std::endl );

      initial_point.resize(problem->num_real_vars);
   }

   ///
   bool check_convergence();

   ///
   double init_step_length;

   ///
   utilib::BasicArray<real> grad;

   ///
   std::vector<real> rlower;

   ///
   std::vector<real> rupper;

   ///
   bool bc_flag;

private:

   ///
   void reset_GradientDescent();

};

}

#endif
