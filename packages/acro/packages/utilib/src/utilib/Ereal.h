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
 * \file Ereal.h
 *
 * Defines the utilib::Ereal class and associated operators
 */

#ifndef utilib_Ereal_h
#define utilib_Ereal_h

#include <utilib/std_headers.h>
#include <utilib/CommonIO.h>
#include <utilib/math_basic.h>
#include <utilib/PackObject.h>
#include <utilib/traits.h>

#include <utilib/TypeManager.h>
#include <utilib/Serialize.h>

namespace utilib {

/// A Trait structure used to indicate whether a data type is a 'real'
template <class Type>
struct is_real {
  public:
  /// The value of this trait.
  static const bool value = false;
};

}

#if !defined(DOXYGEN)
SetTrait(utilib,is_real,float,true)
SetTrait(utilib,is_real,double,true)
#endif

namespace utilib {


template <class Type> class Ereal;

template <class Type> Ereal<Type>
operator+ (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> Ereal<Type>
operator+ (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> Ereal<Type>
operator+ (const NType& x, const Ereal<Type>& y);

template <class Type> Ereal<Type>
operator- (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> Ereal<Type>
operator- (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> Ereal<Type>
operator- (const NType& x, const Ereal<Type>& y);

template <class Type> Ereal<Type>
operator* (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> Ereal<Type>
operator* (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> Ereal<Type>
operator* (const NType& x, const Ereal<Type>& y);

template <class Type> Ereal<Type>
operator/ (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> Ereal<Type>
operator/ (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> Ereal<Type>
operator/ (const NType& x, const Ereal<Type>& y);

template <class Type> Ereal<Type>
operator+ (const Ereal<Type>& x);

template <class Type> Ereal<Type> 
operator- (const Ereal<Type>& num);

template <class LType, class RType> bool
operator== (const Ereal<LType>& x, const Ereal<RType>& y);

template <class Type, class NType> bool
operator== (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator== (const NType& x, const Ereal<Type>& y);

template <class Type> bool
operator!= (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> bool
operator!= (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator!= (const NType& x, const Ereal<Type>& y);

template <class Type> bool
operator> (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> bool
operator> (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator> (const NType& x, const Ereal<Type>& y);

template <class Type> bool
operator>= (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> bool
operator>= (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator>= (const NType& x, const Ereal<Type>& y);

template <class Type> bool
operator< (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> bool
operator< (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator< (const NType& x, const Ereal<Type>& y);

template <class Type> bool
operator<= (const Ereal<Type>& x, const Ereal<Type>& y);

template <class Type, class NType> bool
operator<= (const Ereal<Type>& x, const NType& y);

template <class Type, class NType> bool
operator<= (const NType& x, const Ereal<Type>& y);

template <class Type>
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
bool Ereal_isinf(const Ereal<Type>& num);
#else
bool isinf(const Ereal<Type>& num);
#endif

template <class Type>
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
bool Ereal_isnan(const Ereal<Type>& num);
#else
bool isnan(const Ereal<Type>& num);
#endif

template <class Type>
bool isind(const Ereal<Type>& num);

template <class Type>
bool isinv(const Ereal<Type>& num);

template <class Type>
bool finite(const Ereal<Type>& num);


/**
 * \brief Defines an extension of 'real' data types (e.g. double, float, long
 * double)  that can 'assume' all 'real' values, as well as negative infinity
 * and positive infinity.
 */
template <class Type> // is_real<Type>::value>
class Ereal : public PackObject
{
public:

  
  /**
   * \brief Constructor that initializes the Ereal with a value.
   *
   * Creates a finite Ereal with the specified value.
   *
   * \param num The numeric value to give this Ereal.
   */
  Ereal(const Type num=0.0);

  
  /**
   * \brief Copy constructor.
   *
   * \param num A reference to the Ereal to create a copy of.
   */
  Ereal(const Ereal& num)
     : PackObject(num)
  {
    static_cast<void>(registrations_complete);
    val    = num.val;
    Finite = num.Finite;
  }


  /**
   * \brief Copy an Ereal object.
   *
   * \param The object to be copied.
   */
  Ereal<Type> operator=(const Ereal<Type>& num)
  {
	  val = num.val;
	  Finite = num.Finite;
	  return *this;
  }


  /// Coerce the Ereal to an integer
  operator int () const
	{ return this->as_int();}


  /**
   * \brief Returns the value of the \a conservativeError flag.
   *
   * \return \p true if conservative error handling is in effect, \p false if
   * it is not
   *
   * \see conservativeError, setConservativeError
   */
  static bool getConservativeError()
  {
    return conservativeError;
  }

  
  /**
   * \brief Sets the conservativeError flag.
   *
   * Setting \a flag to \p true will cause exceptions to be thrown when an
   * arithmetic operation results in either an indeterminate value or in NaN.
   * Setting \a flag to \p false turns this behavior off.  The default behavior
   * is not to throw exceptions.
   *
   * \note This flag has no effect on exceptions thrown as a result of using an
   * indeterminate value or NaN in a relational operation.
   *
   * \param flag \p true to enable conservative error handling, \p false to
   * disable it.
   *
   * \see conservativeError, getConservativeError, plus, minus, mult, div
   */
  static void setConservativeError(bool flag)
  {
    conservativeError = flag;
  }

  
  /** \brief The plus-equals operator.
   *
   * Performs addition according to the rules described for Ereal::plus.  The
   * right-hand side will be added to the left-hand side, and the result will
   * be stored in the left-hand side.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of addition is either an indeterminate value or is NaN.
   *
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   * 
   * \see plus, conservativeError, getConservativeError, setConservativeError
   */
  const Ereal& operator+= (const Ereal<Type>& num)
  {
    plus(val,Finite,num.val,num.Finite,val,Finite);
    return *this;
  }

  
  /**
   * \brief The minus-equals operator.
   *
   * Performs subtraction according to the rules described for Ereal::minus.
   * The right-hand side will be subtracted from the left-hand side, and the
   * result will be stored in the left-hand side.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of subtraction is either an indeterminate value or is NaN.  
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   * 
   * \see minus, conservativeError, getConservativeError, setConservativeError
   */
  const Ereal& operator-= (const Ereal<Type>& num)
  {
    minus(val,Finite,num.val,num.Finite,val,Finite);
    return *this;
  }

  
  /** \brief The times-equals operator.
   *
   * Performs multiplication according to the rules described for Ereal::mult.
   * The left-hand side will be multiplied by the right-hand side, and the
   * result will be stored in the left-hand side.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of multiplication is either an indeterminate value or is NaN.
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   * 
   * \see mult, conservativeError, getConservativeError, setConservativeError
   */
  const Ereal& operator*= (const Ereal<Type>& num)
  {
    mult(val,Finite,num.val,num.Finite,val,Finite);
    return *this;
  }

  
  /** \brief The divide-equals operator.
   *
   * Performs division according to the rules described for Ereal::div.  The
   * left-hand side will be divided by the right-hand side, and the result will
   * be stored in the left-hand side.
   *
   * \throw domain_error if division by zero is attempted.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of division is either an indeterminate value or is NaN.
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   * 
   * \see div, conservativeError, getConservativeError, setConservativeError
   */
  const Ereal& operator/= (const Ereal<Type>& num)
  {
    div(val,Finite,num.val,num.Finite,val,Finite);
    return *this;
  }

  
  /**
   * \brief The plus-equals operator.
   *
   * \overload
   */
  template <class NType>
  const Ereal& operator+= (const NType& num)
  {
    Type xval = num;
    bool xFinite = true;
    Ereal<Type>::check_if_infinite(val,Finite);

    plus(xval,xFinite,val,Finite,val,Finite);
    return *this;
  }

  
  /**
   * \brief The minus-equals operator.
   *
   * \overload
   */
  template <class NType>
  const Ereal& operator-= (const NType& num)
  {
    Type xval = num;
    bool xFinite = true;
    Ereal<Type>::check_if_infinite(val,Finite);
    
    minus(val,Finite,xval,xFinite,val,Finite);
    return *this;
  }

  
  /**
   * \brief The times-equals operator.
   *
   * \overload
   */
  template <class NType>
  const Ereal& operator*= (const NType& num)
  {
    Type xval = num;
    bool xFinite = true;
    Ereal<Type>::check_if_infinite(val,Finite);
    
    mult(xval,xFinite,val,Finite,val,Finite);
    return *this;
  }

  
  /**
   * \brief The divide-equals operator.
   *
   * \overload
   */
  template <class NType>
  const Ereal& operator/= (const NType& num)
  {
    Type xval = num;
    bool xFinite = true;
    Ereal<Type>::check_if_infinite(val,Finite);
    
    div(val,Finite,xval,xFinite,val,Finite);
    return *this;
  }

  
  /**
   * \brief Coerces the Ereal to type \a Type.
   *
   * \throw bad_cast if an attempt is made to cast either an indeterminate
   * value or NaN.
   *
   * \throw runtime_error If an Ereal with an invalid internal state is
   * enountered.
   *
   * \see isind, isnan, isinv
   */
  operator Type () const;

  /// Return the value of this Ereal converted to an integer
  int as_int() const;

  ///
  void write(std::ostream& os) const;

  ///
  void write(PackBuffer& os) const;

  ///
  void read(std::istream& is);
  
  ///
  void read(UnPackBuffer& is);

  
  /**
   * \brief Plus operator: Ereal + Ereal
   *
   * Performs addition on two Ereals according to the rules described in
   * Ereal::plus.
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return A new Ereal that is the sum of \a x and \a y.
   *
   * \throw range_error if conservative error handling is enabled and the result
   * of addition is either an indeterminate value or is NaN.
   *
   * \throw runtime_error if an Ereal with an invalid internal state is
   * encountered.
   *
   * \see plus, conservativeError, getConservativeError, setConservativeError
   */
  template <class LType>
  friend Ereal<LType> operator+ (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Plus operator: Ereal + Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator+ (const Ereal<LType>& x, const NType& y);

  
  /**
   * \brief Plus operator: Type + Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator+ (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Plus-equals operator: Type += Ereal
   *
   * \overload
   */
  template <class LType>
  friend LType operator += (LType& x, const Ereal<LType>& y);


  /**
   * \brief Minus operator: Ereal - Ereal
   *
   * Performs subtraction on two Ereals according to the rules described in
   * Ereal::minus.
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return A new Ereal that is the diferrence of \a x and \a y.
   *
   * \throw range_error if conservative error handling is enabled and the result
   * of subtraction is either an indeterminate value or is NaN.
   *
   * \throw runtime_error if an Ereal with an invalid internal state is
   * encountered.
   *
   * \see minus, conservativeError, getConservativeError, setConservativeError
   */
  template <class LType>
  friend Ereal<LType> operator- (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Minus operator: Ereal - Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator- (const Ereal<LType>& x, const NType& y);

  
  /**
   * \brief Minus operator: Type - Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator- (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Minus-equals operator: Type -= Ereal
   *
   * \overload
   */
  template <class LType>
  friend LType operator -= (LType& x, const Ereal<LType>& y);


  /**
   * \brief Times operator: Ereal * Ereal
   *
   * Performs multiplication on two Ereals according to the rules described in
   * Ereal::mult.
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return A new Ereal that is the product of \a x and \a y.
   *
   * \throw range_error if conservative error handling is enabled and the result
   * of multiplication either an indeterminate value or is NaN.
   *
   * \throw runtime_error if an Ereal with an invalid internal state is
   * encountered.
   *
   * \see mult, conservativeError, getConservativeError, setConservativeError
   */
  template <class LType>
  friend Ereal<LType> operator* (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Times operator: Ereal * Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator* (const Ereal<LType>& x, const NType& y);

  
  /**
   * \brief Times operator: Type * Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator* (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Times-equals operator: Type *= Ereal
   *
   * \overload
   */
  template <class LType>
  friend LType operator *= (LType& x, const Ereal<LType>& y);


  /**
   * \brief Division operator: Ereal / Ereal
   *
   * Performs division on two Ereals according to the rules described in
   * Ereal::div.
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return A new Ereal that is the quotient of \a x and \a y.
   *
   * \throw domain_error if division by zero is attempted.
   *
   * \throw range_error if conservative error handling is enabled and the result
   * of division either an indeterminate value or is NaN.
   *
   * \throw runtime_error if an Ereal with an invalid internal state is
   * encountered.
   *
   * \see div, conservativeError, getConservativeError, setConservativeError
   */
  template <class LType>
  friend Ereal<LType> operator/ (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Division operator: Ereal / Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator/ (const Ereal<LType>& x, const NType& y);

  
  /**
   * \brief Division operator: Type / Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend Ereal<LType> operator/ (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Division-equals operator: Type /= Ereal
   *
   * \overload
   */
  template <class LType>
  friend LType operator /= (LType& x, const Ereal<LType>& y);


  /**
   * \brief Unary plus operator.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  friend Ereal<Type> operator+ <> (const Ereal<Type>& x);

  
  /**
   * \brief Unary negation operator.
   *
   * \param num The Ereal to negate.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  friend Ereal<Type> operator- <>(const Ereal<Type>& num);

  
  /**
   * \brief Equality operator: Type == Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator== (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Equality operator: Ereal == Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if x and y are equal, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  template <class LType, class RType>
  friend bool operator== (const Ereal<LType>& x, const Ereal<RType>& y);

  
  /**
   * \brief Equality operator: Ereal == Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator== (const Ereal<LType>& x, const NType& y);


  /**
   * \brief Non-equality operator: Type != Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator!= (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Non-equality operator: Ereal != Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if x and y are not equal, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  friend bool operator!= <Type> (const Ereal<Type>& x, const Ereal<Type>& y);

  
  /**
   * \brief Non-equality operator: Ereal != Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator!= (const Ereal<LType>&, const NType&);

  
  /**
   * \brief Less-Than operator: Ereal < Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if \a x is less than \a y, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  template <class LType>
  friend bool operator< (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Less-Than operator: Ereal < Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator< (const Ereal<LType>&, const NType&);

  
  /**
   * \brief Less-Than operator: Type < Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator< (const NType& x, const Ereal<LType>& y);

  
  /**
   * \brief Less-Than-or-Equal operator: Ereal <= Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if \a x is less than or equal to \a y, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  template <class LType>
  friend bool operator<= (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Less-Than-or-Equal operator: Ereal <= Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator<= (const Ereal<LType>&, const NType&);

  
  /**
   * \brief Less-Than-or-Equal operator: Type <= Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator<= (const NType& x, const Ereal<LType>& y);


  /**
   * \brief Greater-Than operator: Ereal > Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if \a x is greater than \a y, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  template <class LType>
  friend bool operator> (const Ereal<LType>& x, const Ereal<LType>& y);

  
  /**
   * \brief Greater-Than operator: Ereal > Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator> (const Ereal<LType>& x, const NType& y);

  
  /**
   * \brief Greater-Than operator: Type > Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator> (const NType& x, const Ereal<LType>& y);

  
#if 0
  /**
   * \brief Greater-Than-or-Equal operator: Ereal >= Ereal
   *
   * \param x The left-hand side operand.
   *
   * \param y The right-hand side operand.
   *
   * \return \p true if \a x is greater than or equal to \a y, \p false otherwise.
   *
   * \throw invalid_argument if an attempt is made to use an indeterminate
   * value or NaN in a comparison.
   *
   * \throw runtime_error if it is passed an Ereal with an invalid internal
   * state.
   */
  template <class LType>
  friend bool operator<= (const Ereal<LType>&, const Ereal<LType>&);

  
  /**
   * \brief Greater-Than-or-Equal operator: Ereal >= Type
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator<= (const Ereal<LType>&, const NType&);

  
  /**
   * \brief Greater-Than-or-Equal operator: Type >= Ereal
   *
   * \overload
   */
  template <class LType, class NType>
  friend bool operator<= (const NType& x, const Ereal<LType>& y);
#endif


  /**
   * \brief Check if Ereal is infinite.
   *
   * \param num The Ereal to check.
   *
   * \return \p true if \a num is negative infinity or positive infinity,
   * \p false otherwise.
   *
   * \note This will still return false for non-finite Ereals such as an
   * indeterminate value or NaN, so this should \b not be used to check for
   * finite/non-finite.  If you need to make such a comparison, use
   * Ereal::finite instead.
   *
   * \see finite, isnan, isind, isinv, negative_infinity, positive_infinity
   */
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
  friend bool Ereal_isinf<> (const Ereal<Type>& num);
#else
  friend bool isinf<> (const Ereal<Type>& num);
#endif

  
  /**
   * \brief Check if Ereal is NaN.
   *
   * \param num The Ereal to check.
   *
   * \return \p true if \a num is NaN, \p false otherwise.
   *
   * \see isinf, isind, isinv, finite
   */
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
  friend bool Ereal_isnan<> (const Ereal<Type>& num);
#else
  friend bool isnan<> (const Ereal<Type>& num);
#endif

  
  /**
   * \brief Check if the Ereal is an indeterminate value.
   *
   * \param num The Ereal to check.
   *
   * \return \p true if \a num is indeterminate, \p false otherwise.
   *
   * \see isinf, isnan, isinv, finite
   */
#if defined(COUGAR) || defined(SGI)
  friend bool utilib::isind<> (const Ereal<Type>& num);
#else
  friend bool isind<> (const Ereal<Type>& num);
#endif

  
  /**
   * \brief Check if the Ereal has an invalid internal state.
   *
   * \param num The Ereal to check.
   *
   * \return \p true if \a num has an invalid internal state, \p false
   * otherwise.
   *
   * \see isinf, isnan, isind, finite
   */
  friend bool isinv<> (const Ereal<Type>& num);

  
  /**
   * \brief Check if Ereal is finite.
   *
   * \param num The Ereal to check.
   *
   * \return \p true if \a num is finite, \p false otherwise.
   *
   * \note This will return \p false for any non-finite Ereal, including
   * indeterminate values and NaN, so a return value of \p false does \b not
   * imply that \a num is negative infinity or positive infinity.  If you need
   * to check specifically for negative infinity or positive infinity, use
   * Ereal::isinf instead.
   *
   * \see finite, isnan, isind, isinv, positive_infinity, negative_infinity
   */
  friend bool finite<> (const Ereal<Type>& num);

  
  /**
   * \brief A standard definition of positive infinity.
   */
  static Ereal<Type> positive_infinity;

  
  /**
   * \brief A standard definition of negative infinity.
   */
  static Ereal<Type> negative_infinity;

  
  /**
   * \brief A standard definition of NaN.
   */
  static Ereal<Type> NaN;

  
  /**
   * \brief A standard definition of an indeterminate value.
   */
  static Ereal<Type> indeterminate;

  
  /**
   * \brief A standard definition of an invalid number.
   *
   * This is only here for use in testing, and should not be used for other
   * purposes.
   */
  static Ereal<Type> invalid;

#ifndef DARWIN 
#if ( (__GNUC__ != 3) || (__GNUC_MINOR__ != 3) ) 
  // JPW: under gcc 3.3, protection of this constructor prevents initialization
  //      of the static positive_infinity and negative_infinity members (the 
  //      initializer can't call the constructor even though it's in the same
  //      class). This is a compiler bug - I think.
protected:
#endif
#endif

  /**
   * \brief A constructor that specifies the internal states of the Ereal.
   *
   * \param num The numeric value of this Ereal.
   *
   * \param f_flag \p true is this Ereal is finite, \p false otherwise.
   */
  Ereal(const Type num, const bool f_flag) : val(num), Finite(f_flag) 
  { static_cast<void>(registrations_complete); }

#ifdef DARWIN
 protected:
#endif
  
  /** 
   * \brief The numeric, 'real' value of the Ereal.
   */
  Type val;

  
  /**
   * \brief A flag that indicates whether or not the Ereal is finite
   * 
   * If \a Finite is \p true, then this Ereal represents a finite number, if \a
   * Finite is \p false then this Ereal is non-finite.
   *
   * \note Non-finite does \b not imply negative infinity or positive infinity.
   * Indeterminate values, NaN, and Ereals with invalid internal states are
   * also considered non-finite.
   *
   * \see finite, isinf, isind, isnan, isinv
   */
  bool Finite;

  
  /**
   * \brief A flag that controls whether or not an exception will be thrown if
   * an aritmetic operation results in an indeterminate value or in NaN.
   *
   * \see setConservativeError, getConservativeError
   */
  static bool conservativeError;

  
  /**
   * \brief A plus operator.
   *
   * Performs the operation \a "r = x + y" according to the following rules:
   *
   * \li x is finite, \a y is finite:  If \a (x+y) is greater than \a
   * positive_infinity_val, or is less than \a negative_infinity_val, \a r will
   * be positive infinity or negative infinity respectively.  Otherwise \a r is
   * the finite result of \a (x+y). 
   *
   * \li x is finite, \a y is negative infinity: \a r will be negative
   * infinity.
   *
   * \li x is finite, \a y is positive infinity: \a r will be positive
   * infinity.
   *
   * \li x is finite, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li x is finite, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \li \a x is negative infinity, \a y is finite: \a r will be negative
   * infinity.
   *
   * \li \a x is negative infinity, \a y is negative infinity: \a r will be
   * negative infinity.
   *
   * \li \a x is negative infinity, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is positive infinity, \a y is finite: \a r will be positive
   * infinity.
   *
   * \li \a x is positive infinity, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is positive infinity: \a r will be
   * positive infinity.
   *
   * \li \a x is positive infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   * 
   * \li \a x is indeterminate, \a y is finite: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is indeterminate: If conservative error
   * handling is enabled a \a range_error exception will be thrown, otherwise
   * \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is NaN: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is finite: If conservative error handling is enabled
   * a \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \li \a x is NaN, \a y is negative infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is positive infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \param xval The numeric value of \a x.
   * 
   * \param xFinite \p true if \a x represents a finite number.
   * 
   * \param yval The numeric value of \a y.
   * 
   * \param yFinite \p true if \a y represents a finite number.
   * 
   * \param rval Location to store the numeric value of \a r.
   * 
   * \param rFinite Location to specify if \a r is finite or not.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of addition is either an indeterminate value or is NaN.
   *
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   *
   * \see conservativeError, getConservativeError, setConservativeError
   */
  static void plus(const Type xval, const bool xFinite, 
		const Type yval, const bool yFinite, 
		Type& rval, bool& rFinite);

   
  /**
   * \brief A minus operator.
   *
   * Performs the operation \a "r = x - y" according to the following rules:
   * 
   * \li x is finite, \a y is finite:  If \a (x-y) is greater than \a
   * positive_infinity_val, or is less than \a negative_infinity_val, \a r will
   * be positive infinity or negative infinity respectively.  Otherwise \a r is
   * the finite result of \a (x-y). 
   *
   * \li x is finite, \a y is negative infinity: \a r will be positive
   * infinity.
   *
   * \li x is finite, \a y is positive infinity: \a r will be negative
   * infinity.
   *
   * \li x is finite, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li x is finite, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \li \a x is negative infinity, \a y is finite: \a r will be negative
   * infinity.
   *
   * \li \a x is negative infinity, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is positive infinity: \a r will be
   * negative infinity.
   *
   * \li \a x is negative infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is positive infinity, \a y is finite: \a r will be positive
   * infinity.
   *
   * \li \a x is positive infinity, \a y is negative infinity: \a r will be
   * positive infinity.
   *
   * \li \a x is positive infinity, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   * 
   * \li \a x is indeterminate, \a y is finite: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is indeterminate: If conservative error
   * handling is enabled a \a range_error exception will be thrown, otherwise
   * \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is NaN: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is finite: If conservative error handling is enabled
   * a \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \li \a x is NaN, \a y is negative infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is positive infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \param xval The numeric value of \a x.
   * 
   * \param xFinite \p true if \a x represents a finite number.
   * 
   * \param yval The numeric value of \a y.
   * 
   * \param yFinite \p true if \a y represents a finite number.
   * 
   * \param rval Location to store the numeric value of \a r.
   * 
   * \param rFinite Location to specify if \a r is finite or not.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of subtraction is either an indeterminate value or is NaN.
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   *
   * \see conservativeError, getConservativeError, setConservativeError
   */
  static void minus(const Type xval, const bool xFinite, 
		const Type yval, const bool yFinite, 
		Type& rval, bool& rFinite);

   
  /**
   * \brief A multiplication operator.
   *
   * Performs the operation \a "r = x * y" according the following rules:
   *
   * \li \a x is finite, \a y is finite: If \a (x*y) is greater than \a
   * positive_infinity_val, or is less than \a negative_infinity_val, \a r will
   * be positive infinity or negative infinity respectively.  Otherwise \a r is
   * the finite result of \a (x*y).
   *
   * \li \a x is finite and \a (x != 0), \a y is negative infinity: If \a (x >
   * 0) \a r will be negative infinity, otherwise \a r will be positive
   * infinity.
   * 
   * \li \a x is 0, \a y is negative infinity: If conservative error handling
   * is enabled a \a range_error exception is thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li \a x is finite and \a (x != 0), \a y is positive infinity: If \a (x >
   * 0) \a r will be positive infinity, otherwise \a r will be negative
   * infinity.
   * 
   * \li \a x is 0, \a y is positive infinity: If conservative error handling
   * is enabled a \a range_error exception is thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li \a x is finite, \a y is indeterminate: If conservative error handling
   * is enabled a \a range_error exception is thrown, otherwise \a r will be
   * indeterminate.
   * 
   * \li \a x is finite, \a y is NaN: If conservative error handling is enabled
   * a \a range_error exception is thrown, otherwise \a r will be NaN.
   * 
   * \li \a x is negative infinity,  \a y is finite and \a (y != 0): If \a (y >
   * 0) \a r will be negative infinity, otherwise \a r will be positive
   * infinity.
   * 
   * \li \a x is negative infinity, \a y is 0: If conservative error handling
   * is enabled a \a range_error exception is thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li \a x is negative infinity, \a y is negative infinity: \a r will be
   * positive infinity.
   *
   * \li \a x is negative infinity, \a y is positve infinity: \a r will be
   * negative infinity.
   *
   * \li \a x is negative infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is positive infinity,  \a y is finite and \a (y != 0): If \a (y >
   * 0) \a r will be positive infinity, otherwise \a r will be negative
   * infinity.
   * 
   * \li \a x is positive infinity, \a y is 0: If conservative error handling
   * is enabled a \a range_error exception is thrown, otherwise \a r will be
   * indeterminate.
   *
   * \li \a x is positive infinity, \a y is negative infinity: \a r will be
   * negative infinity.
   *
   * \li \a x is positive infinity, \a y is positve infinity: \a r will be
   * positive infinity.
   *
   * \li \a x is positive infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is indeterminate, \a y is finite: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
     * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is indeterminate: If conservative error
   * handling is enabled a \a range_error exception will be thrown, otherwise
   * \a r will be indeterminate.
   * 
   * \li \a x is indeterminate, \a y is NaN: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   * 
   * \li \a x is NaN, \a y is finite: If conservative error handling is enabled
   * a \a range_error exception will be thrown, otherwise \a r will be NaN.
   * 
   * \li \a x is NaN, \a y is negative infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   * 
   * \li \a x is NaN, \a y is positive infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   * 
   * \li \a x is NaN, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   * 
   * \li \a x is NaN, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   * 
   * \param xval The numeric value of \a x.
   * 
   * \param xFinite \p true if \a x represents a finite number.
   * 
   * \param yval The numeric value of \a y.
   * 
   * \param yFinite \p true if \a y represents a finite number.
   * 
   * \param rval Location to store the numeric value of \a r.
   * 
   * \param rFinite Location to specify if \a r is finite or not.
   *
   * \throw range_error if conservative error handling is enabled and the
   * result of multiplication is either an indeterminate value or is NaN.
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   *
   * \see conservativeError, getConservativeError, setConservativeError
   */
  static void mult(const Type xval, const bool xFinite, 
		const Type yval, const bool yFinite, 
		Type& rval, bool& rFinite);

   
  /**
   * \brief A division operator.
   *
   * Performs the operation \a "r = x / y" according to the following rules:
   * 
   * \li \a x is finite, \a y is finite: If y is 0, a \a domain_error exception
   * is thrown. If \a (x/y) is greater than \a positive_infinity_val, or is
   * less than \a negative_infinity_val, \a r will be positive infinity or
   * negative infinity respectively.  Otherwise \a r is the finite result of \a
   * (x/y). 
   *
   * \li \a x is finite, \a y is negative infinity: \a r will be 0
   *
   * \li \a x is finite, \a y is positive infinity: \a r will be 0
   *
   * \li \a x is finite, \a y is indeterminate:  If conservative error handling
   * is enabled, a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   *
   * \li \a x is finite, \a y is NaN:  If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is negative infinity, \a y is 0.0: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   *
   * \li \a x is negative infinity, \a y is finite and \a y > 0: \a r will be
   * negative infinity.
   *
   * \li \a x is negative infinity, \a y is finite and \a y < 0: \a r will be
   * positive infinity.
   *
   * \li \a x is negative infinity, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is negative infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is positive infinity, \a y is 0: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   *
   * \li \a x is positive infinity, \a y is finite and \a y > 0: \a r will be
   * positive infinity.
   *
   * \li \a x is positive infinity, \a y is finite and \a y < 0: \a r will be
   * negative infinity.
   *
   * \li \a x is positive infinity, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is indeterminate: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is positive infinity, \a y is NaN: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r
   * will be NaN.
   *
   * \li \a x is indeterminate, \a y is finite: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be indeterminate.
   *
   * \li \a x is indeterminate, \a y is negative infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is indeterminate, \a y is positive infinity: If conservative
   * error handling is enabled a \a range_error exception will be thrown,
   * otherwise \a r will be indeterminate.
   *
   * \li \a x is indeterminate, \a y is indeterminate: If conservative error
   * handling is enabled a \a range_error exception will be thrown, otherwise
   * \a r will be indeterminate.
   *
   * \li \a x is indeterminate, \a y is NaN: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is finite: If conservative error handling is enabled
   * a \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \li \a x is NaN, \a y is negative infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is positive infinity: If conservative error handling
   * is enabled a \a range_error exception will be thrown, otherwise \a r will
   * be NaN.
   *
   * \li \a x is NaN, \a y is indeterminate: If conservative error handling is
   * enabled a \a range_error exception will be thrown, otherwise \a r will be
   * NaN.
   *
   * \li \a x is NaN, \a y is NaN: If conservative error handling is enabled a
   * \a range_error exception will be thrown, otherwise \a r will be NaN.
   *
   * \param xval The numeric value of \a x.
   * 
   * \param xFinite \p true if \a x represents a finite number.
   * 
   * \param yval The numeric value of \a y.
   * 
   * \param yFinite \p true if \a y represents a finite number.
   * 
   * \param rval Location to store the numeric value of \a r.
   * 
   * \param rFinite Location to specify if \a r is finite or not.
   *
   * \throw domain_error if division of a finite value by 0 is attempted.
   * 
   * \throw range_error if conservative error handling is enabled and the
   * result of division is either an indeterminate value or is NaN.
   * 
   * \throw runtime_error If an Ereal with an invalid internal state is
   * encountered.
   *
   * \see conservativeError, getConservativeError, setConservativeError
   */
  static void div(const Type xval, const bool xFinite, 
		const Type yval, const bool yFinite, 
		Type& rval, bool& rFinite);

   
  /**
   * \brief Converts an Ereal to positive infinity or negative infinity if \a
   * val is large enough to be considered infinite.
   *
   * If \a val is larger than \a positive_infinity_val, \a val and \a Finite
   * will be set to the values used by Ereal to represent positive infinity.
   * Similarly, if \a val is smaller than a\ negative_infinity_val, \a val and
   * \a Finite will be set to the values ued by Ereal to represent negative
   * infinity.
   *
   * \note The actual values of \a positive_infinity_val and \a negative_infinity_val 
   * may vary depending on \a Type.
   *
   * \see positive_infinity_val, negative_infinity_val
   */
  static void check_if_infinite(Type& val, bool& Finite);

  
  /**
   * \brief The value of \a Type that represents positive infinity.
   */
  static const Type positive_infinity_val;

  
  /**
   * \brief The value of \a Type that represents negative infinity.
   */
  static const Type negative_infinity_val;

private:
  /// A flag to force the automagic registration of the Ereal serializer
  static const volatile bool registrations_complete;

  /// The Ereal serializer / deserializer
  static int serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize )
  {
     Ereal<Type> &tmp 
        = const_cast<Ereal<Type>&>(data.template expose<Ereal<Type> >());
     int ans = serial_transform(serial, tmp.val, serialize);
     if ( ans != 0 ) 
        return ans;
     return serial_transform(serial, tmp.Finite, serialize);
  }

   template<typename FROM, typename TO>
   static int assign_cast(const Any& from, Any& to)
   {
      to.set<TO>() = from.template expose<FROM>();
      return OK;
   }

   template<typename FROM, typename TO>
   static int stream_cast(const Any& from, Any& to)
   {
      to.set<std::vector<TO> >() << from.template expose<std::vector<FROM> >();
      return OK;
   }

   static bool register_aux_functions()
   {
      Serializer().template register_serializer<Ereal<Type> >
         ( std::string("utilib::Ereal;") + mangledName(typeid(Type)),
           Ereal<Type>::serializer );

      TypeManager()->register_lexical_cast
         (typeid(Type), typeid(Ereal<Type>), &assign_cast<Type, Ereal<Type> >);
      TypeManager()->register_lexical_cast
         (typeid(Ereal<Type>), typeid(Type), &assign_cast<Ereal<Type>, Type>);

      TypeManager()->register_lexical_cast
         ( typeid(std::vector<Type>), typeid(std::vector<Ereal<Type> >), 
           &stream_cast<Type, Ereal<Type> > );
      TypeManager()->register_lexical_cast
         ( typeid(std::vector<Ereal<Type> >), typeid(std::vector<Type>), 
           &stream_cast<Ereal<Type>, Type> );
      return true;
   }
}; //end class


template <class Type>
Ereal<Type> Ereal<Type>::positive_infinity(1.0,false);

template <class Type>
Ereal<Type> Ereal<Type>::negative_infinity(-1.0,false);

template <class Type>
Ereal<Type> Ereal<Type>::NaN(2.0,false);

template <class Type>
Ereal<Type> Ereal<Type>::indeterminate(0.0,false);

template <class Type>
Ereal<Type> Ereal<Type>::invalid(-2.0,false);


template <class Type>
inline void Ereal<Type>::check_if_infinite(Type& val, bool& Finite)
{
if (Finite) {
   if (val <= negative_infinity_val) {
      Finite = false;
      val = -1.0;
      }
   else if (val >= positive_infinity_val) {
      Finite = false;
      val = 1.0;
      }
   }
}

// Ereals support operator<() && operator==()
template<typename T>
class Any::Comparator<Ereal<T> > : public Any::DefaultComparable<Ereal<T> > 
{};

// Ereals support operator<<(std::ostream&)
template<typename T>
class Any::Printer<Ereal<T> > : public Any::DefaultPrinter<Ereal<T> > 
{};

template<typename T>
const volatile bool 
Ereal<T>::registrations_complete = Ereal<T>::register_aux_functions();


/// Construct an Ereal from num
template <class Type>
inline Ereal<Type>::Ereal(const Type num)
{
  static_cast<void>(registrations_complete);
  if (is_real<Type>::value == false)
     EXCEPTION_MNGR(std::runtime_error, "Ereal must be defined with a 'real' type: float or double.");
  val = num;
  Finite = true;
  check_if_infinite(val,Finite);
}


/// Coerce Ereal to Type
template <class Type>
inline Ereal<Type>::operator Type () const
{
  if (Finite) 
    return val;

  if (val == -1.0)      // -infinity
    return negative_infinity_val;
  else if (val == 1.0)  // +infinity
    return positive_infinity_val;
  else if (val == 2.0)  // NaN
     EXCEPTION_MNGR(std::invalid_argument, "Ereal:: Type (): Attempt to cast NaN to a numeric value.");
  else if (val == 0.0)  // indeterminate
     EXCEPTION_MNGR(std::invalid_argument, "Ereal:: Type (): Attempt to cast indeterminate value to a numeric value.");
  else                  // unknown, non-finite value
     EXCEPTION_MNGR(std::runtime_error,"Ereal:: Type (): Invalid internal state detected: val=" << val << ", finite=false");
  
  return negative_infinity_val; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Coerce Ereal to an integer
template <class Type>
inline int Ereal<Type>::as_int() const
{
  if (Finite) {
     if (val > INT_MAX)
        return INT_MAX;
     if (val < INT_MIN)
        return INT_MIN;
     return static_cast<int>(val);
  }

  if (val == -1.0)      // -infinity
    return INT_MIN;
  else if (val == 1.0)  // +infinity
    return INT_MAX;
  else if (val == 2.0)  // NaN
     EXCEPTION_MNGR(std::invalid_argument, "Ereal:: Type (): Attempt to cast NaN to a numeric value.");
  else if (val == 0.0)  // indeterminate
     EXCEPTION_MNGR(std::invalid_argument, "Ereal:: Type (): Attempt to cast indeterminate value to a numeric value.");
  else                  // unknown, non-finite value
     EXCEPTION_MNGR(std::runtime_error,"Ereal:: Type (): Invalid internal state detected: val=" << val << ", finite=false");
  
  return INT_MIN; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Addition operation
template <class Type> 
inline void Ereal<Type>::plus(const Type xval, const bool xFinite, 
			const Type yval, const bool yFinite, 
			Type& rval, bool& rFinite)
{
  if (xFinite) {
    if (yFinite) {  // x is finite, y is finite
      rFinite = true;
      rval = xval + yval;
      check_if_infinite(rval,rFinite);
    }
    else {          // x is finite, y is non-finite
      rFinite = false;
      if (conservativeError) {
        if (yval == 2.0)                      // y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in NaN.");
        else if (yval == 0.0)                 // y is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in an indeterminate value.");
        else if (yval == -1.0 || yval == 1.0) // y is -infinity or infinity
          rval = yval;
        else                                  // y is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
      }
      else {
        if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  //y is not a known, non-fintite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
        else
          rval = yval;
      }
    }
  }
  else {
    rFinite = false;
    if (yFinite) {  // x is non-finite, y is finite
      if (conservativeError) {
        if (xval == 2.0)                      // x is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in NaN.");
        else if (xval == 0.0)                 // x is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in an indeterminate value.");
        else if (xval == -1.0 || xval == 1.0) // x is -infinity or infinity
          rval = xval;
        else                                  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
      }
      else {
        if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
        else
          rval = xval;
      }
    }
    else {          // x is non-finite, y is non-finite
      if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");

      if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  // y is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::plus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");

      if (conservativeError) {
        if (xval == 2.0 || yval == 2.0) // x and/or y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in NaN.");
        else if (xval * yval == 0.0)    // x and/or y is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in an indeterminate value.");
        else if (xval * yval == -1.0)   // infinity - infinity
           EXCEPTION_MNGR(std::range_error, "Ereal::plus - addtion resulted in an indeterminate value.");
        else rval = xval;
      }
      else {
        if (xval == 2.0 || yval == 2.0) // x and/or y is NaN
          rval = 2.0;
        else if (xval * yval == 0.0)    // x and/or y is indeterminate
          rval = 0.0;
        else if (xval * yval == -1.0)   // infinity - infinity
          rval = 0.0;
        else
          rval = xval;                  
      }
    }
  }
}


/// Subtraction operation
template <class Type> 
inline void Ereal<Type>::minus(const Type xval, const bool xFinite, 
			const Type yval, const bool yFinite, 
			Type& rval, bool& rFinite)
{
  if (xFinite) {
    if (yFinite) {  // x is finite, y is finite
      rFinite = true;
      rval = xval - yval;
      check_if_infinite(rval,rFinite);
    }
    else {  // x is finite, y is non-finite
      rFinite = false;
      if (conservativeError) {
        if (yval == 0.0)                      // y is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in an indeterminate value.");
        else if (yval == 2.0)                 // y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in NaN.");
        else if (yval == -1.0 || yval == 1.0) // y is (+/-)infinity
          rval = -yval;
        else                                  // y is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
      }
      else {
        if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  // y is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");

        if (yval == 0.0 || yval == 2.0) // y is indeterminate or NaN
          rval = yval;
        else                            // y is (+/-)infinity
          rval = -yval;
      }
    }
  }
  else {
    rFinite = false;
    if (yFinite) {  // x is non-finite, y is finite
      if (conservativeError) {
        if (xval == 0.0)                      // x is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in an indeterminate value.");
        else if (xval == 2.0)                 // x in NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in NaN.");
        else if (xval == -1.0 || xval == 1.0)    // x is (+/-)infinity
          rval = xval;
        else                                  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
      }
      else {
        if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
        else                                  // x is +infinity, -infinity, indeterminate, or NaN
          rval = xval;
      }
    }
    else {  // x is non-finite, y is non-finite
      if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");

      if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  // y is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::minus - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
      
      if (conservativeError) {
        if (xval == 2.0 || yval == 2.0) // x and/or y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in NaN.");
        else if (xval * yval == 0.0)    // x and/or y is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in an indeterminate value.");
        else if (xval * yval == 1.0)     // infinity - infinity or -infinity - (-infinity)
           EXCEPTION_MNGR(std::range_error, "Ereal::minus - subtraction resulted in an indeterminate value.");
        else                            // -infinity - infinity or infinity - (-infinity)
          rval = xval;
      }
      else {
        if (xval == 2.0 || yval == 2.0) // x and/or y is NaN
          rval = 2.0;
        else if (xval * yval == 0.0)    // x and/or y is indeterminate
          rval = 0.0;
        else if (xval * yval == 1.0)     // infinity - infinity
          rval = 0.0;
        else                            // -infinity - infinity or infinity - (-infinity)
          rval = xval;
      }
    }
  }
}


/// Multiplication operation
template <class Type> 
inline void Ereal<Type>::mult(const Type xval, const bool xFinite, 
			const Type yval, const bool yFinite, 
			Type& rval, bool& rFinite)
{
  if (xFinite) {
    if (yFinite) {  // x is finite, y is finite
      rval = xval * yval;
      rFinite = true;
      check_if_infinite(rval,rFinite);
    }
    else {  // x is finite, y is non-finite
      rFinite = false;
      if (conservativeError) {
        if (yval == 2.0)                        // y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in NaN.");
        else if (yval == 0.0)                   // y is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in an indeterminate value.");
        else if (yval == -1.0 || yval == 1.0) { // y is (+/-)infinity
          if (xval == 0.0)                      // 0 * (+/-)infinity
             EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in an indeterminate value.");
          else if (xval > 0.0)
            rval = yval;                        // +finite * (+/-)infinity
          else
            rval = -yval;                       // -finite * (+/-)infinity
        }
        else                                    // y is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
      }
      else {
        if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  // y is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
        
        if (yval == 2.0)              // y is NaN
          rval = 2.0;
        else if (xval * yval == 0.0)  // (0 * (+/-)infinity) or (finite * indeterminate)
          rval = 0.0;
        else {
          if (xval > 0.0)             // +finite * (+/-)infinity
            rval = yval;
          else                        // -finite * (+/-)infinity
            rval = -yval;
        }
      }
    }
  }
  else {
    rFinite = false;
    if (yFinite) {  // x is non-finite, y is finite
      if (conservativeError) {
        if (xval == 2.0)                        // x is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in NaN.");
        else if (xval == 0.0)                   // x is indeterminate
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in an indeterminate value.");
        else if (xval == -1.0 || xval == 1.0) { // x is (+/-)infinity
          if (yval == 0.0)                      // (+/-)infinity * 0
             EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in an indeterminate value.");
          else if (yval > 0.0)                  // (+/-)infinity * +finite
            rval = xval;
          else                                  // (+/-)infinity * -finite
            rval = -xval;
        }
        else                                    // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
      }
      else {
        if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
        
        if (xval == 2.0)              // x is NaN
          rval = 2.0;
        else if (xval * yval == 0.0)  // ((+/-)infinity * 0) or (indeterminate * finite)
          rval = 0.0;
        else if (yval > 0.0)          // (+/-)infinity * +finite
          rval = xval;
        else                          // (+/-)infinity * -finite
          rval = -xval;
      }
    }
    else { // x is non-finite, y is non-finite
      if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)  // x is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
      
      if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)  // y is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal::mult - Invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");

      if (conservativeError) {
        if (xval == 2.0 || yval == 2.0) // x and/or y is NaN
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in NaN.");
        else if (xval * yval == 0.0)    // x and/or y is indeterminate.
           EXCEPTION_MNGR(std::range_error, "Ereal::mult - multiplication resulted in an indeterminate value.");
        else                            // (+/-)infinity * (+/-)infinity
          rval = xval * yval;
      }
      else {
        if (xval == 2.0 || yval == 2.0)
          rval = 2.0;
        else if (xval * yval == 0.0)
          rval = 0.0;
        else
          rval = xval * yval;
      }
    }
  }
}  


/// Division operation
template <class Type>
inline void Ereal<Type>::div(const Type xval, const bool xFinite, 
		const Type yval, const bool yFinite, 
		Type& rval, bool& rFinite)
{
  if (xFinite) {
    if (yFinite) {  // x finite, y finite
      if (yval == 0.0)
         EXCEPTION_MNGR(std::domain_error,"Ereal::div - division by zero");
      else {
        rFinite = true;
        rval = xval / yval;
      }
    }
    else {  // x finite, y non-finite
      if (yval == -1.0 || yval == 1.0) {
        rFinite = true;
        rval = 0.0;
      }
      else if (yval == 0.0 || yval == 2.0) {
        if (conservativeError) {
          if (yval == 0.0)
             EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in an indeterminate number.");
          else if (yval == 2.0)
             EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in NaN.");
        }
        else {
          rFinite = false;
          rval = yval;
        }
      }
      else // y is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error,"Ereal::div - invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
    }
  }
  else {
    if (yFinite) {  // x non-finite, y finite
      if (conservativeError) {
        rFinite = false;
        if (xval == 2.0)  // x is NaN
           EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in NaN.");
        else if (xval == 0.0) // x is indeterminate
           EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in an indeterminate number.");
        else if (xval != -1.0 && xval != 1.0) // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error,"Ereal::div - invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
        else if (yval > 0.0)  //+-infinity/+finite
          rval = xval;
        else if (yval < 0.0)  //+-infinity/-finite
          rval = -xval;
        else                  //+-infinity/0
           EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in an indeterminate number.");
      }
      else {
        rFinite = false;
        if (xval == 2.0 || xval == 0.0) // x is NaN or indeterminate
          rval = xval;
        else if (xval != -1.0 && xval!= 1.0)  // x is not a known, non-finite value
           EXCEPTION_MNGR(std::runtime_error,"Ereal::div - invalid internal state detected: val=" << xval << ", finite=" << xFinite << ".");
        else if (yval > 0.0)  // +-infinity/+finite
          rval = xval;
        else if (yval < 0.0)  // +-infinity/-finite
          rval = -xval;
        else                  // +-infinity/0
          rval = 0.0;
      }
    }
    else {  //x non-finite, y non-finite
      //throw an exception if either x or y is not a known, non-finite number
      if (xval != -1.0 && xval != 1.0 && xval != 0.0 && xval != 2.0)
         EXCEPTION_MNGR(std::runtime_error,"Ereal::div - invalid internal state detected: val=" << xval << ",finite=" << xFinite << ".");
      else if (yval != -1.0 && yval != 1.0 && yval != 0.0 && yval != 2.0)
         EXCEPTION_MNGR(std::runtime_error,"Ereal::div - invalid internal state detected: val=" << yval << ", finite=" << yFinite << ".");
      
      if (conservativeError) {
        if (xval == 2.0 || yval == 2.0) //x is NaN or y is NaN
           EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in NaN.");
        else
           EXCEPTION_MNGR(std::range_error,"Ereal::div - division resulted in an indeterminate number.");
      }
      else {
        rFinite = false;
        if (xval == 2.0 || yval == 2.0)
          rval = 2.0;
        else
          rval = 0.0;
      }
    }
  } 
}


/**
 * \brief Write the Ereal to an output stream.
 *
 * \param os The output stream to write to.
 *
 * \return A reference to the output stream.
 *
 * \see operator<<
 */
template <class Type> inline
void Ereal<Type>::write(std::ostream& os) const
{
if (Finite)
   os << val;
else {
   if (val == -1.0)
      os << "-Infinity";
   else if (val == 1.0)
      os << "Infinity";
   else if (val == 2.0)
      os << "NaN";
   else if (val == 0.0)
      os << "Indeterminate";
   else
      os << "Ereal_Bad_Value";		// Generate error
   }
}


/**
 * \brief Read an Ereal from an input stream.
 *
 * For a finite Ereal, the value to read must be in a valid format for \a
 * Type.  For a non-finite Ereal, the value to read must be one of the
 * following strings:
 *
 * \li \b negative \b infinity: -Infinity, -Inf, -infinity, -inf
 *
 * \li \b positive \b infinity: +Infinity, +Inf, +infinity, +inf, Infinity,
 * Inf, infinity, inf
 *
 * \li \b indeterminate: Indeterminate, Ind, indeterminate, ind
 *
 * \li \b NaN: NaN, nan
 *
 * \param is The input stream to read from.
 * 
 * \return A reference to the input stream
 *
 * \throw runtime_error if unable to read an ereal from the input stream.
 *
 * \see operator>>
 */
template <class Type> inline
void Ereal<Type>::read(std::istream& is)
{
  string s;
  Type t;

  if (is >> s) {
    std::istringstream iss(s);
    if (iss >> t) { // try to read a 'Type'
      val = t;
      Finite = true;
      check_if_infinite(val,Finite);
    }
    else {  //couldn't read 'Type', so we'll try for string values
      Finite = false;
      if (s == "-Infinity" || s == "-Inf" || s == "-infinity" || s == "-inf")             // -infinity
        val = -1.0;
      else if (s == "+Infinity" || s == "+Inf" || s == "+infinity" || s == "+inf"         //+infinity
                || s == "Infinity" || s == "Inf" || s == "infinity" || s == "inf")
        val = 1.0;
      else if (s == "Indeterminate" || s == "Ind" || s == "indeterminate" || s == "ind")  // indeterminate
        val = 0.0;
      else if (s == "NaN" || s == "nan")                                                  // NaN
        val = 2.0;
      else if (s == "Invalid" || s == "invalid" || s == "Inv" || s == "inv")              // invalid
        val = -2.0; //for testing only.  intentionally undocumented.
      else
         EXCEPTION_MNGR(std::runtime_error, "Ereal::read - Unrecognized value: " << s );
    }
  }
  else {
     EXCEPTION_MNGR(std::runtime_error,"Ereal::read - unable to read in value.");
  }

}


/**
 * \brief Pack an Ereal into a \a PackBuffer.
 * 
 * \param os The \a PackBuffer to pack the Ereal into.
 *
 * \return A reference to the \a PackBuffer.
 */
template <class Type> inline
void Ereal<Type>::write(PackBuffer& os) const
{ os << Finite << val; }


/**
 * \brief Unpack an Ereal from an \a UnPackBuffer.
 *
 * \param is The \a UnPackBuffer to read from.
 *
 * \return A reference to the \a UnPackBuffer.
 */
template <class Type> inline
void Ereal<Type>::read(UnPackBuffer& is)
{ is >> Finite >> val; }


/// Plus operator: Ereal + Ereal
template <class Type> inline Ereal<Type>
operator+ (const Ereal<Type>& x, const Ereal<Type>& y)
{
  Type val;
  bool Finite;
  Ereal<Type>::plus(x.val, x.Finite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Plus operator: Ereal + Type
template <class Type, class NType> inline Ereal<Type>
operator+ (const Ereal<Type>& x, const NType& y)
{
  Type yval = y;
  bool yFinite = true;
  Ereal<Type>::check_if_infinite(yval,yFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::plus(x.val, x.Finite, yval, yFinite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Plus operator: Type + Ereal
template <class Type, class NType> inline Ereal<Type>
operator+ (const NType& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::plus(xval, xFinite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Plus-equals operator: Type += Ereal
template <class Type> inline Type
operator+= (Type& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::plus(xval, xFinite, y.val, y.Finite, val, Finite);
  Ereal<Type> tmp(val,Finite);
  x = tmp;
  return x;
}


/// Minus operator: Ereal - Ereal
template <class Type> inline Ereal<Type>
operator- (const Ereal<Type>& x, const Ereal<Type>& y)
{
  Type val;
  bool Finite;
  Ereal<Type>::minus(x.val, x.Finite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Minus operator: Ereal - Type
template <class Type, class NType> inline Ereal<Type>
operator- (const Ereal<Type>& x, const NType& y)
{
  Type yval = y;
  bool yFinite = true;
  Ereal<Type>::check_if_infinite(yval,yFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::minus(x.val, x.Finite, yval, yFinite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Minus operator: Type - Ereal
template <class Type, class NType> inline Ereal<Type>
operator- (const NType& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::minus(xval, xFinite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Minus-equals operator: Type -= Ereal
template <class Type> inline Type
operator-= (Type& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::minus(xval, xFinite, y.val, y.Finite, val, Finite);
  Ereal<Type> tmp(val,Finite);
  x = tmp;
  return x;
}


/// Times operator: Ereal * Ereal
template <class Type> inline Ereal<Type>
operator* (const Ereal<Type>& x, const Ereal<Type>& y)
{
  Type val;
  bool Finite;
  Ereal<Type>::mult(x.val, x.Finite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Times operator: Ereal * Type
template <class Type, class NType> inline Ereal<Type>
operator* (const Ereal<Type>& x, const NType& y)
{
  Type yval = y;
  bool yFinite = true;
  Ereal<Type>::check_if_infinite(yval,yFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::mult(x.val, x.Finite, yval, yFinite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Times operator: Type * Ereal
template <class Type, class NType> inline Ereal<Type>
operator* (const NType& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::mult(xval, xFinite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Times-equals operator: Type *= Ereal
template <class Type> inline Type
operator*= (Type& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::mult(xval, xFinite, y.val, y.Finite, val, Finite);
  Ereal<Type> tmp(val,Finite);
  x = tmp;
  return x;
}


/// Division operator: Ereal / Ereal
template <class Type> Ereal<Type>
operator/ (const Ereal<Type>& x, const Ereal<Type>& y)
{
  Type val;
  bool Finite;
  Ereal<Type>::div(x.val, x.Finite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Division operator: Ereal / Type
template <class Type, class NType> Ereal<Type>
operator/ (const Ereal<Type>& x, const NType& y)
{
  Type yval = y;
  bool yFinite = true;
  Ereal<Type>::check_if_infinite(yval,yFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::div(x.val, x.Finite, yval, yFinite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Division operator: Type / Ereal
template <class Type, class NType> Ereal<Type>
operator/ (const NType& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::div(xval, xFinite, y.val, y.Finite, val, Finite);
  return Ereal<Type>(val,Finite);
}


/// Division-equals operator: Type /= Ereal
template <class Type> inline Type
operator/= (Type& x, const Ereal<Type>& y)
{
  Type xval = x;
  bool xFinite = true;
  Ereal<Type>::check_if_infinite(xval,xFinite);
  
  Type val;
  bool Finite;
  Ereal<Type>::div(xval, xFinite, y.val, y.Finite, val, Finite);
  Ereal<Type> tmp(val,Finite);
  x = tmp;
  return x;
}


/// Unary plus operator: + Ereal
template <class Type> inline Ereal<Type>
operator+ (const Ereal<Type>& x)
{
  if (x.Finite) {
    return x;
  }
  else {
    if (x.val == -1.0 || x.val == 1.0)
      return x;
    else if (x.val == 0.0)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator+ - Attempted operation on an indeterminate value.");
    else if (x.val == 2.0)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator+ - Attempted operation on NaN.");
    else
       EXCEPTION_MNGR(std::runtime_error, "Ereal::operator+ - Invalid internal state detected: val=" << x.val << ", finite=false");
  }
  
  return x; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Minus operator: - Ereal
template <class Type> inline 
Ereal<Type> operator- (const Ereal<Type>& num)
{
  if (num.Finite || num.val == -1.0 || num.val == 1.0)  // num is finite, -infinity, or +infinity
    return Ereal<Type>(-num.val, num.Finite);
  else if (num.val == 0.0)                              // num is indeterminate
     EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator- - Attempted operation on an indeterminate value.");
  else if (num.val == 2.0)                              // num is NaN
     EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator- - Attempted operation on NaN.");
  else                                                  // num is not a known, non-finite value
     EXCEPTION_MNGR(std::runtime_error, "Ereal::operator+ - Invalid internal state detected: val=" << num.val << ", finite=false.");
  
  return 0.0; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Equality operator: Ereal == Ereal
template <class LType, class RType> inline
bool operator== (const Ereal<LType>& x, const Ereal<RType>& y)
{
  if (x.Finite) {
    if (y.Finite) {
      return (x.val == y.val);
    }
    else{
      if (y.val == 0.0)
         EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - indeterminate value used in equality comparison");
      else if (y.val == 2.0)
         EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - NaN used in equality comparison.");
      else if (y.val != -1.0 && y.val != 1.0)
         EXCEPTION_MNGR(std::runtime_error, "Ereal::operator== - Invalid internal state detected: val =" << y.val << ", finite="<< y.Finite << ".");
      else
        return false;   
    }
  } 
  else {
    if (y.Finite) {
      if (x.val == -1.0 || x.val == 1.0) {
        return false;
      }
      else {
        if (x.val == 0.0)
           EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - indeterminate value used in equality comparison");
        else if (x.val == 2.0)
           EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - NaN used in equality comparison.");
        else
           EXCEPTION_MNGR(std::runtime_error, "Ereal::operator== - Invalid internal state detected: val =" << x.val << ", finite="<< x.Finite << ".");
      }
    }
    else {
      if ((x.val == -1.0 || x.val == 1.0 ) && (y.val == -1.0 || y.val == 1.0))
        return (x.val == y.val);
      else if (x.val == 0.0 || y.val == 0.0)
         EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - indeterminate value used in equality comparison");
      else if (x.val == 2.0 || y.val == 2.0)
         EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - NaN used in equality comparison.");
      else
         EXCEPTION_MNGR(std::runtime_error, "Ereal::operator== - Invalid internal state detected: xval =" << x.val << ", xFinite="<< x.Finite << ", yval=" << y.val << ", yFinite=" << y.Finite << ".");
    }
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Equality operator: Ereal == Type
template <class Type, class NType> inline
bool operator== (const Ereal<Type>& x, const NType& y)
{
  if (x.Finite) {
    return x.val == y;
  }
  else {
    if (x.val == -1.0 || x.val == 1.0)
      return false;
    else if (x.val == 0.0)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - indeterminate value used in equality comparison");
    else if (x.val == 2.0)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::operator== - NaN used in equality comparison.");
    else
       EXCEPTION_MNGR(std::runtime_error, "Ereal::operator== - Invalid internal state detected: val =" << x.val << ", finite=false");
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Equality operator: Type == Ereal
template <class Type, class NType> inline
bool operator== (const NType& x, const Ereal<Type>& y)
{ return (y == x); }


/// Not-equal operator: Ereal != Ereal
template <class Type> inline bool
operator!= (const Ereal<Type>& x, const Ereal<Type>& y)
{ return (!(y == x)); }


/// Not-equal operator: Ereal != Type
template <class Type, class NType> inline bool
operator!= (const Ereal<Type>& x, const NType& y)
{ return (!(y == x)); }


/// Not-equal operator: Type != Ereal
template <class Type, class NType> inline bool
operator!= (const NType& x, const Ereal<Type>& y)
{ return (y != x); }


/// Greater-Than operator: Ereal > Ereal
template <class Type> inline
bool operator> (const Ereal<Type>& x, const Ereal<Type>& y)
{ return (! (x <= y)); }


/// Greater-Than operator: Ereal > Type
template <class Type, class NType> inline
bool operator> (const Ereal<Type>& x, const NType& y)
{ return (! (x <= y)); }


/// Greater-Than operator: Type > Ereal
template <class Type, class NType> inline 
bool operator> (const NType& x, const Ereal<Type>& y)
{ return (!(x <= y)); }


/// Greater-Than-Or-Equal operator: Ereal >= Ereal
template <class Type> inline 
bool operator>= (const Ereal<Type>& x, const Ereal<Type>& y)
{ return (!(x < y)); }


/// Greater-Than-Or-Equal operator: Type >= Ereal
template <class Type, class NType> inline 
bool operator>= (const NType& x, const Ereal<Type>& y)
{ return (!(x < y)); }


/// Greater-Than-Or-Equal operator: Ereal >= Type
template <class Type, class NType> inline 
bool operator>= (const Ereal<Type>& x, const NType& y)
{ return (!(x < y)); }


/// Less-Than operator: Ereal < Ereal
template <class Type> inline
bool operator< (const Ereal<Type>& x, const Ereal<Type>& y)
{
  if (x.Finite) {
    if (y.Finite) {           // finite < finite
      return (x.val < y.val);
    }
    else {
      if ( y.val == -1.0)     // finite < -infinity
        return false;
      else if (y.val == 1.0)  // finite < +infinity
        return true;
      else if (y.val == 0.0)  // finite < indeterminate
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (y.val == 2.0)  // finite < NaN
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                    // finite < invalid
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << y.val << ", finite=false");
    }
  }
  else {
    if (y.Finite) {
      if (x.val == -1.0)      // -infinity < finite
        return true;
      else if (x.val == 1.0)  // +infinity < finite
        return false;
      else if (x.val == 0.0)  // indeterminate < finite
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (x.val == 2.0)  //  NaN < finite
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                    // invalid < finite
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << x.val << ", finite=false");
    }
    else {
      if ( (x.val == -1.0 || x.val == 1.0) && (y.val == -1.0 || y.val == 1.0) )
        return x.val < y.val;                 // x and y are (+-)infinity
      
      if (x.val == 0.0 || y.val == 0.0)       // x and/or y is indeterminate
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (x.val == 2.0 || y.val == 2.0)  // x and/or y is NaN
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                                    // x and/or y are not known, non-finite values
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: xval=" << x.val << ", xFinite=" << x.Finite << ", yval=" << y.val << ", yFinite=" << y.Finite);
    }
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}
       

/// Less-Than operator: Ereal < Type
template <class Type, class NType> inline
bool operator< (const Ereal<Type>& x, const NType& y)
{
  if (x.Finite) {
    return x.val < y;       // finite < finite
  }
  else {
    if (x.val == -1.0)      // -infinity < finite
      return true;
    else if (x.val == 1.0)  // +infinity < finite
      return false;
    else if (x.val == 0.0)  // indeterminate < finite
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
    else if (x.val == 2.0)  // NaN < finite
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
    else                    // x is not a known, non-finite value
       EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << x.val << ", finite=false");
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Less-Than operator: Type < Ereal
template <class Type, class NType> inline bool
operator< (const NType& x, const Ereal<Type>& y)
{
  if (y.Finite) {
    return x < y.val;
  }
  else {
    if (y.val == -1.0)      // finite < -infinity
      return false;
    else if (y.val == 1.0)  // finite < +infinity
      return true;
    else if (y.val == 0.0)  // finite < indeterminate
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
    else if (y.val == 2.0)  // finite < NaN
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
    else                    // y is not a known, non-finite type
       EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << y.val << ", finite=false");
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Less-Than-Or-Equal operator: Ereal <= Ereal
template <class Type> inline
bool operator<= (const Ereal<Type>& x, const Ereal<Type>& y)
{
  if (x.Finite) {
    if (y.Finite) {
      return x.val <= y.val;  // finite <= finite
    }
    else {
      if (y.val == -1.0)      // finite <= -infinity
        return false;
      else if (y.val == 1.0)  // finite <= +infinity
        return true;
      else if (y.val == 0.0)  // finite <= indeterminate
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (y.val == 2.0)  // finite <= NaN
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                    // y is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << y.val << ", finite=false");
    }
  }
  else {
    if (y.Finite) {
      if (x.val == -1.0)      // -infinity <= finite
        return true;
      else if (x.val == 1.0)  // +infinity <= finite
        return false;
      else if (x.val == 0.0)  // indeterminate <= finite
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (x.val == 2.0)  // NaN <= finite
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                    // x is not a known, non-finite value
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << x.val << ", finite=false");
    }
    else {
      if ( (x.val == -1.0 || x.val == 1.0) && (y.val == -1.0 || y.val == 1.0) )
        return x.val <= y.val;                // x and y are (+-)infinity
      
      if (x.val == 0.0 || y.val == 0.0)       // x and/or y is indeterminate
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
      else if (x.val == 2.0 || y.val == 2.0)  // x and/or y is NaN
         EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
      else                                    // x and/or y are not known, non-finite values
         EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: xval=" << x.val << ", xFinite=" << x.Finite << ", yval=" << y.val << ", yFinite=" << y.Finite);
    }
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}
      

/// Less-Than-Or-Equal operator: Ereal <= Type
template <class Type, class NType> inline
bool operator<= (const Ereal<Type>& x, const NType& y)
{
  if (x.Finite) {
    return x.val <= y;
  }
  else {
    if (x.val == -1.0)      // -infinity <= finite
      return true;
    else if (x.val == 1.0)  // +infinity <= finite
      return false;
    else if (x.val == 0.0)  // indeterminate <= finite
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
    else if (x.val == 2.0)  // NaN <= finite
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
    else                    // x is not a known, non-finte value
       EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << x.val << ", finite=false");
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}
  

/// Less-Than-Or-Equal operator: Type <= Ereal
template <class Type, class NType> inline bool
operator<= (const NType& x, const Ereal<Type>& y)
{
  if (y.Finite) {
    return x <= y.val;
  }
  else {
    if (y.val == -1.0)      // finite <= -infinity
      return false;
    else if (y.val == 1.0)  // finite <= +infinity
      return true;
    else if (y.val == 0.0)  // finite <= indeterminate
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using an indeterminate value.");
    else if (y.val == 2.0)  // finite <= NaN
       EXCEPTION_MNGR(std::invalid_argument, "Ereal - Attempted relational operation using NaN.");
    else                    // y is not a known, non-finte value
       EXCEPTION_MNGR(std::runtime_error, "Ereal - invalid internal state detected: val=" << y.val << ", finite=false");
  }
  
  return false; //this should never get reached, but gets rid of a lot of compiler warning messages.
}


/// Returns true if the Ereal object represents infinity
template <class Type> inline
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
bool Ereal_isinf(const Ereal<Type>& num)
#else
bool isinf(const Ereal<Type>& num)
#endif
{ return (!num.Finite && ((num.val == 1.0) || (num.val == -1.0))); }


/// Returns true if the Ereal object represents not-a-number
template <class Type> inline
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
bool Ereal_isnan(const Ereal<Type>& num)
#else
bool isnan(const Ereal<Type>& num)
#endif
{ return (!num.Finite && num.val == 2.0); }


/// Returns true if the Ereal object represents an indeterminate value
template <class Type> inline
bool isind(const Ereal<Type>& num)
{ return (!num.Finite && num.val == 0.0); }


/// Returns true if the Ereal object is a finite value
template <class Type> inline
bool finite(const Ereal<Type>& num)
{ return (num.Finite); }


/// Returns true if the Ereal object has an invalid internal state
template <class Type> inline
bool isinv(const Ereal<Type>& num)
{
  if (num.Finite)
    return false; // num is finite
  else if (num.val == -1.0 || num.val == 1.0 || num.val == 0.0 || num.val == 2.0)
    return false; // num is non-finite, but known
  else
    return true;  // num is not a known non-finte value
}


/// Numeric value used for positive infinity.
template<class Type>
const Type Ereal<Type>::positive_infinity_val = Type(1e307);

/// Numeric value used for negative infinity.
template<class Type>
const Type Ereal<Type>::negative_infinity_val = Type(-1e307);


// We provide specializations in Ereal.cpp for <double> and <long
// double>.  As a result, we need to declare to the compiler that those
// specializations are coming so that CPP files (other than Ereal) do
// not instantiate the default value above (leading to duplicate
// symbols)
#ifndef _MSC_VER
template<> const double Ereal<double>::positive_infinity_val;
template<> const double Ereal<double>::negative_infinity_val;
template<> const long double Ereal<long double>::positive_infinity_val;
template<> const long double Ereal<long double>::negative_infinity_val;
#else
// MSVS, unfortunately, treats the above declarations as definitions,
// which leads to duplicate definition errors when compiling Ereal.cpp.
// So, we need to explicitly declare the values here.
template<> const double Ereal<double>::positive_infinity_val = MAXDOUBLE;
template<> const double Ereal<double>::negative_infinity_val = -MAXDOUBLE;
template<> const long double Ereal<long double>::positive_infinity_val = MAXLONGDOUBLE;
template<> const long double Ereal<long double>::negative_infinity_val = -MAXLONGDOUBLE;
#endif

/// Controls whether or not conservative error handling is enabled.
template<class Type>
bool Ereal<Type>::conservativeError = false;


/// Rounds an Ereal to a long integer
template<class Type>
long int lround(Ereal<Type>& num)
{
  if ( finite(num) ) {
    if (num < LONG_MIN)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::lround: Attempt to round a value less than LONG_MIN");
    else if (num > LONG_MAX)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::lround: Attempt to round a value greater than LONG_MAX");
    else 
      return ::lround( (double)num );
  }
  else {
    if (num == Ereal<Type>::negative_infinity)
      return LONG_MIN;
    else if (num == Ereal<Type>::positive_infinity)
      return LONG_MAX;
    else if (num == Ereal<Type>::indeterminate)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::lround: Attempt to round an indeterminate value to an integer");
    else if (num == Ereal<Type>::NaN)
       EXCEPTION_MNGR(std::invalid_argument, "Ereal::lround: Attempt to round NaN to an integer.");
    else
       EXCEPTION_MNGR(std::runtime_error, "Ereal::lround: Invalid internal state detected:" << num);
  }
  
  return 0; //this should never get reached, but gets rid of a lot of compiler warning messages.
}

///
/// pscientific returns a portable scientific notation output, consistent
/// across platforms and bit sizes (within the machine precision).
/// stream options ios::uppercase and ios::showpos are supported
/// this does not support all stream options, and will have undetermined
/// results if flags such as left, right, hex, etc are set.
/// ios::setpoint and ios::width are not supported, but probably should be
///
template <class Type>
std::string pscientific(const Ereal<Type>& t, int mantissa=6, int expwidth=3,
        std::ostream* os = NULL)
{
std::string ans;

#if ( defined(UTILIB_AIX_CC) || (__PGI) )
if (utilib::Ereal_isinf(t)) 
#else
if (utilib::isinf(t))
#endif
   {
   if (t < 0.0)
      ans = "-Infinity";
   else
      ans = "Infinity";
   }
#if ( defined(UTILIB_AIX_CC) || (__PGI) )
else if (utilib::Ereal_isnan(t))
#else
else if (utilib::isnan(t))
#endif
   ans = "NaN";
else if (utilib::isind(t))
   ans = "Indeterminate";
else {
   double tmp = 0.0;
   try {
	   tmp = static_cast<double>(t);
   }
   catch (std::runtime_error& ) {
      ans = "Ereal_Bad_Value";
	  return ans;
   }
   catch (...) {
      ucout << "Caught unknown exception!" << std::endl;
   }
   return pscientific(tmp,mantissa,expwidth,os);
}

return ans;
}

} // namespace utilib


/// Returns fabs(val), where val is an extended real
template <class Type>
Type fabs(const utilib::Ereal<Type>& num)
{
return fabs(static_cast<Type>(num));
}

/// Returns exp(val), where val is an extended real
template <class Type>
Type exp(const utilib::Ereal<Type>& num)
{
return std::exp(static_cast<Type>(num));
}

/// Returns floor(val), where val is an extended real
template <class Type>
Type floor(const utilib::Ereal<Type>& num)
{
return floor(static_cast<Type>(num));
}

/// Returns ceil(val), where val is an extended real
template <class Type>
Type ceil(const utilib::Ereal<Type>& num)
{
return ceil(static_cast<Type>(num));
}

//
// Stream operators from stl_auxiliary.h
//

/// Stream operator for filling a vector with a given value
template <class TYPE>
std::vector<utilib::Ereal<TYPE> >& operator<< (std::vector<utilib::Ereal<TYPE> >& vec, const TYPE& val)
{
typename std::vector<utilib::Ereal<TYPE> >::iterator curr = vec.begin();
typename std::vector<utilib::Ereal<TYPE> >::iterator last = vec.end();
while (curr != last) {
  *curr = val;
  ++curr;
  }
return vec;
}


#include <utilib/Any.h>
namespace utilib {
DEFINE_FULL_ANY_EXTENSIONS(utilib::Ereal<double>);
} // namespace utilib


#ifdef CXXTEST_RUNNING
#include <cxxtest/ValueTraits.h>
namespace CxxTest {

/// Printing utility for use in CxxTest unit tests
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits< utilib::Ereal<double> >
{
public:
   ValueTraits(const utilib::Ereal<double>& t)
   { 
      std::ostringstream ss;
      ss << t;
      str = ss.str().c_str();
   }

   const char *asString() const
   { return str.c_str(); }

private:
   std::string str;
};

} // namespace CxxTest
#endif // CXXTEST_RUNNING


#endif // utilib_Ereal_h
