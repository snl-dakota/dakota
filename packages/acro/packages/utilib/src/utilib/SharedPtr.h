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
 * \file SharedPtr.h
 *
 * Defines the utilib::SharedPtr class.
 *
 * This container manages a copy of an object with reference counts.
 * This enables shallow copying of the underyling object.
 */

#ifndef utilib_SharedPtr_h
#define utilib_SharedPtr_h

#include <utilib_config.h>
#include <utilib/exception_mngr.h>

namespace utilib
{

/** This class stores a shared copy of a dynamically-allocated object.
 *
 */
template <class TYPE>
class SharedPtr
{
public:
   /// Empty constructor
   SharedPtr()
      :  m_data(0)
   {}

   /// Constructor that copies a ptr
   SharedPtr(TYPE* ptr)
   {
      m_data = new Container(ptr);
   }

   /// Copy constructor (creates a "shallow copy" of the contained cargo data)
   SharedPtr(const SharedPtr<TYPE>& rhs)
      : m_data(rhs.m_data)
   {
      if (m_data != NULL)
         ++(m_data->refCount);
   }

   /// Destructor
   virtual ~SharedPtr()
   {
      if (( m_data != NULL ) && ( --(m_data->refCount) == 0 ))
         delete m_data;
   }

   /// Copy one object to another with a shallow copy
   SharedPtr<TYPE>& operator=(const SharedPtr<TYPE>& rhs)
   {
      if ( m_data == rhs.m_data )
         return *this;

      if (( m_data != NULL ) && ( --(m_data->refCount) == 0 ))
         delete m_data;

      m_data = rhs.m_data;

      if (m_data != NULL)
         ++(m_data->refCount);
      return *this;
   }

   /// Initialize the object with a new ptr
   SharedPtr<TYPE>& operator=(TYPE* &ptr)
   {
      if (( m_data != NULL ) && ( --(m_data->refCount) == 0 ))
         delete m_data;
      
      m_data = new Container(ptr);
      return *this;
   }

   ///
   TYPE* operator->()
   {
      if ( m_data == NULL )
         EXCEPTION_MNGR(std::runtime_error, 
                        "SharedPtr::operator->(): NULL data");
      return m_data->data;
   }

   /// NB: this is a "const pointer" and not a "pointer to const"
   TYPE* operator->() const
   {
      if ( m_data == NULL )
         EXCEPTION_MNGR(std::runtime_error, 
                        "SharedPtr::operator->() - NULL data");
      return m_data->data;
   }

   ///
   TYPE& operator*()
   {
      if ( m_data == NULL )
         EXCEPTION_MNGR(std::runtime_error, 
                        "SharedPtr::operator->(): NULL data");
      return *m_data->data;
   }

   /// NB: this is a "const pointer" and not a "pointer to const"
   TYPE& operator*() const
   {
      if ( m_data == NULL )
         EXCEPTION_MNGR(std::runtime_error, 
                        "SharedPtr::operator->() - NULL data");
      return *m_data->data;
   }


   /// Clear the underlying data for this SharedPtr (empty() will become true)
   void clear()
   {
      if (( m_data != NULL ) && ( --(m_data->refCount) == 0 ))
         delete m_data;

      m_data = NULL;
   }


   /// Returns true if the object ptr has not been set
   inline bool empty() const
   { return m_data == NULL; }

   /// Returns the total number of SharedPtr instances that refer to this data
   inline unsigned int refCount() const
   { return m_data ? m_data->refCount : 0; }

protected:

   ///
   struct Container
   {
      ///
      Container(TYPE* &ptr)
      : refCount(1), data(ptr)
      {}

      ///
      unsigned int refCount;

      ///
      TYPE *data;
   };

protected:

   ///
   Container *m_data;

};


}

#endif // utilib_SharedPtr_h
