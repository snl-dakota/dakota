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

#include <colin/ColinDriver.h>

//FACTORY_REFERENCE(rmoo)
//FACTORY_REFERENCE(ls)
//FACTORY_REFERENCE(gd)

int main(int argc, char* argv[])
{
   return colin::colin_driver(argc, argv, "colintest");
}
