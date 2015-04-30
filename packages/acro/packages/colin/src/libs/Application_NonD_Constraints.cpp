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
 * \file Application_NonD_Constraints.cpp
 *
 * Defines the colin::Application_NonD_Constraints class members.
 */

#include <colin/application/NonD_Constraints.h>
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

size_t splitConstraint( const Application_NonD_Constraints& me, 
                        const bound_t &whole, bound_t &part, 
                        bool equality, double EPS )
{
   const bound_t& lb = 
      me.nondeterministic_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = 
      me.nondeterministic_constraint_upper_bounds.expose<bound_t>();
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
// Application_NonD_Constraints Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_NonD_Constraints::Application_NonD_Constraints()
   : _num_nondeterministic_constraints(Property::Bind<size_t>()),
     _nondeterministic_constraint_lower_bounds(Property::Bind<bound_t>()),
     _nondeterministic_constraint_upper_bounds(Property::Bind<bound_t>()),
     _nondeterministic_constraint_labels(Property::Bind<labels_t>()),
     num_nondeterministic_constraints(_num_nondeterministic_constraints.set_readonly()),
     nondeterministic_constraint_lower_bounds(_nondeterministic_constraint_lower_bounds.set_readonly()),
     nondeterministic_constraint_upper_bounds(_nondeterministic_constraint_upper_bounds.set_readonly()),
     nondeterministic_constraint_labels(_nondeterministic_constraint_labels.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare( "num_nondeterministic_constraints", 
                       _num_nondeterministic_constraints, t );
   properties.declare( "nondeterministic_constraint_lower_bounds",
                       _nondeterministic_constraint_lower_bounds, t );
   properties.declare( "nondeterministic_constraint_upper_bounds",
                       _nondeterministic_constraint_upper_bounds, t );
   properties.declare( "nondeterministic_constraint_labels", 
                       _nondeterministic_constraint_labels, t );


   _num_nondeterministic_constraints.onChange().connect
      ( bind(&Application_NonD_Constraints::cb_onChange_num, this, _1) );

   _nondeterministic_constraint_lower_bounds.validate().connect
      ( bind(&Application_NonD_Constraints::cb_validate_vector, this, 
             _1, _2) );
   _nondeterministic_constraint_upper_bounds.validate().connect
      ( bind(&Application_NonD_Constraints::cb_validate_vector, this, 
             _1, _2) );

   _nondeterministic_constraint_labels.validate().connect
      ( bind(&Application_NonD_Constraints::cb_validate_labels, this, 
             _1, _2) );


   print_signal.connect
      (11, boost::bind(&Application_NonD_Constraints::cb_print, this, _1));

   constraint_initializer("Nondeterministic").connect
      (bind( &Application_NonD_Constraints::cb_initialize, 
             this, _1 ));


   request_expansion_signal.connect
      (boost::bind( &Application_NonD_Constraints::cb_expand_request, 
             this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_NonD_Constraints::cb_map_request, 
             this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_NonD_Constraints::cb_map_response, 
             this, _1, _2, _3, _4 ));

   constraint_request_signal.connect
      (boost::bind( &Application_NonD_Constraints::cb_constraint_request,
             this, _1 ));

   constraint_info_signal.connect
      (boost::bind( &Application_NonD_Constraints::cb_constraint_info,
             this, _1, _2, _3, _4, _5 ));

   // propagate num_nondeterministic_constraints::onChange to num_constraints
   _num_nondeterministic_constraints.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &num_constraints.onChange(), num_constraints));

   // initialize everything (relying on the callbacks)
   _num_nondeterministic_constraints = 0;
}


/// Destructor
Application_NonD_Constraints::~Application_NonD_Constraints()
{ }


//--------------------------------------------------------------------
// Application_NonD_Constraints public information methods
//--------------------------------------------------------------------

/// Returns the number of nondeterministic equality constraints
size_t
Application_NonD_Constraints::numNondeterministicEqConstraints() const
{
   size_t ans = 0;
   size_t maxIndex = num_nondeterministic_constraints;

   const bound_t& lb = 
      nondeterministic_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = 
      nondeterministic_constraint_upper_bounds.expose<bound_t>();
   //const BoundTypeArray& lbt = 
   //   nondeterministic_constraint_lower_bound_types.expose<BoundTypeArray>();
   //const BoundTypeArray& ubt = 
   //   nondeterministic_constraint_upper_bound_types.expose<BoundTypeArray>();

   double EPS = equality_epsilon;
   for( size_t i = 0; i < maxIndex; ++i )
   {
      if ( isEquality(i, lb, ub, /*lbt, ubt,*/ EPS) )
         ++ans;
   }
   return ans; 
}


/// Returns the number of nondeterministic inequality constraints
size_t
Application_NonD_Constraints::numNondeterministicIneqConstraints() const
{
   return num_nondeterministic_constraints.expose<size_t>()
      - numNondeterministicEqConstraints();
}


//--------------------------------------------------------------------
// Application_NonD_Constraints public request & eval methods
//--------------------------------------------------------------------

/// Register a nondeterministic constraint evaluation computation
DEFINE_REQUEST( Application_NonD_Constraints, 
                NDCF, ndcf_info, nondeterministic constraint );

/// Register a nondeterministic constraint violation evaluation computation
DEFINE_REQUEST( Application_NonD_Constraints, 
                NDCFViol, ndcvf_info, 
                nondeterministic constraint violation );

/// Register a nondeterministic equality constraint evaluation computation
DEFINE_REQUEST( Application_NonD_Constraints, 
                NDEqCF, ndeqcf_info, 
                nondeterministic equality constraint );

/// Register a nondeterministic inequality constraint evaluation computation
DEFINE_REQUEST( Application_NonD_Constraints, 
                NDIneqCF, ndineqcf_info, 
                nondeterministic inequality constraint );


//--------------------------------------------------------------------
// Application_NonD_Constraints public access methods
//--------------------------------------------------------------------


/// Get bounds for a specific nondeterministic constraint 
void 
Application_NonD_Constraints::
nondeterministicConstraintBound( size_t index,
                                 utilib::AnyFixedRef lower,
                                 utilib::AnyFixedRef upper ) const
{
   if ( num_nondeterministic_constraints <= index )
   {
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Constraints"
                     "::nondeterministicConstraintBound(): specified index "
                      << index << ") out of range (max = " << 
                      num_nondeterministic_constraints << ")" );
   }

   utilib::TypeManager()->lexical_cast
      ( nondeterministic_constraint_lower_bounds.expose<bound_t>()[index],
        lower );
   utilib::TypeManager()->lexical_cast
      ( nondeterministic_constraint_upper_bounds.expose<bound_t>()[index],
        upper );
}


/// Get nondeterministic equality constraint bounds - dense format (i.e. RHS)
void 
Application_NonD_Constraints::
nondeterministicEqConstraintBounds(utilib::AnyFixedRef rhs) const
{
   Any ans;
   splitConstraint( *this, 
                    nondeterministic_constraint_lower_bounds.expose<bound_t>(),
                    ans.set<bound_t>(), true, equality_epsilon );
   utilib::TypeManager()->lexical_cast(ans, rhs);
}


/// Get nondeterministic inequality constraint bounds - dense format
void 
Application_NonD_Constraints::
nondeterministicIneqConstraintBounds( utilib::AnyFixedRef lower,
                               utilib::AnyFixedRef upper ) const
{
   Any ans;
   bound_t &tmp = ans.set<bound_t>();
   double EPS = equality_epsilon;

   splitConstraint( *this, 
                    nondeterministic_constraint_lower_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, lower);

   splitConstraint( *this, 
                    nondeterministic_constraint_upper_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, upper);
}


//============================================================================
//============================================================================
// Application_NonD_Constraints protected member functions
//============================================================================
//============================================================================

///
void
Application_NonD_Constraints::
_setNondeterministicConstraintLabel(const size_t i, const std::string &label)
{
   if ( num_nondeterministic_constraints <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Constraints::"
                     "_setNondeterministicConstraintLabel(): index (" << i << 
                     ") past num_nondeterministic_constraints (" << 
                     num_nondeterministic_constraints << ")" );

   labels_t tmp = nondeterministic_constraint_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _nondeterministic_constraint_labels = tmp;
}


//============================================================================
//============================================================================
// Application_NonD_Constraints private member functions
//============================================================================
//============================================================================


///
void
Application_NonD_Constraints::
cb_onChange_num( const utilib::ReadOnly_Property &prop )
{
   size_t num = prop;

   // this sets any newly-added nondeterministic_constraints to have
   // +/-Inf bounds (without changing any already-configured bounds).
   bound_t bounds = nondeterministic_constraint_lower_bounds;
   if ( num == bounds.size() )
      return;

   bounds.resize(num, real::negative_infinity);
   _nondeterministic_constraint_lower_bounds = bounds;

   bounds = nondeterministic_constraint_upper_bounds;
   bounds.resize(num, real::positive_infinity);
   _nondeterministic_constraint_upper_bounds = bounds;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = nondeterministic_constraint_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= num )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _nondeterministic_constraint_labels = labels;
}


bool
Application_NonD_Constraints::
cb_validate_vector( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);

   size_t num = value.expose<bound_t>().size();

   bool ok = num_nondeterministic_constraints == num;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Constraints::"
                     "cb_validate_vector(): vector length (" << num <<
                     ") does not match num_nondeterministic_constraints (" 
                     << num_nondeterministic_constraints << ")");
   return ok;
}


bool
Application_NonD_Constraints::
cb_validate_labels( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);
   const labels_t &labels = value.expose<labels_t>();

   if ( labels.empty() )
      return true;

   if ( num_nondeterministic_constraints <= labels.left.rbegin()->first )
      EXCEPTION_MNGR(std::runtime_error, "Application_NonD_Constraints::"
                     "cb_validate_labels(): label id (" << 
                     labels.left.rbegin()->first <<
                     ") is greater than num_nondeterministic_constraints (" 
                     << num_nondeterministic_constraints << ")");
   return true;
}


void
Application_NonD_Constraints::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(ndcvf_info) || 
        requests.count(ndeqcf_info) || 
        requests.count(ndineqcf_info) )
      cb_constraint_request(requests);
}


void
Application_NonD_Constraints::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(ndcvf_info);
   inner_requests.erase(ndeqcf_info);
   inner_requests.erase(ndineqcf_info);

   // If there are no nondeterministic constraints to calculate, we want to
   // (silently) create a 0-length vector.
   if ( num_nondeterministic_constraints == 0 )
      inner_requests.erase(ndcf_info);
}


int
Application_NonD_Constraints::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   //std::cerr << utilib::demangledName(typeid(this)) << std::endl;
   static_cast<void>(domain);
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(ndcf_info) && ! response.count(ndcf_info) )
   {
      if ( num_nondeterministic_constraints != 0 )
         return count;

      Any ans;
      ans.set<std::vector<real> >();
      response.insert(make_pair(ndcf_info, ans));
      ++count;
   }

   if ( outer_request.count(ndcvf_info) && ! response.count(ndcvf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VIOLATION, BOTH, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(ndcvf_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(ndeqcf_info) && ! response.count(ndeqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, EQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(ndeqcf_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(ndineqcf_info) && ! response.count(ndineqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, INEQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(ndineqcf_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_NonD_Constraints::
cb_constraint_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(ndcf_info, Any()));
}


size_t
Application_NonD_Constraints::
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
         return num_nondeterministic_constraints;
      case EQUALITY:
         return numNondeterministicEqConstraints();
      case INEQUALITY:
         return numNondeterministicIneqConstraints();
      };

   case VALUE: 
   case VIOLATION:
      {
         AppResponse::response_map_t::iterator it = response.find(ndcf_info);
         if ( it == response.end() )
         {
            dataPresent = false;
            return 0;
            //EXCEPTION_MNGR(std::runtime_error, 
            //               "Application_NonD_Constraints::"
            //               "cb_constraint_info(): request for ndcf_info, "
            //               "but no data found in the response object!");
         }
         components.push_back(it->second);
      }

      // If necessary, compute the violation
      if ( type == VIOLATION )
      {
         // This makes a copy of the NDCF residual (so we don't actually
         // overrite the ndcf_info in the response map)
         Any tmp;
         vector<real>& violation = tmp.set<vector<real> >();
         TypeManager()->lexical_cast(components.back(), violation);
         components.back() = tmp;

         size_t maxIndex = num_nondeterministic_constraints;
         if ( violation.size() != maxIndex )
            EXCEPTION_MNGR(std::runtime_error,
                           "Application_NonD_Constraints::"
                           "cb_constraint_info(): ndcf.size() != "
                           "numNonD_Constraints.");

         real test;
         const bound_t &lb = 
            nondeterministic_constraint_lower_bounds.expose<bound_t>();
         const bound_t &ub = 
            nondeterministic_constraint_upper_bounds.expose<bound_t>();
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
      components.push_back(nondeterministic_constraint_lower_bounds.get());
      break;

   case UPPER_BOUND:
      components.push_back(nondeterministic_constraint_upper_bounds.get());
      break;
   };

   if ( equality == BOTH )
      return num_nondeterministic_constraints;
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
Application_NonD_Constraints::
cb_print(std::ostream& os)
{
   size_t num = num_nondeterministic_constraints;
   os << "Nondeterministic constraints:  " << num << endl;
   os << "  Equality:               " 
      << numNondeterministicEqConstraints() << endl;
   os << "  Inequality:             " 
      << numNondeterministicIneqConstraints() << endl;

   if ( num == 0 )
      return;

   const labels_t &labels = 
      nondeterministic_constraint_labels.expose<labels_t>();
   labels_t::left_const_iterator it;
   labels_t::left_const_iterator itEnd = labels.left.end();

   const bound_t &lb = 
      nondeterministic_constraint_lower_bounds.expose<bound_t>();
   const bound_t &ub = 
      nondeterministic_constraint_upper_bounds.expose<bound_t>();

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
Application_NonD_Constraints::
cb_initialize(TiXmlElement* elt)
{
   size_t size;
   utilib::get_num_attribute(elt, "num", size);

   _num_nondeterministic_constraints = size;

   //
   // Process "Lower"  and "Upper" elements
   //
   bound_t  lb            = nondeterministic_constraint_lower_bounds;
   bound_t  ub            = nondeterministic_constraint_upper_bounds;
   const labels_t& labels = nondeterministic_constraint_labels.expose<labels_t>();

   BoundTypeArray tmp;
   tmp.resize(size);

   process_bounds(elt, lb, ub, tmp, tmp, labels,
                  real::negative_infinity, real::positive_infinity );

   _nondeterministic_constraint_lower_bounds = lb;
   _nondeterministic_constraint_upper_bounds = ub;
}


//====================================================================
//====================================================================

//--------------------------------------------------------------------
// Application_NonD_ConstraintGradients Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_NonD_ConstraintGradients::
Application_NonD_ConstraintGradients()
{
   register_application_component(ObjectType::get(this));

   request_expansion_signal.connect
      (boost::bind( &Application_NonD_ConstraintGradients::cb_expand_request, 
             this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_NonD_ConstraintGradients::cb_map_request, 
             this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_NonD_ConstraintGradients::cb_map_response, 
             this, _1, _2, _3, _4 ));

   jacobian_request_signal.connect
      (boost::bind( &Application_NonD_ConstraintGradients::cb_jacobian_request,
             this, _1 ));

   jacobian_info_signal.connect
      (boost::bind( &Application_NonD_ConstraintGradients::cb_jacobian_info,
             this, _1, _2, _3, _4 ));
}


/// Destructor
Application_NonD_ConstraintGradients::
~Application_NonD_ConstraintGradients()
{}



void
Application_NonD_ConstraintGradients::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(ndeqcg_info) || requests.count(ndineqcg_info) )
      cb_jacobian_request(requests);
}


void
Application_NonD_ConstraintGradients::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(ndeqcg_info);
   inner_requests.erase(ndineqcg_info);

   // If there are no nondeterministic constraints to calculate, we want to
   // (silently) create a 0-length vector.
   if ( num_nondeterministic_constraints == 0 )
      inner_requests.erase(ndcg_info);
}


int
Application_NonD_ConstraintGradients::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   static_cast<void>(domain);
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(ndcg_info) && ! response.count(ndcg_info) )
   {
      if ( num_nondeterministic_constraints != 0 )
         return count;

      Any ans;
      ans.set<grad_t>();
      response.insert(make_pair(ndcg_info, ans));
      ++count;
   }

   if ( outer_request.count(ndeqcg_info) && ! response.count(ndeqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::EQUALITY, 
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(ndeqcg_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(ndineqcg_info) && ! response.count(ndineqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::INEQUALITY,
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(ndineqcg_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_NonD_ConstraintGradients::
cb_jacobian_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(ndcg_info, Any()));
}


size_t
Application_NonD_ConstraintGradients::
cb_jacobian_info( Application_Jacobian::EqualityFilter equality, 
                  bool& dataPresent, 
                  std::list<utilib::Any>& components, 
                  AppResponse::response_map_t& response ) const
{
   AppResponse::response_map_t::iterator it = response.find(ndcg_info);
   if ( it == response.end() )
   {
      dataPresent = false;
      return 0;
      //EXCEPTION_MNGR(std::runtime_error, 
      //               "Application_NonD_Constraints::"
      //               "cb_constraint_info(): request for ndcf_info, "
      //               "but no data found in the response object!");
   }

   if ( equality == Application_Jacobian::BOTH )
   {
      components.push_back(it->second);
      return num_nondeterministic_constraints;
   }
   else
   {
      const bound_t& lb = 
         nondeterministic_constraint_lower_bounds.expose<bound_t>();
      const bound_t& ub = 
         nondeterministic_constraint_upper_bounds.expose<bound_t>();

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
// Application_NonD_ConstraintGradients public request & eval methods
//--------------------------------------------------------------------

/// Register a nondeterministic constraint evaluation computation
DEFINE_REQUEST( Application_NonD_ConstraintGradients, 
                NDCG, ndcg_info, 
                nondeterministic constraint Jacobian );

/// Register a nondeterministic equality constraint evaluation computation
DEFINE_REQUEST( Application_NonD_ConstraintGradients, 
                NDEqCG, ndeqcg_info, 
                nondeterministic equality constraint Jacobian );

/// Register a nondeterministic inequality constraint evaluation computation
DEFINE_REQUEST( Application_NonD_ConstraintGradients, 
                NDIneqCG, ndineqcg_info, 
                nondeterministic inequality constraint Jacobian );


} // namespace colin
