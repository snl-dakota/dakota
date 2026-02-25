/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_PYTHON_UTILS_H
#define DAKOTA_PYTHON_UTILS_H

#include "dakota_data_util.hpp"
#include <pybind11/numpy.h>
namespace py = pybind11;

namespace Dakota {
    
    namespace PythonUtils {

      /// copy Dakota arrays to pybind11 lists or numpy arrays
      template<typename RetT, class ArrayT, typename T>
        RetT copy_array_to_pybind11(const ArrayT & src)
        {
          std::vector<T> tmp_vec;
          for( auto const & a : src )
            tmp_vec.push_back(a);
          return py::cast(tmp_vec);
        }

      /// specialized copy Dakota arrays to pybind11 lists or numpy arrays
      template<typename RetT, class O, class S>
        RetT copy_array_to_pybind11(const Teuchos::SerialDenseVector<O,S> & src)
        {
          std::vector<S> tmp_vec;
          copy_data(src, tmp_vec);
          return py::cast(tmp_vec);
        }

      bool check_for_attr(py::object & pyObj, const std::string& attr,
                          std::string name="");
    }

    inline bool PythonUtils::check_for_attr(py::object & pyObj, const std::string& attr,
                                            std::string name)
    {
      try {
        py::object py_chk = pyObj.attr(attr.c_str());
      }
      catch(py::error_already_set &e) {
        if (e.matches(PyExc_AttributeError)) {
          if( !name.empty() )
            std::cout << "Module '" << name << "' does not "
                      << "contain method '" << attr << "'"
                      << std::endl;
        }
        return false;;
      }
      return true;
    }
}

#endif
