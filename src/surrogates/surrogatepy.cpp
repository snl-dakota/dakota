//#include "Surrogate.hpp"
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
  for (auto di : pydict) {
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

    // Can't do with ParameterList...    
//    else if (pybind11::isinstance<pybind11::list>(value))
//      pl.set(key, value.cast<std::vector<double>>());
    // dict (recursive parameter list)
    else if (pybind11::isinstance<pybind11::dict>(value))
      pl.set(key, convert_options(value.cast<pybind11::dict>()));
    // NumPy array to Eigen MatrixXd
    // TODO: Other datatypes such as intmatrix would be easy
    else if (pybind11::isinstance<pybind11::array>(value))
      pl.set(key, value.cast<Eigen::MatrixXd>());
    else
      throw
	std::runtime_error("dict2optslist: key '" + key + "' has unknown type: " +
			   pybind11::str(value.get_type()).cast<std::string>());

//    std::cout << "Key " << key << " ; type stored: "
//	      << pl.get(key)->type().name() << std::endl;
  }
  return pl;
}


// BMA: Explored a number of approaches to avoid bleeding pybind11
// code into the core surrogates modules. Attempting to maintain an
// adapter model

// Could have a free factory if we don't want to extend PR with a separate class
dakota::surrogates::PolynomialRegression gen_poly(const pybind11::dict& pydict)
{
  return dakota::surrogates::PolynomialRegression(convert_options(pydict));
}


class PyPolyReg: public dakota::surrogates::PolynomialRegression
{
  PyPolyReg(const pybind11::dict& pydict):
    dakota::surrogates::PolynomialRegression(convert_options(pydict))
  { }
};


// Instead try simple conversion from dictionary to ParameterList
// PolynomialRegression(ParameterList(PyParameterList(dict)))
class PyParameterList
{
public:
  const pybind11::dict& pydict_;
  PyParameterList(const pybind11::dict& pydict):
    pydict_(pydict) {  }
  //  converts PyParameterList to ParameterList
  explicit operator ParameterList()
  { return ParameterList(convert_options(pydict_)); }
};

// Attempt at specializing parameteter list with ParameterListExt
// which can be constructed from dict and passes as parameter
// list. Didn't add value as dictionary didn't get implicitly
// converted through this type.
class ParameterListExt: public ParameterList
{
public:
  ParameterListExt(const pybind11::dict& pydict):
    ParameterList(convert_options(pydict))
  { }
};

// These both work, but won't work in the PYBIND11_MODULE
static ParameterList pl(pybind11::dict());
static dakota::surrogates::PolynomialRegression pr(pybind11::dict());

PYBIND11_MODULE(dakmod, m) {

  py::class_<ParameterListExt>
    (m, "ParameterListExt")
    .def(py::init<const pybind11::dict&>());

  py::class_<PyParameterList>
    (m, "PyParameterList")
    .def(py::init<const pybind11::dict&>());

  //  py::implicitly_convertible<pybind11::dict, PyParameterList>();
  //  py::implicitly_convertible<pybind11::dict, ParameterListExt>();

  py::class_<dakota::surrogates::PolynomialRegression>
    (m, "PolynomialRegression")
    //    m.doc() = "Dakota Surrogate class";
    //    .def(py::init<const std::string &>())
    .def(py::init<>())
    // Try just options
    //.def(py::init<const pybind11::dict&>())
    // Bind the factory function as a constructor:
    //.def(py::init(&gen_poly))
    // Try direct init from dict
//    .def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
//	 const pybind11::dict&>())

//    .def(py::init([](const pybind11::dict& d)
//		  { return new dakota::surrogates::PolynomialRegression(d); }))

    // WORKS!
    //TODO: verify this is copy-constructible
    .def(py::init([](const pybind11::dict& d)
		  { return dakota::surrogates::
		      PolynomialRegression(convert_options(d)); }))
    .def(py::init([](const Eigen::MatrixXd& samples,
		     const Eigen::MatrixXd& response, 
		     const pybind11::dict& d)
		  { return dakota::surrogates::
		      PolynomialRegression(samples, response,
					   convert_options(d)); }))
//    .def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
//	 const ParameterList&>())
    // can't return through arguments in Python like this
    .def("value", &dakota::surrogates::PolynomialRegression::value);
    // workaround is like this (https://pybind11.readthedocs.io/en/stable/faq.html#limitations-involving-reference-arguments), but not sure how to apply to class
    //    [](int i) { int rv = foo(i); return std::make_tuple(rv, i); })
    //   .def("value", [](const Eigen::MatrixXd& samples) { Eigen::MatrixXd resp; dakota::surrogates::PolynomialRegression::value(samples, resp); return resp);

}
