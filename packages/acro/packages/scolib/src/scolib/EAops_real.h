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

//
// EAops_real.h
//

#if 0
/**
 * \file FOO 
 */

#ifndef scolib_EAops_real_h
#define scolib_EAops_real_h

#include <acro_config.h>
#include <utilib/NumArray.h>
#include <utilib/DoubleMatrix.h>
#include <utilib/Uniform.h>
#include <utilib/Normal.h>
#include <utilib/Cauchy.h>
#include <utilib/CharString.h>
#include <scolib/EAops.h>
#include <scolib/EApoint.h>

namespace scolib {

using utilib::NumArray;

class EApoint_real : public EApoint<NumArray<double> >
{
public:

  ///
  EApoint_real() 
	: ls_scale(1.0), epsa_flag(false) {}

  ///
  bool operator==(const EApoint_real& pt) const
                {return (point == pt.point);}

  /// A scale factor for mutation that can be self-adapted
  NumArray<double> mutation_scale;

  /// A scale factor for the initial local search step
  double ls_scale;

  /// Flag used by EPSAs
  bool epsa_flag;

  ///
  EApoint_real& operator=(const EApoint_real& pt)
                {
		mutation_scale.resize(pt.mutation_scale.size());
   		mutation_scale << pt.mutation_scale;
   		ls_scale = pt.ls_scale;
   		epsa_flag = pt.epsa_flag;
                point = pt.point;
                return *this;
                }

  ///
  void copy(const EApoint_real& pt)
                {
		mutation_scale.resize(pt.mutation_scale.size());
   		mutation_scale << pt.mutation_scale;
   		ls_scale = pt.ls_scale;
   		epsa_flag = pt.epsa_flag;
		point << pt.point;
		}

  ///
  void write_point(std::ostream& os) const
                {os << point;}

  ///
  void write(std::ostream& os) const
                {
		os << "MScale: " << mutation_scale << " LScale: " <<
              		ls_scale << " EPSA: " << epsa_flag;
		}

  ///
  void read(std::istream& is)
                {is >> point >> mutation_scale >> ls_scale >> epsa_flag;}

  ///
  void ls_info(std::ostream&) const {}

  ///
  void write(PackBuffer& pack) const
                {pack << point << mutation_scale << ls_scale << epsa_flag;}

  ///
  void read(UnPackBuffer& unpack)
                {unpack >> point >> mutation_scale >> ls_scale >> epsa_flag;}

protected:
 
  ///
  void copy(EApoint<NumArray<double> >& pt)
		{ EApoint<NumArray<double> >::copy(pt); }

  ///
  EApoint<NumArray<double> >& operator=(const EApoint<NumArray<double> >& pt)
		{ return EApoint<NumArray<double> >::operator=(pt); }

};



/** Class that defines operations used with NumArray<double> genotypes.
*/
class EAops_real : public EAops<EApoint_real, NumArray<double> >
{
public:

  /**@name General Information */
  //@{
  ///
  EAops_real();

  ///
  void write(std::ostream& os) const;

  ///
  void reset();
  //@}


protected:

  /**@name Configuration Controls */
  //@{
  ///
  virtual void randomize();

  /**@name Mutation Controls */
  //@{

  /**@name Standard Mutation */
  //@{
  /// Add a uniform r.v. 
  void mutation_uniform(double rate=-1.0, double per_bit_rate=-1.0, double scale=0.1);

  ///
  void mutation_normal(double rate=-1.0, double per_bit_rate=-1.0, double scale=0.1);

  ///
  void mutation_cauchy(double rate=-1.0, double per_bit_rate=-1.0, double scale=0.1);

  ///
  void mutation_interval(double rate=-1.0, double per_bit_rate=-1.0);

  ///
  void mutation_general_triangular(double rate=-1.0, double scale=0.1);
  //@}

  /**@name Pattern Search Mutation */
  //@{
  ///
  void mutation_discrete(double rate=-1.0, double per_bit_rate=-1.0, double scale=0.1,
				int ptype=1);

  ///
  void mutation_unary_discrete(double rate=-1.0, double scale=0.1, int ptype=1);

  ///
  void setup_ecfactors(double ex_factor, double ct_factor);

  ///
  int Npart;

  ///
  double delta_thresh;

  double mutation_allele_rate;
  //@}
  //@}


  /**@name Setup Methods for the Crossover Operator */
  //@{
	///
  void set_xover_type(const char* name);
	///
  void set_xover_alpha(double alpha_)
		{crossover.alpha=alpha_;}
	///
  void xover_blend(double rate=-1.0, double alpha = 0.5, bool Fstat=false);
	///
  void xover_twopoint(double rate=-1.0, int block_size=1, bool Fstat=false);
	///
  void xover_uniform(double rate=-1.0, int block_size=1, bool Fstat=false);
  //@}


  //**@name Local Search Controls */
  //@{
  ///
  int self_adapted_ls_scale;

  ///
  int update_ls_scale;

  ///
  int stepsize_flag;

  ///
  double ls_grad_thresh;

  //@}
  //@}

protected:

  /**@name General Information */
  //@{
  ///
  int	nvars();

  //@}


  /**@name Configuration Controls */
  //@{
  ///
  void set_op_rng(utilib::AnyRNG&);

  /**@name Mutation Controls */
  //@{
  struct mutation_info {
    int type;
    double init_scale;
    double scale;
    bool self_adaptation;
    utilib::SimpleRandomVariable<double> *rnd;
    utilib::SimpleRandomVariable<double> *rnd2;
    int ptype;
    BitArray map;
    BitArray ctr;
    } mutation;

	///
  bool using_pattern_search;

	///
  bool apply_mutation(EAindividual<EApoint_real,NumArray<double> >& point, int parent_ndx);


  /**@name Standard Mutation */
  //@{
	/// Used for self-adapted mutation
  double taudash;

	/// Used for self-adapted mutation
  double tau;

	/// Used for self-adapted mutation
  utilib::Normal es_rng;

	///
  double bounded_deviation(double val, double offset, double l, double u);
  //@}

  /**@name Pattern Search */
  //@{
	///
  int pattern_search_flag;

	///
  void apply_epsa_mutation(EAindividual<EApoint_real,NumArray<double> >& point, int parent_ndx);

	///
  BasicArray<NumArray<double> > pattern;

	///
  void perform_pattern_step(int ndx, const int ptype, bool& feasible,
			EApoint_real& pt);

        ///
  double ct_factor;

        ///
  double ex_factor;

        ///
  int num_min;

        ///
  void update_stats(bool first_time=false);

        ///
  void find_min();
  
	/// Defines default algorithmic options for evolutionary pattern search
  void setup_pattern_search();
  //@}
  //@}


  /**@name Crossover Controls */
  //@{
#ifndef SWIG
  struct crossover_info {
    int type;
    bool Fstat;
    bool adaptation;
    double alpha;
    int block_size;
    } crossover;
#endif

	///
  void apply_xover(EAindividual<EApoint_real,NumArray<double> >& parent1_, EAindividual<EApoint_real,NumArray<double> >& parent2_,
                        EAindividual<EApoint_real,NumArray<double> >& child_);
  //@}


  /**@name Local Search Controls */
  //@{
	///
  double       ls_epsilon;

        ///
  NumArray<double> ls_tmpvec;

        ///
  NumArray<double> ls_gradient;

        ///
  BasicArray<colin::real> ls_buf;

        ///
  NumArray<double> ls_point;

        ///
  utilib::IntVector  ls_selection;

  ///
  void init_ls(EAindividual<EApoint_real, NumArray<double> >& child);

  ///
  void fini_ls(EAindividual<EApoint_real, NumArray<double> >& /*child*/);

  ///
  void setup_local_search();

  ///
  void exec_local_search();

  ///
  void ls_initialization(const int method);

  ///
  void setup_ls(EAindividual<EApoint_real, NumArray<double> >& ptr);

  ///
  double          calc_closest(EAindividual<EApoint_real,NumArray<double> >& child, int flag);
  //@}

  //@}


  /**@name Debugging Controls */
  //@{
	///
  void virt_debug_io(std::ostream& os, const bool finishing, const int io_level);
  //@}


  /**@name Iteration Controls */
  //@{
        ///
  bool check_convergence(real& fret);

	///
  utilib::DoubleMatrix popmatrix;

	///
  int calc_popdiam_stats(double& diam, double& meandist, double& mediandist,
				double& var_dist, double& mad_dist, 
				double& mean_dist_min, double& median_dist_min);
  //@}
  
private:

	/// The variable lower bound
  BasicArray<real> lower;

	/// The variable upper bounds
  BasicArray<real> upper;

	/// upper - lower
  BasicArray<double> scale;

	///
  NumArray<double> tmpvec;

	/// Work vectors for blend xover
  NumArray<double> xover_tmp1, xover_tmp2, search_dir;

	///
  int grad_approx;

};


} // namespace scolib

#endif
#endif
