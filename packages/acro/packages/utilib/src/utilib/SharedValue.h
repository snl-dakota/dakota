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
 * \file SharedValue.h
 *
 * Defines the utilib::SharedValue class.
 *
 * This container manages a copy of an object with reference counts.
 * This enables shallow copying of the underyling object.
 */

#ifndef utilib_SharedValue_h
#define utilib_SharedValue_h

#include <utilib_config.h>
#include <utilib/exception_mngr.h>

#include <utilib/Serialize.h>

namespace utilib
{

/** This class stores an object by value.
 *
 */
template <class TYPE>
class SharedValue
{
public:

   template <class LTYPE> friend class SharedValue;

   /// Empty constructor
   SharedValue()
       :  m_data(0)
   {
      (void)serializer_registered;
   }

   /// Constructor that copies a value
   SharedValue(const TYPE& value)
   {
      m_data = new Container(value);
   }

   /// Copy constructor (creates a "shallow copy" of the contained cargo data)
   SharedValue(const SharedValue<TYPE>& rhs)
         : m_data(rhs.m_data)
   {
      if (m_data != NULL)
      { ++(m_data->refCount); }
   }

   /// Destructor
   virtual ~SharedValue()
   {
      if (m_data != NULL)
      {
         if (--(m_data->refCount) == 0)
         { delete m_data; }
      }
   }

   /// Copy one object to another with a shallow copy
   SharedValue<TYPE>& operator=(const SharedValue<TYPE>& rhs)
   {
      if ( m_data == rhs.m_data )
         return *this;

      if (m_data != NULL)
      {
         if (--(m_data->refCount) == 0)
         { delete m_data; }
      }

      m_data = rhs.m_data;
      if (m_data != NULL)
      { ++(m_data->refCount); }
      return *this;
   }

   /// Initialize the object with a new value
   SharedValue<TYPE>& operator=(const TYPE& value)
   {
      if (m_data != NULL)
      {
         if (--(m_data->refCount) == 0)
         { delete m_data; }
      }

      m_data = new Container(value);
      return *this;
   }

   /// Perform a deep copy one object to another
   SharedValue<TYPE>& operator<<(const SharedValue<TYPE>& rhs)
   {
      if (rhs.m_data == NULL) 
      {
      clear();
      return *this;
      }
      if (m_data == NULL)
      {
         m_data = new Container(rhs.m_data->value);
         return;
      }
      m_data->data = rhs.m_data->data;
      return *this;
   }

   /// Copy this value into the current data
   SharedValue<TYPE>& operator<<(const TYPE& value)
   {
      if (m_data == NULL)
      {
         m_data = new Container(value);
         return *this;
      }
      m_data->data = value;
      return *this;
   }

   ///
   void initialize()
    {
    if (!m_data)
           m_data = new Container();
    }

   ///
   TYPE* operator-> ()
   {
      if (m_data == NULL)
      { EXCEPTION_MNGR(std::runtime_error, "SharedValue::operator() - NULL data"); }
      return &(m_data->data);
   }

   ///
   const TYPE* operator-> () const
   {
      if (m_data == NULL)
      { EXCEPTION_MNGR(std::runtime_error, "SharedValue::operator() - NULL data"); }
      return &(static_cast<Container* >(m_data)->data);
   }

   ///
   operator TYPE&()
    {return this->operator()();}

   ///
   operator const TYPE&() const
    {return this->operator()();}

   ///
   TYPE& operator()()
   {
      if (m_data == NULL)
      { EXCEPTION_MNGR(std::runtime_error, "SharedValue::operator() - NULL data"); }
      return m_data->data;
   }

   ///
   const TYPE& operator()() const
   {
      if (m_data == NULL)
      { EXCEPTION_MNGR(std::runtime_error, "SharedValue::operator() - NULL data"); }
      return static_cast<Container* >(m_data)->data;
   }

   /// Clear the underlying data for this SharedValue object (empty() will become true)
   void clear()
   {
      if (m_data != NULL)
      {
         if (--(m_data->refCount) == 0)
         { delete m_data; }
         m_data = NULL;
      }
   }

   /// Returns true if the data stored in the rhs SharedValue is equal to this data
   template <class LTYPE>
   bool operator==(const SharedValue<LTYPE>& rhs) const
   {
      if (m_data == NULL)
      { return rhs.m_data == NULL; }
      if (rhs.m_data == NULL)
      { return false; }
      return m_data->data == rhs.m_data->data;
   }

   /// Returns true if the value equals the this data
   template <class LTYPE>
   bool operator==(const LTYPE& value) const
   {
      if (m_data == NULL)
         return false;
      return m_data->data == value;
   }

   /// Returns true if the data stored in this SharedValue is less than the rhs data
   template <class LTYPE>
   bool operator<(const SharedValue<LTYPE>& rhs) const
   {
      if (m_data == NULL)
      { return rhs.m_data != NULL; }
      if (rhs.m_data == NULL)
      { return false; }
      return m_data->data < rhs.m_data->data;
   }

   /// Returns true if the data stored in this SharedValue is less than the value
   template <class LTYPE>
   bool operator<(const LTYPE& value) const
   {
      if (m_data == NULL)
         return false;
      return m_data->data == value;
   }

   /// Returns true if the object value has not been set
   inline bool empty() const
   { return m_data == NULL; }

   /// Returns the total number of SharedValue instances that refer to this data
   inline unsigned int refCount() const
   { return m_data ? m_data->refCount : 0; }

protected:

   ///
   struct Container
   {
      ///
      Container()
      : refCount(1)
      {}

      ///
      Container(const TYPE& value)
      : refCount(1), data(value)
      {}

      ///
      unsigned int refCount;

      ///
      TYPE data;
   };

protected:

   ///
   Container *m_data;


private:
   static int serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize );

   static const volatile int serializer_registered;
};


//============================================================================

template<typename T>
const volatile int SharedValue<T>::serializer_registered = Serializer()
   .template register_serializer<SharedValue<T> >
    ( std::string("utilib::SharedValue;") + mangledName(typeid(T)),
      SharedValue<T>::serializer );

//============================================================================

/// Serialize a SharedValue object
template <class T>
int SharedValue<T>::serializer( SerialObject::elementList_t& serial, 
                                Any& data, bool serialize )
{
   SharedValue<T>& me 
      = const_cast<SharedValue<T>&>(data.template expose<SharedValue<T> >());

   Any tmp;
   if ( serialize )
   {
      // make sure the contained data is registered
      if ( me.m_data == NULL )
         serial_transform(serial, tmp, serialize);
      else
         serial_transform(serial, me.m_data->data, serialize);
   }
   else
   {
      me.clear();
      utilib::serial_transform(serial, tmp, serialize); 
      if ( tmp.empty() )
         me.m_data = NULL;
      else
         me.m_data = new Container(tmp.template expose<T>());
   }
   return 0;
}


}

#endif // utilib_SharedValue_h
