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
 * \file AppResponse.h
 *
 * Defines the colin::AppResponse class.
 */

#ifndef colin_AppResponse_h
#define colin_AppResponse_h

#include <acro_config.h>

#include <colin/AppResponseInfo.h>

#include <utilib/exception_mngr.h>
#include <utilib/RNG.h>

namespace colin {
class AppRequest;
class AppResponse;
} // namespace colin

/// Support "printing" AppResponse objects
std::ostream& operator<<(std::ostream& os, const colin::AppResponse& r);


namespace colin
{
class Application_Base;
class ResponseGenerator;

/**
 *  The class \c AppResponse provides a class for managing the interface
 *  to data about a candidate design point.  This class defines a
 *  reference-counted object where the actual object data is stored in
 *  the AppResponse::Implementation structure.
 */
class AppResponse
{
   friend class ResponseGenerator;
   friend std::ostream& ::operator<<(std::ostream& os, const AppResponse& r);

public: // typedefs

   ///
   typedef std::pair<response_info_t, const utilib::Any >  response_pair_t;

   ///
   typedef std::map<response_info_t, const utilib::Any >  response_map_t;

public: // construct / destruct / copy / compare methods

   /// Generic constructor
   AppResponse()
      : data(NULL)
   {}
   
   /// Copy constructor
   AppResponse(const AppResponse& rhs)
      : data(NULL)
   { *this = rhs; }

   /// Destructor
   virtual ~AppResponse();


   /// Standard assignment operator: perform a shallow copy
   AppResponse& operator=(const AppResponse& rhs);

   /// Returns true if there is no allocated response data object.
   bool empty() const
   { return this->data == NULL; }

   /// Return true if rhs AppResponse refer to the same AppResponse
   bool operator==(const AppResponse &rhs) const
   { return (( this->data != NULL ) && ( this->data == rhs.data )); }

public: // methods

   /// Return a locally-unique id (equal to the ID from original request)
   app_request_id_t id() const;

   /// Return the random seed used to compute this response
   utilib::seed_t seed() const;

   /// Return the number of calculated components in this response
   size_t size(const Application_Base* app = NULL) const;

   /// Get domain point
   const utilib::Any get_domain(const Application_Base* app = NULL) const;

   /// Get domain point
   void get_domain( utilib::AnyFixedRef point,
                    const Application_Base* app = NULL ) const;

   /// True if this Response object contains a value for Response Info type
   bool is_computed( response_info_t id, 
                     const Application_Base* app = NULL ) const;

   /// Return a list of all response_info_t in this response
   //void computed( std::list<response_info_t> &ids, 
   //               const Application_Base* app = NULL ) const;

   /// Get the response data and return it in an Any object
   const utilib::Any get( response_info_t id,
                          const Application_Base* app = NULL ) const;

   /// Get the response data and store it into a user-defined data type
   void get( response_info_t id, utilib::AnyFixedRef value,
             const Application_Base* app = NULL ) const;

   /// Get the response data and store it into a user-defined data type
   const response_map_t get( const Application_Base* app = NULL ) const;

   /// Get the response data and return it as the specified type
   template <class DataT>
   DataT value(response_info_t id, Application_Base* app = NULL) const
   {
      DataT val;
      get(id, val, app);
      return val;
   }
   

private: // data

   /// The actual data for this AppResponse object
   struct Implementation;
   /// Intermediate info for each application in the transformation path
   struct ApplicationInfo;

   /// Pointer to the reference-counted (singleton) data for this AppResponse
   Implementation *data;

private: // methods

   /// Generic constructor
   AppResponse(const AppRequest &request,
               const AppResponse::response_map_t &raw_resp,
               const utilib::Any &raw_domain);

   /// Transform the responses from the innermost application through the chain
   void transform_response();
};

} // namespace colin


#endif
