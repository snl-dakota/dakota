/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINApplication
//- Description: Specialized application class derived from COLIN's 
//-              application class which redefines virtual evaluation functions
//-              with DAKOTA's response computation procedures
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id: COLINApplication.hpp 6829 2010-06-18 23:10:23Z pdhough $

#include "DakotaModel.hpp"

#include <colin/Application.h>
#include <colin/AppRequest.h>
#include <colin/AppResponse.h>
#include <utilib/Any.h>


namespace Dakota {


/** COLINApplication is a DAKOTA class that is derived from COLIN's
    Application hierarchy.  It redefines a variety of virtual COLIN 
    functions to use the corresponding DAKOTA functions.  This is a
    more flexible algorithm library interfacing approach than can be
    obtained with the function pointer approaches used by
    NPSOLOptimizer and SNLLOptimizer. */

class COLINApplication : 
    virtual public colin::Application<colin::MO_MINLP2_problem>
    // TODO: consider specialization for MI vs. continuous problems
    // virtual public colin::Application<colin::MO_NLP2_problem>
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Default constructor. Rrequired by COLIN's ApplicationHandle creation.
  COLINApplication() 
  { /* empty constructor */ }

  /// Constructor with Model (not presently used).
  COLINApplication(Model& model);

  /// Destructor.
  ~COLINApplication()
  { /* empty destructor */ }

  /// Helper function called after default construction to extract problem
  /// information from the Model and set it for COLIN.
  void set_problem(Model& model);

  /// publishes whether or not COLIN is operating synchronously
  void set_blocking_synch(const bool blockingSynchFlag)
  {blockingSynch = blockingSynchFlag;}

  //
  //- Heading: colin::Application virtual member function redefinitions
  //

  /// Schedule one or more requests at specified domain point,
  /// returning a DAKOTA-specific evaluation tracking ID.
  virtual utilib::Any 
  spawn_evaluation_impl(const utilib::Any &domain,
		   const colin::AppRequest::request_map_t &requests,
		   utilib::seed_t &seed);

  /// Check to see if there are any function values ready to be collected.
  virtual bool
  evaluation_available();

  /// Perform a function evaluation at t given point.
  virtual void
  perform_evaluation_impl(const utilib::Any &domain,
                     const colin::AppRequest::request_map_t &requests,
                     utilib::seed_t &seed,
                     colin::AppResponse::response_map_t &colin_responses);

  /// Collect a completed evaluation from DAKOTA.
  virtual utilib::Any
  collect_evaluation_impl(colin::AppResponse::response_map_t &responses,
		     utilib::seed_t &seed);

  /// Helper function to convert evaluation request data from COLIN
  /// structures to DAKOTA structures.
  virtual void
  colin_request_to_dakota_request(const utilib::Any &domain,
				  const colin::AppRequest::request_map_t &requests,
				  utilib::seed_t &seed);

  /// Gelper function to convert evaluation response data from DAKOTA
  /// structures to COLIN structures.
  virtual void
  dakota_response_to_colin_response(const Response &dakota_response,
				    colin::AppResponse::response_map_t &colin_responses);

  /// Map the domain point into data type desired by this application context.
  virtual bool map_domain (const utilib::Any &src, utilib::Any &native, 
			   bool forward = true) const;

protected:

  /// Shallow copy of the model on which COLIN will iterate.
  Model iteratedModel;

  /// Flag for COLIN synchronous behavior (Pattern Search only).
  bool blockingSynch;

  /// Local copy of model's active set for convenience.
  ActiveSet activeSet;

  /// Evaluations queued for asynch evaluation.
  std::vector<int> requestedEvals;

  /// eval_id to response mapping to cache completed jobs.
  IntResponseMap dakota_responses;

}; // class COLINApplication

} // namespace Dakota
