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
 * \file PIDOMS.h
 *
 * Parallel Iterative Decomposition of Multistart Search.
 *
 * Defines the scolib::PIDOMS class.
 */

#ifndef scolib_PIDOMS_h
#define scolib_PIDOMS_h

#include <acro_config.h>
#include <pebbl/parBranching.h>
#include <colin/Cache.h>
#include <colin/solver/ColinSolver.h>
#include <colin/OptResponse.h>
#include <colin/reformulation/RelaxableMixedIntDomain.h>
#include <utilib/MixedIntVars.h>
#include <utilib/Ereal.h>
#include <utilib/ParameterSet.h>
#include <utilib/BasicArray.h>

namespace scolib {

using utilib::Ereal;
using colin::real;

namespace pidoms {

template <class problem_t>
class PIDOMSSolver;

template <class problem_t>
class PIDOMSHandler;

}

/** An interface to the PIDOMS Global Optimizer
  */
class PIDOMS : public colin::ColinSolver<utilib::MixedIntVars, colin::UMINLP0_problem>
{
public:

  typedef colin::ColinSolver<utilib::MixedIntVars, colin::UMINLP0_problem> solver_t;
  typedef colin::UMINLP0_problem problem_t;

  /// Constructor
  PIDOMS();

  ///
  virtual ~PIDOMS();

  ///
  void reset_PIDOMS();

  ///
  void optimize();

  ///
  void set_handler(pidoms::PIDOMSHandler<problem_t>* app_handler_);

protected:

   std::string define_solver_type() const
   { return "PIDOMS"; }

  ///
  double step_tolerance;

  ///
  double initial_step;

  ///
  utilib::MixedIntVars x;

  ///
  pidoms::PIDOMSSolver<problem_t>* solver;

  ///
  pidoms::PIDOMSHandler<problem_t>* app_handler_;

  ///
  double Lipshitz;
};




namespace pidoms {

///
/// A class that defines an PIDOMS problem instance
///
template <class ProblemT>
class PIDOMSHandler
{
public:

  ///
  PIDOMSHandler() 
    : solver(0), Lipshitz_constant(0.0), intvars(0) {}

  ///
  void set_problem(colin::Problem<ProblemT>& prob)
    {
    problem = prob;
    colin::RelaxableMixedIntDomainApplication<ProblemT>* app
       = dynamic_cast<colin::RelaxableMixedIntDomainApplication<ProblemT>*>
       (prob.application());
    if (!app) {
       EXCEPTION_MNGR(std::runtime_error, "Problem with dynamic cast.");
    }
/*
    utilib::TypeManager()->lexical_cast(app->relaxed_problem(), relaxed_problem);
*/
    // JDS - there _must_ be a simpler way!
    intvars = utilib::anyval_cast<int>(problem->num_int_vars.get()) + 
                + utilib::anyval_cast<int>(problem->num_binary_vars.get());
    }

  ///
  Ereal<double> operator()(utilib::MixedIntVars& x)
    {
    colin::real val;
    problem->EvalF(solver->eval_mngr(),x,val);
    return val;
    }

  ///
  Ereal<double> operator()(utilib::BasicArray<double>& x)
    {
    colin::real val;
    relaxed_problem->EvalF(solver->eval_mngr(),x,val);
    return val;
    }

  ///
  void apply_subsolver(utilib::BasicArray<double>& x, int sub_solver=0);

  ///
  int num_integer_variables()
        {return intvars;}

  ///
  colin::Problem<ProblemT> problem;

  ///
  colin::Problem<colin::NLP0_problem> relaxed_problem;

  ///
  utilib::BasicArray<real> lower;

  ///
  utilib::BasicArray<real> upper; 

  ///
  PIDOMS* solver;

  ///
  double Lipshitz_constant;

  ///
  int intvars;

  ///
  //Cache cache;
};



/**
 * Defines the branching class for a serial PIDOMS optimizer
 */
template <class HandleT>
class serialPIDOMS : virtual public pebbl::branching 
{
public:

  /// An empty constructor
  serialPIDOMS()
    : app_handler(0)
    { branchingInit(); }
                
  /// A destructor
  virtual ~serialPIDOMS()
    { }

  ///
  bool setup(int& argc, char**& argv, HandleT& app_handler_)
    {
    app_handler = &app_handler_;
    return pebbl::branching::setup(argc,argv);
    }

  ///
  bool setup(int& argc, char**& argv)
    { return pebbl::branching::setup(argc,argv); }

  ///
  void reset(bool resetVB = true)
    { pebbl::branching::reset(resetVB); }

  /// Return a new subproblem
  pebbl::branchSub* blankSub();

  ///
  //void printAllStatistics(std::ostream& stream = std::cout)
    //{
    //pebbl::branching::printAllStatistics();
    //}

  /// The function object for this application
  HandleT* app_handler;

  ///
  enum branch_type {branch_up, branch_down, no_branch};

};


/**
 * Defines the nodes used for a serial PIDOMS optimizer
 */
template <class HandleT>
class serialPIDOMSNode : virtual public pebbl::branchSub
{
public:

  /// The type of the branching class
  typedef serialPIDOMS<HandleT> branching_t;

  /// An empty constructor for a subproblem
  serialPIDOMSNode()
    : bestpt_value(Ereal<double>::negative_infinity),
      corner_dist(0.0),
      branch_status(branching_t::no_branch),
      branchVariable(-2)
    { }

  /// A virtual destructor for a subproblem
  virtual ~serialPIDOMSNode()
    { }

  /// Return a pointer to the global branching object
  branching_t* global() const
    { return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::branching* bGlobal() const
    { return global(); }

  /**
   * Link the debugging in the subproblem to the debugging level
   * set within the global branching object.
   */
  REFER_DEBUG(global())

  /**
   * Initialize a subproblem using a branching object
   */
  void initialize(branching_t* master)
    {
    globalPtr = master;
    lower << master->app_handler->lower;
    upper << master->app_handler->upper;
    best_point.resize(lower.size());
    midpoint.resize(lower.size());
    }

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   */
  void initialize(serialPIDOMSNode<HandleT>* parent,int whichChild)
    {
    globalPtr = parent->globalPtr;
    branchSubAsChildOf(parent);
    //
    // If we are asked for the 2nd child, switch the branch
    // status to be the "opposite" of the last branch (in the parent!)
    //
    if (whichChild > 0) {
       if (parent->branch_status == branching_t::branch_down)
          parent->branch_status = branching_t::branch_up;
       else if (parent->branch_status == branching_t::branch_up)
          parent->branch_status= branching_t::branch_down;
       else 
          EXCEPTION_MNGR(std::runtime_error, "Trying to create a child without a proper branching status: whichChild = " << whichChild);
       }
    //
    // Initialize this node's info
    //
    lower = parent->lower;
    upper = parent->upper;
    best_point = parent->best_point;
    midpoint = parent->midpoint;
    //
    // If the branch_status is no_branch, then this is the first
    // branch for a subproblem and the user has not defined a default
    // branching policy.  We are simply making this down_branch for now.
    //
    //
    // Compute the branch
    //
    if (parent->branch_status == branching_t::branch_down)
       upper[parent->branchVariable] = midpoint[parent->branchVariable];
    else
       lower[parent->branchVariable] = midpoint[parent->branchVariable];
    }

  /// Initialize this subproblem to be the root of the branching tree
  virtual void setRootComputation()
    { branchVariable = -1; }

  /**
   * Compute the lower bound on this subproblem's value.
   * The value of controlParam is not currently defined.
   *
   * When this routine is finished the state of the subproblem should be
   * set to bounded OR dead.
   */
  void boundComputation(double* )
    {
    double max_range = 0.0;
    for (unsigned int i=0; i<lower.size(); i++) {
      midpoint[i] = (upper[i] + lower[i])/2.0;
      if ((upper[i] - lower[i]) > max_range)
         max_range = upper[i] - lower[i];
      }
    //
    // Compute the distance to the corner using a numerically stable 
    // computation
    //
    corner_dist=0.0;
    for (unsigned int i=0; i<lower.size(); i++) {
      double tmp = (upper[i] - lower[i])/(max_range);
      corner_dist += tmp*tmp;
      }
    corner_dist = max_range*std::sqrt(corner_dist);
    bestpt_value = (*(global()->app_handler))(midpoint);
    // Create subproblem with new bounds
    //     Create new problem instance using revised bounds (John?)
    // then...
    //     app_handler->apply_subsolver(midpoint);
    // then ... look at the cache...
    bound = bestpt_value - global()->app_handler->Lipshitz_constant * corner_dist;
    //
    // Perform local search in this subregion
    //
    //colin::AppResponse response;
    //this->call_local_search(upper,lower,midpoint,response);
    #if 1
    DEBUGPR(300,
                ucout << "boundComputation - bestpt_val = " 
                      << bestpt_value << std::endl;
                ucout << "boundComputation - best_point = " 
                      << best_point << std::endl;
                ucout << "boundComputation - lower = " << lower << std::endl;
                ucout << "boundComputation - upper = " << upper << std::endl;
                ucout << "boundComputation - dist     = " 
                      << corner_dist << std::endl;
                ucout << "boundComputation - L        = " 
                      << global()->app_handler->Lipshitz_constant << std::endl;
                ucout << "boundComputation - bound    = " << bound << std::endl
                );
    #endif
    setState(bounded);
    }

  /**
   * Determine how many children will be generated and how they will be
   * generated (e.g. the branching variable).
   * The return value is the number of children that can be generated from
   * this node.
   */
  virtual int splitComputation()
    {
    branchVariable = 0;
    double range = upper[0] - lower[0];
    for (unsigned int i=1; i<lower.size(); i++) {
      double tmp = upper[i] - lower[i];
      if (tmp > range) {
         range = tmp;
         branchVariable = i;
         }
      }
    DEBUGPR(300,
                ucout << "boundComputation - bestpt_val = " 
                      << bestpt_value << std::endl;
                ucout << "boundComputation - best_point = " 
                      << best_point << std::endl;
                ucout << "splitComputation - lower = " << lower << std::endl;
                ucout << "splitComputation - upper = " << upper << std::endl;
                ucout << "splitComputation - branchVar = " 
                      << branchVariable << std::endl;
                );
    branch_status = branching_t::branch_down;
    setState(separated);
    return 2;
    }

  /// Create a child subproblem of the current subproblem
  virtual branchSub* makeChild(int whichChild)
    {
    serialPIDOMSNode<HandleT> *temp = new serialPIDOMSNode<HandleT>;
    temp->initialize(this, whichChild);
    return temp;
    }
  
  /// Returns true if this subproblem represents a feasible solution
  bool candidateSolution()
    { return true; }

  /**
   * Gets a solution from a subproblem
   */
  virtual pebbl::solution* extractSolution()
    {
    if (!globalPtr->canFathom(bestpt_value))
        {
        DEBUGPR(300, ucout << "extractSolution - value = " 
                                        << bestpt_value << std::endl);
        return new pebbl::arraySolution<double>(bestpt_value,best_point,globalPtr);
        }
    return NULL;
    }

protected:

  /// A pointer to the global branching object
  branching_t* globalPtr;

  /// The best_point of the node
  utilib::BasicArray<double> best_point;

  /// The midpoint of this node
  utilib::BasicArray<double> midpoint;

  /// Lower bounds on the search domain
  utilib::BasicArray<double> lower;

  /// Upper bounds on the search domain
  utilib::BasicArray<double> upper;

  /// Value of the best_point of the current node
  Ereal<double> bestpt_value;

  /// Distance from the best_point to the farthest corner of the box
  double corner_dist;

  /// Flag that indicates which branch to take next to create a subproblem
  typename branching_t::branch_type branch_status;

  /**
   * The variable that will be branched next.
   * If set to -1, then this is the root computation.
   */
  int branchVariable;
};


template <class HandleT>
pebbl::branchSub* serialPIDOMS<HandleT>::blankSub()
{
serialPIDOMSNode<HandleT> *temp = new serialPIDOMSNode<HandleT>();
temp->initialize(this);
return temp;
}


#if defined(ACRO_HAVE_MPI)
/**
 * Defines the branching class for a parallel PIDOMS optimizer
 */
template <class HandleT>
class parallelPIDOMS : public pebbl::parallelBranching, public serialPIDOMS<HandleT>
{
public:

  #if !defined(SOLARIS)
  using serialPIDOMS<HandleT>::app_handler;
  #endif

  /// An empty constructor for a branching object
  parallelPIDOMS()
    : buffer_size(0)
    { }

  /// A destructor
  virtual ~parallelPIDOMS()
    { }

  /// Note: should this include the VB flag?
  void reset(bool VBflag=true)
    {
      serialPIDOMS<HandleT>::reset(VBflag);
      size_type numVars = app_handler->lower.size();
      registerFirstSolution(new pebbl::arraySolution<double>(numVars,this));
      pebbl::parallelBranching::reset(false);
     }

  /// Return a new subproblem
  pebbl::parallelBranchSub* blankParallelSub();

  /// Pack the branching information into a buffer
  void pack(utilib::PackBuffer& outBuffer)
    {
    #if 0
    outBuffer << Lipshitz_constant;
    unsigned int len = app_handler->lower.size();
    outBuffer << len;
    for (unsigned int i=0; i<len; i++)
      outBuffer << app_handler->lower[i] << app_handler->upper[i];
    double flag = -999.0;
    outBuffer << flag;
    #endif
    }

  /// Unpack the branching information from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
    {
    #if 0
    inBuffer >> Lipshitz_constant;
    unsigned int len;
    inBuffer >> len;
    app_handler->lower.resize(len);
    app_handler->upper.resize(len);
    for (unsigned int i=0; i<len; i++)
      inBuffer >> app_handler->lower[i] >> app_handler->upper[i];
    double flag;
    inBuffer >> flag;
    if (flag != -999.0)
       EXCEPTION_MNGR(std::runtime_error,"parallelLipshitzian - problem unpacking");
    #endif
    }

  /// Compute the size of the buffer needed for a subproblem
  int spPackSize()
    {
    if (buffer_size == 0) {
       utilib::PackBuffer buffer;
       pebbl::parallelBranchSub* tmp = blankParallelSub();
       tmp->pack(buffer);
       buffer_size = 1000*buffer.size();
       delete tmp;
       //ucout << "parallelLipshitzian::spPackSize - size initialized to " << buffer_size << endl;
       }
    return buffer_size;
    }

  ///
  bool setup(int& argc,char**& argv, HandleT& app_handler_)
    {
      app_handler = &app_handler_;
      return pebbl::parallelBranching::setup(argc,argv);
    }

  ///
  bool setup(int& argc,char**& argv)
    { return pebbl::parallelBranching::setup(argc,argv); }

  ///
  void printSolution(const char* header = "",
             const char* footer = "",
             std::ostream& outStream = ucout)
    {
      pebbl::parallelBranching::printSolution(header,footer,outStream);
    }

protected:

  /// Buffer size
  int buffer_size;
};
#endif


///
/// An abstract class that defines the particular solver
///
template <class ProblemT>
class PIDOMSSolver
{
public:

  /// Destructor
  virtual ~PIDOMSSolver() {}

  /// Get a ParameterSet object, which can be used to set parameters
  virtual utilib::ParameterSet& parameters() = 0;

  /// Reset the solver's state
  virtual void reset() = 0;

  /// Reset the solver's state
  virtual pebbl::branching* base() = 0;

  /// Perform minimization
  virtual void minimize(colin::OptResponse& response) = 0;

  /// Returns true if this is a serial solver
  virtual bool isSerial() = 0;

  /// Set the function used in the search
  virtual void set_handler(PIDOMSHandler<ProblemT>* app_handler_) = 0;
};


///
/// PIDOMS serial solver class
///
template <class ProblemT>
class PIDOMSSerialSolver : public PIDOMSSolver<ProblemT>
{
public:

  /// The serial solver
  pidoms::serialPIDOMS<PIDOMSHandler<ProblemT> > solver;

  ///
  utilib::ParameterSet& parameters()
    { return solver; }

  ///
  void reset()
    {
    int argc=0;
    char** argv=0;
    solver.setup(argc,argv);
    solver.reset();
    }

  /// 
  pebbl::branching* base()
    { return &solver; }

  ///
  void minimize(colin::OptResponse& response)
    {
    solver.solve();
    pebbl::arraySolution<double>* soln = static_cast<pebbl::arraySolution<double>*>(solver.incumbent);
    response.point = soln->array;
    response.value() = soln->value;
    if (solver.abortReason)
       response.termination_info = "Error";
    else
       response.termination_info = "Successful";
    }

  ///
  bool isSerial()
    { return true; }

  ///
  void set_handler(PIDOMSHandler<ProblemT>* app_handler)
    { solver.app_handler = app_handler; }
};


///
/// PIDOMS parallel solver class
///
#if defined(ACRO_HAVE_MPI)
template <class ProblemT>
class PIDOMSParallelSolver : public PIDOMSSolver<ProblemT>
{
public:

  /// The serial solver
  pidoms::parallelPIDOMS<PIDOMSHandler<ProblemT> > solver;

  ///
  utilib::ParameterSet& parameters()
    { return solver; }

  ///
  void reset()
    {
    int argc=0;
    char** argv=0;
    solver.setup(argc,argv);
    solver.reset();
    }

  /// 
  pebbl::branching* base()
    { return &solver; }

  ///
  void minimize(colin::OptResponse& response)
    {
    solver.solve();
    pebbl::arraySolution<double>* soln = static_cast<pebbl::arraySolution<double>*>(solver.incumbent);
    response.point = soln->array;
    //response.value = soln->value;
    if (solver.abortReason)
       response.termination_info = "Error";
    else
       response.termination_info = "Successful";
    }

  ///
  bool isSerial()
    { return false; }

  ///
  void set_handler(PIDOMSHandler<ProblemT>* app_handler_)
    { solver.app_handler = app_handler_; }
#if 0
    {
     /// WEH - this isn't portable, so I'm commenting it out for now.
     static_cast<pebbl::serialLipshitzian<PIDOMSHandler> >(solver).func 
            = func;
    }
#endif
};


/**
 * Defines the core methods needed to extend the PEBBL branchSub class.
 */
template <class ProblemT>
class parallelPIDOMSNode : public pebbl::parallelBranchSub, public serialPIDOMSNode<ProblemT>
{
  #if !defined(SOLARIS)
  using serialPIDOMSNode<ProblemT>::branchVariable;
  using serialPIDOMSNode<ProblemT>::lower;
  using serialPIDOMSNode<ProblemT>::upper;
  using serialPIDOMSNode<ProblemT>::branch_status;
  using serialPIDOMSNode<ProblemT>::midpoint;
  using serialPIDOMSNode<ProblemT>::corner_dist;
  #endif

public:

  /// Return a pointer to the global branching object
  parallelPIDOMS<ProblemT>* global() const
    { return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::parallelBranching* pGlobal() const
    { return global(); }

  /// An empty constructor for a subproblem
  parallelPIDOMSNode()
    { }

  /// A virtual destructor for a subproblem
  virtual ~parallelPIDOMSNode()
    { }

  /**
   * Initialize a subproblem using a branching object
   */
  void initialize(parallelPIDOMS<ProblemT>* master)
    {
    globalPtr = master;
    serialPIDOMSNode<ProblemT>::initialize( master );
    }

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   */
  void initialize(parallelPIDOMSNode<ProblemT>* parent,int whichChild)
    {
    globalPtr = parent->globalPtr;
    serialPIDOMSNode<ProblemT>::initialize(parent, whichChild);
    }

  /// Pack the information in this subproblem into a buffer
  void pack(utilib::PackBuffer& outBuffer)
    {
    unsigned int len = midpoint.size();
    outBuffer << len;
    for (unsigned int i=0; i<len; i++)
          outBuffer << midpoint[i] << lower[i] << upper[i];

    //outBuffer << midpoint_value << corner_dist << branchVariable;
    if (branch_status == serialPIDOMS<ProblemT>::no_branch)
       outBuffer << "no_branch";
    if (branch_status == serialPIDOMS<ProblemT>::branch_down)
       outBuffer << "branch_down";
    if (branch_status == serialPIDOMS<ProblemT>::branch_up)
       outBuffer << "branch_up";

    double flag = -999.0;
    outBuffer << flag;
    }

  /// Unpack the information for this subproblem from a buffer
  void unpack(utilib::UnPackBuffer& inBuffer)
    {
    unsigned int len;
    inBuffer >> len;
    for (unsigned int i=0; i<len; i++)
          inBuffer >> midpoint[i] >> lower[i] >> upper[i];

    //inBuffer >> midpoint_value >> corner_dist >> branchVariable;
    //
    // Need to be be careful packing and unpacking enum types
    //
    utilib::CharString branch_code;
    inBuffer >> branch_code;
    if (branch_code == "branch_down")
       branch_status = serialPIDOMS<ProblemT>::branch_down;
    if (branch_code == "branch_up")
       branch_status = serialPIDOMS<ProblemT>::branch_up;
    if (branch_code == "no_branch")
       branch_status = serialPIDOMS<ProblemT>::no_branch;

    double flag;
    inBuffer >> flag;
    if (flag != -999.0)
       EXCEPTION_MNGR(std::runtime_error,"parallelPIDOMSNode - problem unpacking");
    }

  /// Create a child subproblem of the current subproblem
  virtual parallelBranchSub* makeParallelChild(int whichChild)
    {
    parallelPIDOMSNode<ProblemT> *temp = new parallelPIDOMSNode<ProblemT>;
    temp->initialize(this, whichChild);
    return temp;
    }

protected:

  /// A pointer to the global branching object
  parallelPIDOMS<ProblemT>* globalPtr;

};


template <class ProblemT>
pebbl::parallelBranchSub* parallelPIDOMS<ProblemT>::blankParallelSub()
{
parallelPIDOMSNode<ProblemT> *temp = new parallelPIDOMSNode<ProblemT>();
temp->initialize(this);
return temp;
}


#endif


}



PIDOMS::PIDOMS()
 : step_tolerance(1e-4),
   initial_step(1.0),
   Lipshitz(1.0)
{
this->properties.declare("Lipshitz", 
        "The Lipshitz constant for bounding the subdomain",
        utilib::Privileged_Property(Lipshitz) );
        //"<double>", "0.0",
app_handler_ = new pidoms::PIDOMSHandler<problem_t>();
#if defined(ACRO_HAVE_MPI)
if (utilib::uMPI::running() && (utilib::uMPI::size == 1))
   solver = new pidoms::PIDOMSSerialSolver<problem_t>;
else {
   solver = new pidoms::PIDOMSParallelSolver<problem_t>;
   }
#else
solver = new pidoms::PIDOMSSerialSolver<problem_t>;
#endif

app_handler_->solver = this;

//augment_parameters(solver->parameters(),true);

properties.erase("accuracy");
properties.erase("ftol");

solver->base()->set_parameter("absTolerance",1e-5);
//solver->base()->set_parameter("lipshitzConstant",1.0);

this->reset_signal.connect(boost::bind(&PIDOMS::reset_PIDOMS, this));
}


PIDOMS::~PIDOMS()
{ delete solver; }


void PIDOMS::reset_PIDOMS()
{
if ( this->problem.empty() )
   return;
//if (problem->numConstraints() > 0)
   //EXCEPTION_MNGR(std::runtime_error, "PIDOMS::reset() - PIDOMS cannot solve a constrained optimization problem");

app_handler_->set_problem(this->problem);
app_handler_->Lipshitz_constant=Lipshitz;
solver->set_handler(app_handler_);

#if defined(ACRO_HAVE_MPI)
if (utilib::uMPI::size > 1)
   #ifdef UTILIB_MPI_COMM_IS_POINTER
   //this->set_parameter("mpicomm",static_cast<void*>(utilib::uMPI::comm));
   #else
   //this->set_parameter("mpicomm",utilib::uMPI::comm);
   #endif
#endif

if (this->problem->finite_bound_constraints()) {
    app_handler_->lower = this->problem->real_lower_bounds;
    app_handler_->upper = this->problem->real_upper_bounds;
    }

ucout << utilib::Flush;
std::cout.flush();
solver->reset();
}


void PIDOMS::optimize()
{
if (!this->problem->finite_bound_constraints()) {
   this->best().termination_info = "Missing-Bound-Constraints";
   return;
   }

// JDS - This is not necessary??
//this->opt_init();

int tmp_neval = std::max(this->max_neval-this->neval(),0);
if (this->max_neval_curr != 0)
   tmp_neval = std::min(tmp_neval, this->max_neval_curr);

try {
  solver->minimize(this->best());
}
STD_CATCH(;)

#if 0
colin::AppRequest request = problem->set_domain(x);
problem->Request_response(request, best().response);
problem->Request_F(request, best().value());
eval_mngr().perform_evaluation(request);
if (problem->numConstraints() > 0)
   problem->Eval(x, best().response, colin::mode_f | colin::mode_cf);
else
   problem->Eval(x, best().response, colin::mode_f);
compute_response_info(best().response,problem->state->constraint_lower_bounds, problem->state->constraint_upper_bounds, best().value(),best().constraint_violation);
#endif
}


void PIDOMS::set_handler(pidoms::PIDOMSHandler<problem_t>* app_handler_)
{
app_handler_=app_handler_;
app_handler_->set_problem(this->problem);
}

} // namespace scolib

#endif
