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
 * \file PropertyDict.h
 *
 * Defines the utilib::PropertyDict class.
 */

#ifndef utilib_PropertyDict_h
#define utilib_PropertyDict_h

#include <utilib/Property.h>

namespace utilib {

class propertyDict_error : public std::runtime_error
{
public:
   /// Constructor
   propertyDict_error(const std::string& msg) 
      : runtime_error(msg)
   {}
};

/// Probably not a good place to put it, but we need to support property lists
typedef std::vector<Property> PropertyList;

/** \TODO: There is an inherent danger to this implementation of Promoted
 *  Properties: callbacks registered with the promoted property are
 *  actually registered with the original property.  This means that the
 *  callback registration may persist past the end of the promoted
 *  instance's lifespan.  Ideally, we would use the same onChange system
 *  that the replicated properties use.  Unfortunately, the primary use
 *  of promoted properties is to expose a *writable* property to the
 *  derived PropertyDict, and it is not at all clear how to allow a
 *  replicated property to be writable (and still guarantee consistency
 *  with the original promoted property.
 *
 *  \TODO: Once a property is set to replicate an external value, it
 *  becomes read-only.  That read-only flag currently persists, even if
 *  the replication connection is broken...
 */
class PropertyDict
{
protected:
   class Data;
public:
   class PropertyStore
   {
   public:
      PropertyStore( Property& property_, 
                     Any category_, 
                     Data* promote_,
                     std::string description_ )
         : property(property_),
           category(category_),
           promote(promote_),
           description(description_),
           id(0)
#ifdef UTILIB_HAVE_BOOST
           , source()
#endif
      {}

      virtual ~PropertyStore() {}

      virtual Privileged_Property* privileged() = 0;

      Property&      property;
      Any            category;
      Data*          promote;
      std::string    description;
      size_t         id;
#ifdef UTILIB_HAVE_BOOST
      boost::signals::scoped_connection  source;
#endif
   };

protected:
   class PropertyStore_property;
   class PropertyStore_privileged;

   typedef std::map<std::string, PropertyStore*>  propertyDict_t;

public:

   template<typename INNER, typename FIRST, typename SECOND>
   struct iterator_template {
      struct value_type {
         value_type(INNER& _it)
            : base(_it)
         {}

         FIRST& first() const
         { return base->first; }

         SECOND& second() const
         { return base->second->property; }

      private:
         INNER& base;
      };
      typedef const value_type& reference;
      typedef const value_type* pointer;
      friend struct value_type;

      typedef std::bidirectional_iterator_tag iterator_category;
      typedef std::ptrdiff_t                  difference_type;

      typedef iterator_template<INNER, FIRST, SECOND>  _self;
      typedef INNER  _base_ptr;

      iterator_template()
         : it(), ref(it) {}

      explicit
      iterator_template(_base_ptr _x)
         : it(_x), ref(it) {}

      iterator_template(const iterator_template& rhs)
         : it(rhs.it), ref(it) {}

      iterator_template&
      operator=(const iterator_template& rhs)
      {
         if ( this != &rhs )
            it = rhs.it;
         return *this;
      }

      reference
      operator*() const
      {
         return ref;
      }

      pointer
      operator->() const
      {          
         return &ref;
      }

      _self&
      operator++()
      {
         ++it;
         return *this;
      }

      _self
      operator++(int)
      {
         _self tmp = *this;
         ++it;
         return tmp;
      }

      _self&
      operator--()
      {
         --it;
         return *this;
      }

      _self
      operator--(int)
      {
         _self tmp = *this;
         --it;
         return tmp;
      }

      bool
      operator==(const _self& _x) const
      { return it == _x.it; }

      bool
      operator!=(const _self& _x) const
      { return it != _x.it; }

   private:
      _base_ptr it;
      value_type ref;
   };

   typedef 
   iterator_template< propertyDict_t::iterator, 
                      const std::string, Property >
   iterator;


   typedef 
   iterator_template< propertyDict_t::const_iterator, 
                      const std::string, const Property >
   const_iterator;

public:
   class Writer
   {
   public:
      static const std::string DEFAULT_INDENT;
      static const int DEFAULT_MAX_KEY_WIDTH = 20;
      static const int DEFAULT_LINE_WRAP = 78;
   public:
      Writer() {}
      virtual ~Writer() {}
      virtual void start(const PropertyDict*) {}
      virtual void item( const Property& /*property*/, 
                         const std::string& /*name*/,
                         const std::string& /*description*/, 
                         bool /*promoted*/ ) {}
      virtual void end(const PropertyDict*) {}
   };

   class DescriptionWriter : public Writer
   {
   public:
      DescriptionWriter()
         : os(std::cout), indent(Writer::DEFAULT_INDENT), 
           line_wrap(Writer::DEFAULT_LINE_WRAP), 
           max_key_width(-1*Writer::DEFAULT_MAX_KEY_WIDTH), key_width(0)
      {}
      DescriptionWriter( std::ostream& os_, 
                         std::string indent_ = Writer::DEFAULT_INDENT,
                         int line_wrap_ = Writer::DEFAULT_LINE_WRAP,
                         int max_key_width_ = Writer::DEFAULT_MAX_KEY_WIDTH,
                         int key_width_ = 0 )
         : os(os_), indent(indent_), line_wrap(line_wrap_), 
           max_key_width(-1*max_key_width_), key_width(key_width_)
      {}
      virtual ~DescriptionWriter() {}
      virtual void item( const Property& property, const std::string& name,
                         const std::string& description, bool promoted );
      virtual void end(const PropertyDict*);
   private:
      std::ostream& os;
      std::string indent;
      int line_wrap;
      int max_key_width;
      int key_width;
   };

   class ValueWriter : public Writer
   {
   public:
      ValueWriter()
         : os(std::cout), indent(Writer::DEFAULT_INDENT), 
           max_key_width(-1*Writer::DEFAULT_MAX_KEY_WIDTH), key_width(0)
      {}
      ValueWriter( std::ostream& os_, 
                   std::string indent_ = Writer::DEFAULT_INDENT,
                   int max_key_width_ = Writer::DEFAULT_MAX_KEY_WIDTH,
                   int key_width_ = 0 )
         : os(os_), indent(indent_), 
           max_key_width(-1*max_key_width_), key_width(key_width_)
      {}
      virtual ~ValueWriter() {}
      virtual void item( const Property& property, const std::string& name,
                         const std::string& description, bool promoted );
      virtual void end(const PropertyDict*);
   private:
      std::ostream& os;
      std::string indent;
      int max_key_width;
      int key_width;
   };

public:
   PropertyDict(bool declareIfDNE = false)
      : data()
   {
      data->implicitDeclareIfDNE = declareIfDNE;
   }

   virtual ~PropertyDict();

   bool empty() const
   { return data->properties.empty(); }

   /// Returns true if the property is defined in the dictionary
   bool exists(const std::string name) const
   { return data->lookup(name) != data->properties.end(); }

   /// Declare a new property in the dictionary
   Property& 
   declare( const std::string name,  Property property,
            Any category = Any(),  bool promote = false, 
            std::string description = "" );

   /// Declare a new privileged property in the dictionary
   Privileged_Property& 
   declare( const std::string name,  
            Privileged_Property property = Privileged_Property(),
            Any category = Any(),  bool promote = false,
            std::string description = "" );

   Property& 
   declare( const std::string name,  std::string description,
            Property property, Any category = Any(),  bool promote = false)
   { return declare(name, property, category, promote, description); }

   Privileged_Property& 
      declare( const std::string name, std::string description,
            Privileged_Property property = Privileged_Property(),
            Any category = Any(),  bool promote = false )
   { return declare(name, property, category, promote, description); }

   /// Remove a property from the dictionary
   void erase(const std::string name)
   { data->erase(name); }


   Property& get(const std::string name)
   { return get_impl(name)->second->property; }

   const Property& get(const std::string name) const
   { return get_impl(name)->second->property; }

   Property& operator[](const std::string name)
   { return get(name); }

   const Property& operator[](const std::string name) const
   { return get(name); }

   iterator find(std::string name) 
   { return iterator(data->lookup(name)); }

   const_iterator find(std::string name) const
   { return const_iterator(data->lookup(name)); }

   iterator begin()
   { return iterator(data->properties.begin()); }

   iterator end()
   { return iterator(data->properties.end()); }

   const_iterator begin() const
   { return const_iterator(data->properties.begin()); }

   const_iterator end() const
   { return const_iterator(data->properties.end()); }

   static const int connection_group; // = -1000;

   void 
   reference( PropertyDict &source,
              std::set<Any> exclude = std::set<Any>(), 
              std::set<std::string> block_promotion = std::set<std::string>());

   void dereference( PropertyDict &source );

   void dereference_all();

   bool& implicitDeclareIfDNE()
   { return data->implicitDeclareIfDNE; }

   bool& exceptionOnReplicationError()
   { return data->exceptionOnReplicationError; }

   bool& normalizeKeys()
   { return data->normalizeKeys; }

   void write(const Writer &writer = ValueWriter()) const;

protected:

   class Data 
   {
   public:
      Data()
         : implicitDeclareIfDNE(false),
           exceptionOnReplicationError(true),
           normalizeKeys(false),
           max_propertyStore_id(0)
      {}
      
      ~Data();

      /// The map of properties in this PropertyDict
      propertyDict_t  properties;

      std::set<Data*> data_sources;
      
      std::set<Data*> data_sinks;
      
      /// true: getting an undefined property will create an unbound property
      bool implicitDeclareIfDNE;

      /// true: replicate remote value throws exception for read-only property
      bool exceptionOnReplicationError;

      /// true: replace ' ' and '_' with '-' in all key strings
      bool normalizeKeys;

      size_t max_propertyStore_id;

   public:
      propertyDict_t::iterator lookup(const std::string& name)
      {
         return properties.find
            (normalizeKeys ? Data::normalize(name) : name);
      }
      
      propertyDict_t::const_iterator lookup(const std::string& name) const
      {
         return properties.find
            (normalizeKeys ? Data::normalize(name) : name);
      }

      /// Remove a property from the dictionary
      void erase(const std::string name);

      void erase_impl(propertyDict_t::iterator it);

      void erase_promoted(propertyDict_t::iterator& source);

      /// Common functionality used by all forms of declare()
      propertyDict_t::iterator
      declare_impl( const std::string &name, PropertyStore *store );
      
      void dereference_impl( Data &source );

      void dereference_all( );

      static inline char char_transform(char c) 
      { return c == '_' || c == ' ' ? '-' : c; }
      static inline std::string normalize(const std::string& name_)
      {
         std::string ans = name_;
         std::transform(ans.begin(), ans.end(), ans.begin(), char_transform);
         return ans;
      }
   };
      
   utilib::ReferenceCounted<Data> data;

   propertyDict_t::iterator get_impl(const std::string& name);
      
   propertyDict_t::const_iterator get_impl(const std::string& name) const;

private:
};


class Privileged_PropertyDict : public PropertyDict
{
public:
   Privileged_PropertyDict()
      : PropertyDict()
   {}

   typedef propertyDict_t::iterator privileged_iterator;
   typedef propertyDict_t::const_iterator privileged_const_iterator;

public:
   Privileged_Property& privilegedGet(const std::string name)
   {
      Privileged_Property* ans = get_impl(name)->second->privileged();
      if ( ! ans )
         EXCEPTION_MNGR( propertyDict_error, 
                         "Privileged_PropertyDict::privilegedGet(): "
                         "Property '" << name << "' is not Privileged" );
      
      return *ans;
   }

   const Privileged_Property& privilegedGet(const std::string name) const
   {
      Privileged_Property* ans = get_impl(name)->second->privileged();
      if ( ! ans )
         EXCEPTION_MNGR( propertyDict_error, 
                         "Privileged_PropertyDict::privilegedGet() const: "
                         "Property '" << name << "' is not Privileged" );
      
      return *ans;
   }

   privileged_iterator privilegedFind(std::string name) 
   { return data->lookup(name); }

   privileged_iterator privilegedBegin()
   { return data->properties.begin(); }

   privileged_iterator privilegedEnd()
   { return data->properties.end(); }

   privileged_const_iterator privilegedBegin() const
   { return data->properties.begin(); }

   privileged_const_iterator privilegedEnd() const
   { return data->properties.end(); }
};

} // namespace utilib

#endif // defined utilib_PropertyDict_h
