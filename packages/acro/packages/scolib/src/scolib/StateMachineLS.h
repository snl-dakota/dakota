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
 * \file StateMachineLS.h
 *
 * Defines the scolib::StateMachineLS class
 */

#ifndef scolib_StateMachineLS_h
#define scolib_StateMachineLS_h

#include <acro_config.h>
#include <colin/Solver.h>

namespace scolib {

class StateMachineLS : public colin::Solver<colin::UINLP_problem>
{
public:
   /// Constructor
   StateMachineLS();

   ///
   void reset();

   ///
   void optimize();

   /// The file to read for the list of valid state transitions
   utilib::Property state_definition_file;

   /// Maximum number of main algorithm iterations
   utilib::Property max_iterations;

   /// Maximum number of function evaluations
   utilib::Property max_fcn_evaluations;

   /// Maximum solver run time
   utilib::Property max_time;

   /// Verbosity level
   utilib::Property verbosity;


   struct Data;

private:
   Data *data;

   void load_states();


protected:
   std::string define_solver_type() const
   { return "StateMachineLS"; }
};


} // namespace scolib

#endif // defined scolib_StateMachineLS_h
