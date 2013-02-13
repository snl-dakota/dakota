/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPOFDarts
//- Description: Class for the Probability of Failure DARTS approach
//- Owner:       Mohamed Ebeida and Laura Swiler
//- Checked by:
//- Version:

#include "NonDPOFDarts.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "pecos_stat_util.hpp"

//#define DEBUG

static const char rcsId[] = "@(#) $Id: NonDPOFDart.cpp 6080 2009-09-08 19:03:20Z gtang $";

namespace Dakota {

NonDPOFDarts::NonDPOFDarts(Model& model): NonD(model)
{
  // any initialization is done here.   For now, you should just specify 
  // the number of samples, but eventually we will get that from the input spec

}

NonDPOFDarts::~NonDPOFDarts()
{}


void NonDPOFDarts::quantify_uncertainty()
{
}


} // namespace Dakota

