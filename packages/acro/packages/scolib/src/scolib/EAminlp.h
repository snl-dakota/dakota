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
 * \file EAminlp.h
 *
 * Defines the scolib::EAminlp typedef.
 */

#ifndef scolib_EAminlp_h
#define scolib_EAminlp_h

#include <acro_config.h>
#include <scolib/EAgeneric.h>
#include <scolib/DomainOpsMixedInteger.h>
#include <scolib/DomainInfoMixedInteger.h>

namespace scolib {

///
template <>
class EAgeneric_name<utilib::MixedIntVars,
	colin::UMINLP0_problem,
        scolib::DomainInfoMixedInteger,
        scolib::DomainOpsMixedInteger<scolib::DomainInfoMixedInteger> >
{ protected: const char* get_name() const { return "EAminlp"; } };


///
typedef scolib::EAgeneric<utilib::MixedIntVars, 
	colin::UMINLP0_problem,
	scolib::DomainInfoMixedInteger,
	scolib::DomainOpsMixedInteger<scolib::DomainInfoMixedInteger> > EAminlp;

}

#endif
