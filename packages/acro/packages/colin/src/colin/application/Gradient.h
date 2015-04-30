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
 * \file Application_Gradient.h
 *
 * Defines the colin::Application_Gradient class.
 */

#ifndef colin_Application_Gradient_h
#define colin_Application_Gradient_h

#include <acro_config.h>
#include <colin/application/Base.h>

namespace colin
{

//============================================================================
//============================================================================
// Class Application_Gradient
//============================================================================
//============================================================================


/**
 *  Defines the elements of an Application that pertain to problems
 *  where objective gradient information is available.
 */
class Application_Gradient : virtual public Application_Base
{
public:
   /// Constructor
   Application_Gradient();

   /// Virtual destructor
   virtual ~Application_Gradient();

   /// Register gradient evaluation computations
   DECLARE_REQUEST(G);

protected: // methods

};



template <bool active>
class Application_Include_Gradient
   : public Application_Gradient
{};

template<>
class Application_Include_Gradient<false>
{};

} // namespace colin

#endif // defined colin_Application_Gradient_h
