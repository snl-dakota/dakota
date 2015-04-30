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

#include <acro_config.h>
#include <utilib/BasicArray.h>
#include <scolib/DomainOpsBase.h>
#include <scolib/DomainInfoBase.h>
#include <scolib/EAgeneric.h>


int main()
{
scolib::EAgeneric<utilib::BasicArray<double>,
		  scolib::DomainInfoBase,
		  scolib::DomainOpsBase<utilib::BasicArray<double>,scolib::DomainInfoBase > > ea;

return 0;
}
