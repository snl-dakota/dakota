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

#ifndef colin_ColinDriver_h
#define colin_ColinDriver_h

#include <string> 

namespace colin
{

int colin_driver(int argc, char* argv[], 
                 const std::string codename="colin", 
                 const std::string version="");

};

#endif
