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
 * \file selection
 *
 * Defines the scolib::selection class.
 */

#ifndef scolib_selection_h
#define scolib_selection_h

#include <acro_config.h>
#include <utilib/sort.h>
#include <utilib/_math.h>
#include <utilib/CommonIO.h>
#include <utilib/Uniform.h>
#include <utilib/default_rng.h>
#include <colin/real.h>

namespace scolib {

using utilib::BasicArray;
using utilib::NumArray;
using colin::real;

/** A class that can be used to perform a competitive selection with a variety
of different competition methods as well as stochastic selection methods.
*/
template <class ObjectT, class AccessorT, class CompareT>
class selection : virtual public utilib::CommonIO {

public:

  /// Constructor.
  selection(utilib::PropertyDict& properties);

  /// Setup random number generator.
  template <class RNGT>
  void set_rng(RNGT* rng_)
		{urnd.generator(rng_);}

  /// Reset this object.
  virtual void reset();

  /// Generate a probability vector with a vector of function evaluations.
  virtual void initialize(BasicArray<ObjectT>& Evals, AccessorT accessor, 
			CompareT compare);

  /// Return the array of expected selection likelihoods.
  BasicArray<real >& Expectations()
		{return Exp;}

  /// Perform selection, returning an array of selected items.
  void perform_selection(BasicArray<int>& map, const int Nsubset=-1);

  /// The value of the 'worst' item selected.
  double worst_factor;

  /// Debugging flag.
  bool Debug_flush;

protected:

  ///
  real New_worst(real Worst);

  ///
  void exec_sus(BasicArray<int>& map, int Nsubset);

  ///
  void exec_boltzmann_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor);

  ///
  void exec_rank_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor, CompareT& compare);

  ///
  void exec_pb_tournament_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor);

  ///
  void exec_proportional_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor, CompareT& compare);

  ///
  enum selection_type {Linear_Rank_Selection, PB_Tournament_Selection,
				Proportional_Selection, Boltzmann_Selection};

  ///
  utilib::Uniform urnd;

  ///
  real worst_const;

  ///
  double sigma_factor;

  ///
  int  ctr;

  ///
  int window_size;

  ///
  bool first_time;

  ///
  BasicArray<int> order_vec;

  ///
  BasicArray<int> rank_vec;

  ///
  NumArray<real > Exp;

  ///
  int	N;

  ///
  enum selection_mechanism { RWS, SRS, SUS };

  ///
  selection_mechanism smech;

  ///
  std::string smech_str;

  ///
  selection_type stype;

  ///
  std::string stype_str;

  ///
  double boltzmann_T;

  ///
  double K;

  ///
  double rank_C;

};


template <class ObjectT, class AccessorT, class CompareT>
selection<ObjectT,AccessorT,CompareT>::
selection(utilib::PropertyDict& properties) 
{
   smech_str = "sus";
   properties.declare
      ( "selection_mechanism", 
        "Specifies how random sampling is performed for selection:\n"
        "   rws - roulette wheel selection\n"
        "   srs - stochastic remainder selection\n"
        "   sus - stochastic universal selection",
        utilib::Privileged_Property(smech_str) );
   //option.add("selection_mechanism",smech_str,
   //           "Specifies how random sampling is performed for selection:\n \
   //   rws - roulette wheel selection\n                             \
   //   srs - stochastic remainder selection\n                       \
   //   sus - stochastic universal selection");
   //option.categorize("selection_mechanism", "Selection/Replacement");

   stype_str = "proportional";
   properties.declare
      ( "selection_type", 
        "Specifies the type of selection that will be performed:\n"
        "   proportional  - select points in proportion to their value "
        "from a specified baseline statistic.\n"
        "   linear_rank   - linear rank selection\n"
        "   pb_tournament - TODO\n"
        "   boltzmann     - TODO",
        utilib::Privileged_Property(stype_str) );
   //option.add("selection_type",stype_str,
   //           "Specifies the type of selection that will be performed:\n \
   //    proportional  - select points in proportion to their value from\n \
   //        a specified baseline statistic.\n                       \
   //    linear_rank   - linear rank selection\n                     \
   //    pb_tournament - TODO\n                                      \
   //    boltzmann     - TODO");
   //option.categorize("selection_type", "Selection/Replacement");

   boltzmann_T=1.0;
   K=2.0;
   worst_factor = -1;
   debug=0;
   Debug_flush=1;
   ctr=0;
   sigma_factor=1.0;
}


template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::reset()
{
if (smech_str == "rws") smech = RWS;
else if (smech_str == "srs") smech = SRS;
else if (smech_str == "sus") smech = SUS;
else
   EXCEPTION_MNGR(std::runtime_error,"selection::reset - bad selection mechanism: " << smech_str);

if (stype_str == "proportional")
   stype = Proportional_Selection;
else if (stype_str == "linear_rank")
   stype = Linear_Rank_Selection;
else if (stype_str == "pb_tournament")
   stype = PB_Tournament_Selection; 
else if (stype_str == "boltzmann")
   stype = Boltzmann_Selection;
else
   EXCEPTION_MNGR(std::runtime_error,"selection::reset - bad selection type: " << stype_str);

if (K < 0)
   rank_C = -K;
else
   rank_C=2*K/(1.0 + K);
}


template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::initialize(BasicArray<ObjectT>& Eval, AccessorT accessor, CompareT compare)
{
Exp.resize(Eval.size());
N = Exp.size();

switch (stype) {
  case Proportional_Selection:
	exec_proportional_selection(Eval, worst_const, accessor, compare);
	break;
  case Boltzmann_Selection:
	exec_boltzmann_selection(Eval, worst_const, accessor);
	break;
  case Linear_Rank_Selection:
	exec_rank_selection(Eval, worst_const, accessor, compare);
	break;
  default:
	EXCEPTION_MNGR(std::runtime_error, "selection::initialize : Undefined selection type.");
  }
}


template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::perform_selection(BasicArray<int>& map, const int Nsubset_)
{
int Nsubset = (Nsubset_==-1? map.size() : Nsubset_);

switch (smech) {
  case SUS:
	exec_sus(map,Nsubset);
	break;

  case RWS:
  case SRS:
	EXCEPTION_MNGR(std::runtime_error,"selection::initialize : Undefined selection mechanism.");
	break;

  default:
	break;
  }
}


template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::exec_sus(BasicArray<int>& map, int Nsubset)
{
unsigned int i,k;
double ptr;
real total=0.0;

ptr = urnd();
for (k = i = 0; i<Exp.size(); i++)
  for (total += (Nsubset*Exp[i]); total > ptr; ptr++)
    map[k++] = i;
if ((int)k != Nsubset) {
   ucout << "EXP " << Exp << "\n";
   ucout << "MAP " << map << "\n";
   ucout << "K " << k << " Nsubset " << Nsubset << "\n";
   EXCEPTION_MNGR(std::runtime_error,"selection::exec_sus : Select: internal scaling error");
   }
}


template <class ObjectT, class AccessorT, class CompareT>
real selection<ObjectT,AccessorT,CompareT>::New_worst(real Worst)
{
if (Worst == 0.0) return(1e-7);

if (Worst > 0.0) return(Worst*(1.0 + 1e-7));

return(Worst*(1.0 - 1e-7));
}


/*
 * Proportional Selection
 *
 *
 *  We want to perform minimization on a function.  To do so, we
 *	take fitness values in a given generation and normalize them s.t.
 *	they are non-negative, and such that smaller f's are given a higher
 *	weighting.
 *  If f in [a,b], then f' in [A,B] s.t. f(a) => f'(B) and f(b) => f'(A) is
 *
 *	f' = (B-A) * (1 - (f-a)/(b-a)) + A
 *
 *	   = (B-A) * (b-f)/(b-a) + A
 *
 *  Note that it suffices to map f into [0,1], giving
 *
 *	f' = (b-f)/(b-a)
 *
 *  Now the expected number of samples (out of N) generated from a given 
 *  point is
 *
 *	N * f'_i / \sum f'_i  =  N * ((b-f_i)/(b-a)) / \sum ((b-f_i)/(b-a))
 *
 *			      =  N * (b-f_i) / (N*b - \sum f_i)
 *
 *  Within a given generation, let 'b' be the maximal (worst) individual and 
 *	let 'a' be the minimal individual. 
 *
 *  Note:
 *	(1) This calculation is invariant to the value of B, but _not_ 
 *		invariant to the value of A.
 *	(2) This selection strategy works fine for functions bounded above,
 *		but it won't necessarily work for functions which are unbounded
 *		above.
 *
 *  The 'b' parameter is represented as 'Worst' and is selected by the 
 *    scale_fitness method.  If a value is greater than Worst, it is given a 
 *    value of zero for it's expectation.
 */
template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::exec_proportional_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor, CompareT& compare)
{
   OUTPUTPR(2, ucout << "Worst: " << Worst << "\n";
            ucout << "Eval-Vec: ";
            for (unsigned int i=0; i<Eval.size(); i++)
               ucout << " " << accessor(Eval[i]);
            ucout << "\n");

if (worst_factor >= 0.0) {
   Worst = (1 + worst_factor)*accessor(max(Eval,compare)) - worst_factor*accessor(min(Eval,compare));
   for (unsigned int i=0; i<Eval.size(); i++)
     Exp[i] = accessor(Eval[i]);
   Exp -= Worst;
   Exp *= 1.0/sum(Exp);
   }

else {
   bool flag=false;
   for (int i = 0; i<N ; i++)
       if (accessor(Eval[i]) < Worst) {
	  Exp[i] = Worst - accessor(Eval[i]);
          flag = true;
          }
       else {
	  Exp[i] = 0.0;
 	  }
   if (flag == false)  			// All of the samples were >= Worst
      Exp << static_cast<real>(1.0/(double)N);
   else
      Exp *= 1.0/sum(Exp);
   }

OUTPUTPR(2, ucout << "Proportional Exp-Vec: " << Exp << "\n");
OUTPUTPR(2, ucout << "Proportional Total Exp: " << sum(Exp) 
         << " N: " << N << "\n");
}


/* Boltzman Selection
 *
 *
 *  We want to perform minimization on a function.  To do so, we
 *	take fitness values in a given generation and normalize them 
 *	such that lower f's are given a higher weighting.
 *  Following de la Maza and Tido (1991), we use a Boltmann normalized
 *	fitness:
 *
 * 	f' = e^{f/T}
 *
 *  Now the expected number of samples (out of N) generated from a given 
 *  point is
 *
 *	N * f'_i / \sum f'_i  =  N * e^{f_i / T} / \sum_{f_j < b} e^{f_j / T}
 *
 *  Within a given generation, let 'b' be the maximal (worst) individual and 
 *	let 'a' be the minimal individual. 
 *
 *  The 'b' parameter is represented as 'Worst'.  If a value is greater 
 *	than Worst, it is given a value of zero for it's expectation.
 */
template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::exec_boltzmann_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor)
{
if (worst_factor >= 0.0) {
   for (int i=0; i<N; i++)
     Exp[i] = ::exp(- accessor(Eval[i])/boltzmann_T );
   Exp *= 1.0 / sum(Exp);
   }

else {
   for (int i=0; i<N; i++) {
     if (accessor(Eval[i]) < Worst)
        Exp[i] = ::exp(- accessor(Eval[i])/boltzmann_T );
     else
        Exp[i] = 0.0;
     }
   Exp *= 1.0 / sum(Exp);
   }
}


/* Rank Selection
 *
 *
 *  We want to perform minimization on a function.  To do so, we
 *	take fitness values in a given generation and normalize them 
 *	such that lower f's are given a higher weighting.  To perform
 *	ranked selection, we use the rank information from the population
 *	to determine the fitness.  We assume that the rank of the
 *	ith individual is r_i s.t. the best individual has r_i=0 and the
 *	worst has r_i = 1.
 *
 *  Linear ranking maps the ranks into proportions which vary linearly.
 *	Goldberg and Deb (1991) show that this linear function must have the
 *	form
 *
 *	a(r_i) = C - 2(C-1) r_i		, 2 >= C >= 1
 *
 *	The expected number of samples (out of N) generated by rank r_i 
 *      is a(r_i).
 *
 * 	We can parameterize this ranking with K, the relative probability
 *	between the best and worst individual.  Since K = C/(2-C),
 *	C = 2K/(1+K).  If infty > K >= 1, then 2 > C >= 1.
 */
template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::exec_rank_selection(BasicArray<ObjectT>& Eval, double Worst, AccessorT& accessor, CompareT& compare)
{
BasicArray<int> rank_vec;
utilib::rank(rank_vec, Eval, compare);
OUTPUTPR(2, ucout << "Rank-Vec: " << rank_vec << "\n");
OUTPUTPR(2, ucout << "worst_factor: " << worst_factor << "\n");

if (worst_factor < 0.0) {
   for (int i=0; i<N; i++)
     Exp[i] =  (rank_C - 2*(rank_C-1)* rank_vec[i]/((double) (N-1.0))) / N;
   }

else {
   // Because we are throwing away some of the orders, we need to perform a
   //	slightly different remapping.  If J is the number of orders
   //	which are ignored, we do
   //
   //	r_i = o_i/((N-1) - J)		, o_i < N - J
   int i,j;
   for (i = j = 0; i<N ; i++)
       if (accessor(Eval[i]) >= Worst)
          j++;
   for (i=0; i<N; i++)
     if (rank_vec[i] >= (N-j))
        Exp[i] = 0.0;
     else {
	Exp[i] = (rank_C - 2*(rank_C-1)*
			rank_vec[i]/((double) (N - 1.0 - j)) ) / N;
        }
   }

OUTPUTPR(2, ucout << "Linear Rank Exp-Vec: " << Exp << "\n");
OUTPUTPR(2, ucout << "Linear Rank Total Exp: " << sum(Exp) 
         << " N: " << N << "\n");
}


/* Probabilistic Binary Tournament Selection
 *
 * This type of tournament selection was described in Goldberg and Deb (1991),
 * and performs the same type of selection as is seen in linear rank
 * selection.  Two individuals are chosen at random, and the better individual
 * is selected with probability P, 0.5 <= P <= 1.  If we let 2P = C, then
 * we see that the expected number of samples generated by rank r_i is
 *
 *      N * [2P - 2(2P-1) r_i]            , 1 >= P >= 0.5
 *
 * We can again parameterize this ranking with K, the relative probability
 * between the best and worst individual.  Since 2P = C, 
 * P = K/(1+K).
 */
template <class ObjectT, class AccessorT, class CompareT>
void selection<ObjectT,AccessorT,CompareT>::exec_pb_tournament_selection(BasicArray<ObjectT>& /*Eval*/, double /*Worst*/, AccessorT& )
{ }

} // namespace scolib

#endif
