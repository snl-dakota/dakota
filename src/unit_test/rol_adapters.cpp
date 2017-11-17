
#include "ROL_OptimizationSolver.hpp"

#include "ROL_RandomVector.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_Bounds.hpp"

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_GlobalMPISession.hpp"

#include "opt_tpl_test_fixture.hpp"

using namespace Dakota;

/* Objective Function based on Wrapped Dakota Model */

template<class ScalarT> 
class DakotaModelObjective : public ROL::StdObjective<ScalarT>
{
  private:

    // data
    Model * dakModel;

  public:

    DakotaModelObjective(Model * model) :
      dakModel(model)
    { 
    }

    ScalarT value(const std::vector<ScalarT> &x, ScalarT &tol)
    {
      ScalarT result = 0;
      dakModel->continuous_variable(x[0], 0);
      dakModel->continuous_variable(x[1], 1);
      dakModel->evaluate();
      const Response& test_resp  = dakModel->current_response();
      result = test_resp.function_value(0);
      return result;
    }

    //void gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol )
    //{
    //}  

    //void hessVec( std::vector<Real> &hv, const std::vector<Real> &v, const std::vector<Real> &x, Real &tol )
    //{
    //}

}; // class DakotaModelObjective


//----------------------------------------------------------------

TEUCHOS_UNIT_TEST(rol, quad)
{
  LibraryEnvironment * p_env = Opt_TPL_Test_Fixture::create_default_env(Dakota::OPTPP_PDS);
  LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    TEST_ASSERT( false ); // This test only works for serial builds

  Dakota::Model & model = *(env.problem_description_db().model_list().begin());

  // ROL stuff from here down ...

  using Teuchos::RCP;
  using Teuchos::rcp;

  int iprint = 1;
  RCP<std::ostream> outStream;
  Teuchos::oblackholestream bhs; // outputs nothing
  if (iprint > 0)
    outStream = rcp(&std::cout, false);
  else
    outStream = rcp(&bhs, false);

  Teuchos::ParameterList parlist;
  parlist.sublist("Step").set("Type","Line Search");


  RCP<std::vector<Real> > x_rcp  = rcp( new std::vector<Real>(5,1.0) );
  RCP<ROL::Vector<Real> > x      = rcp( new ROL::StdVector<Real>(x_rcp) );
  RCP<ROL::Objective<Real> > obj = rcp( new DakotaModelObjective<Real>(&model) );

  try {
 
    ROL::OptimizationProblem<Real> problem( obj, x );

    // Not needed but informative ...
    //problem.check(*outStream);

    ROL::OptimizationSolver<Real> solver( problem, parlist );

    solver.solve(*outStream); 

    *outStream << "x_opt = [";
    for(int i=0;i<2;++i) {
      *outStream << (*x_rcp)[i] << ", " ;
    } 
    *outStream << (*x_rcp)[2] << "]" << std::endl;
  }
  catch (std::logic_error err) {
    *outStream << err.what() << "\n";
    TEST_ASSERT( false );
  }; // end try

  TEST_FLOATING_EQUALITY( (*x_rcp)[0], -1.50, 1.e-10 );
  TEST_FLOATING_EQUALITY( (*x_rcp)[1],  0.75, 1.e-10 );

  // Make sure to cleanup the object we own
  delete p_env;
}

