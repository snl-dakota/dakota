#include "OutputManager.hpp"
#include "ParamResponsePair.hpp"
#include "SimulationResponse.hpp"

#include <Teuchos_UnitTestHarness.hpp>
#include <boost/filesystem/operations.hpp>

using namespace Dakota;

TEUCHOS_UNIT_TEST(io, restart_1var)
{
  String rst_filename("restart_1var.rst");
  if (boost::filesystem::exists(rst_filename))
    boost::filesystem::remove(rst_filename);

  // Mock up a single variable
  SizetArray vc_totals(NUM_VC_TOTALS);
  vc_totals[0] = 1;
  std::pair<short, short> view(MIXED_ALL, EMPTY_VIEW);
  SharedVariablesData svd(view, vc_totals);
  Variables vars(svd);
  vars.continuous_variable(M_LOG2E, 0);

  // Mock up a single response
  // active set with 1 var / 1 resp
  ActiveSet as(1, 1);
  // TODO: can't default construct srd...
  SharedResponseData srd(as);
  Response resp(srd);
  resp.function_value(M_PI, 0);

  String iface_id = "RST_IFACE";
  int eval_id = 1;

  // TODO: code is not robust to writing empty PRP (seg fault)
  ParamResponsePair prp_out(vars, iface_id, resp, eval_id), prp_in;

  // added scope to force destruction of writer/reader and close the file
  {
    RestartWriter rst_writer(rst_filename);
    rst_writer.append_prp(prp_out);
  }
  {
    std::ifstream restart_input_fs(rst_filename.c_str(), std::ios::binary);
    boost::archive::binary_iarchive restart_input_archive(restart_input_fs);
    restart_input_archive & prp_in;
  }

  TEST_EQUALITY(prp_in, prp_out);

  // std::cout << std::setprecision(20) << std::setw(30) << prp_in << '\n'
  // 	    << std::setprecision(20)  << prp_out << '\n';

}

TEUCHOS_UNIT_TEST(io, restart_allvar)
{
  String rst_filename("restart_allvar.rst");
  if (boost::filesystem::exists(rst_filename))
    boost::filesystem::remove(rst_filename);

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
    vars.all_discrete_string_variable(String("sv") +
				      boost::lexical_cast<String>(i), i);
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

  // The following aren't deterministic, but that might be okay for
  // this kind of test...

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
  ParamResponsePair prp_out(vars, iface_id, resp, eval_id), prp_in;

  // added scope to force destruction of writer/reader and close the file
  {
    RestartWriter rst_writer(rst_filename);
    rst_writer.append_prp(prp_out);
  }
  {
    std::ifstream restart_input_fs(rst_filename.c_str(), std::ios::binary);
    boost::archive::binary_iarchive restart_input_archive(restart_input_fs);
    restart_input_archive & prp_in;
  }

  TEST_EQUALITY(prp_in, prp_out);

  // std::cout << std::setprecision(20) << std::setw(30) << prp_in << '\n'
  // 	    << std::setprecision(20)  << prp_out << '\n';

}
