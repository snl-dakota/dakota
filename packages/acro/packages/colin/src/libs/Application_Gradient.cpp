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
 * \file Application_Gradient.cpp
 *
 * Defines the colin::Application_Gradient class members.
 */

#include <colin/application/Gradient.h>

using std::cerr;
using std::endl;
using std::vector;

namespace colin {



//--------------------------------------------------------------------
// Application_Gradient Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_Gradient::Application_Gradient()
{
   register_application_component(ObjectType::get(this));
}


/// Virtual destructor
Application_Gradient::~Application_Gradient()
{}


/// Register gradient evaluation
DEFINE_REQUEST(Application_Gradient, G, g_info, gradient);



} // namespace colin
