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

#if 0
/**
 * \file FOO 
 */
//
// EAops_array.h
//

#ifndef scolib_EAops_array_h
#define scolib_EAops_array_h

#include <acro_config.h>
#include <utilib/NumArray.h>
#include <utilib/Uniform.h>
#include <utilib/Normal.h>
#include <utilib/Cauchy.h>
#include <utilib/CharString.h>
#include <scolib/EAops.h>
#include <scolib/EApoint.h>

namespace scolib {


template <typename Type>
class EApoint_array : public EApoint<BasicArray<Type> >
{
public:

  using EApoint<BasicArray<Type> >::point;

  ///
  EApoint_array() 
	{}

  ///
  void write_point(std::ostream& os) const
                {os << point;}

  ///
  void write(std::ostream& os) const
                { }

  ///
  void read(std::istream& is)
                {is >> point ;}

  ///
  void ls_info(std::ostream&) const {}

  ///
  void write(PackBuffer& pack) const
                {pack << point ;}

  ///
  void read(UnPackBuffer& unpack)
                {unpack >> point ;}

};



/** Class that defines operations used with NumArray<double> genotypes.
*/
template <class Type>
class EAops_array : public EAops<EApoint_array<Type>, BasicArray<Type> >
{
public:

  typedef EAops<EApoint_array<Type>, BasicArray<Type> > base_t;
  #if !defined(SOLARIS)
  using base_t::init_unique;
  using base_t::popsize_;
  using base_t::geno_array;
  using base_t::pheno_array;
  using base_t::using_phenotype;
  using base_t::num_min;
  using base_t::Xover_type;
  using base_t::problem;
  using base_t::init_filename;
  using base_t::lower;
  using base_t::upper;
  using base_t::tmpvec;
  using base_t::XOVER_TWOPOINT;
  using base_t::XOVER_UNIFORM;
  #endif

  /**@name General Information */
  //@{
  ///
  EAops_array();

  ///
  void write(std::ostream& os) const;

  ///
  void reset();
  //@}

protected:

  ///
  double mutation_allele_rate;

  ///
  virtual void randomize();

  ///
  int nvars()
		{return num_vars;}

  ///
  EApoint_array<Type>& Genotype(const EAindividual<EApoint_array<Type>,BasicArray<Type> >& indiv)
		{return geno_array[indiv.pop_id];}

  ///
  const EApoint_array<Type>& Genotype(const EAindividual<EApoint_array<Type>,BasicArray<Type> >& indiv) const
		{return geno_array[indiv.pop_id];}

  ///
  EApoint_array<Type>& Phenotype(const EAindividual<EApoint_array<Type>,BasicArray<Type> >& indiv)
		{
		if (using_phenotype)
		   return pheno_array[indiv.pop_id];
		return geno_array[indiv.pop_id];
		}
		
  ///
  const EApoint_array<Type>& Phenotype(const EAindividual<EApoint_array<Type>,BasicArray<Type> >& indiv) const
		{
		if (using_phenotype)
		   return pheno_array[indiv.pop_id];
		return geno_array[indiv.pop_id];
		}
		
#if 0
  ///
  void geno_pheno_init(unsigned int i);
#endif

  ///
  void apply_mutation(EAindividual<EApoint_array<Type>,BasicArray<Type> >& point, int parent_ndx);


  struct crossover_info {
    int type;
    bool Fstat;
    int block_size;
    } crossover;

  ///
  void apply_xover(EAindividual<EApoint_array<Type>,BasicArray<Type> >& parent1_, EAindividual<EApoint_array<Type>,BasicArray<Type> >& parent2_, EAindividual<EApoint_array<Type>,BasicArray<Type> >& child_);

  ///
  int num_vars;
};



template <typename Type>
EAops_array<Type>::EAops_array()
{
num_vars=0;
Xover_type = "uniform";
crossover.Fstat = false;
crossover.block_size=1;

this->set_option_default("xover_type","uniform");

ParameterSet::create_parameter("xover_blocksize",crossover.block_size,
	"<int>","1",
	"Block size used with two-point and uniform crossover");

ParameterSet::create_parameter("num_vars",num_vars,
	"<int>","0",
	"Array length for solutions");

ParameterSet::create_parameter("xover_fstat",crossover.Fstat,
	"<bool>","false",
	"If true, compute Fstat statistics within crossover");

}
 

template <typename Type>
void EAops_array<Type>::write(std::ostream& os) const
{
os << "##\n## EAops_array: Controls for Array genotypes\n##\n";
os << "#\n# Genetic Operators\n#\n";
switch (crossover.type) {
  case XOVER_TWOPOINT:
	os << "xover-twopoint: Fstat=" << crossover.Fstat
		<< " Block=" << crossover.block_size << std::endl;
	break;

  case XOVER_UNIFORM:
	os << "xover-uniform: Fstat=" << crossover.Fstat
		<< " Block=" << crossover.block_size << std::endl;
	break;
  };

OUTPUTPR(1,
  os << "#\n# Current Solutions\n#\n";
  this->print_points(os);
  );
}


template <typename Type>
void EAops_array<Type>::reset()
{
//
// Reset the base class
//
EAbase<EApoint_array<Type>, BasicArray<Type> >::reset();
//
// Setup xover
//
if (Xover_type == "twopoint")
   crossover.type = XOVER_TWOPOINT;
else if (Xover_type == "uniform")
   crossover.type = XOVER_UNIFORM;
else
   EXCEPTION_MNGR(std::runtime_error, "EAops_array::set_xover_type -- bad xover type: \"" << Xover_type << "\".\n\t\tValid types are twopoint and uniform\n");

//if (this->colin_solver.size() && problem) 
   //colin_solver.set_problem(problem);

randomize();
}


template <typename Type>
void EAops_array<Type>::randomize()
{
if (!problem)
   return;
if (nvars() == 0)
   return;

unsigned int i=0;
//
// Read up to the popsize_ elements from this file
//
if (init_filename != "") {
   std::ifstream ifstr(init_filename.c_str());
   if (ifstr) {
      Type val;
      ifstr >> val;
      while (ifstr && (i < popsize_)) {
        int j=0;
        while (j < nvars()) {
          if (val < lower[j]) val = lower[j];
          if (val > upper[j]) val = upper[j];
          Genotype(this->population(i)).point[j] = val;
          j++;
          }
        this->population(i).reset();
        ifstr >> val;
        i++;
        }
      }
   else
      ucerr << "Bad filename? \"" << init_filename << "\"" << std::endl;
   }
//
// Generate remaining points randomly, possibly testing for uniqueness
//
for (; i<popsize_; i++) {
  bool flag = false;
  while (!flag) {
    for (unsigned int j=0; j<nvars(); j++)
      Genotype(this->population(i)).point[j] = this->random_value(i);
    if ((i == 0) || !init_unique) {
       flag=true;
       continue;
       }
    flag=true;
    for (unsigned int k=0; k<i; k++)
      if (Genotype(this->population(i)).point == Genotype(this->population(k)).point) {
         flag=false;
         break;
         }
    }
  this->population(i).reset();
  }
}


template <typename Type>
void EAops_array<Type>::apply_xover(EAindividual<EApoint_array<Type>, BasicArray<Type> >& parent1_, EAindividual<EApoint_array<Type>, BasicArray<Type> >& parent2_, EAindividual<EApoint_array<Type>, BasicArray<Type> >& child_)
{
if (parent1_ == parent2_) {
   child_.copy(parent1_);
   child_.Fstat = 1.0;
   return;
   }

EApoint_array<Type>& p1 = Genotype(parent1_);
EApoint_array<Type>& p2 = Genotype(parent2_);
EApoint_array<Type>& ch = Genotype(child_);

switch (crossover.type) {
  //
  // Generate a point using coordinate swap at two points.
  // This code can swap blocks, and it implicitly manage bound constraints.
  //
  case XOVER_TWOPOINT:
	{
	//
	// Generate new point
	//
	int lambda1 = Discretize(this->rnd(), 1, nvars()/crossover.block_size-1);
	int lambda2 = Discretize(this->rnd(), 1, nvars()/crossover.block_size-2);
	if (lambda2 >= lambda1)
           lambda2++;	// This keeps lambda2 unif on nvars-1 other points
	else {
  	   int tmp;
  	   tmp = lambda1; lambda1 = lambda2; lambda2 = tmp;
  	   }
	lambda1 *= crossover.block_size;
	lambda2 *= crossover.block_size;         
	ch.point.set_subvec(0,lambda1,p1.point,0);
	ch.point.set_subvec(lambda1,lambda2-lambda1,p2.point,lambda1);
	ch.point.set_subvec(lambda2,nvars()-lambda2,p1.point,lambda2);
	//
	// See if new point is identical to a parent
	//
	if (ch == p1) {
	   child_.copy(parent1_);
	   }
	else if (ch == p2) {
	   child_.copy(parent2_);
	   }
	else
	   child_.reset();
	
	OUTPUTPR(4, ucout << "Xover-TwoPoint : " << ch << "\n");
	}
	break;


  //
  // Generate a point using coordinate swap at all points.
  // This code can swap blocks, and it implicitly manage bound constraints.
  //
  case XOVER_UNIFORM:
	{
	//
	// Generate new point
	//
	for (int i=0; i<nvars(); i++) {
	  double val = this->rnd();
	  ch.point.set_subvec(i*crossover.block_size, crossover.block_size,
			(val >= 0.5? p1.point: p2.point), i*crossover.block_size);
	}
	//
	// See if new point is identical to a parent
	//
	if (ch == p1) {
	   child_.copy(parent1_);
	   }
	else if (ch == p2) {
	   child_.copy(parent2_);
	   }
	else {
	   child_.reset();
	   }
	
	OUTPUTPR(4, ucout << "Xover-Uniform : " << ch << "\n");
	}
	break;
  };


OUTPUTPR(4,
   ucout << "Parent 1    : " << parent1_ << "\n";
   ucout << "Parent 2    : " << parent2_ << "\n";
         );
}


////
//// Apply a MUTATION Method
////

template <typename Type>
void EAops_array<Type>::apply_mutation(EAindividual<EApoint_array<Type>, BasicArray<Type> >& point, int parent_ndx)
{
if (mutation_allele_rate == 0.0)
   return;

//
// Initial Information
//
EApoint_array<Type>& pt = Genotype(point);
tmpvec << pt.point;
bool flag=false;

//
// Perform mutation, by computing where the next mutation point will
// occur.  This is more efficient when the per_bit mutation rate is low.
//
int Mu_next=0;
int len=nvars();
while (1) {
     //
     // Update next mutation location
     //
     if (mutation_allele_rate < 1.0) {
        double r = this->rnd();
        Mu_next += (int) std::ceil (std::log(r) / std::log(1.0 - mutation_allele_rate));
        }
     else
        Mu_next += 1;
     if (Mu_next > len)
        break;
     int i= Mu_next-1;
     OUTPUTPR(1,
        ucout << "Fancy Mutate #" << i << " ";
        ucout << pt.point[i];
              );

     //
     // Perform the mutation step
     //
     mutation_value(i,pt.point[i]);
     }

//
// Print the progress of the best point in the population
//
if (flag && 
    (this->Debug_search_steps) && (parent_ndx != -1) && (parent_ndx < num_min)) {
   ucout << "[" << std::endl << "Curr_best_point: ";
   this->population(parent_ndx).write_point(ucout);
   ucout << "  New_point: ";
   point.write_point(ucout);
#if 0
   double tmp=0.0;
   for (int i=0; i<nvars(); i++) {
     double x = Genotype(this->population(parent_ndx)).point[i];
     double y = Genotype(point).point[i];
     tmp += (x-y)*(x-y);
     }
   ucout << "  Delta: " << std::sqrt(tmp);
#endif
   ucout << std::endl << "]" << std::endl;
   }

//
// Reset the point
//
if (flag)
   point.reset();
}


} // namespace scolib

#endif
#endif
