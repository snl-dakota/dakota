/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file traits.h
 *
 * Defines a macro used to set object traits.
 */

#ifndef utilib_traits_h
#define utilib_traits_h

/// Set a trait in a given namespace with a given type and value.
#define SetTrait(nspace,trait,type,val)\
namespace nspace { \
template <> \
struct trait < type > { \
public: \
static const bool value = val; \
}; }

#endif
