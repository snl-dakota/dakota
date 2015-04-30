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
 * \file Any.cpp
 *
 * Implements non-inline, non-template members for the UTILIB Any class.
 */

#include <utilib/Any.h>

namespace utilib {

Any& Any::to_value_any()
{ 
   if ( m_data == NULL )
   { }
   else if ( m_data->isReference() )
   {
      // make a local (value) copy of the referenced data
      ContainerBase *tmp = m_data;
      m_data = m_data->newValueContainer();
      if ( --(tmp->refCount) == 0 )
      { delete tmp; }
   }
   else if ( m_data->immutable )
   {
      // ONLY copy the contained data if someone else holds a reference to it
      if ( m_data->refCount > 1 )
      {
         --(m_data->refCount);
         m_data = m_data->newValueContainer();
      }
      m_data->immutable = false;
   }
   return *this;
}

/** This both indicates whether Any::extract() should throw
 *  exceptions for invalid casts, and allows the client to set the
 *  result of future throwCastExceptions() calls [as the bool value is
 *  returned by non-constant reference).
 *
 *  By default, throwCastExceptions() == true and Any::extract()
 *  will generate cast exceptions.
 */
bool& Any::throwCastExceptions() 
{ 
   static bool value = true; 
   return value;
}


std::string Any::describe(std::string name) const
{
   std::stringstream os;

   os << name << ": ";
   if ( empty() )
      { os << "[empty]"; }
   else
   { 
      os << demangledName(m_data->type());
      if ( is_immutable() )
      { os << " [immutable]"; }
      if ( is_reference() )
          { os << " [reference]"; }
   }

   return os.str();
}

/// Attempts to print out the contents of the Any to the ostream
void Any::write(std::ostream& os) const
{
   if ( m_data == NULL ) 
      os << "[Empty Any]";
   else
      m_data->print(os); 
}

/// Attempts to print out the contents of the Any to the ostream
void Any::read(std::istream& is)
{
   if ( m_data == NULL ) 
      EXCEPTION_MNGR(any_not_readable, 
                     "Any::read(): cannot read into an empty Any>");
   else
      m_data->read(is); 
}


/// Attempts to print out the contents of the Any to the ostream
void Any::write(utilib::PackBuffer& os) const
{ m_data->write(os); }

/// Attempts to print out the contents of the Any to the ostream
void Any::read(utilib::UnPackBuffer& is)
{
   if ( m_data == NULL ) 
      EXCEPTION_MNGR(any_not_packable, 
                     "Any::read(): cannot unpack into an empty Any>");
   else
      m_data->read(is); 
}


} // namespace utilib


std::ostream& operator<<( std::ostream& os, 
                          const utilib::ConstAnyAntiCoercionWrapper any )
{
   any.m_any.write(os);
   return os;
}

std::istream& operator>>( std::istream& is, 
                          utilib::AnyAntiCoercionWrapper any )
{
   any.m_any.read(is);
   return is;
}

utilib::PackBuffer& operator<<( utilib::PackBuffer& os, 
                                const utilib::ConstAnyAntiCoercionWrapper any )
{
   any.m_any.write(os);
   return os;
}

utilib::UnPackBuffer& operator>>( utilib::UnPackBuffer& is, 
                                  utilib::AnyAntiCoercionWrapper any )
{
   any.m_any.read(is);
   return is;
}
