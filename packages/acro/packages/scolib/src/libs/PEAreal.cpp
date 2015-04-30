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
// PEAreal.cpp
//

#include <acro_config.h>
#include <scolib/PEAreal.h>

using namespace utilib;
using namespace std;

namespace scolib {


PEAreal::PEAreal() 
  : 
    EAbase<EApoint_real, NumArray<double> >(),
    PEAbase<EApoint_real, NumArray<double> >(),
    EAops_real()
{ }


void PEAreal::write(ostream& os) const
{
PEAbase<EApoint_real, NumArray<double> >::write(os);
EAops_real::write(os);
}


void PEAreal::reset() 
{
if (popsize() == 0)
   init_population(100);
PEAbase<EApoint_real, NumArray<double> >::reset(); 
EAops_real::reset(); 
}

} // namespace scolib
