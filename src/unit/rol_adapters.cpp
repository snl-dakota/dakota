/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_windows.hpp"

#include "ROL_OptimizationSolver.hpp"

#include "ROL_RandomVector.hpp"
#include "ROL_StdVector.hpp"
#include "ROL_StdObjective.hpp"
// #include "ROL_Teuchos_Objective.hpp"
// #include "ROL_Teuchos_Constraint.hpp"
#include "ROL_Bounds.hpp"

#define BOOST_TEST_MODULE dakota_rol_adapters
#include <boost/test/included/unit_test.hpp>

#include "opt_tpl_test_fixture.hpp"

using namespace Dakota;


//----------------------------------------------------------------

class StdFactory
{
  public:

    typedef std::vector<Real> VT;
    typedef ROL::StdVector<Real> RVT;
    typedef ROL::StdObjective<Real> RObjT;
};

  //----------------------------

// class TeuchosSerialDenseFactory
// {
//   public:

//     typedef RealVector VT;
//     typedef ROL::TeuchosVector<int, Real> RVT;
//     typedef ROL::TeuchosObjective<int, Real> RObjT;
// };

//----------------------------------------------------------------

/* Objective Function based on Wrapped Dakota Model */

template<class FactoryT> 
class DakotaModelObjective : public FactoryT::RObjT
{
  private:

    // data
    Model * dakModel;

  public:

    DakotaModelObjective(Model * model) :
      dakModel(model)
    { 
    }

    Real value(const typename FactoryT::VT &x, Real &tol)
    {
      Real result = 0;
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

template<class FactoryT> 
void rol_quad_solv()
{
  std::shared_ptr<LibraryEnvironment> p_env(Opt_TPL_Test_Fixture::create_default_env(Dakota::OPTPP_PDS));
  LibraryEnvironment & env = *p_env;

  if (env.parallel_library().mpirun_flag())
    BOOST_CHECK( false ); // This test only works for serial builds

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

  RCP<typename FactoryT::VT> x_rcp  = rcp( new typename FactoryT::VT(2) );
  RCP<ROL::Vector<Real> >    x      = rcp( new typename FactoryT::RVT(x_rcp) );
  RCP<ROL::Objective<Real> > obj    = rcp( new DakotaModelObjective<FactoryT>(&model) );

  try {
 
    ROL::OptimizationProblem<Real> problem( obj, x );

    // Not needed but informative ...
    problem.check(*outStream);

    ROL::OptimizationSolver<Real> solver( problem, parlist );

    solver.solve(*outStream); 

    *outStream << "x_opt = [";
    for(int i=0;i<1;++i) {
      *outStream << (*x_rcp)[i] << ", " ;
    } 
    *outStream << (*x_rcp)[1] << "]" << std::endl;
  }
  catch (std::logic_error err) {
    *outStream << err.what() << "\n";
    BOOST_CHECK( false );
  }; // end try

  // Assess correctness
  BOOST_CHECK_CLOSE( (*x_rcp)[0], -1.50, 1.e-4 );
  BOOST_CHECK_CLOSE( (*x_rcp)[1],  0.75, 1.e-3 );
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(a_rol_std_quad)
{
  rol_quad_solv<StdFactory>();
}

//----------------------------------------------------------------

 // BOOST_AUTO_TEST_CASE(a_rol_teuchos_quad)
 // {
 //   rol_quad_solv<TeuchosSerialDenseFactory>();
 // }

