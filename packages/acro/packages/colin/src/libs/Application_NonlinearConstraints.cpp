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
 * \file Application_NonlinearConstraints.cpp
 *
 * Defines the colin::Application_NonlinearConstraints class members.
 */

#include <colin/application/NonlinearConstraints.h>

#include <colin/BoundTypeArray.h>

#include <colin/TinyXML.h>
#include <colin/real.h>

#include <utilib/SparseMatrix.h>

#include <boost/bimap.hpp>

using std::cerr;
using std::endl;
using std::vector;
using std::list;
using std::make_pair;

using boost::bind;

using utilib::seed_t;
using utilib::Any;
using utilib::AnyRef;
using utilib::TypeManager;
using utilib::Property;


namespace colin {

namespace {

typedef boost::bimap<size_t, std::string>  labels_t;
typedef std::vector<colin::real>           bound_t;
typedef utilib::RMSparseMatrix<double>     grad_t;

inline bool isEquality( size_t i,
                        const bound_t & lb,
                        const bound_t & ub,
                        //const BoundTypeArray &lbt;
                        //const BoundTypeArray &ubt;
                        const double &EPS)
{
   return (//( ubt[i] != no_bound ) && ( lbt[i] != no_bound ) &&
           ( ub[i] - lb[i] <= EPS ));
}

size_t splitConstraint( const Application_NonlinearConstraints& me, 
                        const bound_t &whole, bound_t &part, 
                        bool equality, double EPS )
{
   const bound_t& lb = me.nonlinear_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = me.nonlinear_constraint_upper_bounds.expose<bound_t>();
   /*
   const BoundTypeArray& lbt = 
      me.linear_constraint_lower_bound_types.expose<BoundTypeArray>();
   const BoundTypeArray& ubt = 
      me.linear_constraint_upper_bound_types.expose<BoundTypeArray>();
   */

   size_t max = whole.size();
   part.clear();
   part.reserve(max);
   for( size_t i = 0; i < max; ++i )
   {
      if ( isEquality(i, lb, ub, /*lbt, ubt,*/ EPS) == equality )
         part.push_back(whole[i]);
   }

   return part.size();
}

} // namespace colin::(local)



//--------------------------------------------------------------------
// Application_NonlinearConstraints Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_NonlinearConstraints::Application_NonlinearConstraints()
   : _num_nonlinear_constraints(Property::Bind<size_t>()),
     _nonlinear_constraint_lower_bounds(Property::Bind<bound_t>()),
     _nonlinear_constraint_upper_bounds(Property::Bind<bound_t>()),
     _nonlinear_constraint_labels(Property::Bind<labels_t>()),
     num_nonlinear_constraints(_num_nonlinear_constraints.set_readonly()),
     nonlinear_constraint_lower_bounds(_nonlinear_constraint_lower_bounds.set_readonly()),
     nonlinear_constraint_upper_bounds(_nonlinear_constraint_upper_bounds.set_readonly()),
     nonlinear_constraint_labels(_nonlinear_constraint_labels.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare( "num_nonlinear_constraints", 
                       _num_nonlinear_constraints, t );
   properties.declare( "nonlinear_constraint_lower_bounds",
                       _nonlinear_constraint_lower_bounds, t );
   properties.declare( "nonlinear_constraint_upper_bounds",
                       _nonlinear_constraint_upper_bounds, t );
   properties.declare( "nonlinear_constraint_labels", 
                       _nonlinear_constraint_labels, t );


   _num_nonlinear_constraints.onChange().connect
      ( bind(&Application_NonlinearConstraints::cb_onChange_num, this, _1) );

   _nonlinear_constraint_lower_bounds.validate().connect
      ( bind(&Application_NonlinearConstraints::cb_validate_vector, this, 
             _1, _2) );
   _nonlinear_constraint_upper_bounds.validate().connect
      ( bind(&Application_NonlinearConstraints::cb_validate_vector, this, 
             _1, _2) );

   _nonlinear_constraint_labels.validate().connect
      ( bind(&Application_NonlinearConstraints::cb_validate_labels, this, 
             _1, _2) );


   print_signal.connect
      (11, boost::bind(&Application_NonlinearConstraints::cb_print, this, _1));

   constraint_initializer("Nonlinear").connect
      (bind( &Application_NonlinearConstraints::cb_initialize, 
             this, _1 ));


   request_expansion_signal.connect
      (boost::bind( &Application_NonlinearConstraints::cb_expand_request, 
             this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_NonlinearConstraints::cb_map_request, 
             this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_NonlinearConstraints::cb_map_response, 
             this, _1, _2, _3, _4 ));

   constraint_request_signal.connect
      (boost::bind( &Application_NonlinearConstraints::cb_constraint_request,
             this, _1 ));

   constraint_info_signal.connect
      (boost::bind( &Application_NonlinearConstraints::cb_constraint_info,
             this, _1, _2, _3, _4, _5 ));

   // propagate num_nonlinear_constraints::onChange to num_constraints
   _num_nonlinear_constraints.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &num_constraints.onChange(), num_constraints));

   // initialize everything (relying on the callbacks)
   _num_nonlinear_constraints = 0;
}


/// Destructor
Application_NonlinearConstraints::~Application_NonlinearConstraints()
{ }


//--------------------------------------------------------------------
// Application_NonlinearConstraints public information methods
//--------------------------------------------------------------------

/// Returns the number of nonlinear equality constraints
size_t
Application_NonlinearConstraints::numNonlinearEqConstraints() const
{
   size_t ans = 0;
   size_t maxIndex = num_nonlinear_constraints;

   const bound_t& lb = 
      nonlinear_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = 
      nonlinear_constraint_upper_bounds.expose<bound_t>();
   //const BoundTypeArray& lbt = 
   //   nonlinear_constraint_lower_bound_types.expose<BoundTypeArray>();
   //const BoundTypeArray& ubt = 
   //   nonlinear_constraint_upper_bound_types.expose<BoundTypeArray>();

   double EPS = equality_epsilon;
   for( size_t i = 0; i < maxIndex; ++i )
   {
      if ( isEquality(i, lb, ub, /*lbt, ubt,*/ EPS) )
         ++ans;
   }
   return ans; 
}


/// Returns the number of nonlinear inequality constraints
size_t
Application_NonlinearConstraints::numNonlinearIneqConstraints() const
{
   return num_nonlinear_constraints.expose<size_t>()
      - numNonlinearEqConstraints();
}


//--------------------------------------------------------------------
// Application_NonlinearConstraints public request & eval methods
//--------------------------------------------------------------------

/// Register a nonlinear constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraints, 
                NLCF, nlcf_info, nonlinear constraint );

/// Register a nonlinear constraint violation evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraints, 
                NLCFViol, nlcvf_info, nonlinear constraint violation );

/// Register a nonlinear equality constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraints, 
                NLEqCF, nleqcf_info, nonlinear equality constraint );

/// Register a nonlinear inequality constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraints, 
                NLIneqCF, nlineqcf_info, nonlinear inequality constraint );


//--------------------------------------------------------------------
// Application_NonlinearConstraints public access methods
//--------------------------------------------------------------------


/// Get bounds for a specific nonlinear constraint 
void 
Application_NonlinearConstraints::
nonlinearConstraintBound( size_t index,
                          utilib::AnyFixedRef lower,
                          utilib::AnyFixedRef upper ) const
{
   if ( num_nonlinear_constraints <= index )
   {
      EXCEPTION_MNGR(std::runtime_error, "Application_NonlinearConstraints"
                     "::nonlinearConstraintBound(): specified index "
                      << index << ") out of range (max = " << 
                      num_nonlinear_constraints << ")" );
   }

   utilib::TypeManager()->lexical_cast
      ( nonlinear_constraint_lower_bounds.expose<bound_t>()[index], lower );
   utilib::TypeManager()->lexical_cast
      ( nonlinear_constraint_upper_bounds.expose<bound_t>()[index], upper );
}


/// Get nonlinear equality constraint bounds - dense format (i.e. RHS)
void 
Application_NonlinearConstraints::
nonlinearEqConstraintBounds(utilib::AnyFixedRef rhs) const
{
   Any ans;
   splitConstraint( *this, nonlinear_constraint_lower_bounds.expose<bound_t>(),
                    ans.set<bound_t>(), true, equality_epsilon );
   utilib::TypeManager()->lexical_cast(ans, rhs);
}


/// Get nonlinear inequality constraint bounds - dense format
void 
Application_NonlinearConstraints::
nonlinearIneqConstraintBounds( utilib::AnyFixedRef lower,
                               utilib::AnyFixedRef upper ) const
{
   Any ans;
   bound_t &tmp = ans.set<bound_t>();
   double EPS = equality_epsilon;

   splitConstraint( *this, nonlinear_constraint_lower_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, lower);

   splitConstraint( *this, nonlinear_constraint_upper_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, upper);
}


/// Get the label for a specific nonlinear constraint
std::string  
Application_NonlinearConstraints::
nonlinearConstraintLabel(size_t i) const
{
   if ( num_nonlinear_constraints <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonlinearConstraints"
                     "::nonlinearConstraintLabel(): specified index ("
                     << i << ") out of range (max = " << 
                     num_nonlinear_constraints.expose<size_t>() - 1 << ")" );

   const labels_t::left_map &labels = 
      nonlinear_constraint_labels.expose<labels_t>().left;
   labels_t::left_map::const_iterator it = labels.find(i);
   if ( it != labels.end() )
      return it->second;
   else
      return "";
}


//============================================================================
//============================================================================
// Application_NonlinearConstraints protected member functions
//============================================================================
//============================================================================

///
void
Application_NonlinearConstraints::
_setNonlinearConstraintLabel(const size_t i, const std::string &label)
{
   if ( num_nonlinear_constraints <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonlinearConstraints::"
                     "_setNonlinearConstraintLabel(): index (" << i << 
                     ") past num_nonlinear_constraints (" << 
                     num_nonlinear_constraints << ")" );

   labels_t tmp = nonlinear_constraint_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _nonlinear_constraint_labels = tmp;
}


//============================================================================
//============================================================================
// Application_NonlinearConstraints private member functions
//============================================================================
//============================================================================


///
void
Application_NonlinearConstraints::
cb_onChange_num( const utilib::ReadOnly_Property &prop )
{
   size_t num = prop;

   // this sets any newly-added nonlinear_constraints to have +/-Inf bounds
   // (without changing any already-configured bounds).
   bound_t bounds = nonlinear_constraint_lower_bounds;
   if ( num == bounds.size() )
      return;

   bounds.resize(num, real::negative_infinity);
   _nonlinear_constraint_lower_bounds = bounds;

   bounds = nonlinear_constraint_upper_bounds;
   bounds.resize(num, real::positive_infinity);
   _nonlinear_constraint_upper_bounds = bounds;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = nonlinear_constraint_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= num )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _nonlinear_constraint_labels = labels;
}


bool
Application_NonlinearConstraints::
cb_validate_vector( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);

   size_t num = value.expose<bound_t>().size();

   bool ok = num_nonlinear_constraints == num;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonlinearConstraints::"
                     "cb_validate_vector(): vector length (" << num <<
                     ") does not match num_nonlinear_constraints (" 
                     << num_nonlinear_constraints << ")");
   return ok;
}


bool
Application_NonlinearConstraints::
cb_validate_labels( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);
   const labels_t &labels = value.expose<labels_t>();

   if ( labels.empty() )
      return true;

   if ( num_nonlinear_constraints <= labels.left.rbegin()->first )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonlinearConstraints::"
                     "cb_validate_labels(): label id (" << 
                     labels.left.rbegin()->first <<
                     ") is greater than num_nonlinear_constraints (" 
                     << num_nonlinear_constraints << ")");
   return true;
}


void
Application_NonlinearConstraints::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(nlcvf_info) ||
        requests.count(nleqcf_info) || 
        requests.count(nlineqcf_info) )
      cb_constraint_request(requests);
}


void
Application_NonlinearConstraints::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(nlcvf_info);
   inner_requests.erase(nleqcf_info);
   inner_requests.erase(nlineqcf_info);

   // If there are no nonlinear constraints to calculate, we want to
   // (silently) create a 0-length vector.
   if ( num_nonlinear_constraints == 0 )
      inner_requests.erase(nlcf_info);
}


int
Application_NonlinearConstraints::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   //std::cerr << utilib::demangledName(typeid(this)) << std::endl;
   static_cast<void>(domain);
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(nlcf_info) && ! response.count(nlcf_info) )
   {
      if ( num_nonlinear_constraints != 0 )
         return count;

      Any ans;
      ans.set<std::vector<real> >();
      response.insert(make_pair(nlcf_info, ans));
      ++count;
   }

   if ( outer_request.count(nlcvf_info) && ! response.count(nlcvf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VIOLATION, BOTH, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(nlcvf_info, ans.front())); 
      ++count;
   }

   if ( outer_request.count(nleqcf_info) && ! response.count(nleqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, EQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(nleqcf_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(nlineqcf_info) && ! response.count(nlineqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, INEQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(nlineqcf_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_NonlinearConstraints::
cb_constraint_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(nlcf_info, Any()));
}


size_t
Application_NonlinearConstraints::
cb_constraint_info( VectorType type, 
                    EqualityFilter equality, 
                    bool& dataPresent, 
                    std::list<utilib::Any>& components, 
                    AppResponse::response_map_t& response ) const
{
   switch ( type ) 
   {
   case COUNT:
      switch ( equality ) 
      {
      case BOTH:
         return num_nonlinear_constraints;
      case EQUALITY:
         return numNonlinearEqConstraints();
      case INEQUALITY:
         return numNonlinearIneqConstraints();
      };

   case VALUE: 
   case VIOLATION:
      {
         AppResponse::response_map_t::iterator it = response.find(nlcf_info);
         if ( it == response.end() )
         {
            dataPresent = false;
            return 0;
            //EXCEPTION_MNGR(std::runtime_error, 
            //               "Application_NonlinearConstraints::"
            //               "cb_constraint_info(): request for nlcf_info, "
            //               "but no data found in the response object!");
         }
         components.push_back(it->second);
      }

      // If necessary, compute the violation
      if ( type == VIOLATION )
      {
         // This makes a copy of the NLCF residual (so we don't actually
         // overrite the nlcf_info in the response map)
         Any tmp;
         vector<real>& violation = tmp.set<vector<real> >();
         TypeManager()->lexical_cast(components.back(), violation);
         components.back() = tmp;

         size_t maxIndex = num_nonlinear_constraints;
         if ( violation.size() != maxIndex )
            EXCEPTION_MNGR(std::runtime_error,
                           "Application_NonlinearConstraints::"
                           "cb_constraint_info(): nlcf.size() != "
                           "numNonlinearConstraints.");

         real test;
         const bound_t &lb = 
            nonlinear_constraint_lower_bounds.expose<bound_t>();
         const bound_t &ub = 
            nonlinear_constraint_upper_bounds.expose<bound_t>();
         for( size_t i = 0; i < maxIndex; ++i )
         {
            real& val = violation[i];
            if ( (test = val - lb[i]) < 0 )
               val = test;
            else if ( (test = val - ub[i]) > 0 )
               val = test;
            else
               val = 0.0;
         }
      }
      break;

   case LOWER_BOUND:
      components.push_back(nonlinear_constraint_lower_bounds.get());
      break;

   case UPPER_BOUND:
      components.push_back(nonlinear_constraint_upper_bounds.get());
      break;
   };

   if ( equality == BOTH )
      return num_nonlinear_constraints;
   else
   {
      Any whole;
      TypeManager()->lexical_cast( components.back(), whole, 
                                   typeid(bound_t) );
      bound_t &part = components.back().set<bound_t>();
      return splitConstraint( *this, whole.expose<bound_t>(), 
                              part,
                              equality == EQUALITY, 
                              equality_epsilon );
   }
}


///
void
Application_NonlinearConstraints::
cb_print(std::ostream& os)
{
   size_t num = num_nonlinear_constraints;
   os << "Nonlinear constraints:    " << num << endl;
   os << "  Equality:               " << numNonlinearEqConstraints() << endl;
   os << "  Inequality:             " << numNonlinearIneqConstraints() << endl;

   if ( num == 0 )
      return;

   const labels_t &labels = nonlinear_constraint_labels.expose<labels_t>();
   labels_t::left_const_iterator it;
   labels_t::left_const_iterator itEnd = labels.left.end();

   const bound_t &lb = nonlinear_constraint_lower_bounds.expose<bound_t>();
   const bound_t &ub = nonlinear_constraint_upper_bounds.expose<bound_t>();

   std::streamsize old_precision = os.precision();
   os.precision(6);

   os << "Index" << std::setw(20) 
      << "Label" << std::setw(15)
      << "Lower Bound" << std::setw(15) 
      << "Upper Bound" << endl;
   for (size_t i = 0; i < num; i++)
   {
      //
      // Index
      //
      os << std::setw(5) << (i + 1);

      //
      // Label
      //
      it = labels.left.find(i);
      if (it != itEnd)
         os << " " << std::setw(19) << it->second;
      else
         os << " " << std::setw(19) << "_none_";

      //
      // Bounds Info
      //
      os << " " << std::setw(14) << lb[i];
      os << " " << std::setw(14) << ub[i];
      os << std::endl;
   }

   // restore the precision
   os.precision(old_precision);
}


///
void
Application_NonlinearConstraints::
cb_initialize(TiXmlElement* elt)
{
   size_t size;
   utilib::get_num_attribute(elt, "num", size);

   _num_nonlinear_constraints = size;

   //
   // Process "Lower"  and "Upper" elements
   //
   bound_t  lb            = nonlinear_constraint_lower_bounds;
   bound_t  ub            = nonlinear_constraint_upper_bounds;
   const labels_t& labels = nonlinear_constraint_labels.expose<labels_t>();

   BoundTypeArray tmp;
   tmp.resize(size);

   process_bounds(elt, lb, ub, tmp, tmp, labels,
                  real::negative_infinity, real::positive_infinity );

   _nonlinear_constraint_lower_bounds = lb;
   _nonlinear_constraint_upper_bounds = ub;
}


//====================================================================
//====================================================================

//--------------------------------------------------------------------
// Application_NonlinearConstraintGradients Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_NonlinearConstraintGradients::
Application_NonlinearConstraintGradients()
{
   register_application_component(ObjectType::get(this));

   request_expansion_signal.connect
      (boost::bind( &Application_NonlinearConstraintGradients::cb_expand_request, 
             this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_NonlinearConstraintGradients::cb_map_request, 
             this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_NonlinearConstraintGradients::cb_map_response, 
             this, _1, _2, _3, _4 ));

   jacobian_request_signal.connect
      (boost::bind( &Application_NonlinearConstraintGradients::cb_jacobian_request,
             this, _1 ));

   jacobian_info_signal.connect
      (boost::bind( &Application_NonlinearConstraintGradients::cb_jacobian_info,
             this, _1, _2, _3, _4 ));
}


/// Destructor
Application_NonlinearConstraintGradients::
~Application_NonlinearConstraintGradients()
{}



void
Application_NonlinearConstraintGradients::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(nleqcg_info) || requests.count(nlineqcg_info) )
      cb_jacobian_request(requests);
}


void
Application_NonlinearConstraintGradients::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(nleqcg_info);
   inner_requests.erase(nlineqcg_info);

   // If there are no nonlinear constraints to calculate, we want to
   // (silently) create a 0-length vector.
   if ( num_nonlinear_constraints == 0 )
      inner_requests.erase(nlcg_info);
}


int
Application_NonlinearConstraintGradients::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   static_cast<void>(domain);
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(nlcg_info) && ! response.count(nlcg_info) )
   {
      if ( num_nonlinear_constraints != 0 )
         return count;

      Any ans;
      ans.set<grad_t>();
      response.insert(make_pair(nlcg_info, ans));
      ++count;
   }

   if ( outer_request.count(nleqcg_info) && ! response.count(nleqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::EQUALITY, 
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(nleqcg_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(nlineqcg_info) && ! response.count(nlineqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::INEQUALITY,
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(nlineqcg_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_NonlinearConstraintGradients::
cb_jacobian_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(nlcg_info, Any()));
}


size_t
Application_NonlinearConstraintGradients::
cb_jacobian_info( Application_Jacobian::EqualityFilter equality, 
                  bool& dataPresent, 
                  std::list<utilib::Any>& components, 
                  AppResponse::response_map_t& response ) const
{
   AppResponse::response_map_t::iterator it = response.find(nlcg_info);
   if ( it == response.end() )
   {
      dataPresent = false;
      return 0;
      //EXCEPTION_MNGR(std::runtime_error, 
      //               "Application_NonlinearConstraints::"
      //               "cb_constraint_info(): request for nlcf_info, "
      //               "but no data found in the response object!");
   }

   if ( equality == Application_Jacobian::BOTH )
   {
      components.push_back(it->second);
      return num_nonlinear_constraints;
   }
   else
   {
      const bound_t& lb = nonlinear_constraint_lower_bounds.expose<bound_t>();
      const bound_t& ub = nonlinear_constraint_upper_bounds.expose<bound_t>();

      list<size_t> remove;
      size_t i = lb.size();
      bool equality_test = equality == Application_Jacobian::EQUALITY;
      double EPS = equality_epsilon;
      while( i > 0 )
      {
         --i;
         if ( isEquality(i, lb, ub, /*lbt, ubt,*/ EPS) != equality_test )
            remove.push_front(i);
      }

      Any part;
      grad_t &ans = part.set<grad_t>();
      TypeManager()->lexical_cast( it->second, ans );

      while ( ! remove.empty() )
      {
         ans.delete_row(remove.back());
         remove.pop_back();
      }

      components.push_back(part);
      return ans.get_nrows();
   }

}


//--------------------------------------------------------------------
// Application_NonlinearConstraintGradients public request & eval methods
//--------------------------------------------------------------------

/// Register a nonlinear constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraintGradients, 
                NLCG, nlcg_info, 
                nonlinear constraint Jacobian );

/// Register a nonlinear equality constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraintGradients, 
                NLEqCG, nleqcg_info, 
                nonlinear equality constraint Jacobian );

/// Register a nonlinear inequality constraint evaluation computation
DEFINE_REQUEST( Application_NonlinearConstraintGradients, 
                NLIneqCG, nlineqcg_info, 
                nonlinear inequality constraint Jacobian );


} // namespace colin
