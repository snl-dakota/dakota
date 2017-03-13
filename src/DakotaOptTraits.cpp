/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OptTraits
//- Description: Implementation code for the OptTraits class
//- Owner:       Moe Khalil
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "DakotaOptTraits.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"

static const char rcsId[]="@(#) $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $";


namespace Dakota {


/** Default constructor */
inline OptTraits::OptTraits():
requiresBounds(false),
supportsLinearEquality(false),
supportsNonlinearEquality(false),
supportsScaling(false),
supportsLeastSquares(false),
supportsMultiobjectives(false),
linearInequalityType(1),
nonlinearInequalityType(1),
supportsContinuousVariables(false),
supportsIntegerVariables(false),
supportsRelaxableDiscreteVariables(false),
supportsCategoricalVariables(false),
providesBestObjective(false),
providesBestParameters(false),
providesBestConstraint(false),
providesFinalGradient(false),
providesFinalHessian(false)
{
  typedef boost::bimap< std::string, int >::value_type position;

  // Supported constraint forms
  validConstraints.insert( position("not_supported"    ,1) );
  validConstraints.insert( position("one_sided_lowel"  ,2) );
  validConstraints.insert( position("one_sided_upper"  ,3) );
  validConstraints.insert( position("two_sided"        ,4) );

  // Supported response forms
  validResponses.insert( position("function"              ,1) );
  validResponses.insert( position("nonlinear_equality"    ,2) );
  validResponses.insert( position("nonlinear_inequality"  ,3) );

  // Make room for ordered list of responses
  for (int i = 1; i < 4; i++) responseOrder.push_back(0);
}


/** Sets the ordered list of accepted responses */
void OptTraits::set_response_order(const std::vector<std::string>& response_order)
{
  bool err_flag = false;

  std::vector<std::string>::const_iterator i;

  //iterate over list of provided responses
  int indi = 0; //index into responseOrder vector
  for(i=response_order.begin(); i!=response_order.end(); ++i) 
  {
    // check if provided type of response is a valid type
    boost::bimap< std::string, int >::left_const_iterator it = validResponses.left.find(*i);
  
    if(it == validResponses.left.end()) {
      Cerr << "\nError: provided type of response is not valid." << std::endl;
      err_flag = true;
      break;
    }else {
      responseOrder[indi++] = it->second;
    }
  }

  if (err_flag)
    abort_handler(-1);
}


/** Return the ordered list of accepted responses */
void OptTraits::get_response_order(std::vector<std::string>& response_order)
{
  bool err_flag = false;

  std::vector<std::string> responses;

  std::vector<int>::const_iterator i;

  //iterate over stored responses
  for(i=responseOrder.begin(); i!=responseOrder.end(); ++i) 
  {
    // check if provided type of response is a valid type
      boost::bimap< std::string, int >::right_const_iterator it = validResponses.right.find(*i);
  
    if(it == validResponses.right.end()) {
      Cerr << "\nError: provided type of response is not valid." << std::endl;
      err_flag = true;
      break;
    }else {
      responses.push_back(it->second);
    }
  }

  if (err_flag)
    abort_handler(-1);

  response_order = responses;
}


/** Sets the type of nonlinear inequality supported */
void OptTraits::set_nonlinear_inequality_type(const std::string& nonlinear_inequality_type)
{
  bool err_flag = false;

  // check if provided type of inequality is a valid type
  boost::bimap< std::string, int >::left_const_iterator it = validConstraints.left.find(nonlinear_inequality_type);
  if(it == validConstraints.left.end()) {
    Cerr << "\nError: provided type of nonlinear inequality is not valid." << std::endl;
    err_flag = true;
  }else {
    nonlinearInequalityType = it->second;
  }

  if (err_flag)
    abort_handler(-1);
}


/** Return the type of nonlinear inequality supported */
void OptTraits::get_nonlinear_inequality_type(std::string& nonlinear_inequality_type)
{
  bool err_flag = false;
    
  // check if provided type of inequality is a valid type
  boost::bimap< std::string, int >::right_const_iterator it = validConstraints.right.find(nonlinearInequalityType);
  if(it == validConstraints.right.end()) {
    Cerr << "\nError: stored type of nonlinear inequality is not valid." << std::endl;
    err_flag = true;
  }else {
    nonlinear_inequality_type = it->second;
  }

  if (err_flag)
    abort_handler(-1);
}


/** Sets the type of linear inequality supported */
void OptTraits::set_linear_inequality_type(const std::string& linear_inequality_type)
{
  bool err_flag = false;

  // check if provided type of inequality is a valid type
  boost::bimap< std::string, int >::left_const_iterator it = validConstraints.left.find(linear_inequality_type);
  if(it == validConstraints.left.end()) {
    Cerr << "\nError: provided type of linear inequality is not valid." << std::endl;
    err_flag = true;
  }else {
    linearInequalityType = it->second;
  }

  if (err_flag)
    abort_handler(-1);
}


/** Return the type of linear inequality supported */
void OptTraits::get_linear_inequality_type(std::string& linear_inequality_type)
{
  bool err_flag = false;
    
  // check if provided type of inequality is a valid type
  boost::bimap< std::string, int >::right_const_iterator it = validConstraints.right.find(linearInequalityType);
  if(it == validConstraints.right.end()) {
    Cerr << "\nError: stored type of linear inequality is not valid." << std::endl;
    err_flag = true;
  }else {
    linear_inequality_type = it->second;
  }

  if (err_flag)
    abort_handler(-1);
}


/** Sets requiresBounds to true */
void OptTraits::set_requires_bounds(bool requires_bounds)
{
  requiresBounds = requires_bounds;
}


/** Return the value of requiresBounds */
bool OptTraits::get_requires_bounds()
{
  return requiresBounds;
}


/** Sets supportsLinearEquality to true */
void OptTraits::set_supports_linear_equality(bool supports_linear_equality)
{
  supportsLinearEquality = supports_linear_equality;
}


/** Return the value of supportsLinearEquality */
bool OptTraits::get_supports_linear_equality()
{
  return supportsLinearEquality;
}


/** Sets supportsNonlinearEquality to true */
void OptTraits::set_supports_nonlinear_equality(bool supports_nonlinear_equality)
{
  supportsNonlinearEquality = supports_nonlinear_equality;
}


/** Return the value of supportsNonlinearEquality */
bool OptTraits::get_supports_nonlinear_equality()
{
  return supportsNonlinearEquality;
}


/** Sets supportsScaling to true */
void OptTraits::set_supports_scaling(bool supports_scaling)
{
  supportsScaling = supports_scaling;
}


/** Return the value of supportsScaling */
bool OptTraits::get_supports_scaling()
{
  return supportsScaling;
}


/** Sets supportsLeastSquares to true */
void OptTraits::set_supports_least_squares(bool supports_least_squares)
{
  supportsLeastSquares = supports_least_squares;
}


/** Return the value of supportsLeastSquares */
bool OptTraits::get_supports_least_squares()
{
  return supportsLeastSquares;
}


/** Sets supportsMultiobjectives to true */
void OptTraits::set_supports_multiobjectives(bool supports_multiobjectives)
{
  supportsMultiobjectives = supports_multiobjectives;
}


/** Return the value of supportsMultiobjectives */
bool OptTraits::get_supports_multiobjectives()
{
  return supportsMultiobjectives;
}


/** Sets supportsContinuousVariables to true */
void OptTraits::set_supports_continuous_variables(bool supports_continuous_variables)
{
  supportsContinuousVariables = supports_continuous_variables;
}


/** Return the value of supportsContinuousVariables */
bool OptTraits::get_supports_continuous_variables()
{
  return supportsContinuousVariables;
}


/** Sets supportsIntegerVariables to true */
void OptTraits::set_supports_integer_variables(bool supports_integer_variables)
{
  supportsIntegerVariables = supports_integer_variables;
}


/** Return the value of supportsIntegerVariables */
bool OptTraits::get_upports_integer_variables()
{
  return supportsIntegerVariables;
}


/** Sets supportsRelaxableDiscreteVariables to true */
void OptTraits::set_supports_relaxable_discrete_variables(bool relaxable_discrete_variables)
{
  supportsRelaxableDiscreteVariables = relaxable_discrete_variables;
}


/** Return the value of supportsRelaxableDiscreteVariables */
bool OptTraits::get_supports_relaxable_discrete_variables()
{
  return supportsRelaxableDiscreteVariables;
}


/** Sets supportsCategoricalVariables to true */
void OptTraits::set_supports_categorical_variables(bool supports_categorical_variables)
{
  supportsCategoricalVariables = supports_categorical_variables;
}


/** Return the value of supportsCategoricalVariables */
bool OptTraits::get_supports_categorical_variables()
{
  return supportsCategoricalVariables;
}


/** Sets providesBestObjective to true */
void OptTraits::set_provides_best_objective(bool provides_best_objective)
{
  providesBestObjective = provides_best_objective;
}


/** Return the value of providesBestObjective */
bool OptTraits::get_provides_best_objective()
{
  return providesBestObjective;
}


/** Sets providesBestParameters to true */
void OptTraits::set_provides_best_parameters(bool provides_best_parameters)
{
  providesBestParameters = provides_best_parameters;
}


/** Return the value of providesBestParameters */
bool OptTraits::get_provides_best_parameters()
{
  return providesBestParameters;
}


/** Sets providesBestConstraint to true */
void OptTraits::set_provides_best_constraint(bool provides_best_constraint)
{
  providesBestConstraint = provides_best_constraint;
}


/** Return the value of providesBestConstraint */
bool OptTraits::get_provides_best_constraint()
{
  return providesBestConstraint;
}


/** Sets providesFinalGradient to true */
void OptTraits::set_provides_final_gradient(bool provides_final_gradient)
{
  providesFinalGradient = provides_final_gradient;
}


/** Return the value of providesFinalGradient */
bool OptTraits::get_provides_final_gradient()
{
  return providesFinalGradient;
}


/** Sets providesFinalHessian to true */
void OptTraits::set_provides_final_hessian(bool provides_final_hessian)
{
  providesFinalHessian = provides_final_hessian;
}


/** Return the value of providesFinalHessian */
bool OptTraits::get_provides_final_hessian()
{
  return providesFinalHessian;
}

} // namespace Dakota
