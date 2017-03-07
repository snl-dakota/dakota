/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptTraits
//- Description: Implementation code for the OptTraits class
//- Owner:       Moe Khalil
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "DakotaOptTraits.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"

static const char rcsId[]="@(#) $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $";


namespace Dakota {


/** Sets supportsMultiobjectives to true */
void OptTraits::set_supports_multiobjectives()
{
  supportsMultiobjectives = true;
}


/** Return the value of supportsMultiobjectives */
void OptTraits::get_supports_multiobjectives(bool supports_multiobjectives)
{
  supports_multiobjectives = supportsMultiobjectives;
}

} // namespace Dakota
