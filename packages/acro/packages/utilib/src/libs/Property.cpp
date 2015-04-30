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

#include <utilib/Property.h>

#ifdef UTILIB_HAVE_BOOST
#include <boost/bind.hpp>
#endif

namespace utilib {

//---------------------------------------------------------------------
#ifdef UTILIB_HAVE_BOOST

namespace {

void property_updater(Property* target, const ReadOnly_Property& source)
{
   target->set(source.get());
}
void privileged_updater( Privileged_Property* target, 
                         const ReadOnly_Property& source )
{
   target->set(source.get());
}

} // namespace utilib::(local)

Property::bound_set_t
Property::bind_set()
{
   return boost::bind(&utilib::property_updater, this, _1); 
}

Property::bound_set_t
Privileged_Property::bind_set()
{
   return boost::bind(&utilib::privileged_updater, this, _1); 
}

#endif // UTILIB_HAVE_BOOST
//---------------------------------------------------------------------


namespace {
/** This map is treated as a stack of the currently active onChange
 *  events so we can easily identify and break out of infinite onChange
 *  loops.
 *
 *  NOTE: this implementation is NOT threadsafe.  A threadsafe
 *  implementation would probably look more like:
 *
 *   std::map<thread_id, std::map<Property*, size_t> >
 */
typedef std::map<Property*, size_t>  _s_onchange_stack_t;
_s_onchange_stack_t _s_onchange_stack = std::map<Property*, size_t>();

/** This map allows Privileged Properties to temporarily bypass the
 *  onChange loop detection system in a controlled manner.
 */
std::map< Privileged_Property*, 
          std::list<_s_onchange_stack_t> >  _s_onchange_loops;


bool push_onchange_stack(Property* prop)
{
   return _s_onchange_stack.insert
      ( std::make_pair(prop, _s_onchange_stack.size()+1) ).second;
}

void pop_onchange_stack(Property* prop)
{
   std::map<Property*, size_t>::iterator it = _s_onchange_stack.find(prop);
   if ( it == _s_onchange_stack.end() )
      EXCEPTION_MNGR(std::logic_error, "Property: pop_onchange_stack(): "
                     "attempt to pop a property off the stack that is not "
                     "on the stack");

   if ( it->second != _s_onchange_stack.size() )
      EXCEPTION_MNGR(std::logic_error, "Property: pop_onchange_stack(): "
                     "attempt to pop a property off the stack that is not "
                     "on the top of the stack");

   _s_onchange_stack.erase(it);
}

} // namespace utilib::(local)


/** This implements the actual routine for setting a Property value.  The
 *  protected indirection allows us to declare sub-classes that bypass
 *  the "is_writable" flag.
 */
void Property::set_impl(utilib::Any new_value, bool writable)
{
   if ( ! writable )
      EXCEPTION_MNGR(property_error, "Property::set(): attempt to set "
                     "a Property declared read-only.");

   // NOTE: this loop check implementation is NOT thread safe!
   if ( ! push_onchange_stack(this) )
      EXCEPTION_MNGR(property_error, "Property::set(): "
                     "onChange callback loop detected "
                     "(onChange triggers changes which trigger itself).");

   // Remembering code for split onChange / replicate events
   //
   //bool bypass_afterChange = true;
   //if ( _s_cb_replicate_buffer.empty() )
   //   bypass_afterChange = false;
   //if ( _s_cb_replicate_visited.count(this) )
   //   EXCEPTION_MNGR(property_error, "Property::set(): "
   //                  "replicate callback loop detected "
   //                  "(replicate triggers an already set onChange.");
   //_s_cb_replicate_buffer.push_back(this);


   // Because of the loop detection, we MUST implement this entire
   // function within a try-catch block so that this Property is not
   // left on the _s_onchange_stack.
   try {
      // For most cases (i.e. using the default set functor on a bound
      // Property), we should first make sure the data is convertable to
      // the Property's bound type.  This has the nice side benefit
      // that any validate listeners who know how the Property was
      // declared can avoid a call to lexical_cast.
      Any converted_new_value;
      if ( ( ! connected(data->set_functor) ) && data->value.is_immutable() )
      {
         if ( new_value.empty() )
            EXCEPTION_MNGR( property_error, "Property::set(): "
                            "attempt to empty a bound Property" );
         else
            TypeManager()->lexical_cast(new_value, converted_new_value,
                                        data->value.type() );
      }
      else
         converted_new_value = new_value;


      // check if the new value is OK
      if ( ! validate()(*this, converted_new_value) )
         EXCEPTION_MNGR(property_error, "Property::set(): set disallowed "
                        "by validator callback for new value = " << new_value);


      // set the Property's data...
      if ( connected(data->set_functor) )
         data->set_functor(data->value, converted_new_value);
      else
      {
         if ( converted_new_value.empty() )
            data->value.clear();
         else
            TypeManager()->lexical_cast(converted_new_value, data->value);
      }

      // We want to defeat the normal Any shallow copies so that the
      // user can't set the Property value without going through the
      // set() method.
      //
      // NB: We must specially check for the Unbound properties
      Any& tmp = ( data->value.is_type(typeid(UntypedAnyContainer)) 
                   ? data->value.expose<UntypedAnyContainer>().m_data
                   : data->value );
      if ( tmp.references_same_data_as(new_value) )
      {
         Any clone = tmp.clone();
         tmp.clear();
         tmp = clone;
      }

      onChange()(*this);

   } catch ( ... ) {
      pop_onchange_stack(this);
      throw;
   }

   pop_onchange_stack(this);

   // Remembering code for split onChange / replicate events
   //
   //if ( bypass_afterChange )
   //   return;
   //
   //std::set<Property*> visited;
   //while ( ! _s_cb_replicate_buffer.empty() )
   //{
   //   Property* prop = _s_cb_replicate_buffer.front();
   //   if ( _s_cb_replicate_visited.insert(prop).second )
   //      prop->replicate()(*prop);
   //   _s_cb_replicate_buffer.pop_front();
   //}
}



/** Special case: casting a Property to "void" should do nothing.
 */
template<>
void ReadOnly_Property::as<void>() const
{}

/** Special case: casting a Property to "Any" should return the
 *  contained value, and not embed the Property within an Any.
 */
template<>
utilib::Any ReadOnly_Property::as<utilib::Any>() const
{ return get(); }

/** Special case: casting a Property to "AnyRef" should return a
 *  reference to the Property within an AnyRef
 */
template<>
utilib::AnyRef ReadOnly_Property::as<utilib::AnyRef>() const
{ 
   return asAnyRef();
}

/** Special case: casting a Property to "AnyFixedRef" should return an
 *  immutable reference to the Property within an AnyFixedRef
 */
template<>
utilib::AnyFixedRef ReadOnly_Property::as<utilib::AnyFixedRef>() const
{ 
   return asAnyFixedRef();
}

/** Special case: when constructing one Property from another with the
 *  following syntax:
 *    \code
 *    Property a;
 *    Property b(a);
 *    \endcode
 *
 *  Some compilers (i.e. GCC 4.1.2) will pick up this template, EVEN if
 *  the explicit copy constructor exists (and in some cases, when it
 *  does, the compiler will die on internal segmentation fault).
 *  Explicitly overloading this template instantiation to "do the right
 *  thing" appears to be the best solution.
 */
template<>
Property::
Property(Property& rhs)
   : ReadOnly_Property(rhs)
{}

template<>
Property::
Property(Privileged_Property& rhs)
   : ReadOnly_Property(rhs)
{}

template<>
Privileged_Property::
Privileged_Property(Privileged_Property& rhs)
   : Property(rhs)
{}

template<>
Property::
Property(const Privileged_Property& rhs)
   : ReadOnly_Property(rhs)
{}


template<>
bool ReadOnly_Property::operator==(const ReadOnly_Property& rhs) const
{ return equality_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator==(const Property& rhs) const
{ return equality_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator==(const Privileged_Property& rhs) const
{ return equality_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator==(const Any& rhs) const
{ return equality_compare_any(rhs); }

template<>
bool ReadOnly_Property::operator==(const AnyRef& rhs) const
{ return equality_compare_any(rhs); }

template<>
bool ReadOnly_Property::operator==(const AnyFixedRef& rhs) const
{ return equality_compare_any(rhs); }


template<>
bool ReadOnly_Property::operator<(const ReadOnly_Property& rhs) const
{ return lessThan_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator<(const Property& rhs) const
{ return lessThan_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator<(const Privileged_Property& rhs) const
{ return lessThan_compare_property(rhs); }

template<>
bool ReadOnly_Property::operator<(const Any& rhs) const
{ return lessThan_compare_any(rhs); }

template<>
bool ReadOnly_Property::operator<(const AnyRef& rhs) const
{ return lessThan_compare_any(rhs); }

template<>
bool ReadOnly_Property::operator<(const AnyFixedRef& rhs) const
{ return lessThan_compare_any(rhs); }



bool 
ReadOnly_Property::
equality_compare_property(const ReadOnly_Property& rhs) const
{
   return equality_compare_any(rhs.get());
}

bool 
ReadOnly_Property::
equality_compare_any(const Any& rhs) const
{
   if ( rhs.is_type(typeid(ReadOnly_Property)) )
      return equality_compare_property( rhs.expose<ReadOnly_Property>() );
   else if ( rhs.is_type(typeid(Property)) )
      return equality_compare_property( rhs.expose<Property>() );
   else if ( rhs.is_type(typeid(Privileged_Property)) )
      return equality_compare_property( rhs.expose<Privileged_Property>() );

   Any L = get();
   if ( L.empty() || rhs.empty() )
      return L.empty() == rhs.empty();

   utilib::Any tmp;
   try {
      TypeManager()->lexical_cast(L, tmp, rhs.type());
      return rhs == tmp;
   } catch ( utilib::bad_lexical_cast &e ) {
      // silently fall through to casting the rhs to match this property
   } catch ( utilib::any_not_comparable &e ) {
      // silently fall through to casting the rhs to match this property
   }

   TypeManager()->lexical_cast(rhs, tmp, L.type());
   return tmp == L;
}

bool 
ReadOnly_Property::
lessThan_compare_property(const ReadOnly_Property& rhs) const
{
   return lessThan_compare_any(rhs.get());
}

bool 
ReadOnly_Property::
lessThan_compare_any(const Any& rhs) const
{
   if ( rhs.is_type(typeid(ReadOnly_Property)) )
      return lessThan_compare_property( rhs.expose<ReadOnly_Property>() );
   else if ( rhs.is_type(typeid(Property)) )
      return lessThan_compare_property( rhs.expose<Property>() );
   else if ( rhs.is_type(typeid(Privileged_Property)) )
      return lessThan_compare_property( rhs.expose<Privileged_Property>() );

   Any L = get();
   if ( L.empty() )
      return ! rhs.empty();
   if ( rhs.empty() )
      return false;

   utilib::Any tmp;
   try {
      TypeManager()->lexical_cast(L, tmp, rhs.type());
      return tmp < rhs;
   } catch ( utilib::bad_lexical_cast &e ) {
      // silently fall through to casting the rhs to match this property
   } catch ( utilib::any_not_comparable &e ) {
      // silently fall through to casting the rhs to match this property
   }

   TypeManager()->lexical_cast(rhs, tmp, L.type());
   return L < tmp;
}


Privileged_Property::onChangeLoop_t
Privileged_Property::allowOnChangeRecursion(size_t max)
{
   if ( _s_onchange_loops[this].size() >= max )
      EXCEPTION_MNGR(property_error, "Privileged_Property::"
                     "allowOnChangeRecursion: "
                     "exceeded maximum number of loops (" <<
                     _s_onchange_loops[this].size() << " >= " << max << ")");

   _s_onchange_loops[this].push_back(_s_onchange_stack);
   _s_onchange_stack.clear();
   onChangeLoop_t ans;
   ans->configure(this, _s_onchange_loops[this].size());
   return ans;
}

Privileged_Property::onChangeLoopData::~onChangeLoopData()
{
   if ( ! _s_onchange_stack.empty() )
      EXCEPTION_MNGR(property_error, "Privileged_Property::onChangeLoopData: "
                     "declared onChange loop did not completely unroll.");

   if ( _s_onchange_loops[owner].size() != id )
      EXCEPTION_MNGR(property_error, "Privileged_Property::onChangeLoopData: "
                     "declared onChange loops are unrolling out of order.");

   _s_onchange_stack = _s_onchange_loops[owner].back();
   _s_onchange_loops[owner].pop_back();
}


} // namespace utilib


/** Printing the ReadOnlyProperty prints the contained Any
 *
 *  NB: By declaring a streaming operator for the ReadOnlyProperty
 *  class, all Properties are streamable.
 */
std::ostream& operator<<( std::ostream& os, 
                          const utilib::ReadOnly_Property& property )
{
   return os << property.get();
}


/** Reading from an istream reads into the contained Any
 */
std::istream& operator>>( std::istream& is, utilib::Property& property )
{
   // NB: read the property first so that we "know" what data type we
   // should attempt to read into.
   utilib::Any tmp = property.get();
   is >> tmp;
   property = tmp;
   return is;
}
