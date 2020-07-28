/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file
    Python module wrapping surrogates modules
 */

#include "SurrogatesGaussianProcess.hpp"
#include "SurrogatesPolynomialRegression.hpp"

#include <pybind11/eigen.h> 
#include <pybind11/pybind11.h>

namespace py = pybind11;

using dakota::ParameterList;


/// Convert Python dictionary to options list
ParameterList convert_options(pybind11::dict pydict)
{
  ParameterList pl;
  // this assumes di.first is convertible to string and di.second is
  // convertible to a C++ type
  // This isn't there yet, as the second will have type pybind11::handle
  for (const auto &di : pydict) {
    auto key = di.first.cast<std::string>();
    auto value = di.second;
    
    // basic data types
    if (pybind11::isinstance<pybind11::bool_>(value))
      pl.set(key, value.cast<bool>());
    else if (pybind11::isinstance<pybind11::int_>(value))
      pl.set(key, value.cast<int>());
    else if (pybind11::isinstance<pybind11::float_>(value))
      pl.set(key, value.cast<double>());
    else if (pybind11::isinstance<pybind11::str>(value))
      pl.set(key, value.cast<std::string>());
    // TODO: add general tuples, lists to std:: containers? May
    // require directly registering additional converters for certain
    // types.
    // WARNING: only works for list<double>; this might be tricky
    // since Python list are allowed to be heterogeneous in general
    // could just support for homogeneous if important, and skip tuples.

    // Can't do with ParameterList, perhaps as std::vector isn't serializable
    //else if (pybind11::isinstance<pybind11::list>(value))
    //  pl.set(key, value.cast<std::vector<double>>());

    // dict (recursive parameter list)
    /* would this work? */
    else if (pybind11::isinstance<pybind11::dict>(value)) {
      pl.set(key, convert_options(value.cast<pybind11::dict>()));
      //pl.sublist(key).set(convert_options(value.cast<pybind11::dict>()));
    }
    // NumPy array to Eigen MatrixXd
    // TODO: Other datatypes such as intmatrix would be easy
    else if (pybind11::isinstance<pybind11::array>(value))
      pl.set(key, value.cast<Eigen::MatrixXd>());
    else
      throw
	std::runtime_error("dict2optslist: key '" + key + "' has unknown type: " +
			   pybind11::str(value.get_type()).cast<std::string>());

    // Can't guarantee all types are printable
    //std::cout << "Key " << key << " ; type stored: "
    //          << pl.get(key)->type().name() << std::endl;
  }
  return pl;
}


// BMA: Explored a number of approaches to avoid bleeding pybind11
// datatypes and code into the core surrogates modules. Attempting to
// maintain an adapter model for now.


/// A free factory that can be used in Python constructors to return a
/// C++ surrogate model. This is one model that lets us avoid
/// extending PolynomialRegression with a separate class or adding
/// constructors that take pybind types.
dakota::surrogates::PolynomialRegression gen_poly(const pybind11::dict& pydict)
{
  return dakota::surrogates::PolynomialRegression(convert_options(pydict));
}


/// Extend PolynomialRegression with a new type for Python
/** Explore idea of extension as a way to specialize
    constructors. Permits mapping datatypes for any member functions
    or constructors that differ, while leaving most
    untouched. Downside is requires new class for each surrogates
    class. */
class PyPolyReg: public dakota::surrogates::PolynomialRegression
{
public:

  PyPolyReg():
    dakota::surrogates::PolynomialRegression()
  { }

  /// ctor that accepts a dictionary
  PyPolyReg(const pybind11::dict& pydict):
    dakota::surrogates::PolynomialRegression(convert_options(pydict))
  { }

  /// ctor that accepts a dictionary
  PyPolyReg(const Eigen::MatrixXd& samples,
	    const Eigen::MatrixXd& response,
	    const pybind11::dict& pydict):
    dakota::surrogates::PolynomialRegression(samples, response,
					     convert_options(pydict))
  { }

  /// Example workaround for default Eigen pass-by-copy semantics
  Eigen::MatrixXd value(const Eigen::MatrixXd& eval_points)
  {
    /*
    Eigen::MatrixXd approx_values;
    PolynomialRegression::value(eval_points, approx_values);
    return approx_values;
    */
    return dakota::surrogates::Surrogate::value(eval_points);
  }
};


/// Induce conversion from dictionary to ParameterList
/** Idea is to enable direct construction like this:
    PolynomialRegression(pydict) via
    PolynomialRegression(ParameterList(PyParameterList(pydict)))
    which works in C++ as follows, but not in the PYBIND_MODULE
*/
class PyParameterList
{
public:
  PyParameterList(const pybind11::dict& pydict):
    pydict_(pydict) {  }
  //  converts PyParameterList to ParameterList
  explicit operator ParameterList()
  { return ParameterList(convert_options(pydict_)); }
private:
  const pybind11::dict& pydict_;
};


/// Extend ParameterList with ParameterListExt
/** This approach also allows construciton from pydict, but didn't add
    value as dictionary didn't get implicitly converted through this
    type in PYBIND11_MODULE constructors. */
class ParameterListExt: public ParameterList
{
public:
  ParameterListExt(const pybind11::dict& pydict):
    ParameterList(convert_options(pydict))
  { }
};


// These both work for implicitly converting in C++, but won't work in
// the PYBIND11_MODULE, even when I tried registering as implicit
// converstions in Pybind11. It might be required (see docs and Github
// issues) to publish all involved datatypes as Pybind11 types.
static ParameterList pl(pybind11::dict());
static dakota::surrogates::PolynomialRegression pr(pybind11::dict());

/// Define a Python module called dakmod wrapping a few surrogates classes
PYBIND11_MODULE(dakmod, m) {

  // attempts to get implicit conversion working from the Python side
  py::class_<ParameterListExt>
    (m, "ParameterListExt")
    .def(py::init<const pybind11::dict&>());

  py::class_<PyParameterList>
    (m, "PyParameterList")
    .def(py::init<const pybind11::dict&>());

  //  py::implicitly_convertible<pybind11::dict, PyParameterList>();
  //  py::implicitly_convertible<pybind11::dict, ParameterListExt>();

  // BMA: Do we want load/save to be free or member? Tradeoff between
  // load being able to load into base class, potentially... Perhaps
  // most natural for save to be a member and load to be invoked from
  // a ctor kwarg. For now modeling as free functions.

  m.def("save_poly",
	static_cast<void (*)(const dakota::surrogates::PolynomialRegression&,
			     const std::string&, const bool)>
	(&dakota::surrogates::Surrogate::save));

  m.def("save_gp",
	static_cast<void (*)(const dakota::surrogates::GaussianProcess&,
			     const std::string&, const bool)>
	(&dakota::surrogates::Surrogate::save));

  m.def("load_poly",
	static_cast<void (*)(const std::string&, const bool,
			     dakota::surrogates::PolynomialRegression&)>
	(&dakota::surrogates::Surrogate::load));

  m.def("load_gp",
	static_cast<void (*)(const std::string&, const bool,
			     dakota::surrogates::GaussianProcess&)>
	(&dakota::surrogates::Surrogate::load));

  /* Doesn't work with Surrogate (base class) *
   * probably need to add some info about the relationship between classes */
  /*
  m.def("load", 
    static_cast<void (*)(const std::string&, const bool, dakota::surrogates::Surrogate&)>
    (&dakota::surrogates::Surrogate::load));
   */

  py::class_<dakota::surrogates::Surrogate>
    (m, "Surrogate")
    // no init since this is a virtual base
    .def("value",
      py::detail::overload_cast_impl<const Eigen::MatrixXd&>()
      (&dakota::surrogates::Surrogate::value))

    .def("gradient",
      py::detail::overload_cast_impl<const Eigen::MatrixXd&, int>()
      (&dakota::surrogates::Surrogate::gradient))

    .def("hessian",
      py::detail::overload_cast_impl<const Eigen::MatrixXd&, int>()
      (&dakota::surrogates::Surrogate::hessian))

    ; // Surrogate


  py::class_<dakota::surrogates::PolynomialRegression,
	     dakota::surrogates::Surrogate // base class
	     >
    (m, "PolynomialRegression")
    //    m.doc() = "Dakota Surrogate class";
    //    .def(py::init<const std::string &>())
    .def(py::init<>())

    // Try just options; doesn't compile due to no matching C++ ctor
    //.def(py::init<const pybind11::dict&>())

    // Bind the factory function as a constructor (works):
    //.def(py::init(&gen_poly))

    // Try direct init from dict
    //.def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
    //     const pybind11::dict&>())

    // Can put the factory in a lambda:
    //.def(py::init([](const pybind11::dict& d)
    //	   { return new dakota::surrogates::PolynomialRegression(d); }))

    // The following WORK!
    //TODO: verify Surrogate classes are default copy-constructible
    .def(py::init([](const pybind11::dict& d)
		  { return dakota::surrogates::
		      PolynomialRegression(convert_options(d)); }))
    .def(py::init([](const Eigen::MatrixXd& samples,
		     const Eigen::MatrixXd& response,
		     const pybind11::dict& d)
		  { return dakota::surrogates::
		      PolynomialRegression(samples, response,
					   convert_options(d)); }))
    //.def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
    //     const ParameterList&>())

    // Initially thought we can't return through arguments in Python
    // like this. Turns out just not for Eigen types which are default
    // copied when passed by reference. Could workaround with a lambda
    // for mapping to return by value.

    ; // PolynomialRegression

  // WORKAROUND (1) is like this (https://pybind11.readthedocs.io/en/stable/faq.html#limitations-involving-reference-arguments), but not sure how to apply to class
  //    [](int i) { int rv = foo(i); return std::make_tuple(rv, i); })
  //   .def("value", [](const Eigen::MatrixXd& samples) { Eigen::MatrixXd resp; dakota::surrogates::PolynomialRegression::value(samples, resp); return resp);

  // WORKAROUND (2) requires changing API to accept reference, but requires care to get sizing and memory right:
  // https://pybind11.readthedocs.io/en/stable/advanced/cast/eigen.html#pass-by-reference

  // WORKAROUND (3) wouldn't use Eigen::Ref, but instead instruct Pybind11 to pass by reference. Long discussion in the pull request which enabled other ways: https://github.com/pybind/pybind11/pull/610 


  // Alternate attempt to wrap an intermediate specialization (this WORKS)

  py::class_<PyPolyReg>
    (m, "PyPolyReg")
    //    m.doc() = "Dakota Surrogate class";
    //    .def(py::init<const std::string &>())
    .def(py::init<>())
    .def(py::init<const pybind11::dict&>())
    .def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
	 const pybind11::dict&>())
    .def("value", &PyPolyReg::value)
    .def_static("load", static_cast<void (*)(const std::string&, const bool, PyPolyReg&)>(&dakota::surrogates::Surrogate::load));
  // Load/save: TODO would probably want as a free static function?

  py::class_<dakota::surrogates::GaussianProcess,
	     dakota::surrogates::Surrogate // base class
	     >
    (m, "GaussianProcess")

    .def(py::init<>())

    .def(py::init([](const pybind11::dict& d)
		  { return dakota::surrogates::
		      GaussianProcess(convert_options(d)); }))

    .def(py::init([](const Eigen::MatrixXd& samples,
		     const Eigen::MatrixXd& response,
		     const pybind11::dict& d)
		  { return dakota::surrogates::
		      GaussianProcess(samples, response,
					   convert_options(d)); }))

    ; // GaussianProcess

}
