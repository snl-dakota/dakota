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
 * \file AppRequest.h
 *
 * Defines the colin::AppRequest class.
 */

#ifndef colin_AppRequest_h
#define colin_AppRequest_h

#include <acro_config.h>
#include <colin/AppResponseInfo.h>

#include <utilib/Any.h>
#include <utilib/RNG.h>

namespace colin
{

class Application_Base;
class ResponseGenerator;
class AppResponse;


/**
 *  The class \c AppRequest provides a class for managing the interface
 *  for requesting Response calculations for a candidate design point.
 *  This class defines a reference-counted object where the actual
 *  object data is stored in the AppRequest::Implementation structure.
 */
class AppRequest 
{
   friend class Application_Base;       // for adding request components
   friend class ResponseGenerator;      // converting requests->responses
   friend class AppResponse;            // to help convert to response

public: // typedefs
   /// The fundamental structure for storing application request information
   typedef std::map <response_info_t, utilib::Any>  request_map_t;
   /// A non-const pair to support app_request_map_t
   typedef std::pair<response_info_t, utilib::Any>  request_pair_t;

public: // construct / destruct / copy / compare methods

   /// Generic constructor
   AppRequest()
      : data(NULL)
   {}

   /// Copy constructor
   AppRequest(const AppRequest& rhs)
      : data(NULL)
   { *this = rhs; }

   /// Destructor
   ~AppRequest();

   /// Standard assignment operator: perform a shallow copy
   AppRequest& operator=(const AppRequest& rhs);

   /// Returns true if there is no allocated request data object.
   bool empty() const
   { return data == NULL; }


public: // methods

   /// Return a copy of this request that uses a different seed
   AppRequest replicate(utilib::seed_t new_seed);

   /// Finalize this request (Transform requests through the application chain)
   void finalize_request();


   /// Return the domain in the context of the specified Application
   utilib::Any domain(Application_Base* app = NULL) const;

   /// Return a locally-unique ID for this request
   app_request_id_t id() const;

   /// Return the seed that this request will use
   utilib::seed_t seed() const;

   /// Return the number of components this request wants calculated
   size_t size() const;

   /// Did the user request a forced recalculation?
   bool forced_recalc() const;

   /// Is this request finalized (i.e. transformed & no new requests allowed)?
   bool finalized() const;

private: // data

   /// A container for all AppRequest member data
   struct Implementation;
   /// Intermediate info for each application in the transformation path
   struct ApplicationInfo;

   /// Pointer to the reference-counted (singleton) data for this AppRequest
   Implementation * data;


private: // methods

   /// Create a new request for a given domain point
   AppRequest( utilib::Any domain,
               const bool recalculate = false,
               const utilib::seed_t seed = 0 );

   /// Add an inner application to the request transformation chain
   ApplicationInfo& add_reformulation(const Application_Base* app);

   /// Get the outermost application (i.e. the original requestor)
   const Application_Base* application() const;

};

} // namespace colin

#endif
