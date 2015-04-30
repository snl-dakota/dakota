/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file Handle.h
 *
 * Wrapper for managing and reference-counting COLIN objects.
 */

#ifndef colin_Handle_h
#define colin_Handle_h

#include <utilib/Any.h>

namespace colin {

template<typename TYPE> class Handle;
template<typename TYPE> class Handle_Data;

/// The base class for any object that can be put inside a colin::Handle.
/** The base class for any object that can be put inside a
 *  colin::Handle.  It provides a new get_handle() public method as well
 *  as private methods used by the colin::Handle system to maintain
 *  bidirectional reference management.
 *
 *  See colin::Handle for more information.
 */
template<typename TYPE>
class Handle_Client
{
   friend class Handle_Data<TYPE>;

public:
   Handle_Client()
      : self_handle(NULL), handle_references()
   {}

   virtual ~Handle_Client()
   {
      // Notify any remaining Haandles that the referenced object (ME!)
      // is no longer in scope.
      typename std::set<Handle_Data<TYPE>*>::iterator it
         = handle_references.begin();
      typename std::set<Handle_Data<TYPE>*>::iterator end
         = handle_references.end();
      for( ; it != end; ++it )
         (*it)->handle_client = NULL;
   }

   /// Get a new handle for this object.
   /** Return a new colin::Handle for this object.  If this object is
    *  currently owned by a Handle (i.e. it was created through
    *  colin::Handle::create()), then the returned Handle will be a
    *  reference to the same Handle.  If the object was created outside
    *  the Handle, then this will return a new \e reference \e handle to
    *  this object.
    */
   Handle<TYPE> get_handle() const
   {
      // Why is this safe?
      //
      // Because the self_handle Handle_Data contains an Any that in
      // turn contains THIS OBJECT, the pointer to the self_handle is
      // valid as long as *this* object is -- that is, this object will
      // not be deleted by the self_handle until the last instance of
      // the self_handle falls out of scope (this is why we only hold a
      // pointer to the handle and not the handle itself -- holding the
      // handle would create a circular reference and the object would
      // never be deleted).
      //
      // Note: creating a Handle with a reference to an existing object
      // will NOT set that object's self_handle.
      if ( self_handle == NULL )
      {
         TYPE* me = static_cast<TYPE*>(const_cast<Handle_Client<TYPE>*>(this));
         // Creating a reference-counted object manually 
         Handle_Data<TYPE> *tmp
            = new Handle_Data<TYPE>(me, utilib::Any(me, false, true));
         // Decrement *my* reference to the object
         --(tmp->refCount);
         // (Handle will increment the reference count, so things are good)
         return Handle<TYPE>(tmp);
      }
      else
         return Handle<TYPE>(self_handle);
   }


private: // methods (called by Handle_Data)

   /// Give this object a Handle to itself (only called via Handle::create())
   void set_self_handle(Handle_Data<TYPE> *handle)
   {
      if ( handle != NULL )
      {
         if ( self_handle != NULL )
            EXCEPTION_MNGR(std::runtime_error, "Handle_Client<"
                           << utilib::demangledName(typeid(TYPE)) << ">::"
                           "set_self_handle(): self handle already set.");
         if ( handle->handle_client != this )
            EXCEPTION_MNGR(std::runtime_error, "Handle_Client<"
                           << utilib::demangledName(typeid(TYPE)) << ">::"
                           "set_self_handle(): handle refers to a "
                           "different object!");
      }
      self_handle = handle;
   }

   /// Mark the creation/destruction of a reference handle to this object
   void set_handle_reference(Handle_Data<TYPE> *reference, bool incoming)
   {
      if ( incoming )
         handle_references.insert(reference);
      else
         handle_references.erase(reference);
   }


private: // data

   /// A pointer to the Handle_Data containing this object (can be NULL)
   Handle_Data<TYPE> *self_handle;

   /// Set of reference handles pointing to this object
   std::set<Handle_Data<TYPE>*> handle_references;
};


/// The reference-counted object held by a colin::Handle
/** This is the reference-counted object held by a colin::Handle object
 *  and referenced by a Handle_Client object.  It has three members: the
 *  referece count, a pointer to the "base class" of the held object,
 *  and an Any that (may) contain the object.  
 *
 *  If the object was created through colin::Handle::create(), then the
 *  actual object will be held within the (immutable) Any.  Otherwise,
 *  the Any contains a pointer to the "base class" of the actual object.
 *  The latter case allows for \e reference \e handles -- that is,
 *  getting a handle to an object created programmatically outside the
 *  Handle system.
 *
 *  \b NOTE: We use the immutability flag on the Any object to indicate
 *  if the Any contains the actual object or a pointer to the object.
 */
template<typename TYPE>
class Handle_Data
{
   friend class Handle<TYPE>;
   friend class Handle_Client<TYPE>;

   ///
   Handle_Data(TYPE* handle_client_, utilib::Any handle_)
      : refCount(1), handle_client(handle_client_), handle(handle_)
   {
      assert(( handle_client == NULL ) == handle.empty() );
      if ( handle.is_immutable() )
         handle_client->set_handle_reference(this, true);
      else
         handle_client->set_self_handle(this);
   }

   ///
   ~Handle_Data()
   {
      if (( handle_client != NULL ) && ( handle.is_immutable() ))
         handle_client->set_handle_reference(this, false);
   }

   /// Number of active references to this Handle_Data object
   size_t       refCount;
   /// Pointer to the "base class" this Handle_Data holds
   TYPE *       handle_client;
   /// The Any that (may) contain the held object
   utilib::Any  handle;
};


/// A reference-counted Handle to a "handleable" object.
/** The colin::Handle system provides yet another variant on the concept
 *  of a "Smart Pointer."  In its most basic sense, the Handle behaves
 *  like the utilib::Any -- it keeps track of the number of instances of
 *  the Handle that exist, and when the last one falls out of scope,
 *  "held" object is deleted.  The motivation for creating this object
 *  is that we cannot always guarantee the manner in which the object
 *  was originally created.
 *
 *  For typical referenced-counted containers (like the utilib::Any), if
 *  the container is going to destroy the contained object at
 *  end-of-life, the object must be "owned" by the container (usually by
 *  being constructed within it).  For objects created systemmatically
 *  through constructs like factories, this is not an issue.  However,
 *  one of the major tenants in COLIN is that it should be equally easy
 *  to write simple driver programs that directly instantiate and
 *  manipulate concrete classes.  While we can get "reference handles"
 *  to pre-existing objects (see utilib::AnyRef), these can be dangerous
 *  for large, long-lived objects and handles where the handle may exist
 *  longer than the referenced object.  Dereferencing the handle after
 *  the object falls out of scope produces the always-helpful
 *  "Segmentation fault: core dumped" message.
 *
 *  The colin::Handle class system assists in this case by maintaining
 *  bidirectional references beween the handles and the held object so
 *  that the held object can notify any remaining reference handles that
 *  they are no longer valid.  That way, if they are dereferenced, they
 *  can produce a useful exception (and stack trace).  
 *
 *  \b Note: in order to create a handle<TYPE>, that TYPE \b must derive
 *  from colin::Handle_Client<TYPE>.
 */
template<typename TYPE>
class Handle
{
public:
   /// Create a empty handle
   Handle()
      : data(NULL)
   {}

   /// Create a new Handle that shares (references) data with another Handle
   Handle(Handle_Data<TYPE>* data_)
      : data(data_)
   {
      if ( data != NULL )
         ++(data->refCount);
   }

   /// Automatically convert an object pointer into its handle
   Handle(const Handle_Client<TYPE>* reference)
      : data(NULL)
   {
      if ( reference == NULL )
         *this = Handle<TYPE>();
      else
         *this = reference->get_handle();
   }

   /// Automatically convert an object into its handle
   Handle(const Handle_Client<TYPE>& reference)
      : data(NULL)
   {
      *this = reference.get_handle();
   }

   /// Standard copy constructor
   Handle(const Handle& rhs)
      : data(NULL)
   {
      *this = rhs;
   }

   /// Destructor that updates reference counting
   ~Handle()
   {
      if ( data != NULL )
         if ( --(data->refCount) == 0 )
            delete data;
   }

   /// Assignment: perform a shallow copy and update reference counts
   Handle& operator=(const Handle& rhs)
   {
      if ( data == rhs.data )
         return *this;

      if ( data != NULL )
         if ( --(data->refCount) == 0 )
            delete data;
      data = rhs.data;
      if ( data != NULL )
         ++(data->refCount);
      return *this;
   }


   /// Create a new Handle.  The new object will be contained within the Handle
   template<typename T>
   static std::pair<Handle<TYPE>, T*> create()
   {
      utilib::Any handle;
      T& new_object = handle.set<T, utilib::Any::NonCopyable<T> >();
      Handle<TYPE> ans;
      ans.data = new Handle_Data<TYPE>(&new_object, handle);
      return std::pair<Handle<TYPE>, T*>(ans, &new_object);
   }

   /// If true, this Handle holds no object
   bool empty() const
   { return data == NULL; }

   /// Expose the "base class" pointer to the held object
   TYPE* operator->() const
   { 
      if ( data == NULL )
         EXCEPTION_MNGR(std::runtime_error, "Handle::operator->(): "
                        "dereferencing empty object handle (type "
                        << utilib::demangledName(typeid(TYPE)) << ").");
      if ( data->handle_client == NULL )
         EXCEPTION_MNGR(std::runtime_error, "Handle::operator->(): "
                        "dereferencing Handle whose core object has "
                        "fallen out of scope (type "
                        << utilib::demangledName(typeid(TYPE)) << ").");
      return data->handle_client; 
   }

   /// Expose the "base class" pointer to the held object
   TYPE& operator*() const
   { 
      return *operator->();
   }

   /// Convenience: Expose the "base class" pointer to the held object
   TYPE* object() const
   {
      // Bypass the exception for empty objects and just return NULL.
      return empty() ? NULL : operator->();
   }

   /// Expose the raw Any that holds object (ot the pointer to the object)
   utilib::Any raw_object() const
   { 
      if ( data == NULL )
         return utilib::Any();
      if ( data->handle_client == NULL )
         EXCEPTION_MNGR(std::runtime_error, "Handle::raw_object(): "
                        "dereferencing Handle whose core object has "
                        "fallen out of scope (type "
                        << utilib::demangledName(typeid(TYPE)) << ").");
      return data->handle; 
   }

private:
   /// Pointer to the reference-counted data
   Handle_Data<TYPE>* data;
};


} // namespace colin

#endif // defined colin_Handle_h
