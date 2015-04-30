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
 * \file SimpleMILocalSearch.h
 *
 * Defines the colin::SimpleMILocalSearch class.
 */

#ifndef colin_SimpleMILocalSearch_h
#define colin_SimpleMILocalSearch_h

#include <acro_config.h>

#include <colin/solver/ColinSolver.h>

#include <utilib/MixedIntVars.h>


namespace colin
{

/** A simple local optimizer for mixed-integer domains. */
class SimpleMILocalSearch
   : public colin::ColinSolver<utilib::MixedIntVars, UMINLP0_problem >
{
public:

   ///
   SimpleMILocalSearch();

   ///
   virtual void optimize();

protected:
   virtual std::string define_solver_type() const;

   ///
   void initialize_best_point();

   ///
   bool check_convergence()
   {
      if ( colin::ColinSolver<utilib::MixedIntVars, UMINLP0_problem >
           ::check_convergence() )
         return true;

      if (step_length <= fini_step_length)
      {
         std::stringstream tmp;
         tmp << "Step-Length Delta=" << step_length << "<="
         << fini_step_length << "=Delta_thresh";
         this->solver_status.termination_info = tmp.str();
         return true;
      }
      return false;
   }

   ///
   double init_step_length;

   ///
   double step_length;

   ///
   double fini_step_length;

   ///
   std::vector<int> ilower;

   ///
   std::vector<int> iupper;

   ///
   std::vector<real> rlower;

   ///
   std::vector<real> rupper;

   ///
   bool bc_flag;

private:
   ///
   void reset_SimpleMILocalSearch();
};

} // namespace colin

#endif
