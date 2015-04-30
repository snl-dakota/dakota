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
 * \file Application_Hessian.h
 *
 * Defines the colin::Application_Hessian class.
 */

#ifndef colin_Application_Hessian_h
#define colin_Application_Hessian_h

#include <acro_config.h>
#include <colin/application/Base.h>

namespace colin
{

//============================================================================
//============================================================================
// Class Application_Hessian
//============================================================================
//============================================================================

/**
 *  Defines the elements of an Application that pertain to real parameters.
 */
class Application_Hessian : virtual public Application_Base
{
public:
   /// Constructor
   Application_Hessian();

   /// Virtual destructor
   virtual ~Application_Hessian();

   /// Register Hessian evaluation computations
   DECLARE_REQUEST(H);

protected: // methods

};



template <bool active>
class Application_Include_Hessian
   : public Application_Hessian
{};

template<>
class Application_Include_Hessian<false>
{};

} // namespace colin

#endif // defined colin_Application_Hessian_h
