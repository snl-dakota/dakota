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
 * \file UnconMultiobjApplication.h
 *
 * Defines the colin::UnconMultiobjApplication class
 */

#ifndef colin_UnconMultiobjApplication_h
#define colin_UnconMultiobjApplication_h

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/Utilities.h>

#include <utilib/SparseMatrix.h>

#include <limits>

namespace colin
{
template <typename ProblemT> class UnconMultiobjApplication;

template<typename ProblemT, bool>
struct UnconMultiobjApplication_initNonD 
{
   typedef UnconMultiobjApplication<ProblemT> class_t;

   static void init( class_t* object )
   {
      ApplicationHandle &remote = object->remote_app;
      object->_nond_objective.set_readonly();

      // Note: because these update callbacks change *values* and not
      // problem size (i.e. num_XXX), we want these updates to fire
      // *after* the normal connection_group callbacks (so that any
      // re-sizing of the problem is already complete).

      object->_reformulation_cb_list.push_back
         ( remote->property("nond_objective").onChange()
           .connect( //utilib::PropertyDict::connection_group,
                     boost::bind(&update, object) ) );

      if ( remote->has_property("num_nondeterministic_constraints") )
         object->_reformulation_cb_list.push_back
            ( remote->property("num_nondeterministic_constraints").onChange()
              .connect( //utilib::PropertyDict::connection_group,
                        boost::bind(&update, object) ) );

      update(object);
   }

   static void update( class_t* object )
   {
      utilib::BitArray tmp = object->remote_app->property("nond_objective");
      if ( object->augmentedObjective )
      {
         size_t i = tmp.size();
         tmp.resize(i+1);
         tmp[i] = 
            object->remote_app->has_property("num_nondeterministic_constraints") 
            && ( object->remote_app->property("num_nondeterministic_constraints") > 0 );
      }
      object->_nond_objective = tmp;
   }
};

template<class ProblemT>
struct UnconMultiobjApplication_initNonD<ProblemT, false> 
{
   typedef UnconMultiobjApplication<ProblemT> class_t;
   static void init( class_t* ) {}
   static void update( class_t* )  {}
};



template <class ProblemT>
class UnconMultiobjApplication
   : public Application<ProblemT>,
     public ReformulationApplication
{
   typedef UnconMultiobjApplication<ProblemT>  this_t;

   typedef UnconMultiobjApplication_initNonD
      <ProblemT, HasProblemTrait(ProblemT, nondeterministic_response)>
      helper_t;

   friend struct UnconMultiobjApplication_initNonD
      <ProblemT, HasProblemTrait(ProblemT, nondeterministic_response)>;

public:

   ///
   UnconMultiobjApplication()
      : augmentedObjective(false)
   {
      constructor();
   }

   ///
   UnconMultiobjApplication(ApplicationHandle src)
      : augmentedObjective(false)
   {
      constructor();
      // set the base problem that we are wrapping
      reformulate_application(src);
   }

   ///
   virtual ~UnconMultiobjApplication()
   { }


protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      if ( ( ( handle->problem_type() 
               & ~ ProblemTrait(linear_constraints)
               & ~ ProblemTrait(nonlinear_constraints) )
             != this->problem_type() )
           || ( this->problem_type() == handle->problem_type() ) )
         EXCEPTION_MNGR(std::runtime_error, "UnconMultiobjApplication::"
                        "validate_reformulated_application(): invalid base "
                        "application type " << handle->problem_type_name() << 
                        " for UnconMultiobjApplication<" << 
                        this->problem_type_name() << ">");
   }
   
   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info EXCEPT the objective
      // -- we will do that by hand.
      std::set<ObjectType> exclude;
      exclude.insert(ObjectType::get<Application_MultiObjective>());
      exclude.insert(ObjectType::get<Application_NonD_Objective>());
      exclude.insert(ObjectType::get<Application_Gradient>());
      exclude.insert(ObjectType::get<Application_Hessian>());
      this->reference_reformulated_application_properties(exclude);

      _reformulation_cb_list.push_back
         ( remote_app->property("num_objectives").onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::update_obj_info, this) ) );
      _reformulation_cb_list.push_back
         ( remote_app->property("sense").onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::update_obj_sense, this) ) );
      _reformulation_cb_list.push_back
         ( remote_app->property("num_constraints").onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::update_obj_info, this) ) );
      
      // Initialize the objective information
      update_obj_info();

      // Initialize the sense information
      update_obj_sense();

      // Initialize the NonD properties (if applicable)
      helper_t::init(this);
   }


private: // central constructor method (called by all constructors)
   void constructor()
   {
      if ( ! HasProblemTrait(ProblemT, multiple_objectives) )
         EXCEPTION_MNGR(std::logic_error, "UnconMultiobjApplication - "
                        "cannot be instantiated as a single-objective "
                        "application.");
      if ( HasProblemTrait(ProblemT, hessians) )
         EXCEPTION_MNGR(std::logic_error, "UnconMultiobjApplication - "
                        "cannot be instantiated with Hessian information.");

      request_transform_signal.connect
         (boost::bind( &UnconMultiobjApplication::cb_map_request, 
                       this, _1, _2 ));
      response_transform_signal.connect
         (boost::bind( &UnconMultiobjApplication::cb_map_f_response, 
                       this, _1, _2, _3, _4 ));
      response_transform_signal.connect
         (boost::bind( &UnconMultiobjApplication::cb_map_g_response, 
                       this, _1, _2, _3, _4 ));
   }

private: // methods

   void update_obj_info()
   {
      Application_Base* base = remote_app.object();

      // Initialize the MultiObj application info
      size_t remote_nCon = base->property("num_constraints");
      augmentedObjective = remote_nCon > 0;

      size_t nObj = base->property("num_objectives");
      nObj += ( augmentedObjective ? 1 : 0 );
      if ( this->_num_objectives != nObj )
         this->_num_objectives = nObj;
   }


   void update_obj_sense()
   {
      std::vector<optimizationSense> tmp = remote_app->property("sense");

      if ( augmentedObjective )
         tmp.push_back(colin::minimization);

      this->_sense = tmp;
   }


   void 
   cb_map_request( const AppRequest::request_map_t &local,
                   AppRequest::request_map_t &remote )
   {
      static_cast<void>(local);
      if ( ! augmentedObjective )
         return;

      if ( remote.find(mf_info) != remote.end() )
         remote.insert(std::make_pair(cvf_info, utilib::Any()));

      if (( remote.find(g_info) != remote.end() )&& 
          ( remote_app->property("num_constraints") > 0 ))
      {
         remote.insert(std::make_pair(cg_info, utilib::Any()));
         remote.insert(std::make_pair(cvf_info, utilib::Any()));
      }
   }


   int
   cb_map_f_response( const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response )
   {
      //std::cerr << utilib::demangledName(typeid(this)) << "(f)" << std::endl;
      static_cast<void>(domain);
      typedef AppResponse::response_map_t::const_iterator sub_iterator;

      if ( ! augmentedObjective ) 
         return -1;

      // remove any mf_info that was automatically propagated
      response.erase(mf_info);

      sub_iterator mf_it = sub_response.find( mf_info );
      if ( mf_it == sub_response.end() )
         return requests.count(mf_info) ? 0 : -1;

      sub_iterator c_it = sub_response.find(cvf_info);
      if ( c_it == sub_response.end() )
         return 0;

      utilib::Any cv;
      utilib::TypeManager()->lexical_cast
         ( c_it->second, cv, typeid(std::vector<real>) );
      const std::vector<real> &cviol_v = cv.expose<std::vector<real> >();
      
      utilib::Any ans;
      std::vector<real> &mf_val = ans.set<std::vector<real> >();
      utilib::TypeManager()->lexical_cast(mf_it->second, mf_val);
      mf_val.push_back( l2_norm_sq(cviol_v) );

      response.insert(AppResponse::response_pair_t(mf_info, ans));
      return -1;
   }

   /**  \brief Calculate the gradient for the penalized objective.  
    *
    *   Define:
    *     f = function; 
    *    cv = constraint violation; 
    *     x = domain; 
    *     r = residual;
    *     p = penalty value (==1);
    *
    *   Note:
    *     cv = r - upper_bound (if r > upper bound)
    *     cv = 0               (if lower_bound < r < upper bound)
    *     cv = r - lower_bound (if r < lower bound)
    *   Thus, dcv/dx == dr/dx iif cv != 0; otherwise dcv/dx == 0
    *
    *   As we are forming the objective from the violation squared:
    *     F = sum( p*cv^2 )
    *     dF/dx = sum( p * 2 * cv * dcv/dx )
    *           = sum( p * 2 * cv * dr/dx )
    *           = sum( p * 2 * cv * cg )
    */
   int
   cb_map_g_response( const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response )
   {
      //std::cerr << utilib::demangledName(typeid(this)) << "(g)" << std::endl;
      static_cast<void>(domain);
      typedef AppResponse::response_map_t::const_iterator sub_iterator;
      typedef utilib::RMSparseMatrix<double> sparse_t;

      if ( !augmentedObjective )
         return -1;

      // remove the g_info that was automatically propagated
      response.erase(g_info);

      sub_iterator g_it = sub_response.find(g_info);
      if ( g_it == sub_response.end() )
         return requests.count(g_info) ? 0 : -1;

      sub_iterator cv_it = sub_response.find(cvf_info);
      if ( cv_it == sub_response.end() )
         return 0;

      sub_iterator cg_it = sub_response.find(cg_info);
      if ( cg_it == sub_response.end() )
         return 0;

      utilib::Any any_cv;
      utilib::TypeManager()->lexical_cast
         ( cv_it->second, any_cv, typeid(std::vector<real>) );
      const std::vector<real>& cv = any_cv.expose<std::vector<real> >();

      utilib::Any any_cg;
      utilib::TypeManager()->lexical_cast
         ( cg_it->second, any_cg, typeid(sparse_t) );
      const sparse_t &cg = any_cg.expose<sparse_t>();
      
      // The extra objective's gradient is: sum(2*cv*cg)
      std::vector<double> g(cg.get_ncols(), 0.0);
      size_t row = cg.get_nrows(); 
      while ( row > 0 )
      {
         --row;
         size_t i = cg.get_matbeg()[row];
         size_t n = cg.get_matcnt()[row];
         while ( n > 0 )
         {
            g[cg.get_matind()[i]] += 2.0*cv[row]*cg.get_matval()[i];
            ++i;
            --n;
         }
         
      }

      // append to the original gradient result
      utilib::Any ans;
      sparse_t &g_val = ans.set<sparse_t>();
      utilib::TypeManager()->lexical_cast(g_it->second, g_val);
      g_val.adjoinRow( g.size(), &g[0] );

      // Return the result
      response.insert(AppResponse::response_pair_t(g_info, ans));
      return -1;
   }

private: // data

   bool augmentedObjective;

};


} // namespace colin

#endif // defined colin_UnconMultiobjApplication_h
