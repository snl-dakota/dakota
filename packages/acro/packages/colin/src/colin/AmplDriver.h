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
 * \file AmplDriver.h
 *
 * Defines the colin::AmplDriver class.
 */

#ifndef colin_AmplDriver_h
#define colin_AmplDriver_h

namespace colin {

/// Standard driver for running COLIN through AMPL
int AMPL_driver(int argc, char* argv[], const char* codename);

} // namespace colin

#endif // colin_AmplDriver_h
