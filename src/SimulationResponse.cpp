/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SimulationResponse
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "SimulationResponse.hpp"
#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: SimulationResponse.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

SimulationResponse::
SimulationResponse(const Variables& vars, const ProblemDescDB& problem_db):
  Response(BaseConstructor(), vars, problem_db)
{ sharedRespData.response_type(SIMULATION_RESPONSE); }


SimulationResponse::
SimulationResponse(const SharedResponseData& srd, const ActiveSet& set):
  Response(BaseConstructor(), srd, set)
{ }


SimulationResponse::
SimulationResponse(const SharedResponseData& srd):
  Response(BaseConstructor(), srd)
{ }


SimulationResponse::SimulationResponse(const ActiveSet& set):
  Response(BaseConstructor(), set)
{ sharedRespData.response_type(SIMULATION_RESPONSE); }

} // namespace Dakota
