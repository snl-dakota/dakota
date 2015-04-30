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
 * \file Application_Hessian.cpp
 *
 * Defines the colin::Application_Hessian class members.
 */

#include <colin/application/Hessian.h>

using std::cerr;
using std::endl;
using std::vector;

namespace colin {

//--------------------------------------------------------------------
// Application_Hessian Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_Hessian::Application_Hessian()
{
   register_application_component(ObjectType::get(this));
}


/// Virtual destructor
Application_Hessian::~Application_Hessian()
{}


/// Register Hessian matrix evaluation
DEFINE_REQUEST(Application_Hessian, H, h_info, Hessian );


} // namespace colin
