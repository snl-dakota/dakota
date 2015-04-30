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
 * \file OptResponse.h
 *
 * Defines the colin::OptResponse class.
 */

#ifndef __colin_OptResponse_h
#define __colin_OptResponse_h

#include <acro_config.h>

#include <colin/StatusCodes.h>
#include <colin/AppResponse.h>
#include <colin/real.h>

#include <utilib/enum_def.h>
#include <utilib/PackObject.h>

namespace colin
{

using utilib::PackBuffer;
using utilib::UnPackBuffer;

/// A response object for optimization.
class OptResponse : public utilib::PackObject
{
public:

   /// Constructor.
   OptResponse()
   { values.resize(1); reset(); }

   /// Destructor.
   virtual ~OptResponse()
   {}

   /// Reset the data in this response.
   void reset()
   {
      value() = real::positive_infinity;
      // BUG: WEH - the value of constraint violation should be
      // 	set to infinity, but right now we cannot easily
      //	evaluate constraint violations.   When the
      //	constraint management is resolved, we can
      //	do that directly.
      //constraint_violation = real::positive_infinity;
      constraint_violation = 0.0;
      response = AppResponse();
      //response->set(f_info, 0, real::positive_infinity); // AppResponseContainer used here!
      termination_info = "Unknown";
      model_status = model_error_unknown;
      solver_status = solver_unknown;
      termination_condition = termination_unknown;
   }

   /// The final/best point.
   utilib::Any point;

   /// The application response for this point.
   AppResponse response;

   /// The value of this point.
   real& value()
   { return values[0]; }

   /// The value of this point.
   const real& value() const
      { return values[0]; }

   /// The value of this point.
   std::vector<real> values;

   /// The constraint violation for this point.
   real constraint_violation;

   /// Termination information for the solver.
   std::string termination_info;

   /// Name of 'other' termination type
   std::string termination_other;

   /// The status of model use.
   model_status_enum model_status;

   /// The status of solver use.
   solver_status_enum solver_status;

   /// The termination_condition of the solver
   termination_condition_enum termination_condition;

   /// Write this object
   void write(std::ostream& os) const;

   /// Read this object
   void read(std::istream& is);

   /// Pack this object
   void write(PackBuffer& os) const;

   /// Unpack this object
   void read(UnPackBuffer& is);

};

inline void OptResponse::write(std::ostream& os) const
{
   static_cast<void>(os);
   EXCEPTION_MNGR(std::runtime_error, "Not currently supported.");
#if 0
   os << point << std::endl;
   os << response << std::endl;
   if (values.size() > 0)
   {
      os << values << " " << constraint_violation << std::endl;
   }
   else
   {
      os << value() << " " << constraint_violation << std::endl;
   }
   os << termination_info << " " << model_status << " " << solver_status << std::endl;
#endif
}

inline void OptResponse::read(std::istream& is)
{
   static_cast<void>(is);
   EXCEPTION_MNGR(std::runtime_error, "Not currently supported.");
//is >> point >> response >> values >> constraint_violation
   //>> termination_info >> model_status >> solver_status;
}

inline void OptResponse::write(PackBuffer& os) const
{
   static_cast<void>(os);
   EXCEPTION_MNGR(std::runtime_error, "Not currently supported.");
//os << point << response << values << constraint_violation
   //<< termination_info;
//os << model_status << solver_status;
}

inline void OptResponse::read(UnPackBuffer& is)
{
   static_cast<void>(is);
   EXCEPTION_MNGR(std::runtime_error, "Not currently supported.");
//is >> point >> response >> values >> constraint_violation
   //>> termination_info >> model_status >> solver_status;
}

} // namespace colin


/// Copy colin::AppResponse information into a colin::OptResponse object.
inline colin::OptResponse& operator<< 
(colin::OptResponse& x, const colin::AppResponse& info)
{
   x.point = info.get_domain();
   x.response = info;
   if (info.is_computed(colin::mf_info))
      info.get(colin::mf_info, x.values);
   else
   {
      x.values.resize(1);
      info.get(colin::f_info, x.values[0]);
   }
// TODO - reconsistute the constraint violation mechanism
//x.constraint_violation = info->l2_constraint_violation();
   return x;
}


#endif
