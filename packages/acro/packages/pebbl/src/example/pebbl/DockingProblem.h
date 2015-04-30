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
 * \file DockingProblem.h
 * \author William Hart
 *
 * Defines the pebbl::DockingProblem class.
 */

#ifndef DockingProblem_h
#define DockingProblem_h

//
// Defines data structures needed for a peptide docking problem
//

#include <iostream>
#include <utilib/BasicArray.h>
#include <utilib/exception_mngr.h>
#include <utilib/CharString.h>
#include <utilib/sort.h>
#include <utilib/BitArray.h>
#include <utilib/_math.h>
#include <utilib/PackObject.h>



using namespace utilib;


///
/// Object that contains information about a particular peptide-peptide
/// interaction.  For each location pair, (I,J), an IntraInfo object is 
/// used to define the possible energies for rotamer assigments.
///
/// This object is tailored to order the rotamer-rotamer pairs by value,
/// which aids in optimization.
///
class IntraInfo :  public utilib::PackObject
{
public:

  /// Constructor
  IntraInfo() : n(0), N(0) {}

  /// Number of rotamer-rotamer pairs with nonzero energy
  int n;

  /// Number of feasible rotamer-rotamer pairs
  int N;

  /// r[order[i]] is the index of the (r,s) rotamer pair with the ith lowest
  /// energy
  BasicArray<unsigned int> r;
  BasicArray<unsigned int> R;

  /// s[order[i]] is the index of the (r,s) rotamer pair with the ith lowest
  /// energy
  BasicArray<unsigned int> s;
  BasicArray<unsigned int> S;

  /// Array that orders the the (r,s) pairs from lowest to highest
  BasicArray<int> order;

  /// Energies of rotamer-rotamer interactions
  BasicArray<double> e;
  BasicArray<double> E;

  /// e[index[i][j]] is the energy of the interaction between rotamer i and 
  /// rotamer j.  If index[i][j] equals -1, then this interaction is not
  /// feasible.
  BasicArray<BasicArray<int> > index;

  /// JE added this to improve sparse nested bound computation
  BasicArray<BasicArray<unsigned int> > suborder;

  /// Write this object to a buffer
  void write(PackBuffer& os) const
    { os << n << N << r << s << order << e << index << R << S << E; }

  /// Read this object from a buffer
  void read(UnPackBuffer& os)
    { os >> n >> N >> r >> s >> order >> e >> index >> R >> S >> E; }

};


/// Class that defines a peptide docking problem

class DockingProblem : public utilib::PackObject
{
public:

  /// Constructor
  DockingProblem()
    : type(0), n(0), Edefault(0.0), Etolerance(1e-7), ioPtr(NULL) {}

  /// Read in data from a file
  void read(const char* filename, bool rotamer_weighting_flag);

  /// Perform greedy local search
  double find_greedy(BasicArray<int>& x, 
		     int maxiters=1000, 
		     BasicArray<BitArray>* free_rotamers=0);

  /// Perform greedy local search
  double find_random_greedy(BasicArray<int>& x, 
			    Uniform& urnd, 
			    int maxiters=1000, 
			    double bias=0.75, 
			    BasicArray<BitArray>* free_rotamers=0);

  /// Compute the energy of a peptide configuration
  ///   x[i] is the id of the rotamer chosen at location i
  double energy(BasicArray<int>& x);

  /// Compute a bound on the minimum value of a peptide configuration
  ///   free_rotamers[i] is a binary array, where 0's indicate that a 
  ///   rotamer choice is invalid.

  double compute_bound(BasicArray<BitArray>& free_rotamers,
		       int bound_type,
		       bool sparse);

  double simple_bound_dense(BasicArray<BitArray>& free_rotamers);
  double simple_bound_sparse(BasicArray<BitArray>& free_rotamers);
  double nested_bound(BasicArray<BitArray>& free_rotamers,bool sparse);

  /// Note: this is currently ignored
  int type;

  /// The number of peptides in the docking problem.
  unsigned int n;

  /// rCount[i] is the number of rotamers at location i
  BasicArray<int> rCount;

  /// Energy between the peptide and the protein
  /// E_inter[i][j] is the energy added by the j-th rotamer at location i
  BasicArray<BasicArray<double> > E_inter;

  /// An array to reorder E_inter from lowest to highest
  /// inter_order[i] is an array used to re-order E_inter, so
  ///   E_inter[i][ inter_order[i][k] ] is the k-th lowest E_inter value for
  ///   location i
  BasicArray<BasicArray<int> > inter_order;

  /// An array to reorder E_inter, using a weighting scheme.
  /// By default, this array equals inter_order
  BasicArray<BasicArray<int> > weighted_inter_order;

  /// Energy within the peptide
  ///
  /// E_intra[i][j] contains an IntraInfo object that describes the
  /// feasible interactions between locations i and j.
  BasicArray<BasicArray<IntraInfo> > E_intra;

  /// Default inter energy, if no value is given
  double Edefault;

  /// Tolerance for the absolute value of energies.  Smaller values
  /// are treated as zero.
  double Etolerance;

  /// An array used to define the order in which dimensions are 
  /// searched during the greedy local search, from lowest to highest
  BasicArray<int> rorder;

  /// Evaluate a local search step
  double eval_move(BasicArray<int>& x, unsigned int i, int r, double ans);

  /// Used in a type of reweighted docking application
  Ereal<double> compute_min_rotamer(unsigned int i, unsigned int k,
					BasicArray<BitArray>& free_rotamers);

  /// Write this object to a buffer
  void write(PackBuffer& os) const
    { os << n << rCount << E_inter << inter_order << weighted_inter_order 
         << E_intra << Edefault << Etolerance << rorder; }

  /// Read this object from a buffer
  void read(UnPackBuffer& os)
    { 
      os >> n >> rCount >> E_inter >> inter_order >> weighted_inter_order 
         >> E_intra >> Edefault >> Etolerance >> rorder; 
      setupSparseBoundStructures();
    }

  CommonIO* ioPtr;

  REFER_DEBUG(ioPtr);

 protected:

  // Support routines added by JE

  // Improve a given solution; ans is assumed to be the current
  // energy; the default value triggers a recalculation

  double improve_solution(BasicArray<int>& x,
			  int maxiters,
			  BasicArray<BitArray>* free_rotamers,
			  double ans = -MAXDOUBLE);

  // Common exit cleanup from both regular and random greedy heuristics

  double greedy_cleanup(BasicArray<int>& x, Ereal<double> ans);

  /// Working arrays used by nested sparse bound computation
  /// Here to avoid overhead of repeated construction and destruction

  BasicArray<int> numFree;

  /// Routine to set up auxiliary data structures needed for the

  void setupSparseBoundStructures();
 
};



double DockingProblem::energy(BasicArray<int>& x)
{
  double ans=0.0;
  // JE: Merged the two main loops; should lower overhead infinitesimally
  for (unsigned int i=0; i<n; i++) {
    if (rCount[i] == 0) continue;
    ans += E_inter[i][ x[i] ];
    for (unsigned int j=i+1; j<n; j++) {
      if (rCount[j] == 0) continue;
      IntraInfo& info = E_intra[i][j];
      int k = info.index[ x[i] ][ x[j] ];
      if (k >= 0)
	ans += info.e[ k ];
    }
  }
  return ans;
}


// Main bound computation -- now switches between alternate methods
//
// The current 'solution' is represented as an array of bitarrays.  
// free_rotamers[i] is a bitarray, where 1's represent potential 
// rotamer choices and 0's represent rotamer choices that have been
// disallowed.

double DockingProblem::compute_bound(BasicArray<BitArray>& free_rotamers,
				     int bound_type, 
				     bool sparse)
{
  //
  // Misc error checking
  //
#ifdef ACRO_VALIDATING
  if (free_rotamers.size() != n)
    EXCEPTION_MNGR(runtime_error,"Bad list length for free_rotamers");
  for (unsigned int i=0; i<n; i++)
    if ((rCount[i] > 0) && (free_rotamers[i].nbits() == 0))
      EXCEPTION_MNGR(runtime_error, "No free bits for amino acid " << i);
#endif

  switch (bound_type) {
  case 1:
    if (sparse)
      return simple_bound_sparse(free_rotamers);
    else
      return simple_bound_dense(free_rotamers);
  case 2:
    return nested_bound(free_rotamers,sparse);
  default:
    EXCEPTION_MNGR(runtime_error,"Unknown bound computation type");
  }

  return 0.0;   // Just to avoid compiler warnings
}


// Bill's original bound method -- simpler bound, taking advantage of sparsity
//
// (Old, probably irrelevant) TODO: eliminate the num_feasible computation
//

double 
DockingProblem::simple_bound_sparse(BasicArray<BitArray>& free_rotamers)
{
double ans=0.0;
//
// The peptide-protein energy is bounded by simply selecting the rotamer
// with the lowest energy at each location.
//
// This calculation is optimized by re-ordering the rotamers at each location
// from lowest to highest.
// 
for (unsigned int i=0; i<n; i++) {
  if (rCount[i] == 0) continue;

  unsigned int k=0;
  BitArray& rotamers = free_rotamers[i];
  while (k<inter_order[i].size()) {
    DEBUGPR(70,ucout << i << " " << E_inter[i][ inter_order[i][k] ] << " " 
	    << inter_order[i][k] << std::endl);
    if (rotamers(inter_order[i][k]))
       break;
    k++;
    }
  DEBUGPR(70,ucout << i << " " << E_inter[i][ inter_order[i][k] ] 
	  << std::endl);

  ans += E_inter[i][ inter_order[i][k] ];
  }

//
// The peptide-peptide energies are bounded by summing over all
// location pairs, the minimum rotamer-rotamer energy over all 
// rotamer pairs.
//
for (unsigned int i=0; i<n; i++) {
  if (rCount[i] == 0) continue;
  for (unsigned int j=i+1; j<n; j++) {
    if (rCount[j] == 0) continue;

    //
    // For each i,j pair, find the (r,s) pair with lowest energy
    //
    IntraInfo& info = E_intra[i][j];
    int k=-1;
    unsigned int num_feasible=0;
    for (unsigned int kk=0; kk<info.order.size(); kk++) {
      if (free_rotamers[i]( info.r[ info.order[kk]] )  && 
          free_rotamers[j]( info.s[ info.order[kk]] ) ) {
         num_feasible++;
         if (k == -1) {
            k = kk;
            if (info.e[info.order[k]] < 0.0) break;
            }
         }
      }

    //
    // if num_feasible is zero, then the energy is zero.
    // if the energy is negative, then it is clearly the lowest value
    //    otherwise, there may be a zero valued interaction
    // but if all interactions have been counted then we can use the lowest
    //    value found.
    //
    if ((num_feasible > 0) && 
        ((info.e[info.order[k]] < 0.0) ||
         (num_feasible == free_rotamers[i].nbits()*free_rotamers[j].nbits()))) {
       ans += info.e[ info.order[k] ];
       DEBUGPR(70,ucout << ans << std::endl);
       DEBUGPR(70,ucout << i << " " << j << " " << info.e[info.order[k]] 
	       << " " << k << std::endl);
       }
    }
  }

return ans;
}

//
// A simpler way of implementing the previous bound computation
//

double DockingProblem::simple_bound_dense(BasicArray<BitArray>& free_rotamers)
{
double ans=0.0;

//
// The peptide-protein energy is bounded by simply selecting the rotamer
// with the lowest energy at each location.
//
// This calculation is optimized by re-ordering the rotamers at each location
// from lowest to highest.
// 
for (unsigned int i=0; i<n; i++) {
  if (rCount[i] == 0) continue;

  unsigned int k=0;
  BitArray& rotamers = free_rotamers[i];
  double emin=DBL_MAX;
  int n=rCount[i];
  bool flag=false;
  for (int j=0; j<n; j++) {
    if (rotamers(j) && (E_inter[i][j] < emin)) {
        emin=E_inter[i][j];
        flag=true;
        }
    }
  if (flag)
     ans += emin;
  else
     ucout << "HERE - error?" << std::endl;
  }

//
// The peptide-peptide energies are bounded by summing over all
// location pairs, the minimum rotamer-rotamer energy over all 
// rotamer pairs.
//
for (unsigned int i=0; i<n; i++) {
  if (rCount[i] == 0) continue;
  BitArray& i_rotamers = free_rotamers[i];
  for (unsigned int j=i+1; j<n; j++) {
    if (rCount[j] == 0) continue;
    IntraInfo& info = E_intra[i][j];
    BitArray& j_rotamers = free_rotamers[j];
    double emin=DBL_MAX;
    //ucout << "I " << i << " J " << j << std::endl;
    bool flag=false;
    unsigned int num_feasible=0;
    for (int k=0; k<info.E.size(); k++) {
        //ucout << "DBG " << k << " " << info.R[k] << " " << i_rotamers(info.R[k]) << std::endl;
        //ucout << "DBG " << k << " " << info.S[k] << " " << j_rotamers(info.S[k]) << std::endl;
        //ucout << "DBG " << k << " " << info.E[k] << " " << emin << std::endl;
        if (j_rotamers(info.S[k]) && i_rotamers(info.R[k])) {
            num_feasible++;
            if (info.E[k] < emin) {
                emin = info.E[k];
                flag=true;
                }
            }
        }
    if (flag && ((emin<0.0) || (num_feasible == j_rotamers.nbits()*i_rotamers.nbits())))
       ans += emin;
    //ucout << "ANS " << ans << std::endl;
  }
}

//ucout << "BOUND " << ans << " " << old_compute_bound(free_rotamers) << std::endl;
return ans;
}


//
// Tighter bound by Jonathan (implemented with Cindy)
// This version has the option to use sparsity if desired (usually)
//

double DockingProblem::nested_bound(BasicArray<BitArray>& free_rotamers,
				    bool sparse)
{
  if (sparse)
    {
      DEBUGPR(50,ucout << "Sparse nested bound\n");
      if (Edefault != 0)
	EXCEPTION_MNGR(runtime_error,"Nested sparse bound can only be used"
		       " if Edefault=0");
    for (unsigned int i=0; i<n; i++)
      if (rCount[i] > 0)
	numFree[i] = free_rotamers[i].nbits();
    }
  else
    DEBUGPR(50,ucout << "Dense nested bound\n");
  
  double ans=0.0;

  for (unsigned int i=0; i<n; i++) 
    {
      if (rCount[i] == 0) 
	continue;
      DEBUGPR(70,ucout << "i=" << i << '\n');

      BitArray& rotamers = free_rotamers[i];
      double emin=DBL_MAX;
      int nrotamers=rCount[i];
 
      for (int r=0; r<nrotamers; r++) 
	{
	  if (rotamers(r)) 
	    {
	      double innersum = E_inter[i][r];
	      DEBUGPR(70,ucout << "rotamer r=" << r << '\n');
	      DEBUGPR(70,ucout << "innersum=self-energy=" 
		      << innersum << '\n');
	      for (int j=i+1; j<n; j++) 
		{
		  int njrotamers = rCount[j];
		  if (njrotamers == 0) 
		    continue;
		  DEBUGPR(70,ucout << "j=" << j << '\n');
		  IntraInfo& info = E_intra[i][j];
		  BitArray& j_rotamers = free_rotamers[j];
		  double innermin=DBL_MAX;

		  if (sparse)
		    {
		      BasicArray<unsigned int>& suborder = info.suborder[r];
		      DEBUGPR(70,ucout << "suborder array: " << suborder 
			      << std::endl);
		      unsigned int numNonzero = suborder.size();
		      int numScanned = 0;
		      for(unsigned int kk=0; kk<numNonzero; kk++)
			{
			  unsigned int k = suborder[kk];
			  unsigned int s = info.s[k];
			  if (j_rotamers(s)) 
			    {
			      DEBUGPR(70,ucout << "Pair energy " << info.e[k] 
				      << '\n');
			      if (numScanned++ == 0)
				innermin = info.e[k];
			      DEBUGPR(70,ucout << "innermin=" 
				      << innermin << '\n');
			      if (innermin < 0.0)
				break;
			    }
			}
		      if ((innermin < 0) || (numScanned == numFree[j]))
			{
			  innersum += innermin;
			  DEBUGPR(70,ucout << "Added innermin=" << innermin 
				  << ", innersum=" << innersum << '\n');
			}
		    }
		  else  // dense
		    {
		      DEBUGPR(70,ucout << "info.index[r] = " 
			      << info.index[r] << '\n');
		      for (int s=0; s<njrotamers; s++) 
			{
			  if (j_rotamers(s)) 
			    {
			      DEBUGPR(70,ucout << "rotamer s=" << s << '\n'); 
			      int whichindex = info.index[r][s];
			      DEBUGPR(70,ucout << "whichindex=" << whichindex 
				      << '\n');
			      double pairEnergy = Edefault;
			      if (whichindex != -1) 
				pairEnergy = info.e[whichindex];
			      DEBUGPR(70,ucout << "pairEnergy=" << pairEnergy 
				      << '\n');
			      if (pairEnergy < innermin)
				{
				  innermin = pairEnergy;
				  DEBUGPR(70,ucout << "Lowest -- innermin now " 
					  << innermin << '\n');
				}
			    }
			}
		      innersum += innermin;
		      DEBUGPR(70,ucout << "innersum=" << innersum << '\n');
		    }
		}

	      if (innersum < emin)
		{
		  emin = innersum;
		  DEBUGPR(70,ucout << "Lowest (r) -- emin now " 
			  << emin << '\n');
		}

	    }
	}

      ans += emin;
      DEBUGPR(70,ucout << "emin=" << emin << " ans=" << ans << '\n');

    }

  DEBUGPR(50,ucout << "Bound=" << ans << ", old bound="   
	  << simple_bound_sparse(free_rotamers) << '\n');

  return ans;
}


double DockingProblem::improve_solution(BasicArray<int>& x,
					int maxiters,
					BasicArray<BitArray>* free_rotamers,
					double ans)  // current energy
{
  if (ans == -MAXDOUBLE)
    ans = energy(x);
  for (int iter=0; iter<maxiters; iter++) {
    double tans = ans;
    //
    // Iterate over all locations
    //
    for (unsigned int i_=0; i_<n; i_++) {
      if (rCount[i_] == 0) continue;
      int i = rorder[i_];
      //
      //  Iterate through all rotamers at location i
      //
      int    curr_r   = x[i];
      double curr_ans = ans;
      for (int r_=0; r_<rCount[i]; r_++) {
	if (r_ == x[i]) 
	  continue;
	int r = weighted_inter_order[i][r_];
	if (free_rotamers && (!((*free_rotamers)[i](r))) ) 
	  continue;
      //
      // Evaluate move 'r', and keep it if it better
      //
      double tval = eval_move(x, i, r, ans);
      if (tval < curr_ans) {
         curr_r   = r_;
         curr_ans = tval;
         }
      }
    //
    // If an improving point is found, update the current 
    // solution.
    //
    if (curr_ans < ans) {
       ans = curr_ans;
       x[i] = curr_r;
       DEBUGPR(20, ucout << "NEW POINT - Iteration: " << iter 
	       << " Value: " << ans << " Solution: ";
       for (unsigned int jj=0; jj<x.size(); jj++) 
         if (x[jj] < 0)
            ucout << ". ";
         else
            ucout << (x[jj]+1) << " ";
	       ucout << std::endl);
       }
    }

  //
  // Give up if no improving solution was found on this pass
  //
    if (tans == ans)
     return ans;
  }

  return ans;
}


//
// This does a simple greedy local search
// JE modified to include feasibility.  Currently, just gives up
// if it can't get feasible.
//
double DockingProblem::find_greedy(BasicArray<int>& x, 
				   int maxiters, 
				   BasicArray<BitArray>* free_rotamers)
{
  DEBUGPR(20, ucout << std::endl << "Starting Greedy Heuristic" << std::endl);

  //
  // We use 'internal coordinates' until the end, when we translate
  // using the inter_order arrays.  The 'init' array is a temporary array
  // used to call the energy() function.
  //

  BasicArray<int> init(n);

  for (unsigned int i=0; i<n; i++) {
    if (rCount[i] == 0) {
      x[i] = -1;
      init[i] = -1;
    }
    else 
      {
	if (free_rotamers == 0) {
	  x[i] = 0;
	  init[i] = weighted_inter_order[i][x[i]];
	} else {
	  unsigned int k=0;
	  BitArray& rotamers = (*free_rotamers)[i];
	  while (k < rotamers.size()) {
            if (rotamers(weighted_inter_order[i][k])) {
	      x[i] = k;
	      init[i] = weighted_inter_order[i][k];
	    }
            k++;
	  }
        }
      }
  }

  Ereal<double> ans = energy(init);
  DEBUGPR(20, ucout << "Initial Value: " << ans << std::endl);

  ans = improve_solution(x,maxiters,free_rotamers,ans);

  return greedy_cleanup(x,ans);
}



//  Added by JE -- common exit from both greedy algorithms.

double DockingProblem::greedy_cleanup(BasicArray<int>& x, Ereal<double> ans)
{
  DEBUGPR(4, ucout << "Greedy heuristic final Value: " << ans << std::endl);

  for (unsigned int i=0; i<n; i++)
    if (rCount[i] > 0) 
      x[i] = weighted_inter_order[i][x[i]];

  DEBUGPR(20, ucout << "Natural rotamer numbering solution: " 
	  << x << std::endl);

#ifdef ACRO_VALIDATING
  double confirmE = energy(x);
  DEBUGPR(20, ucout << "Confirm Energy = " << confirmE << std::endl);
  if (abs(confirmE - ans) > 1e-5)
    EXCEPTION_MNGR(runtime_error,
		   "Energy mismatch after improveSolution");
#endif

  return ans;
}


//
// This does a randomized greedy local search
//
double DockingProblem::find_random_greedy(BasicArray<int>& x, 
					  Uniform& urnd, 
					  int maxiters, 
					  double bias, 
					  BasicArray<BitArray>* free_rotamers)
{
  DEBUGPR(20, ucout << std::endl << "Starting Randomized Greedy Heuristic" 
	  << std::endl);
  //
  // We use 'internal coordinates' until the end, when we translate
  // using the inter_order arrays.  The 'init' array is a temporary array
  // used to call the energy() function.
  //

  BasicArray<int> init(n);

  for (unsigned int i=0; i<n; i++) {
    if (rCount[i] == 0) {
      x[i] = -1;
      init[i] = -1;
    }
    else 
      {
	BasicArray<double> proby(rCount[i]);
	proby << 0.0;
	double tmp=1.0;
	double total=0.0;
	for (int j=0; j<rCount[i]; j++) {
	  if (free_rotamers && 
	      (!((*free_rotamers)[i](weighted_inter_order[i][j]))) ) 
	    continue;
	  tmp *= bias;
	  proby[j] = tmp;
	  total += tmp;
	}
	for (int j=0; j<rCount[i]; j++) {
	  proby[j] /= total;
	}
	double rval = urnd();
	int j = 0;
	while (1) {
	  if (rval <= proby[j]) {
	    x[i] = j;
	    break;
	  }
	  rval -= proby[j];
	  j++;
	  if (j==rCount[i])
	    j=rCount[i-1];
	}
	init[i] = weighted_inter_order[i][x[i]];
      }
  }

 Ereal<double> ans = energy(init);
 DEBUGPR(20, ucout << "Initial Value: " << ans << std::endl);

#ifdef ACRO_VALIDATING
  if (ans == MAXDOUBLE)
    EXCEPTION_MNGR(runtime_error,
		   "Heuristic first pass did not detect infeasibility");
#endif

  ans = improve_solution(x,maxiters,free_rotamers,ans);

  return greedy_cleanup(x,ans);
}


//
// Evaluate the value of moving location i to rotamer r
//
double DockingProblem::eval_move(BasicArray<int>& x, 
				 unsigned int i, 
				 int r, 
				 double ans)
{
  double val = ans;
  int ndx = weighted_inter_order[i][x[i]];

  val -= E_inter[i][ ndx ];
  for (unsigned int j=0; j<n; j++) {
    if (rCount[j] == 0) continue;
    if (j == i) continue;
    IntraInfo* info = NULL;
    int k = -1;
    if (i < j) {
      info = &(E_intra[i][j]);
      k = info->index[ ndx ][ weighted_inter_order[j][x[j]] ];
    }
    else if (i > j) {
      info = &(E_intra[j][i]);
      k = info->index[ weighted_inter_order[j][x[j]] ][ ndx ];
    }
    if (k == -1)
      val -= Edefault;
    else
      val -= info->e[ k ];
  }

  val += E_inter[i][ r ];
  for (unsigned int j=0; j<n; j++) {
    if (rCount[j] == 0) continue;
    if (j == i) continue;
    IntraInfo* info = NULL;
    int k = -1;
    if (i < j) {
      info = &(E_intra[i][j]);
      k = info->index[ r ][ weighted_inter_order[j][x[j]] ];
    }
    else if (i > j) {
      info = &(E_intra[j][i]);
      k = info->index[ weighted_inter_order[j][x[j]] ][ r ];
    }
    if (k == -1)
      val += Edefault;
    else
      val += info->e[ k ];
  }

  return val;
}


void DockingProblem::read(const char* filename, 
			  bool advanced_rotamer_weighting)
{
std::ifstream is(filename);
if (!is) EXCEPTION_MNGR(runtime_error, "Bad filename \"" <<
                        filename << "\"");

int state=0;

CharString tmp;
int ctr1=0,ctr2=0;
int pctr1=0,pctr2=0;
int maxrotamers=-1;

while (1) {
  is >> tmp;
  if (!is) break;

  switch (state) {
	case 0:
		{
		if (tmp != "param:") break;
		is >> tmp;
		if (tmp != "ValidInterIndices:") 
		  EXCEPTION_MNGR(runtime_error,"Badly formated input - 0");
		is >> tmp;
		is >> tmp;
		is >> tmp;
		int nsites=-1;
		while (tmp != ";") {
		  if (std::atoi(tmp.data()) > nsites)
			  nsites = std::atoi(tmp.data());
		  is >> tmp;
		  if (std::atoi(tmp.data()) > maxrotamers)
			  maxrotamers = std::atoi(tmp.data());
		  is >> tmp;
		  is >> tmp;
		}
		n = nsites;
		is.close();
		is.open(filename);
		state = 1;
		}
		break;

	case 1:
		{
		if (tmp != "param:") break;
		is >> tmp;
		if (tmp != "ValidInterIndices:") 
		  EXCEPTION_MNGR(runtime_error,"Badly formated input - 1");
		is >> tmp;
		//ucout << tmp << std::endl;
		is >> tmp;
		//ucout << tmp << std::endl;
		rCount.resize(n);
		rCount << 0;
		E_inter.resize(n);
		for (unsigned int i=0; i<n; i++) {
		  E_inter[i].resize(maxrotamers);
		  E_inter[i] << 0.0;
		}
		ctr1=0;
		state = 2;
		}
		break;

	case 2:
		{	
		if (tmp == ";") {
		   rCount[pctr1-1] = pctr2;
		   E_inter[pctr1-1].resize(pctr2);
		   state = 3;
		   break;
		   }
		//ucout << tmp << std::endl;
		ctr1 = std::atoi(tmp.data());
		is >> ctr2;
		is >> E_inter[ctr1-1][ctr2-1];
#if 0
		E_inter[ctr1-1][ctr2-1] = floor(E_inter[ctr1-1][ctr2-1]);
#endif
		if ((pctr1 != 0) && (ctr1 != pctr1)) {
		   rCount[pctr1-1] = pctr2;
		   E_inter[pctr1-1].resize(pctr2);
		}
		pctr1=ctr1;
		pctr2=ctr2;
		}
		break;

	case 3:
		{
		if (tmp != "param:") break;
		is >> tmp;
		if (tmp != "ValidIntraIndices:") EXCEPTION_MNGR(runtime_error,"Badly formated input - 3");
		is >> tmp;
		is >> tmp;
		E_intra.resize(n);
		for (unsigned int i=0; i<n; i++) {
		  if (rCount[i] == 0) continue;
		  E_intra[i].resize(n);
		  for (unsigned int j=0; j<n; j++) {
		    if (rCount[j] == 0) continue;
		    if (i < j) {
		       E_intra[i][j].r.resize(rCount[i]*rCount[j]);
		       E_intra[i][j].s.resize(rCount[i]*rCount[j]);
		       E_intra[i][j].e.resize(rCount[i]*rCount[j]);
		       E_intra[i][j].R.resize(rCount[i]*rCount[j]);
		       E_intra[i][j].S.resize(rCount[i]*rCount[j]);
		       E_intra[i][j].E.resize(rCount[i]*rCount[j]);
		       }
   		    }
		  }
		state = 4;
		}
		break;

	case 4:
		{	
		if (tmp == ";") {
		   state = 5;
		   break;
		   }
		ctr1 = std::atoi(tmp.data());
		is >> tmp;
		is >> ctr2;
        int i = E_intra[ctr1-1][ctr2-1].n;
        int j = E_intra[ctr1-1][ctr2-1].N;
        E_intra[ctr1-1][ctr2-1].r[i] = std::atoi(tmp.data())-1;
        E_intra[ctr1-1][ctr2-1].R[j] = std::atoi(tmp.data())-1;
        is >> tmp;
        E_intra[ctr1-1][ctr2-1].s[i] = std::atoi(tmp.data())-1;
        E_intra[ctr1-1][ctr2-1].S[j] = std::atoi(tmp.data())-1;
        is >> E_intra[ctr1-1][ctr2-1].e[i];
        if ((E_intra[ctr1-1][ctr2-1].e[i] < -Etolerance) ||
            (E_intra[ctr1-1][ctr2-1].e[i] >  Etolerance)) {
           E_intra[ctr1-1][ctr2-1].n++;
           E_intra[ctr1-1][ctr2-1].E[j] = E_intra[ctr1-1][ctr2-1].e[i];
        } else {
           E_intra[ctr1-1][ctr2-1].E[j] = 0.0;
        }
	E_intra[ctr1-1][ctr2-1].N++;
		/*ucout << ctr1 << " " << ctr2 << " "
			<< E_intra[ctr1-1][ctr2-1].r[i] << " "
			<< E_intra[ctr1-1][ctr2-1].s[i] << std::endl;*/
		}
		break;

	case 5:
		break;
    };
  }

BasicArray<BitArray> free_rotamers(n);
for (unsigned int i=0; i<n; i++) {
  free_rotamers[i].resize(rCount[i]);
  free_rotamers[i].set();
  }

//ucout << "rCount " << rCount << std::endl;
inter_order.resize(n);
weighted_inter_order.resize(n);
for (unsigned int i=0; i<n; i++) {
  if (rCount[i] == 0) continue;
  for (unsigned int j=i+1; j<n; j++) {
    if (rCount[j] == 0) continue;
       IntraInfo& info = E_intra[i][j];
       info.index.resize(rCount[i]);
       for (int k=0; k<rCount[i]; k++) {
         info.index[k].resize(rCount[j]);
         info.index[k] << -1;
         }
       for (int k=0; k<E_intra[i][j].n; k++) {
         info.index[ info.r[k] ][ info.s[k] ] = k;
         }
       info.R.resize( info.N );
       info.S.resize( info.N );
       info.E.resize( info.N );
       info.r.resize( info.n );
       info.s.resize( info.n );
       info.e.resize( info.n );
       info.order.resize( info.n );
       order(info.order, info.e);
       //ucout << std::endl;
       //ucout << i << " " << j << std::endl;
       //ucout << info.e << std::endl;
       //ucout << info.order << std::endl;
    }
  inter_order[i].resize(rCount[i]);
  weighted_inter_order[i].resize(rCount[i]);
  BasicArray<double> E_inter_tmp(rCount[i]);
  order(inter_order[i], E_inter[i]);
  if (advanced_rotamer_weighting) {
     double tmpn = static_cast<double>(rCount[i]);
     for (int k=0; k<rCount[i]; k++) {
       E_inter_tmp[k] = E_inter[i][k]/tmpn + compute_min_rotamer(i,k,free_rotamers);
       }
     order(weighted_inter_order[i], E_inter_tmp);
     }
  else
     weighted_inter_order[i] << inter_order[i];
  }

rorder.resize(n);
order(rorder,rCount);

setupSparseBoundStructures();

DEBUGPR(4, ucout << "Docking Problem Summary" << std::endl;
ucout << "Num Locations: " << rCount.size() << std::endl;
ucout << "Num Assignment Choices: ";
  for (size_t i=0; i<rCount.size(); i++)
    ucout << rCount[i] << " ";
	ucout << std::endl);
}


// Code fragment to set up auxiliary data structures needed for
// the sparse nested bound computation.

void DockingProblem::setupSparseBoundStructures()
{
  numFree.resize(n);

  // int maxRCount = 0;
  // for (unsigned int i=0; i<n; i++)
  //   if (rCount[i] > maxRCount)
  //     maxRCount = rCount[i];

  // sum.resize(maxRCount);
  // numScanned.resize(maxRCount);
  // firstFound.resize(maxRCount);

  for(unsigned int i=0; i<n; i++)
    if (rCount[i] > 0)
      for(unsigned int j=0; j<n; j++)
	if (rCount[j] > 0)
	  {
	    IntraInfo& info = E_intra[i][j];
	    info.suborder.resize(rCount[i]);
	    BasicArray<int> counter(rCount[i]);
	    counter << 0;
	    for(unsigned int r=0; r<rCount[i]; r++)
	      info.suborder[r].resize(rCount[j]);
	    for(unsigned int kk=0; kk<info.e.size(); kk++)
	      {
		unsigned int k = info.order[kk];
		unsigned int r = info.r[k];
		info.suborder[r][counter[r]++] = k;
	      }
	    for(unsigned int r=0; r<rCount[i]; r++)
	      info.suborder[r].resize(counter[r]);
	  }
}


//
// TODO: document this later
// This isn't being used right now.
//
Ereal<double> DockingProblem::compute_min_rotamer(unsigned int i, 
						  unsigned int k,
					   BasicArray<BitArray>& free_rotamers)
{
double bound_val=0.0;
double tmpn = (double)n;

for (unsigned int j=0; j<n; j++) {
  if (rCount[j] == 0) continue;
  if (j == i) continue;

  IntraInfo* info;
  if (i<j) info = &E_intra[i][j];
  else     info = &E_intra[j][i];

  bool first=true;
  Ereal<double> minval=Ereal<double>::positive_infinity;
  for (unsigned int kk=0; kk<info->order.size(); kk++) {
    if ((i < j) && free_rotamers[j](info->s[kk]) && (info->r[kk]==k))
       if (first || (info->e[kk]/2.0 + E_inter[j][info->s[kk]]/tmpn < minval)) {
	  minval = info->e[kk]/2.0 + E_inter[j][info->s[kk]]/tmpn;
#ifdef DEBUG_BOUND
	  ucout << i << " " << j << " " <<minval << std::endl;
#endif
	  first=false;
          }
    if ((i > j) && free_rotamers[j](info->r[kk]) && (info->s[kk]==k))
       if (first || (info->e[kk]/2.0 + E_inter[j][info->r[kk]]/tmpn < minval)) {
	  minval = info->e[kk]/2.0 + E_inter[j][info->r[kk]]/tmpn;
#ifdef DEBUG_BOUND
	  ucout << i << " " << j << " " <<minval << std::endl;
#endif
	  first=false;
          }
    }
  if (first) return Ereal<double>::positive_infinity;
  bound_val += minval;
  }

return bound_val;
}


#endif
