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
 * \file DirectFuncApplication.h
 *
 * Defines the colin::DirectFuncApplication class, along with
 * template instances of the function colin::derived_compute_response,
 * which are used by this class.
 */

#ifndef colin_DirectFuncApplication_h
#define colin_DirectFuncApplication_h

#include <acro_config.h>
#include <colin/ConfigurableApplication.h>
#include <colin/SynchronousApplication.h>

namespace colin
{

/**
  * This application class calls the function \c derived_compute_response to
  * perform the evaluation of the user's objective function FuncT.
  */
template <class ProblemT>
class DirectFuncApplication 
   : public Synchronous<ConfigurableApplication<ProblemT> >
{};


template <class ProblemT, class DomainT, class FuncT>
class DirectFuncApplicationImpl : public DirectFuncApplication<ProblemT>
{
public:

   /// Generic constructor.
   DirectFuncApplicationImpl(FuncT _fn)
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


   virtual void 
   perform_evaluation_impl( const utilib::Any &domain,
                            const AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            AppResponse::response_map_t &responses )
   {
      derived_compute_response(domain, requests, responses, seed, fn);
   }

protected:
   ///
   FuncT fn;
};


///
template <class FuncT>
void derived_compute_response
(const utilib::Any& domain,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 FuncT fn)
{
   EXCEPTION_MNGR(std::runtime_error, "Undefined direct application");
}




#if !defined(DOXYGEN)

///
inline void derived_compute_response
(const utilib::Any& domain,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const utilib::Any& ,
                const AppRequest::request_map_t& ,
                AppResponse::response_map_t&,
                utilib::seed_t&))
{
   (*eval_fn)(domain, request_map, response_map, seed);
}




///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 double(*eval_fn)(DomainT&))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      const DomainT& pt = utilib::anyref_cast<DomainT>(point);
      response_map.insert( AppResponse::response_pair_t
                           ( f_info, (*eval_fn)(const_cast<DomainT&>(pt)) ) );
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 double(*eval_fn)(const DomainT&))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      response_map.insert( AppResponse::response_pair_t
                           ( f_info, (*eval_fn)(point.expose<DomainT>()) ) );
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, double&))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      double ans;
      (*eval_fn)(point.expose<DomainT>(), ans);
      tmp = ans;
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, double&))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      double ans;
      (*eval_fn)(point.expose<DomainT>(), ans);
      tmp = ans;
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, real&))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      const DomainT& pt = utilib::anyref_cast<DomainT>(point);
      (*eval_fn)(const_cast<DomainT&>(pt), tmp);
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, real&))
{
   AppRequest::request_map_t::const_iterator it;
   if ((it = request_map.find(f_info)) != request_map.end())
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      (*eval_fn)(point.expose<DomainT>(), tmp);
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT, class CArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, real&, CArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   if (((it = request_map.find(f_info)) != request_map.end()) ||
         ((it = request_map.find(nlcf_info)) != request_map.end()))
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      utilib::Any nlcf_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& ctmp = nlcf_ans.set<CArrayT>();
      (*eval_fn)(point.expose<DomainT>(), tmp, ctmp);
      response_map.insert(std::make_pair(f_info, f_ans));
      response_map.insert(std::make_pair(nlcf_info, nlcf_ans));
   }
   // As this problem type supports linear consrtaints, we MUST
   // return an empty lcf_info if asked for
   if ( request_map.find(lcf_info) != request_map.end() )
      response_map[lcf_info].set<CArrayT>();
}


///
template <class DomainT, class CArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, real&, CArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   if (((it = request_map.find(f_info)) != request_map.end()) ||
         ((it = request_map.find(nlcf_info)) != request_map.end()))
   {
      seed = 0; // this form doesn't support random seeds
      utilib::Any f_ans;
      utilib::Any nlcf_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& ctmp = nlcf_ans.set<CArrayT>();
      (*eval_fn)(point.expose<DomainT>(), tmp, ctmp);
      response_map.insert(std::make_pair(f_info, f_ans));
      response_map.insert(std::make_pair(nlcf_info, nlcf_ans));
   }
   // As this problem type supports linear consrtaints, we MUST
   // return an empty lcf_info if asked for
   if ( request_map.find(lcf_info) != request_map.end() )
      response_map.insert(AppResponse::response_pair_t(lcf_info, CArrayT()));
}


///
template <class DomainT, class CArrayT, class GArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, int, real&, CArrayT&, GArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   int ctr = 0;
   if ((it = request_map.find(f_info)) != request_map.end())
      ctr += 1;
   if ((it = request_map.find(nlcf_info)) != request_map.end())
      ctr += 2;
   if ((it = request_map.find(g_info)) != request_map.end())
      ctr += 4;

   if (ctr > 0)
   {
      seed = 0; // this form doesn't support random seeds

      /// JDS: you can't do the following: if you are using a caching
      /// system, this will result in inserting empty vectors into the
      /// cache if the function actually listened to the 'ctr' ASV.

      //real& tmp     = response_map[f_info].set<real>();
      //CArrayT& ctmp = response_map[nlcf_info].set<CArrayT>();
      //GArrayT& gtmp = response_map[g_info].set<GArrayT>();
      //(*eval_fn)(point.expose<DomainT>(),ctr,tmp,ctmp,gtmp);

      // JDS: instead, do the following:
      //   (it also has a side benefit of saving a copy)
      utilib::Any f_ans;
      utilib::Any cf_ans;
      utilib::Any g_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& ctmp = cf_ans.set<CArrayT>();
      GArrayT& gtmp = g_ans.set<GArrayT>();

      (*eval_fn)(point.expose<DomainT>(), ctr, tmp, ctmp, gtmp);

      if (ctr & 1)
         response_map.insert(AppResponse::response_pair_t(f_info, f_ans));
      if ((ctr & 2) || ! ctmp.empty())
         response_map.insert(AppResponse::response_pair_t(nlcf_info, cf_ans));
      if ((ctr & 4) || ! gtmp.empty())
         response_map.insert(AppResponse::response_pair_t(g_info, g_ans));
   }

   // As this problem type supports linear consrtaints, we MUST
   // return an empty lcf_info if asked for
   if ( request_map.find(lcf_info) != request_map.end() )
      response_map.insert(AppResponse::response_pair_t(lcf_info, CArrayT()));

}


///
template <class DomainT, class CArrayT, class GArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, int, real&, CArrayT&, GArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   int ctr = 0;
   if ((it = request_map.find(f_info)) != request_map.end())
      ctr += 1;
   if ((it = request_map.find(nlcf_info)) != request_map.end())
      ctr += 2;
   if ((it = request_map.find(g_info)) != request_map.end())
      ctr += 4;

   if (ctr > 0)
   {
      seed = 0; // this form doesn't support random seeds

      /// JDS: you can't do the following: if you are using a caching
      /// system, this will result in inserting empty vectors into the
      /// cache if the function actually listened to the 'ctr' ASV.

      //real tmp;
      //CArrayT ctmp;
      //GArrayT gtmp;
      //(*eval_fn)(point.expose<DomainT>(),ctr,tmp,ctmp,gtmp);
      //utilib::TypeManager()->lexical_cast(tmp, response_map[f_info].data);
      //utilib::TypeManager()->lexical_cast(ctmp, response_map[nlcf_info].data);
      //utilib::TypeManager()->lexical_cast(gtmp, response_map[g_info].data);

      // JDS: instead, do the following:
      //   (it also has a side benefit of saving a copy)
      utilib::Any f_ans;
      utilib::Any cf_ans;
      utilib::Any g_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& ctmp = cf_ans.set<CArrayT>();
      GArrayT& gtmp = g_ans.set<GArrayT>();

      (*eval_fn)(point.expose<DomainT>(), ctr, tmp, ctmp, gtmp);

      if (ctr & 1)
         response_map.insert(AppResponse::response_pair_t(f_info, f_ans));
      if ((ctr & 2) || ! ctmp.empty())
         response_map.insert(AppResponse::response_pair_t(nlcf_info, cf_ans));
      if ((ctr & 4) || ! gtmp.empty())
         response_map.insert(AppResponse::response_pair_t(g_info, g_ans));
   }

   // As this problem type supports linear consrtaints, we MUST
   // return an empty lcf_info if asked for
   if ( request_map.find(lcf_info) != request_map.end() )
      response_map.insert(AppResponse::response_pair_t(lcf_info, CArrayT()));
}


///
template <class DomainT, class CArrayT, class GArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, int, real&, CArrayT&, CArrayT&, GArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   int ctr = 0;
   if ((it = request_map.find(f_info)) != request_map.end())
      ctr += 1;
   if ((it = request_map.find(nlcf_info)) != request_map.end())
      ctr += 2;
   if ((it = request_map.find(g_info)) != request_map.end())
      ctr += 4;
   if ((it = request_map.find(lcf_info)) != request_map.end())
      ctr += 8;

   if (ctr > 0)
   {
      seed = 0; // this form doesn't support random seeds

      /// JDS: you can't do the following: if you are using a caching
      /// system, this will result in inserting empty vectors into the
      /// cache if the function actually listened to the 'ctr' ASV.

      //real& tmp     = response_map[f_info].set<real>();
      //CArrayT& ctmp = response_map[cf_info].set<CArrayT>();
      //GArrayT& gtmp = response_map[g_info].set<GArrayT>();
      //(*eval_fn)(point.expose<DomainT>(),ctr,tmp,ctmp,gtmp);

      // JDS: instead, do the following:
      //   (it also has a side benefit of saving a copy)
      utilib::Any f_ans;
      utilib::Any cf_ans;
      utilib::Any lcf_ans;
      utilib::Any g_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& lctmp = lcf_ans.set<CArrayT>();
      CArrayT& nlctmp = cf_ans.set<CArrayT>();
      GArrayT& gtmp = g_ans.set<GArrayT>();

      (*eval_fn)(point.expose<DomainT>(), ctr, tmp, lctmp, nlctmp, gtmp);

      if (ctr & 1)
         response_map.insert(AppResponse::response_pair_t(f_info, f_ans));
      if ((ctr & 2) || ! nlctmp.empty())
         response_map.insert(std::make_pair(nlcf_info, cf_ans));
      if ((ctr & 8) || ! lctmp.empty())
         response_map.insert(std::make_pair(lcf_info, lcf_ans));
      if ((ctr & 4) || ! gtmp.empty())
         response_map.insert(std::make_pair(g_info, g_ans));
   }
}

///
template <class DomainT, class CArrayT, class GArrayT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, int, real&, CArrayT&, CArrayT&, GArrayT&))
{
   AppRequest::request_map_t::const_iterator it;

   int ctr = 0;
   if ((it = request_map.find(f_info)) != request_map.end())
      ctr += 1;
   if ((it = request_map.find(nlcf_info)) != request_map.end())
      ctr += 2;
   if ((it = request_map.find(g_info)) != request_map.end())
      ctr += 4;
   if ((it = request_map.find(lcf_info)) != request_map.end())
      ctr += 8;

   if (ctr > 0)
   {
      seed = 0; // this form doesn't support random seeds

      /// JDS: you can't do the following: if you are using a caching
      /// system, this will result in inserting empty vectors into the
      /// cache if the function actually listened to the 'ctr' ASV.

      //real& tmp     = response_map[f_info].set<real>();
      //CArrayT& ctmp = response_map[cf_info].set<CArrayT>();
      //GArrayT& gtmp = response_map[g_info].set<GArrayT>();
      //(*eval_fn)(point.expose<DomainT>(),ctr,tmp,ctmp,gtmp);

      // JDS: instead, do the following:
      //   (it also has a side benefit of saving a copy)
      utilib::Any f_ans;
      utilib::Any nlcf_ans;
      utilib::Any lcf_ans;
      utilib::Any g_ans;
      real& tmp     = f_ans.set<real>();
      CArrayT& lctmp = lcf_ans.set<CArrayT>();
      CArrayT& nlctmp = nlcf_ans.set<CArrayT>();
      GArrayT& gtmp = g_ans.set<GArrayT>();

      (*eval_fn)(point.expose<DomainT>(), ctr, tmp, lctmp, nlctmp, gtmp);

      if (ctr & 1)
         response_map.insert(std::make_pair(f_info, f_ans));
      if ((ctr & 2) || ! nlctmp.empty())
         response_map.insert(std::make_pair(nlcf_info, nlcf_ans));
      if ((ctr & 8) || ! lctmp.empty())
         response_map.insert(std::make_pair(lcf_info, lcf_ans));
      if ((ctr & 4) || ! gtmp.empty())
         response_map.insert(std::make_pair(g_info, g_ans));
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(DomainT&, real&, utilib::seed_t))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      (*eval_fn)(const_cast<DomainT&>(point.expose<DomainT>()), tmp, seed);
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}


///
template <class DomainT>
void derived_compute_response
(const utilib::Any& point,
 const AppRequest::request_map_t& request_map,
 AppResponse::response_map_t& response_map,
 utilib::seed_t &seed,
 void(*eval_fn)(const DomainT&, real&, utilib::seed_t))
{
   AppRequest::request_map_t::const_iterator it;

   if ((it = request_map.find(f_info)) != request_map.end())
   {
      utilib::Any f_ans;
      real& tmp = f_ans.set<real>();
      (*eval_fn)(point.expose<DomainT>(), tmp, seed);
      response_map.insert(std::make_pair(f_info, f_ans));
   }
}




}

#endif
#endif
