/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        OptTraits
//- Description:  Traits class used in registering the various functionalities
//                supported by individual Dakota optimization TPLs.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $

#ifndef DAKOTA_OPT_TRAITS_H
#define DAKOTA_OPT_TRAITS_H

namespace Dakota {


/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, and JEGAOptimizer. */

class OptTraits
{
public:

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  OptTraits();

  /// destructor
  ~OptTraits();

  //
  //- Heading: Convenience/Helper functions
  //

  /// Sets supportsMultiobjectives to true
  void set_supports_multiobjectives();

  /// Return the value of supportsMultiobjectives
  void get_supports_multiobjectives(bool supports_multiobjectives);

  /// Sets the type of linear inequality supported
  void set_linear_inequality_type(const std::string& linear_inequality_type);

  /// Return the type of linear inequality supported
  void get_linear_inequality_type(std::string& linear_inequality_type);

  /// Sets the type of nonlinear inequality supported
  void set_nonlinear_inequality_type(const std::string& nonlinear_inequality_type);

  /// Return the type of nonlinear inequality supported
  void get_nonlinear_inequality_type(std::string& nonlinear_inequality_type);

  /// Sets the ordered list of accepted responses
  void set_response_order(const std::vector<std::string>& response_order);

  /// Return the ordered list of accepted responses
  void get_response_order(std::vector<std::string>& response_order);

  //
  //- Heading: Data
  //
  
  /// flag indicating whether method supports multiobjective optimization
  bool supportsMultiobjectives;

  /// structure conatining supported constraint forms
  boost::bimap < std::string, int > validConstraints;

  /// type of linear inequality supported
  int linearInequalityType;

  /// type of nonlinear inequality supported
  int nonlinearInequalityType;

  /// structure conatining supported response forms
  boost::bimap < std::string, int > validResponses;

  /// ordered list of accepted responses
  std::vector<int> responseOrder;


private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //
};


inline OptTraits::OptTraits()
{ }


inline OptTraits::~OptTraits()
{ }


} // namespace Dakota

#endif
