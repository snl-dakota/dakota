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
supportsMultiobjectives(false)
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
void OptTraits::set_requires_bounds()
{
  requiresBounds = true;
}


/** Return the value of requiresBounds */
void OptTraits::get_requires_bounds(bool requires_bounds)
{
  requires_bounds = requiresBounds;
}


/** Sets supportsLinearEquality to true */
void OptTraits::set_supports_linear_equality()
{
  supportsLinearEquality = true;
}


/** Return the value of supportsLinearEquality */
void OptTraits::get_supports_linear_equality(bool supports_linear_equality)
{
  supports_linear_equality = supportsLinearEquality;
}


/** Sets supportsLinearInequality to true */
void OptTraits::set_supports_linear_inequality()
{
  supportsLinearInequality = true;
}


/** Return the value of supportsLinearInequality */
void OptTraits::get_supports_linear_inequality(bool supports_linear_inequality)
{
  supports_linear_inequality = supportsLinearInequality;
}


/** Sets supportsNonlinearEquality to true */
void OptTraits::set_supports_nonlinear_equality()
{
  supportsNonlinearEquality = true;
}


/** Return the value of supportsNonlinearEquality */
void OptTraits::get_supports_nonlinear_equality(bool supports_nonlinear_equality)
{
  supports_nonlinear_equality = supportsNonlinearEquality;
}


/** Sets supportsNonlinearInequality to true */
void OptTraits::set_supports_nonlinear_inequality()
{
  supportsNonlinearInequality = true;
}


/** Return the value of supportsNonlinearInequality */
void OptTraits::get_supports_nonlinear_inequality(bool supports_nonlinear_inequality)
{
  supports_nonlinear_inequality = supportsNonlinearInequality;
}


/** Sets supportsScaling to true */
void OptTraits::set_supports_scaling()
{
  supportsScaling = true;
}


/** Return the value of supportsScaling */
void OptTraits::get_supports_scaling(bool supports_scaling)
{
  supports_scaling = supportsScaling;
}


/** Sets supportsLeastSquares to true */
void OptTraits::set_supports_least_squares()
{
  supportsLeastSquares = true;
}


/** Return the value of supportsLeastSquares */
void OptTraits::get_supports_least_squares(bool supports_least_squares)
{
  supports_least_squares = supportsLeastSquares;
}


/** Sets supportsMultiobjectives to true */
void OptTraits::set_supports_multiobjectives()
{
  supportsMultiobjectives = true;
}


/** Return the value of supportsMultiobjectives */
void OptTraits::get_supports_multiobjectives(bool supports_multiobjectives)
{
  supports_multiobjectives = supportsMultiobjectives;
}

} // namespace Dakota
