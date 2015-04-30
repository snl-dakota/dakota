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

#include <utilib/PropertyDict.h>

using std::list;
using std::set;
using std::map;
using std::pair;

using std::cerr;
using std::endl;

#define ERR_HANDLER(MSG)                                           \
   if ( data->exceptionOnReplicationError )                        \
      EXCEPTION_MNGR( utilib::propertyDict_error, MSG );           \
   else                                                            \
      cerr << "WARNING: " << MSG << endl


namespace utilib {

//----------------------------------------------------------------------
// Property Store definitions
//

class PropertyDict::PropertyStore_property 
   : public PropertyDict::PropertyStore
{
public:
   PropertyStore_property()
      : PropertyStore(m_property, Any(), NULL, ""),
        m_property()
   {}

   PropertyStore_property( Property& property_,
                           Any category_,
                           Data* promote_,
                           string description_ )
      : PropertyStore(m_property, category_, promote_, description_),
        m_property(property_)
   {}

   virtual ~PropertyStore_property() {}

   virtual Privileged_Property* privileged()
   { return NULL; }

private:
   Property m_property;
};


class PropertyDict::PropertyStore_privileged
   : public PropertyDict::PropertyStore
{
public:
   PropertyStore_privileged()
      : PropertyStore(m_property, Any(), NULL, ""),
        m_property()
   {}

   PropertyStore_privileged( Privileged_Property& property_,
                             Any category_,
                             Data* promote_,
                             string description_ )
      : PropertyStore(m_property, category_, promote_, description_),
        m_property(property_)
   {}

   virtual ~PropertyStore_privileged() {}

   virtual Privileged_Property* privileged()
   { return &m_property; }

private:
   Privileged_Property m_property;
};



//----------------------------------------------------------------------
// Common PropertyDict writers
//

const std::string PropertyDict::Writer::DEFAULT_INDENT = "   ";

void 
PropertyDict::DescriptionWriter::
item( const Property& /*prop*/, const std::string& name,
      const std::string& description, bool /*promoted*/ )
{
   if ( key_width > 0 )
   {
      std::ios_base::fmtflags orig = os.flags() & std::ios_base::adjustfield;
      os << indent 
         << std::left << std::setw(key_width) << name 
         << std::left;

      string base = string(indent.size() + key_width + 2, ' ' );
      string subindent = "";
      if ( name.size() > static_cast<size_t>(key_width) )
         os << std::endl << string(indent.size() + key_width, ' ' );

      os << ": ";
      size_t len = base.size();
      string::size_type index = 0;
      string::size_type space = 0;
      do {
         space = description.find_first_of(" \t\n", index);
         size_t fragLen = 
            ( space == string::npos ? description.size() : space ) - index;
         if ( len + fragLen > line_wrap )
         {
            os << std::endl << base << subindent;
            len = base.size() + subindent.size();
         }
         os << description.substr(index, fragLen);
         len += fragLen;
         if ( space != string::npos )
         {
            if ( description[space] == '\n' )
            {
               ++space;
               subindent = "";
               while ( space < description.size() && 
                       (description[space]==' ' || description[space]=='\t') )
                  subindent += description[space++];
               os << std::endl << base << subindent;
               len = base.size() + subindent.size();
            }
            else
            {
               os << description[space];
               ++len;
               ++space;
            }
         }
         index = space;
      } while ( index != string::npos );
      os << endl;
      os.setf(orig, std::ios_base::adjustfield);
   }
   else
   {
      int len = -1*static_cast<int>(name.size());
      if ( len < key_width )
      {
         key_width = len;
         if ( max_key_width && key_width < max_key_width )
            key_width = max_key_width;
      }
   }
}


void 
PropertyDict::DescriptionWriter::
end( const PropertyDict* pd )
{
   if ( key_width < 0 )
      pd->write( DescriptionWriter( os, indent, line_wrap, 
                                    max_key_width, -1*key_width ) );
}


void 
PropertyDict::ValueWriter::
item( const Property& prop, const std::string& name,
      const std::string& /*description*/, bool /*promoted*/ )
{
   if ( key_width > 0 )
   {
      std::ios_base::fmtflags orig = os.flags() & std::ios_base::adjustfield;
      os << indent 
         << std::left << std::setw(key_width) << name 
         << ": "
         << std::left << prop << endl;
      os.setf(orig, std::ios_base::adjustfield);
   }
   else
   {
      int len = -1*static_cast<int>(name.size());
      if ( len < key_width )
      {
         key_width = len;
         if ( max_key_width && key_width < max_key_width )
            key_width = max_key_width;
      }
   }
}


void
PropertyDict::ValueWriter::
end( const PropertyDict* pd )
{
   if ( key_width < 0 )
      pd->write(ValueWriter(os, indent, max_key_width, -1*key_width));
}

//----------------------------------------------------------------------
// The PropertyDict Data members
//

PropertyDict::Data::~Data()
{
   while ( ! data_sources.empty() )
   {
      (*data_sources.begin())->data_sinks.erase(this);
      data_sources.erase(data_sources.begin());
   }

   set<Data*>::iterator r_it = data_sinks.begin();
   set<Data*>::iterator r_itEnd = data_sinks.end();

   while ( ! properties.empty() )
      erase_impl(properties.begin());

   while ( ! data_sinks.empty() )
   {
      (*data_sinks.begin())->data_sources.erase(this);
      data_sinks.erase(data_sinks.begin());
   }
}

void 
PropertyDict::Data::
erase(const std::string name)
{
   propertyDict_t::iterator it = lookup(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(propertyDict_error, "PropertyDict::erase(): "
                     "attempt to erase a nonexistent Property '"
                     << name << "'");
   erase_impl(it);
}

void
PropertyDict::Data::
erase_impl(propertyDict_t::iterator it)
{
   if ( it->second->promote && ! data_sinks.empty() )
   {
      set<Data*>::iterator r_it = data_sinks.begin();
      set<Data*>::iterator r_itEnd = data_sinks.end();
      for( ; r_it != r_itEnd; ++r_it )
         (*r_it)->erase_promoted(it);
   }

   delete it->second;
   properties.erase(it);
}

void
PropertyDict::Data::
erase_promoted(propertyDict_t::iterator& source)
{
   propertyDict_t::iterator it = properties.find(source->first);
   if ( it == properties.end() )
      return;
   if ( it->second->promote != source->second->promote )
      return;

   erase_impl(it);
}


PropertyDict::propertyDict_t::iterator
PropertyDict::Data::
declare_impl( const std::string &name, PropertyStore *store )
{
   string normalized_name = normalizeKeys ? normalize(name) : name;
   pair<propertyDict_t::iterator, bool> inserted =
      properties.insert(make_pair(normalized_name, store));
   if ( ! inserted.second )
   {
      delete store;
      EXCEPTION_MNGR(propertyDict_error, 
                     "PropertyDict::declare(): "
                     "attempt to declare duplicate Property '"
                     << name << "'");
   }
   store->id = ++max_propertyStore_id;

   if ( store->promote && ! data_sinks.empty() )
   {
      set<Data*>::iterator r_it = data_sinks.begin();
      set<Data*>::iterator r_itEnd = data_sinks.end();
      try {
         for( ; r_it != r_itEnd; ++r_it )
            (*r_it)->declare_impl
               ( normalized_name, new PropertyStore_property
                 ( store->property, store->category, store->promote, 
                   store->description ) );
      } catch ( propertyDict_error& e ) {
         // unwind all declarations
         while ( r_it != data_sinks.begin() )
         {
            --r_it;
            (*r_it)->erase( normalized_name );
         }
         properties.erase(normalized_name);
         delete store;
         throw;
      }
   }

   return inserted.first;
}

void
PropertyDict::Data::dereference_impl( Data &source )
{
#ifndef UTILIB_HAVE_BOOST
      EXCEPTION_MNGR
         ( propertyDict_error, "PropertyDict::dereference() is not "
           "available when UTILIB is compiled without Boost support." );
#else
   if ( ! data_sources.erase(&source) )
      EXCEPTION_MNGR( propertyDict_error, "PropertyDict::dereference(): "
                      "specified source not found in reference_sources()" );
   source.data_sinks.erase(this);

   propertyDict_t::iterator p;
   propertyDict_t::iterator prop = source.properties.begin();
   propertyDict_t::iterator propEnd = source.properties.end();
   for( ; prop != propEnd; ++prop)
   {
      if ( prop->second->promote )
         erase_promoted(prop);
      else if ( ( p = properties.find(prop->first) ) != properties.end() )
      {
         // NB: This WILL cause problems if there are 2 sources with the
         // same property name.  While this situation usually results in
         // an exception, clearing exceptionOnReplicationError will only
         // throw a warning...
         p->second->source.disconnect();
      }
   }
#endif
}

void
PropertyDict::Data::dereference_all( )
{
   while ( ! data_sources.empty() )
      dereference_impl( **(data_sources.begin()) );
}


//----------------------------------------------------------------------
// The PropertyDict class members
//

PropertyDict::~PropertyDict()
{}


Property& 
PropertyDict::
declare( const std::string name, Property property, 
         Any category, bool promote, std::string description )
{
   return data->declare_impl
      ( name, new PropertyStore_property
        ( property, category, promote ? &*this->data : NULL, description )
        )->second->property;
}


Privileged_Property& 
PropertyDict::
declare( const std::string name, Privileged_Property property,
         Any category, bool promote, std::string description )
{
   return *data->declare_impl
      ( name, new PropertyStore_privileged
        ( property, category, promote ? &*this->data : NULL, description ) 
        )->second->privileged();
}



PropertyDict::propertyDict_t::iterator 
PropertyDict::get_impl(const std::string& name)
{
   propertyDict_t::iterator it = data->lookup(name);
   if ( it == data->properties.end() )
   {
      if ( data->implicitDeclareIfDNE )
         return data->declare_impl( name, new PropertyStore_privileged());

      EXCEPTION_MNGR(propertyDict_error, "PropertyDict::get_impl(): "
                     "attempt to retrieve nonexistent Property '"
                     << name << "'");
   }

   return it;
}

PropertyDict::propertyDict_t::const_iterator 
PropertyDict::get_impl(const std::string& name) const
{
   propertyDict_t::const_iterator it = data->lookup(name);
   if ( it == data->properties.end() )
      EXCEPTION_MNGR(propertyDict_error, "PropertyDict::get_impl() const: "
                     "attempt to retrieve nonexistent Property '"
                     << name << "'");

   return it;
}



const int PropertyDict::connection_group = -1000;

/** NB: This method has side-effects: any properties that are set to
 *  reference the source PropertyDict will have their value reset to
 *  that of the source PropertyDict (therefore calling any
 *  locally-registered onChange() callbacks!)
 */
void 
PropertyDict::
reference( PropertyDict &source,
           std::set<Any> exclude, 
           std::set<std::string> block_promotion )
{
#ifndef UTILIB_HAVE_BOOST
      EXCEPTION_MNGR
         ( propertyDict_error, "PropertyDict::reference() is not "
           "available when UTILIB is compiled without Boost support." );
#else
   if ( ! source.data->data_sinks.insert(&*this->data).second ||
        ! data->data_sources.insert(&*source.data).second )
      EXCEPTION_MNGR(propertyDict_error, "PropertyDict::reference(): "
                     "duplicate reference to an external PropertyDict");

   propertyDict_t::iterator src = source.data->properties.begin();
   propertyDict_t::iterator srcEnd = source.data->properties.end();

   propertyDict_t::iterator it = data->properties.begin();
   propertyDict_t::iterator itEnd = data->properties.end();

   typedef 
      map<size_t, pair<propertyDict_t::iterator,propertyDict_t::iterator> >
      referenceBuffer_t;

   referenceBuffer_t references;
   referenceBuffer_t::iterator ref_it = references.begin();
   list<string> promoted;

   try {
      int test = 0;
      while ( src != srcEnd )
      {
         if ( it == itEnd )
            test = 1;
         else
            test = it->first.compare(src->first);

         if ( test < 0 )
            ++it;
         else if ( test == 0 )
         {
            if ( exclude.count(it->second->category) == 0 )
            {
               if ( ! it->second->privileged() )
               {
                  ERR_HANDLER( "PropertyDict::reference(): "
                               "non-privileged property, '" << it->first 
                               << "' cannot reference remote value." );
               }
               else if ( it->second->source.connected() )
               {
                  ERR_HANDLER( "PropertyDict::reference(): "
                               "attempt to reference property '"
                               << it->first << "', which is already connected "
                               "to a remote value." );
               }
               else if ( src->second->promote &&
                         ! block_promotion.count(src->first) )
               {
                  ERR_HANDLER( "PropertyDict::reference(): "
                               "local property '" << it->first 
                               << "', masks a remote promotable property." );
               }
               else
               {
                  references.insert
                     ( references.end(),
                       make_pair(it->second->id, make_pair(it, src)) );
               }
            }
            ++it;
            ++src;
         }
         else
         {
            if ( src->second->promote && ! block_promotion.count(src->first) )
            {
               data->declare_impl( src->first, new PropertyStore_property
                                   ( src->second->property,
                                     src->second->category,
                                     src->second->promote,
                                     src->second->description ) );
               promoted.push_back(src->first);
            }
            ++src;
         }
      }

      for(ref_it = references.begin(); ref_it != references.end(); ++ref_it)
      {
         Privileged_Property* prop
            = ref_it->second.first->second->privileged();
         Property& src = ref_it->second.second->second->property;

         Property::bound_set_t _set = prop->bind_set();
         ref_it->second.first->second->source = 
            src.onChange().connect(connection_group, _set);
         prop->set_readonly();

         // copy the current value!
         try {
            _set(src);
         } catch ( std::exception &e ) {
            EXCEPTION_MNGR( propertyDict_error, "PropertyDict::reference(): "
                            "exception caught while replicating value of '"
                            << ref_it->second.first->first << "':" 
                            << std::endl << e.what() );
         }
      }
   } catch ( ... ) {
      while ( ! promoted.empty() )
      {
         erase(promoted.front());
         promoted.pop_front();
      }
      while ( ref_it != references.begin() )
      {
         --ref_it;
         ref_it->second.first->second->source.disconnect();
      }
      throw;
   }
#endif // UTILIB_HAVE_BOOST
}


void
PropertyDict::dereference( PropertyDict &source )
{
   data->dereference_impl( *source.data );
}

void
PropertyDict::dereference_all()
{
   data->dereference_all();
}


void PropertyDict::write(const PropertyDict::Writer& writer) const
{
   PropertyDict::Writer& w = const_cast<PropertyDict::Writer&>(writer);
   propertyDict_t::const_iterator it = data->properties.begin();
   propertyDict_t::const_iterator itEnd = data->properties.end();
   w.start(this);
   for ( ; it != itEnd; ++it )
      w.item( it->second->property, 
               it->first, 
               it->second->description,
               it->second->promote != NULL );
   w.end(this);
}

} // namespace utilib
