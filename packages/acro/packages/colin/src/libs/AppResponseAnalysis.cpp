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

#include <acro_config.h>
#include <colin/AppResponseAnalysis.h>

namespace colin
{


AppResponseAnalysis::AppResponseAnalysis()
      : constraint_penalty(1000.0),
      ignore_convergence_factor(false),
      _numNonlinearIneqConstraints(0),
      _constraint_tolerance(0.0)
{
   create_parameter("constraint_penalty", constraint_penalty,
                    "<double>", "1000.0",
                    "If nonzero, then add the penalty times the sum-squared\
                    \n\tconstraint violation to the objective");

   create_parameter("constant_constraint_penalty", ignore_convergence_factor,
                    "<bool>", "false",
                    "If true, then the constraint penalty function calculation ignores\
                    \n\tthe convergence factor, so the constraint penalty is constant.");
}

} // namespace colin
