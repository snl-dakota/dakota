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
 * \file Application_IntDomain.cpp
 *
 * Defines the colin::Application_IntDomain class member functions
 */

#include <colin/application/IntDomain.h>
#include <colin/TinyXML.h>

#include <boost/bimap.hpp>

using utilib::Property;

using boost::bind;

using std::vector;
using std::string;
using std::cerr;
using std::endl;

namespace colin {

namespace {

typedef boost::bimap<size_t, std::string>  labels_t;
typedef std::vector<int>   int_domain_t;
typedef std::vector<bool>  binary_domain_t;

} // namespace colin::(local)

//============================================================================
//============================================================================
// Class Application_IntDomain
//============================================================================
//============================================================================

Application_IntDomain::Application_IntDomain()
   : _num_int_vars(Property::Bind<size_t>()),
     _int_lower_bounds(Property::Bind<int_domain_t>()),
     _int_upper_bounds(Property::Bind<int_domain_t>()),
     _int_lower_bound_types(Property::Bind<BoundTypeArray>()),
     _int_upper_bound_types(Property::Bind<BoundTypeArray>()),
     _int_labels(Property::Bind<labels_t>()),
     _num_binary_vars(Property::Bind<size_t>()),
     _binary_labels(Property::Bind<labels_t>()),
     num_int_vars(_num_int_vars.set_readonly()),
     int_lower_bounds(_int_lower_bounds.set_readonly()),
     int_upper_bounds(_int_upper_bounds.set_readonly()),
     int_lower_bound_types(_int_lower_bound_types.set_readonly()),
     int_upper_bound_types(_int_upper_bound_types.set_readonly()),
     int_labels(_int_labels.set_readonly()),
     num_binary_vars(_num_binary_vars.set_readonly()),
     binary_labels(_binary_labels.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare("num_int_vars", _num_int_vars, t);
   properties.declare("int_lower_bounds", _int_lower_bounds, t);
   properties.declare("int_upper_bounds", _int_upper_bounds, t);
   properties.declare("int_lower_bound_types", _int_lower_bound_types, t);
   properties.declare("int_upper_bound_types", _int_upper_bound_types, t);
   properties.declare("int_labels", _int_labels, t);

   properties.declare("num_binary_vars", _num_binary_vars, t);
   properties.declare("binary_labels", _binary_labels, t);

   _num_int_vars.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_int, this, _1) );
   _num_binary_vars.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_binary, this, _1) );

   _int_lower_bounds.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_bounds, this, _1) );
   _int_upper_bounds.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_bounds, this, _1) );

   _int_lower_bound_types.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_bound_types, this, _1) );
   _int_upper_bound_types.onChange().connect
      ( bind(&Application_IntDomain::cb_onChange_bound_types, this, _1) );

   _int_lower_bounds.validate().connect
      ( bind(&Application_IntDomain::cb_validate_bounds, this, _1, _2) );
   _int_upper_bounds.validate().connect
      ( bind(&Application_IntDomain::cb_validate_bounds, this, _1, _2) );
   _int_lower_bound_types.validate().connect
      ( bind(&Application_IntDomain::cb_validate_bound_types, this, _1, _2) );
   _int_upper_bound_types.validate().connect
      ( bind(&Application_IntDomain::cb_validate_bound_types, this, _1, _2) );

   _int_labels.validate().connect
      ( bind(&Application_IntDomain::cb_validate_labels, this, _1, _2) );
   _binary_labels.validate().connect
      ( bind(&Application_IntDomain::cb_validate_labels, this, _1, _2) );

   print_signal.connect
      ( 11, boost::bind(&Application_IntDomain::cb_print, this, _1) );

   domain_initializer("BinaryVars").connect
      ( bind(&Application_IntDomain::cb_initialize_binary, this, _1));
   // support both "IntegerVars" and "IntVars"
   domain_initializer("IntegerVars").connect
      ( bind(&Application_IntDomain::cb_initialize_int, this, _1));
   domain_initializer("IntVars").connect
      ( bind(&Application_IntDomain::cb_initialize_int, this, _1));

   // notify Application_Domain about our part of the domain
   get_domain_size_signal.connect
      ( bind(&utilib::ReadOnly_Property::as<size_t>, &this->num_int_vars) );
   get_domain_size_signal.connect
      ( bind(&utilib::ReadOnly_Property::as<size_t>, &this->num_binary_vars) );

   // propagate num_X_vars::onChange to domain_size
   _num_int_vars.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &domain_size.onChange(), domain_size));
   _num_binary_vars.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &domain_size.onChange(), domain_size));

   // initialize everything (relying on the callbacks)
   _num_int_vars = 0;
   _num_binary_vars = 0;
}

/// Virtual destructor
Application_IntDomain::~Application_IntDomain()
{ }


/// Returns the lower bound on the $i$-th variable
int
Application_IntDomain::intLowerBound(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "intLowerBound(): index past num_int_vars");
   if ( ! this->enforcing_domain_bounds || intLowerBoundType(i) == no_bound )
      return INT_MIN;
   return int_lower_bounds.expose<int_domain_t>()[i];
}


/// Returns the upper bound on the $i$-th variable
int
Application_IntDomain::intUpperBound(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "intUpperBound(): index past num_int_vars");
   if ( ! this->enforcing_domain_bounds || intUpperBoundType(i) == no_bound )
      return INT_MAX;
   return int_upper_bounds.expose<int_domain_t>()[i];
}


/// Returns the type of the int lower bound on the $i$-th variable
bound_type_enum
Application_IntDomain::intLowerBoundType(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "intLowerBoundType(): index past num_int_vars");
   if ( ! this->enforcing_domain_bounds )
      return no_bound;
   return int_lower_bound_types.expose<BoundTypeArray>()[i];
}


/// Returns the type of the int upper bound on the $i$-th variable
bound_type_enum
Application_IntDomain::intUpperBoundType(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "intUpperBoundType(): index past num_int_vars");
   if ( ! this->enforcing_domain_bounds )
      return no_bound;
   return int_upper_bound_types.expose<BoundTypeArray>()[i];
}


/// Return the label for a specific int variable
std::string 
Application_IntDomain::intLabel(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "intLabel(): index past num_int_vars");
   return int_labels.expose<labels_t>().left.at(i);
}


/// Returns true if this problem has a lower bound on the $i$-th variable
bool
Application_IntDomain::hasIntLowerBound(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "hasIntLowerBound(): index past num_int_vars");
   return this->enforcing_domain_bounds && 
       int_lower_bound_types.expose<BoundTypeArray>()[i] != no_bound;
}


/// Returns true if this problem has a upper bound on the $i$-th variable
bool
Application_IntDomain::hasIntUpperBound(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "hasIntUpperBound(): index past num_int_vars");
   return this->enforcing_domain_bounds && 
       int_upper_bound_types.expose<BoundTypeArray>()[i] != no_bound;
}


/// Returns true if the bounds on the $i$-th int variable are periodic
bool
Application_IntDomain::hasPeriodicIntBound(size_t i) const
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "hasPeriodicIntBound(): index past num_int_vars");
   return this->enforcing_domain_bounds &&
      ( int_lower_bound_types.expose<BoundTypeArray>()[i] == periodic_bound );
}


/// Return the label for a specific binary variable
std::string 
Application_IntDomain::binaryLabel(size_t i) const
{
   if ( num_binary_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "binaryLabel(): index past num_binary_vars");
   return binary_labels.expose<labels_t>().left.at(i);
}


///
bool 
Application_IntDomain::finiteBoundConstraints() const
{
   if ( ! this->enforcing_domain_bounds )
      return false; 

   size_t numv = num_int_vars;
   const int_domain_t &lb = int_lower_bounds.expose<int_domain_t>();
   const int_domain_t &ub = int_upper_bounds.expose<int_domain_t>();
   const BoundTypeArray &lbt = int_lower_bound_types.expose<BoundTypeArray>();
   const BoundTypeArray &ubt = int_upper_bound_types.expose<BoundTypeArray>();
   for (size_t i = 0; i < numv; i++)
   {
      if ( lbt[i] == no_bound || ubt[i] == no_bound )
         return false;
      if ( lb[i] <= INT_MIN || ub[i] >= INT_MAX )
         return false;
   }
   return true;
}


/// Returns true if the point is feasible with respect to bound constraints
bool
Application_IntDomain::testBoundsFeasibility(const utilib::Any& point)
{
   Domain::IntegerComponent vals;
   utilib::TypeManager()->lexical_cast(point, vals);
   if ( num_int_vars != vals.size() )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_IntDomain::testBoundsFeasibility(): "
                     "domain size (" << vals.size() << 
                     ") does not match num_int_vars (" << num_int_vars 
                     << ")");

   if ( ! this->enforcing_domain_bounds )
      return true;

   const int_domain_t &lb = int_lower_bounds.expose<int_domain_t>();
   const int_domain_t &ub = int_upper_bounds.expose<int_domain_t>();
   const BoundTypeArray &lbt = int_lower_bound_types.expose<BoundTypeArray>();
   const BoundTypeArray &ubt = int_upper_bound_types.expose<BoundTypeArray>();
   for (size_t i = 0; i < vals.size(); i++)
   {
      if ( lbt[i] == hard_bound && lb[i] > vals[i] )
         return false;
      if ( ubt[i] == hard_bound && ub[i] < vals[i] )
         return false;
   }
   return true;
}


ApplicationHandle
Application_IntDomain::relaxed_application() const
{
   return ApplicationHandle();
}

//============================================================================
//============================================================================
// Class Application_IntDomain protected member definitions
//============================================================================
//============================================================================


/// Sets the type of a int lower bound
void
Application_IntDomain::_setIntLowerBoundType(size_t i, bound_type_enum type)
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "_setIntLowerBoundType(): index past num_int_vars");

   BoundTypeArray tmp = int_lower_bound_types;
   tmp[i] = type;
   _int_lower_bound_types = tmp;
}


/// Sets the type of a int upper bound
void
Application_IntDomain::_setIntUpperBoundType(size_t i, bound_type_enum type)
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "_setIntUpperBoundType(): index past num_int_vars");

   BoundTypeArray tmp = int_upper_bound_types;
   tmp[i] = type;
   _int_upper_bound_types = tmp;
}


/// Indicates that the bounds on the $i$-th int variable are periodic
void
Application_IntDomain::_setPeriodicIntBound(size_t i)
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "_setPeriodicIntBound(): index past num_int_vars");

   BoundTypeArray tmp = int_lower_bound_types;
   tmp[i] = periodic_bound;
   _int_lower_bound_types = tmp;

   tmp = int_upper_bound_types;
   tmp[i] = periodic_bound;
   _int_upper_bound_types = tmp;
}


///
void
Application_IntDomain::
_setIntLabel(const size_t i, const std::string &label)
{
   if ( num_int_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "_setIntLabel(): index past num_int_vars");

   labels_t tmp = int_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _int_labels = tmp;
}


///
void
Application_IntDomain::
_setBinaryLabel(const size_t i, const std::string &label)
{
   if ( num_binary_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "_setBinaryLabel(): index past num_binary_vars");

   labels_t tmp = binary_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _binary_labels = tmp;
}


//============================================================================
//============================================================================
// Class Application_IntDomain private methods
//============================================================================
//============================================================================

///
void
Application_IntDomain::
cb_onChange_int( const utilib::ReadOnly_Property &prop )
{
   size_t nvars = prop;

   // this sets any newly-added int variables to have +/-Inf bounds
   // (without changing any already-configured bounds).
   int_domain_t bounds = int_lower_bounds;
   if ( nvars == bounds.size() )
      return;

   bounds.resize(nvars, INT_MIN);
   _int_lower_bounds = bounds;

   bounds = int_upper_bounds;
   bounds.resize(nvars, INT_MAX);
   _int_upper_bounds = bounds;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = int_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= nvars )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _int_labels = labels;
}


///
void
Application_IntDomain::
cb_onChange_binary( const utilib::ReadOnly_Property &prop )
{
   size_t nvars = prop;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = binary_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= nvars )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _binary_labels = labels;
}


/// Set int boundary constraints - dense format
void
Application_IntDomain::
cb_onChange_bounds( const utilib::ReadOnly_Property &prop )
{
   size_t nvars = num_int_vars;

   utilib::Privileged_Property& bound_type_property = 
      ( prop.equivalentTo(_int_lower_bounds) 
        ? _int_lower_bound_types : _int_upper_bound_types );

   const int_domain_t &bounds = prop.expose<int_domain_t>();
   BoundTypeArray bound_types = bound_type_property;
   // NB: EnumBitArray does not allow specifying the default value for
   // newly-created elements, so this is more complicated than it needs
   // to be.
   size_t old_size = bound_types.size();
   bound_types.resize(nvars);
   for(; old_size < nvars; bound_types[old_size++] = no_bound );

   for ( size_t i = 0; i < nvars; ++i )
   {
      if ( bounds[i] > INT_MIN && bounds[i] < INT_MAX )
      {
         if ( bound_types[i] == no_bound )
            bound_types[i] = hard_bound;
      }
      else
      {
         if ( bound_types[i] == periodic_bound )
         {
            // clearing one side of a periodic bound must clear the
            // periodic flag from the other side
            utilib::Privileged_Property& opposite_property = 
               prop.equivalentTo(_int_lower_bounds)
               ? _int_upper_bound_types : _int_lower_bound_types;
            BoundTypeArray opposite_types = opposite_property;
            opposite_types[i] = hard_bound;
            opposite_property = opposite_types;
         }
         bound_types[i] = no_bound;
      }
   }

   bound_type_property = bound_types;
}


/// Set int boundary constraint types - dense format
void
Application_IntDomain::
cb_onChange_bound_types(const utilib::ReadOnly_Property &prop)
{
   const BoundTypeArray &bound_types = prop.expose<BoundTypeArray>();
   utilib::Privileged_Property &bounds_prop =
      ( prop.equivalentTo(_int_lower_bound_types) 
        ? _int_lower_bounds 
        : _int_upper_bounds );
   const int_domain_t &bounds = bounds_prop.expose<int_domain_t>();

   size_t nvars = bound_types.size();
   bool flag = false;
   for (size_t i = 0; i < nvars; i++)
   {
      bool is_finite = bound_types[i] != no_bound;
      flag |= is_finite;
      if ( is_finite != (bounds[i] > INT_MIN && bounds[i] < INT_MAX) )
      {
         // update the bound *values* if someone set no_bound
         int_domain_t _bounds = bounds;
         int inf = prop.equivalentTo(_int_lower_bound_types)
            ? INT_MIN : INT_MAX;
         for (size_t j = i; j < nvars; j++)
            if ( bound_types[j] == no_bound && bounds[j] != inf )
               _bounds[j] = inf;
         
         *bounds_prop.allowOnChangeRecursion()->property() = _bounds;
         return; // This callback has already been re-triggered to
                 // recompute the enforcing_domain_bounds flag
      }
   }
   this->_enforcing_domain_bounds = this->enforcing_domain_bounds || flag;
}

bool
Application_IntDomain::
cb_validate_bounds( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);
   size_t nvars = value.expose<int_domain_t>().size();
   bool ok = num_int_vars == nvars;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "cb_validate_bounds(): vector length (" << nvars <<
                     ") does not match num_int_vars (" 
                     << num_int_vars << ")");
   return ok;
}


bool
Application_IntDomain::
cb_validate_bound_types( const utilib::ReadOnly_Property &prop,
                         const utilib::Any &value )
{
   size_t nvars = value.expose<BoundTypeArray>().size();

   bool ok = num_int_vars == nvars;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "cb_validate_vector(): vector length (" << nvars <<
                     ") does not match num_int_vars (" 
                     << num_int_vars << ")");

   const BoundTypeArray &bound_types = value.expose<BoundTypeArray>();
   const int_domain_t &bounds =
      ( prop.equivalentTo(_int_lower_bound_types) 
        ? _int_lower_bounds 
        : _int_upper_bounds ).expose<int_domain_t>();
   
   for (size_t j = 0; j < nvars; j++)
      if ( bound_types[j] != no_bound && 
           (bounds[j] <= INT_MIN || bounds[j] >= INT_MAX) )
      {
         ok = false;
         EXCEPTION_MNGR(std::logic_error, "Application_IntDomain::"
                        "cb_validate_bound_types(): unsetting no_bound "
                        "on an infinite bound (index=" << j << ").");
      }
   return ok;
}


bool
Application_IntDomain::
cb_validate_labels( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   const labels_t &labels = value.expose<labels_t>();

   if ( labels.empty() )
      return true;

   size_t nvars = 
      prop.equivalentTo(_int_labels) ? num_int_vars : num_binary_vars;
   if ( nvars <= labels.left.rbegin()->first )
      EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                     "cb_validate_labels(): label id (" << 
                     labels.left.rbegin()->first << ") is greater than " <<
                     ( prop.equivalentTo(_int_labels) ? 
                       string("num_int_vars") : string("num_binary_vars") )
                     << " (" << nvars << ")");
   return true;
}


///
void 
Application_IntDomain::
cb_print(std::ostream& os)
{
   os << "Integer variables: " << num_int_vars << std::endl;

   if ( num_int_vars > 0 )
   {
      const labels_t &labels = int_labels.expose<labels_t>();
      
      labels_t::left_const_iterator it;
      labels_t::left_const_iterator itEnd = labels.left.end();

      os << "Index " << std::setw(12) 
         << "Label" << std::setw(20) 
         //<< "Fixed Val" << std::setw(20)
         << "Lower Bound T" << std::setw(20) 
         << "Upper Bound T" << std::endl;
      for (size_t i = 0; num_int_vars > i; i++)
      {
         //
         // Index
         //
         os << std::setw(5) << (i + 1) << " ";
         //
         // Label
         //
         it = labels.left.find(i);
         if (it != itEnd)
            os << std::setw(12) << it->second << " ";
         else
            os << std::setw(12) << "_none_" << " ";
         //
         // Fixed Value
         //
         //fixed_it = get_fixed_int_vars().find(i);
         //if (fixed_it != fixed_itEnd)
         //{
         //   os << std::setw(19) << std::setprecision(4) 
         //      << fixed_it->second << " ";
         //}
         //else
         //{
         //   os << std::setw(19) << "_none_" << " ";
         //}
         //
         // Bounds Info
         //
         char c = ' ';
         convert_to_char( intLowerBoundType(i), c );
         if ( intLowerBound(i) <= INT_MIN )
            os << " " << std::setw(17) << "-Infinity " << c << " ";
         else
            os << std::setw(17) << std::setprecision(4) 
               << intLowerBound(i) << " " << c << " ";
         
         convert_to_char( intUpperBoundType(i), c );
         if ( intUpperBound(i) >= INT_MAX)
            os << " " << std::setw(17) << "Infinity " << c << " ";
         else
            os << std::setw(17) << std::setprecision(4) 
               << intUpperBound(i) << " " << c << " ";
         #if 0
         // This SHOULD be unreachable
         else
         {
            os << std::setw(17) << "-Infinity" << " * ";
            os << std::setw(17) << " Infinity" << " * ";
         }
         #endif
         os << std::endl;
      }
   }

   os << "Binary variables:  " << num_binary_vars << std::endl;
   if ( num_binary_vars > 0 )
   {
      const labels_t &labels = binary_labels.expose<labels_t>();
      
      labels_t::left_const_iterator it;
      labels_t::left_const_iterator itEnd = labels.left.end();

      os << "Index " << std::setw(12) 
         << "Label" << std::setw(20) 
         //<< "Fixed Val" 
         << std::endl;
      for (size_t i = 0; num_binary_vars > i; i++)
      {
         //
         // Index
         //
         os << std::setw(5) << (i + 1) << " ";
         //
         // Label
         //
         it = labels.left.find(i);
         if (it != itEnd)
            os << std::setw(12) << it->second << " ";
         else
            os << std::setw(12) << "_none_" << " ";
         //
         // Fixed Value
         //
         //fixed_it = get_fixed_binary_vars().find(i);
         //if (fixed_it != fixed_itEnd)
         //{
         //   os << std::setw(19) << std::setprecision(4) << fixed_it->second;
         //}
         //else
         //{
         //   os << std::setw(19) << "_none_";
         //}
         os << std::endl;
      }
   }

}

///
void 
Application_IntDomain::
cb_initialize_binary(TiXmlElement* elt)
{
   size_t size = 0;
   utilib::get_num_attribute(elt, "num", size);
   _num_binary_vars = size;

   //
   // Process "Labels" elements
   //
   TiXmlElement *node = elt->FirstChildElement("Labels");
   if ( node != NULL )
   {
      std::vector<std::string> labels;
      std::istringstream istr(node->GetText());
      utilib::Any any(labels, true, true);
      parse_array<std::string, std::vector<std::string> > (istr, any);
      if ( labels.size() != size )
      {
         EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                        "Implementation::init_binary_vars: parsed " <<
                        labels.size() << " labels but there are " <<
                        size << " binary variables");
      }

      labels_t tmp;
      for (size_t i = 0; i < labels.size(); i++)
         tmp.insert(labels_t::value_type(i, labels[i]));

      _binary_labels = tmp;
   }
}


///
void 
Application_IntDomain::
cb_initialize_int(TiXmlElement* elt)
{  
   size_t size = 0;
   utilib::get_num_attribute(elt, "num", size);
   _num_int_vars = size;

   //
   // Process "Labels" elements
   //
   TiXmlElement* node = elt->FirstChildElement("Labels");
   for ( ; node != NULL; node = node->NextSiblingElement("Labels"))
   {
      std::vector<std::string> labels;
      std::istringstream istr(node->GetText());
      utilib::Any any(labels, true, true);
      parse_array<std::string, std::vector<std::string> > (istr, any);
      if ( labels.size() != size )
      {
         EXCEPTION_MNGR(std::runtime_error, "Application_IntDomain::"
                        "xml_initialize_IntDomain: parsed " <<
                        labels.size() << " labels but there are " <<
                        size << " integer variables");
      }

      labels_t tmp;
      for (size_t i = 0; i < labels.size(); i++)
         tmp.insert(labels_t::value_type(i, labels[i])); 

      _int_labels = tmp;
   }

   //
   // Process "Lower"  and "Upper" elements
   //
   int_domain_t lower         = int_lower_bounds;
   int_domain_t upper         = int_upper_bounds;
   BoundTypeArray lower_type  = int_lower_bound_types;
   BoundTypeArray upper_type  = int_upper_bound_types;
   const labels_t& labels     = _int_labels.expose<labels_t>();

   process_bounds( elt, lower, upper, lower_type, upper_type, labels,
                   INT_MIN, INT_MAX );

   // NB: set the types before the bounds: the bounds onchange will
   // update the no/hard bound marker.
   _int_lower_bounds = lower;
   _int_upper_bounds = upper;
   _int_lower_bound_types = lower_type;
   _int_upper_bound_types = upper_type;
}

} // namespace colin
