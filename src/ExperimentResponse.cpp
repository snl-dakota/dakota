/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ExperimentResponse
//- Description:  Class implementation
//- Owner:        Laura Swiler

#include "ExperimentResponse.hpp"
#include "DakotaVariables.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: ExperimentResponse.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

ExperimentResponse::
ExperimentResponse(const Variables& vars, const ProblemDescDB& problem_db):
  Response(BaseConstructor(), vars, problem_db)
{
  sharedRespData.response_type(EXPERIMENT_RESPONSE);

  // initialize the response covariance data...
}


ExperimentResponse::
ExperimentResponse(const SharedResponseData& srd):
  Response(BaseConstructor(), srd)
{
  // initialize the response covariance data...
}


ExperimentResponse::ExperimentResponse(const ActiveSet& set):
  Response(BaseConstructor(), set)
{
  sharedRespData.response_type(EXPERIMENT_RESPONSE);

  // initialize the response covariance data...
}

} // namespace Dakota
