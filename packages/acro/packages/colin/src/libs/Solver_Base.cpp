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

#include <colin/Utilities.h>
#include <colin/solver/Base.h>
#include <colin/SolverMngr.h>
#include <colin/StatusCodes.h>
#include <colin/ApplicationMngr.h>
#include <colin/TinyXML_data_parser.h>
#include <utilib/TinyXML_helper.h>

using std::string;
using std::endl;

using std::list;
using std::set;
using std::map;
using std::make_pair;

using utilib::Any;


namespace colin {

struct Solver_Base::Data
{
   Data()
      : construct_map(), 
        init_cache(false), 
        final_cache(true), 
        optimal_cache(true)
   {}

   struct CacheInfo {
      CacheInfo(bool reset_)
         : has_data(false), 
           reset(reset_), 
           name(cache::ImplicitInterSolverCacheName), 
           points()
      {}
      
      bool       has_data;
      bool       reset;
      string     name;
      list<Any>  points;
   };

/// Container to hold the XML sub-element processors
   /** Container to hold the XML sub-element processors
    *
    *  This is a trick: boost::signals are not copyable, and thus cannot
    *  be put inside STL containers.  However, if we first put the
    *  signal * into a non-copyable Any, and then put the Any into the
    *  STL * container, all is well and good in the world!
    */
   typedef map<string, Any>  ConstructMap_t;

   ///
   ConstructMap_t construct_map;
   ///
   CacheInfo init_cache;
   ///
   CacheInfo final_cache;
   ///
   CacheInfo optimal_cache;
};


//============================================================================
//
//
Solver_Base:: Solver_Base()
   : solver_statistics(true),
     data(new Data)
{
   solver_status.solver_status=solver_unknown;
   solver_status.model_status=model_status_unknown;
   solver_status.termination_condition=termination_unknown;

   properties.normalizeKeys() = true;

   // register the local reset function
   reset_signal.connect
      (boost::bind(&Solver_Base::reset_Solver_Base, this));

   // register the results generator
   results_signal.connect
      (boost::bind(&Solver_Base::cb_results, this, _1, _2));

   // register the default XML components
   register_construct("Problem").connect
      (boost::bind(&Solver_Base::process_xml_problem, this, _1, _2));

   register_construct("InitialPoint").connect
      (boost::bind(&Solver_Base::process_xml_initialPoint, this, _1, _2));

   register_construct("FinalPoint").connect
      (boost::bind(&Solver_Base::process_xml_finalPoint, this, _1, _2));

   //register_construct("OptimalPoint").connect
   //   (boost::bind(&Solver_Base::process_xml_optimalPoint, this, _1, _2));

   register_construct("Options").connect
      (boost::bind(&Solver_Base::process_xml_options, this, _1, _2));
}



/// Virtual destructor
Solver_Base::~Solver_Base()
{
   delete data;
}


std::string
Solver_Base::type() const
{
   string ans = SolverMngr().get_solver_type(this);
   if ( ans.empty() )
      return define_solver_type();
   else
      return ans;
}


void Solver_Base::construct(TiXmlElement* root, bool describe)
{
   if ( root == NULL )
      return;
   if ( describe )
   {
      root->SetAttribute("id", "string");

      // Now, describe all registered sub-elements
      Data::ConstructMap_t::iterator it = data->construct_map.begin();
      Data::ConstructMap_t::iterator itEnd = data->construct_map.end();
      for ( ; it != itEnd; ++it )
      {
         TiXmlElement* node = new TiXmlElement(it->first);
         root->LinkEndChild(node);
         it->second.expose<ConstructSignal_t>()(node, true); 
      }
      return;
   }

   // Parse my attributes first
   const char* name = root->Attribute("id");
   if ( name != NULL )
      SolverMngr().reregister_solver(this, name);

   // Now, parse all valid sub-elements
   Data::ConstructMap_t &init = data->construct_map;
   Data::ConstructMap_t::iterator it;
   Data::ConstructMap_t::iterator itEnd = init.end();
   TiXmlElement* node = root->FirstChildElement();
   while ( node != NULL )
   {
      it = init.find(node->ValueStr());
      if ( it == itEnd )
         EXCEPTION_MNGR(std::runtime_error, "Solver_Base::construct(): "
                        "No handler registered for " << 
                        utilib::get_element_info(node));

      it->second.expose<ConstructSignal_t>()(node, false);
      node = node->NextSiblingElement();
   }
}


utilib::Property& Solver_Base::property( std::string name )
{
   utilib::PropertyDict::iterator it = properties.find(name);
   if ( it == properties.end() )
      EXCEPTION_MNGR(std::runtime_error, "Solver_Base::property(): "
                     "Attempt to retrieve nonexistent property, '"
                     << name << "'");

   return it->second();
}


void Solver_Base::set_initial_points(PointSet ps)
{
   initial_points = ps;

   // This action overrides any intial point info specified through configure()
   data->init_cache.name = "";
   data->init_cache.points.clear();
   data->init_cache.has_data = false;
}


void Solver_Base::add_initial_point(const utilib::AnyRef point)
{
   // If there was info specified through configure(), parse it now.
   if ( data->init_cache.has_data )
      initialize_xml_init_cache();

   if ( point.is_type(typeid(AppResponse)) )
   {
      initial_points->insert(point.expose<AppResponse>());
      return;
   }

   ApplicationHandle app = get_problem_handle();
   if ( app.empty() )
      EXCEPTION_MNGR(std::logic_error, "Solver_Base::add_initial_point(): "
                     "Cannot add initial domain points before setting "
                     "the problem.");

   // Should we force an evaluation of the objective function here?
   // Right now we are just inserting an empty response.
   initial_points->insert
      ( eval_mngr().perform_evaluation(app->set_domain(point)) );
}


PointSet Solver_Base::get_final_points() const
{
   return final_points;
}


//PointSet Solver_Base::get_optimal_points() const
//{
//   return optimal_points;
//}


void Solver_Base::set_statistic(std::string name, utilib::Any value)
{
   if ( solver_statistics.exists(name) )
   {
      ucerr << "WARNING: overwriting solver statistic '"
            << name << "' = " << solver_statistics[name] << endl
            << "         with new value = " << value << endl;
   }
   solver_statistics[name] = value;
}


//========================================================================
// Solver_Base protected members
//========================================================================

boost::signal<void(TiXmlElement*,bool)>& 
Solver_Base::register_construct(std::string element)
{
   Any& ans = data->construct_map[element];
   if ( ans.empty() )
      ans.set<ConstructSignal_t, Any::NonCopyable<ConstructSignal_t> >();
   return ans.expose<ConstructSignal_t>();
}



//========================================================================
// Solver_Base private members
//========================================================================

void
Solver_Base::reset_Solver_Base()
{
   // If we got cache configuration information through configure(), but
   // are not being executed by the XML parser (i.e. the ExecuteMngr),
   // we need to actually setup / initialize our caches based on that
   // information.  As initialize_xml_*_cache() clears the incoming
   // data, this will only be executed te first time reset is called.
   // Additionally, if the initial data was parsed using the
   // SolverExecuteFunctor, SolverExecuteFunctor::execute() sets
   // has_data to false, so nothing will happen here.

   if ( data->init_cache.has_data )
      initialize_xml_init_cache();

   if ( data->final_cache.has_data )
      initialize_xml_final_cache();

   //if ( data->optimal_cache.has_data )
   //   initialize_xml_optimal_cache();
}

void
Solver_Base::cb_results(utilib::PropertyDict &pd, int verbosity)
{
   pd["problem"] = get_problem_handle()->describe(verbosity);
   utilib::PropertyDict solver_pd = utilib::PropertyDict(true);
   pd["solver"] = solver_pd;
   solver_pd["name"] = define_solver_type();
   solver_pd["status"] = solver_status.describe(verbosity);
   solver_pd["statistics"] = solver_statistics;
}


void 
Solver_Base::process_xml_problem( TiXmlElement* node, bool describe )
{
   if ( describe )
   {
      node->SetAttribute("id", "string");
      return;
   }

   string problem;
   utilib::get_string_attribute( node, "id", problem, "" );
   if ( ! problem.empty() )
      set_problem(ApplicationMngr().get_application(problem));
   if ( get_problem_handle().empty() )
   {
      // Attempt to use the most recently created application 
      problem = ApplicationMngr().get_newest_application();
      if ( ! problem.empty() )
         set_problem( ApplicationMngr().get_application(problem) );
   }

   if ( ! get_problem_handle().empty() )
      get_problem_handle()->initialize(node);
}

void
Solver_Base::process_xml_initialPoint( TiXmlElement* node, bool describe )
{
   if ( describe )
      return;

   bool cache_specified = false;
   cache_specified |= utilib::get_string_attribute
      ( node, "cache", data->init_cache.name, 
        cache::ImplicitInterSolverCacheName );
   cache_specified |= utilib::get_bool_attribute
      (node, "clear", data->init_cache.reset, false);

   data->init_cache.points.clear();
   TiXmlElement* elt = node->FirstChildElement(); 
   if ( elt == NULL )
   {
      // Convenience: if there are no child elements, but it has a
      // single text child, assume it is a point
      const char* text = node->GetText();
      if ( text != NULL )
         data->init_cache.points.push_back(parse_xml_data(node));
   }
   for( ; elt != NULL; elt = elt->NextSiblingElement() )
   {
      if ( elt->ValueStr().compare("Point") != 0 )
         EXCEPTION_MNGR(std::runtime_error, "[Solver_Base] "
                        "process_xml_options(): invalid element "
                        << elt->ValueStr() << " in " 
                        << utilib::get_element_info(elt));

      data->init_cache.points.push_back(parse_xml_data(elt));
   }

   if ( ! cache_specified && ! data->init_cache.points.empty() )
      data->init_cache.reset = true;

   data->init_cache.has_data = true;
}


void
Solver_Base::process_xml_finalPoint( TiXmlElement* node, bool describe )
{
   if ( describe )
      return;

   utilib::get_string_attribute
      ( node, "cache", data->final_cache.name, "" );
   utilib::get_bool_attribute
      (node, "clear", data->final_cache.reset, true);

   data->final_cache.has_data = true;
}


//void
//Solver_Base::process_xml_optimalPoint( TiXmlElement* node, bool describe )
//{
//   if ( describe )
//      return;
//
//   utilib::get_string_attribute
//      ( node, "cache", data->optimal_cache.name, ImplicitInterSolverCacheName );
//   utilib::get_bool_attribute
//      (node, "clear", data->optimal_cache.reset, true);
//
//   data->optimal_cache.has_data = true;
//}


void 
Solver_Base::process_xml_options( TiXmlElement* node, bool describe )
{
   if ( describe )
   {
      TiXmlElement *opt = new TiXmlElement("Option");
      opt->SetAttribute("name", "string");
      node->LinkEndChild(opt);
      return;
   }

   TiXmlElement* n = node->FirstChildElement();
   for( ; n != NULL; n = n->NextSiblingElement() )
   {
      if ( n->ValueStr().compare("Option") != 0 )
         EXCEPTION_MNGR(std::runtime_error, "[Solver_Base] "
                        "process_xml_options(): invalid element "
                        << n->ValueStr() << " in " 
                        << utilib::get_element_info(n));
      string name = "";
      utilib::get_string_attribute(n, "name", name);
      properties[name] = parse_xml_data(n);
   }
}


void
Solver_Base::initialize_xml_init_cache()
{
   //
   // Now we actually set up everything for the initial cache...
   //
   if ( ! data->init_cache.name.empty() )
   {
      CacheHandle cache = CacheFactory().get_cache(data->init_cache.name);
      if ( cache.empty() )
      {
         // register a new instance of the cache
         cache = PointSet().operator->()->get_handle();
         CacheFactory().register_cache(cache, data->init_cache.name);
      }
      
      if ( data->init_cache.reset )
         cache->clear();

      initial_points = PointSet(cache);
   }

   // Must set has_data to false, or add_initial_point will infinitely recurse
   data->init_cache.has_data = false;

   list<Any> &points = data->init_cache.points;
   for (; ! points.empty(); points.pop_front() )
      add_initial_point(points.front());

   data->init_cache = Data::CacheInfo(false);
}


void
Solver_Base::initialize_xml_final_cache()
{
   CacheHandle cache = CacheFactory().get_cache(data->final_cache.name);
   if ( data->final_cache.reset && ! cache.empty() )
   {
      // Don't clear the cache (we might be using it for our input
      // points!).  Rather, unregister it and let the CacheHandle take
      // care of deleting it when the last one falls out of scope.
      CacheFactory().unregister_cache(data->final_cache.name);
      cache = CacheHandle();
   }
   if ( cache.empty() )
   { 
      // defer to the default cache used by PointSets
      cache = PointSet().operator->()->get_handle();
      CacheFactory().register_cache(cache, data->final_cache.name);
   } 
   final_points = PointSet(cache);
   data->final_cache = Data::CacheInfo(true);
}


//void
//Solver_Base::initialize_xml_optimal_cache()
//{
//   CacheHandle cache = CacheFactory().get_cache(data->optimal_cache.name);
//   if ( data->optimal_cache.reset && ! cache.empty() )
//   {
//      // Don't clear the cache (we might be using it for our input
//      // points!).  Rather, unregister it and let the CacheHandle take
//      // care of deleting it when the last one falls out of scope.
//      CacheFactory().unregister_cache(data->optimal_cache.name);
//      cache = CacheHandle();
//   }
//   if ( cache.empty() )
//   { 
//      // defer to the default cache used by PointSets
//      cache = PointSet().operator->()->get_handle();
//      CacheFactory().register_cache(cache, data->optimal_cache.name);
//   } 
//   optimal_points = PointSet(cache);
//   data->optimal_cache = Data::CacheInfo(true);
//}

} // namespace colin
