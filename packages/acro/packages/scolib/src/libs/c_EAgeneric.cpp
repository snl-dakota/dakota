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
// A C interface to an EAgeneric class
//

#include <acro_config.h>
#include <scolib/DomainOpsBase.h>
#include <scolib/DomainInfoBase.h>
#include <scolib/EAgeneric.h>
#include <scolib/c_EAgeneric.h>

#include <utilib/BasicArray.h>
#include <utilib/pvector.h>

using namespace std;
using namespace utilib;

namespace local {

class Container
{
public:

  Container() : obj(0) {}
  Container& operator=(const Container& item)
	{ obj = item.obj; return *this; }
  EAgenericUserType obj;
  bool operator==(const Container& pt) const
	{return c_EAgeneric_point_equal(obj,pt.obj);}
  bool operator<(const Container& pt) const
	{return c_EAgeneric_point_less(obj,pt.obj);}
  void eval(colin::real& val, utilib::BasicArray<colin::real>& cval) const
	{
	double tmp;
        if (num_constraints == 0)
	   c_EAgeneric_evaluate_point(obj,tmp,&tmp);
        else
	   c_EAgeneric_evaluate_point(obj,tmp,cvec.data());
	val = tmp;
	for (int i=0; i<num_constraints; i++)
          cval[i] = cvec[i];
	}

  static int num_constraints;
  static utilib::BasicArray<double> cvec;
};

int Container::num_constraints = 0;
utilib::BasicArray<double> Container::cvec;

void testfunc(const Container& x, colin::real& val, utilib::BasicArray<colin::real>& cval)
{
x.eval(val,cval);
}

PackBuffer& operator<<(PackBuffer& os, const Container& pt)
{ return os; }

UnPackBuffer& operator>>(UnPackBuffer& is, Container& pt)
{ return is; }


ostream& operator<<(ostream& os, const Container& pt)
{
os.flush();
c_EAgeneric_write_point(pt.obj);
return os;
}

istream& operator>>(istream& is, Container& pt)
{ return is; }


Container& operator<<(Container& to, const Container& from)
{
c_EAgeneric_copy_point(from.obj,to.obj);
return to;
}


typedef scolib::DomainInfoBase DomainInfo;

class DomainOps  : public utilib::ParameterSet, public utilib::CommonIO
{
public:

  DomainOps() {}

  void reset() { c_EAgeneric_init_operators(); }

  void write(ostream& os) const {}

  void initialize(
  		colin::Problem<colin::MINLP0_problem>& problem,
		unsigned int popsize, Container& best_pt, double xover_rate, double m_rate) {}

  template <class A, class B>
  bool check_domain(A&,B&) {return false;}

  void read_point(istream& is, Container& point) {}

  /// Randomize the point
  void randomize(Container& point, DomainInfo& info)
	{ c_EAgeneric_randomize_point(point.obj); }

  /// Allocate the memory for this point
  void initialize_point(Container& point, DomainInfo& info)
	{ c_EAgeneric_initialize_point(point.obj); }

  int apply_xover(Container& parent1, DomainInfo& info1,
		  Container& parent2, DomainInfo& info2,
		  Container& child,   DomainInfo& info_c)
        { return c_EAgeneric_xover(parent1.obj, parent2.obj, child.obj); }

  bool apply_mutation(Container& point, DomainInfo& info, int parent_ndx)
        {
	int tmp=-1;
	c_EAgeneric_mutation(point.obj, &parent_ndx, &tmp);
	return (tmp == 0 ? false : true);
	}

  void set_rng(utilib::AnyRNG& rng_) {}

};


class ColinyHandle
{
public:
  ColinyHandle() : setup_problem(false) {}
  bool setup_problem;
  scolib::EAgeneric<Container,colin::MINLP0_problem,DomainInfo,DomainOps> solver;
  colin::Problem<colin::MINLP0_problem> problem;
};

typedef ColinyHandle scolib_handle_t;
} // namespace local




EAgenericObjectType c_EAgeneric_allocate()
{
local::ColinyHandle* tmp = new local::ColinyHandle;
return static_cast<EAgenericObjectType>(tmp);
}



void c_EAgeneric_deallocate(EAgenericObjectType& obj_)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj) {
   delete obj;
   obj_ = 0;
   }
}


void c_EAgeneric_set_parameter(EAgenericObjectType obj_, 
				char* name, char* value)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj) {
   obj->solver.set_parameter_with_string(name,value);
   }
}


void c_EAgeneric_reset(EAgenericObjectType obj_)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj) {
   if (!(obj->setup_problem)) {
      obj->setup_problem = true;
      c_EAgeneric_init_problem(local::Container::num_constraints);
      utilib::pvector<double> clower(local::Container::num_constraints);
      utilib::pvector<double> cupper(local::Container::num_constraints);
      c_EAgeneric_init_constraints(&(clower[0]),&(cupper[0]));
      #if COLIN
      colin::DirectFuncApplication<
         colin::MINLP0_problem, 
         const local::Container&,
         void(*)(const local::Container&, colin::real&, 
                 utilib::BasicArray<colin::real>&)> * app
         = colin::new_application("c_EAgeneric",&local::testfunc);
      obj->problem.set_application( app );
      app->config_nonlinear_constraints(local::Container::num_constraints,0);
      app->set_nonlinear_constraint_bounds(clower,cupper);
      #endif
      obj->solver.set_problem(obj->problem);
      local::Container::cvec.resize(local::Container::num_constraints);
      }
   obj->solver.reset();
   }
}



void c_EAgeneric_minimize(EAgenericObjectType obj_)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj)
   obj->solver.minimize();
}


void c_EAgeneric_get_best_point(EAgenericObjectType obj_,
				EAgenericUserType pt)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj) {
   pt = utilib::anyref_cast<local::Container>(obj->solver.best().point).obj;
   }
}


void c_EAgeneric_get_value_of_best_point(EAgenericObjectType obj_,
				double* value)
{
local::scolib_handle_t* obj = static_cast<local::scolib_handle_t*>(obj_);
if (obj) {
   *value = obj->solver.best().value();
   }
}
