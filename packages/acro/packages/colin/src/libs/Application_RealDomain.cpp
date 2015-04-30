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
 * \file Application_RealDomain.cpp
 *
 * Defines the colin::Application_RealDomain class member functions
 */

#include <colin/application/RealDomain.h>
#include <colin/TinyXML.h>
#include <colin/BoundTypeArray.h>

#include <boost/bimap.hpp>

using utilib::Property;
using utilib::Privileged_Property;

using boost::bind;

using std::vector;
using std::string;
using std::cerr;
using std::endl;

namespace colin
{
namespace {

typedef boost::bimap<size_t, std::string>  labels_t;
typedef std::vector<colin::real>           domain_t;

} // namespace colin::(local)


//============================================================================
//============================================================================
// Class Application_RealDomain
//============================================================================
//============================================================================

/// Constructor
Application_RealDomain::Application_RealDomain()
   : _num_real_vars(Property::Bind<size_t>()),
     _real_lower_bounds(Property::Bind<domain_t>()),
     _real_upper_bounds(Property::Bind<domain_t>()),
     _real_lower_bound_types(Property::Bind<BoundTypeArray>()),
     _real_upper_bound_types(Property::Bind<BoundTypeArray>()),
     _real_labels(Property::Bind<labels_t>()),
     num_real_vars(_num_real_vars.set_readonly()),
     real_lower_bounds(_real_lower_bounds.set_readonly()),
     real_upper_bounds(_real_upper_bounds.set_readonly()),
     real_lower_bound_types(_real_lower_bound_types.set_readonly()),
     real_upper_bound_types(_real_upper_bound_types.set_readonly()),
     real_labels(_real_labels.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare("num_real_vars", _num_real_vars, t);
   properties.declare("real_lower_bounds", _real_lower_bounds, t);
   properties.declare("real_upper_bounds", _real_upper_bounds, t);
   properties.declare("real_lower_bound_types", _real_lower_bound_types, t);
   properties.declare("real_upper_bound_types", _real_upper_bound_types, t);
   properties.declare("real_labels", _real_labels, t);

   _num_real_vars.onChange().connect
      (bind(&Application_RealDomain::cb_onChange_nvars, this, _1));

   _real_lower_bounds.onChange().connect
      (bind(&Application_RealDomain::cb_onChange_bounds, this, _1));
   _real_upper_bounds.onChange().connect
      (bind(&Application_RealDomain::cb_onChange_bounds, this, _1));

   _real_lower_bound_types.onChange().connect
      (bind(&Application_RealDomain::cb_onChange_bound_types, this, _1));
   _real_upper_bound_types.onChange().connect
      (bind(&Application_RealDomain::cb_onChange_bound_types, this, _1));

   _real_lower_bounds.validate().connect
      (bind(&Application_RealDomain::cb_validate_bounds, this, _1, _2));
   _real_upper_bounds.validate().connect
      (bind(&Application_RealDomain::cb_validate_bounds, this, _1, _2));
   _real_lower_bound_types.validate().connect
      (bind(&Application_RealDomain::cb_validate_bound_types, this, _1, _2));
   _real_upper_bound_types.validate().connect
      (bind(&Application_RealDomain::cb_validate_bound_types, this, _1, _2));

   _real_labels.validate().connect
      (bind(&Application_RealDomain::cb_validate_labels, this, _1, _2));

   print_signal.connect(11, boost::bind(&Application_RealDomain::cb_print, this, _1));

   domain_initializer("RealVars").connect
      (boost::bind(&Application_RealDomain::cb_initialize, this, _1));

   // notify Application_Domain about our part of the domain
   get_domain_size_signal.connect
      ( bind(&utilib::ReadOnly_Property::as<size_t>, &this->num_real_vars) );

   // propagate num_real_vars::onChange to domain_size
   _num_real_vars.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &domain_size.onChange(), domain_size));


   // initialize everything (relying on the callbacks)
   _num_real_vars = 0;
}


/// Virtual destructor
Application_RealDomain::~Application_RealDomain()
{ }


/// Returns the lower bound on the $i$-th variable
real
Application_RealDomain::realLowerBound(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "realLowerBound(): index past num_real_vars");
   if ( ! this->enforcing_domain_bounds || realLowerBoundType(i) == no_bound )
      return real::negative_infinity;
   return real_lower_bounds.expose<domain_t>()[i];
}


/// Returns the upper bound on the $i$-th variable
real
Application_RealDomain::realUpperBound(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "realUpperBound(): index past num_real_vars");
   if ( ! this->enforcing_domain_bounds || realUpperBoundType(i) == no_bound )
      return real::positive_infinity;
   return real_upper_bounds.expose<domain_t>()[i];
}


/// Returns the type of the real lower bound on the $i$-th variable
bound_type_enum
Application_RealDomain::realLowerBoundType(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "realLowerBoundType(): index past num_real_vars");
   if ( ! this->enforcing_domain_bounds )
      return no_bound;
   return real_lower_bound_types.expose<BoundTypeArray>()[i];
}


/// Returns the type of the real upper bound on the $i$-th variable
bound_type_enum
Application_RealDomain::realUpperBoundType(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "realUpperBoundType(): index past num_real_vars");
   if ( ! this->enforcing_domain_bounds )
      return no_bound;
   return real_upper_bound_types.expose<BoundTypeArray>()[i];
}


/// Return the label for a specific real variable
std::string 
Application_RealDomain::realLabel(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "realLabel(): index past num_real_vars");
   const labels_t::left_map &labels = real_labels.expose<labels_t>().left;
   labels_t::left_map::const_iterator it = labels.find(i);
   if ( it != labels.end() )
      return it->second;
   else
      return "";
}


/// Returns true if this problem has a lower bound on the $i$-th variable
bool
Application_RealDomain::hasRealLowerBound(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "hasRealLowerBound(): index past num_real_vars");
   return this->enforcing_domain_bounds &&
      real_lower_bound_types.expose<BoundTypeArray>()[i] != no_bound;
}


/// Returns true if this problem has a upper bound on the $i$-th variable
bool
Application_RealDomain::hasRealUpperBound(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "hasRealUpperBound(): index past num_real_vars");
   return this->enforcing_domain_bounds &&
      real_upper_bound_types.expose<BoundTypeArray>()[i] != no_bound;
}


/// Returns true if the bounds on the $i$-th real variable are periodic
bool
Application_RealDomain::hasPeriodicRealBound(size_t i) const
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "hasPeriodicRealBound(): index past num_real_vars");
   return this->enforcing_domain_bounds &&
      ( real_lower_bound_types.expose<BoundTypeArray>()[i] == periodic_bound );
}


///
bool
Application_RealDomain::finiteBoundConstraints() const
{
   if ( ! this->enforcing_domain_bounds )
      return false;

   size_t numv = num_real_vars;
   const domain_t &lb = real_lower_bounds.expose<domain_t>();
   const domain_t &ub = real_upper_bounds.expose<domain_t>();
   const BoundTypeArray &lbt = real_lower_bound_types.expose<BoundTypeArray>();
   const BoundTypeArray &ubt = real_upper_bound_types.expose<BoundTypeArray>();
   for (size_t i = 0; i < numv; i++)
   {
      if ( lbt[i] == no_bound || ubt[i] == no_bound )
         return false;
      if ( !finite(lb[i]) || !finite(ub[i]) )
         return false;
   }
   return true;
}


/// Returns true if the point is feasible with respect to bound constraints
bool
Application_RealDomain::testBoundsFeasibility(const utilib::Any& real_domain)
{
   Domain::RealComponent vals;
   utilib::TypeManager()->lexical_cast(real_domain, vals);
   if ( num_real_vars != vals.size() )
      EXCEPTION_MNGR(std::runtime_error,
                     "Application_RealDomain::testBoundsFeasibility(): "
                     "domain size (" << vals.size() << 
                     ") does not match num_real_vars (" << num_real_vars 
                     << ")");

   if ( ! this->enforcing_domain_bounds )
      return true;
   
   const domain_t &lb = real_lower_bounds.expose<domain_t>();
   const domain_t &ub = real_upper_bounds.expose<domain_t>();
   const BoundTypeArray &lbt = real_lower_bound_types.expose<BoundTypeArray>();
   const BoundTypeArray &ubt = real_upper_bound_types.expose<BoundTypeArray>();
   for (size_t i = 0; i < vals.size(); i++)
   {
      if ( lbt[i] == hard_bound && lb[i] > vals[i] )
         return false;
      if ( ubt[i] == hard_bound && ub[i] < vals[i] )
         return false;
   }
   return true;
}

//============================================================================
//============================================================================
// Class Application_RealDomain protected methods
//============================================================================
//============================================================================


/// Sets the type of a real lower bound
void
Application_RealDomain::_setRealLowerBoundType(size_t i, bound_type_enum type)
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "_setRealLowerBoundType(): index past num_real_vars");

   BoundTypeArray tmp = real_lower_bound_types;
   tmp[i] = type;
   _real_lower_bound_types = tmp;
}


/// Sets the type of a real upper bound
void
Application_RealDomain::_setRealUpperBoundType(size_t i, bound_type_enum type)
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "_setRealUpperBoundType(): index past num_real_vars");

   BoundTypeArray tmp = real_upper_bound_types;
   tmp[i] = type;
   _real_upper_bound_types = tmp;
}


/// Indicates that the bounds on the $i$-th real variable are periodic
void
Application_RealDomain::_setPeriodicRealBound(size_t i)
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "_setPeriodicRealBound(): index past num_real_vars");

   BoundTypeArray tmp = real_lower_bound_types;
   tmp[i] = periodic_bound;
   _real_lower_bound_types = tmp;

   tmp = real_upper_bound_types;
   tmp[i] = periodic_bound;
   _real_upper_bound_types = tmp;
}


///
void
Application_RealDomain::
_setRealLabel(const size_t i, const std::string &label)
{
   if ( num_real_vars <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "_setRealLabel(): index past num_real_vars");

   labels_t tmp = real_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _real_labels = tmp;
}


//============================================================================
//============================================================================
// Class Application_RealDomain private methods
//============================================================================
//============================================================================

///
void
Application_RealDomain::
cb_onChange_nvars( const utilib::ReadOnly_Property &prop )
{
   size_t nvars = prop;

   // this sets any newly-added real variables to have +/-Inf bounds
   // (without changing any already-configured bounds).
   domain_t bounds = real_lower_bounds;
   if ( nvars == bounds.size() )
      return;

   bounds.resize(nvars, real::negative_infinity);
   _real_lower_bounds = bounds;

   bounds = real_upper_bounds;
   bounds.resize(nvars, real::positive_infinity);
   _real_upper_bounds = bounds;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = real_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= nvars )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _real_labels = labels;
}

/// Set real boundary constraints - dense format
void
Application_RealDomain::
cb_onChange_bounds( const utilib::ReadOnly_Property &prop )
{
   size_t nvars = num_real_vars;

   utilib::Privileged_Property& bound_type_property = 
      prop.equivalentTo(_real_lower_bounds)
      ? _real_lower_bound_types : _real_upper_bound_types;

   const domain_t &bounds = prop.expose<domain_t>();
   BoundTypeArray bound_types = bound_type_property;
   // NB: EnumBitArray does not allow specifying the default value for
   // newly-created elements, so this is more complicated than it needs
   // to be.
   if ( bound_types.size() != nvars )
   {
      size_t old_size = bound_types.size();
      bound_types.resize(nvars);
      for(; old_size < nvars; bound_types[old_size++] = no_bound );
   }

   for ( size_t i = 0; i < nvars; ++i )
   {
      if ( finite(bounds[i]) )
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
               prop.equivalentTo(_real_lower_bounds)
               ? _real_upper_bound_types : _real_lower_bound_types;
            BoundTypeArray opposite_types = opposite_property;
            opposite_types[i] = hard_bound;
            opposite_property = opposite_types;
         }
         bound_types[i] = no_bound;
      }
   }

   bound_type_property = bound_types;
}


/// Set real boundary constraint types - dense format
void
Application_RealDomain::
cb_onChange_bound_types(const utilib::ReadOnly_Property &prop)
{
   const BoundTypeArray &bound_types = prop.expose<BoundTypeArray>();
   utilib::Privileged_Property &bounds_prop =
      ( prop.equivalentTo(_real_lower_bound_types) 
        ? _real_lower_bounds 
        : _real_upper_bounds );
   const domain_t &bounds = bounds_prop.expose<domain_t>();

   size_t nvars = bound_types.size();
   bool flag = false;
   for (size_t i = 0; i < nvars; i++)
   {
      bool is_finite = bound_types[i] != no_bound;
      flag |= is_finite;
      if ( is_finite != finite(bounds[i]) )
      {
         // update the bound *values* if someone set no_bound
         domain_t _bounds = bounds;
         real inf = prop.equivalentTo(_real_lower_bound_types)
            ? real::negative_infinity : real::positive_infinity;
         for (size_t j = i; j < nvars; j++)
            if ( bound_types[j] == no_bound && finite(bounds[j]) )
               _bounds[j] = inf;
         
         *bounds_prop.allowOnChangeRecursion()->property() = _bounds;
         return; // This callback has already been re-triggered to
                 // recompute the enforcing_domain_bounds flag
      }
   }

   this->_enforcing_domain_bounds = this->enforcing_domain_bounds || flag;
}

bool
Application_RealDomain::
cb_validate_bounds( const utilib::ReadOnly_Property &prop,
                   const utilib::Any &value )
{
   static_cast<void>(prop);
   size_t nvars = value.expose<domain_t>().size();
   bool ok = num_real_vars == nvars;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "cb_validate_bounds(): vector length (" << nvars <<
                     ") does not match num_real_vars (" 
                     << num_real_vars << ")");
   return ok;
}


bool
Application_RealDomain::
cb_validate_bound_types( const utilib::ReadOnly_Property &prop,
                        const utilib::Any &value )
{
   size_t nvars = value.expose<BoundTypeArray>().size();

   bool ok = num_real_vars == nvars;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "cb_validate_bound_types(): vector length (" << nvars <<
                     ") does not match num_real_vars (" 
                     << num_real_vars << ")");

   const BoundTypeArray &bound_types = value.expose<BoundTypeArray>();
   const domain_t &bounds =
      ( prop.equivalentTo(_real_lower_bound_types) 
        ? _real_lower_bounds 
        : _real_upper_bounds ).expose<domain_t>();
   
   for (size_t j = 0; j < nvars; j++)
      if ( bound_types[j] != no_bound && ! finite(bounds[j]) )
      {
         ok = false;
         EXCEPTION_MNGR(std::logic_error, "Application_RealDomain::"
                        "cb_validate_bound_types(): unsetting no_bound "
                        "on an infinite bound (index=" << j << ").");
      }
   return ok;
}


bool
Application_RealDomain::
cb_validate_labels( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);
   const labels_t &labels = value.expose<labels_t>();

   if ( labels.empty() )
      return true;

   if ( num_real_vars <= labels.left.rbegin()->first )
      EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                     "cb_validate_labels(): label id (" << 
                     labels.left.rbegin()->first <<
                     ") is greater than num_real_vars (" 
                     << num_real_vars << ")");
   return true;
}


///
void
Application_RealDomain::
cb_print(std::ostream& os)
{
   os << "Real variables:    " << num_real_vars << std::endl;
   if (num_real_vars == 0) return;

   const labels_t &labels = real_labels.expose<labels_t>();

   labels_t::left_const_iterator it;
   labels_t::left_const_iterator itEnd = labels.left.end();

   os << "Index " << std::setw(12)
      << "Label" << std::setw(20)
      //<< "Fixed Val" << std::setw(20)
      << "Lower Bound T" << std::setw(20)
      << "Upper Bound T" << std::endl;

   for (size_t i = 0; num_real_vars > i; i++)
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
      //fixed_it = get_fixed_real_vars().find(i);
      //if (fixed_it != fixed_itEnd)
      //{
      //   os << std::setw(19) << std::setprecision(4)
      //   << fixed_it->second << " ";
      //}
      //else
      //{
      //   os << std::setw(19) << "_none_" << " ";
      //}
      //
      // Bounds Info
      //
      char c = ' ';
      convert_to_char( realLowerBoundType(i), c );
      os << std::setw(17) << std::setprecision(4)
         << realLowerBound(i) << " " << c << " ";

      convert_to_char( realUpperBoundType(i), c );
      os << std::setw(17) << std::setprecision(4)
         << realUpperBound(i) << " " << c << " ";
      os << std::endl;
   }
}


///
void
Application_RealDomain::
cb_initialize(TiXmlElement* elt)
{
   size_t size = 0;
   utilib::get_num_attribute(elt, "num", size);
   _num_real_vars = size;

   //
   // Process "Labels" elements
   //
   TiXmlElement* node = elt->FirstChildElement("Labels");
   if ( node != NULL )
   {
      std::vector<std::string> labels;
      std::istringstream istr(node->GetText());
      utilib::Any any(labels, true, true);
      parse_array<std::string, std::vector<std::string> > (istr, any);
      if (labels.size() != size)
      {
         EXCEPTION_MNGR(std::runtime_error, "Application_RealDomain::"
                        "xml_initialize_RealDomain: parsed " << labels.size()
                        << " labels but there are "
                        << size << " real variables");
      }

      labels_t tmp;
      for (size_t i = 0; i < labels.size(); i++)
         tmp.insert(labels_t::value_type(i, labels[i]));

      _real_labels = tmp;
   }

   //
   // Process "Lower"  and "Upper" elements
   //
   domain_t lower              = real_lower_bounds;
   domain_t upper              = real_upper_bounds;
   BoundTypeArray lower_type   = real_lower_bound_types;
   BoundTypeArray upper_type   = real_upper_bound_types;
   const labels_t& labels      = real_labels.expose<labels_t>();
   
   process_bounds( elt, lower, upper, lower_type, upper_type, labels,
                   real::negative_infinity, real::positive_infinity );

   // NB: set the types before the bounds: the bounds onchange will
   // update the no/hard bound marker.
   _real_lower_bounds = lower;
   _real_upper_bounds = upper;
   _real_lower_bound_types = lower_type;
   _real_upper_bound_types = upper_type;
}


} // namespace colin

