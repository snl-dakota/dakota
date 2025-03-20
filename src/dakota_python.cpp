/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file
    Python module wrapping top-level Dakota
 */


#include "ExecutableEnvironment.hpp"
#include "LibraryEnvironment.hpp"
#include "Pybind11Interface.hpp"

//#include "Eigen/Dense"

//#include <pybind11/eigen.h> 

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifdef DAKOTA_PYTHON_NUMPY
#include <pybind11/numpy.h>
#endif

namespace py = pybind11;

namespace Dakota {

namespace python {

  // demonstrate a little wrapper to get Dakota version info without
  // making an Environment
  void print_version() {
    OutputManager out_mgr;
    out_mgr.output_version();
  }

  // Eigen::VectorXd final_response(const Environment& dakenv) {
  // const Response& resp = dakenv.response_results();
  // size_t num_fns = resp.num_functions();
  // return Eigen::Map<Eigen::VectorXd>(resp.function_values().values(), num_fns);
  // }

  std::vector<double> get_variables_values(const Dakota::LibraryEnvironment & env)
  {
    const Variables& vars = env.variables_results();
    const RealVector& var_vals = vars.continuous_variables();
    std::vector<double> values;
    copy_data(var_vals, values);
    return values;
  }


#ifdef DAKOTA_PYTHON_NUMPY
  // This version requires numpy
  py::array_t<double> get_variables_values_numpy(const Dakota::LibraryEnvironment & env)
  {
    const Variables& vars = env.variables_results();
    const RealVector& var_vals = vars.continuous_variables();

    auto result = py::array(py::buffer_info(
          nullptr,                              /* Pointer to data (nullptr -> ask NumPy to allocate!) */
          sizeof(double),                       /* Size of one item */
          py::format_descriptor<double>::value, /* Buffer format */
          1,                                    /* How many dimensions? */
          { vars.cv() },                        /* Number of elements for each dimension */
          { sizeof(double) }                    /* Strides for each dimension */
          ));

    auto buffer = result.request();

    double *ptr = (double *) buffer.ptr;

    // Could consider using either copy_data or an adapter - RWH
    for (size_t i=0; i<buffer.shape[0]; ++i)
      ptr[i] = var_vals[i];

    return result;
  }
#endif
    

  Real get_response_fn_val(const Dakota::LibraryEnvironment & env) {
    // retrieve the final response values
    const Response& resp  = env.response_results();
    return resp.function_value(0);
  }
  
  Dakota::LibraryEnvironment* create_libEnv( const std::string& input_string) {

      assert(!input_string.empty());

      Dakota::ProgramOptions opts;
      opts.echo_input(false);
      // avoid calling std::exit and taking down the Python instance too
      opts.exit_mode("throw");
      opts.input_string(input_string);

      auto p_libEnv = new Dakota::LibraryEnvironment(opts);

      return p_libEnv;
    }
}
}


/// Define a Python module that wraps a few top-level dakota functions
/// Module name is really generic due to overly simple Python
///  packaging scheme we're using
PYBIND11_MODULE(environment, m) {

  // demo a module-level function
  m.def("version",
	&Dakota::python::print_version,
	"Print Dakota version to console"
	);

  // demo a Dakota command-line wrapper
  // probably don't need this at all, but demoing...
  py::class_<Dakota::ExecutableEnvironment>(m, "CommandLine")
    .def(py::init
	 ([](const std::string& input_filename)
	  {
	    int argc = 2;
            std::string infile_copy(input_filename);
	    char* argv[] = { "dakota",
			     const_cast<char*>(infile_copy.data()),
			     NULL
	    };
            return new Dakota::ExecutableEnvironment(argc, argv);
	  })
	 )

    .def("execute", &Dakota::ExecutableEnvironment::execute)
    ;

  // demo a Variables wrapper
  py::class_<Dakota::Variables>(m, "Variables")
    .def(py::init
	 ([]()
	  {
	    return new Dakota::Variables(); 
	  }))

    .def("num_active_cv", &Dakota::Variables::cv
         , "Return number of active continuous vars"
         )
    ;

  // demo a Response wrapper
  py::class_<Dakota::Response>(m, "Response")
    .def(py::init
	 ([]()
	  {
	    return new Dakota::Response(); 
	  }))

    .def("function_value", static_cast<const Dakota::Real & (Dakota::Response::*)(size_t) const>(&Dakota::Response::function_value)
         , "Return function value by index"
         , py::arg("i"))
    ;

  // demo a library environment that models opt_tpl_test semantics
  py::class_<Dakota::LibraryEnvironment>(m, "study")
    .def(py::init
	 ([](py::object callback,
	     const std::string& input_string)
	  {
	    assert(!input_string.empty());
            auto p_libEnv = Dakota::python::create_libEnv(input_string);

            // Associate the single python callback with all Pybind11Interface interfaces
            Dakota::InterfaceList & interfaces = p_libEnv->problem_description_db().interface_list();
            for( auto & interface : interfaces )
            {
              auto py11_int = std::dynamic_pointer_cast<Dakota::Pybind11Interface>(
                                (*p_libEnv->problem_description_db().interface_list().begin()));
              if( py11_int )
                py11_int->register_pybind11_callback_fn(callback);
            }

	    return p_libEnv;
	  })
	 , py::arg("callback"), py::arg("input_string"))

    .def(py::init
	 ([]( py::dict callbacks,
	     const std::string& input_string)
	  {
	    assert(!input_string.empty());
            auto p_libEnv = Dakota::python::create_libEnv(input_string);

            // Associate callbacks with interface specs
            auto callbacks_map = callbacks.cast< std::map<std::string,py::function> >();
            Dakota::InterfaceList & interfaces = p_libEnv->problem_description_db().interface_list();
            for( auto & interface : interfaces )
            {
              auto py11_int = std::dynamic_pointer_cast<Dakota::Pybind11Interface>(
                                (*p_libEnv->problem_description_db().interface_list().begin()));
              if( py11_int )
              {
                for( auto const & idrv : interface->analysis_drivers() )
                {
                  if( callbacks_map.count(idrv) > 0 )
                    py11_int->register_pybind11_callback_fn(callbacks_map[idrv]);
                  else {
                    Cout << "Warning: Could not find a pybind11 callback \"" << idrv << "\" needed "
                            "by Dakota interface \"" << interface->interface_id() << "\".\n"
                            "... will try to use module:function interface specification.\n";
                  }
                }
              }
            }

	    return p_libEnv;
	  })
	 , py::arg("callbacks"), py::arg("input_string"))

    .def("execute", &Dakota::LibraryEnvironment::execute)
    .def("variables_results", &Dakota::LibraryEnvironment::variables_results)
    .def("response_results", &Dakota::LibraryEnvironment::response_results)
    ;

#ifdef DAKOTA_PYTHON_NUMPY
  m.def("get_variable_values_np",
        &Dakota::python::get_variables_values_numpy,
	"Get active continuous Variable values"
	);
#endif

  m.def("get_variable_values",
        &Dakota::python::get_variables_values,
	"Get active continuous Variable values"
	);

  m.def("get_response_fn_val",
        &Dakota::python::get_response_fn_val,
	"Get final Response function value"
	);

}
