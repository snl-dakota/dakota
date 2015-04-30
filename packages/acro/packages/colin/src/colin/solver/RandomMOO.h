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
 * \file RandomMOO.h
 *
 * Defines the colin::RandomMOO class.
 */

#ifndef colin_RandomMOO_h
#define colin_RandomMOO_h

#include <acro_config.h>

#include <colin/solver/ColinSolver.h>
#include <colin/ParetoSet.h>

#include <utilib/Normal.h>
#include <utilib/BasicArray.h>

#include <list>

namespace colin
{

/** A simple randomized multi-objective search method. */
class RandomMOO 
   : public colin::ColinSolver<utilib::BasicArray<double>, MO_UNLP0_problem>
{
public:

   ///
   RandomMOO();

   ///
   virtual void optimize();

protected:
   virtual std::string define_solver_type() const;

   ///
   bool check_convergence() {return false;}

   ///
   utilib::Normal nrnd;

   ///
   utilib::Uniform urnd;

private:
   ///
   void reset_RandomMOO();


};

}

#endif
