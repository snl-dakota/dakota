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
 * \file EAindividual.h
 *
 * Defines the scolib::EAindividual class.
 */

#ifndef scolib_EAindividual_h
#define scolib_EAindividual_h

#include <acro_config.h>
#include <utilib/PackBuf.h>
#include <utilib/_generic.h>
#include <utilib/PackObject.h>
#include <colin/real.h>
#include <colin/OptResponse.h>
#include <colin/AppResponse.h>

namespace scolib {

using colin::real;

///
/// Base header for the classes for GA operators
/// The handle class which holds an id for the genotype, phenotype,
/// eval and true_eval  of an individual.  This class maintains the 
/// current status of evaluation, as well as the mapping between genotype 
/// and phenotype.  
///
template <class PointT, class DomainT>
class EAindividual : public utilib::PackObject
{
public:

  /// Constructor.
  EAindividual(colin::AppResponse& response_);

  /// Constructor.
  EAindividual();

  ///
  virtual ~EAindividual() {}

  ///
  void reset();

  /// The value used to perform selection
  real& selection_eval()
		{return Sel_Val;}

  /// The true value of the point
  real& true_eval()
		{return Val;}

  /// The value used to perform selection
  real selection_eval() const
		{return Sel_Val;}

  /// The true value of the point
  real true_eval() const
		{return Val;}

  ///
  void write(std::ostream&) const;

  ///
  void write_point(std::ostream& os) const
		{ pt.write_point(os); }

  ///
  void read(std::istream&);

  ///
  void write(utilib::PackBuffer&) const;

  ///
  void read(utilib::UnPackBuffer&);

  ///
  void update_from_response()
  {
     response.get(colin::f_info, Sel_Val);
     response.get(colin::f_info, Val);
     if ( response.is_computed(colin::cvf_info) )
        response.get(colin::cvf_info, Cval);
     else
        Cval = colin::real::positive_infinity;
     reset();
  }

  ///
  void update(colin::OptResponse& opt_response, bool update_point);

  ///
   void update( colin::AppResponse response_, utilib::Any domain_ );

  ///
  PointT  pt;

  /// The response object for evaluations
  colin::AppResponse response;

  /// The total constraint violation of the point pt
  real Cval;

  /// The true value
  real Val;

  /// The value used for selection (which defaults to Val in most cases)
  real Sel_Val;

  ///
  int id;

  ///
  bool eval_flag;

  ///
  bool ls_flag;

  ///
  EAindividual<PointT,DomainT>& operator=(const EAindividual<PointT,DomainT>& indiv);

  ///
  bool operator==(const EAindividual<PointT,DomainT>& indiv) const
		{ return (pt == indiv.pt); }

  ///
  EAindividual<PointT,DomainT>& copy(EAindividual<PointT,DomainT>& pt);

  ///
  int parent1_id;

  ///
  int parent2_id;

  ///
  int gen_status;

  ///
  real& parent_eval()
		{ return Parent_Eval; }

  ///
  real parent_eval() const
		{ return Parent_Eval; }

  ///
  double Fstat;

protected:

  /// Only used if eval_id==-1
  real True_Eval;

  ///
  real Parent_Eval;

};


template <class PointT, class DomainT>
void EAindividual<PointT,DomainT>::reset()
{
eval_flag=true;
ls_flag=false;
}


template <class PointT, class DomainT>
EAindividual<PointT,DomainT>::EAindividual(colin::AppResponse& response_)
{
response = response_;
gen_status = 0;
id = parent1_id = parent2_id = -1;
eval_flag = true; ls_flag=false; 
Fstat=0.0;
if ( ! response.empty() )
   update_from_response();
}


template <class PointT, class DomainT>
EAindividual<PointT,DomainT>::EAindividual()
{
gen_status = 0;
id = parent1_id = parent2_id = -1;
eval_flag = true; ls_flag=false; 
Fstat=0.0;
}


template <class PointT, class DomainT>
EAindividual<PointT,DomainT>& EAindividual<PointT,DomainT>::operator=(const EAindividual<PointT,DomainT>& indiv)
{
eval_flag = indiv.eval_flag;
ls_flag = indiv.ls_flag;

pt = indiv.pt;

parent1_id = indiv.parent1_id;
parent2_id = indiv.parent2_id;
gen_status = indiv.gen_status;

return *this;
}


template <class PointT, class DomainT>
EAindividual<PointT,DomainT>& EAindividual<PointT,DomainT>::copy(EAindividual<PointT,DomainT>& point)
{
id = point.id;
Cval = point.Cval;
response = point.response;
Val = point.Val;
Sel_Val = point.Sel_Val;
eval_flag = point.eval_flag;		// CHECK!
ls_flag = point.ls_flag;

parent1_id = point.parent1_id;
parent2_id = point.parent2_id;
gen_status = point.gen_status;
 
pt.copy(point.pt);

return *this;
}
 
template <class PointT, class DomainT>
void EAindividual<PointT,DomainT>::update(colin::OptResponse& opt_response, bool update_point)
{
response = opt_response.response;
update_from_response();
if (update_point)
   pt.point = utilib::anyref_cast<DomainT>(opt_response.point);
ls_flag=true;
}

template <class PointT, class DomainT>
void EAindividual<PointT,DomainT>::
update( colin::AppResponse response_, utilib::Any domain_ )
{
response = response_;
update_from_response();
utilib::TypeManager()->lexical_cast(domain_, pt.point);
ls_flag=true;
}


#if 0
template <class PointT, class DomainT>
void EAindividual<PointT,DomainT>::set_eval(const real& fret)
{
response.function_value() = fret;
response.constraint_values() << 0.0;
Val = fret;
Sel_Val = fret;
ls_flag = false;
eval_flag = false;
}
#endif
 
 
template <class PointT, class DomainT>
void  EAindividual<PointT,DomainT>::write(std::ostream& os) const
{
os << pt;
os << "Sel_Val " << Sel_Val << "  Eval " << Val << "  " << "  CViol " << Cval;
pt.ls_info(os);
os << 	"  Eval_Flag " << eval_flag << "  LS_Flag " << ls_flag << 
	"  ID " << id <<
	"  P1ID " << parent1_id << "  P2ID " << parent2_id <<
	"  GenStat " << (gen_status > 1 ? "x" : "-") <<
			((gen_status == 1) || (gen_status == 3) ? "m" : "-");

}


template <class PointT, class DomainT>
void  EAindividual<PointT,DomainT>::read(std::istream&)
{
}


template <class PointT, class DomainT>
void  EAindividual<PointT,DomainT>::write(utilib::PackBuffer& os) const
{
EXCEPTION_MNGR(std::logic_error, "PackBuffer is deprecated in COLIN/Coliny");
//os << id << response << Val << Sel_Val;
os << eval_flag << ls_flag;
os << parent1_id << parent2_id << gen_status;
pt.write(os);
}
 
 
template <class PointT, class DomainT>
void  EAindividual<PointT,DomainT>::read(utilib::UnPackBuffer& is)
{
EXCEPTION_MNGR(std::logic_error, "PackBuffer is deprecated in COLIN/Coliny");
//is >> id >> response >> Val >> Sel_Val;
is >> eval_flag >> ls_flag;
is >> parent1_id >> parent2_id >> gen_status;
pt.read(is);
}


/// Class used to access elements of arrays that contain EAindividual pointers
template <class PointT, class DomainT>
class EAindividual_array_accessor
{
public:

  ///
  EAindividual_array_accessor(bool flag_) : flag(flag_) {}

  ///
  real operator()(EAindividual<PointT,DomainT>* ptr)
                {return (flag ? ptr->Sel_Val : ptr->Val);}

  /// Accessor flag: true=selection value, false=point value
  bool flag;
};


/// Class used to compare elements of arrays that contain EAindividual pointers
template <class PointT, class DomainT>
class EAindividual_array_compare
{
public:

  ///
  EAindividual_array_compare(bool flag_) : flag(flag_) {}

  ///
  bool operator()(EAindividual<PointT,DomainT>* ptr1,
                 EAindividual<PointT,DomainT>* ptr2)
                {return (flag ? (ptr1->Sel_Val < ptr2->Sel_Val):
                                (ptr1->Val < ptr2->Val));}

  /// Compare flag: true=selection value, false=point value
  bool flag;
};



} // namespace scolib

template <class PointT, class DomainT>
inline scolib::EAindividual<PointT,DomainT>& operator<<(scolib::EAindividual<PointT,DomainT>& x, const scolib::EAindividual<PointT,DomainT>& pt)
{x.copy(pt); return x;}

#endif
