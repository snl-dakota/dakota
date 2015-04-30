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
 * \file ContextMngr.h
 *
 * Function wrapper for managing optimization contexts.
 */

#ifndef colin_ContextMngr_h
#define colin_ContextMngr_h

#include <acro_config.h>
#include <utilib/TypeManager.h>

namespace colin
{

/// Global singleton for storing colin-specific type conversions
//utilib::Type_Manager& ContextMngr();


/// Global singleton for converting among Problem types
utilib::Type_Manager& ProblemMngr();

}

#endif
