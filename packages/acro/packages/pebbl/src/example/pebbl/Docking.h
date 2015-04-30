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
 * \file serialDocking.h
 *
 * A simple nonlinear branch-and-bound method for Docking using PEBBL.
 */
#ifndef Docking_h
#define Docking_h

#include <acro_config.h>
#include <utilib/exception_mngr.h>
#include <utilib/BitArray.h>
#include <utilib/BasicArray.h>
#include <utilib/Ereal.h>
#include <utilib/ValuedContainer.h>
#include <utilib/stl_auxiliary.h>
#include <pebbl/branching.h>
#include <pebbl/DockingProblem.h>
#include <pebbl/gRandom.h>

namespace pebbl {

using utilib::BasicArray;
using utilib::BitArray;
using utilib::Ereal;
using utilib::ValuedContainer;


// JE added this very simple class so the solution prints as 
// "Docking solution" both in serial and parallel 

class DockingSolution :
public arraySolution<int>
{
 public:

  const char* typeDescription() const { return "Docking solution"; };

  // Constructors

  // Construct a solution of known size.  Typically used to create a
  // reference solution for registration and buffer size calculation.

  DockingSolution(size_type  size,
		  branching* bGlobal) :
  arraySolution<int>(size,bGlobal,NULL,NULL)
    { };

  // Copy-like constructor for use by blankClone().  Does *not* copy
  // data itself.

 DockingSolution(DockingSolution* otherDockingSolution) :
 arraySolution<int>(otherDockingSolution) 
   { };

  // Explicitly construct a solution with contents from a supplied array
  // and objective value

 DockingSolution(double           obj,
		 BasicArray<int>& point,
		 branching*       bGlobal) :
 arraySolution<int>(obj,point,bGlobal)
   { };
  
  // Communication-related methods

#ifdef ACRO_HAVE_MPI

  virtual solution* blankClone() { return new DockingSolution(this); };

#endif

};



/**
 * Defines the branching class for a serialDocking
 */
class serialDocking : virtual public pebbl::branching 
{
public:

  /// An empty constructor
  serialDocking()
        {
        relTolerance=0.0;
        using_dee=false;
        balanced_branching=false;
        evenly_balanced_branching=true;
        limited_dee=true;
        branchingInit();
        bound_type=2;                       // Default now nested bound
	sparse_flag=true;                   // With sparse computation
        rotamer_weighting_flag=false;
        guess_iterations = 3;
	heur_iterations = 3;
	heur_refine = 1000;
	heur_bias = 0.75;
	max_heur_depth = 10;

        create_parameter("using-dee",using_dee,"<bool>",
                "false if enumerating, true if not enumerating",
                "If true, then try to apply DEE at each node");
        create_parameter("limited-dee",limited_dee,"<bool>","true",
                "If true, then only apply DEE to a branch variable once");
        create_parameter("balanced_branching",balanced_branching,
			 "<bool>","false",
                "If true, then divide each window in half");
        create_parameter("evenly-balanced-branching",
			 evenly_balanced_branching,
			 "<bool>","true",
                "If true, then divide each window in half in an even manner");
        create_parameter("bound-type",bound_type,"<int>","2",
			 "1: simple bound, 2: nested (stronger) bound",
			 ParameterBounds<int>(1,2));
        create_parameter("sparse",sparse_flag,
			 "<bool>","true",
			 "If true, exploit sparsity in bound computation");
        create_parameter("rotamer-weighting",rotamer_weighting_flag,
			 "<bool>","false",
                "If true, then used the advanced rotamer weighting scheme.");
        create_parameter("guess-iterations",guess_iterations,"<int>","2",
			 "Number of attempts for initial guess",
			 ParameterNonnegative<int>());
	create_parameter("heur-iterations",heur_iterations,"<int>","3",
			 "Maximum attempts for heuristic",
			 ParameterNonnegative<int>());
	create_parameter("heur-refine",heur_refine,"<int>","1000",
			 "Maximum refinement steps in heuristic",
			 ParameterNonnegative<int>());
	create_parameter("max-heur-depth",max_heur_depth,"<int>","10",
			 "Maximum tree depth to apply heuristic",
			 ParameterNonnegative<int>());
	create_parameter("heur-bias",heur_bias,"<double>","0.75",
			 "Heuristic randomness bias",
			 ParameterBounds<double>(0.0,1.0));

        info.ioPtr = this;
        }
                
  /// A destructor
  virtual ~serialDocking()
        { }

  ///
  void reset(bool resetVB = true)
        {
        pebbl::branching::reset(resetVB);
        if (!parameter_initialized("using-dee"))
	  // If DEE not specified, default is yes if not enumerating,
          // no if enumerating.
	    using_dee = !enumerating;
        }

  ///
  bool setupProblem(int& argc, char**& argv)
        {
        initialize(argv[1]);
        return true;
        }

  /// Initialize the optimizer with standard information
  void initialize(char* filename)
        {
        info.read(filename,rotamer_weighting_flag);
        DEBUGPR(3000,
                ucout << "INFO MAP: " << std::endl;
                for (size_type i=0; i<info.n; i++)
                   ucout << info.inter_order[i] << std::endl;
                );
        initializationGuts();
        }

  /// Initialization guts.  We need to split this from the file reading,
  /// since all processors need to do this in parallel (1 from the file,
  /// the others from a broadcast.

  void initializationGuts()
    {
      pCostTotal.resize(info.n);
      pCostCtr.resize(info.n);
      pCostVal.resize(info.n);
      pCostTotal << 0.0;
      pCostCtr << 0;
      pCostVal << -1.0;
    }


  virtual bool guessDeterministic(int i)
  { return i==0; };

  /// Initial guess routine
  solution* initialGuess()
  {
    DEBUGPR(4,ucout << "In initialGuess\n");
    double    guessVal = MAXDOUBLE;
    BasicArray<int> tryArray(info.n);
    BasicArray<int> solArray(info.n);
    double    tryVal;
    for(int i=0; i<guess_iterations; i++)
      {
        if (guessDeterministic(i))
          {
            DEBUGPR(4,ucout << "Deterministic greedy guess\n");
            tryVal = info.find_greedy(tryArray,heur_refine);
          }
        else
          {
            DEBUGPR(4,ucout << "Random greedy guess\n");
            tryVal = info.find_random_greedy(tryArray,gRandom,
					     heur_refine,heur_bias);
          }
        DEBUGPR(4,ucout << "Trial guess value " << tryVal << std::endl);
        if (tryVal < guessVal)
          {
            guessVal = tryVal;
            solArray << tryArray;
          }
      }
    DEBUGPR(4,ucout << "Initial guess value is " << guessVal << std::endl);
    if (guessVal < MAXDOUBLE)
      return new DockingSolution(guessVal,solArray,this);
    return NULL;
  }


  /// Return a new subproblem
  pebbl::branchSub* blankSub();


  ///
  DockingProblem info;

  /**
   * Apply DEE to a single dimension.
   * If DEE eliminates all but one value, then this returns that fixed
   * dimension.  Otherwise, it returns -1
   */
  int apply_dee(size_type variable, 
                std::list<ValuedContainer<double,int> >& dee_status, 
                BasicArray<BitArray>& free_rotamers);

  ///
  void dee_calculation(size_type i, size_type minndx, size_type variable, 
                bool& i_flag, bool& min_flag, 
                Ereal<double>& i_diff, Ereal<double>& min_diff, 
                BasicArray<BitArray>& );

  ///
  bool haveIncumbentHeuristic() {return true;}

  ///
  enum branch_type {branch_up, branch_down, no_branch, random_branch};

  /// Flag that is true if DEE can be applied.
  bool using_dee;

  /// Flag is true if DEE is applied only once to each dimension
  bool limited_dee;

  /// 
  int bound_type;

  ///
  bool sparse_flag;

  ///
  bool rotamer_weighting_flag;

  ///
  bool balanced_branching;

  ///
  bool evenly_balanced_branching;


  /// Number of attempts at (random) initial greedy solution
  int guess_iterations;

  /// Number of attempts for node heuristic
  int heur_iterations;

  /// Maximum refinement steps for heuristic
  int heur_refine;

  /// Randomness bias for heuristic
  double heur_bias;

  /// Maximum depth to apply heuristic
  int max_heur_depth;

  BasicArray<double> pCostTotal;

  ///
  BasicArray<int> pCostCtr;

  ///
  BasicArray<double> pCostVal;

};



/**
 * Defines the nodes used for a serialDocking
 */
class serialDockingNode : virtual public pebbl::branchSub
{
public:

  /// Return a pointer to the global branching object
  serialDocking* global() const
        { return globalPtr; }

  /// Return a pointer to the base class of the global branching object
  pebbl::branching* bGlobal() const
        { return global(); }

  /**
   * Link the debugging in the subproblem to the debugging level
   * set within the global branching object.
   */
  REFER_DEBUG(global())

  /// An empty constructor for a subproblem
  serialDockingNode()
        : branch_status(serialDocking::no_branch),
          branchVariable(-2)
        { }

  /// A virtual destructor for a subproblem
  virtual ~serialDockingNode()
        { }

  /**
   * Initialize a subproblem using a branching object
   */
  void initialize(serialDocking* master)
        {
        globalPtr = master;
        free_rotamers.resize(globalPtr->info.n);
        lower.resize(globalPtr->info.n);
        upper.resize(globalPtr->info.n);
        point.resize(globalPtr->info.n);
        dee_branch.resize(globalPtr->info.n);
        for (size_type i=0; i<free_rotamers.size(); i++) {
          if (globalPtr->info.rCount[i] > 0) {
             free_rotamers[i].resize(globalPtr->info.rCount[i]);
             free_rotamers[i].set();
             lower[i] = 0;
             upper[i] = globalPtr->info.rCount[i]-1;
          }
          else {
             lower[i] = -1;
             upper[i] = -1;
          }
          }
        bound = Ereal<double>::negative_infinity;
        parent_bound = Ereal<double>::negative_infinity;
        dee_branch.set();
        parent_branch = -1;
        }

  /**
   * Initialize a subproblem as a child of a parent subproblem.
   * This method is not strictly necessary, but its use here illustrates
   * a flexible mechanism for managing the initialization of subproblems.
   * The following crude fragment illustrates some common steps needed in
   * this method.
   */
  void initialize(serialDockingNode* parent, int whichChild)
        {
        globalPtr = parent->globalPtr;
        branchSubAsChildOf(parent);
        //
        // Initialize this node's info
        //
        free_rotamers = parent->free_rotamers;
        lower = parent->lower;
        upper = parent->upper;
        point = parent->point;
        parent_bound = parent->bound;
        parent_branch = parent->branchVariable;
        bound = parent->bound;
        branch_size=1;
        if (globalPtr->limited_dee)
           dee_branch = parent->dee_branch;
        else {
           dee_branch.resize(parent->dee_branch.size());
           dee_branch.set();
           }

        //
        // Perform a branch using DEE information
        //
        if ((free_rotamers[parent->branchVariable].size() == 
             free_rotamers[parent->branchVariable].nbits()) && 
                                (parent->dee_status.size() > 0)) {
           //
           // The 'down' branch is to simply eliminate the rotamers that
           // DEE identified
           //
           DEBUGPR(300, ucout << "DEE Status: " 
                   << parent->dee_status << std::endl);
           DEBUGPR(300, 
                   if (parent->branch_status == serialDocking::branch_down) {
                      ucout << "BRANCH DOWN " << parent->branchVariable 
                            << std::endl; 
                   } else {
                      ucout << "BRANCH UP " << parent->branchVariable 
                            << std::endl; 
                   }
                   );
           if (parent->branch_status == serialDocking::branch_down) {
              std::list<ValuedContainer<double,int> >::iterator curr 
		= parent->dee_status.begin();
              std::list<ValuedContainer<double,int> >::iterator end
		= parent->dee_status.end ();
              while (curr != end) {
                free_rotamers[parent->branchVariable].reset( 
                        globalPtr->info.inter_order[parent->branchVariable][curr->info]);
                curr++;
                }
              }
           else {
              free_rotamers[parent->branchVariable].reset();
              std::list<ValuedContainer<double,int> >::iterator curr 
		= parent->dee_status.begin();
              std::list<ValuedContainer<double,int> >::iterator end  
		= parent->dee_status.end ();
              while (curr != end) {
                free_rotamers[parent->branchVariable].set( 
                        globalPtr->info.inter_order[parent->branchVariable][curr->info]);
                curr++;
                }
              }
           //
           // Update the lower/upper info
           //
           upper[parent->branchVariable] = -1;
           lower[parent->branchVariable] = static_cast<int>(free_rotamers[parent->branchVariable].size());
           for (size_type i=0; 
		i<free_rotamers[parent->branchVariable].size(); 
		i++) {
             if (free_rotamers[parent->branchVariable]( 
                        globalPtr->info.inter_order[parent->branchVariable][i]))
	       {
                lower[parent->branchVariable] = 
                        std::min(lower[parent->branchVariable], 
				 static_cast<int>(i));
                upper[parent->branchVariable] = 
                        std::max(upper[parent->branchVariable], 
				 static_cast<int>(i));
                }
             }
           DEBUGPR(300,
                   ucout << "DEE BRANCH - " << whichChild << std::endl;
                   ucout << parent->lower << std::endl;
                   ucout << parent->upper << std::endl;
                   ucout << lower << std::endl;
                   ucout << upper << std::endl;
                   ucout << "DEE INFO: " << parent->dee_status << std::endl;
                   ucout << free_rotamers << std::endl << std::endl;
                   );

           parent->branch_status = serialDocking::branch_up;
           }

        //
        // Perform a regular branch
        //
        else {
           //
           // If we are asked for the 2nd child, switch the branch
           // status to be the "opposite" of the last branch (in the parent!)
           //
           if (whichChild > 0) {
              if (parent->branch_status == serialDocking::branch_down)
                 parent->branch_status = serialDocking::branch_up;
              else if (parent->branch_status == serialDocking::branch_up)
                 parent->branch_status= serialDocking::branch_down;
              else 
                 EXCEPTION_MNGR(runtime_error, 
				"Trying to create a child without a "
				"proper branching status: whichChild = " 
				<< whichChild);
              }
           //
           // If the branch_status is no_branch, then this is the first
           // branch for a subproblem and the user has not defined a default
           // branching policy.  We are simply making this down_branch for now.
           //
           // Compute the branch
           //
           if (lower[parent->branchVariable] == upper[parent->branchVariable])
              EXCEPTION_MNGR(runtime_error,"Branching on a fixed variable");
           if (free_rotamers[parent->branchVariable].nbits() < 2)
              EXCEPTION_MNGR(runtime_error,"Branching on a fixed variable");

           //cerr << "HERE " << parent->branchVariable << "\t" << free_rotamers[parent->branchVariable].nbits() << std::endl;
           int ndx;
           if (globalPtr->evenly_balanced_branching) {
              int num_free = free_rotamers[parent->branchVariable].nbits() / 2;
              ndx = 0;
              for (size_type i=lower[parent->branchVariable];
                   static_cast<int>(i) <= upper[parent->branchVariable]; i++) {
                if (free_rotamers[parent->branchVariable](
                     globalPtr->info.inter_order[parent->branchVariable][i])) {
                   ndx++;
                   if (ndx == num_free) { ndx = i; break; }
                   }
                }
              }

           else if (globalPtr->balanced_branching)
              ndx = lower[parent->branchVariable] + static_cast<int>(std::floor((upper[parent->branchVariable] - lower[parent->branchVariable])/2.0));

           else
              ndx = lower[parent->branchVariable];

           if (parent->branch_status == serialDocking::branch_down)
              upper[parent->branchVariable] = ndx;
           else
              lower[parent->branchVariable] = ndx+1;
           branch_size=0;
           for (size_type i=0; i<free_rotamers[parent->branchVariable].size(); i++) {
             if (((static_cast<int>(i) < lower[parent->branchVariable]) || 
                  (static_cast<int>(i) > upper[parent->branchVariable])) &&
                free_rotamers[parent->branchVariable](
                        globalPtr->info.inter_order[parent->branchVariable][i])
                ) {
                branch_size++;
                DEBUGPR(300, ucout << "RESET " << i << " " << 
                        globalPtr->info.inter_order[parent->branchVariable][i]
                        << std::endl);
                free_rotamers[parent->branchVariable].reset(
                        globalPtr->info.inter_order[parent->branchVariable][i]);
                }
             }
           if ( (branch_size==0) || 
                (free_rotamers[parent->branchVariable].nbits() == 0) )
              EXCEPTION_MNGR(runtime_error, "Problem with branching: " << ndx << " " << parent->branchVariable << " " << lower[parent->branchVariable] << " " << upper[parent->branchVariable] << std::endl);

           //
           // Update the lower/upper info
           //
           upper[parent->branchVariable] = -1;
           lower[parent->branchVariable] 
	     = static_cast<int>(free_rotamers[parent->branchVariable].size());
           for (size_type i=0; 
		i<free_rotamers[parent->branchVariable].size(); 
		i++) {
             if (free_rotamers[parent->branchVariable]( 
                        globalPtr->info.inter_order[parent->branchVariable][i])) {
                lower[parent->branchVariable] = 
                        std::min(lower[parent->branchVariable], 
				 static_cast<int>(i));
                upper[parent->branchVariable] = 
                        std::max(upper[parent->branchVariable], 
				 static_cast<int>(i));
                }
             }
           DEBUGPR(300,
                   ucout << "STANDARD BRANCH - " << whichChild << std::endl;
                   ucout << "Branch Variable: " 
		       << parent->branchVariable << std::endl;
                   ucout << parent->lower << std::endl;
                   ucout << parent->upper << std::endl;
                   ucout << lower << std::endl;
                   ucout << upper << std::endl;
                   for (unsigned int i=0; i<free_rotamers.size(); i++) {
                      ucout << free_rotamers[i].nbits() << " " 
                            << free_rotamers[i] << std::endl;
                   }
                   ucout << std::endl;
                   ucout << Flush;
                   );
           }
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
  void boundComputation(double* controlParam)
        {
	  double 
	    calcBound = globalPtr->info.compute_bound(free_rotamers,
						      globalPtr->bound_type,
						      globalPtr->sparse_flag);
	  bound = std::max(bound,calcBound);
	  // We will see bound == parent_bound during a DEE branch
	  if ((parent_branch >= 0) && (bound > parent_bound)) {
	    globalPtr->pCostTotal[parent_branch] 
	      += (bound - parent_bound)/branch_size;
	    globalPtr->pCostCtr[parent_branch]++;
	    globalPtr->pCostVal[parent_branch] 
	      = globalPtr->pCostTotal[parent_branch]
	        / globalPtr->pCostCtr[parent_branch];
           }

        setState(bounded);
        /// Some debugging info
        }

  /**
   * Determine how many children will be generated and how they will be
   * generated (e.g. the branching variable).
   * The return value is the number of children that can be generated from
   * this node.
   */
  virtual int splitComputation()
        { 
// #if 0
//         double ctr=0;
//         for (size_type i=0; i<lower.size(); i++)
//           ctr += std::log(upper[i]-lower[i]+1.0);
//         ucout << "Subproblem size: " << ctr << std::endl;
// #endif

        if (upper == lower) {
           setState(separated);
           branch_status = serialDocking::no_branch;
           return 0;
           }

        #if 0
        if (globalPtr->evenly_balanced_branching) {
           branchVariable = 0;
           for (unsigned int i=1; i<lower.size(); i++)
             if (free_rotamers[i].nbits() > free_rotamers[branchVariable].nbits())
                branchVariable=i;
           }
        else {
           //
           // Search for dimension with lowest range
           //
           double range = upper[0] - lower[0];
           branchVariable = 0;
           unsigned int i=1;
           while ((range == 0.0) && (i < lower.size())) {
             range = upper[i] - lower[i];
             branchVariable=i;
             i++;
             }
           for (; i<lower.size(); i++) {
             double tmp = upper[i] - lower[i];
             if ((tmp > 0.0) && (tmp > range)) {
                range = tmp;
                branchVariable = i;
                }
             }
           }
        #else
        unsigned int i=0;
        while (lower[i] == upper[i]) i++;
        branchVariable = i;
        while (i < lower.size()) {
          if (lower[i] < upper[i]) {
             if (globalPtr->pCostVal[i] == -1.0) {
                branchVariable=i;
                break;
                }
             if (globalPtr->pCostVal[i]*free_rotamers[i].nbits() > globalPtr->pCostVal[branchVariable]*free_rotamers[branchVariable].nbits())
                branchVariable=i;
             }
          i++;
          }
        DEBUGPR(3000,ucout << "BranchVariable=" << branchVariable 
                << " pCostVal: " << globalPtr->pCostVal << std::endl);
        #endif
        //
        // Apply DEE to the selected dimension
        //
        if (globalPtr->using_dee && dee_branch[branchVariable] &&
            (free_rotamers[branchVariable].nbits() > 2)) {
           globalPtr->apply_dee(branchVariable,dee_status,free_rotamers);
           DEBUGPR(3000,ucout << "apply_dee information: BranchVariable=" 
                   << branchVariable << " DEE Info: " << dee_status 
                   << std::endl);
           dee_branch.reset(branchVariable);
           }
        else {
           DEBUGPR(3000,ucout << "DEE NOT APPLIED: BranchVariable=" 
                   << branchVariable << " globalPtr->using_dee " 
                   << globalPtr->using_dee << " dee_branch[branchVariable] " 
                   << dee_branch[branchVariable] << " nfree rotamers " 
                   << free_rotamers[branchVariable].nbits() << std::endl);
           }

        setState(separated);
        branch_status = serialDocking::branch_down;
        if ((dee_status.size() > 0) && !(globalPtr->enumerating)) return 1;
#if 1
        //
        // If performing enumeration, get rid of the first part of the
        // dee_status info to help balance the search
        //
        while (dee_status.size() > free_rotamers[branchVariable].nbits()/2)
          dee_status.pop_front();         
#endif
        return 2;
        }

  /// Create a child subproblem of the current subproblem
  virtual branchSub* makeChild(int whichChild)
        {
        serialDockingNode *temp = new serialDockingNode;
        temp->initialize(this, whichChild);
        return temp;
        }
  
  /// Returns true if this subproblem represents a feasible solution
  bool candidateSolution()
        {
        if (lower == upper) {
           for (unsigned int i=0; i<lower.size(); i++)
             if (globalPtr->info.rCount[i] > 0)
                point[i] =  globalPtr->info.inter_order[i][lower[i]];
             else
                point[i] =  -1;
           DEBUGPR(3000, ucout << "candidateSolution found: lower= " << lower 
                   << " upper= " << upper << std::endl);
           DEBUGPR(3000, ucout << "candidateSolution found: " << point_value 
                   << " point= " << point << std::endl);
           point_value = globalPtr->info.energy(point);
           return true;
           }
        return false;
        }

  /// Creates a solution for a node
  virtual solution* extractSolution()
    {
      DEBUGPR(300, ucout << "extractSolution    - value = " << \
              point_value << std::endl);
      if (!globalPtr->canFathom(point_value))
        return new DockingSolution(point_value,point,globalPtr);
      return NULL;
    }

  void incumbentHeuristic()
    {
    DEBUGPR(10,ucout << "Docking incumbent heuristic running...\n");
    // TODO: revisit this value
    if (depth > globalPtr->max_heur_depth) return;

    DEBUGPR(40,ucout << "Subproblem guess\n");
    double    guessVal = MAXDOUBLE;
    BasicArray<int> tryArray(globalPtr->info.n);
    BasicArray<int> solArray(globalPtr->info.n);
    double    tryVal;

    for(int i=0; i<globalPtr->heur_iterations; i++)
      {
        if (globalPtr->guessDeterministic(i))
          {
            DEBUGPR(40,ucout << "Deterministic greedy guess\n");
            tryVal = globalPtr->info.find_greedy(tryArray, 
						 globalPtr->heur_refine, 
						 &free_rotamers);
          }
        else
          {
            DEBUGPR(40,ucout << "Random greedy guess\n");
            //
            // TODO: revisit the bias used here
            // When this number approaches 1.0, the randomization 
	    // becomes uniform
            //
            tryVal = globalPtr->info.find_random_greedy(tryArray,
							gRandom, 
							globalPtr->heur_refine, 
							globalPtr->heur_bias, 
							&free_rotamers);
          }
        DEBUGPR(40,ucout << "Trial guess value " << tryVal << std::endl);
        if (tryVal < guessVal)
          {
            guessVal = tryVal;
            solArray << tryArray;
          }
      }
    DEBUGPR(40,ucout << "Initial guess value is " << guessVal << std::endl);
    if (guessVal < MAXDOUBLE)
      foundSolution( new DockingSolution(guessVal,solArray,globalPtr) );
    }

protected:

  /// Valid solutions.  These are represented in an UNMAPPED format
  BasicArray<int> point;

  /// A pointer to the global branching object
  serialDocking* globalPtr;

  /// Lower bounds on the search domain
  BasicArray<int> lower;

  /// Upper bounds on the search domain
  BasicArray<int> upper;

  ///
  BasicArray<BitArray> free_rotamers;

  /// Flag that indicates which branch to take next to create a subproblem
  serialDocking::branch_type branch_status;

  /**
   * The variable that will be branched next.
   * If set to -1, then this is the root computation.
   */
  int branchVariable;

  /**
   * Status of DEE information for the branchVariable dimension.
   * A 'true' value indicates that this variable has been eliminated.
   */
  std::list<ValuedContainer<double,int> > dee_status;

  BitArray dee_branch;

  ///
  Ereal<double> parent_bound;

  ///
  Ereal<double> point_value;

  ///
  int parent_branch;

  ///
  int branch_size;

};


inline int serialDocking::apply_dee(size_type variable, std::list<ValuedContainer<double,int> >& dee_status, BasicArray<BitArray>& free_rotamers)
{
size_type minndx=0;
while ( !free_rotamers[variable]( info.inter_order[variable][minndx] ) )
  minndx++;

for (size_type i=free_rotamers[variable].size()-1; i>minndx; i--) {
  bool i_flag, min_flag;
  //
  // Ignore rotamers that are not allowed
  //
  if ( !free_rotamers[variable]( info.inter_order[variable][i] ) ) continue;

  Ereal<double> i_diff, min_diff;
  dee_calculation(i,minndx,variable,i_flag,min_flag,i_diff,min_diff,free_rotamers);

  //cerr << "DEE " << i << " " << i_diff << " " << i_flag << std::endl;
  if (i_flag) {
     double dtmp = i_diff;
     int itmp = i;
     ValuedContainer<double,int> tmp(dtmp,itmp);
     dee_status.push_back(tmp);
     }
  }

//cerr << "DEE Status " << dee_status << std::endl;
dee_status.sort();
//cerr << "DEE Status " << dee_status << std::endl;

//
// Check to see if there is just one rotamer left
//
if ((dee_status.size() + free_rotamers[variable].nbits()+1) ==
        free_rotamers[variable].size()) {
   for (size_type i=0; i<free_rotamers[variable].size(); i++)
     if (free_rotamers[variable]( info.inter_order[variable][i] ))
        return i;
   }

return -1;
}




inline void serialDocking::dee_calculation(size_type i, size_type minndx, size_type variable, bool& i_flag, bool& min_flag, Ereal<double>& i_diff, Ereal<double>& min_diff, BasicArray<BitArray>& free_rotamers)
{
  min_diff = 0.0;
  i_diff = 0.0;
  i_flag=min_flag=false;
  //
  // Compute the DEE criterion to determin if rotamer 'i' can be eliminated.
  // Note that this comparison is only done against rotamer 'minndx'.  More
  // generally, we could compare against all other rotamers ... but it's
  // unclear that that would necessarily improve the search efficacy.
  //
  ////
  //// Difference of INTER energies
  ////
  Ereal<double> dee_val = 
                info.E_inter[variable][ info.inter_order[variable][i] ]  -
                info.E_inter[variable][ info.inter_order[variable][minndx] ];
  ////
  //// Sum of differences of INTRA energies
  ////
  for (size_type varj=0; varj<info.n; varj++) {
    if (info.rCount[varj] == 0) continue;
    if (varj == variable) continue;

    IntraInfo* intra_info;
    if (varj < variable)
       intra_info = &info.E_intra[varj][variable];
    else
       intra_info = &info.E_intra[variable][varj];

    Ereal<double> minval = Ereal<double>::positive_infinity;
    for (size_type j=0; j<free_rotamers[varj].size(); j++) {
      if ( !free_rotamers[varj]( info.inter_order[varj][j] ) ) continue;
      //
      // Now we know that (variable,i), (variable,minndx) and (varj,j)
      // are all valid (side-chain,rotamer) combinations.
      //
      int ki = -2;
      int kmin = -2;
      if (varj < variable) {
         ki   = intra_info->index[ info.inter_order[varj][j] ]
                                  [ info.inter_order[variable][i] ];
         kmin = intra_info->index[ info.inter_order[varj][j] ]
                                  [ info.inter_order[variable][minndx] ];
         }
      else {
         ki   = intra_info->index[ info.inter_order[variable][i] ]
                                  [ info.inter_order[varj][j] ];
         kmin = intra_info->index[ info.inter_order[variable][minndx] ]
                                  [ info.inter_order[varj][j] ];
         }
      double ival = (ki == -1 ? 0.0 : intra_info->e[ki]);
      double mval = (kmin == -1 ? 0.0 : intra_info->e[kmin]);
      if ((ival - mval) < minval)
         minval = ival - mval;
      }
    //
    // If we have infinity, then there wasn't a feasible rotamer-rotamer 
    // interaction in this comparison, so we should ignore this DEE
    // comparison.
    //
    if (minval == Ereal<double>::positive_infinity)
       EXCEPTION_MNGR(runtime_error, "No feasible DEE calculation.");             
    //
    // Otherwise, update 'dee_val'
    //
    dee_val += minval;
    }

  if (dee_val > 0.0) {
     i_flag=true;
     i_diff = dee_val;
     }
}


inline pebbl::branchSub* serialDocking::blankSub()
{
serialDockingNode *temp = new serialDockingNode();
temp->initialize(this);
return temp;
}


#ifdef ACRO_HAVE_MPI
}

#include <pebbl/parBranching.h>

namespace pebbl {


class parallelDocking : public parallelBranching, public serialDocking
{
public:

  void pack(PackBuffer& outBuffer)
    { 
      outBuffer << info << using_dee << limited_dee << bound_type 
		<< sparse_flag << rotamer_weighting_flag 
		<< balanced_branching << evenly_balanced_branching; 
    }

  void unpack(UnPackBuffer& inBuffer)
    { 
      inBuffer >> info >> using_dee >> limited_dee >> bound_type 
	       >> sparse_flag >> rotamer_weighting_flag 
	       >> balanced_branching >> evenly_balanced_branching;
      // Initialize based on this information
      initializationGuts();
    }

  int spPackSize()
    { return _spPackSize; }

  parallelBranchSub* blankParallelSub();

  parallelDocking() : serialDocking(), _spPackSize(0) {}
  ~parallelDocking() { }

  bool setup(int& argc,char**& argv);

  void solve() 
    { parallelBranching::solve(); };

  void printSolution(const char* header = "",
             const char* footer = "",
             std::ostream& outStream = ucout)
    { parallelBranching::printSolution(header,footer,outStream); }


  void reset(bool VBflag=true)
    {
      serialDocking::reset();
      registerFirstSolution(new DockingSolution(info.n,this));
      parallelBranching::reset();
    }

  bool guessDeterministic(int i)
  { return serialDocking::guessDeterministic(i) && (uMPI::rank == 0); };

  int _spPackSize;

};

class parDockingNode : public parallelBranchSub, public serialDockingNode
{
public:

  parDockingNode() {}
  ~parDockingNode() { }

  void initialize(parallelDocking* master_)
    {
      globalPtr = master_;
      serialDockingNode::initialize(master_);
    }

  void initialize(parDockingNode* parent,int whichChild)
    {
      globalPtr = parent->globalPtr;
      serialDockingNode::initialize(parent,whichChild);
    }

  parallelDocking*      global()  const { return globalPtr; };
  parallelBranching*    pGlobal() const { return globalPtr; };

  void pack(PackBuffer& outBuffer)
    { 
    outBuffer << point;
    outBuffer << lower;
    outBuffer << upper;
    outBuffer << free_rotamers;
    outBuffer << (int)branch_status;
    outBuffer << branchVariable;
    outBuffer << dee_status;
    outBuffer << dee_branch;
    }

  void unpack(UnPackBuffer& inBuffer)
    {
    inBuffer >> point;
    inBuffer >> lower;
    inBuffer >> upper;
    inBuffer >> free_rotamers;
    // Have to unpack enums into temporary variables
    int temp;
    inBuffer >> temp;
    branch_status = (serialDocking::branch_type) temp;
    inBuffer >> branchVariable;
    inBuffer >> dee_status;
    inBuffer >> dee_branch;
    }


  parallelBranchSub* makeParallelChild(int whichChild)
    {
      parDockingNode *temp = new parDockingNode;
      temp->initialize(this,whichChild);
      return temp;
    }

    void quickIncumbentHeuristic()
        { incumbentHeuristic(); }

private:

  /// Valid solutions.  These are represented in an UNMAPPED format
  parallelDocking* globalPtr;

};


inline parallelBranchSub* parallelDocking::blankParallelSub()
{
  parDockingNode *temp = new parDockingNode;
  temp->initialize(this);
  return temp;
}

inline bool parallelDocking::setup(int& argc,char**& argv)
{ 
  bool status = parallelBranching::setup(argc,argv);
  // parameters read from the command line in the branching::setup method
  if (status)
    {
      parDockingNode node;
      // The false says to not generate an incumbent.  Too early to do that.
      node.initialize(this);
      PackBuffer buf;
      node.pack(buf);
      _spPackSize = 10*buf.size();
    }
  return status;
}


#else

// If MPI is not present, define a dummy parallel class

 typedef void parallelDocking;

#endif



} // namespace pebbl

#endif
