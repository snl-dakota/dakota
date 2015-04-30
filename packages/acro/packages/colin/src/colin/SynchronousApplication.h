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
 * \file SynchronousApplication.h
 *
 * Defines the colin::SynchronousApplication class.
 */


#ifndef colin_SynchronousApplication_h
#define colin_SynchronousApplication_h

#include <colin/Application.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace colin {

class SynchronousApplication_impl
{
   typedef boost::function<void( const utilib::Any&, 
                                 const AppRequest::request_map_t &,
                                 utilib::seed_t&,
                                 AppResponse::response_map_t&
                                 )>  performEval_ptr;

public:
   SynchronousApplication_impl();

   virtual ~SynchronousApplication_impl();

   virtual utilib::Any
   spawn_evaluation( const utilib::Any &domain,
                     const AppRequest::request_map_t &requests,
                     utilib::seed_t &seed );
      
   /// Execute and return the next evaluation in the pending list
   virtual utilib::Any
   collect_evaluation( AppResponse::response_map_t &responses,
                       utilib::seed_t &seed,
                       performEval_ptr performEval);
   
   /// This will return true if there are any spawned evaluations
   virtual bool evaluation_available();
      
private:
   ///
   struct Data;
   ///
   Data * data;
};


/** This provides an interface that mimics the asynchronous COLIN
 *  application API for applications that only support a synchronous
 *  interface.
 */
template<typename TYPE>
class Synchronous : public TYPE
{
public:

   ///
   Synchronous()
   {}

   ///
   virtual ~Synchronous()
   {}

protected:

   /// This queues an "asynchronous" evaluation to be performed later
   virtual utilib::Any 
   spawn_evaluation_impl( const utilib::Any &domain,
                          const AppRequest::request_map_t &requests,
                          utilib::seed_t &seed )
   { return impl.spawn_evaluation(domain, requests, seed); }

   /// Execute and return the next evaluation in the pending list
   virtual utilib::Any 
   collect_evaluation_impl( AppResponse::response_map_t &responses,
                            utilib::seed_t &seed)
   { 
      return impl.collect_evaluation
         ( responses, seed, 
           boost::bind( &Synchronous::perform_evaluation, 
                        this, _1, _2, _3, _4 ) ); 
   }

   /// This will return true if there are any spawned evaluations
   virtual bool evaluation_available()
   { return impl.evaluation_available(); }

private:
   SynchronousApplication_impl impl;
};


} // namespace colin

#endif // defined colin_SynchronousApplication_h
