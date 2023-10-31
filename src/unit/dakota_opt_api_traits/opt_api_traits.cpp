/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_data_util.hpp"
#include "DakotaTraitsBase.hpp"
#include "DakotaIterator.hpp"
#include "DataMethod.hpp"
#include "DataModel.hpp"

#ifdef DAKOTA_HOPS
#include "APPSOptimizer.hpp"
#endif
#ifdef HAVE_ACRO
#include "COLINOptimizer.hpp"
#include "PEBBLMinimizer.hpp"
#endif
#ifdef HAVE_JEGA
#include "JEGAOptimizer.hpp"
#endif
#ifdef HAVE_NOMAD
#include "NomadOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif
#include "SurrBasedGlobalMinimizer.hpp"

#include <limits>
#include <string>
#include <map>

#define BOOST_TEST_MODULE dakota_opt_api_traits
#include <boost/test/included/unit_test.hpp>


using namespace Dakota;


//----------------------------------------------------------------

namespace {

  bool check_variables( unsigned short methodName,
                        int num_cont_vars,
                        int num_disc_int_vars,
                        int num_disc_real_vars,
                        int num_disc_string_vars,
                        bool & continuous_only )
  {
    if (methodName == MOGA        || methodName == SOGA ||
        methodName == COLINY_EA   || methodName == SURROGATE_BASED_GLOBAL ||
        methodName == COLINY_BETA || methodName == MESH_ADAPTIVE_SEARCH || 
        methodName == ASYNCH_PATTERN_SEARCH || methodName == BRANCH_AND_BOUND)
    {
      continuous_only = false;
      if (!num_cont_vars && !num_disc_int_vars && !num_disc_real_vars && !num_disc_string_vars)
        return false;
    }
    else 
    { // methods supporting only continuous design variables
      continuous_only = true;
      if (!num_cont_vars)
        return false;
    }
    return true;
  }

  //----------------------------------

  bool check_variables( std::shared_ptr<TraitsBase> traits,
                        int num_cont_vars,
                        int num_disc_int_vars,
                        int num_disc_real_vars,
                        int num_disc_string_vars,
                        bool & continuous_only )
  {
    if( traits->supports_continuous_variables()              && 
        traits->supports_discrete_variables())
    {
      continuous_only = false;
      if (!num_cont_vars && !num_disc_int_vars && !num_disc_real_vars && !num_disc_string_vars)
        return false;
    }
    else { // methods supporting only continuous design variables
      continuous_only = true;
      if (!num_cont_vars)
        return false;
    }
    return true;
  }

  //----------------------------------

  void check_variable_consistency( unsigned short methodName,
                                   std::shared_ptr<TraitsBase> traits )
  {
    bool continuous_only_enum   = false;
    bool continuous_only_traits = false;
    bool is_consistent_enums    = false;
    bool is_consistent_traits   = false;

    std::shared_ptr<Iterator> method_iter;

    // Test Traits
    method_iter.reset( new Iterator(traits) );
    BOOST_CHECK( method_iter->traits()->is_derived() );

    for( int i=0; i<2; ++i )
      for( int j=0; j<2; ++j )
        for( int k=0; k<2; ++k )
          for( int l=0; l<2; ++l )
          {
            is_consistent_enums  = check_variables(methodName,            i, j, k, l, continuous_only_enum);
            is_consistent_traits = check_variables(method_iter->traits(), i, j, k, l, continuous_only_traits);
            BOOST_CHECK( is_consistent_enums  == is_consistent_traits );
            BOOST_CHECK( continuous_only_enum == continuous_only_traits );
          }
  }
}


BOOST_AUTO_TEST_CASE(test_opt_api_traits_var_consistency)
{
  // Test various TPL Traits as they become available
#ifdef DAKOTA_HOPS
  check_variable_consistency( ASYNCH_PATTERN_SEARCH , std::shared_ptr<TraitsBase>(new AppsTraits())           );
#endif
#ifdef HAVE_JEGA
  check_variable_consistency( MOGA                  , std::shared_ptr<TraitsBase>(new JEGATraits())           );
  check_variable_consistency( SOGA                  , std::shared_ptr<TraitsBase>(new JEGATraits())           );
#endif
  check_variable_consistency( SURROGATE_BASED_GLOBAL, std::shared_ptr<TraitsBase>(new SurrBasedGlobalTraits()));
#ifdef HAVE_NOMAD
  check_variable_consistency( MESH_ADAPTIVE_SEARCH  , std::shared_ptr<TraitsBase>(new NomadTraits())          );
#endif
#ifdef HAVE_ACRO
  check_variable_consistency( BRANCH_AND_BOUND      , std::shared_ptr<TraitsBase>(new PebbldTraits())         );
#endif
}

//----------------------------------------------------------------
