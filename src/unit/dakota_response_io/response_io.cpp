/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


/** \file response_io.cpp Test Response reads from results files. */

#include "DakotaActiveSet.hpp"
#include "DakotaResponse.hpp"

#define BOOST_TEST_MODULE dakota_response_io
#include <boost/test/included/unit_test.hpp>

namespace btt = boost::test_tools;

std::vector<double> functions = {54.93, 93855432.34E+02, 3};

std::vector<std::vector<double>> gradients =
  { {4398349.32423, 0.0, 0.0},
    {389490.2,      0.0, 0.0}  };

std::vector<std::vector<double>> hessian_F3 = 
  { { 2.0, -1.0},
    {-1.0,  2.0}  };


// valid results file for ASV = 1, 1, 1, with no metadata
std::string valid_functions_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
)";

// invalid files
std::string extra_functions_file = R"(  45.93  f1
39855432.34E+02 eff2
  2  F3
  858943.0 XTRA
)";

std::string bad_floats_file = R"(  45.93  f1
39855432.34H+02 eff2
  f3  F3
  858943.0 XTRA
)";

std::string bad_labels_file = R"(  45.93  f1
39855432.34E+02 12
  2  3.0
  858943.0 XTRA
)";
std::string extra_gradient_file = R"(  45.93  f1
39855432.34E+02 eff2
[ 3.21 6.54 9.43 ]
  2  F3
)";

std::string bad_functions_file = R"(  45.93  f1
39855432.34E+02 eff2
  2  F3
)";


Dakota::Response get_fn_only_response()
{
  size_t num_derivs = 2, num_fns = 3;
  Dakota::ActiveSet as(num_fns, num_derivs);
  Dakota::SharedResponseData srd(as);
  return Dakota::Response(srd);
}

template<class VectorT>
void check_vector(const std::vector<double>& truth_vec, const VectorT& trial_vec)
{
  for (size_t i=0; i<truth_vec.size(); ++i)
    BOOST_TEST(truth_vec[i] == trial_vec[i], btt::tolerance(1.0e-15));
}


template<class TeuchosMatrixT>
void check_matrix(const std::vector<std::vector<double>>& truth_matrix,
		  const TeuchosMatrixT& trial_matrix)
{
  const auto num_rows = truth_matrix.size();
  const auto num_cols = truth_matrix[0].size();
  for (size_t i=0; i<num_rows; ++i)
    for (size_t j=0; j<num_cols; ++j)
      BOOST_TEST(truth_matrix[i][j] == trial_matrix(i,j), btt::tolerance(1.0e-15));
}


void read_response(const std::string& results_string,
		   const unsigned short format, Dakota::Response& resp)
{
  std::istringstream results_stream(results_string);
  resp.read(results_stream, format);
}


void malformed_file_throws(const std::string& results_string,
			   const unsigned short format, Dakota::Response& resp)
{
  resp.reset();
  BOOST_CHECK_THROW(read_response(results_string, format, resp),
		    Dakota::ResultsFileError);
}


// checks throws with each format
void malformed_file_throws(const std::string& results_string,
			   Dakota::Response& resp)
{
  for (const auto format : {Dakota::FLEXIBLE_RESULTS, Dakota::LABELED_RESULTS})
    malformed_file_throws(results_string, format, resp);
}


// check functions only with default ASV = 1's
BOOST_AUTO_TEST_CASE(test_response_read_functions_only)
{
  Dakota::Response resp = get_fn_only_response();

  read_response(valid_functions_file, Dakota::FLEXIBLE_RESULTS, resp);
  check_vector(functions, resp.function_values());

  resp.reset();
  read_response(bad_functions_file, Dakota::FLEXIBLE_RESULTS, resp);
  const auto& bad_vals = resp.function_values();
  for (size_t i=0; i<functions.size(); ++i)
    BOOST_TEST(functions[i] != bad_vals[i], btt::tolerance(1.0e-15));

  malformed_file_throws(extra_functions_file, Dakota::FLEXIBLE_RESULTS, resp);
  malformed_file_throws(bad_floats_file, Dakota::FLEXIBLE_RESULTS, resp);
  malformed_file_throws(bad_labels_file, Dakota::FLEXIBLE_RESULTS, resp);
  malformed_file_throws(extra_gradient_file,  Dakota::FLEXIBLE_RESULTS, resp);

  // verify throw if labels don't match
  malformed_file_throws(valid_functions_file, Dakota::LABELED_RESULTS, resp);

  // verify works if labels do match
  resp.reset();
  resp.function_labels({"f1", "eff2", "F3"});
  read_response(valid_functions_file, Dakota::LABELED_RESULTS, resp);
  check_vector(functions, resp.function_values());

  malformed_file_throws(extra_functions_file, Dakota::LABELED_RESULTS, resp);
  malformed_file_throws(extra_gradient_file, Dakota::LABELED_RESULTS, resp);
}


// valid results file for ASV = 3, 1, 5, with no metadata
std::string valid_mixed_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)";


std::vector<std::pair<std::string, std::string>> invalid_mixed_files_named = 
  {
    { "extra value",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
  858943.0 XTRA
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    { "intervening junk",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
junk
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    { "errant metadata / trailing junk",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
    54589.45  cost
)"
    },
    { "extra gradient value",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 -1.0 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    {" missing gradient value",
     R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    { "extra gradient row",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
[ -3.4 5.6 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    { "missing gradient row",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
  [[  2.0 -1.0
     -1.0  2.0 ]]
)"
    },
    { "missing Hessian value",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 
     -1.0  2.0 ]]
)"
    },
    { "extra Hessian value",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0 4.5
     -1.0  2.0 ]]
)"
    },
    { "missing Hessian",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
)"
    },
    { "extra Hessian",
      R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
[[  9.8 -3.2
   -3.2  10.9 ]]
)"
    }
  };


Dakota::Response get_derivs_response()
{
  size_t num_derivs = 2, num_fns = 3;
  Dakota::ActiveSet as({3, 1, 5}, {1, 2});
  Dakota::SharedResponseData srd(as);
  return Dakota::Response(srd, as);
}


BOOST_AUTO_TEST_CASE(test_response_read_mixed_derivs)
{
  Dakota::Response resp = get_derivs_response();

  read_response(valid_mixed_file, Dakota::FLEXIBLE_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_matrix(gradients, resp.function_gradients());
  check_matrix(hessian_F3, resp.function_hessian(2));

  //read_response(valid_mixed_file, Dakota::LABELED_RESULTS, resp);

  // verify throw if labels don't match
  malformed_file_throws(valid_mixed_file, Dakota::LABELED_RESULTS, resp);

  // verify works if labels do match
  resp.reset();
  resp.function_labels({"f1", "eff2", "F3"});
  read_response(valid_mixed_file, Dakota::LABELED_RESULTS, resp);
  check_vector(functions, resp.function_values());

  // verify all invalid files throw, even though labels match
  for (const auto& invalid_file : invalid_mixed_files_named) {
    // to see checkpoints, run test with --log_level=all
    BOOST_TEST_CHECKPOINT(invalid_file.first);
    malformed_file_throws(invalid_file.second, resp);
  }
}


// valid results file for ASV = 1, 1, 1, with 2 metadata
std::string valid_functions_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
3.1415 ApplePi
  4000 cost
)";

std::string one_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
  4000 cost
)";

std::string missing_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
)";

std::vector<double> metadata = {3.1415, 4000};


// check functions + metadata with default ASV = 1's
BOOST_AUTO_TEST_CASE(test_response_read_functions_metadata)
{
  Dakota::Response resp = get_fn_only_response();
  resp.reshape_metadata(metadata.size());

  read_response(valid_functions_metadata_file, Dakota::FLEXIBLE_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_vector(metadata, resp.metadata());

  // throws on metadata label mismatch
  resp.function_labels({"f1", "eff2", "F3"});
  malformed_file_throws(valid_functions_metadata_file, Dakota::LABELED_RESULTS, resp);

  // works on matching MD labels
  resp.reset();
  resp.shared_data().metadata_labels({"ApplePi", "cost"});
  read_response(valid_functions_metadata_file, Dakota::LABELED_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_vector(metadata, resp.metadata());

  // throws if metadata missing
  malformed_file_throws(one_metadata_file, resp);
  malformed_file_throws(missing_metadata_file, resp);
}


// for object expecting two metadata
// TODO: improve error reporting?
/*
fns,meta,grad,hess

fns,grad,hess,meta
*/



// valid results file for ASV = 3, 1, 5, with 2 metadata
std::string valid_derivs_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
3.1415 ApplePi
  4000 cost
)";

std::string metadata_grad_hess_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
3.1415 ApplePi
  4000 cost
[ 4398349.32423 389490.2 ]
  [[  2.0 -1.0
     -1.0  2.0 ]]
)";

std::string grad_metadata_hess_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
3.1415 ApplePi
  4000 cost
  [[  2.0 -1.0
     -1.0  2.0 ]]
)";

std::string missing_gradient_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
  [[  2.0 -1.0
     -1.0  2.0 ]]
3.1415 ApplePi
  4000 cost
)";

std::string missing_hessian_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
3.1415 ApplePi
  4000 cost
)";


BOOST_AUTO_TEST_CASE(test_response_read_derivs_metadata)
{
  Dakota::Response resp = get_derivs_response();
  resp.reshape_metadata(metadata.size());

  read_response(valid_derivs_metadata_file, Dakota::FLEXIBLE_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_matrix(gradients, resp.function_gradients());
  check_matrix(hessian_F3, resp.function_hessian(2));
  check_vector(metadata, resp.metadata());

  // throws on metadata label mismatch
  resp.function_labels({"f1", "eff2", "F3"});
  malformed_file_throws(valid_derivs_metadata_file, Dakota::LABELED_RESULTS, resp);

  // works on matching MD labels
  resp.reset();
  resp.shared_data().metadata_labels({"ApplePi", "cost"});
  read_response(valid_derivs_metadata_file, Dakota::LABELED_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_matrix(gradients, resp.function_gradients());
  check_matrix(hessian_F3, resp.function_hessian(2));
  check_vector(metadata, resp.metadata());

  // throws for malformed results
  malformed_file_throws(metadata_grad_hess_file, resp);
  malformed_file_throws(grad_metadata_hess_file, resp);
  malformed_file_throws(missing_gradient_metadata_file, resp);
  malformed_file_throws(missing_hessian_metadata_file, resp);
}


// valid results file for ASV = 3, 1, 1, with 2 metadata
std::string valid_gradient_metadata_file = R"(  54.93  f1
93855432.34E+02 eff2
  3  F3
[ 4398349.32423 389490.2 ]
3.1415 ApplePi
  4000 cost
)";


// test for gradient immediately followed by metadata (validate that
// reader puts back the data from forward seek)
BOOST_AUTO_TEST_CASE(test_response_read_grads_metadata)
{
  Dakota::Response resp = get_derivs_response();
  resp.active_set_request_vector({3, 1, 1});
  resp.reshape_metadata(metadata.size());

  read_response(valid_gradient_metadata_file, Dakota::FLEXIBLE_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_matrix(gradients, resp.function_gradients());
  check_vector(metadata, resp.metadata());

  // throws on metadata label mismatch
  resp.function_labels({"f1", "eff2", "F3"});
  malformed_file_throws(valid_gradient_metadata_file, Dakota::LABELED_RESULTS, resp);

  // works on matching MD labels
  resp.reset();
  resp.shared_data().metadata_labels({"ApplePi", "cost"});
  read_response(valid_gradient_metadata_file, Dakota::LABELED_RESULTS, resp);
  check_vector(functions, resp.function_values());
  check_matrix(gradients, resp.function_gradients());
  check_vector(metadata, resp.metadata());
}
