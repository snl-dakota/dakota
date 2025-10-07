/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "python_utils.hpp"

//namespace Dakota {
//
//bool PythonUtils::check_for_attr(py::object & pyObj, const std::string& attr,
//                                 std::string name)
//{
//  try {
//    py::object py_chk = pyObj.attr(attr.c_str());
//  }
//  catch(py::error_already_set &e) {
//    if (e.matches(PyExc_AttributeError)) {
//      if( !name.empty() )
//        std::cout << "Module '" << name << "' does not "
//                  << "contain method '" << attr << "'"
//                  << std::endl;
//    }
//    return false;;
//  }
//  return true;
//}
//}
