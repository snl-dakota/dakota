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
 * \file real.h
 *
 * Typedef for colin::real.
 */

#ifndef colin_real_h
#define colin_real_h

#include <acro_config.h>
#include <utilib/Ereal.h>

namespace colin
{

/// The 'real' type used within COLIN.
typedef utilib::Ereal<double> real;

}

#endif
