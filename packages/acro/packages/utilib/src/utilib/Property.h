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
 * \file Property.h
 *
 * Defines the utilib::Property, utilib::ReadOnly_Property, and
 * utilib::Privileged_Property classes.
 */

#ifndef utilib_Property_h
#define utilib_Property_h

#include <utilib/std_headers.h>
#include <utilib/ReferenceCounted.h>
#include <utilib/Any.h>
#include <utilib/TypeManager.h>

#ifdef UTILIB_HAVE_BOOST
#include <boost/function.hpp>
#include <boost/signal.hpp>
#endif // UTILIB_HAVE_BOOST

namespace utilib {

class property_error : public std::runtime_error
{
public:
   /// Constructor
   property_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};


class Property;
class Privileged_Property;

/** The utilib::ReadOnly_Property class implements the bulk of the
 *  utilib::Property class.  This provides all of the functionality of
 *  the Property class, except for the ability to set new property
 *  values.
 *
 *  See the utilib::Property documentation for a discussion of the API.
 */
class ReadOnly_Property
{
#ifdef UTILIB_HAVE_BOOST
   struct logical_and_combiner 
   {
      typedef bool result_type;

      template<typename InputIterator>
      bool operator()(InputIterator first, InputIterator last) const
      {
         if (first == last)
            return true;

         bool ans = true;
         for( ; first != last; ++first )
            ans &= *first;
         return ans;
      }
   };

public:
   // NB: the "&" for the arguments IS REQUIRED for things to work
   // correctly, and many compilers WILL NOT catch it if you omit it.
   // [get & set function parameters are void(prop_data, external_value)]
   typedef boost::function<void( utilib::Any&, const utilib::Any  )> set_fcn_t;
   typedef boost::function<void( const utilib::Any&, utilib::Any& )> get_fcn_t;
   typedef boost::signal< void( const ReadOnly_Property& ) >  onChange_t;
   typedef boost::signal< bool( const ReadOnly_Property&, const Any& ), 
                          logical_and_combiner >  validate_t;

   inline bool connected(set_fcn_t &f) const { return ! f.empty(); }
   inline bool connected(get_fcn_t &f) const { return ! f.empty(); }
#else
   /// Simple proxy for the boost::signals class
   template<typename FCN>
   class Listener
   {
   public:
      void connect( FCN cb )
      { listeners.insert(cb); }

      void disconnect( FCN cb )
      { listeners.erase(cb); }

   protected:
      std::set<FCN> listeners;
      typedef typename std::set<FCN>::iterator iterator_t;
   };

   /// Specialization for the onChange() callback
   class OnChangeListener : public Listener<void(*)(const ReadOnly_Property&)>
   {
   public:
      void operator()(const ReadOnly_Property& property)
      {
         iterator_t itEnd = listeners.end();
         for(iterator_t it = listeners.begin(); it != itEnd; ++it)
            (*it)(property);
      }
   };

   /// Specialization for the validate() callback
   class ValidateListener 
      : public Listener<bool(*)(const ReadOnly_Property&, const Any&)>
   {
   public:
      bool operator()(const ReadOnly_Property& property, Any& new_value)
      {
         bool ans = true;
         iterator_t itEnd = listeners.end();
         for(iterator_t it = listeners.begin(); it != itEnd; ++it)
            ans &= (*it)(property, new_value);
         return ans;
      }
   };

public:
   // NB: the "&" for the arguments IS REQUIRED for things to work
   // correctly, and many compilers WILL NOT catch it if you omit it.
   // [get & set function parameters are void(prop_data, external_value)]
   typedef void(*set_fcn_t)(utilib::Any&, const utilib::Any);
   typedef void(*get_fcn_t)(const utilib::Any&, utilib::Any&);
   typedef OnChangeListener  onChange_t;
   typedef ValidateListener  validate_t;

   inline bool connected(set_fcn_t &f) const { return f != NULL; }
   inline bool connected(get_fcn_t &f) const { return f != NULL; }
#endif // UTILIB_HAVE_BOOST

   typedef void(onChange_t::*onChange_signal_t)( const ReadOnly_Property& );

public:
   ReadOnly_Property()
      : data()
   {
      data->value.set<UntypedAnyContainer>();
      data->set_functor = set_fcn_t();
      data->get_functor = get_fcn_t();
   }

   ReadOnly_Property(const ReadOnly_Property& rhs)
      : data(rhs.data)
   {}

   virtual ~ReadOnly_Property() 
   {}

   /// Get the current property value
   inline utilib::Any get() const
   {
      if ( connected(data->get_functor) )
      {
         utilib::Any tmp;
         data->get_functor(data->value, tmp);

         // We want to defeat the normal Any shallow copies so that the
         // user can't set the ReadOnly_Property value without going
         // through the set() method.
         if ( tmp.anyCount() == 0 )
            return tmp;
         else
            return tmp.clone();
      }
      else if ( data->value.is_type(typeid(UntypedAnyContainer)) )
         return data->value.expose<UntypedAnyContainer>().m_data.clone();
      else
         return data->value.clone();
   }

   /// Rebind the get and set functors to call new functions
   /* JDS: Disabled until we determine it is really needed.
    */
   //void rebind(set_fcn_t set = set_fcn_t(), get_fcn_t get = get_fcn_t())
   //{
   //   set_functor = set;
   //   get_functor = get;
   //}

   /// Convenience method for implicitly getting the value of a property
   template<typename T>
   operator T() const
   {
      return as<T>();
   }

   /// Convenience method for explicitly getting the value of a property
   template<typename T>
   T as() const
   { 
      utilib::Any tmp;
      TypeManager()->lexical_cast(get(), tmp, typeid(T), false);
      return tmp.template expose<T>();
   }

   /// Convenience method for getting a reference to the stored data
   /** This method somewhat violates the spirit of the Property class,
    *  in that it allows the user to obtain a reference to the bound,
    *  contained data.  In theory, providing this method can allow users
    *  to directly change the contained value (via const_cast),
    *  bypassing the validate() and onChange() events.
    *
    *  However, for accessing elements of properties that contain large
    *  amounts of data (i.e. an element from a vector), this is
    *  significantly more efficient (it bypasses several functions and a
    *  complete copy of the data).
    */
   template<typename T>
   const T& expose() const
   { 
      if ( connected(data->get_functor) )
         EXCEPTION_MNGR(property_error, "Property::expose(): not availiable "
                        "for Properties with non-default get_functor");

      return data->value.expose<T>(); 
   }

   virtual utilib::AnyRef asAnyRef() const
   { return utilib::AnyRef(*this); }

   virtual utilib::AnyFixedRef asAnyFixedRef() const
   { return utilib::AnyFixedRef(*this); }

   /// Special case for getting the value of the property into an Any
   /** This explicit conversion is needed to resolve an ambiguity
    *  between using the Property's templated conversion operator
    *  [operator T() const] and the templated utilib::Any constructor.
    *  By providing an explicit conversion operator, the compiler will
    *  pick this function over the templated Any constructor.
    */
   operator utilib::Any() const
   { return get(); }

// For most compilers, these explicit conversion operations are
// legitimate.  However, for AIX compilers (xlC v8.0, tested Feb 2010),
// the compiler throws an ambiguous override error for these explicit
// conversion operations when attempting:
//     utilib::Property p(utilib::Property::Bind<int>(5));
//     utilib::Any a = p;
//
// For that platform, conversions to AnyRef and AnyFixedRef MUST either use:
//     utilib::AnyRef      a1 = p.as<utilib::AnyRef>();
//     utilib::AnyFixedRef a2 = p.as<utilib::AnyFixedRef>();
// or the explicit Any::set() method.
//
#if ! defined(UTILIB_AIX_CC)

   /// Special case for getting an AnyRef that holds this property
   /** This prevents the ambiguous case of should I convert the Property
    *  to a "AnyRef" using the Property's implicit cast operator, or
    *  should I use the AnyRef's implicit conctructor?  That is:
    *    \code
    *    utilib::Property p;
    *    utilib::AnyRef a = p;
    *    \endcode
    *
    *  This forces the latter case.
    *
    *    - GCC (by chance) picks up the latter automatically.
    *    - ICC threw up its arms in confusion.
    */
   operator utilib::AnyRef() const
   { return asAnyRef(); }

   /// Special case for getting an AnyFixedRef that holds this property
   /** This prevents the ambiguous case of should I convert the Property
    *  to a "AnyFixedRef" using the Property's implicit cast operator, or
    *  should I use the AnyFixedRef's implicit conctructor?  That is:
    *    \code
    *    utilib::Property p;
    *    utilib::AnyFixedRef a = p;
    *    \endcode
    *
    *  This forces the latter case.
    *
    *    - GCC (by chance) picks up the latter automatically.
    *    - ICC threw up its arms in confusion.
    */
   operator utilib::AnyFixedRef() const
   { return asAnyFixedRef(); }

#endif // ! defined(UTILIB_AIX_CC)

   /// is this really a read-only property?
   bool readonly() const
   { return ! data->is_writable; }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator==(const T& rhs) const
   {
      utilib::Any L = get();
      if ( L.empty() )
         return false;

      utilib::Any tmp;
      try {
         int ans = TypeManager()->lexical_cast(L, tmp, typeid(T), false);
         if ( ans == 0 )
            return tmp.template expose<T>() == rhs;
      } catch ( utilib::bad_lexical_cast &e ) {
      }

      return equality_compare_any(utilib::Any(rhs));
   }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator!=(const T& rhs) const
   { return ! operator==(rhs); }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator<(const T& rhs) const
   { 
      utilib::Any L = get();
      if ( L.empty() )
         return true;

      utilib::Any tmp;
      try {
         int ans = TypeManager()->lexical_cast(L, tmp, typeid(T), false);
         if ( ans == 0 )
            return tmp.template expose<T>() < rhs;
      } catch ( utilib::bad_lexical_cast &e ) {
      }

      return lessThan_compare_any(utilib::Any(rhs));
   }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator<=(const T& rhs) const
   { return operator<(rhs) || operator==(rhs); }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator>(const T& rhs) const
   { return !operator<(rhs) && !operator==(rhs); }

   /// Convenience: implicitly compare this Property's value with "anything"
   template<typename T>
   bool operator>=(const T& rhs) const
   { return ! operator<(rhs); }

   // Convenience: implicitly convert to bool and return negation
   bool operator!() const
   { return ! as<bool>(); }

   // Convenience: logical AND
   bool operator&&(const bool& rhs) const
   { return as<bool>() && rhs; }

   // Convenience: logical OR
   bool operator||(const bool& rhs) const
   { return as<bool>() || rhs; }

   /// Allow listeners to validate a new value before it is set()
   validate_t & validate() const
   { return data->validate; }

   /// Allow listeners to be notified (but not validate) when the value changes
   onChange_t & onChange() const
   { return data->onChange; }

   /// True if this Property references the same data as the parameter
   bool equivalentTo(const ReadOnly_Property& ref) const
   { return data == ref.data; }

private:  // member functions

   /// Compare the value of this Property to another Property
   bool equality_compare_property(const ReadOnly_Property& rhs) const;

   /// Compare the value of this Property to the data contained in an Any
   bool equality_compare_any(const Any& rhs) const;

   /// Compare the value of this Property to another Property
   bool lessThan_compare_property(const ReadOnly_Property& rhs) const;

   /// Compare the value of this Property to the data contained in an Any
   bool lessThan_compare_any(const Any& rhs) const;


protected:  // member data

   /// Create a new ReadOnly_Property
   ReadOnly_Property( Any _value, set_fcn_t set, get_fcn_t get )
      : data()
   {
      data->value = _value;
      data->set_functor = set;
      data->get_functor = get;
   }

   /// The Data structure holds all data used by the class
   /** This indirection structure is a convenient place to put
    *  callbacks.  The ReadOnly_Property class holds the Data structure
    *  within a ReferenceCounted object.  This allows us to use
    *  non-copyable callbacks (i.e. boost::signal), while still allowing
    *  the ReadOnly_Property to be "copied" around.  It is also slightly more
    *  memory efficient to put all the callbacks into a single
    *  ReferenceCounted object instead of putting each into its own
    *  ReferenceCounted object.
    */
   struct Data {
      Data()
         : is_writable(true)
      {}

      /// The actual data held (or referened) by this Property
      utilib::Any  value;

      /// True if this Property is writable (i.e. not ReadOnly)
      /** Although the presence of a "writable" state in addition to the
       *  ReadOnly_Property class appears to be redundant, it is
       *  necessary in order to allow insertion of read only Properties
       *  into a PropertyDict (which only holds and returns Property
       *  instances).
       *
       *  Control over this state is implemented in the
       *  Privileged_Property class.
       */
      bool is_writable;

      /// Single function called to set the ReadOnly_Property value
      set_fcn_t  set_functor;

      /// Single function called to get the ReadOnly_Property value
      get_fcn_t  get_functor;

      /// (External) function(s) to call after the value is set()
      onChange_t onChange;
      /// (External) function(s) to call before the value is set()
      validate_t validate;
   };

   /// All data necessary for implementing the entire Property system
   /// (including full Property and Privileged_Property classes).
   utilib::ReferenceCounted<Data>  data;
};


/** The utilib::Property class provides a mechanism for managing
 *  "untyped" references to object "properties".  The Property system
 *  makes heavy use of utilib::Any and the global utilib::TypeManager()
 *  to provide a type-agnostic interface to the property value.
 *
 *  Key features:
 *    - Type agnostic system for accessing the property value.  This
 *      relies on the TypeManager() to implicitly convert data to and from
 *      the bound property type.  For example, the following is legal:
 *
 *      \code
 *      int value = 5;
 *      utilib::Property prop(value);  // bind the property to 'value'
 *      prop = 10;                     // 'value' now == 10
 *      int i = prop;                  // 'i' == 10
 *      double d = prop;               // 'd' == 10.0
 *      \endcode
 *
 *    - A Property may be bound to a variable, in which case accessing 
 *      the property (read or write) reads or changes the bound variable.
 *      Properties may also be bound to a type (with the value stored in
 *      an Any within the Property), or completely unbound, where the
 *      contained data type matches the last value assigned to the
 *      Property.
 *
 *    - Users may declare custom callbacks that are called for both
 *      getting and setting the property value.
 *
 *    - (External) users may register callbacks to be notified when the
 *      property value has changed [see onChange()]
 *
 *    - (External) users may register callbacks to be notified before
 *      setting a new property value, and have the opportunity to reject
 *      the new value (throwing an exception) [see validate()]
 */
class Property : public ReadOnly_Property
{
public:
   /// Utility for binding a Property to a type (access via Property::Bind())
   struct BoundType {
   private:
      friend class Property;
      BoundType() : handle() {}
      utilib::Any handle;
   };

   /// Create an unbound property
   /** This creates an unbound property that is not bound to a specific
    *  external data source or data type.  Any assignment to an Unbound
    *  Untyped property will throw away any current value and replace it
    *  with the new value (and the new value's type).
    *
    *  NB: This relies on the default constructor for the set_fcn_t and
    *  get_fcn_t().  If we are compiling with Boost, this will create an
    *  empty() function object.  If we are compiling without Boost, per
    *  the C++ standard, the default constructor for pointer types is a
    *  NULL pointer (get() / set() relies on this).
    */
   Property()
      : ReadOnly_Property(UntypedAnyContainer(), set_fcn_t(), get_fcn_t())
   {}

   /// Explicit copy constructor (to prevent accidental wrapping)
   Property(const Property& rhs)
      : ReadOnly_Property(rhs)
   {}

   /// Create an unbound property
   /** This creates an unbound property that is not bound to a specific
    *  external data source or data type.  Any assignment to an Unbound
    *  Untyped property will throw away any current value and replace it
    *  with the new value (and the new value's type).
    *
    *  NB: This form of the constructor allows the caller to specify
    *  user-defined functions / functors for setting and getting the
    *  value.  We do not rely on implicit values for the set/get
    *  functors becausesome compilers (notably, ICC 10.1) interpret
    *  "Property(set, get)" as a bound property
    *  "Property<set_fcn_t>(bound_value=set, set=get, get=get_fcn_t())"
    */
   explicit Property( set_fcn_t set, get_fcn_t get )
      : ReadOnly_Property(UntypedAnyContainer(), set, get)
   {}

   /// Create a property that is bound to the provided external data source
   /** This creates an typed property that is bound to a specific
    *  external data source.  Any assignment to this property will
    *  convert the new value into the bound type and store it in the
    *  bound external data source.
    *
    *  NB: using externally-bound data sources opens you to the
    *  possibility that the external source may be changed directly
    *  without calling the validate() or onChange() events.
    *
    *  NB: This relies on the default constructor for the set_fcn_t and
    *  get_fcn_t().  If we are compiling with Boost, this will create an
    *  empty() function object.  If we are compiling without Boost, per
    *  the C++ standard, the default constructor for pointer types is a
    *  NULL pointer (get() / set() relies on this).
    */
   template<typename T>
   explicit Property( T& bound_value )
      : ReadOnly_Property(Any(bound_value,true,true), set_fcn_t(), get_fcn_t())
   {}

   /// Create a property that is bound to the provided external data source
   /** This creates an typed property that is bound to a specific
    *  external data source.  Any assignment to this property will
    *  convert the new value into the bound type and store it in the
    *  bound external data source.
    *
    *  NB: using externally-bound data sources opens you to the
    *  possibility that the external source may be changed directly
    *  without calling the validate() or onChange() events.
    */
   template<typename T>
   explicit Property( T& bound_value, set_fcn_t set, get_fcn_t get )
      : ReadOnly_Property(Any(bound_value, true, true), set, get)
   {}

   /// Create a property that is bound to a specific data type
   /** This creates an typed property that is not bound to a specific
    *  external data source.  Any assignment to this property will
    *  convert the new value into the bound type and store it within
    *  this property.  Note: use the Bind() static method to crete an
    *  unbound typed property; for example, to create an unbound
    *  Property of fixed type double, use:
    *  \code
    *  utilib::Property property(utilib::Property::Bind<double>());
    *  \endcode
    *
    *  NB: This relies on the default constructor for the set_fcn_t and
    *  get_fcn_t().  If we are compiling with Boost, this will create an
    *  empty() function object.  If we are compiling without Boost, per
    *  the C++ standard, the default constructor for pointer types is a
    *  NULL pointer (get() / set() relies on this).
    */
   explicit Property(Property::BoundType type,
                     set_fcn_t set = set_fcn_t(), get_fcn_t get = get_fcn_t())
      : ReadOnly_Property(type.handle, set, get)
   {}

   virtual ~Property() 
   {}


   template<typename T>
   static BoundType Bind()
   { 
      BoundType tmp;
      tmp.handle.template set_immutable<T>();
      return tmp;
   }
   template<typename T>
   static BoundType Bind(T value)
   { 
      BoundType tmp;
      tmp.handle.template set<T>(value, false, true);
      return tmp;
   }

   /// Set the property value
   void set(utilib::Any new_value)
   { set_impl(new_value, data->is_writable); }

#ifdef UTILIB_HAVE_BOOST
   /// Typedef for a functor bound to this Property's set() operation
   typedef onChange_t::slot_function_type bound_set_t;
   /// Utility to return a bound set() suitable for the onChange callback
   bound_set_t bind_set();
#endif

   /// Return this Property within an AnyRef
   virtual utilib::AnyRef asAnyRef() const
   { return utilib::AnyRef(*this); }

   /// Return this Property within an AnyFixedRef
   virtual utilib::AnyFixedRef asAnyFixedRef() const
   { return utilib::AnyFixedRef(*this); }

   /// Convenience method for implicitly setting the value of a property
   Property& operator=(utilib::Any new_value)
   {
      set_impl(new_value, data->is_writable);
      return *this;
   }

   Property& operator=(ReadOnly_Property &rhs)
   {
      ReadOnly_Property::operator=(rhs);
      return *this;
   }

protected:
   
   /// Set the property value
   void set_impl(utilib::Any new_value, bool writable);

};

/// Special case: compilers pick up this template when copy constructing
template<>
Property::
Property(const Privileged_Property& rhs);

/// Special case: compilers pick up this template when copy constructing
template<>
Property::
Property(Property& rhs);

/// Special case: compilers pick up this template when copy constructing
template<>
Property::
Property(Privileged_Property& rhs);


/** The utilib::Privileged_Property class is a simple derivative of a
 *  "normal" Property that provides control over the "is_writable" flag.
 *  It introduces methods for setting and clearing the is_writable flag,
 *  as well as a set() method that will always succeed, regardless of
 *  the current is_writable state.
 *
 *  This class also supports the following use case: suppose I have a
 *  class that wants to have a public ReadOnly property "Foo".  Foo is
 *  used by consumers of a class both to check on the state of Foo and
 *  to register onChange (or even validate) callbacks.  To make sure
 *  that the onChange callbacks are always called, the data is stored
 *  within an unbound typed property.  In addition, the property is
 *  added to the class's base class's PropertyDict.  The "catch" is that
 *  the owning class still needs a handle on the Property though which
 *  it can set the property's value.  Further, derived classes may want
 *  to relax the read-only restriction for Foo.  To accomplish this, the
 *  class will declare a public ReadOnly Foo and a protected Privileged
 *  _Foo, both bound to the same data:
 *
 *    \code
 *    class MyClass : public MyBase
 *    {
 *    protected:
 *       Privileged_Property _Foo;
 *
 *    public:
 *       ReadOnly_Property Foo;
 *
 *       MyClass()
 *          : _Foo(Property::Bind<double>()),
 *            Foo(_Foo.set_readonly())
 *       {
 *          MyBase::properties.declare("Foo", _Foo);
 *       }
 *    };
 *    \endcode
 */
class Privileged_Property : public Property
{
public:
   class onChangeLoopData
   {
   private:
      friend class Privileged_Property;
      void configure(Privileged_Property* _owner, size_t _id)
      {
         owner = _owner;
         id = _id;
      }

      Privileged_Property*  owner;
      size_t  id;

   public:
      onChangeLoopData()
         : owner(NULL), id(0)
      {}

      ~onChangeLoopData();

      Privileged_Property* property() 
      { return owner; }
   };
   typedef ReferenceCounted<onChangeLoopData> onChangeLoop_t;

public:
   /// Create an unbound untyped property
   Privileged_Property()
      : Property()
   {}

   /// Explicit copy constructor (to prevent accidental wrapping)
   Privileged_Property(const Privileged_Property& rhs)
      : Property(rhs)
   {}

   /// Create an unbound untyped property
   explicit Privileged_Property( set_fcn_t set, get_fcn_t get )
      : Property(set, get)
   {}

   /// Create a property that is bound to the provided value
   template<typename T>
   explicit Privileged_Property( T& bound_value )
      : Property(bound_value)
   {}

   /// Create a property that is bound to the provided value
   template<typename T>
   explicit Privileged_Property( T& bound_value, set_fcn_t set, get_fcn_t get )
      : Property(bound_value, set, get)
   {}

   /// Create an unbound typed property
   explicit Privileged_Property( Property::BoundType type,
                                 set_fcn_t set = set_fcn_t(),
                                 get_fcn_t get = get_fcn_t() )
      : Property(type, set, get)
   {}

   virtual ~Privileged_Property() 
   {}


   /// Allow setting the property value regardless of the is_writable flag
   void set(utilib::Any new_value)
   { Property::set_impl(new_value, true); }

#ifdef UTILIB_HAVE_BOOST
   /// Utility to return a bound set() suitable for the onChange callback
   bound_set_t bind_set();
#endif

   /// Explicit conversion; see ReadOnly_Property::operator Any() const
   virtual operator utilib::Any() const
   { return get(); }

   /// Return this Privileged_Property within an AnyRef
   virtual utilib::AnyRef asAnyRef() const
   { return utilib::AnyRef(*this); }

   /// Return this Privileged_Property within an AnyFixedRef
   virtual utilib::AnyFixedRef asAnyFixedRef() const
   { return utilib::AnyFixedRef(*this); }

   /// Convenience method for implicitly setting the value of a property
   Privileged_Property& operator=(utilib::Any new_value)
   {
      Property::set_impl(new_value, true);
      return *this;
   }

   /// Calling this method allows setting the value through Property::set()
   ReadOnly_Property& set_readonly()
   {
      data->is_writable = false;
      return *this;
   }

   /// Calling this method disallows setting the value through Property::set()
   Property& unset_readonly()
   {
      data->is_writable = true;
      return *this;
   }

   /// Allow the onChange callback to recurse (up to max times)
   onChangeLoop_t allowOnChangeRecursion(size_t max = 1);
};

/// Special case: casting a Property to "void" should do nothing.
template<>
void ReadOnly_Property::as<void>() const;

/// Special case: casting a Property to "Any" should return the contained value
template<>
utilib::Any ReadOnly_Property::as<utilib::Any>() const;

/// Special case: casting a Property to "Any" should do nothing.
template<>
utilib::AnyRef ReadOnly_Property::as<utilib::AnyRef>() const;

/// Special case: casting a Property to "Any" should do nothing.
template<>
utilib::AnyFixedRef ReadOnly_Property::as<utilib::AnyFixedRef>() const;


/// Special case: compilers pick up this template when copy constructing
template<>
Privileged_Property::
Privileged_Property(Privileged_Property& rhs);


/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator==(const ReadOnly_Property& rhs) const;

/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator==(const Property& rhs) const;

/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator==(const Privileged_Property& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator==(const Any& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator==(const AnyRef& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator==(const AnyFixedRef& rhs) const;

/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator<(const ReadOnly_Property& rhs) const;

/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator<(const Property& rhs) const;

/// Special case: compilers pick up this template when comparing properties
template<>
bool ReadOnly_Property::operator<(const Privileged_Property& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator<(const Any& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator<(const AnyRef& rhs) const;

/// Special case: compilers pick up this template when comparing anys
template<>
bool ReadOnly_Property::operator<(const AnyFixedRef& rhs) const;


// Properties are printable and comparable within Anys...
DEFINE_DEFAULT_ANY_PRINTER(ReadOnly_Property);
DEFINE_DEFAULT_ANY_PRINTER(Property);
DEFINE_DEFAULT_ANY_PRINTER(Privileged_Property);
DEFINE_DEFAULT_ANY_COMPARATOR(ReadOnly_Property);
DEFINE_DEFAULT_ANY_COMPARATOR(Property);
DEFINE_DEFAULT_ANY_COMPARATOR(Privileged_Property);

// NB: of course, ReadOnly_Property cannot read
DEFINE_DEFAULT_ANY_READER(Property);
DEFINE_DEFAULT_ANY_READER(Privileged_Property);

} // namespace utilib



/// Method for printing out the value of properties
std::ostream& operator<<(std::ostream& os, 
                         const utilib::ReadOnly_Property& property);

/// Method for reading into properties
std::istream& operator>>( std::istream& is, utilib::Property& property );



#ifdef CXXTEST_RUNNING
#include <cxxtest/ValueTraits.h>
namespace CxxTest {

/// Printing utility for use in CxxTest unit tests
CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<utilib::ReadOnly_Property>
{
public:
   ValueTraits( const utilib::ReadOnly_Property &p )
   { 
      std::ostringstream ss;
      ss << p;
      str = ss.str();
   }

   const char* asString() const
   { return str.c_str(); }

private:
   std::string str;
};

CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<utilib::Property>
   : public ValueTraits<utilib::ReadOnly_Property>
{
public:
   ValueTraits(const utilib::Property& p)
      : ValueTraits<utilib::ReadOnly_Property>(p)
   {}
};

CXXTEST_TEMPLATE_INSTANTIATION
class ValueTraits<utilib::Privileged_Property>
   : public ValueTraits<utilib::Property>
{
public:
   ValueTraits(const utilib::Privileged_Property& p)
      : ValueTraits<utilib::Property>(p)
   {}
};

} // namespace CxxTest
#endif // CXXTEST_RUNNING

#endif // defined utilib_Property_h
