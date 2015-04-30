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
 * \file APPS.h
 *
 * Defines the interfaces::APPS class.
 */

#ifndef interfaces_APPS_h
#define interfaces_APPS_h

#include <acro_config.h>
#ifdef ACRO_USING_APPSPACK

namespace APPSPACK {
namespace Parameter {
class List;
}
}
 
//#include <colin/AppResponseAnalysis.h>
#include <colin/CommonOptions.h>
#include <colin/Solver.h>
#include <utilib/std_headers.h>


namespace interfaces {

class APPS;

//! Namespace for the Feval Manager used by AppsOptimizer
namespace ColinyAPPS {

class FevalMgr;

} // namespace ColinyAPPS




/** COLIN wrapper class for asynchronous parallel pattern search class.

TODO: describe APPS and cite papers from Kolda, Hough and Torczon.
*/

class APPS : virtual public colin::Solver<colin::UNLP0_problem>, public colin::CommonOptions
{

  friend class ColinyAPPS::FevalMgr;

public:

  /// Constructor
  APPS();

  /// Destructor
  virtual ~APPS();

  /// Perform optimization
  void optimize();

protected:

  ///
  std::string define_solver_type() const
    { return "APPS"; }

  ///
  bool auto_rescale;

  ///
  double step_tolerance;

  ///
  double alpha;

  ///
  std::string pattern;

  /// 
  int num_search_directions;

  /// 
  int num_active_directions;

  ///
  int profile;

  ///
  bool inc_flag;

  ///
  double contraction_factor;

  ///
  double initial_step;

  ///
  double min_step_allowed;

  ///
  double max_step_allowed;

  ///
  int arg_count;

  ///
  std::vector<std::string> args;

  ///
  char** args_;

  ///
  ColinyAPPS::FevalMgr* feval_mgr;

  ///
  APPSPACK::Parameter::List* params;

  ///
  bool synch_flag;

  ///
  std::string batch_str;

  /// 
  void reset_APPS();

};

} // namespace interfaces 
#endif

#endif
