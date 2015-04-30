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
 * \file WeightedSumApplication.h
 *
 * Defines the colin::WeightedSumApplication class
 */

#ifndef colin_WeightedSumApplication_h
#define colin_WeightedSumApplication_h

#include <acro_config.h>
#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/TinyXML.h>

#include <utilib/SparseMatrix.h> 

namespace colin
{
// Forward declaration of the reformulation class
template <typename ProblemT> class WeightedSumApplication;


template<class ProblemT, bool>
struct WeightedSumApplication_nonD_helper
{
   typedef WeightedSumApplication<ProblemT>  class_t;

   static void init( class_t *object )
   {
      utilib::Property& nond = object->remote_app->property("nond_objective");

      object->_reformulation_cb_list.push_back
            ( nond.onChange().connect
              ( utilib::PropertyDict::connection_group,
                boost::bind(&update, object, _1) ) );

      update(object, nond);
   }

   static void update( class_t* object, utilib::Property& nond )
   {
      object->nond_objective = 
         ( nond.template as<utilib::BitArray>().nbits() > 0 );
   }
};

template<class ProblemT>
struct WeightedSumApplication_nonD_helper<ProblemT, false>
{
   typedef WeightedSumApplication<ProblemT>  class_t;
   static void init( class_t* ) {}
};



template <class ProblemT>
class WeightedSumApplication 
   : public Application<ProblemT>,
     public ReformulationApplication
{
   typedef std::vector<double>  weights_t;

   typedef WeightedSumApplication<ProblemT>  this_t;

   /// The ProblemT for the base multi-objective application
   typedef typename
   GetProblem<ProblemT::value
                 | ProblemTrait(multiple_objectives)>::type
   remote_app_t;


   friend struct WeightedSumApplication_nonD_helper
      <ProblemT, HasProblemTrait(ProblemT, nondeterministic_response)>;

public:

   ///
   WeightedSumApplication()
      : weights(utilib::Property::Bind<weights_t>())
   {
      constructor();
   }

   ///
   WeightedSumApplication(ApplicationHandle src)
      : weights(utilib::Property::Bind<weights_t>())
   {
      constructor();
      // set the base problem that we are wrapping
      reformulate_application(src);
   }

   ///
   virtual ~WeightedSumApplication()
   { }


   /// The vector of weights (alpha) used for the linear weighted sum
   utilib::Property weights;


protected:

   virtual void validate_reformulated_application(ApplicationHandle handle)
   {
      if ( handle->problem_type() != remote_app_t::value )
         EXCEPTION_MNGR(std::runtime_error, "WeightedSumApplication::"
                        "validate_reformulated_application(): invalid base "
                        "application type " << handle->problem_type_name() 
                        << " for WeightedSumApplication<" << 
                        this->problem_type_name() << ">");
   }
   
   virtual void configure_reformulated_application()
   {
      // reference all appropriate external info EXCEPT the objective
      std::set<ObjectType> exclude;
      exclude.insert(ObjectType::get<Application_SingleObjective>());
      exclude.insert(ObjectType::get<Application_NonD_Objective>());
      exclude.insert(ObjectType::get<Application_Gradient>());
      exclude.insert(ObjectType::get<Application_Hessian>());
      this->reference_reformulated_application_properties(exclude);

      //
      // Initialize weights
      //
      weights = weights_t((*remote_app)["num_objectives"], 1.0);

      WeightedSumApplication_nonD_helper
         <ProblemT, HasProblemTrait(ProblemT, nondeterministic_response)>
         ::init(this);

      _reformulation_cb_list.push_back
         ( remote_app->property("num_objectives").onChange().connect
           ( utilib::PropertyDict::connection_group,
             boost::bind(&this_t::cb_update_nobj, this, _1) ) );      
   }


private: // central constructor method (called by all constructors)
   void constructor()
   {
      if ( HasProblemTrait(ProblemT, multiple_objectives) )
         EXCEPTION_MNGR(std::logic_error, "WeightedSumApplication - "
                        "cannot be created as a multi-objective application.");

      if (HasProblemTrait(ProblemT, hessians))
         EXCEPTION_MNGR(std::logic_error, "WeightedSumApplication - "
                        "cannot be created with Hessian information.");

      properties.declare( "objective_weights", weights,
                          ObjectType::get(this), true );

      weights.validate().connect
         ( bind(&this_t::cb_validate_weights, this, _1, _2));

      initializer("Weights").connect
         (boost::bind(&this_t::cb_initialize, this, _1));

      request_transform_signal.connect
         (boost::bind( &this_t::cb_map_request, this, _1, _2 ));
      response_transform_signal.connect
         (boost::bind( &this_t::cb_map_f_response, this, _1, _2, _3, _4 ));
      response_transform_signal.connect
         (boost::bind( &this_t::cb_map_g_response, this, _1, _2, _3, _4 ));
   }

private: // callback methods
   typedef utilib::RMSparseMatrix<real> sparse_t;
   typedef std::vector<real> vector_t;

   bool cb_validate_weights( const utilib::ReadOnly_Property &,
                             const utilib::Any &value )
   {
      if ((*remote_app)["num_objectives"] != value.expose<weights_t>().size())
         EXCEPTION_MNGR(std::runtime_error, "WeightedSumApplication::"
                        "cb_validate_weights(): new weight vector size (" <<
                        value.expose<weights_t>().size() 
                        << ") does not match the number of "
                        "objectives in the wrapped problem (" << 
                        (*remote_app)["num_objectives"] << ")");
      return true;
   }

   void cb_update_nobj(const utilib::ReadOnly_Property &prop)
   {
      weights_t tmp = weights;
      tmp.resize(prop, 1.0);
      weights = tmp;
   }

   ///
   void cb_initialize(TiXmlElement* elt)
   {
      weights_t tmp;
      read_xml_object(tmp, elt);

      // You know this, and I know this, but if they want to shoot
      // themselves in the foot, why should we stop them?
      //for (size_t i=0; i<tmp.size(); i++) 
      //   if ( tmp[i] < 0.0 ) 
      //      EXCEPTION_MNGR(std::runtime_error, 
      //                     "WeightedSumApplication::cb_initialixe_xml()"
      //                     " - all weights must be non-negative.");
      weights = tmp;
   }
   

   void 
   cb_map_request( const AppRequest::request_map_t &local,
                   AppRequest::request_map_t &remote )
   {
      static_cast<void>(local);

      // convert f_info requests into mf_info requests
      if ( remote.erase(f_info) > 0 ) 
         remote.insert(AppRequest::request_pair_t(mf_info, utilib::Any()));
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

      // NB: get the wrapped objective values.  If none are present,
      // then return OK (nothing more this map needs to do) iif the
      // original request didn't include the objective.
      sub_iterator mf_it = sub_response.find(mf_info);
      if ( mf_it == sub_response.end() )
         return requests.count(f_info) ? 0 : -1;

      Problem<remote_app_t> base = remote_app->get_problem()
         .template expose<Problem<remote_app_t > >();

      size_t num = base->num_objectives;

      utilib::Any mf;
      utilib::TypeManager()->lexical_cast(mf_it->second, mf, typeid(vector_t));
      const vector_t &mf_v = mf.expose<vector_t>();
      
      if ( num != mf_v.size() )
         EXCEPTION_MNGR(std::runtime_error, "WeightedSumApplication::"
                        "cb_map_f_response - array of objectives "
                        "has " << mf_v.size() << " values, but " << num <<
                        " objectives are defined.");

      std::vector<optimizationSense> base_sense = base->sense;
      const weights_t& weight = weights.expose<weights_t>();

      real f_val = 0.0;
      for (size_t i = 0; i < num; i++) 
      {
         if ( base_sense[i] == minimization )
            f_val += weight[i] * mf_v[i];
         else 
            f_val -= weight[i] * mf_v[i];
      }
      response.insert(AppResponse::response_pair_t(f_info, f_val));
      return -1;
   }

   int
   cb_map_g_response( const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response )
   {
      //std::cerr << utilib::demangledName(typeid(this)) << "(g)" << std::endl;
      static_cast<void>(domain);
      typedef AppResponse::response_map_t::const_iterator sub_iterator;

      // Erase the g_info that was automatically propagated
      response.erase(g_info);
 
       // NB: get the wrapped gradient values.  If none are present,
      // then return OK (nothing more this map needs to do) iif the
      // original request didn't include the objective.
      sub_iterator g_it = sub_response.find(g_info);
      if ( g_it == sub_response.end() )
         return requests.count(g_info) ? 0 : -1;
 
      Problem<remote_app_t> base = remote_app->get_problem()
         .template expose<Problem<remote_app_t > >();

      size_t nObj = base->num_objectives;
      size_t nReal = this->num_real_vars;

      utilib::Any g_base;
      utilib::TypeManager()->lexical_cast
          (g_it->second, g_base, typeid(sparse_t));
      const sparse_t &g = g_base.expose<sparse_t>();
      
      if ( nObj != static_cast<size_t>(g.get_nrows()) )
         EXCEPTION_MNGR(std::runtime_error, "WeightedSumApplication::"
                        "cb_map_g_response - objective gradient matrix "
                        "has " << g.get_nrows() << " rows, but " << 
                        nObj << " objectives are defined.");
      if ( nReal < static_cast<size_t>(g.get_ncols()) )
         EXCEPTION_MNGR(std::runtime_error, "WeightedSumApplication::"
                        "cb_map_g_response - objective gradient matrix "
                        "has " << g.get_ncols() << " columns, but " << 
                        nReal << " variables are defined.");

      std::vector<optimizationSense> base_sense = base->sense;
      const weights_t& weight = weights.expose<weights_t>();

      utilib::Any ans;
      vector_t &g_val = ans.set<vector_t>();
      g_val.resize(nReal, 0.0);
      for (size_t i = 0; i < nObj; i++)
      {
         int offset = g.get_matbeg()[i];
         for( int j = g.get_matcnt()[i]; j>0; --j )
         {
            real tmp = weight[i] * g.get_matval()[offset];
            if ( base_sense[i] == minimization )
               g_val[g.get_matind()[offset]] += tmp;
            else
               g_val[g.get_matind()[offset]] -= tmp;

            ++offset;
         }
      }
      response.insert(AppResponse::response_pair_t(g_info, ans));
      return -1;
   }
};


} // namespace colin

#endif // defined colin_WeightedSumApplication_h
