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
 * \file AnalysisCodeApplication.h
 *
 * Defines the colin::AnalysisCodeApplication class.
 */

#ifndef colin_AnalysisCodeApplication_h
#define colin_AnalysisCodeApplication_h

#if 0
#ifdef __MINGW32__
#include <_mingw.h>
#include <windows.h>
#endif
#include <acro_config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utilib/std_headers.h>
#include <utilib/CachedAllocator.h>
#include <colin/CommandShell.h>
#endif

#include <colin/AnalysisCode.h>
#include <colin/ConfigurableApplication.h>
#include <colin/SynchronousApplication.h>

#include <utilib/TinyXML_helper.h>

namespace colin
{


/** The problem provides an interface for functions that are evaluated
 *  in an external analysis code that is spawned by this class.
 */
template <class ProblemT, class DomainT>
class AnalysisCodeApplication 
   : public Synchronous<ConfigurableApplication<ProblemT> >
{
public:

   /// Constructor.
   AnalysisCodeApplication()
   { 
      this->initializer("Driver").connect
         (boost::bind(&AnalysisCodeApplication::cb_initialize, this, _1));
   }

   /// Destructor
   virtual ~AnalysisCodeApplication()
   {}

   /// Setup this application
   void setup(const std::string& program, const std::string& input_filename,
              const std::string& output_filename,
              bool ctr_suffix = true,
              bool remove_files = true,
	      AnalysisCode::method_t method=AnalysisCode::syscall)
   {
      simulator.setup(program, input_filename, output_filename,
                      ctr_suffix, !remove_files, method);
   }


protected:

   /// 
   virtual bool 
   map_domain(const utilib::Any &src, utilib::Any &native, bool forward) const
   { 
      static_cast<void>(forward);
      return utilib::TypeManager()->lexical_cast
         (src, native, typeid(DomainT)) == 0;
   }


   // Ideally, we this API would support evaluations in non-blocking
   // mode (i.e. spawn_evaluation(false)).  However, the function
   // doesn't return anything like a process ID, so we have nothing to
   // poll or spin-lock on (i.e. waitpid())...  so for now, this will
   // only support blocking execution.

   virtual void 
   perform_evaluation_impl( const utilib::Any &domain,
                            const AppRequest::request_map_t &requests,
                            utilib::seed_t &seed,
                            AppResponse::response_map_t &responses )
   {
      int id = get_unique_id();
      simulator.define_filenames(id);
      simulator.write_input_file(domain.expose<DomainT>(), requests, id, seed);

      simulator.spawn_evaluation(true);

      simulator.read_output_file(responses, id, seed);
   }
  
#if 1 // TRUE until we implement a true async interface to AnalysisCode
#else
   /// 
   virtual 
   utilib::Any spawn_evaluation( const utilib::Any &domain,
                                 const AppRequest::request_map_t &requests,
                                 utilib::seed_t &seed )
   {
      int id = get_unique_id();

      simulator.define_filenames(id);
      simulator.write_input_file(domain.expose<DomainT>(), requests, id, seed);

      simulator.spawn_evaluation(true);

      request_list.push_front(id);
      return id;
   }


   /// 
   virtual 
   utilib::Any collect_evaluation( AppResponse::response_map_t &responses,
                                   utilib::seed_t &seed )
   {
      if ( request_list.empty() )
         EXCEPTION_MNGR(std::runtime_error, "AnalysisCodeApplication::"
                        "collect_evaluation: no spawned requests to collect");

      std::list<int>::iterator it = request_list.begin();

      int id = *it;
      request_list.erase(it);

      simulator.read_output_file(responses, id, seed);
      return id;
   }

   std::list<int> request_list;
#endif

   /// The simulator object
   AnalysisCode simulator;

private:

   ///
   int get_unique_id()
   {
      static int id = 0;
      return ++id;
   }

   ///
   void cb_initialize(TiXmlElement* elt)
   {
      //std::string workingdir=".";
      std::string request_prefix = "colin.in";
      std::string response_prefix = "colin.out";
      std::string program = "unknown";
      bool keep_files = false;
      bool no_counter_suffix = false;
      #if _WIN32
      AnalysisCode::method_t method=AnalysisCode::spawn;
      #else
      AnalysisCode::method_t method=AnalysisCode::syscall;
      #endif

      TiXmlElement* node = elt->FirstChildElement();
      for( ; node != NULL; node = node->NextSiblingElement() )
      {
         const std::string& value = node->ValueStr();

         if ( value.compare("RequestPrefix") == 0 )
            request_prefix = node->GetText(); 
         else if ( value.compare("ResponsePrefix") == 0 )
            response_prefix = node->GetText(); 
         else if ( value.compare("Command") == 0 )
            program = node->GetText(); 
         else if ( value.compare("KeepFiles") == 0 )
            keep_files = true; 
         else if ( value.compare("NoCounterSuffix") == 0 )
            no_counter_suffix = true; 
         else if ( value.compare("Method") == 0 )
         {
            std::string tmp;
            utilib::get_string_attribute(node, "type", tmp, "syscall");
            if ( tmp.compare("syscall") == 0 )
               method = AnalysisCode::syscall;
            else if ( tmp.compare("fork") == 0 )
               method = AnalysisCode::fork;
            else if ( tmp.compare("spawn") == 0 )
               method = AnalysisCode::spawn;
            else
               EXCEPTION_MNGR(std::runtime_error, "AnalysisCodeApplication::"
                              "xml_initialize: unknown Method");
         }
#if 0
         else if ( value.compare("WorkingDirectory") == 0 )
            workingdir = node->GetText(); 
#endif
         else
            EXCEPTION_MNGR(std::runtime_error, "AnalysisCodeApplication::"
                           "xml_initialize: unknown element \"" << value 
                           << "\" in " << utilib::get_element_info(elt));
      }

      if (program == "unknown")
      {
         EXCEPTION_MNGR(std::runtime_error, "AnalysisCodeApplication::"
                        "xml_initialize - no command specified.");
      }
      setup(program, request_prefix, response_prefix,
            !no_counter_suffix, !keep_files, method);
   }


};


} // namespace colin

#endif // defined colin_AnalysisCodeApplication_h
