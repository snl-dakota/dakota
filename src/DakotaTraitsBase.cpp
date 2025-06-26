/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaTraitsBase.hpp"

#include "DakotaModel.hpp"
#include "DataTransformModel.hpp"
#include "PRPMultiIndex.hpp"
#include "ParamResponsePair.hpp"
#include "ScalingModel.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"

static const char rcsId[] =
    "@(#) $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $";

namespace Dakota {

TraitsBase::TraitsBase() { /* empty ctor */ }

}  // namespace Dakota
