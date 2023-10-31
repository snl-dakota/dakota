/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "NonDLHSSingleInterval.hpp"
#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"

//#define DEBUG

namespace Dakota {


NonDLHSSingleInterval::
NonDLHSSingleInterval(ProblemDescDB& problem_db, Model& model):
  NonDLHSInterval(problem_db, model)
{ }


NonDLHSSingleInterval::~NonDLHSSingleInterval()
{ }


void NonDLHSSingleInterval::initialize()
{ statCntr = 0; }


void NonDLHSSingleInterval::post_process_samples()
{
  const IntResponseMap& all_responses = lhsSampler.all_responses();
  Real lwr, upr; IntRespMCIter r_it;
  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {
    Cout << ">>>>> Identifying minimum and maximum samples for response "
	 << "function " << respFnCntr+1 << '\n';
    r_it = all_responses.begin();
    lwr = upr = r_it->second.function_value(respFnCntr); ++r_it;
    for (; r_it!=all_responses.end(); ++r_it) {
      const Real& fn_val = r_it->second.function_value(respFnCntr);
      if (fn_val < lwr)
	lwr = fn_val;
      else if (fn_val > upr)
	upr = fn_val;
    }
    finalStatistics.function_value(lwr, statCntr++);
    finalStatistics.function_value(upr, statCntr++);
  }
}

} // namespace Dakota
