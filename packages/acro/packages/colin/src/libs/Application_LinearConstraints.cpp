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
 * \file Application_LinearConstraints.cpp
 *
 * Defines the colin::Application_LinearConstraints class members.
 */

#include <colin/application/LinearConstraints.h>
#include <colin/application/Domain.h>
#include <colin/TinyXML.h>
#include <colin/real.h>

#include <utilib/SparseMatrix.h>

#include <boost/bimap.hpp>

using std::cerr;
using std::endl;
using std::vector;
using std::list;
using std::make_pair;

using utilib::seed_t;
using utilib::Any;
using utilib::AnyRef;
using utilib::NumArray;
using utilib::Property;
using utilib::TypeManager;
using utilib::RMSparseMatrix;
using utilib::CMSparseMatrix;


namespace colin {

namespace {

typedef boost::bimap<size_t, std::string>  labels_t;
typedef std::vector<colin::real>           bound_t;
typedef utilib::RMSparseMatrix<double>     matrix_t;
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

size_t splitConstraint( const Application_LinearConstraints& me, 
                        const bound_t &whole, bound_t &part, 
                        bool equality, double EPS )
{
   const bound_t& lb = me.linear_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = me.linear_constraint_upper_bounds.expose<bound_t>();
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
// Application_LinearConstraints Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_LinearConstraints::Application_LinearConstraints()
   : _num_linear_constraints(Property::Bind<size_t>()),
     _linear_constraint_lower_bounds(Property::Bind<bound_t>()),
     _linear_constraint_upper_bounds(Property::Bind<bound_t>()),
     _linear_constraint_labels(Property::Bind<labels_t>()),
     num_linear_constraints(_num_linear_constraints.set_readonly()),
     linear_constraint_lower_bounds(_linear_constraint_lower_bounds.set_readonly()),
     linear_constraint_upper_bounds(_linear_constraint_upper_bounds.set_readonly()),
     linear_constraint_labels(_linear_constraint_labels.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare( "num_linear_constraints",
                       _num_linear_constraints, t );
   properties.declare( "linear_constraint_lower_bounds",
                       _linear_constraint_lower_bounds, t );
   properties.declare( "linear_constraint_upper_bounds",
                       _linear_constraint_upper_bounds, t );
   properties.declare( "linear_constraint_labels", 
                       _linear_constraint_labels, t );


   _num_linear_constraints.onChange().connect
      (bind(&Application_LinearConstraints::cb_onChange_num, this, _1));

   _linear_constraint_lower_bounds.validate().connect
      (bind(&Application_LinearConstraints::cb_validate_vector, this, _1, _2));
   _linear_constraint_upper_bounds.validate().connect
      (bind(&Application_LinearConstraints::cb_validate_vector, this, _1, _2));

   _linear_constraint_labels.validate().connect
      (bind(&Application_LinearConstraints::cb_validate_labels, this, _1, _2));


   print_signal.connect
      (11, boost::bind(&Application_LinearConstraints::cb_print, this, _1));

   constraint_initializer("Linear").connect
      (boost::bind(&Application_LinearConstraints::cb_initialize, this, _1));


   request_expansion_signal.connect
      (boost::bind( &Application_LinearConstraints::cb_expand_request, 
                    this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_LinearConstraints::cb_map_request, 
                    this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_LinearConstraints::cb_map_response, 
                    this, _1, _2, _3, _4 ));

   constraint_request_signal.connect
      (boost::bind( &Application_LinearConstraints::cb_constraint_request,
                    this, _1 ));

   constraint_info_signal.connect
      (boost::bind( &Application_LinearConstraints::cb_constraint_info,
                    this, _1, _2, _3, _4, _5 ));

   // propagate num_linear_constraints::onChange to num_constraints
   _num_linear_constraints.onChange().connect
      (boost::bind( static_cast<Property::onChange_signal_t>
             (&Property::onChange_t::operator()), 
             &num_constraints.onChange(), num_constraints));

   // initialize everything (relying on the callbacks)
   _num_linear_constraints = 0;
}


/// Destructor
Application_LinearConstraints::~Application_LinearConstraints()
{ }


//--------------------------------------------------------------------
// Application_LinearConstraints public information methods
//--------------------------------------------------------------------

/// Returns the number of linear equality constraints
size_t
Application_LinearConstraints::numLinearEqConstraints() const
{ 
   size_t ans = 0;
   size_t maxIndex = num_linear_constraints;

   const bound_t& lb = 
      linear_constraint_lower_bounds.expose<bound_t>();
   const bound_t& ub = 
      linear_constraint_upper_bounds.expose<bound_t>();
   //const BoundTypeArray& lbt = 
   //   linear_constraint_lower_bound_types.expose<BoundTypeArray>();
   //const BoundTypeArray& ubt = 
   //   linear_constraint_upper_bound_types.expose<BoundTypeArray>();

   double EPS = equality_epsilon;
   for( size_t i = 0; i < maxIndex; ++i )
   {
      if ( isEquality(i, lb, ub, /*lbt, ubt,*/ EPS) )
         ++ans;
   }
   return ans; 
}


/// Returns the number of linear inequality constraints
size_t
Application_LinearConstraints::numLinearIneqConstraints() const
{
   return num_linear_constraints.as<size_t>() - numLinearEqConstraints();
}


//--------------------------------------------------------------------
// Application_LinearConstraints public request & evaluation methods
//--------------------------------------------------------------------

/// Register a linear constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraints, 
                LCF, lcf_info, linear constraint );

/// Register a linear constraint violation evaluation computation
DEFINE_REQUEST( Application_LinearConstraints, 
                LCFViol, lcvf_info, linear constraint violation );

/// Register a linear equality constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraints, 
                LEqCF, leqcf_info, linear equality constraint );

/// Register a linear inequality constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraints, 
                LIneqCF, lineqcf_info, linear inequality constraint );


//--------------------------------------------------------------------
// Application_LinearConstraints public access methods
//--------------------------------------------------------------------


/// Return linear constraint bounds
void
Application_LinearConstraints::
linearConstraintBound( size_t index,
                       utilib::AnyFixedRef lower,
                       utilib::AnyFixedRef upper ) const
{
   if ( num_linear_constraints <= index )
   {
      EXCEPTION_MNGR( std::runtime_error, "Application_LinearConstraints"
                      "::linearConstraintBound(): specified index ("
                      << index << ") out of range (max = " << 
                      num_linear_constraints << ")" );
   }

   utilib::TypeManager()->lexical_cast
      (linear_constraint_lower_bounds.expose<bound_t>()[index], lower);
   utilib::TypeManager()->lexical_cast
      (linear_constraint_upper_bounds.expose<bound_t>()[index], upper);
}


/// Get linear equality constraint bounds - dense format (i.e. RHS)
void 
Application_LinearConstraints::
linearEqConstraintBounds(utilib::AnyFixedRef rhs) const
{
   Any ans;
   splitConstraint( *this, linear_constraint_lower_bounds.expose<bound_t>(), 
                    ans.set<bound_t>(), true, equality_epsilon );
   utilib::TypeManager()->lexical_cast(ans, rhs);
}


/// Get linear inequality constraint bounds - dense format
void 
Application_LinearConstraints::
linearIneqConstraintBounds( utilib::AnyFixedRef lower,
                            utilib::AnyFixedRef upper ) const
{
   Any ans;
   bound_t &tmp = ans.set<bound_t>();
   double EPS = equality_epsilon;

   splitConstraint( *this, linear_constraint_lower_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, lower);

   splitConstraint( *this, linear_constraint_upper_bounds.expose<bound_t>(),
                    tmp, false, EPS );
   utilib::TypeManager()->lexical_cast(ans, upper);
}


/// Get the label for a specific linear constraint
std::string  
Application_LinearConstraints::
linearConstraintLabel(size_t i) const
{
   if ( num_linear_constraints <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints"
                     "::linearConstraintLabel(): specified index ("
                     << i << ") out of range (max = " << 
                     num_linear_constraints.expose<size_t>() - 1 << ")" );

   const labels_t::left_map &labels = 
      linear_constraint_labels.expose<labels_t>().left;
   labels_t::left_map::const_iterator it = labels.find(i);
   if ( it != labels.end() )
      return it->second;
   else
      return "";
}


//============================================================================
//============================================================================
// Application_LinearConstraints protected member functions
//============================================================================
//============================================================================

///
void
Application_LinearConstraints::
_setLinearConstraintLabel(const size_t i, const std::string &label)
{
   if ( num_linear_constraints <= i )
      EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints::"
                     "_setLinearConstraintLabel(): "
                     "index (" << i << ") past num_linear_constraints ("
                     << num_linear_constraints << ")");

   labels_t tmp = linear_constraint_labels;
   tmp.left.erase(i);
   if ( ! label.empty() )
      tmp.insert( labels_t::value_type(i, label) );

   _linear_constraint_labels = tmp;
}


//============================================================================
//============================================================================
// Application_LinearConstraints private member functions
//============================================================================
//============================================================================

///
void
Application_LinearConstraints::
cb_onChange_num( const utilib::ReadOnly_Property &prop )
{
   size_t num = prop;

   // this sets any newly-added linear_constraints to have +/-Inf bounds
   // (without changing any already-configured bounds).
   bound_t bounds = linear_constraint_lower_bounds;
   if ( num == bounds.size() )
      return;

   bounds.resize(num, real::negative_infinity);
   _linear_constraint_lower_bounds = bounds;

   bounds = linear_constraint_upper_bounds;
   bounds.resize(num, real::positive_infinity);
   _linear_constraint_upper_bounds = bounds;

   // NB: the Bound Types are updated by the bounds onChange() callback

   bool updateLabels = false;
   labels_t labels = linear_constraint_labels;
   while ( ! labels.empty() && labels.left.rbegin()->first >= num )
   {
      labels.left.erase(labels.left.rbegin()->first);
      updateLabels = true;
   }
   if ( updateLabels )
      _linear_constraint_labels = labels;
}


bool
Application_LinearConstraints::
cb_validate_vector( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);

   size_t num = value.is_type(typeid(BoundTypeArray)) 
      ? value.expose<BoundTypeArray>().size()
      : value.expose<bound_t>().size();

   bool ok = num_linear_constraints == num;
   if ( ! ok )
      EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints::"
                     "cb_validate_vector(): vector length (" << num <<
                     ") does not match num_linear_constraints (" 
                     << num_linear_constraints << ")");
   return ok;
}


bool
Application_LinearConstraints::
cb_validate_labels( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);
   const labels_t &labels = value.expose<labels_t>();

   if ( labels.empty() )
      return true;

   if ( num_linear_constraints <= labels.left.rbegin()->first )
      EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints::"
                     "cb_validate_labels(): label id (" << 
                     labels.left.rbegin()->first <<
                     ") is greater than num_linear_constraints (" 
                     << num_linear_constraints << ")");
   return true;
}


utilib::Any
Application_LinearConstraints::
compute_lcf( const utilib::Any &domain ) const
{
   // NB: the caller should check that num_linear_constraints > 0 &&
   // linear_constraint_matrix is defined and the appropriate size.
   //if ( num_linear_constraints == 0 )
   //   return vector<real>();

   const matrix_t &mat = 
      properties["linear_constraint_matrix"].expose<matrix_t>();

   //if ( mat.get_ncols() == 0 && mat.get_nrows() == 0 )
   //   EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints::"
   //                  "compute_lcf(): cannot compute LCF without a local "
   //                  "copy of the constraint matrix!");

   Any concrete_domain;
   TypeManager()->lexical_cast( domain, concrete_domain, 
                                typeid(NumArray<double>) );
   NumArray<double> const * d = &concrete_domain.expose<NumArray<double> >();

   //if ( static_cast<size_t>(mat.get_ncols()) != d->size() )
   //{
   //   EXCEPTION_MNGR(std::runtime_error, "Application_LinearConstraints::"
   //                  "compute_lcf(): ncols [" << mat.get_ncols() << "] != "
   //                  "domain size [" << d->size() << "].");
   //}

   Any ans;
   utilib::product( ans.set<utilib::NumArray<double> >(), mat, *d );
   return ans;
}


void
Application_LinearConstraints::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(lcvf_info) || 
        requests.count(leqcf_info) || 
        requests.count(lineqcf_info) )
      cb_constraint_request(requests);
}


void
Application_LinearConstraints::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(lcvf_info);
   inner_requests.erase(leqcf_info);
   inner_requests.erase(lineqcf_info);

   // If we have a local constraint matrix, we do NOT want to ask the
   // underlying application to calculate it.  We will do that
   // ourselves.  Also, if there are no linear constraints to calculate,
   // we want to (silently) create a 0-length vector.
   if ( num_linear_constraints == 0 || 
        ( properties.exists("linear_constraint_matrix") &&
          ( properties["linear_constraint_matrix"].expose<matrix_t>().
            get_ncols() != 0 ) ) )
      inner_requests.erase(lcf_info);
}


int
Application_LinearConstraints::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   //std::cerr << utilib::demangledName(typeid(this)) << std::endl;
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(lcf_info) && ! response.count(lcf_info) )
   {
      if ( properties.exists("linear_constraint_matrix") &&
           ( properties["linear_constraint_matrix"].expose<matrix_t>().
             get_ncols() != 0 ) )
         response.insert(make_pair(lcf_info, compute_lcf(domain)));
      else if ( num_linear_constraints == 0 )
      {
         Any ans;
         ans.set<std::vector<real> >();
         response.insert(make_pair(lcf_info, ans));
      }
      else
         return count;
      ++count;
   }

   if ( outer_request.count(lcvf_info) && ! response.count(lcvf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VIOLATION, BOTH, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(lcvf_info, ans.front())); 
      ++count;
   }

   if ( outer_request.count(leqcf_info) && ! response.count(leqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, EQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(leqcf_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(lineqcf_info) && ! response.count(lineqcf_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_constraint_info(VALUE, INEQUALITY, dataPresent, ans, response);
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(lineqcf_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_LinearConstraints::
cb_constraint_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(lcf_info, Any()));
}


size_t
Application_LinearConstraints::
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
         return num_linear_constraints;
      case EQUALITY:
         return numLinearEqConstraints();
      case INEQUALITY:
         return numLinearIneqConstraints();
      };

   case VALUE: 
   case VIOLATION:
      {
         AppResponse::response_map_t::iterator it = response.find(lcf_info);
         if ( it == response.end() )
         {
            dataPresent = false;
            return 0;
            //EXCEPTION_MNGR(std::runtime_error, 
            //               "Application_LinearConstraints::"
            //               "cb_constraint_info(): request for lcf_info, "
            //               "but no data found in the response object!");
         }
         components.push_back(it->second);
      }

      // If necessary, compute the violation
      if ( type == VIOLATION )
      {
         // This makes a copy of the LCF residual (so we don't actually
         // overrite the lcf_info in the response map)
         Any tmp;
         vector<real>& violation = tmp.set<vector<real> >();
         TypeManager()->lexical_cast(components.back(), violation);
         components.back() = tmp;

         size_t maxIndex = num_linear_constraints;
         if ( violation.size() != maxIndex )
            EXCEPTION_MNGR(std::runtime_error,
                           "Application_LinearConstraints::"
                           "cb_constraint_info(): lcf.size() != "
                           "numLinearConstraints.");

         real test;
         const bound_t &lb = 
            linear_constraint_lower_bounds.expose<bound_t>();
         const bound_t &ub = 
            linear_constraint_upper_bounds.expose<bound_t>();
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
      components.push_back(linear_constraint_lower_bounds.get());
      break;

   case UPPER_BOUND:
      components.push_back(linear_constraint_upper_bounds.get());
      break;
   };

   if ( equality == BOTH )
      return num_linear_constraints;
   else
   {
      Any whole;
      TypeManager()->lexical_cast( components.back(), whole, 
                                   typeid(bound_t) );
      vector<real> &part = components.back().set<bound_t>();
      return splitConstraint( *this, whole.expose<bound_t>(), 
                              part,
                              equality == EQUALITY, 
                              equality_epsilon );
   }
}


///
void
Application_LinearConstraints::
cb_print(std::ostream& os)
{
   size_t num = num_linear_constraints;
   os << "Linear constraints:       " << num << endl;
   os << "  Equality:               " << numLinearEqConstraints() << endl;
   os << "  Inequality:             " << numLinearIneqConstraints() << endl;

   if ( num == 0 )
      return;

   const labels_t &labels = linear_constraint_labels.expose<labels_t>();
   labels_t::left_const_iterator it;
   labels_t::left_const_iterator itEnd = labels.left.end();

   const bound_t &lb = linear_constraint_lower_bounds.expose<bound_t>();
   const bound_t &ub = linear_constraint_upper_bounds.expose<bound_t>();

   std::streamsize old_precision = os.precision();
   os.precision(6);

   os << "Index" << std::setw(20)
      << "Label" << std::setw(15)
      << "Lower Bound" << std::setw(15) 
      << "Upper Bound" << std::endl;
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
Application_LinearConstraints::
cb_initialize(TiXmlElement* elt)
{
   size_t size;
   utilib::get_num_attribute(elt, "num", size);

   _num_linear_constraints = size;

   //
   // Process "Lower"  and "Upper" elements
   //
   bound_t  lb            = linear_constraint_lower_bounds;
   bound_t  ub            = linear_constraint_upper_bounds;
   const labels_t& labels = linear_constraint_labels.expose<labels_t>();

   BoundTypeArray tmp;
   tmp.resize(size);

   process_bounds(elt, lb, ub, tmp, tmp, labels,
                  real::negative_infinity, real::positive_infinity );

   _linear_constraint_lower_bounds = lb;
   _linear_constraint_upper_bounds = ub;
}



//====================================================================
//====================================================================

//--------------------------------------------------------------------
// Application_LinearConstraintGradients Public member functions
//--------------------------------------------------------------------

/// Constructor
Application_LinearConstraintGradients::
Application_LinearConstraintGradients()
   : _linear_constraint_matrix(Property::Bind<RMSparseMatrix<double> >()),
     linear_constraint_matrix(_linear_constraint_matrix.set_readonly())
{
   ObjectType t = ObjectType::get(this);
   register_application_component(t);
   properties.declare( "linear_constraint_matrix", 
                       _linear_constraint_matrix, t );

   _num_linear_constraints.onChange().connect
      (bind( &Application_LinearConstraintGradients::cb_onChange_num, 
             this, _1 ));

   _linear_constraint_matrix.validate().connect
      (bind( &Application_LinearConstraintGradients::cb_validate_matrix, 
             this, _1, _2 ));

   request_expansion_signal.connect
      (boost::bind( &Application_LinearConstraintGradients::cb_expand_request, 
             this, _1 ));
   request_transform_signal.connect
      (boost::bind( &Application_LinearConstraintGradients::cb_map_request, 
             this, _1, _2 ));

   response_transform_signal.connect
      (boost::bind( &Application_LinearConstraintGradients::cb_map_response, 
             this, _1, _2, _3, _4 ));

   jacobian_request_signal.connect
      (boost::bind( &Application_LinearConstraintGradients::cb_jacobian_request,
             this, _1 ));

   jacobian_info_signal.connect
      (boost::bind( &Application_LinearConstraintGradients::cb_jacobian_info,
             this, _1, _2, _3, _4 ));
}


/// Destructor
Application_LinearConstraintGradients::
~Application_LinearConstraintGradients()
{}


//============================================================================
//============================================================================
// Application_LinearConstraintGradients private member functions
//============================================================================
//============================================================================

///
void
Application_LinearConstraintGradients::
cb_onChange_num( const utilib::ReadOnly_Property &prop )
{
   static_cast<void>(prop);
   // Because the number of constraints is changing, our old constraint
   // matrix is no longer valid.
   _linear_constraint_matrix = RMSparseMatrix<double>();
}


/** Set linear constraint matrix.  If the matrix is not set, the
 *  lcf_info response type will be passed to the application
 *  compute_response() as part of the request map.  If the matris is
 *  set, have_local_constraint_matrix will be true and the lcf_info will
 *  be calculated locally and NOT passed along to compute_response() as
 *  part of the request map.
 */
bool
Application_LinearConstraintGradients::
cb_validate_matrix( const utilib::ReadOnly_Property &prop,
                    const utilib::Any &value )
{
   static_cast<void>(prop);

   const matrix_t& mat = value.expose<matrix_t>();

   if ( mat.get_nrows() == 0 && mat.get_ncols() == 0 )
      return true;

   bool fail = false;
   if ( (fail = num_linear_constraints != mat.get_nrows()) )
      EXCEPTION_MNGR(std::runtime_error, 
                     "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of rows (" << 
                     mat.get_nrows() << 
                     ") does not match num_linear_constraints (" 
                     << num_linear_constraints << ")");
   if ( (fail = this->properties["domain_size"] != mat.get_ncols()) )
      EXCEPTION_MNGR(std::runtime_error, 
                     "Application_LinearConstraintGradients::"
                     "cb_validate_matrix(): number of columns (" << 
                     mat.get_ncols() << 
                     ") does not match domain_size (" 
                     << this->properties["domain_size"] << ")");
   return !fail;
}


void
Application_LinearConstraintGradients::
cb_expand_request( AppRequest::request_map_t &requests ) const
{
   // Expand the generic Constraint requests to compute the constraint
   // components
   if ( requests.count(leqcg_info) || requests.count(lineqcg_info) )
      cb_jacobian_request(requests);
}


void
Application_LinearConstraintGradients::
cb_map_request( const AppRequest::request_map_t &outer_requests,
                AppRequest::request_map_t &inner_requests ) const
{
   static_cast<void>(outer_requests);
   // Remove the "pseudo requests" that this class inserts into the
   // request map
   inner_requests.erase(leqcg_info);
   inner_requests.erase(lineqcg_info);

   // If there are no linear constraints to calculate, we want to
   // (silently) create a 0-length vector; if we have a local matrix, we
   // will calculate the lcg_info ourselves...
   if ( num_linear_constraints == 0 || 
        _linear_constraint_matrix.expose<matrix_t>().get_ncols() != 0 )
      inner_requests.erase(lcg_info);
}


int
Application_LinearConstraintGradients::
cb_map_response( const utilib::Any &domain,
                 const AppRequest::request_map_t& outer_request,
                 const AppResponse::response_map_t& inner_response,
                 AppResponse::response_map_t& response) const
{
   static_cast<void>(domain);
   static_cast<void>(inner_response);
   int count = 0;

   if ( outer_request.count(lcg_info) && ! response.count(lcg_info) )
   {
      Any ans;
      if ( num_linear_constraints == 0 )
         ans.set<grad_t>();
      else if ( linear_constraint_matrix.expose<matrix_t>().get_ncols() != 0 )
      {
         // The jacobian is only the *differentiable* portion of the A-matrix
         size_t cols = linear_constraint_matrix.expose<matrix_t>().get_ncols();
         Domain::DifferentiableComponent dc;
         TypeManager()->lexical_cast(domain, dc);
         if ( dc.source_index.size() == cols )
            ans = linear_constraint_matrix.get();
         else
         {
            CMSparseMatrix<double> &tmp = ans.set<CMSparseMatrix<double> >();
            TypeManager()->lexical_cast(linear_constraint_matrix.get(), tmp);
            // a vain attempt to do this efficiently
            std::set<size_t> del;
            for(size_t i=0; i<cols; del.insert(i++));
            for(size_t i=0; i<dc.size(); del.erase(dc.source_index[i++]));
            while ( ! del.empty() )
            {
               size_t start = *del.begin();
               size_t count = 1;
               del.erase(del.begin());
               while ( ! del.empty() && *del.begin() == start + count )
               {
                  ++count;
                  del.erase(del.begin());
               }
               tmp.delete_cols(start, count);
            }
         }
      }
      else
         return count;

      response.insert(make_pair(lcg_info, ans));
      ++count;
   }

   if ( outer_request.count(leqcg_info) && ! response.count(leqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::EQUALITY, 
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(leqcg_info, ans.front()));
      ++count;
   }

   if ( outer_request.count(lineqcg_info) && ! response.count(lineqcg_info) )
   {
      list<Any> ans;
      bool dataPresent = true;
      cb_jacobian_info( Application_Jacobian::INEQUALITY,
                        dataPresent, ans, response );
      if ( ! dataPresent )
         return count;

      response.insert(make_pair(lineqcg_info, ans.front()));
      ++count;
   }

   return -1;
}


void
Application_LinearConstraintGradients::
cb_jacobian_request(AppRequest::request_map_t& requests) const
{
   requests.insert(make_pair(lcg_info, Any()));
}


size_t
Application_LinearConstraintGradients::
cb_jacobian_info( Application_Jacobian::EqualityFilter equality, 
                  bool& dataPresent, 
                  std::list<utilib::Any>& components, 
                  AppResponse::response_map_t& response ) const
{
   AppResponse::response_map_t::iterator it = response.find(lcg_info);
   if ( it == response.end() )
   {
      dataPresent = false;
      return 0;
      //EXCEPTION_MNGR(std::runtime_error, 
      //               "Application_LinearConstraintGradients::"
      //               "cb_constraint_info(): request for lcf_info, "
      //               "but no data found in the response object!");
   }

   if ( equality == Application_Jacobian::BOTH )
   {
      components.push_back(it->second);
      return num_linear_constraints;
   }
   else
   {
      const bound_t& lb = linear_constraint_lower_bounds.expose<bound_t>();
      const bound_t& ub = linear_constraint_upper_bounds.expose<bound_t>();

      list<size_t> remove;
      size_t i = lb.size();
      bool equality_test = equality == Application_Jacobian::EQUALITY;
      double EPS = equality_epsilon;
      while( i > 0 )
      {
         --i;
         if ( isEquality(i, lb, ub, EPS) != equality_test )
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


void
Application_LinearConstraintGradients::
cb_initialize(TiXmlElement* elt)
{   //
   // Process the actual linear constraint matrix
   //
   TiXmlElement* child = elt->FirstChildElement("Matrix");
   for ( ; child != NULL; child = child->NextSiblingElement("Matrix") )
   {
      if ( child->ValueStr().compare("Matrix") != 0 )
         EXCEPTION_MNGR(std::runtime_error, 
                        "Application_LinearConstraintGradients::"
                        "cb_initialize(): Unrecognized " 
                        << utilib::get_element_info(child));

      utilib::Any any;
      std::string mat_type;
      utilib::get_string_attribute(child->ToElement(), "type", 
                                   mat_type, "dense");
      read_xml_matrix(any, child->ToElement(), mat_type);
      _linear_constraint_matrix = any;
   }
}

//--------------------------------------------------------------------
// Application_LinearConstraintGradients public request & eval methods
//--------------------------------------------------------------------

/// Register a linear constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraintGradients, 
                LCG, lcg_info, 
                linear constraint Jacobian );

/// Register a linear equality constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraintGradients, 
                LEqCG, leqcg_info, 
                linear equality constraint Jacobian );

/// Register a linear inequality constraint evaluation computation
DEFINE_REQUEST( Application_LinearConstraintGradients, 
                LIneqCG, lineqcg_info, 
                linear inequality constraint Jacobian );


} //namespace colin
