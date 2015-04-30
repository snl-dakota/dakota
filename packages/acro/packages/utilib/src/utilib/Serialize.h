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
 * \file Serialize.h
 *
 * Defines: 
 *    - utilib::SerialObject struct
 *    - utilib::SerialPOD class 
 *    - utilib::Serialization_Manager class
 *    - utilib::Serializer() singleton
 * 
 */

#ifndef utilib_Serialize_h
#define utilib_Serialize_h

#include <utilib/Any.h>

namespace utilib {

class Serialization_Manager;

namespace error {
namespace Serialization {
  const int EmptyData                = -300;
  const int ExtraData                = -301;
  const int UnknownType              = -302;
  const int DuplicateRegistration    = -303;
  const int BadPODData               = -304;
  const int BadPODTextConversion     = -305;
  const int MissingStringQuote       = -306;
  const int UnconvertedPODText       = -307;
} // namespace utilib::error::Serialization
} // namespace utilib::error


/// Return the system-wide Serialization_Manager singleton.
Serialization_Manager& Serializer();

/// Generic serializer exception from which all other exceptions will inherit
class serialization_error : public std::runtime_error
{
public:
   /// Constructor
   serialization_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};


class serializer_unknown_type : public serialization_error
{
public:
   /// Constructor
   serializer_unknown_type(const std::string& msg) 
      : serialization_error(msg)
   {}
};

class serializer_bad_pod_size : public serialization_error
{
public:
   /// Constructor
   serializer_bad_pod_size(const std::string& msg) 
      : serialization_error(msg)
   {}
};


/// A data "pseudo-object" for containing serialized Plain-ol-Data types
class SerialPOD
{
public:
   SerialPOD()
      : buffer(),
        text_mode(false)
   {}
   SerialPOD(size_t init_size)
      : buffer(init_size),
        text_mode(false)
   {}

   /// Set the contents of this SerialPOD (binary mode)
   void set(const void* buffer_src, const size_t length);

   /// Set the contents of this SerialPOD (text mode)
   void set(const std::string& buffer_src);

   /// Set the contents of this SerialPOD (text mode)
   bool is_text_mode() const
   { return text_mode; }

   /// Print out the stream of character values for this SerialPOD
   void print(std::ostream& os, std::string indent = "") const;

   size_t size() const
      { return buffer.size(); }
   const char* data() const
      { return &buffer[0]; }

private:
   std::vector<char>  buffer;
   bool  text_mode;
};


/// A structural "pseudo-object" for representing a class / structure 
class SerialObject
{
public:
   SerialObject() 
      : type(0),
        data()
   {}
   SerialObject(size_t type_id) 
      : type(type_id),
        data()
   {}

   void print(std::ostream& os, std::string indent = "") const;

   /// A list of SerialObjects (each element is a serialized object member)
   typedef std::list<SerialObject> elementList_t;

   size_t  type;
   Any     data;
};



/// The prototype for serilization/deserialization transformation functions
typedef int(*transform_fcn)(SerialObject::elementList_t&, Any&, bool);
/// The prototype for POD serialization/deserialization functions
typedef int(*pod_transform_fcn)(SerialPOD&, Any&, bool);
/// The prototype for serialization/deserialization POD-to-text functions
typedef int(*pod_text_transform_fcn)(std::string&, Any&, bool);
/// The prototype for an instantiation function (populate the provided Any)
typedef void(*initialization_fcn)(Any&);


/// Class for storing and managing all serialization functions
/** This class stores all serialization and type initialization function
 *  pointers and coordinates the process of serializing and
 *  deserializing data types.  It is published to the outside world
 *  through the Serializer() singleton.
 *
 *  The power of the utilib::Serialization_Manager and the
 *  characteristic that distinguishes it from other serialization
 *  approaches is its compatibility with the utilib::Any variant data
 *  type.  Most serialization approaches directly leverage templates to
 *  have the compiler insert the correct serialization / deserialization
 *  functions at compile time.  While this works well for concrete types
 *  (and even templated concrete types), it is incompatible with the
 *  utilib::Any variant data type, as the compiler cannot know what data
 *  is contained within the Any at compile time.  To facilitate the
 *  serialization of Anys the Serialization_Manager uses a registration
 *  system where serializable classes register their serialization
 *  functions with the manager, allowing the manager to dynamically
 *  locate and bind to the correct serialization function at runtime.
 *
 *  \par Interacting with the Serialization_Manager
 *
 *  Apart from registering new user-defined serializers (with
 *  Serialization_Manager::register_serializer()), the general public
 *  should never directly call the Serialization_Manager member
 *  functions.  Instead, the general public should make use of the
 *  utilib namespace-level functions:
 *     - ::Serialize
 *     - ::Deserialize
 *     - ::serial_transform
 *
 *  In general, "users" of the serialization system will call
 *  Serialize() and Deserialize().  As expected, Serialize() takes any
 *  object and returns the corresponding SerialObject, and Deserialize()
 *  takes a SerialObject and returns the deserialized object within an
 *  Any.  In essence, Serialize() and Deserialize() are convenience
 *  wrappers around serial_transform()
 *
 *  serial_transform() is a utility function to assist developers in
 *  extending serialization capabilities by defining and registering new
 *  serializer functions for user-defined types.  It is a wrapper around
 *  Serialization_Manager::transform_impl() that provides a mechanism
 *  for guaranteeing that serializers for template class instantiations
 *  are registered \em before they are encountered during a
 *  serialization operation.  This is covered in more detail in the
 *  "Registering your serialization functions" section below.
 *
 *  \par Serialization Functions
 *
 *  There are three fundamental serialization functions supported by the
 *  Serialization_Manager.  The first (::pod_transform_fcn) is for
 *  serializing and deserializing "Plain 'ol Data" (POD) types.  It will
 *  take the variable and convert it to/from a SerialPOD object by
 *  storing the variable's binary data as a char vector.  The
 *  Serialization_Manager defines and registers serializers for all
 *  non-pointer standard C++ POD data types.  
 *
 *  The second serialization function (::transform_fcn) is for
 *  serializing user-defined objects.  This function converts the
 *  contents the object in question into a SerialObject list, usually by
 *  successively calling serial_transform().  For example, consider the
 *  following user-defined class fragment and its (valid) serializer:
 *
 *  The third serialization function (::pod_text_transform_fcn) is not
 *  necessarily part of the core Serialization_Manager functionality,
 *  but is maintained here to assist some writers and parsers for
 *  serialized objects.  This serialization function converts POD data
 *  types to and from a char-based (not wchar_t-based) std::string.  The
 *  Serialization Manager will automatically fall back to the normal
 *  (binary) transformation if the text serialization function is NULL
 *  (or not provided).  All standard C++ POD types provide valid text
 *  serialization functions, with the exception of wchar_t.
 *
 *  \code
 *  class A
 *  {
 *     // ...
 *  private:
 *     static int serializer( utilib::SerialObject::elementList_t & serial,
 *                            utilib::Any & data, bool serialize )
 *     {
 *        A& me = const_cast<A&>(data.expose<A>());
 *        serial_transform(serial, me.a, serialize);
 *        serial_transform(serial, me.b, serialize);
 *        serial_transform(serial, me.c, serialize);
 *        return 0;
 *     }
 *
 *     int a;
 *     float b;
 *     std::list<double> c;
 *
 *     static const volatile int serializer_registered;
 *  };
 *  \endcode  That's it!
 *
 *  Both ::pod_transform_fcn and ::transform_fcn serialization functions
 *  are registered by calling one of four overloaded
 *  Serializer().register_serializer() template functions.
 *  register_serializer() takes a "user-defined" name, the transform
 *  function, and an optional initialization function that returns a new
 *  valid instance of the class within an Any (::initialization_fcn).
 *  If not provided, ::initialization_fcn defaults to using the Default
 *  Constructor.  
 *
 *  \par Registering your serialization functions 
 *
 *  In general, registering a serialization function entails notifying
 *  the Serialization_Manager of a relationship between a \c typeid, a
 *  user-defined name, a serialization function, and optionally, a
 *  constructor.  For more information on creating user-defined type
 *  names, see the section "Providing a user-defined name" below.
 *
 *  Proper registration of a class serialization function with the
 *  global Serializer() singleton is the critical step in getting the
 *  serialization process working correctly.  One of the most important
 *  aspects is to perform all registrations as \em early in the program
 *  executaion as possible.  Unlike template-based serialization
 *  strategies, it is possible to DEserialize an object of a particular
 *  class \em before that class is ever actually used in the program.
 *  As a result,the preferable way to register a serializer is through
 *  static variable initialization \em before entry into \c main().
 *  While there are many nuances to the process (most revolving around
 *  the C++ compiler and linker), most situations should fall into one
 *  of 4 basic categories:
 *
 *  \par 1) A concrete class you control
 *
 *  This is probably the most straightforward situation.  You have a
 *  concrete class that you have control over.  In this case, the
 *  simplest and most robust approach is to intrusively hook your class
 *  to the utilib::Serializer().  As in the <tt>class A</tt> example
 *  above, make the serialization function a private static member of
 *  the class, and add a <tt>private static const volatile int</tt>.  In
 *  the corresponding code (\c .cpp) file, register the serialization
 *  function by initializing the static const:
 *
 *  \code
 *  const volatile int A::serializer_registered
 *     = utilib::Serializer().register_serializer<A>("A", A::serializer);
 *  \endcode

 *  By tying the registration to the initialization of a static
 *  variable, simply using the class \em somewhere in the program will
 *  cause the serializer to be registered \em before entry into \c
 *  main().  Note, as no arguments depend on the template parameter (the
 *  type being serialized), you must explicitly specify it when invoking
 *  register_serializer().  Also note the use of the "volatile" keyword.
 *  This helps prevent linkers from "optimizing away" what is otherwise
 *  effectively a no-op.
 *
 *  \par 2) A template class you control
 *
 *  Templates are slightly more complicated than concrete classes, as
 *  most compilers only emit template code that is actuallr \em
 *  references somewhere in the program executable.  The solution is to
 *  explicitly reference the static registration variable in \b all
 *  constructors (including any copy constructors):
 *
 *  \code
 *  template<typename T1, typename T2>
 *  class B
 *  {
 *  public:
 *     B() { static_cast<void>(serializer_registered); }
 *
 *     B(const B& rhs)
 *     { 
 *        static_cast<void>(serializer_registered);
 *        // ...
 *     }
 *
 *  private:
 *     static int serializer( utilib::SerialObject::elementList_t & serial,
 *                            utilib::Any & data, bool serialize )
 *     {
 *        B& me = const_cast<B&>(data.expose<B>());
 *        serial_transform(serial, me.a, serialize);
 *        serial_transform(serial, me.b, serialize);
 *        serial_transform(serial, me.c, serialize);
 *        return 0;
 *     }
 *
 *     T1 a;
 *     T2 b;
 *     std::list<T1> c;
 *
 *     static const volatile int serializer_registered;
 *  };
 *
 *  template<typename T1, typename T2>
 *  const volatile int A<T1,T2>::serializer_registered
 *     = utilib::Serializer().register_serializer<A<T1,T2> >
 *        ( std::string("B;") + utilib::mangledName(typeid(T1)) + ";" 
 *            + utilib::mangledName(typeid(T2)), 
            B<T1,T2>::serializer );
 *  \endcode
 *
 *  In this case, the static const is initialized through a templated
 *  initializer, and the \c static_cast<void>(serializer_registered)
 *  lines create explicit references to cause the compiler to emit,
 *  link, and register the serialization function for every
 *  instantiation of the template class.  Note again that the use of the
 *  \c volatile keyword is critical in this scenario.
 *
 *  \par 3) A concrete class that you do \em not control
 *
 *  This case is a relatively trivial extension of case (1).  In this
 *  case, the class is owned or controled by someone else, so we cannot
 *  directly edit the class API.  Given a class \c C, construct a
 *  standalone serialization function and register it through a local
 *  static variable:
 *
 *  \code
 *  class C; // defined elsewhere
 *
 *  namespace {
 *     int serialize_C( utilib::SerialObject::elementList_t & serial,
 *                      utilib::Any & data, bool serialize )
 *     {
 *        C& me = const_cast<C&>(data.expose<C>());
 *        // ... (de)serialize through C's public API ...
 *        return 0;
 *     }
 *
 *     const int c_registered
 *        = utilib::Serializer().register_serializer<C>("C", serialize_C);
 *  } // namespace (local)
 *  \endcode
 *
 *  Note that this entire code fragment appears in a \em source (and not
 *  header) file.  The one "gotcha" is that if this registration is
 *  being done as part of a static library (i.e. \c *.a), it will only
 *  be picked up by the linker if something directly referenced by \c
 *  main() in turn references something in the source in which you
 *  placed the registration code.
 *
 *  \par 4) A template class that you do \em not control
 *
 *  Serializing template classes controlled by others are perhaps the
 *  most complicated example.  While individual serializations for
 *  specific template instantiations may be handled as in case (3), the
 *  general case that provides serialization for \em any instantiation
 *  of the template is more involved.  A prime example of this is the
 *  STL, for which we provide general serialization support at the
 *  bottom of this file.  In this case, the general pattern is to make
 *  use of the serial_transform template family.  In a header (included
 *  before any and all places where the template class is serialized),
 *  create the following:
 *
 *  \code
 *  // Given (defined elsewhere):
 *  template<typename T2, typename T2> class D;
 *
 *  template<typename T1, typename T2>
 *  int serialize_D( utilib::SerialObject::elementList_t & serial,
 *                   utilib::Any & data, bool serialize )
 *  {
 *     D& me = const_cast<D&>(data.expose<D>());
 *     // ... (de)serialize through D's public API ...
 *     return 0;
 *  }
 *
 *  template<typename T1, typename T2>
 *  struct D_registrar {
 *     static const volatile int registered;
 *  };
 *  template<typename T1, typename T2>
 *  const volatile int D_registrar<T1,T2>::registered 
 *     = Serializer().register_serializer< D<T1,T2> >
 *     ( std::string("D;") + mangledName(typeid(T1)) + ";" 
 *       + mangledName(typeid(T2)), serialize_D<T1,T2> );
 *
 *  namespace utilib {
 *     template<typename T1, typename T2>
 *     int serial_transform( SerialObject::elementList_t& serial, 
 *                           D<T1,T2>& data,  bool serialize )
 *     {
 *        (void)&D_registrar<T1,T2>::registered;
 *        AnyFixedRef tmp = data;
 *        return Serializer().transform_impl( typeid(data), serial,
 *                                            tmp, serialize );
 *     }
 *  } // namespace (local)
 *  \endcode
 *
 *  Note that unlike cases (1) and (2), where simply \em using the class
 *  somewhere in your program will cause the serialization function to
 *  be automatically registered with the Serializer(), in this case, the
 *  serializer will only be registered is the class is directly or
 *  indirectly explicitly serialized somewhere in the program.  Indirect
 *  references include other classes that serialize the class in
 *  question as part of \em their serializer.  For example, \c
 *  std::list<double> is indirectly referenced by the serializer for \c
 *  A above, and serializing \c std::map<int,std::list<int>> will
 *  automatically register the \c std::list<int> serializer.
 *
 *  If it is necessary to \em guarantee that a particular template
 *  instantiation is registered, simply reference it in a source file,
 *  as was done in case (3):
 *
 *  \code
 *  namespace {
 *     const int force_registration_1 = D_registrar<T1,T2>::registered;
 *  } // namespace (local)
 *  \endcode
 *
 *  \par Using the Serializer() before \c main()
 *
 *  As with all computations that occur before entry into \c main(),
 *  particular care must be taken to make sure that static data is not
 *  referenced before it is initialized.  As a general rule, <b><i>the
 *  Serializer() should not be used before entry into main()</i></b>.
 *  If it is absolutely necessary, there are two approaches that may
 *  work, although both rely on creating (and including)
 *  serial_transform() definitions for <b>\em all</b> classes that will be
 *  serialized.  The first approach is to construct the
 *  serial_transform() template similar to case (4):
 *
 *  \code
 *  template<typename T1, typename T2>
 *  struct D_registrar {
 *     static const volatile int registered;
 *     const int _register() {
 *        return Serializer().register_serializer<D<T1,T2> >
 *           ( std::string("D;") + mangledName(typeid(T1)) + ";" 
 *             + mangledName(typeid(T2)), serialize_D<T1,T2> );
 *     }
 *  };
 *  template<typename T1, typename T2>
 *  const volatile int 
 *  D_registrar<T1,T2>::registered = D_registrar<T1,T2>::_register()
 *
 *  namespace utilib {
 *     template<typename T1, typename T2>
 *     int serial_transform( SerialObject::elementList_t& serial, 
 *                           std::list<T1,T2>& data,  bool serialize )
 *     {
 *        D_registrar<T1,T2>::registered || D_registrar<T1,T2>::_register();
 *        AnyFixedRef tmp = data;
 *        return Serializer().transform_impl( typeid(data), serial,
 *                                            tmp, serialize );
 *     }
 *  } // namespace (local)
 *  \endcode
 *
 *  While this approach has been observed to be successful, it relies on
 *  the uninitialized \c registered variable to default to \c false.
 *  While in some instances this may be accuralte, in general it is not
 *  safe to rely on uninitialized values.  A more reliable alternative
 *  is to provide another level of indirection:
 *
 *  \code
 *  template<typename T1, typename T2>
 *  struct D_registrar {
 *     static const volatile int auto_register;
 *     const int registered() {
 *        static const int ans = Serializer().register_serializer<D<T1,T2> >
 *           ( std::string("D;") + mangledName(typeid(T1)) + ";" 
 *             + mangledName(typeid(T2)), serialize_D<T1,T2> );
 *        return ans;
 *     }
 *  };
 *  template<typename T1, typename T2>
 *  const volatile int 
 *  D_registrar<T1,T2>::auto_register = D_registrar<T1,T2>::registered()
 *
 *  namespace utilib {
 *     template<typename T1, typename T2>
 *     int serial_transform( SerialObject::elementList_t& serial, 
 *                           std::list<T1,T2>& data,  bool serialize )
 *     {
 *        (void)&D_registrar<T1,T2>::auto_register;
 *        D_registrar<T1,T2>::registered();
 *        AnyFixedRef tmp = data;
 *        return Serializer().transform_impl( typeid(data), serial,
 *                                            tmp, serialize );
 *     }
 *  } // namespace (local)
 *  \endcode
 *
 *  While this solution is more robust, it imposes the additional
 *  overhead of a function call for \em every serialization.  The
 *  preferred mode is to simply not perform any serialization actions
 *  until after entry into \c main().
 *
 *  \par Providing a user-defined name
 *
 *  The user-defined class name provides the Serializer with a unique
 *  compiler-independent "human-readable" form of the class name.
 *  Internally the Serializer relies strictly on the type_info structure
 *  and the mangled type name returned by utilib::mangledName() [\b NB:
 *  ALWAYS use \c utilib::mangledName() and \em NOT \c
 *  std::type_info::name() because Microsoft's Visual C++ implementation
 *  of \c std::type_info::name() does NOT return the mangled name].
 *  However, C++ mangling rules are compiler-dependent and keying off
 *  them would restrict serialized data from a program to only be
 *  readable by other programs (or instances of the same program)
 *  compiled by the same compiler.  To alleviate this, we build a
 *  secondary user-specified naming system to use for storing data in a
 *  more generic format.
 *
 *  A general rule for generating a user-defined name is to use the
 *  human-readable class name, \b including any namespaces.
 *
 *  While the process of giving unique names for standard classes is
 *  relatively straightforward, doing so for each instantiation of a
 *  template class is tedious at best.  To relieve part of this burden,
 *  the Serializer contains explicit support for recursively resolving
 *  the registered names of template parameters for you.  To register a
 *  serializer for a template class, the "user-defined" name is
 *  typically the name of your class, followed by the \em mangled type
 *  names for each of the template arguments (all fields separated by
 *  semicolons).  For example, given a class B templated on two types,
 *  the registration for the instance <tt>B<int,std::vector<double>
 *  ></tt> would be:
 *
 *  \code
 *  template<typename T1, typename T2>
 *  class B
 *  {
 *     // ...
 *  private:
 *     static int serializer( utilib::SerialObject::elementList_t & serial,
 *                            utilib::Any & data, bool serialize )
 *     static const volatile int serializer_registered;
 *  };
 *
 *  template<>
 *  const volatile int B<int, std::vector<double> >::serializer_registered
 *     = Serializer().register_serializer<B<int, std::vector<double> > >
 *        ( std::string("B;") + utilib::mangledName(typeid(int)) + ";"
 *            + utilib::mangledName(typeid(std::vector<double>)), 
 *          B<int, std::vector<double> >::serializer );
 *  \endcode
 *
 *  While awkward for single instantiations, this approach allows the
 *  registration itself to be templitized, thus supporting "automatic"
 *  registration (see the registration discuaaion above, ::Serialize(),
 *  and the STL_SerialRegistrars and STL_Serializers namespaces):
 *
 *  \code
 *  template<typename T1, typename T2>
 *  const volatile int B<T1,T2>::serializer_registered
 *     = Serializer().register_serializer<B<T1,T2> >
 *        ( std::string("B;") + utilib::mangledName(typeid(T1)) + ";" 
 *            + utilib::mangledName(typeid(T2)), 
 *          B<T1,T2>::serializer );
 *  \endcode
 */
class Serialization_Manager
{
public:
   /// A flag indicating the endianness of this platform
   static const unsigned char Endian;

   /// Constructor - registers standard C++ POD types
   Serialization_Manager();
   /// Destructor
   ~Serialization_Manager(); 

   /// Retrieve the typeinfo for the specified type key
   const std::type_info* get_typeinfo(size_t key);

   /// Retrieve the user-defined name for the specified type key
   std::string get_username(size_t key);

   /// Return true if the key is a POD
   bool is_pod(size_t key);

   /// Return true if the key is a POD
   int get_pod_length(size_t key);

   /// Retrieve the type key for the specified user-defined name
   size_t get_keyid(std::string user_name);

   /// Explicit alias for get_keyid(string) to avoid ambiguity under Solaris
   size_t get_keyid(const char* user_name)
   { return get_keyid(std::string(user_name)); }

   /// Retrieve the type key for the specified typeinfo
   size_t get_keyid(const std::type_info &type);

   /// Set whether POD transformations are into text format instead of binary
   void set_pod_text_mode(bool pod_text_mode)
   { serialize_pod_as_text = pod_text_mode; }

   /// Perform the serialize/deserialize transformation
   int transform_impl(const std::type_info& type,
                      SerialObject::elementList_t& serial, 
                      AnyFixedRef& data,
                      bool serialize);

   /// Register a new serialization function (use the default initializer)
   template<typename T>
   int register_serializer(std::string name, transform_fcn fcn)
   { 
      return register_serializer<T>( name, fcn, &DefaultInitializer<T> ); 
   }
   
   /// Register a new serialization function and the specified initializer
   template<typename T>
   int register_serializer( std::string name, 
                            transform_fcn fcn, initialization_fcn init )
   {
      return register_serializer( typeid(T), name, -1, fcn, NULL, NULL, init );
   }

   /// Register a new POD serialization function (use the default initializer)
   template<typename T>
   int register_serializer(std::string name, pod_transform_fcn fcn,
                           pod_text_transform_fcn tfcn = NULL, 
                           int size = static_cast<int>(sizeof(T)) )
   { 
      return register_serializer<T>( name, fcn, tfcn, size, 
                                     &DefaultInitializer<T> ); 
   }

   /// Register a new POD serialization function and the specified initializer
   template<typename T>
   int register_serializer( std::string name, pod_transform_fcn fcn, 
                            pod_text_transform_fcn tfcn, int size,
                            initialization_fcn init )
   {
      return register_serializer(typeid(T), name, size, NULL, fcn, tfcn, init);
   }


   /// Debugging function: list all known transformation functions
   void list_serializers(std::ostream& os);

public:  // sub-classes

   /// The standard default initializer: use the class default constructor
   template<typename T>
   static void DefaultInitializer(Any& data)
   {
      data.set<T>();
   }

private:  // sub-classes
   typedef std::pair<std::string, size_t>                    typename_pair_t;
   typedef std::map<std::string, size_t>                     typename_map_t;
   typedef std::pair<const std::type_info*, size_t>          type_pair_t;
   typedef std::map<const std::type_info*, size_t>           type_map_t;
   typedef std::pair<std::string, typename_map_t::iterator>  username_pair_t;
   typedef std::map<std::string, typename_map_t::iterator>   username_map_t;

   /// Container for holding serializer information about a type
   struct MappingFunctions {
      initialization_fcn  init;          ///< initialization: return new object
      transform_fcn       transform;     ///< serialize to/from SerialObject
      pod_transform_fcn   pod_transform; ///< serialize to/from SerialPOD
      pod_text_transform_fcn    pod_txt_xform; ///< serialize POD to/from text
      const std::type_info*  typeinfo;   ///< The main type_info for this type
      std::string               raw_user_name; ///< Name specified by user
      int                       pod_size;      ///< Data size (-1==variable)
      username_map_t::iterator  username;      ///< Ref.s to resolved user name
   };

private:  // methods
   /// Actual method to register a new serializer with the system
   int register_serializer( const std::type_info& type, 
                            std::string name, 
                            int pod_size,
                            transform_fcn t_fcn, 
                            pod_transform_fcn p_fcn, 
                            pod_text_transform_fcn pt_fcn, 
                            initialization_fcn i_fcn);

   /// (Utility method) Rebuild the user-defined type name map
   void rebuild_user_name_map();

   /// (Utility method) Recursively resolve any templated types for a user name
   std::string resolve_user_name(std::string mangled);

private:  // data
   /// Map mangled names to the index in the functions vector
   typename_map_t                 typename_map;
   /// Map user name to mangled name (actually, iterator into typename_map)
   username_map_t                 username_map;
   /// Map type_info to the index in the functions vector
   type_map_t                     type_map;
   /// The vector of all serialization information
   std::vector<MappingFunctions>  functions;
   /// If true, username_map is out-of-date and must be rebuilt
   bool                           rebuild_usernames;
   /// If true, transform_impl will serialize PODs using pod_text_transform_fcn
   bool                           serialize_pod_as_text;
};


//-----------------------------------------------------------------------
// Declare all STL-based serializers (implemented below)
//
namespace STL_Serializers {

template<typename T>
int pair_serializer( SerialObject::elementList_t& serial, 
                     Any& data, bool serialize );

template<typename T>
int sequence_serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize );

template<typename T>
int adaptor_serializer( SerialObject::elementList_t& serial, 
                        Any& data, bool serialize );

template<typename T>
int set_serializer( SerialObject::elementList_t& serial, 
                    Any& data, bool serialize );

template<typename T>
int map_serializer( SerialObject::elementList_t& serial, 
                    Any& data, bool serialize );

} // namespace STL_Serializers



//-----------------------------------------------------------------------
/** All serialization registrars for standard STL classes.  This set of
 *  templated namespace-level constants assists in the automagic
 *  registration of STL template serializers.  The key trick here is to
 *  get the \b instantiation of the wrapper function (see
 *  serial_transform() below) to cause the registration of the
 *  corresponding serializer \b BEFORE Serialize() / Deserialize() is
 *  ever called.  If we just took care of the registration through a
 *  static variable within the serialization wrapper function, the
 *  serializer would not get registered until the first call to the
 *  serialization wrapper function.  The problem with that is that the
 *  program may attempt to DEserialize that type (i.e. loading from a
 *  file) BEFORE ever serializing the type.  Thus, it is critical that
 *  the registration happens early, preferably before entry into main().
 *  These templates take care of that.
 *
 *  Note that we are using \b both a static volatile variable
 *  initialization and a registration function.  The static volatile
 *  initialization causes the serializer registration to occur before
 *  entry into main().  The definition of the _register() function is
 *  used below in the serial_transform() template to guard against other
 *  classes using the serializer before entry into main().  If another
 *  class (see the BinarySerialStream unit tests) calls the serializer
 *  during static construction, the static volatile registration flag
 *  may not yet be initialized.  By testing the flag, and if false,
 *  explicitly calling the _register() function, automagic registration
 *  should still correctly occur.
 */
namespace STL_SerialRegistrars {

template<typename T1, typename T2>
struct pair_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int pair_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::pair<T1,T2> >
   ( std::string("std::pair;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::pair_serializer<std::pair<T1,T2> > );



template<typename T1, typename T2>
struct list_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int list_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::list<T1,T2> >
   ( std::string("std::list;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::sequence_serializer<std::list<T1,T2> > );

template<typename T1, typename T2>
struct vector_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int vector_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::vector<T1,T2> >
   ( std::string("std::vector;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::sequence_serializer<std::vector<T1,T2> > );

template<typename T1, typename T2>
struct deque_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int deque_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::deque<T1,T2> >
   ( std::string("std::deque;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::sequence_serializer<std::deque<T1,T2> > );



template<typename T1, typename T2>
struct queue_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int queue_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::queue<T1,T2> >
   ( std::string("std::queue;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::adaptor_serializer<std::queue<T1,T2> > );

template<typename T1, typename T2>
struct stack_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2>
const volatile int stack_registrar<T1,T2>::registered = Serializer()
   .template register_serializer<std::stack<T1,T2> >
   ( std::string("std::stack;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)),
     STL_Serializers::adaptor_serializer<std::stack<T1,T2> > );

template<typename T1, typename T2, typename T3>
struct priority_queue_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2, typename T3>
const volatile int priority_queue_registrar<T1,T2,T3>::registered =Serializer()
   .template register_serializer<std::priority_queue<T1,T2,T3> >
   ( std::string("std::priority_queue;") + mangledName(typeid(T1)) + ";" 
     + mangledName(typeid(T2)) + ";" + mangledName(typeid(T3)),
     STL_Serializers::adaptor_serializer<std::priority_queue<T1,T2,T3> > );



template<typename T1, typename T2, typename T3>
struct set_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2, typename T3>
const volatile int set_registrar<T1,T2,T3>::registered = Serializer()
   .template register_serializer<std::set<T1,T2,T3> >
   ( std::string("std::set;") + mangledName(typeid(T1)) + ";" 
     + mangledName(typeid(T2)) + ";" + mangledName(typeid(T3)),
     STL_Serializers::set_serializer<std::set<T1,T2,T3> > );

template<typename T1, typename T2, typename T3>
struct multiset_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2, typename T3>
const volatile int multiset_registrar<T1,T2,T3>::registered = Serializer()
   .template register_serializer<std::multiset<T1,T2,T3> >
   ( std::string("std::multiset;") + mangledName(typeid(T1)) + ";" 
     + mangledName(typeid(T2)) + ";" + mangledName(typeid(T3)),
     STL_Serializers::set_serializer<std::multiset<T1,T2,T3> > );



template<typename T1, typename T2, typename T3, typename T4>
struct map_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2, typename T3, typename T4>
const volatile int map_registrar<T1,T2,T3,T4>::registered = Serializer()
   .template register_serializer<std::map<T1,T2,T3,T4> >
   ( std::string("std::map;") 
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)) 
     + ";" + mangledName(typeid(T3)) + ";" + mangledName(typeid(T4)),
     STL_Serializers::map_serializer<std::map<T1,T2,T3,T4> > );

template<typename T1, typename T2, typename T3, typename T4>
struct multimap_registrar {
   static const volatile int registered;
};
template<typename T1, typename T2, typename T3, typename T4>
const volatile int multimap_registrar<T1,T2,T3,T4>::registered = Serializer()
   .template register_serializer<std::multimap<T1,T2,T3,T4> >
   ( std::string("std::multimap;")
     + mangledName(typeid(T1)) + ";" + mangledName(typeid(T2)) + ";" 
     + mangledName(typeid(T3)) + ";" + mangledName(typeid(T4)),
     STL_Serializers::map_serializer<std::multimap<T1,T2,T3,T4> > );

} // namespace STL_SerialRegistrars



//-----------------------------------------------------------------------
//

/** \param[in,out] serial  A SerialObject list
 *  \param[in,out] data    A reference to the variable to serialize
 *  \param[in] serialize   Indicates whether to serialize or deserialize
 *  \return 0 on success, <>0 on error.
 *
 *  serial_transform() is a "convenience" wrapper function around the
 *  core Serialization_Manager::transform_impl() function.  To provide
 *  implicit support for template classes (including the STL), we need
 *  to assure that the serialization for a particular template
 *  instantiation is registered before the first call to
 *  Serialization_Manager::transform_impl().  The templated overloads of
 *  this namespace-level wrapper function guarantee that the
 *  registration takes place.
 *
 *  This specific function is designed to correctly deal with
 *  serializing and deserializing Any variables (with special care to
 *  correctly process empty Any data).
 *
 *  Each call to serial_transform() with \a serialize == true will
 *  serialize the variable referenced by \a data into a SerialObject and
 *  push the resulting object onto the end of the \a serial list.  Each
 *  call to serial_transform with \a serialize == false will remove the
 *  SerialObject from the front of the \a serial list and deserialize it
 *  into the Any \a data object.
 */
int serial_transform(SerialObject::elementList_t& serial, 
                     Any& data,  bool serialize);

template<typename T>
int serial_transform(SerialObject::elementList_t& serial, 
                     T& data,  bool serialize)
{
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::pair<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::pair_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::list<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::list_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::vector<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::vector_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::deque<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::deque_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::queue<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::queue_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::stack<T1,T2>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::stack_registrar<T1,T2>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2, typename T3>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::priority_queue<T1,T2,T3>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::priority_queue_registrar<T1,T2,T3>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2, typename T3>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::set<T1,T2,T3>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::set_registrar<T1,T2,T3>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2, typename T3>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::multiset<T1,T2,T3>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::multiset_registrar<T1,T2,T3>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2, typename T3, typename T4>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::map<T1,T2,T3,T4>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::map_registrar<T1,T2,T3,T4>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}

template<typename T1, typename T2, typename T3, typename T4>
int serial_transform(SerialObject::elementList_t& serial, 
                     std::multimap<T1,T2,T3,T4>& data,  bool serialize)
{
   (void)&STL_SerialRegistrars::multimap_registrar<T1,T2,T3,T4>::registered;
   AnyFixedRef tmp = data;
   return Serializer().transform_impl(typeid(data), serial, tmp, serialize);
}


/** \param[in] data  A reference to the variable to serialize
 *  \param[in] pod_text_mode Indicate if POD data should be serialized
 *     as a binary vector or a text-based vector (see utilib::XMLSerialStream).
 *  \return The serialized object (as a SerialObject)
 *
 *  This will take the object referenced by \a data and serialize it,
 *  returning the resulting SerialObject.  This function is a
 *  convenience wrapper around serial_transform().
 */
template<typename T>
SerialObject Serialize(const T& data, bool pod_text_mode = false)
{
   Serializer().set_pod_text_mode(pod_text_mode);

   SerialObject::elementList_t tmp;
   int ans = serial_transform(tmp, const_cast<T&>(data), true);
   if ( ans != 0 )
   {
      EXCEPTION_MNGR(std::runtime_error, "Serialize(): "
                     "Serialization failed for '"
                     << typeid(T).name() << "' (Error " << ans << ")");
   }
   if ( tmp.empty() || ( ++(tmp.begin()) != tmp.end() ) )
   {
      EXCEPTION_MNGR(std::runtime_error, "Serialize(): "
                     "[internal error] returned invalid object count for '"
                     << typeid(T).name() << "'");
   }
   return tmp.front();
}

/** \param[in] serial  A serialized object
 *  \return The expanded object specified by \a serial contained in an Any
 *
 *  This will deserialize any SerialObject and return the expanded
 *  object.  This function is a convenience wrapper around
 *  serial_transform().
 */
Any Deserialize(SerialObject &serial);


//-----------------------------------------------------------------------
/** All serialization functions for standard STL classes.
 */
namespace STL_Serializers {

template<typename T>
int pair_serializer( SerialObject::elementList_t& serial, 
                     Any& data, bool serialize )
{
   T& tmp = const_cast<T&>(data.template expose<T>());
   int ans = serial_transform(serial, tmp.first, serialize);
   if ( ans != 0 )
      return ans;
   return serial_transform(serial, tmp.second, serialize);
}


template<typename T>
int sequence_serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize )
{
   T& tmp = const_cast<T&>(data.template expose<T>());
   if ( ! serialize )
      tmp.resize(serial.size());

   int ans = 0;
   typename T::iterator it = tmp.begin();
   typename T::iterator itEnd = tmp.end();
   for ( ; it != itEnd; ++it )
      if ( 0 != (ans = serial_transform(serial, *it, serialize)) )
         return ans;

   return 0;
}

/// Helper class to gain access to protected sequence container in adaptor
template<typename T>
class ModifiedSequenceAdaptor : public T
{
public:
   int sub_transform(SerialObject::elementList_t& serial, bool serialize)
   {
      return serial_transform(serial, T::c, serialize);
   }
};

template<typename T>
int adaptor_serializer( SerialObject::elementList_t& serial, 
                         Any& data, bool serialize )
{
   T& tmp = const_cast<T&>(data.template expose<T>());
   return reinterpret_cast<ModifiedSequenceAdaptor<T>&>(tmp)
      .sub_transform(serial, serialize);
}


template<typename T>
int set_serializer( SerialObject::elementList_t& serial, 
                    Any& data, bool serialize )
{
   int ans = 0;
   T& tmp = const_cast<T&>(data.template expose<T>());
   if ( serialize )
   {      
      typename T::iterator it = tmp.begin();
      typename T::iterator itEnd = tmp.end();
      for ( ; it != itEnd; ++it )
         if ( 0 != (ans = serial_transform(serial, *it, serialize)) )
            return ans;
   }
   else
   {
      while ( ! serial.empty() )
      {
         AnyFixedRef i;
         ans = Serializer().transform_impl(typeid(void), serial, i, serialize);
         if ( ans != 0 )
            return ans;
         tmp.insert(tmp.end(), i.template expose<typename T::value_type>());
      }
   }

   return 0;
}

template<typename T>
int map_serializer( SerialObject::elementList_t& serial, 
                    Any& data, bool serialize )
{
   // Ideally, we would use the set_serializer above for all associative
   // containers.  Unfortunately, the value_type for maps is
   // std::pair<const T1, T2>.  As a result, we need to reinterpret it
   // as the non-const pair<T1,T2> BEFORE we stick it into an Any
   // (because the default "const T1" is not assignable and thus will
   // not compile).
   //
   // NB: This is further complicated because GCC deviates from the SGI
   // standard: SGI publishes T2 as map::data_type, while GCC uses
   // map::mapped_type.  To get around this, we use
   // map::value_type::second_type (i.e. get T2 from the underlying
   // pair), which hopefully will be more portable...
   typedef std::pair<typename T::key_type, typename T::value_type::second_type>
      nonConstPair_t;

   int ans = 0;
   T& tmp = const_cast<T&>(data.template expose<T>());
   if ( serialize )
   {      
      typename T::iterator it = tmp.begin();
      typename T::iterator itEnd = tmp.end();
      for ( ; it != itEnd; ++it )
      {
         nonConstPair_t &element = reinterpret_cast<nonConstPair_t&>(*it);
         if ( 0 != (ans = serial_transform(serial, element, serialize)) )
            return ans;
      }
   }
   else
   {
      while ( ! serial.empty() )
      {
         AnyFixedRef i;
         ans = Serializer().transform_impl(typeid(void), serial, i, serialize);
         if ( ans != 0 )
            return ans;
         tmp.insert( tmp.end(), i.template expose<nonConstPair_t>() );
      }
   }

   return 0;
}

} // namespace STL_Serializers

} // namespace utilib

#endif // utilib_Serialize_h
