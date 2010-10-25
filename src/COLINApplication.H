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

#ifndef COLIN_APPLICATION_H
#define COLIN_APPLICATION_H

#include <colin/colin.h>
#include "data_types.h"
#include "data_util.h"
#include "DakotaModel.H"
#include "DakotaVariables.H"

//
// Response type used for COLIN optimizers
//
typedef colin::AppResponse<> ColinResponse;

/** \class ColinPoint
 *
 * A class containing a vector of doubles and integers.
 */
class ColinPoint
{
public:

  std::vector<double> rvec; ///< continuous parameter values
  std::vector<int>    ivec; ///< discrete integer parameter values
};


//
// Define the domain traits for ColinPoints
//
SetOptDomainTrait(ColinPoint,reals,true);
SetOptDomainTrait(ColinPoint,linearconstraints,true);
SetOptDomainTrait(ColinPoint,boundconstraints,true);
SetOptDomainTrait(ColinPoint,integers,true);


//
// Various mapping routines to/from ColinPoints, required to interface both
// COLINApplication and the range of COLIN solvers.
//
namespace colin {

template<>
inline void map_domain<ColinPoint,Dakota::RealVector>
(ColinPoint& target, const Dakota::RealVector& source)
{
  size_t len = source.length();
  if (target.rvec.size() != len)
    target.rvec.resize(len);
  for (size_t i=0; i<len; i++)
    target.rvec[i] = source[i];
}

template<>
inline void map_domain<Dakota::RealVector,ColinPoint>
(Dakota::RealVector& target, const ColinPoint& source)
{
  size_t len = source.rvec.size();
  if (target.length() != len)
    target.sizeUninitialized(len);
  for (size_t i=0; i<len; i++)
    target[i] = source.rvec[i];
}

/*
template<>
inline void map_domain<ColinPoint,Dakota::IntVector>
(ColinPoint& target, const Dakota::IntVector& source)
{
  size_t len = source.length();
  if (target.ivec.size() != len)
    target.ivec.resize(len);
  for (size_t i=0; i<len; i++)
    target.ivec[i] = source[i];
}

template<>
inline void map_domain<Dakota::IntVector,ColinPoint>
(Dakota::IntVector& target, const ColinPoint& source)
{
  size_t len = source.ivec.size();
  if (target.length() != len)
    target.sizeUninitialized(len);
  for (size_t i=0; i<len; i++)
    target[i] = source.ivec[i];
}
*/

//template<>
inline void map_domain
/*<ColinPoint,Dakota::Variables,Dakota::IntSetArray,Dakota::RealSetArray>*/
(ColinPoint& target, const Dakota::Variables& source,
 const Dakota::IntSetArray&  source_dsiv,
 const Dakota::RealSetArray& source_dsrv)
{
  map_domain(target,source.continuous_variables());
  //map_domain(target,source.discrete_int_variables());

  const Dakota::IntVector&  div = source.discrete_int_variables();
  const Dakota::RealVector& drv = source.discrete_real_variables();
  size_t i, offset, num_div = div.length(), num_drv = drv.length(),
    num_dsiv = source_dsiv.size(), num_dsrv = source_dsrv.size(),
    num_driv = num_div - num_dsiv;
  target.ivec.resize(num_div+num_drv);

  // discrete range
  for (i=0; i<num_driv; ++i)
    target.ivec[i] = div[i];
  offset = num_driv;
  // discrete int set: map from index to set value
  for (i=0; i<num_dsiv; ++i)
    target.ivec[i+offset]
      = Dakota::set_value_to_index(div[i+offset], source_dsiv[i]);
  offset += num_dsiv;
  // discrete real set: map from index to set value
  for (i=0; i<num_dsrv; ++i)
    target.ivec[i+offset] = Dakota::set_value_to_index(drv[i], source_dsrv[i]);
}

//template<>
inline void map_domain
/*<Dakota::Variables,Dakota::IntSetArray,Dakota::RealSetArray,ColinPoint>*/
(Dakota::Variables& target, const Dakota::IntSetArray& target_dsiv,
 const Dakota::RealSetArray& target_dsrv, const ColinPoint& source)
{
  Dakota::RealVector cv;
  map_domain(cv,source);
  target.continuous_variables(cv);

  //Dakota::IntVector dv;
  //map_domain(dv,source);
  //target.discrete_int_variables(dv);

  size_t i, offset = 0, num_div = target.div(), num_drv = target.drv();
  if (num_div) {
    Dakota::IntVector div(num_div, false);
    size_t num_dsiv = target_dsiv.size(), num_driv = num_div - num_dsiv;
    // discrete range
    for (i=0; i<num_driv; ++i)
      div[i] = source.ivec[i];
    offset += num_driv;
    // discrete int set: map from index to set value
    for (i=0; i<num_dsiv; ++i)
      div[i+offset]
	= Dakota::set_index_to_value(source.ivec[i+offset], target_dsiv[i]);
    offset += num_dsiv;
    // set aggregated array
    target.discrete_int_variables(div);
  }
  if (num_drv) {
    Dakota::RealVector drv(num_drv, false);
    size_t num_dsrv = target_dsrv.size();
    // discrete real set: map from index to set value
    for (i=0; i<num_dsrv; ++i)
      drv[i]
	= Dakota::set_index_to_value(source.ivec[i+offset], target_dsrv[i]);
    // set aggregated array
    target.discrete_real_variables(drv);
  }
}

template<>
inline void map_domain<ColinPoint,Dakota::Model>
(ColinPoint& target, const Dakota::Model& source)
{
  map_domain(target, source.current_variables(), 
	     source.discrete_design_set_int_values(),
	     source.discrete_design_set_real_values());
}

template<>
inline void map_domain<Dakota::Model,ColinPoint>
(Dakota::Model& target, const ColinPoint& source)
{
  Dakota::Variables vars_shallow_copy(target.current_variables());
  map_domain(vars_shallow_copy, // must be non-const
	     target.discrete_design_set_int_values(),
	     target.discrete_design_set_real_values(), source);
}

template<>
inline void map_domain<ColinPoint,std::vector<double> >
(ColinPoint& target, const std::vector<double>& source)
{
  target.ivec.resize(0);
  target.rvec.resize(source.size());
  for(int i=0;i<source.size();i++)
    target.rvec[i]=source[i];
}

template<>
inline void map_domain<std::vector<double>,ColinPoint >
(std::vector<double>& target, const ColinPoint& source)
{
  target.resize(source.rvec.size());
  for(int i=0;i<source.rvec.size();i++)
    target[i]=source.rvec[i];
}

template<>
inline void map_domain<ColinPoint,utilib::NumArray<double> >
(ColinPoint& target, const utilib::NumArray<double>& source)
{
  target.ivec.resize(0);
  target.rvec.resize(source.size());
  for(int i=0;i<source.size();i++)
    target.rvec[i]=source[i];
}

template<>
inline void map_domain<utilib::NumArray<double>,ColinPoint >
(utilib::NumArray<double>& target, const ColinPoint& source)
{
  target.resize(source.rvec.size());
  for(int i=0;i<source.rvec.size();i++)
    target[i]=source.rvec[i];
}

template<>
inline void map_domain<ColinPoint,BasicArray<double> >
(ColinPoint& target, const BasicArray<double>& source)
{
  target.rvec.resize(source.size());
  target.rvec << source;
}

template<>
inline void map_domain<BasicArray<double>,ColinPoint >
(BasicArray<double>& target, const ColinPoint& source)
{
  target.resize(source.rvec.size());
  for(int i=0;i<source.rvec.size();i++)
    target[i]=source.rvec[i];
}

template<>
inline void map_domain<ColinPoint,utilib::MixedIntVars>
(ColinPoint& target, const utilib::MixedIntVars& source)
{
  const utilib::NumArray<double>& cVars = source.Real();
  const utilib::NumArray<int>& dVars = source.Integer();

  target.rvec.resize(cVars.size());
  for(size_t i=0;i<cVars.size();i++)
    target.rvec[i]=cVars[i];
  target.ivec.resize(dVars.size());
  for(size_t i=0;i<dVars.size();i++)
    target.ivec[i]=dVars[i];
}

template<>
inline void map_domain<utilib::MixedIntVars,ColinPoint>
(utilib::MixedIntVars& target, const ColinPoint& source)
{
  target.resize(0, source.ivec.size(), source.rvec.size());
  utilib::NumArray<double>& cVars = target.Real();
  utilib::NumArray<int>& dVars = target.Integer();

  for(size_t i=0;i<cVars.size();i++)
    cVars[i] = source.rvec[i];
  for(size_t i=0;i<dVars.size();i++)
    dVars[i] = source.ivec[i];
}

} // namespace colin


namespace Dakota {


/** COLINApplication is a DAKOTA class that is derived from COLIN's
    OptApplication hierarchy.  It redefines a variety of virtual COLIN 
    functions to use the corresponding DAKOTA functions.  This is a
    more flexible algorithm library interfacing approach than can be
    obtained with the function pointer approaches used by
    NPSOLOptimizer and SNLLOptimizer. */

class COLINApplication: public colin::OptApplication<ColinPoint,ColinResponse>
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  COLINApplication(Model& model);

  /// destructor
  ~COLINApplication();
    
  //
  //- Heading: Virtual function redefinitions
  //

  /// launch a function evaluation either synchronously or asynchronously
  void DoEval(ColinPoint& point, int& priority,
	      ColinResponse* response, bool synch_flag);

  /// The number of 'slave' processors that can perform evaluations.
  /// The value '0' indicates that this is a sequential application.
  unsigned int num_evaluation_servers()
  { return iteratedModel.evaluation_capacity(); }
  // NOTE: this must be called after Model::init_communicators()

  /// blocking retrieval of all pending jobs
  void synchronize();

  /// nonblocking query and retrieval of a job if completed
  int next_eval();

  /// This function publishes the iterator's blockingSynch flag at
  /// construct time.
  void blocking_synch(const bool& blocking_synch)
  { blockingSynch = blocking_synch; }

  /// This function publishes the iterator's asynchFlag at run time
  /// (asynchFlag not initialized properly at construction).
  void dakota_asynch_flag(const bool& asynch_flag)
  { dakotaModelAsynchFlag = asynch_flag; }

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Private data
  //

  /// reference to the COLINOptimizer's model passed in the constructor
  Model& iteratedModel;

  /// copy/conversion of the COLIN request vector
  ActiveSet activeSet;

  /// a flag for asynchronous DAKOTA evaluations
  bool dakotaModelAsynchFlag;

  /// flag for user specification of "synchronization blocking".  This is only
  /// needed for APPS, to enforce blocking synch despite call of next_eval().
  bool blockingSynch;

  /// map of DAKOTA responses returned by synchronize_nowait()
  IntResponseMap dakotaResponseMap;

  size_t numObjFns;     ///< number of objective functions
  size_t numNonlinCons; ///< number of nonlinear constraints

  /// utility function for mapping a DAKOTA response to a COLIN response
  void map_response(ColinResponse& colin_response,
		    const Response& dakota_response);

  // number of continuous design variables
  //int num_real_params;
  // number of discrete integer design variables
  //int num_integer_params;

  // tracks the state of asynchronous evaluations
  //int synchronization_state;
};


inline COLINApplication::~COLINApplication(void)
{ }

} // namespace Dakota

#endif
