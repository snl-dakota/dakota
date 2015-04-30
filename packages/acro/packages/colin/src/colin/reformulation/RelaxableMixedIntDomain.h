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
 * \file RelaxableMixedIntDomainApplication.h
 *
 * Defines the colin::RelaxableMixedIntDomainApplication class
 */

#ifndef colin_RelaxableMixedIntDomainApplication
#define colin_RelaxableMixedIntDomainApplication

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/real.h>

#include <utilib/MixedIntVars.h>
#include <utilib/SparseMatrix.h>

#include <boost/bimap.hpp>

namespace colin
{

/// This is a class for reformulating an application from a mixed
/// integer search domain into an application that searches a continuous
/// relaxation of that domain.
template <class ProblemT>
class RelaxableMixedIntDomainApplication
   : public ReformulationApplication, 
     public Application<ProblemT>
     
{
   /// The ProblemT for the base continuous application
   typedef typename
   GetProblem< ProblemT::value 
                  & ~ ProblemTrait(integers)
                  & ~ ProblemTrait(binarys) >::type  remote_app_t;
   typedef RelaxableMixedIntDomainApplication<ProblemT>  this_t;

public:

   /// Constructor
   RelaxableMixedIntDomainApplication()
   {
      constructor();
   }

   /// Constructor
   RelaxableMixedIntDomainApplication(ApplicationHandle src)
   {
      constructor();
      // set the base problem that we are wrapping
      reformulate_application(src);
   }


   /// Destructor
   virtual ~RelaxableMixedIntDomainApplication()
   {}


   /// Convert the domain supplied by the solver to the application domain
   virtual bool
   map_domain(const utilib::Any& src, utilib::Any& native, bool forward) const
   {
      if ( forward )
      {
         utilib::Any tmp;
         utilib::TypeManager()->lexical_cast
             (src, tmp, typeid(utilib::MixedIntVars));
         const utilib::MixedIntVars &vars = tmp.expose<utilib::MixedIntVars>();

         utilib::BasicArray<double> &relaxed 
            = native.set<utilib::BasicArray<double> >();
         relaxed.resize(remote_app->get_problem().template expose
                        <Problem<remote_app_t> >()->num_real_vars);

         if (( this->num_real_vars   != vars.Real().size() ) ||
             ( this->num_int_vars    != vars.Integer().size() ) ||
             ( this->num_binary_vars != vars.Binary().size() ))
            EXCEPTION_MNGR(std::runtime_error,
                           "RelaxableMixedIntDomainApplication::map_domain(): "
                           "invalid domain: provided (binary, int, real) = (" 
                           << vars.Binary().size() << ", " 
                           << vars.Integer().size() << ", " 
                           << vars.Real().size() << "); expected ("
                           << this->num_binary_vars << ", " 
                           << this->num_int_vars << ", " 
                           << this->num_real_vars << ")");

         size_t i=0;
         // Copy binary variables
         for (size_t j=0; j<vars.Binary().size(); j++)
            relaxed[i++] = vars.Binary()[j];

         // Copy integer variables
         for (size_t j=0; j<vars.Integer().size(); j++)
            relaxed[i++] = vars.Integer()[j];

         // Copy real variables
         for (size_t j=0; j<vars.Real().size(); j++)
            relaxed[i++] = vars.Real()[j];

         return true;
      }
      else
      {
         utilib::Any tmp;
         utilib::TypeManager()->lexical_cast
             ( src, tmp, typeid(utilib::BasicArray<double>) );
         const utilib::BasicArray<double> &relaxed 
            = tmp.expose<utilib::BasicArray<double> >();
         
         utilib::MixedIntVars &vars = native.set<utilib::MixedIntVars>();

         size_t i=0;
         bool exact = true;

         // Copy binary variables
         vars.Binary().resize(this->num_binary_vars);
         for (size_t j=0; j<vars.Binary().size(); j++)
         {
            vars.Binary()[j] = static_cast<bool>(relaxed[i]);
            exact &= (static_cast<double>(vars.Binary()[j]) == relaxed[i]);
            ++i;
         }

         // Copy integer variables
         vars.Integer().resize(this->num_int_vars);
         for (size_t j=0; j<vars.Integer().size(); j++)
         {
            vars.Integer()[j] = static_cast<int>(relaxed[i]);
            exact &= (static_cast<double>(vars.Integer()[j]) == relaxed[i]);
            ++i;
         }

         // Copy real variables
         vars.Real().resize(this->num_real_vars);
         for (size_t j=0; j<vars.Real().size(); j++)
            vars.Real()[j] = relaxed[i++];

         return exact;
      }
   }


   /// Configure how to split the underlying relaxed app into a MixedInt space
   void set_discrete_domain(size_t nBinary, size_t nInt)
   {
      Problem<remote_app_t> remote = remote_app->get_problem()
         .template expose<Problem<remote_app_t> >();

      size_t nReal = remote->num_real_vars;
      if ( nReal < nInt + nBinary )
         EXCEPTION_MNGR(std::runtime_error, 
                        "RelaxableMixedIntDomainApplication::"
                        "set_discrete_domain: incompatible discrete domain ("
                        << nBinary << " + " << nInt << " > "
                        << remote->num_real_vars << ")");

      this->_num_real_vars = nReal - nInt - nBinary;
      this->_num_int_vars = nInt;
      this->_num_binary_vars = nBinary;
      
      cb_update_bounds(remote->real_lower_bounds);
      cb_update_bounds(remote->real_upper_bounds);
      cb_update_types(remote->real_lower_bound_types);
      cb_update_types(remote->real_upper_bound_types);
      cb_update_labels(remote->real_labels);
   }

protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      if ( remote_app_t::value != handle->problem_type() )
         EXCEPTION_MNGR(std::runtime_error, 
                        "RelaxableMixedIntDomainApplication::"
                        "validate_reformulated_application(): "
                        "invalid base application type " << 
                        handle->problem_type_name() << 
                        " for RelaxableMixedIntDomainApplication<" << 
                        this->problem_type_name() << ">");
   }

   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info (including the
      // objective... the properties aren't changing, only the way we
      // calculate it.)
      std::set<ObjectType> exclude;
      exclude.insert(ObjectType::get<Application_LinearConstraintGradients>());
      exclude.insert(ObjectType::get<Application_IntDomain>());
      exclude.insert(ObjectType::get<Application_RealDomain>());
      this->reference_reformulated_application_properties(exclude);

      Problem<remote_app_t> remote = remote_app->get_problem()
         .template expose<Problem<remote_app_t> >();

      // set up the remote app callbacks
      _reformulation_cb_list.push_back
         ( remote->num_real_vars.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_domain, this, _1) ) );
      _reformulation_cb_list.push_back
         ( remote->real_lower_bounds.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_bounds, this, _1) ) );
      _reformulation_cb_list.push_back
         ( remote->real_upper_bounds.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_bounds, this, _1) ) );
      _reformulation_cb_list.push_back
         ( remote->real_lower_bound_types.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_types, this, _1) ) );
      _reformulation_cb_list.push_back
         ( remote->real_upper_bound_types.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_types, this, _1) ) );
      _reformulation_cb_list.push_back
         ( remote->real_labels.onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_labels, this, _1) ) );

      if ( HasProblemTrait(ProblemT, gradients) && 
           HasProblemTrait(ProblemT, linear_constraints) )
         _reformulation_cb_list.push_back
            ( remote->property("linear_constraint_matrix").onChange().connect
              ( utilib::PropertyDict::connection_group,
                boost::bind(&this_t::cb_update_linear_matrix, this, _1) ) );

      // initialize things to a straight-through NLP
      set_discrete_domain( 0, 0 );
   }


private:  // central constructor method (called by all constructors)
   typedef utilib::CMSparseMatrix<real> sparse_t;
   typedef std::vector<real> vector_t;

   void constructor()
   {
      // This test is not necessary, as we make explicit reference to
      // num_binary_vars and num_int_vars, the template will not compile
      // if it is instantiated without the discrete component.
      //if ( remote_app_t::value == ProblemT::value )
      //   EXCEPTION_MNGR(std::runtime_error, 
      //                  "RelaxableMixedIntDomainApplication: "
      //                  "invalid application type " << 
      //                  this->problem_type_name() << 
      //                  " (no discrete domain component)");

      // If the underlying problem supplies gradients, then we need to
      // perform some mapping on the g_info to remove the discrete info.
      if ( HasProblemTrait(ProblemT, gradients) )
      {
         if ( HasProblemTrait(ProblemT, multiple_objectives) )
            response_transform_signal.connect
               ( boost::bind( &this_t::template cb_map_response<sparse_t>, 
                              this, g_info, _1, _2, _3, _4 ) );
         else
            response_transform_signal.connect
               ( boost::bind( &this_t::template cb_map_response<vector_t>, 
                              this, g_info, _1, _2, _3, _4 ) );
      }

      // If the underlying problem supplies nonlinear constraints, then
      // we need to perform some mapping on the Jacobian to remove the
      // discrete info.
      if ( HasProblemTrait(ProblemT, gradients) &&
           HasProblemTrait(ProblemT, nonlinear_constraints) )
         response_transform_signal.connect
            ( boost::bind
              ( &this_t::template cb_map_response<sparse_t>, 
                this, nlcg_info, _1, _2, _3, _4 ) );

      // If the underlying problem supplies linear constraints, then
      // we need to perform some mapping on the A-matrix to remove the
      // discrete info.
      if ( HasProblemTrait(ProblemT, gradients) &&
           HasProblemTrait(ProblemT, linear_constraints) )
         response_transform_signal.connect
            ( boost::bind
              ( &this_t::template cb_map_response<sparse_t>, 
                this, lcg_info, _1, _2, _3, _4 ) );

      // If the underlying problem supplies Hessian, then we need to
      // perform some mapping on the g_info to remove the discrete info.
      if ( HasProblemTrait(ProblemT, hessians) )
         response_transform_signal.connect
            ( boost::bind
              ( &this_t::template cb_map_response<sparse_t>, 
                this, h_info, _1, _2, _3, _4 ) );
   }


private:  // methods

   static void rmColumns(sparse_t& var, size_t delCols)
   { var.delete_cols(0, delCols); }

   static void rmColumns(vector_t& var, size_t delCols)
   { var.erase(var.begin(), var.begin() + delCols); }

   template<typename T>
   int cb_map_response( response_info_t field,
                         const utilib::Any &/*domain*/,
                         const AppRequest::request_map_t &outer_request,
                         const AppResponse::response_map_t &sub_response,
                         AppResponse::response_map_t &response) const
   {
      //std::cerr << utilib::demangledName(typeid(this))
      //          << "(" << AppResponseInfo().name(field) << ")" << std::endl;
      response.erase(field);

      typedef AppResponse::response_map_t::const_iterator sub_iterator;

      sub_iterator it = sub_response.find(field);
      if ( it == sub_response.end() )
         return outer_request.count(field) ? 0 : -1;

      utilib::Any ans;
      T &var = ans.set<T>();
      utilib::TypeManager()->lexical_cast( it->second, var );

      size_t delCols = this->domain_size.template as<size_t>()
         - this->num_real_vars.template as<size_t>();
      if ( delCols )
         rmColumns(var, delCols);

      response.insert(std::make_pair(field, ans));
      return -1;
   }

   void cb_update_domain(const utilib::ReadOnly_Property &prop)
   {
      size_t relaxed_size = prop;
      size_t nBin = this->num_binary_vars;
      if ( nBin >= relaxed_size )
      {
         this->_num_binary_vars = relaxed_size;
         this->_num_int_vars = 0;
         this->_num_real_vars = 0;
      }
      else
      { 
         relaxed_size -= nBin;
         size_t nInt = this->num_int_vars;
         if ( nInt >= relaxed_size )
         {
            this->_num_int_vars = relaxed_size;
            this->_num_real_vars = 0;
         }
         else
            this->_num_real_vars = relaxed_size - nInt;
      }

      if ( HasProblemTrait(ProblemT, gradients) && 
           HasProblemTrait(ProblemT, linear_constraints) )
         cb_update_linear_matrix(utilib::Property());
   }


   void cb_update_linear_matrix(const utilib::ReadOnly_Property &prop)
   {
      static_cast<void>(prop);
      this->properties.privilegedGet("linear_constraint_matrix") = sparse_t();
   }

   void cb_update_bounds(const utilib::ReadOnly_Property &prop)
   {
      // copy over the bounds
      std::vector<real> bounds = prop;

      // create within an Any to save a copy
      utilib::Any real_bounds;
      utilib::Any int_bounds;
      std::vector<real> &real_b = real_bounds.set<std::vector<real> >();
      std::vector<int>  &int_b  = int_bounds.set<std::vector<int> >();

      size_t i = this->num_binary_vars;
      size_t max = this->num_int_vars;
      int_b.reserve(max);
      max += i;
      for( ; i < max; ++i )
         int_b.push_back( bounds[i] == real::positive_infinity ? INT_MAX :
                          bounds[i] == real::negative_infinity ? INT_MIN : 
                          static_cast<int>(bounds[i]) );
      real_b.assign(bounds.begin() + max, bounds.end());

      if ( prop.equivalentTo(remote_app->property("real_lower_bounds")) )
      {
         this->_int_lower_bounds  = int_bounds;
         this->_real_lower_bounds = real_bounds;
      }
      else // real_upper_bounds
      {
         this->_int_upper_bounds  = int_bounds;
         this->_real_upper_bounds = real_bounds;
      }
   }


   void cb_update_types(const utilib::ReadOnly_Property &prop)
   {
      // copy over the bounds
      BoundTypeArray bounds = prop;

      // create within an Any to save a copy
      utilib::Any _real_bt;
      utilib::Any _int_bt;
      BoundTypeArray &real_bt = _real_bt.set<BoundTypeArray>();
      BoundTypeArray &int_bt  = _int_bt.set<BoundTypeArray>();

      size_t max = this->num_int_vars;
      size_t offset = this->num_binary_vars;
      int_bt.resize(max);
      max += offset;

      for( size_t i = 0; offset < max; ++i, ++offset )
         int_bt[i] = bounds[offset];

      max = bounds.size();
      real_bt.resize(max-offset);
      for( size_t i = 0; offset < max; ++i, ++offset )
         real_bt[i] = bounds[offset];

      if ( prop.equivalentTo(remote_app->property("real_lower_bound_types")) )
      {
         this->_real_lower_bound_types = _real_bt;
         this->_int_lower_bound_types  = _int_bt;
      }
      else // real_upper_bound_types
      {
         this->_real_upper_bound_types = _real_bt;
         this->_int_upper_bound_types  = _int_bt;
      }
   }


   void cb_update_labels(const utilib::ReadOnly_Property &prop)
   {
      typedef typename boost::bimap<size_t, std::string>  labels_t;
      labels_t labels = prop;

      // create within an Any to save a copy
      utilib::Any _real_l;
      utilib::Any _int_l;
      utilib::Any _bin_l;
      labels_t &real_l = _real_l.set<labels_t>();
      labels_t &int_l  = _int_l.set<labels_t>();
      labels_t &bin_l  = _bin_l.set<labels_t>();

      size_t i_ofs = this->num_binary_vars;
      size_t r_ofs = i_ofs + this->num_int_vars.template as<size_t>();
      labels_t::left_iterator it = labels.left.begin();
      labels_t::left_iterator itEnd = labels.left.end();
      for ( ; it != itEnd; ++it )
         if ( it->first < i_ofs )
            bin_l.insert(labels_t::value_type(it->first, it->second));
         else if ( it->first < r_ofs )
            int_l.insert(labels_t::value_type(it->first - i_ofs, it->second));
         else
            real_l.insert(labels_t::value_type(it->first - r_ofs, it->second));

      this->_real_labels   = _real_l;
      this->_int_labels    = _int_l;
      this->_binary_labels = _bin_l;
   }
};


} // namespace colin

#endif // defined colin_RelaxableMixedIntDomainApplication
