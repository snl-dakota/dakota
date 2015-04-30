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
 * \file DirectSimpleApplication.h
 *
 * Defines colin::DirectSimpleApplication.
 */

#ifndef colin_DirectSimpleApplication_h
#define colin_DirectSimpleApplication_h

#include <acro_config.h>
#include <colin/ConfigurableApplication.h>
#include <colin/SynchronousApplication.h>

namespace colin
{

/** \class DirectSimpleApplication
  * This application class evaluates a userd-defined function where
  * the domain is an array and for which the
  * user function is passed in as a pointer to the data in the array.  We
  * assume, however, that the class ArrayT is used in the C++ code (since
  * this provides a nice degree of encapsulation).
  */
template <class ProblemT, class DomainT, class FuncT, class ParamT>
class DirectSimpleApplication 
   : public Synchronous<ConfigurableApplication<ProblemT> >
{
public:

   /// Generic constructor.
   DirectSimpleApplication(FuncT _fn)
   { fn = _fn; }

protected:

   /// Convert the domain supplied by the solver to the application domain
   virtual bool
   map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
   { 
      static_cast<void>(forward);
      return utilib::TypeManager()->lexical_cast
         (src, native, typeid(DomainT)) == 0; 
   }


   /// Actually perform the calculation(s) requested by the solver
   virtual void
   perform_evaluation_impl( const utilib::Any &domain,
                            const AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            AppResponse::response_map_t &responses )
   {
      // this application DOES NOT support random seed control....
      seed = 0;
      
      DomainT dummy_pt;
      DSA_derived_compute_response(domain, requests, responses, fn, dummy_pt);
   }

protected:
   ///
   FuncT fn;
};


///
template <class DomainT, class ParamT>
void DSA_derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 double(*eval_fn)(ParamT*, int), DomainT& dummy)
{
   static_cast<void>(dummy);
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      const DomainT& pt = point.expose<DomainT>();
      double ans = (*eval_fn)(const_cast<ParamT*>(&(pt[0])), pt.size());
      tmp = ans;
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT, class ParamT>
void DSA_derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 double(*eval_fn)(const ParamT*, int), DomainT& dummy)
{
   static_cast<void>(dummy);
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      const DomainT& pt = point.expose<DomainT>();
      double ans = (*eval_fn)(&(pt[0]), pt.size());
      tmp = ans;
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}

}

#endif
