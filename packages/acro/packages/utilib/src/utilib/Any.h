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
 * \file Any.h
 *
 * Defines the utilib::Any class.  
 * 
 * This is a variant of the Boost any class (along with the deprecated UTILIB
 * AnyValue and AnyReference classes).  Unlike boost::any, utilib::Any
 * implements a Smart Pointer scheme to facilitate shallow copying of
 * the Any structure and reduce the number of times a contained value
 * must be copied to 0 - 2 (depending on the use of Any::extract() or
 * Any::expose(), and if the Any was initialized with a reference to the
 * original variable).
 */

#ifndef utilib_Any_h
#define utilib_Any_h

#include <utilib_config.h>
#include <utilib/exception_mngr.h>

// The following are required for PackBuf support (which I hope to
// deprecate some day)
#include <utilib/PackBuf.h>
#include <utilib/stl_auxiliary.h>
#include <cstddef>
#include <cassert>


namespace utilib {  // ::utilib

namespace error {   // ::utilib::error
namespace Any {     // ::utilib::error::Any
  const int Empty            = -100;
  const int IncompatibleType = -101;
} // namespace ::utilib::error::Any
} // namespace ::utilib::error

class Any;
class AnyRef;
class AnyFixedRef;


/// An exception class used by Any classes
class bad_any_cast : public std::bad_cast
{
public:
   /// Constructor
   bad_any_cast(const std::string& msg_) : msg(msg_) {}
   /// Destructor
   virtual ~bad_any_cast() throw() {}

   /// Return the exception description
   virtual const char * what() const throw()
      { return msg.c_str(); }

protected:
  /// The exception description
  const std::string msg;
};

/// An exception class used by Immutable Any classes
class bad_any_typeid : public std::bad_typeid
{
public:
   /// Constructor
   bad_any_typeid(const std::string& msg_) : msg(msg_) {}
   /// Destructor
   virtual ~bad_any_typeid() throw() {}
   
   /// Return the exception description
   virtual const char * what() const throw()
      { return msg.c_str(); }
   
protected:
   /// The exception description
   const std::string msg;
};

/** \brief An exception class thrown when attempting to compare anys
 *  that do not have a specialized Any::Comparator.
 */
class any_not_comparable : public std::logic_error
{
public:
   /// Constructor
   any_not_comparable(const std::string& msg_) : logic_error(msg_) {}
};

/** \brief An exception class thrown when attempting to copy anys
 *  that has a specialized Any::Copier that disallows copying
 */
class any_not_copyable : public std::logic_error
{
public:
   /// Constructor
   any_not_copyable(const std::string& msg_) : logic_error(msg_) {}
};

/** \brief An exception class thrown when attempting to write
 *  anys that do not have a specialized Any::Printer.
 */
class any_not_printable : public std::logic_error
{
public:
   /// Constructor
   any_not_printable(const std::string& msg_) : logic_error(msg_) {}
};

/** \brief An exception class thrown when attempting to read into an
 *  any that does not have a specialized Any::Reader
 */
class any_not_readable : public std::logic_error
{
public:
   /// Constructor
   any_not_readable(const std::string& msg_) : logic_error(msg_) {}
};


/** \brief An exception class thrown when attempting to read into an
 *  any that does not have a specialized Any::Packer
 */
class any_not_packable : public std::logic_error
{
public:
   /// Constructor
   any_not_packable(const std::string& msg_) : logic_error(msg_) {}
};


/** This class stores any object by value or reference.
 *
 *  This class was adapted from ideas in the boost::any class for
 *  UTILIB by John Siirola.
 *
 *  Extensions to the standard boost::any class:
 *    - stores either a type value or a reference to an external value
 *    - copying is a shallow copy operation (fast & memory efficient)
 *    - can be declared "immutable" (fixes both type and memory reference)
 *    - allows implicit coercion of data into Any objects
 *    - generic access to common class capabilities (read, write, compare)
 *
 *  The storage mechanism used by utilib::Any leverages a smart pointer
 *  to improve memory efficiency and reduce the number of times the
 *  contained data must be copied.  Passing Anys by value and by
 *  reference uses the same amount of memory.  Pass-by-value incurs
 *  only a slight additional overhead for reference counting.
 *
 *  Immutable Anys have their stored data type fixed and the location of
 *  the referred data (either external reference or contained object)
 *  fixed.  The data itself is not const -- this is similar to the
 *  difference between const int* and int* const.  Assignment and "set"
 *  operations must match the current data type (non-matching data types
 *  will throw a bad_any_typeid exception) and will perform DEEP copies
 *  into the Immutable Any's contained data.  Making a copy of the
 *  Immutable Any will perform a shallow copy (as usual), resulting in a
 *  second Immutable Any that refers to the same data.
 *
 *  <B>Type compatibility and Anys</B>: 
 *
 *  The Any class places few restrictions on data types stored
 *  within it.  Namely, the contained class must either: 
 *    -# be assignable and copy-constructible.  That is, it must either
 *       have an explicit operator=() method, OR contain no non-static
 *       const data.
 *    -# be default-constructible.
 *    .
 *  By default, utilib::Any requires assignability and
 *  copy-constructibility.  However, if the data type is declared as
 *  NonCopyable (see Any::Copier), then the contained data must be
 *  default-constructible.
 *
 *  In addition, utilib::Any optionally supports the following common
 *  methods for accessing the contained data:
 *    -# support for < and == comparisons.  This enables support of
 *       proper comparability (operator<() and operator==()) between Any
 *       objects and enables use of all STL containers (see
 *       Any::Comparator).
 *    -# support ostream::operator<<().  This enables support of
 *       operator<<() to print the contents of an Any object to an
 *       std::ostream (see Any::Printer).
 *    -# support ostream::operator>>().  This enables support of
 *       operator>>() to read data from a std::istream into the object
 *       contained within the Any object (see Any::Reader).
 *    -# support utilib::PackBuffer.  This enables the packing and
 *       unpacking of the contained data into a utilib::PackBuffer (see
 *       Any::Packer). \b Note: This capability is deprecated and
 *       targeted for removal before the release of Utilib 4.1.
 */ 
class Any
  {
  public:
    /// Helper class for copying the data within an Any object
    /** This class serves as a level of indirection for copying the data
     *  held by an Any object.  In general, we can assume that most data
     *  is copyable through the standard operator=() method.  However,
     *  in order to put non-copyable data into Any objects, we need a
     *  way to disable the ability to copy the data.  
     *
     *  By default, classes stored within Any objects ARE copyable.  To
     *  disable references to TYPE::operator=(), we supply a standard
     *  NonCopyable<> class.  The user need simply create the
     *  following specialization for their <TT>TYPE</TT>:
     *
     *  <pre>
     *  namespace utilib {
     *    template<>
     *    class Any::Copier<TYPE> : public Any::NonCopyable<TYPE> {};
     *  }
     *  </pre>
     *
     *  This mechanism also allows users to provide a specialized copy
     *  method that does not use operator=() by providing their own
     *  specialization of Any::Copier<TYPE>.
     *
     *  Alternatively, users may declare that an object is non-copyable
     *  when they instantiate the Any using the Any::set<TYPE, COPIER>()
     *  method.  This method allows the user to explicitly specify the
     *  Copier object that the Any should use:
     *
     *  <pre>
     *  Any tmp;
     *  tmp.set<Type, Any::NonCopyable<Type> >();
     *  </pre>
     */
    template<typename T>
    class Copier
      {
      public:
        static const bool CopyConstructable = true;
        static void copy(T& lhs, const T& rhs)
          { lhs = rhs; }
      };
 
    template<typename T>
    class NonCopyable
      {
      public:
        static const bool CopyConstructable = false;
        static void copy(T& lhs, const T& rhs)
          {
          static_cast<void>(lhs);
          static_cast<void>(rhs);
          EXCEPTION_MNGR(any_not_copyable, "An object of type '"
                         << demangledName(typeid(T)) << "' is within an Any "
                         "that is being copied, but this type has been "
                         "registered as being non-copyable.");
          }
      };

    /// Helper class for comparing the contents of two Any objects
    /** This class serves as a level of indirection for comparing the
     *  contents of two Any objects.  These two comparisons are useful
     *  so that we can put Any objects into STL associative containers.
     *  However, there are many cases where we would like to be able to
     *  pass around an object within an Any, but have no use for keying
     *  off the stored value (i.e. it will never be put into a
     *  std::set<> or as the first type in a std::map<>).  In that case,
     *  it is useful to be able to relax the requirement that the class
     *  support operator==() and operator<().  
     *
     *  By default, classes stored within Any objects are NOT
     *  comparable.  To enable references to TYPE::operator==() and
     *  TYPE::operator<(), we supply a standard DefaultComparable<>
     *  class.  The user need simply create the following specialization
     *  for their <TT>TYPE</TT>:
     *
     *  <pre>
     *  namespace utilib {
     *    template<>
     *    class Any::Comparator<TYPE> : public Any::DefaultComparable<TYPE> {};
     *  }
     *  </pre>
     *
     *  Additionally, this mechanism allows users to provide one or both
     *  comparators (isEqual() and isLessThan()) that do not use
     *  operator==() or operator<() by providing their own
     *  specialization of Any::Comparator<TYPE>.
     */
    template<typename T>
    class Comparator
      {
      public:
        static bool isEqual(const T& lhs, const T& rhs)
          {
          static_cast<void>(lhs);
          static_cast<void>(rhs);
          EXCEPTION_MNGR(any_not_comparable, "An object of type '"
                         << demangledName(typeid(T)) << "' is within an Any "
                         "that is being compared, but this type has not been "
                         "registered as being comparable.");
          return false;
          }
        static bool isLessThan(const T& lhs, const T& rhs)
          {
          static_cast<void>(lhs);
          static_cast<void>(rhs);
          EXCEPTION_MNGR(any_not_comparable, "An object of type '"
                         << demangledName(typeid(T)) << "' is within an Any "
                         "that is being compared, but this type has not been "
                         "registered as being comparable.");
          return false;
          }
      };
    template<typename T>
    class DefaultComparable
      {
      public:
        static bool isEqual(const T& lhs, const T& rhs)
          { return lhs == rhs; }
        static bool isLessThan(const T& lhs, const T& rhs)
          { return lhs < rhs; }
      };

    /// Helper class for writing the contents of an Any object
    /** This class serves as a level of indirection for writing an Any
     *  to a std::ostream.  While writing "Any" data can be very useful
     *  in application development, there are many cases where we would
     *  like to be able to pass around an object within an Any, but do
     *  not want to support the standard operator<<() method.  In that
     *  case, it is useful to be able to relax the requirement that the
     *  class support operator<<().
     *
     *  By default, classes stored within Any objects are NOT printable.
     *  To enable references to TYPE::operator<<(), we supply a standard
     *  DefaultPrinter<> class.  The user need simply create the
     *  following specialization for their <TT>TYPE</TT>:
     *
     *  <pre>
     *  namespace utilib {
     *    template<>
     *    class Any::Printer<TYPE> : public Any::DefaultPrinter<TYPE> {};
     *  }
     *  </pre>
     *
     *  Additionally, this mechanism allows users to provide a custom
     *  printing mechanism by providing their own specialization of
     *  Any::Printer<TYPE>.
     *
     *  <B>Note:</B> Because Any allows for implicit coercion, we DO
     *  NOT support the standard ostream::operator<<() directly through
     *  the Any class (see utilib::AnyAntiCoercionWrapper).  The
     *  Any::Printer class is accessed through Any::write().
     */
    template<typename T>
    class Printer
      {
      public:
        static std::ostream& print(std::ostream& os, const T& data)
          {
          static_cast<void>(data);
          os << "[utilib::Any contains non-printable object, \""
             << demangledName(typeid(T)) << "\"]";
          return os;
          }
      };
    template<typename T>
    class DefaultPrinter
      {
      public:
        static std::ostream& print(std::ostream& os, const T& data)
          { return (os << data); }
      };

    /// Helper class for reading data and storing within an Any object
    /** This class serves as a level of indirection for reading
     *  std::istream data into an initialized Any.  While reading from
     *  an std::istream into "Any" data can be very useful in
     *  application development, there are many cases where we would
     *  like to be able to pass around an object within an Any, but do
     *  not want to support the standard operator>>() method.  In that
     *  case, it is useful to be able to relax the requirement that the
     *  class support operator>>().
     *
     *  By default, classes stored within Any objects are NOT readable.
     *  To enable references to TYPE::operator>>(), we supply a standard
     *  DefaultReader<> class.  The user need simply create the
     *  following specialization for their <TT>TYPE</TT>:
     *
     *  <pre>
     *  namespace utilib {
     *    template<>
     *    class Any::Reader<TYPE> : public Any::DefaultReader<TYPE> {};
     *  }
     *  </pre>
     *
     *  Additionally, this mechanism allows users to provide a custom
     *  reading mechanism by providing their own specialization of
     *  Any::Reader<TYPE>.
     *
     *  <B>Note:</B> Because Any allows for implicit coercion, we DO
     *  NOT support the standard istream::operator>>() directly through
     *  the Any class (see utilib::AnyAntiCoercionWrapper).  The
     *  Any::Reader class is accessed through Any::read().
     */
    template<typename T>
    class Reader
      {
      public:
        static std::istream& read(std::istream& is, T& data)
          {
          static_cast<void>(data);
          EXCEPTION_MNGR(any_not_readable, "Type '" << demangledName(typeid(T))
                         << "' is not any-readable");
          return is;
          }
      };
    template<typename T>
    class DefaultReader
      {
      public:
        static std::istream& read(std::istream& is, T& data)
          { return (is >> data); }
      };

    /// Helper class for packing and unpacking within an Any object
    /** This class serves as a level of indirection for using the
     *  utilib::PackBuffer and utilib::UnPackBuffer classes with an Any object.
     *  While packing and unpacking data from "Any" data 
     *  can be very useful in application development, there are many 
     *  cases where we would like to be able to pass around an object 
     *  within an Any, but do not want to support this capability.
     *  In that case, it is useful to be able to relax the requirement 
     *  that the these methods are supported.
     *
     *  By default, classes stored within Any objects are NOT packable.
     *  To enable references to packing, we supply a standard
     *  DefaultPacker<> class.  The user need simply create the
     *  following specialization for their <TT>TYPE</TT>:
     *
     *  <pre>
     *  namespace utilib {
     *    template<>
     *    class Any::Packer<TYPE> : public Any::DefaultPacker<TYPE> {};
     *  }
     *  </pre>
     *
     *  Additionally, this mechanism allows users to provide a custom
     *  packer mechanism by providing their own specialization of
     *  Any::Packer<TYPE>.
     *
     *  <B>Note:</B> Because Any allows for implicit coercion, we DO
     *  NOT support the standard stream operators directly through
     *  the Any class (see utilib::AnyAntiCoercionWrapper).  The
     *  Any::Packer class is accessed through Any::write() and Any::read().
     */
    template<typename T>
    class Packer
      {
      public:
        static UnPackBuffer& read(UnPackBuffer& is, T& data)
          {
          static_cast<void>(data);
          EXCEPTION_MNGR(any_not_packable, "Type '" << demangledName(typeid(T))
                         << "' is not any-packable");
          return is;
          }
        static PackBuffer& write(PackBuffer& os, const T& data)
          {
          static_cast<void>(data);
          EXCEPTION_MNGR(any_not_packable, "Type '" << demangledName(typeid(T))
                         << "' is not any-packable");
          return os;
          }
      };
    template<typename T>
    class DefaultPacker
      {
      public:
        static UnPackBuffer& read(UnPackBuffer& is, T& data)
          { return ( is >> data ); }
        static PackBuffer& write(PackBuffer& os, const T& data)
          { return ( os << data ); }
      };

  public:
    /// Empty constructor
    Any()
       : m_data(NULL)
      {}

    /// Templated constructor that takes (and copies / references) a value
    /** Note, that unlike boost::any, the utilib::Any class does not
     *  force explicit construction.  This allows for implicit coercion
     *  (containment) in constructors and function arguments.  The
     *  latter is rather nice, as a function that takes an utilib::Any
     *  can actually be passed *any* variable as an argument.
     *
     *  If you call this constructor explicitly and provide asReference
     *  = true, then the Any class will refer directly to the value you
     *  provide (no copy will occur).  You will be responsible for
     *  making sure that the original value persists longer than this
     *  Any, AND any other Any that is copied from this one.
     */
    template<typename TYPE>
    Any( const TYPE & value, 
         const bool asReference = false, 
         const bool immutable = false )
      { 
      if ( asReference )
        { m_data = new ReferenceContainer<TYPE>(value); }
      else
        { m_data = new ValueContainer<TYPE>(value); }
      m_data->immutable = immutable;
      }

    /// Copy constructor (creates a "shallow copy" of the contained cargo data)
    Any(const Any & rhs)
       : m_data(rhs.m_data)
      {
      if ( m_data != NULL )
        { ++(m_data->refCount); }
      }

    /// Copy constructor to silently convert AnyRef back to generic Any
    Any(const AnyRef & rhs)
       : m_data(reinterpret_cast<const Any&>(rhs).m_data)
      {
      if ( m_data != NULL )
        { ++(m_data->refCount); }
      }

    /// Copy constructor to silently convert AnyFixedRef back to generic Any
    Any(const AnyFixedRef & rhs)
       : m_data(reinterpret_cast<const Any&>(rhs).m_data)
      {
      if ( m_data != NULL )
        { ++(m_data->refCount); }
      }

    /// Destructor
    virtual ~Any()
      { 
      if ( m_data != NULL )
        {
        if ( --(m_data->refCount) == 0 )
          { delete m_data; }
        }
      }

    /// Copy one Any object to another (USUALLY performs a shallow copy)
    Any& operator=(const Any & rhs)
      {
      // Check so assigning an any to itself doesn't dereference it
      if ( rhs.m_data == m_data )
        { return *this; }
  
      if ( m_data != NULL )
        {
        if ( m_data->immutable )
          {
          if (( rhs.m_data != NULL ) && ( is_type(rhs.m_data->type()) ))
            { 
            m_data->copy(rhs.m_data); 
            return *this;
            }
          else
            { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                              "Any::operator=(): assignment to immutable Any "
                              "from invalid type." ); }
          }
        else
          { 
          if ( --(m_data->refCount) == 0 )
            { delete m_data; }
          }
        }

      m_data = rhs.m_data;
      if ( m_data != NULL )
        { ++(m_data->refCount); }
      return *this;
      }

    /// Copy a derived AnyRef object into an Any (USUALLY a shallow copy)
    Any& operator=(const AnyRef & rhs)
      { return operator=(reinterpret_cast<const Any&>(rhs)); }
    /// Copy a derived AnyFixedRef object into an Any (USUALLY a shallow copy)
    Any& operator=(const AnyFixedRef & rhs)
      { return operator=(reinterpret_cast<const Any&>(rhs)); }


    /// Set the "cargo" data for this Any object
    /** This stores the provided value into the Any object.  If
     *  asReference == true, then only a reference to the provided value
     *  is stored.  You will be responsible for making sure that the
     *  original value persists longer than this Any AND any other
     *  future copy of this Any.
     *
     *  This returns a non-constant reference to the contained data.
     *  This is one of the few cases where we provide a non-const
     *  reference to the contained data.  The intent is that you can
     *  initialize the container data and obtain a reference to the data
     *  without the need to go through type-checking.  The reference can
     *  then be used to populate the data structure.  This avoids the
     *  case where you create the fully-populated data structure, then
     *  must copy the whole thing when you call Any::set() (i.e., this
     *  allows you to prevent a copy).
     */
    template<typename TYPE>
    TYPE& set( const TYPE & value, 
               bool asReference = false, 
               bool immutable = false )
      { return set<TYPE, Copier<TYPE> >(value, asReference, immutable); }

    template<typename TYPE, typename COPIER>
    TYPE& set( const TYPE & value, 
               bool asReference = false, 
               bool immutable = false )
      {
      if ( m_data != NULL )
        { 
        if ( m_data->immutable )
          {
          if ( immutable )
            { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                              "Any::set(value): assigning immutable to an "
                              "already immutable Any." ); }
          if ( asReference )
            { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                              "Any::set(value): assigning reference to an "
                              "immutable Any." ); }

          if ( is_type(typeid(TYPE)) )
            { return static_cast<TypedContainer<TYPE>*>
                  (m_data)->assign(value); }
          else
            { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                              "Any::set(value): assignment to immutable Any "
                              "from invalid type." ); }
          }

        if ( --(m_data->refCount) == 0 )
          { delete m_data; }
        }

      if ( asReference )
        { 
        m_data = new ReferenceContainer<TYPE, COPIER>(value);
        m_data->immutable = immutable;
        return const_cast<TYPE&>(value);
        }
      else
        { 
        m_data = new ValueContainer<TYPE, COPIER>(value); 
        m_data->immutable = immutable;
        return static_cast<ValueContainer<TYPE, COPIER>*>(m_data)->data;
        }
      }



    /** \brief Convenience function for creating a new object within an Any
     *  without calling the object's copy-constructor. 
     *
     *  This method can be used to avoid a call to the object Copy Constructor,
     *  and thus is a slightly more efficient form of:
     *  <DIR><TT>
     *     TYPE value;
     *     Any myAny(value);
     *  </TT></DIR>
     *
     *  As with Any::set<TYPE, COPIER>(const TYPE&, bool, bool), this
     *  returns a non-constant reference to the contained data.
     */
    template<typename TYPE>
    TYPE& set()
      { return set<TYPE, Copier<TYPE> >(); }

     /** \brief Convenience function for creating a new object within an
     *  Any without calling the object's copy-constructor and overriding
     *  the default copy operator.
     *
     *  This method can be used to avoid a call to the object Copy Constructor,
     *  and thus is a slightly more efficient form of:
     *  <DIR><TT>
     *     TYPE value;
     *     Any myAny(value);
     *  </TT></DIR>
     *
     *  As with Any::set<TYPE, COPIER>(const TYPE&, bool, bool), this
     *  returns a non-constant reference to the contained data.
     */
    template<typename TYPE, typename COPIER>
    TYPE& set()
      {
      if ( m_data != NULL )
        { 
        if ( m_data->immutable )
          { 
          if ( is_type(typeid(TYPE)) )
            {
            Any tmp;
            tmp.set<TYPE, COPIER>();
            m_data->copy(tmp.m_data);
            return static_cast<TypedContainer<TYPE>*>(m_data)->cast();
            }
          else
            { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                              "Any::set<>(): assignment to immutable Any "
                              "from invalid type." ); }
          }

        if ( --(m_data->refCount) == 0 )
          { delete m_data; }
        }

      m_data = new ValueContainer<TYPE, COPIER>();
      return static_cast<ValueContainer<TYPE, COPIER>*>(m_data)->data;
      }

    /** \brief Create a new immutable object within an Any without
     *  calling the object's copy-constructor.
     *
     *  This explicit method is required in order to avoid an ambiguous
     *  overload of Any::set<TYPE>(const TYPE&, bool, bool) when TYPE ==
     *  bool.
     *
     *  As with Any::set<TYPE>(), this returns a non-constant reference
     *  to the contained data.
     */
    template<typename TYPE>
    TYPE& set_immutable()
      { return set_immutable<TYPE, Copier<TYPE> >(); }

    /** \brief Create a new immutable object within an Any without
     *  calling the object's copy-constructor.
     *
     *  This explicit method is required in order to avoid an ambiguous
     *  overload of Any::set<TYPE, COPIER>(const TYPE&, bool, bool) when
     *  TYPE == bool.
     *
     *  As with Any::set<TYPE, COPIER>(), this returns a non-constant
     *  reference to the contained data.
     */
    template<typename TYPE, typename COPIER>
    TYPE& set_immutable()
      {
      if (( m_data != NULL ) && ( m_data->immutable ))
        { EXCEPTION_MNGR( utilib::bad_any_typeid, 
                          "Any::set(): assigning immutable to an "
                          "already immutable Any." ); }
      TYPE& ans = set<TYPE, COPIER>();
      m_data->immutable = true;
      return ans;
      }

    /// Shortcut using assignment in lieu of Any::set()
    /** I'm not sure if I like supporting this syntax.  It has a
     *  different behavior when called with an Any RHS (creates a 
     *  duplicate reference to the RHS Any) than for all other types 
     *  (causes this Any to "contain" the RHS as the data cargo).
     *
     *  It is included solely for compatibility / consistency with the
     *  boost::any class -- I would prefer only supporting Any::set()
     */
    template<typename TYPE>
    inline Any& operator=(const TYPE & value)
      { 
      set(value); 
      return *this;
      }


    /// Shortcut for reference assignment in lieu of Any::set(value, true)
    /** This is an extension to the standard assignment operator for
     *  assigning a reference to the RHS value into the Any (instead of
     *  making a deep copy of the RHS value).
     *
     *  I'm not sure if I like supporting this syntax.  It is a
     *  non-standard overload of a standard assignment operator.  
     */
    template<typename TYPE>
    inline Any& operator&=(const TYPE & value)
      { 
      set(value, true); 
      return *this;
      }


    /// Extract the "cargo" data from this Any object (return <0 on error)
    /** This will extract the stored value out of the Any object into
     *  the value reference, provided that the two types match EXACTLY.
     *
     *  See Any::is_type() for details on type resolution.
     *
     *  Returns 0 on success, <0 on error (value left unchanged).  If
     *  Any::throwCastExceptions() has been set to true, error conditions
     *  will also throw a bad_any_cast exception.
     */
    template<typename TYPE>
    int extract(TYPE & value) const
      { 
      if ( m_data == NULL )
        { 
        if ( throwCastExceptions() )
          { EXCEPTION_MNGR(utilib::bad_any_cast, 
                           "Any::extract() - NULL data"); }
        return error::Any::Empty; 
        }

      if ( is_type(typeid(TYPE)) )
        {
        static_cast<TypedContainer<TYPE>*>(m_data)->copyTo(value);
        return 0;
        }

      if ( throwCastExceptions() )
        { EXCEPTION_MNGR( utilib::bad_any_cast, 
                          "Any::extract() - failed conversion from '" 
                          << demangledName(m_data->type()) << "' to '" 
                          << demangledName(typeid(TYPE)) << "'" ); }

      return error::Any::IncompatibleType; 
      }


    /// Exposes a const reference to the "cargo" data from this Any object
    /** This will expose a reference to the stored value in the Any object,
     *  provided that the two types match EXACTLY.  If they do not
     *  match, it will throw an exception (bad_any_cast).
     *
     *  See Any::is_type() for details on type resolution.
     *
     *  Note: you must explicitly specify the destination type when you
     *  call this method (e.g. <tt>Any foo(5); std::cerr <<
     *  foo.expose<int>();</tt>).
     *
     *  WARNING: Abusing this function may be hazardous to your health.
     *  Any is a reference class (a la C#), so if you store a new value
     *  or edit the value exposed by expose(), that change will be
     *  apparent to ANYONE who still maintains a reference to this Any
     *  object.  I try and save you by storing the data as 'const', but
     *  if you attempt any funny-business with const_cast, well,
     *  consider yourself warned.
     *
     *  See Any::extract() for details on type resolution.
     */
    template<typename TYPE>
    const TYPE& expose() const
      {
      if ( m_data == NULL )
        { EXCEPTION_MNGR(utilib::bad_any_cast, "Any::expose() - NULL data"); }

      if ( ! is_type(typeid(TYPE)) )
        {
        EXCEPTION_MNGR( utilib::bad_any_cast, 
                        "Any::expose() - failed conversion from '" 
                        << demangledName(m_data->type()) << "' to '" 
                        << demangledName(typeid(TYPE)) << "'" );
        }

      return static_cast<TypedContainer<TYPE>*>(m_data)->cast();
      }

    /// Exposes a non-const reference to the "cargo" data from this Any object
    /** I waffle back and forth about supporting this method (and the
     *  associated anyref_cast(Any & operand) below.  While it may be
     *  convenient to be able to directly access a non-const reference
     *  to the contained data, this has proved over and over to be a
     *  dangerous way of introducing bugs into otherwise rational code.
     *  This all stems from two fundamental problems:
     *
     *    1) C++ does not really support reference-counted objects.  As
     *    a result, it cannot readily distinguish between const objects
     *    and non-const references to a const object.
     *
     *    2) The templated assignement operator allows users to assign a
     *    const Any to a non-const Any.  After this, the non-const Any
     *    refers to the SAME data as the original const Any.  This
     *    effectively (and silently) violates const-correctness.
     *
     * WEH - I am exposing this.  The first item is not important.  I _want_
     *    to stick data into an Any that I later reference.  The second issue
     *    _is_ important, but we shouldn't limit the API of Any's to const's
     *    while waiting for explicit const management.
     */
    template<typename TYPE>
    TYPE& expose()
      {return const_cast<TYPE&>(const_cast<const Any*>(this)->expose<TYPE>());}

    /// Verifies the type of the stored value.
    /** This will return true if the stored type (EXACTLY) matches the
     *  calling type (i.e. extract / expose would succeed).  
     *
     *  This compares the mangled type name in addition to the type_info
     *  reference because each time a template is emitted into a
     *  compilation unit, it gets its own type_info structure.  If
     *  libraries are dynamically linked, this can cause the same type
     *  to return different type_info structures depending on which
     *  library instantiated it (so a direct reference comparison would
     *  fail).  The mangled type name, however, will be the same.  Note
     *  that this is only a problem with dynamic-link libraries.  Most
     *  static linkers will remove the redundant template emission at
     *  link time.
     */
    bool is_type(const std::type_info &targetType) const
      {
      // 1) the quick test: do the type_info references match?
      const std::type_info &myType = type();
      if ( myType == targetType )
        { return true; }
      
      // 2) the not-so-quick test: do the mangled names match?
      return( 0 == std::strcmp(mangledName(myType), mangledName(targetType)) );
      }

   ///
   template <typename TYPE>
   bool is_type() const
        { return is_type( typeid(TYPE) ); }

    /// Clear the "cargo" data for this Any object (empty() will become true)
    void clear()
      {
      if ( m_data != NULL )
        { 
        if ( --(m_data->refCount) == 0 )
          { delete m_data; }
        m_data = NULL;
        }
      }

    /// Returns true if the rhs Any refers to the same Container
    bool references_same_data_as(const Any& rhs) const
      { return (( m_data != NULL ) && ( m_data == rhs.m_data )); }

    /// Returns true if the data stored in the rhs Any is equal to this data 
    bool operator==(const Any& rhs) const
      {
      if ( m_data == rhs.m_data )
         return true;
      if ( m_data == NULL )
        { return rhs.m_data == NULL; }
      if ( ! rhs.is_type(m_data->type()) )
        { return false; }
      return m_data->isEqual(rhs.m_data);
      }
    
    /// Returns true if the data stored in this Any is less than the rhs data 
    bool operator<(const Any& rhs) const
      {
      if ( m_data == rhs.m_data )
         return false;
      if ( m_data == NULL )
        { return rhs.m_data != NULL; }
      if ( rhs.m_data == NULL )
        { return false; }
      if ( ! rhs.is_type(m_data->type()) )
        {
#ifdef _MSC_VER
        // NB: This silences a warning in MSVC because Microsoft (in
        // their infinite wisdom) returns int and not the standard bool.
        return m_data->type().before(rhs.m_data->type()) != 0;
#else
        return m_data->type().before(rhs.m_data->type());
#endif
        }
      else
        { return m_data->isLessThan(rhs.m_data); }
      }

    /// Returns true if the object value has not been set
    inline bool empty() const
      { return m_data == NULL; }

    /// Returns if the stored type is actually a reference
    inline bool is_reference() const
      { return m_data ? m_data->isReference() : false; }

    /// Returns if this Any is immutable (data is fixed in type and address)
    /** An "immutable" Any is an Any where the data type and the memory
     *  address (either reference or value location) is fixed.
     *
     *  Any.set() on an immutable Any will reset the value of the stored
     *  data if the data types match and throw an exception if they do
     *  not.  
     *
     *  Assigning an immutable Any to another Any will perform a shallow
     *  copy as usual (the new Any will also be immutable).  However,
     *  assigning another Any to the immutable Any will perform a DEEP
     *  copy of the contained data if the data types match and throw an
     *  exception if they do not.
     */
    inline bool is_immutable() const
      { return m_data ? m_data->immutable : false; }

    /// Returns the number of other Any instances that refer to this data
    inline unsigned int anyCount() const
      { return m_data ? (m_data->refCount - 1) : 0; }

    /// Returns the type information of the object contained in this Any object
    inline const std::type_info & type() const
      { return m_data ? m_data->type() : typeid(void); }

    /// Return a deep copy of this Any (as a new value Any)
    Any clone() const
      {
      if ( m_data == NULL )
         return Any();

      Any tmp; 
      tmp.m_data = m_data->newValueContainer(); 
      return tmp;
      }

    /// Remove reference/immutable flags (performs a copy if necessary)
    Any& to_value_any();

    /// Indicates whether extract() should generate exceptions for bad casts
    static bool& throwCastExceptions();

    /// Describe the contents of this Any
    std::string describe(std::string name = "(Any)") const;

    /// Attempt to print the contents of this Any to an ostream
    void write(std::ostream& os) const;

    /// Attempt to read from an istream and store the data into this Any
    void read(std::istream& is);

    /// Attempt to write the contents of this Any to a PackBuffer
    void write(PackBuffer& os) const;

    /// Attempt to read from a UnPackBuffer and store the data into this Any
    void read(UnPackBuffer& is);

  private:
    struct ContainerBase
      {
        ContainerBase()
          : refCount(1),
            immutable(false)
          {}
        virtual ~ContainerBase()
          {}

        /// Return the type of the stored object
        virtual const std::type_info& type() const = 0;
        /// Return whether the derived container holds a reference
        virtual bool isReference() const = 0;
        /// Return whether the rhs container data is equal to this data
        virtual bool isEqual(const ContainerBase* rhs) const = 0;
        /// Return whether this data is less than the rhs container data
        virtual bool isLessThan(const ContainerBase* rhs) const = 0;
        /// Print out the contents of the contained data
        virtual std::ostream& print(std::ostream& os) const = 0;
        /// Copy the data from another (hopefully compatible) ContainerBase*
        virtual std::istream& read(std::istream& is) = 0;
        /// Write the contained data into a PackBuffer 
        virtual PackBuffer& write(PackBuffer& is) const = 0;
        /// Read from the UnPackBuffer into this contained data
        virtual UnPackBuffer& read(UnPackBuffer& is) = 0;
        /// Copy the data from another (hopefully compatible) ContainerBase*
        virtual void copy(ContainerBase *rhs) = 0;
        /// Create a new ReferenceContainer to this container's data 
        virtual ContainerBase* newReferenceContainer() const = 0;
        /// Create a new Container containing this container's data 
        virtual ContainerBase* newValueContainer() const = 0;

        unsigned int   refCount;
        bool           immutable;
      };

    template<typename TYPE>
    struct TypedContainer : public ContainerBase
      {
      public:
        virtual ~TypedContainer()
          {}

        /// Return a reference to the contained data
        virtual TYPE& cast() const = 0;
        /// Return a reference to the contained data
        virtual TYPE& assign(const TYPE& rhs) = 0;
        /// Return a reference to the contained data
        virtual void copyTo(TYPE& rhs) const = 0;

        /// Return the type of the stored object
        virtual const std::type_info& type() const 
          { return typeid(TYPE); }
        /// Return whether the rhs container data is equal to this data
        virtual bool isEqual(const ContainerBase* rhs) const
          { 
          return Comparator<TYPE>::isEqual
             (cast(), static_cast<const TypedContainer<TYPE>*>(rhs)->cast());
          }
        /// Return whether this data is less than the rhs container data
        virtual bool isLessThan(const ContainerBase* rhs) const
          { 
          return Comparator<TYPE>::isLessThan
             (cast(), static_cast<const TypedContainer<TYPE>*>(rhs)->cast());
          }
        /// Print out the contents of the contained data
        virtual std::ostream& print(std::ostream& os) const
          { return Printer<TYPE>::print(os, cast()); }
        /// Read from the istream into this contained data
        virtual std::istream& read(std::istream& is) 
          { return Reader<TYPE>::read(is, cast()); }
        /// Write the contained data into a PackBuffer 
        virtual PackBuffer& write(PackBuffer& os) const
          { return Packer<TYPE>::write(os, cast()); }
        /// Read from the UnPackBuffer into this contained data
        virtual UnPackBuffer& read(UnPackBuffer& is) 
          { return Packer<TYPE>::read(is, cast()); }
      };

    /// A container holding a reference to the Any's data (stored externally)
    template<typename TYPE, typename COPIER = Copier<TYPE> >
    struct ReferenceContainer : public TypedContainer<TYPE>
      {
      public:
        ReferenceContainer(const TYPE & value)
          : TypedContainer<TYPE>(),
            data(const_cast<TYPE&>(value))
          {}
        virtual ~ReferenceContainer()
          {}

        /// Return whether the derived container holds a reference
        virtual bool isReference() const 
          { return true; }
        /// Create a new ReferenceContainer to this container's data 
        virtual ContainerBase* newReferenceContainer() const
          { return new ReferenceContainer<TYPE, COPIER>(data); }
        /// Create a new Container containing this container's data 
        virtual ContainerBase* newValueContainer() const;

        /// Copy the data from another (hopefully compatible) ContainerBase*
        virtual void copy(ContainerBase *rhs)
          { COPIER::copy(data, 
                         static_cast<TypedContainer<TYPE>*>(rhs)->cast()); }
        /// Copy the provided value into this object
        virtual TYPE& assign(const TYPE& rhs)
          { 
          COPIER::copy(data, rhs); 
          return data;
          }
        /// Copy this object's value into the provided reference
        virtual void copyTo(TYPE& lhs) const
          { COPIER::copy(lhs, data); }

        /// Return a reference to the contained data
        TYPE& cast() const
          { return data; }
 
        TYPE & data;
      };

    /** \brief Intermediate container to switch between requiring a
     *  copy constructor (default) or a default constructor.
     *
     *  In order for the utilib::Any to store a class within a
     *  ValueContainer, the class must either support copy construction
     *  (the default) or have a default constructor (required for
     *  NonCopyable classes).  This template provides the switching
     *  functionality governing how the ValueContainer data gets
     *  constructed in each of these scenarios.
     */
    template<typename TYPE, typename COPIER, bool CopyConstructable> 
    struct ValueContainer_Impl : public TypedContainer<TYPE>
      {
        /// default constructor only used by Any::set<TYPE>()
        ValueContainer_Impl()
          : TypedContainer<TYPE>(),
            data()
          {}

        /// Standard constructor used 99\% of the time
        ValueContainer_Impl(const TYPE & value)
          : TypedContainer<TYPE>(),
            data(value) 
          {}

        virtual ~ValueContainer_Impl()
          {}

        TYPE data;
      };

    /// A container for holding an internal copy of the Any's data
    template<typename TYPE, typename COPIER = Copier<TYPE> >
    struct ValueContainer 
       : public ValueContainer_Impl<TYPE, COPIER, COPIER::CopyConstructable>
      {
      public:
        /// default constructor only used by Any::set<TYPE>()
        ValueContainer()
          : ValueContainer_Impl<TYPE, COPIER, COPIER::CopyConstructable>()
        {}

        /// Standard constructor used 99\% of the time
        ValueContainer(const TYPE & value)
          : ValueContainer_Impl<TYPE, COPIER, COPIER::CopyConstructable>(value)
        {}

        virtual ~ValueContainer()
          {}

        /// Return whether the derived container holds a reference
        virtual bool isReference() const 
          { return false; }
        /// Create a new ReferenceContainer to this container's data 
        virtual ContainerBase* newReferenceContainer() const
          { return new ReferenceContainer<TYPE, COPIER>(this->data); }
        /// Create a new Container containing this container's data 
        virtual ContainerBase* newValueContainer() const
          { return new ValueContainer<TYPE, COPIER>(this->data); }

        /// Copy the data from another (hopefully compatible) ContainerBase*
        virtual void copy(ContainerBase *rhs)
          { COPIER::copy(this->data, 
                         static_cast<TypedContainer<TYPE>*>(rhs)->cast()); }
        /// Copy the provided value into this object
        virtual TYPE& assign(const TYPE& rhs)
          { 
          COPIER::copy(this->data, rhs); 
          return this->data;
          }
        /// Copy this object's calue into the provided reference
        virtual void copyTo(TYPE& lhs) const
          { COPIER::copy(lhs, this->data); }

        /// Return a reference to the contained data
        TYPE& cast() const
          { return const_cast<TYPE&>(this->data); }
      };

  protected:
    ContainerBase *m_data;
  };


/// Override definition that avoids reference to copy constructor
template<typename TYPE, typename COPIER> 
struct Any::ValueContainer_Impl<TYPE, COPIER, false>
   : public Any::TypedContainer<TYPE>
{
   /// default constructor only used by Any::set<TYPE>()
   ValueContainer_Impl()
      : TypedContainer<TYPE>(),
        data()
   {}

   /// Standard constructor used 99\% of the time
   ValueContainer_Impl(const TYPE & value)
      : TypedContainer<TYPE>()
   { COPIER::copy(data, value); }

   virtual ~ValueContainer_Impl()
   {}

   TYPE data;
};


template<typename TYPE, typename COPIER>
Any::ContainerBase* 
Any::ReferenceContainer<TYPE, COPIER>::newValueContainer() const
  { return new Any::ValueContainer<TYPE, COPIER>(data); }


/// Cast an Any reference into its value
template <typename TYPE>
inline TYPE 
anyval_cast(const Any & operand)
  { return operand.template expose<TYPE>(); }


/// Cast a const Any and expose a const reference to its value
template <typename TYPE>
inline const TYPE & 
anyref_cast(const Any & operand)
  { return operand.template expose<TYPE>(); }

#if 1
// JDS: NB: I am going to disable this "SOON" (after colin rework is
//      merged back in)
// WEH: we must keep this, since the const semantics for any data is
//      inconsistent with my use of Any reference data.
// [JDS: Moving this discussion onto the Utilib section of the Acro Wiki]
/** \brief Cast an Any and expose a non-const reference to its value
 *  (WARNING: may cause unintended side effects!)
 *
 *  See my notes for the non-const version of Any::expose().  They apply
 *  to this function as well.
 */
template <typename TYPE>
inline TYPE & 
anyref_cast(Any & operand)
  { return const_cast<TYPE&>(operand.template expose<TYPE>()); }
#endif

/// A shadow class to force implicit coercion of reference (not value) Anys
/** This is a "shadow class" (no new members, no new methods) that
 *  causes the compiler to use a different rule for implicitly coercing
 *  variables into Anys.  

 *  Normally, implicitly coercing a non-Any into an Any causes the source
 *  varialbe to be <i>copied</i> into the new Any (that is, it returns a
 *  "value" Any that is independent of the original data).  Implicit
 *  coercion to AnyRef variables will create a reference Any that refers
 *  directly back to the source variable.
 *
 *  This class is designed primarily to be used in function definitions
 *  to control the nature of the Anys that are passed into the function.
 *  AnyRef can be implicitly converted back to a general Any.
 */
class AnyRef : public Any
  {
  public:
    /// Empty constructor
    AnyRef()
      : Any()
      {}

    /// Standard copy constructor
    AnyRef(const AnyRef & rhs)
      : Any(reinterpret_cast<const Any&>(rhs))
      {}

    /// Templated constructor that takes (and references) a value
    /** Note, that unlike boost::any, the utilib::Any class does not
     *  force explicit construction.  This allows for implicit type
     *  coercion (containment) in constructors and function arguments.
     *  The latter is rather nice, as a function that takes an
     *  utilib::Any can actually be passed *any* variable as an
     *  argument.
     */
    template<typename TYPE>
    AnyRef( const TYPE & value, 
            const bool immutable = false )
      : Any(value, true, immutable)
      {}

    /// Allow implicit coercion of Anys to AnyRefs
    AnyRef(const Any & rhs)
      : Any(rhs)
      {}

    /* NB: we *MUST* use reinterpret_cast here as using static_cast will
     * cause a NEW Any to be constructed using Any's templated implicit
     * coercion constructor (because AnyFixedRef has not yet been defined as
     * derived from Any).
     */
    /// Allow implicit coercion of AnyFixedRefs to AnyRefs
    AnyRef(const AnyFixedRef & rhs)
      : Any(reinterpret_cast<const Any&>(rhs))
      {}

    /// Relay all assignment operations to underlying Any for processing
    /** Explicitly cause all assignment operations to use base Any
     *  functions.  While not a big deal right now, putting this in now
     *  can save us from a difficult-to-locate bug later on (see
     *  description for AnyFixedRef::operator=()).
     */
    template<typename T>
    AnyRef& operator=(const T & rhs)
      { Any::set(rhs, true); return *this; }

    /// Copy an Any into this AnyRef (USUALLY a shallow copy)
    AnyRef& operator=(const Any & rhs)
      { Any::operator=(reinterpret_cast<const Any&>(rhs)); return *this; }
    /// Copy a derived AnyRef into this AnyRef (USUALLY a shallow copy)
    AnyRef& operator=(const AnyRef & rhs)
      { Any::operator=(reinterpret_cast<const Any&>(rhs)); return *this; }
    /// Copy a derived AnyFixedRef into this AnyRef (USUALLY a shallow copy)
    AnyRef& operator=(const AnyFixedRef & rhs)
      { Any::operator=(reinterpret_cast<const Any&>(rhs)); return *this; }

  };


/** \brief A shadow class to force implicit coercion to immutable reference
 *  (not value) Anys.
 *
 *  This is a "shadow class" (no new members, no new methods) that
 *  causes the compiler to use a different rule for implicitly coercing
 *  variables into Anys.

 *  Normally, implicitly coercing a non-Any into an Any causes the source
 *  varialbe to be <i>copied</i> into the new Any (that is, it returns a
 *  "value" Any that is independent of the original data).  Implicit
 *  coercion to AnyFixedRef variables will create an immutable reference
 *  Any that refers directly back to the source variable.
 *
 *  This class is designed primarily to be used in function definitions
 *  to control the nature of the Anys that are passed into the function.
 *  AnyFixedRef can be implicitly converted back to a general Any.
 */
class AnyFixedRef : public Any
  {
  public:
    /// Empty constructor
    AnyFixedRef()
      : Any()
      {}

    /// Standard copy constructor
    AnyFixedRef(const AnyFixedRef & rhs)
      : Any(reinterpret_cast<const Any&>(rhs))
      {}

    /// Templated constructor that takes (and references) a value
    /** Note, that unlike boost::any, the utilib::Any class does not
     *  force explicit construction.  This allows for implicit type
     *  coercion (containment) in constructors and function arguments.
     *  The latter is rather nice, as a function that takes an
     *  utilib::Any can actually be passed *any* variable as an
     *  argument.
     */
    template<typename TYPE>
    AnyFixedRef( const TYPE & value )
      : Any(value, true, true)
      {}

    /// Allow implicit coercion of Anys to AnyFixedRefs
    AnyFixedRef(const Any & rhs)
      : Any(rhs)
      {
      if (( m_data == NULL ) || m_data->immutable )
        { return; }

      // As we are copying the data from another Any, this will not
      // result in the refCount going to 0
      --(m_data->refCount);
      
      m_data = m_data->newReferenceContainer();
      m_data->immutable = true;
      }

    /// Allow implicit coercion of AnyRefs to AnyFixedRefs
    AnyFixedRef(const AnyRef & rhs)
      : Any(reinterpret_cast<const Any&>(rhs))
      {
      if (( m_data == NULL ) || m_data->immutable )
        { return; }

      // As we are copying the data from another Any, this will not
      // result in the refCount going to 0
      --(m_data->refCount);
      
      m_data = m_data->newReferenceContainer();
      m_data->immutable = true;
      }

    /// Relay all assignment operations to underlying Any for processing
    /** Explicitly cause all assignment operations to use base Any
     *  functions.  Without this explicit function, the compiler may
     *  assign an Any to an AnyFixedRef variable by first
     *  copy-constructing a new AnyFixedRef and then doing an implicit
     *  assignment (this causes the AnyFixedRef data to REFERENCE the
     *  data in the source Any, and not to update the refCount in the
     *  RHS Any.
     */
    template<typename T>
    AnyFixedRef& operator=(const T & rhs)
      { 
      if ( empty() )
         { Any::set(rhs, true, true); }
      else
         { Any::operator=(rhs); }
      return *this; 
      }
     
    /// Copy an Any into this AnyFixedRef (USUALLY a deep copy)
    AnyFixedRef& operator=(const Any & rhs)
      {
      if ( empty() )
        { operator=(AnyFixedRef(rhs)); }
      else
        { Any::operator=(reinterpret_cast<const Any&>(rhs)); }
      return *this; 
      }
    /// Copy a derived AnyRef into this AnyFixedRef (USUALLY a deep copy)
    AnyFixedRef& operator=(const AnyRef & rhs)
      {
      if ( empty() )
        { operator=(AnyFixedRef(rhs)); }
      else
        { Any::operator=(reinterpret_cast<const Any&>(rhs)); }
      return *this; 
      }
    /// Copy a derived AnyFixedRef into this AnyFixedRef (USUALLY a deep copy)
    AnyFixedRef& operator=(const AnyFixedRef & rhs)
      {
      Any::operator=(reinterpret_cast<const Any&>(rhs));
      return *this; 
      }
  };


//---------------------------------------------------------------------

/// A simple wrapper class to prevent automatic coersion into an Any
/** This class is designed to allow us to define functions that ONLY
 *  take an Any object.  That is, it blocks the standard implicit
 *  coercion of all data types into an Any.  This is particularly
 *  important for defining Any-specific overloads for system-level
 *  functions (like the ostream-based operator<<()).  If the user
 *  attempts to call the overloaded function for which there is not an
 *  exact overloaded type match, then the compiler can potentially
 *  become confused as to whether to coerce the type into an Any or into
 *  one of the other overloaded types.  That is, assume there exists the
 *  following overloaded functions:
 *  <pre>
 *    void foo(int);
 *    void foo(Any);
 *  </pre>
 *  the compiler will throw an ambiguous overload error for the following:
 *  <pre>
 *    short int si = 0;
 *    foo(si);
 *  </pre>
 *
 *  By defining the overload function to take an AnyAntiCoercionWrapper,
 *  the compiler will automatically coerce any Any objects into the
 *  wrapper class, but it will not chain coercions (that is, any other
 *  data into an Any, and that Any into the wrapper).  This way, we can
 *  make use of the powerful application of implicit coercion for new
 *  functions, while still being able to provide Any-specific overloads
 *  for system or third-party functions.
 */
class AnyAntiCoercionWrapper
{
public:
   AnyAntiCoercionWrapper(Any& any)
      : m_any(any)
   {}

   Any& m_any;
};

/// A const form of the AnyAntiCoercionWrapper class
/** See the utilib::AnyAntiCoercionWrapper class.  This variant accepts
 *  const or non-const Anys and recrods them as a const reference.
 */
class ConstAnyAntiCoercionWrapper
{
public:
   ConstAnyAntiCoercionWrapper(const Any& any)
      : m_any(any)
   {}

   const Any& m_any;
};


//---------------------------------------------------------------------
// Default comparators and printers for standard C++ classes

/** \brief Declare that a TYPE implements operator<() & operator==() 
 *  (must be called within utilib namespace)
 */
#define DEFINE_DEFAULT_ANY_COMPARATOR(TYPE)       \
   template<>                                     \
   class Any::Comparator<TYPE >                   \
      : public Any::DefaultComparable<TYPE > {}

/** \brief Declare that a TYPE implements operator<<(ostream&, TYPE)
 *  (must be called within utilib namespace)
 */
#define DEFINE_DEFAULT_ANY_PRINTER(TYPE)          \
   template<>                                     \
   class Any::Printer<TYPE >                      \
      : public Any::DefaultPrinter<TYPE > {}

/** \brief Declare that a TYPE implements operator<<(ostream&, TYPE) and
 *  requires setting the stream precision (must be called within utilib
 *  namespace)
 */
#define DEFINE_FLOATING_ANY_PRINTER(TYPE, PRECISION)                    \
   template<>                                                           \
   class Any::Printer<TYPE > {                                          \
   public:                                                              \
      static std::ostream& print(std::ostream& os, const TYPE& data) {  \
         std::streamsize tmp = os.precision();                          \
         os.precision(PRECISION);                                       \
         os << data;                                                    \
         os.precision(tmp);                                             \
         return os;                                                     \
      }                                                                 \
   }


/** \brief Declare that a TYPE implements operator>>(istream&, TYPE)
 *  (must be called within utilib namespace)
 */
#define DEFINE_DEFAULT_ANY_READER(TYPE)           \
   template<>                                     \
   class Any::Reader<TYPE >                       \
      : public Any::DefaultReader<TYPE > {}

/** \brief Declare that a TYPE implements operator<<(utilib::PackBuf&, TYPE)
 *  (must be called within utilib namespace)
 */
#define DEFINE_DEFAULT_ANY_PACKER(TYPE)           \
   template<>                                     \
   class Any::Packer<TYPE >                       \
      : public Any::DefaultPacker<TYPE > {}

/** \brief Declare that a TYPE implements operator<() & operator==() 
 *  (must be called within utilib namespace)
 */
#define DEFINE_FULL_ANY_EXTENSIONS(TYPE) \
   DEFINE_DEFAULT_ANY_COMPARATOR(TYPE);  \
   DEFINE_DEFAULT_ANY_PRINTER(TYPE);     \
   DEFINE_DEFAULT_ANY_READER(TYPE);      \
   DEFINE_DEFAULT_ANY_PACKER(TYPE)

DEFINE_FULL_ANY_EXTENSIONS(char);
DEFINE_FULL_ANY_EXTENSIONS(signed char);
DEFINE_FULL_ANY_EXTENSIONS(unsigned char);

DEFINE_FULL_ANY_EXTENSIONS(short int);
DEFINE_FULL_ANY_EXTENSIONS(unsigned short int);
DEFINE_FULL_ANY_EXTENSIONS(int);
DEFINE_FULL_ANY_EXTENSIONS(unsigned int);
DEFINE_FULL_ANY_EXTENSIONS(long);
DEFINE_FULL_ANY_EXTENSIONS(unsigned long);

DEFINE_FULL_ANY_EXTENSIONS(bool);
DEFINE_FULL_ANY_EXTENSIONS(std::string);

// wchar_t is not readable...
DEFINE_DEFAULT_ANY_COMPARATOR(wchar_t);
DEFINE_DEFAULT_ANY_PRINTER(wchar_t);

// floating-point requires a special printer
DEFINE_DEFAULT_ANY_COMPARATOR(float);
DEFINE_DEFAULT_ANY_READER(float);
DEFINE_DEFAULT_ANY_PACKER(float);
DEFINE_FLOATING_ANY_PRINTER(float, FLT_DIG);
DEFINE_DEFAULT_ANY_COMPARATOR(double);
DEFINE_DEFAULT_ANY_READER(double);
DEFINE_DEFAULT_ANY_PACKER(double);
DEFINE_FLOATING_ANY_PRINTER(double, DBL_DIG);
DEFINE_DEFAULT_ANY_COMPARATOR(long double);
DEFINE_DEFAULT_ANY_READER(long double);
DEFINE_DEFAULT_ANY_PACKER(long double);
DEFINE_FLOATING_ANY_PRINTER(long double, LDBL_DIG);

// Believe it or not, Any's are actually default comparable & printable
DEFINE_FULL_ANY_EXTENSIONS(utilib::Any);
DEFINE_FULL_ANY_EXTENSIONS(utilib::AnyRef);
DEFINE_FULL_ANY_EXTENSIONS(utilib::AnyFixedRef);


//---------------------------------------------------------------------
// Comparators (but not printers) for standard C++ STL classes

/** We define 2 versions of the std::pair Comparators.  The combination
 *  of const & non-const templates appears to cover all permutations.
 */
template<typename T, typename U>
class Any::Comparator<std::pair<const T, const U> > 
{
public:
   static bool isEqual(const std::pair<const T, const U>& lhs, 
                       const std::pair<const T, const U>& rhs)
   {
      return Any::Comparator<T>::isEqual(lhs.first, rhs.first) 
         && Any::Comparator<U>::isEqual(lhs.second, rhs.second);
   }
   static bool isLessThan(const std::pair<const T, const U>& lhs, 
                          const std::pair<const T, const U>& rhs)
   {
      if ( Any::Comparator<T>::isLessThan(lhs.first, rhs.first) )
         return true;
      if ( Any::Comparator<T>::isLessThan(rhs.first, lhs.first) )
         return false;
      return Any::Comparator<U>::isLessThan(lhs.second, rhs.second);
   }
};


#if !defined(UTILIB_SOLARIS_CC)
// Under Solaris, this definition appears to produce abmiguous function
// overload errors.  With (at least) GCC, this is required for
// const-correctness.  (i.e. without this overload, there are unit tests
// that will fail)
template<typename T, typename U>
class Any::Comparator<std::pair<T, U> > 
   : public Any::Comparator<std::pair<const T, const U> > 
{};
#endif


/** We define 2 versions of the std::pair Printers.  The combination
 *  of const & non-const templates appears to cover all permutations.
 */
template<typename T, typename U>
class Any::Printer<std::pair<const T, const U> > 
{
public:
   static std::ostream& print( std::ostream& os, 
                               const std::pair<const T, const U>& data )
   {
      os << "(";
      Any::Printer<T>::print(os, data.first);
      os << ", ";
      Any::Printer<U>::print(os, data.second);
      os << ")";
      return os;
   }
};


#if !defined(UTILIB_SOLARIS_CC)
// Under Solaris, this definition appears to produce abmiguous function
// overload errors.  With (at least) GCC, this is required for
// const-correctness.  (i.e. without this overload, there is a unit test
// that will fail to print correctly).
template<typename T, typename U>
class Any::Printer<std::pair<T, U> > 
   : public Any::Printer<std::pair<const T, const U> > 
{};
#endif

namespace STL_Any_AuxFcns {  // ::utilib::STL_Any_AuxFcns

/** A comparator for all sequence-based (i.e. iteratable STL containers).
 *
 *  We do NOT want to use the DefaultComparator because that will force
 *  ALL types that are placed in a vector that is subsequently put into
 *  an Any to support operator==() and operator<().  By re-defining the
 *  comparison opperators for the STL classes, we can avoid this
 *  requirement.
 */
template<typename T>
class SequenceComparator 
{
public:
   static bool isEqual(const T& lhs, const T& rhs)
   {
      typename T::const_iterator it1 = lhs.begin();
      typename T::const_iterator it1_end = lhs.end();
      typename T::const_iterator it2 = rhs.begin();
      typename T::const_iterator it2_end = rhs.end();

      for( ; ( it1 != it1_end ) && ( it2 != it2_end ); ++it1, ++it2 )
      {
         if ( ! Any::Comparator<typename T::value_type>::isEqual(*it1, *it2) )
	    return false;
      }
      return it1 == it1_end && it2 == it2_end;
   }
   static bool isLessThan(const T& lhs, const T& rhs)
   {
      typename T::const_iterator it1 = lhs.begin();
      typename T::const_iterator it1_end = lhs.end();
      typename T::const_iterator it2 = rhs.begin();
      typename T::const_iterator it2_end = rhs.end();

      for( ; ( it1 != it1_end ) && ( it2 != it2_end ); ++it1, ++it2 )
      {
         if ( Any::Comparator<typename T::value_type>::isLessThan(*it1, *it2) )
	    return true;
         if ( Any::Comparator<typename T::value_type>::isLessThan(*it2, *it1) )
	    return false;
      }
      return it1 == it1_end && it2 != it2_end;
   }
};

/** A printer for all sequence-based (i.e. iteratable STL containers).
 *
 *  We do NOT want to use the DefaultComparator because that will force
 *  ALL types that are placed in a vector that is subsequently put into
 *  an Any to support operator==() and operator<().  By re-defining the
 *  comparison opperators for the STL classes, we can avoid this
 *  requirement.
 */
template<typename T>
class SequencePrinter
{
public:
   static std::ostream& print(std::ostream& os, const T& data)
   {
      if ( data.empty() )
      {
         os << "[ ]";
         return os;
      }

      os << "[ ";
      typename T::const_iterator it = data.begin();
      typename T::const_iterator it_end = data.end();
      Any::Printer<typename T::value_type>::print(os, *it);
      for( ++it ; it != it_end; ++it )
      {
         os << ", ";
         Any::Printer<typename T::value_type>::print(os, *it);
      }
      os << " ]";
      return os;
   }
};

} // namespace ::utilib::STL_Any_AuxFcns

template<typename T>
class Any::Comparator<std::vector<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::vector<T> > {};

template<typename T>
class Any::Comparator<std::list<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::list<T> > {};

template<typename T>
class Any::Comparator<std::deque<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::deque<T> > {};

template<typename T>
class Any::Comparator<std::set<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::set<T> > {};

template<typename T>
class Any::Comparator<std::multiset<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::multiset<T> > {};

template<typename T, typename U>
class Any::Comparator<std::map<T,U> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::map<T,U> > {};

template<typename T, typename U>
class Any::Comparator<std::multimap<T,U> > 
   : public STL_Any_AuxFcns::SequenceComparator<std::multimap<T,U> > {};

template<typename T>
class Any::Printer<std::vector<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::vector<T> > {};

template<typename T>
class Any::Printer<std::list<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::list<T> > {};

template<typename T>
class Any::Printer<std::deque<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::deque<T> > {};

template<typename T>
class Any::Printer<std::set<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::set<T> > {};

template<typename T>
class Any::Printer<std::multiset<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::multiset<T> > {};

template<typename T, typename U>
class Any::Printer<std::map<T,U> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::map<T,U> > {};

template<typename T, typename U>
class Any::Printer<std::multimap<T,U> > 
   : public STL_Any_AuxFcns::SequencePrinter<std::multimap<T,U> > {};

} // namespace ::utilib



//---------------------------------------------------------------------

/// Provide generic output streaming for the Any class and Any derivatives
std::ostream& operator<<( std::ostream& os, 
                          const utilib::ConstAnyAntiCoercionWrapper any );

/// Provide generic input streaming for the Any class and Any derivatives
std::istream& operator>>( std::istream& os, 
                          utilib::AnyAntiCoercionWrapper any );

/// Provide generic output streaming for the Any class and Any derivatives
utilib::PackBuffer& operator<<(utilib::PackBuffer& os, 
                               const utilib::ConstAnyAntiCoercionWrapper any);

/// Provide generic output streaming for the Any class and Any derivatives
utilib::UnPackBuffer& operator>>( utilib::UnPackBuffer& os, 
                                  utilib::AnyAntiCoercionWrapper any );

// WEH - where should we put STL packer declarations???
namespace utilib {  // :: utilib
template<>
class Any::Packer<std::vector<int> > 
   : public Any::DefaultPacker<std::vector<int> > {};
} // namespace ::utilib



#ifdef CXXTEST_RUNNING
#include <cxxtest/ValueTraits.h>
namespace CxxTest { // ::CxxTest

/// Printing utility for use in CxxTest unit tests
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<utilib::Any>
{
public:
   ValueTraits( const utilib::Any &a )
   { 
      std::ostringstream ss;
      ss << a;
      str = ss.str();
   }

   const char* asString() const
   { return str.c_str(); }

private:
   std::string str;
};

} // namespace ::CxxTest
#endif // CXXTEST_RUNNING

#endif // utilib_Any_h
