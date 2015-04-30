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
 * \file AsynchronousApplication.h
 *
 * Defines the colin::AsynchronousApplication class.
 */


#ifndef colin_AsynchronousApplication_h
#define colin_AsynchronousApplication_h

#include <colin/application/Base.h>

namespace colin {

     
/** This provides an interface that mimics the synchronous COLIN
 *  application API for applications that only support a asynchronous
 *  interface.
 */
class AsynchronousApplication : virtual public Application_Base
{
public:

   ///
   AsynchronousApplication()
   {}

   ///
   virtual ~AsynchronousApplication()
   {}

protected:

   /// This queues an evaluation and spin-locks until it completes
   virtual void
   perform_evaluation_impl( const utilib::Any &domain,
                            const AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            AppResponse::response_map_t &responses );
   
   /// Check the local buffer first, then delegate to the actual collect impl
   virtual utilib::Any 
   collect_evaluation_impl( AppResponse::response_map_t &responses,
                            utilib::seed_t &seed);
   

   /// True if a spawned evaluation is "available" (or previously collected)
   virtual bool evaluation_available()
   { return (! evaluatedBuffer.empty()) || async_evaluation_available(); }


   virtual utilib::Any 
   async_collect_evaluation( AppResponse::response_map_t &responses, 
                             utilib::seed_t &seed ) = 0;

   virtual bool 
   async_evaluation_available() = 0;

private:
   ///
   struct Evaluation {
      Evaluation( utilib::Any _evalID,
                  utilib::seed_t &_seed,
                  AppResponse::response_map_t &_responses )
         : evalID(_evalID), 
           seed(_seed), 
           responses(_responses)
      {}

      utilib::Any  evalID;
      utilib::seed_t  seed;
      AppResponse::response_map_t responses;
   };

   ///
   std::list<Evaluation> evaluatedBuffer;
};


} // namespace colin

#endif // defined colin_AsynchronousApplication_h
