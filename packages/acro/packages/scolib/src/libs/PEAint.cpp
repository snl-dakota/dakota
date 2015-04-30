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


//
// PEAint.cpp
//

#include <acro_config.h>
#include <scolib/PEAint.h>

namespace scolib {


PEAint::PEAint() 
  : EAbase<EApoint_array<int>, BasicArray<int> >(),
    EAops_intarray(),
    PEAbase<EApoint_array<int>, BasicArray<int> >()
{ }


void PEAreal::write(ostream& os) const
{
PEAbase<EApoint_array<int>, BasicArray<int> >::write(os);
EAops_real::write(os);
}


void PEAreal::reset() 
{
if (popsize() == 0)
   init_population(100);
PEAbase<EApoint_array<int>, BasicArray<int> >::reset(); 
EAops_intarray::reset(); 
}

} // namespace scolib
