/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINApplication
//- Description: Specialized application class derived from COLIN's 
//-              application class which redefines virtual evaluation functions
//-              with DAKOTA's response computation procedures
//- Owner:       Jean-Paul Watson/Bill Hart
//- Checked by:
//- Version: $Id

#include "COLINApplication.H"

//#define DEBUG_COLIN

namespace Dakota {

COLINApplication::
COLINApplication(Model& model) :
  iteratedModel(model), activeSet(model.current_response().active_set()),
  blockingSynch(false)
{
  // don't use the probDescDB so that this ctor may be used with both
  // the standard and on-the-fly COLINOptimizer ctors

  this->num_ineq_constr = model.num_nonlinear_ineq_constraints();
  this->num_eq_constr   = model.num_nonlinear_eq_constraints();
  numNonlinCons = this->num_ineq_constr + this->num_eq_constr;

  // for multiobjective, this will be taken from the RecastModel and will be
  // consistent with the COLIN iterator's view
  numObjFns = model.num_functions() - numNonlinCons;

  //num_real_params    = model.cv();
  //num_integer_params = model.div()+model.drv();
}


/** Converts the ColinPoint variables and request vector to DAKOTA variables
    and active set vector, performs a DAKOTA function evaluation with
    synchronization governed by synch_flag, and then copies the
    Response data to the ColinResponse response (synchronous) or
    bookkeeps the response object (asynchronous). */
void COLINApplication::
DoEval(ColinPoint& pt, int& priority, ColinResponse* prob_response,
       bool synch_flag)
{
#ifdef DEBUG_COLIN
cerr << "Entering COLINApplication::DoEval" << '\n' << std::flush;
#endif

 // Shouldn't we verify?
  //verify(prob_response->info->mode);

  //
  // Do something intelligent eventually
  if (response_exists(pt,*prob_response)) {
  }

  //
  // Map variables
  //
  colin::map_domain(iteratedModel,pt); // pt -> active contin/discrete vars

  //
  // Assign COLIN obj fn request to all DAKOTA obj fns:
  //
  size_t i;
  std::vector<int>& colin_request_vector = prob_response->request_vector();
  ShortArray asv(numObjFns + numNonlinCons);
  for(i=0; i<numObjFns; i++)
    asv[i] = colin_request_vector[0];
  for(i=0; i<numNonlinCons; i++)
    asv[i+numObjFns] = colin_request_vector[i+1];
  activeSet.request_vector(asv);

  //
  // Compute response
  //
  //Cout << "DoEval synch_flag = " << synch_flag << " dakotaModelAsynchFlag = "
  //     << dakotaModelAsynchFlag << std::endl;

  if (!synch_flag && dakotaModelAsynchFlag) {
    // Asynch is requested by COLIN and it is allowed by Model
    iteratedModel.asynch_compute_response(activeSet);
    prob_response->info->id = iteratedModel.evaluation_id();
    this->response_list.push_back(prob_response);
#ifdef DEBUG_COLIN
    Cout << "ADDING COLIN ID: " << prob_response->info->id << " "
	 << prob_response << std::endl;
#endif
    if (!synch_flag)
      this->async_request_ids.push_back(prob_response->info->id);
#ifdef DEBUG_COLIN
    {
      Cout << "COLIN List: ";
      std::list<ColinResponse*>::iterator curr = this->response_list.begin();
      std::list<ColinResponse*>::iterator end  = this->response_list.end();
      while (curr != end) {
        Cout << (*curr)->info->id << " ";
	curr++;
      }
      Cout << std::endl;
    }
#endif
  }
  else {
    // When asynch is requested by COLIN but not allowed by Model, COLIN
    // will still call synchronize and the if (!response_list) check will cause
    // an immediate return from the function.  Thus, it is assumed that COLIN
    // allocates space for each prob_response such that the copy above is 
    // sufficient (i.e., a response_list.add(prob_response) is not needed).
    iteratedModel.compute_response(activeSet);
    map_response(*prob_response, iteratedModel.current_response());
    prob_response->init();
    update_response(pt,*prob_response);
    if (!synch_flag)
      this->async_completed_ids.push_back(prob_response->info->id);
  }
}


/** Blocking synchronize of asynchronous DAKOTA jobs followed by
    conversion of the Response objects to ColinResponse response objects. */
void COLINApplication::synchronize()
{
#ifdef DEBUG_COLIN
cerr << "Entering COLINApplication::synchronize" << '\n' << std::flush;
#endif

  //
  // We've queued up async evaluations, which need to be
  // processed explicitly, or else COLIN solvers will get confused
  //
  //while (this->async_ids.size() > 0) {
  //  COLINApplication::next_eval();
  //}

  if (this->response_list.size() == 0) // return if no pending jobs
    return;

  // Blocking synchronization
  const IntResponseMap& dakota_resp_map = iteratedModel.synchronize();
  size_t num_resp = dakota_resp_map.size();
  if (num_resp != this->response_list.size()) {
    Cout << "Error: there are " << this->response_list.size() << " queued "
	 << "evaluation but only " << num_resp << " were returned." << std::endl;
    abort_handler(-1);
  }

  // since DAKOTA's jobs are returned in the order of asynch calls, can do a 
  // simple item to item copy.
  IntRespMCIter r_it = dakota_resp_map.begin();
  std::list<ColinResponse*>::iterator curr = this->response_list.begin();
  std::list<ColinResponse*>::iterator end  = this->response_list.end();
  while (curr != end) {
    map_response(*(*curr), r_it->second);
    (*curr)->init();
    // pointer now populated, remove from pending list
    //delete *curr;		// BUG??? DO THIS?
    ++r_it;
    ++curr;
  }
  this->response_list.clear();

  colin::OptApplicationBase::synchronize();
}
 

/** Nonblocking job retrieval. Finds a completion (if available),
    populates the COLIN response, and sets id to the completed job's
    id.  Else set id = -1. */
int COLINApplication::next_eval()
{
#ifdef DEBUG_COLIN
  Cerr << "Entering COLINApplication::next_eval" << '\n' << std::flush;
#endif

#ifdef DEBUG_COLIN
  {
    std::list<int>::iterator curr = this->async_completed_ids.begin();
    std::list<int>::iterator end  = this->async_completed_ids.end();
    Cout << "ASYNC IDS:";
    while (curr != end) {
      Cout << " " << *curr;
      curr++;
    }
    Cout << std::endl;
  }
  {
    std::list<int>::iterator curr = async_request_ids.begin();
    std::list<int>::iterator end  = async_request_ids.end();
    Cout << "REQUEST IDS:";
    while (curr != end) {
      Cout << " " << *curr;
      curr++;
    }
    Cout << std::endl;
  }
  {
    std::list<ColinResponse*>::iterator curr = this->response_list.begin();
    std::list<ColinResponse*>::iterator end  = this->response_list.end();
    Cout << "Response List IDs:";
    while (curr != end) {
      Cout << " " << (*curr)->info->id;
      curr++;
    }
    Cout << std::endl;
  }
#endif

  // COLIN only wants one completion, so buffer multiple DAKOTA completions in
  // dakotaResponseMap and only call DAKOTA synchronize functions when the 
  // list has been exhausted.  Each call to synchronize_nowait returns a fresh
  // set of jobs (i.e., returned completions are removed from DAKOTA's lists).
  if (dakotaResponseMap.empty() && (this->response_list.size() > 0)) {
    // APPS will call next_eval() even for blocking synchronization
    if (blockingSynch) {
      dakotaResponseMap = iteratedModel.synchronize();
      size_t i, num_jobs = dakotaResponseMap.size();
      for (i=0; i<num_jobs; i++) {
        int tmp_id = -1;
        if (this->async_request_ids.size() > 0) {
	  tmp_id = this->async_request_ids.front();
	  this->async_request_ids.pop_front();
	  this->async_completed_ids.push_back(tmp_id);
	}
        if (tmp_id == -1) {
	  Cerr << "Error: DAKOTA has a response, but no corresponding"
	       << " asynchronous evaluation was performed by COLIN!" << std::endl;
	  abort_handler(-1);
	}
      }
    }
    else {// nonblocking
      dakotaResponseMap = iteratedModel.synchronize_nowait();
      //Cerr << "HERE " << dakotaResponseMap.size() << " "
      //     << dakotaResponseMap.empty() << std::endl;
      //
      // Remove the id's from async_ids and dead_ids that have been returned
      //
      int ctr=0;
      {
	std::list<int>::iterator curr = this->async_request_ids.begin();
	std::list<int>::iterator end  = this->async_request_ids.end();
	while (curr != end) {
	  if (dakotaResponseMap.find(*curr) != dakotaResponseMap.end()) {
	    //Cerr << "HERE X:" << (*curr) << std::endl;
#ifdef DEBUG_COLIN
	    Cout << "Y " << *curr << std::endl;
#endif
	    this->async_completed_ids.push_back(*curr);
	    curr = this->async_request_ids.erase(curr);
	    ctr++;
	  }
	  else
	    curr++;
	}
      }
      {
	std::set<int>::iterator curr = this->dead_ids.begin();
	std::set<int>::iterator end  = this->dead_ids.end();
	while (curr != end) {
	  IntRespMIter dak_curr = dakotaResponseMap.find(*curr);
	  if (dak_curr != dakotaResponseMap.end()) {
	    //Cerr << "HERE Y:" << (*curr) << std::endl;
	    dakotaResponseMap.erase(dak_curr);
	    dead_ids.erase(curr++);
	  }
	  else
	    curr++;
	}
      }
#if 0
      //
      // This is not an error.  When Dakota runs two COLIN optimizers
      // in sequence, it does not guarantee that it clears the queue
      // before the second optimizer is run.  Thus, this condition may
      // arise.
      //
      if (ctr != dakotaResponseMap.size()) {
	Cerr << "Error: synchronize_nowait() returned " << 
	  dakotaResponseMap.size() << " evaluations, but only " <<
	  ctr << " had valid COLIN ids." << std::endl;
	abort_handler(-1);
      }
#endif
    }
  }

  // TO DO: manage terminated evaluations (new COLIN feature).  One simple way
  // to do this (prior to DAKOTA support of job termination) would be to get
  // any terminated eval ids from COLIN and check them here against
  // dakotaCompletionList.  If present, these evals could be deleted from the 
  // dakota lists.  This would not improve efficiency any (since the 
  // evaluations are still performed by DAKOTA), but it would quiet COLIN 
  // warnings for returning evaluations that it has terminated.

  if (!dakotaResponseMap.empty()) { // synchronize_nowait may return none

    IntRespMCIter map_iter = dakotaResponseMap.begin();
    int dak_id = map_iter->first;

    // find corresponding id in COLIN's response_list and copy DAKOTA's
    // response to COLIN's
    bool found = false;
    {
    std::list<ColinResponse*>::iterator curr = this->response_list.begin();
    std::list<ColinResponse*>::iterator end  = this->response_list.end();
    while (curr != end) {
      //Cout << "COLIN id = " << (*curr)->info->id
      //     << " DAKOTA completion list id = " << dak_id << std::endl;
      if ((*curr)->info->id == dak_id) {
        map_response(*(*curr), map_iter->second);
        (*curr)->init();
        // pointer now populated, remove from pending list
        this->response_list.erase(curr);
        //delete *curr;		// BUG???  DO THIS????
        found = true;
	dakotaResponseMap.erase(dak_id);
        break;
      }
      curr++;
    }
    }
    if (!found) {
      dakotaResponseMap.clear();
      return -1;
#if 0
      //
      // This is not an error.  See note above.
      //
      Cerr << "Error: no matching COLIN id to DAKOTA's evaluation id in "
	   << "COLINApplication::next_eval." << std::endl;
      Cerr << "COLIN List: ";
      std::list<ColinResponse*>::iterator curr = this->response_list.begin();
      std::list<ColinResponse*>::iterator end  = this->response_list.end();
      while (curr != end) {
        Cerr << (*curr)->info->id << " ";
	curr++;
        }
      Cerr << "DAKOTA ID: " << dak_id << std::endl;
      abort_handler(-1);
#endif
    }

    std::list<int>::iterator curr = async_completed_ids.begin();
    std::list<int>::iterator end  = async_completed_ids.end();
    while (curr != end) {
      //Cout << "HERE " << *curr << std::endl;
      if (*curr == dak_id)
	 break;
      curr++;
      }
    if (curr != end)
       async_completed_ids.erase(curr);

    return dak_id;
  }
  else
    return OptApplicationBase::next_eval();
}


/** map_response
 * Maps a Response object into a ColinResponse class that is
 * compatable with COLIN.
 */
void COLINApplication::
map_response(ColinResponse& colin_response, const Response& dakota_response)
{
  size_t dakota_num_funs = dakota_response.num_functions();
  const ShortArray& dakota_asv   = dakota_response.active_set_request_vector();
  const RealVector& dakota_fns   = dakota_response.function_values();
  const RealMatrix& dakota_grads = dakota_response.function_gradients();
  const RealSymMatrixArray& dakota_hessians = dakota_response.function_hessians();

  // colin response vector is separate from COLIN request vector
  std::vector<int>& colin_asv = colin_response.response_vector();

  for (size_t i=0; i<dakota_num_funs; i++) {

    // update colin response vector with actual results to be returned
    colin_asv[i] = dakota_asv[i];

    if (dakota_asv[i] & 1) {
      if (i)
	colin_response.constraint_values()[i-1] = dakota_fns[i];
      else {
	colin_response.function_value(i) = dakota_fns[i];
	this->neval_ctr++;
      }
    }
#if 0
    if (dakota_asv[i] & 2) {
      // TO DO: CONVERT OVER THE GRADIENT (ONCE DERIVATIVE-BASED ALGS ARE USED)
      //colin_response.function_gradient();
      //colin_response.constraint_gradients()
    }
    if (dakota_asv[i] & 4) {
      // TO DO: CONVERT OVER THE HESSIAN (ONCE DERIVATIVE-BASED ALGS ARE USED)
      //colin_response.function_hessian();
      //colin_response.constraint_hessians()
    }
#endif
  }
}

} // namespace Dakota
