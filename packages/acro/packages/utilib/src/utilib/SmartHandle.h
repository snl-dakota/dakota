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

#ifndef utilib_SmartHandle_h
#define utilib_SmartHandle_h

#if 0
#include <utilib/SmartPtr.h>

namespace utilib {

template <class Type>
class SmartHandle
{
public:

  SmartHandle(bool initialize=false)
    {
    if (initialize)
        data = new Type();
    }

  SmartHandle(const SmartHandle<Type>& other)
        {data = other.data;}

  ~SmartHandle() {}

  SmartHandle<Type>& operator=(const SmartHandle<Type>& other)
        {data = other.data; return *this;}

  bool operator==(const SmartHandle<Type>& other) const
        {return *data == *(other.data);}

  bool operator<(const SmartHandle<Type>& other) const
        {return *data < *(other.data);}

  Type& operator()()
        {return *data;}

  const Type& operator()() const
        {return *data;}

protected:

  SmartPtr<Type> data;
};

}

#else

#include <utilib/RefCount.h>

namespace utilib {

template <class Type>
class SmartHandle
{
public:

  SmartHandle(bool initialize=false)
    : ref(0)
    {
    if (initialize)
        ref = new RefCount<Type>(new Type(),AssumeOwnership);
    }

  SmartHandle(const SmartHandle<Type>& other)
        : ref(0)
        {*this = other;}

  ~SmartHandle() {if (ref && ref->decrement()) delete ref;}

  SmartHandle<Type>& operator=(const SmartHandle<Type>& other)
        {
        if (ref && ref->decrement()) delete ref;
        ref = other.ref; 
        if (ref)
           ref->increment();
        return *this;
        }

  bool operator==(const SmartHandle<Type>& other) const
        {return *(ref->data()) == *(other.ref->data());}

  bool operator<(const SmartHandle<Type>& other) const
        {return *(ref->data()) < *(other.ref->data());}

  Type& operator()()
        {return *(ref->data());}

  const Type& operator()() const
        {return *(ref->data());}

protected:

  RefCount<Type>* ref;
};

}

#endif

#endif
