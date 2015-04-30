/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file ValidateParameter.h
 * 
 * Defines the utilib::Parameter class.
 */

#ifndef utilib_ValidateParameter_h
#define utilib_ValidateParameter_h

#include <utilib/std_headers.h>
#include <utilib/TypeManager.h>
#include <utilib/Any.h>

namespace utilib {

/**
 * The base class for a container object that can validate a parameter.
 */
class ParameterValidatorBase
{
public:

  /// Destructor
  virtual ~ParameterValidatorBase() {}

  /// Routine used to validate an Any object
  virtual bool validate(Any& info) = 0;

  /// Write information about the validation criteria
  virtual void write(std::ostream& os) const = 0;

};


/**
 * A generic base class for the function objects that are used by
 * ParameterValidator.  Deriving from this base class allows users
 * to simplify their create_option() declaration by omitting the second
 * template parameter.
 */
class ParameterValidatorFunction : public std::unary_function<Any,bool>
{
};


/**
 * Defines a class that can validate a parameter given a 
 * unary_function object.
 */
template <class Type, class ValidatorFuncT=ParameterValidatorFunction >
class ParameterValidator : public ParameterValidatorBase
{
public:

  /// Constructor
  ParameterValidator(ValidatorFuncT func)
	: test_func(func) {}

  ///
  bool validate(Any& info)
	{
    Type tmp;
    TypeManager()->lexical_cast(info,tmp);
    return test_func( tmp );
    }

  ///
  void write(std::ostream& os) const
	{ test_func.write(os); }

  /// Validation object
  ValidatorFuncT test_func;

};




/**
 * Class that validates a lower bound of a parameter.
 */
template <class Type>
class ParameterLowerBound : public std::unary_function<Any,bool>
{
public:

  /// Constructor
  ParameterLowerBound(Type lower_) : lower(lower_) {}

  /// Write information about the validation criteria
  void write(std::ostream& os) const
	{os << "Lower bound: " << lower;}

  /// Return a boolean that indicates whether the parameter value is valid
  bool operator()(const Type& x)
	{return (x >= lower);}

  /// The lower bound of this parameter
  Type lower;
};


/**
 * Class that validates an upper bound of a parameter.
 */
template <class Type>
class ParameterUpperBound : public std::unary_function<Any,bool>
{
public:

  /// Constructor
  ParameterUpperBound(Type upper_) : upper(upper_) {}

  /// Write information about the validation criteria
  void write(std::ostream& os) const
	{os << "Upper bound: " << upper;}

  /// Return a boolean that indicates whether the parameter value is valid
  bool operator()(const Type& x)
	{return (x <= upper);}

  /// The upper bound of this parameter
  Type upper;
};


/**
 * Class that validates a lower and upper bounds of a parameter.
 */
template <class Type>
class ParameterBounds : public std::unary_function<Any,bool>
{
public:

  /// Constructor
  ParameterBounds(Type lower_, Type upper_) : lower(lower_), upper(upper_) {}

  /// Write information about the validation criteria
  void write(std::ostream& os) const
	{os << "Lower bound: " << lower << "  Upper bound: " << upper;}

  /// Return a boolean that indicates whether the parameter value is valid
  bool operator()(const Type& x)
	{
    return ((x >= lower) && (x <= upper));
    }

  /// The lower bound of this parameter
  Type lower;

  /// The upper bound of this parameter
  Type upper;
};


/**
 * Class that validates a non-negative parameter.
 */
template <class Type>
class ParameterNonnegative : public std::unary_function<Any,bool>
{
public:

  /// Constructor
  ParameterNonnegative() {}

  /// Write information about the validation criteria
  void write(std::ostream& os) const
	{os << "Nonnegative";}

  /// Return a boolean that indicates whether the parameter value is valid
  bool operator()(const Type& x)
	{return (x >= static_cast<Type>(0));}
};


/**
 * Class that validates a positive parameter.
 */
template <class Type>
class ParameterPositive : public std::unary_function<Any,bool>
{
public:

  /// Constructor
  ParameterPositive() {}

  /// Write information about the validation criteria
  void write(std::ostream& os) const
	{os << "Positive";}

  /// Return a boolean that indicates whether the parameter value is valid
  bool operator()(const Type& x)
	{return (x > static_cast<Type>(0));}
};

} // namespace utilib


//============================================================================
//
/// Write a validator object
inline std::ostream& operator<< (std::ostream& ostr, const utilib::ParameterValidatorBase&
 item)
{ item.write(ostr); return ostr; }


#endif
