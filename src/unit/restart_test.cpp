/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "OutputManager.hpp"
#include "ParamResponsePair.hpp"
#include "RestartVersion.hpp"
#include "SimulationResponse.hpp"

#ifdef _WIN32
#include "util_windows.hpp"
#endif

#define BOOST_TEST_MODULE dakota_restart_test
#include <boost/test/included/unit_test.hpp>
#include <boost/filesystem/operations.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_LOG2E
#define M_LOG2E 1.4426950408889634074
#endif

using namespace Dakota;

// generate PRP with just 1 var and resp
PRPArray generate_minimal_prps(const int num_evals, RestartWriter &rst_writer)
{
  // Mock up a single variable
  SizetArray vc_totals(NUM_VC_TOTALS);
  vc_totals[0] = 1;
  std::pair<short, short> view(MIXED_ALL, EMPTY_VIEW);
  SharedVariablesData svd(view, vc_totals);
  Variables vars(svd);

  // Mock up a single response
  // active set with 1 var / 1 resp
  ActiveSet as(1, 1);
  // TODO: can't default construct srd...
  SharedResponseData srd(as);
  Response resp(srd);

  String iface_id = "RST_IFACE";

  PRPArray prps_out;
  for (int eval_id = 1; eval_id <= num_evals; ++eval_id) {
    vars.continuous_variable(M_LOG2E + (Real) (eval_id-1), 0);
    resp.function_value(M_PI + (Real) (eval_id-1), 0);
    // TODO: code is not robust to writing empty PRP (seg fault)
    // NOTE: This makes a deep copy of vars/resp by default:
    ParamResponsePair prp_out(vars, iface_id, resp, eval_id);
    prps_out.push_back(prp_out);
    rst_writer.append_prp(prp_out);
  }

  return prps_out;
}

/// generate num_evals PRPs, write to the passed RestartWriter, and return in array
PRPArray generate_and_write_prps(const int num_evals, RestartWriter &rst_writer)
{
  // Mock up one of each variable
    SizetArray vc_totals(NUM_VC_TOTALS, 1);
    vc_totals.assign(NUM_VC_TOTALS, 1);
    std::pair<short, short> view(MIXED_ALL, EMPTY_VIEW);
    SharedVariablesData svd(view, vc_totals);
    Variables vars(svd);
    for (size_t i=0; i<vars.acv(); ++i)
      vars.all_continuous_variable(M_LOG2E + (Real) i, i);
    for (size_t i=0; i<vars.adiv(); ++i)
      vars.all_discrete_int_variable(i, i);
    for (size_t i=0; i<vars.adsv(); ++i)
      vars.all_discrete_string_variable(String("sv") + std::to_string(i), i);
    for (size_t i=0; i<vars.adrv(); ++i)
      vars.all_discrete_real_variable(100.0 + (Real) i, i);

    // active set with 12 var / 2 resp, including gradients, Hessians
    int num_resp = 2;
    ActiveSet as(num_resp, NUM_VC_TOTALS);
    as.request_values(7);
    as.derivative_start_value(0);

    // TODO: can't default construct srd...
    // Also the Response doesn't get the right size...
    // SharedResponseData srd(as);
    // srd.response_type(SIMULATION_RESPONSE);
    //  Response resp(srd);

    // TODO: A number of Response constructors are partial, and may
    // leave the resulting object in a bad state.  Have to use this ctor
    // for it to work.
    Response resp(SIMULATION_RESPONSE, as);

    resp.function_value(M_PI, 0);
    resp.function_value(2*M_PI, 1);

    // This calls srand, but should be limited to this test
    // TODO: utilities to use Boost RNG to populate matrices
    Teuchos::ScalarTraits<Real>::seedrandom(12345);

    RealMatrix gradient(NUM_VC_TOTALS, num_resp);
    gradient.random();
    resp.function_gradients(gradient);

    RealSymMatrix hess0(NUM_VC_TOTALS, NUM_VC_TOTALS);
    hess0.random();
    resp.function_hessian(hess0, 0);

    RealSymMatrix hess1(NUM_VC_TOTALS, NUM_VC_TOTALS);
    hess1.random();
    resp.function_hessian(hess1, 1);

    String iface_id = "RST_IFACE";
    int eval_id = 1;

    // TODO: code is not robust to writing empty PRP (seg fault)
    PRPArray prps_out;

    for (int eval_id = 1; eval_id <= num_evals; ++eval_id) {
      // For now this just make a small tweak so variables/resp are unique
      vars.continuous_variable(M_LOG2E + (Real) (eval_id-1), 0);
      resp.function_value(M_PI + (Real) (eval_id-1), 0);
      // NOTE: This makes a deep copy of vars/resp by default:
      ParamResponsePair prp_out(vars, iface_id, resp, eval_id);
      prps_out.push_back(prp_out);
      rst_writer.append_prp(prp_out);
    }

    return prps_out;
}

/// read PRPs from specified restart file to an array and return
PRPArray read_prps(const int num_evals, boost::archive::binary_iarchive& restart_input_archive)
{
  PRPArray prps_in;
  for (int eval_id = 1; eval_id <= num_evals; ++eval_id) {
    ParamResponsePair prp_in;
    restart_input_archive & prp_in;
    prps_in.push_back(prp_in);
  }
  return prps_in;
}

/** In-core test with 1 variable/response */
BOOST_AUTO_TEST_CASE(test_io_restart_1var)
{
  // NOTE: A possible problem with a stringstream is it isn't
  // explicitly a binary stream (std::ios::binary)
  std::stringstream rst_stream;

  int num_evals = 10;
  PRPArray prps_out, prps_in;
  // added scope to force destruction of writer/reader and close the file
  {
    RestartWriter rst_writer(rst_stream);
    prps_out = generate_minimal_prps(num_evals, rst_writer);
  }
  boost::archive::binary_iarchive restart_input_archive(rst_stream);
  RestartVersion rst_ver;
  restart_input_archive & rst_ver;
  for (int eval_id = 1; eval_id <= num_evals; ++eval_id) {
    ParamResponsePair prp_in;
    restart_input_archive & prp_in;
    prps_in.push_back(prp_in);
  }

  BOOST_CHECK(prps_in == prps_out);

  // std::cout << std::setprecision(20) << std::setw(30) << prp_in << '\n'
  // 	    << std::setprecision(20)  << prp_out << '\n';
}

/** File-based test with multiple variables/responses */
BOOST_AUTO_TEST_CASE(test_io_restart_allvar)
{
  std::string rst_filename("restart_allvar.rst");
  boost::filesystem::remove(rst_filename);

  const int num_evals = 10;
  PRPArray prps_out, prps_in;
  // scope to force destruction of writer and close the file
  {
    RestartWriter rst_writer(rst_filename);
    prps_out = generate_and_write_prps(num_evals, rst_writer);
  }

  // scope to destruct ifstream so file can be removed
  {
    std::ifstream restart_input_fs(rst_filename, std::ios::binary);
    boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

    RestartVersion rst_ver;
    restart_input_archive & rst_ver;

    prps_in = read_prps(num_evals, restart_input_archive);
    BOOST_CHECK(prps_in == prps_out);
  }

  // std::cout << std::setprecision(20) << std::setw(30) << prp_in << '\n'
  // 	    << std::setprecision(20)  << prp_out << '\n';

  boost::filesystem::remove(rst_filename);
}


// Verify can detect reading an old (pre-versioning) restart file
BOOST_AUTO_TEST_CASE(test_io_restart_read_oldfile)
{
  std::string rst_filename("old.rst");
  boost::filesystem::remove(rst_filename);

  // an old restart file started with a PRP, instead of versioning, so
  // mock one up
  const int num_evals = 1;
  PRPArray prps_out, prps_in;
  // scope to force destruction of writer and close the file
  {
    RestartWriter rst_writer(rst_filename, false);
    prps_out = generate_and_write_prps(num_evals, rst_writer);
  }

  // scope to destruct ifstream so file can be removed
  {
    std::ifstream ifs(rst_filename, std::ios::binary);
    boost::archive::binary_iarchive inarch(ifs);

    RestartVersion rst_ver;
    inarch & rst_ver;
  
    std::cout << "Reading pre-Dakota " << rst_ver.firstSupportedDakotaVersion()
	      << " restart file with assumed version "
	      << rst_ver.friendly_rst_version() << "; " << rst_ver.dakotaRelease
	      << ", " << rst_ver.dakotaSHA1 << std::endl;

    // make sure we're reading an old file
    BOOST_CHECK(rst_ver.restartVersion < RestartVersion::restartFirstVersionNumber);
    BOOST_CHECK(rst_ver.dakotaRelease == "<unknown>");
    BOOST_CHECK(rst_ver.dakotaSHA1 == "<unknown>");

    ifs.seekg(0);
    boost::archive::binary_iarchive inarch2(ifs);
    prps_in = read_prps(num_evals, inarch2);
    BOOST_CHECK(prps_in == prps_out);
  }

  boost::filesystem::remove(rst_filename);
}


// Verify can detect reading an old (pre-versioning) restart file
// when the contained PRP is minimal
BOOST_AUTO_TEST_CASE(test_io_restart_read_minimal_oldfile)
{
  std::string rst_filename("minimal_old.rst");
  boost::filesystem::remove(rst_filename);

  // an old restart file started with a PRP, instead of versioning, so mock one up
  const int num_evals = 1;
  PRPArray prps_out, prps_in;
  // scope to force destruction of writer and close the file
  {
    RestartWriter rst_writer(rst_filename, false);
    prps_out = generate_minimal_prps(num_evals, rst_writer);
  }

  // scope to destruct ifstream so file can be removed
  {
    std::ifstream ifs(rst_filename, std::ios::binary);
    boost::archive::binary_iarchive inarch(ifs);

    RestartVersion rst_ver;
    inarch & rst_ver;

    std::cout << "Reading pre-Dakota " << rst_ver.firstSupportedDakotaVersion()
	      << " restart file with assumed version "
	      << rst_ver.friendly_rst_version() << "; " << rst_ver.dakotaRelease
	      << ", " << rst_ver.dakotaSHA1  << std::endl;

    // make sure we're reading an old file, which didn't contain versioning info
    BOOST_CHECK(rst_ver.restartVersion < RestartVersion::restartFirstVersionNumber);
    BOOST_CHECK(rst_ver.dakotaRelease == "<unknown>");
    BOOST_CHECK(rst_ver.dakotaSHA1 == "<unknown>");

    ifs.seekg(0);
    boost::archive::binary_iarchive inarch2(ifs);
    prps_in = read_prps(num_evals, inarch2);
    BOOST_CHECK(prps_in == prps_out);
  }

  boost::filesystem::remove(rst_filename);
}

BOOST_AUTO_TEST_CASE(test_io_restart_read_newfile)
{
  std::string rst_filename("new.rst");
  boost::filesystem::remove(rst_filename);

  const int num_evals = 1;
  PRPArray prps_out, prps_in;
  // scope to force destruction of writer and close the file
  {
    RestartVersion rst_ver("6.16.0+", "a1b2c3d4e5f6");
    RestartWriter rst_writer(rst_filename, rst_ver);
    prps_out = generate_and_write_prps(num_evals, rst_writer);
  }

  // scope to destruct ifstream so file can be removed
  {
    std::ifstream ifs(rst_filename, std::ios::binary);
    boost::archive::binary_iarchive inarch(ifs);

    RestartVersion rst_ver;
    inarch & rst_ver;

    std::cout << "Reading Dakota " << rst_ver.dakotaRelease
	      << " restart file with version " << rst_ver.friendly_rst_version()
	      << "; " << rst_ver.dakotaRelease << ", " << rst_ver.dakotaSHA1
	      << std::endl;

    // make sure we're reading a new file
    BOOST_CHECK(rst_ver.restartVersion >= RestartVersion::restartFirstVersionNumber);
    BOOST_CHECK(rst_ver.dakotaRelease == "6.16.0+");
    BOOST_CHECK(rst_ver.dakotaSHA1 == "a1b2c3d4e5f6");

    prps_in = read_prps(num_evals, inarch);
    BOOST_CHECK(prps_in == prps_out);
  }

  boost::filesystem::remove(rst_filename);
}
