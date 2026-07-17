/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** \file
    Python module wrapping DI/library-mode Dakota study construction.
 */

#include "DakotaStudyPython.hpp"

namespace Dakota::python {

PYBIND11_MODULE(_study, m)
{
  m.doc() = "Dependency-injection/library-mode Dakota study construction";

  bind_study_config(m);
  bind_variables(m);
  bind_response(m);
  bind_interface(m);
  bind_models(m);
  bind_iterators(m);
  bind_study_factories(m);
  bind_study(m);
}

} // namespace Dakota::python
