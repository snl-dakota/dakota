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
 * \file AnalysisCode.h
 *
 * Defines the colin::AnalysisCode class.
 */

#ifndef colin_AnalysisCode_h
#define colin_AnalysisCode_h

#include <acro_config.h>

#include <colin/ColinGlobals.h>
#include <colin/AppResponseXML.h>
#include <colin/AppResponse.h>
#include <colin/AppRequest.h>

#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/CommonIO.h>
#include <utilib/CharString.h>
#include <utilib/exception_mngr.h>

namespace colin
{

/** \class AnalysisCode
  *
  * A class that supports a variety of mechanisms for spawning separate 
  * processes for managing simulations.
  */
class AnalysisCode
{
public:

   ///
   enum method_t {syscall, fork, spawn};

   /// Constructor
   AnalysisCode();

   /// Destructor
   virtual ~AnalysisCode() {}

   /// Setup the analysis code
   void setup(const std::string& program_name,
              const std::string& input_filename,
              const std::string& output_filename,
              const bool tag_flag = true, const bool save_flag = false,
	      method_t method_=syscall)
   {
      programName = program_name;
      inputFileName = input_filename;
      outputFileName = output_filename;
      fileTagFlag = tag_flag;
      fileSaveFlag = save_flag;
      method = method_;
      setup_args();
   }

   /** Define modified filenames from user input by handling Unix
     * temp file and tagging options.
     */
   void define_filenames(const int id, const int num_analysis_servers = 1);

   /// write the variables and info requests to the
   /// parameters file in XML format
   template <class DomainT>
   void write_input_file(const DomainT& vars,
                         const AppRequest::request_map_t& requests,
                         const int id,
                         const utilib::seed_t seed);

   /// read the output file to fill a response object
   void read_output_file(AppResponse::response_map_t& response,
                         const int id,
                         utilib::seed_t& seed);

   /// Spawn an evaluation with the analysis code.
   void spawn_evaluation(bool block_flag);

protected: // data

   int args_in;
   int args_out;
   ///
   std::vector<char*> args;

   ///
   utilib::BasicArray<utilib::CharString> args_data;

   /// Method for launching the application code
   method_t method;

   /// Flags tagging of parameter/results files
   bool fileTagFlag;

   /// Flags retention of parameter/results files
   bool fileSaveFlag;

   /// The name of the analysis code program
   std::string programName;

   /// The name of the input file from user specification
   std::string inputFileName;

   /// The input file name actually used (modified with tagging
   /// or temp files)
   std::string modifiedInputFileName;

   /// The name of the output file from user specification
   std::string outputFileName;

   /// The output file name actually used (modified with tagging
   /// or temp files)
   std::string modifiedOutputFileName;

   /// Input file names used in spawning function evaluations
   std::map<int, std::string> inputFName;

   /// Output file names used in spawning function evaluations
   std::map<int, std::string> outputFName;

   /// TODO
   bool verboseFlag;

   /// TODO
   bool quietFlag;

   ///
   void setup_args();

#if defined(_MSC_VER) || defined(__MINGW32__)
   /// Spawn an evaluation with a windows spawn
   void windows_spawn(bool block_flag);
#else
   /// Spawn an evaluation with a system fork
   void fork_spawn(bool block_flag);
#endif

   /// Spawn an evaluation with a system call
   void syscall_spawn(bool block_flag);

};


//============================================================================
//
//
template <class DomainT>
void AnalysisCode::write_input_file(const DomainT& vars,
                                    const AppRequest::request_map_t& requests,
                                    const int id,
                                    const utilib::seed_t seed)
{
   // NOTE: we assume that these names are not already in use.  This is 
   // different from what DAKOTA does, which allows for redundant evaluations 
   // when performing a retry.  However, COLIN will use an internal cache to 
   // avoid this in the first place.
   inputFName[id] = modifiedInputFileName;
   outputFName[id] = modifiedOutputFileName;

   //
   // Some debuging I/O
   //
   if (verboseFlag)
   {
      ucout << "\nFile name list entries at fn. eval. " << id << '\n';
      std::map<int, std::string>::const_iterator currParams = inputFName.begin();
      std::map<int, std::string>::const_iterator lastParams = inputFName.end();
      std::map<int, std::string>::const_iterator currResults = outputFName.begin();
      while (currParams != lastParams)
      {
         ucout << "  " << currParams->second << " " << currResults->second << " "
         << currParams->first << '\n';
         currParams++;
         currResults++;
      }
      ucout << std::endl;
   }
   //
   // Write the parameters file
   //
   std::ofstream parameter_stream(inputFName[id].data());
   if (!parameter_stream)
   {
      EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::write_input_file - "
                     "cannot create parameters file \"" <<
                     modifiedInputFileName.data() << "\".");
   }
   //
   // Write out in XML format
   //
#ifdef ACRO_USING_TINYXML
   TiXmlElement ElementColinRequest("ColinRequest");

   ElementColinRequest.LinkEndChild(XMLParameters(vars));

   TiXmlElement SeedElement("Seed");
   {
      std::ostringstream ostr;
      ostr << seed;
      SeedElement.LinkEndChild(new TiXmlText(ostr.str().c_str()));
   }
   ElementColinRequest.InsertEndChild(SeedElement);

   TiXmlElement ElementRequests("Requests");
   {
      AppRequest::request_map_t::const_iterator curr = requests.begin();
      AppRequest::request_map_t::const_iterator end  = requests.end();
      while (curr != end)
      {
         ElementRequests.LinkEndChild
         (new TiXmlElement(AppResponseXML(curr->first)->element_name()));
         curr++;
      }
   }
   ElementColinRequest.InsertEndChild(ElementRequests);

   TiXmlDocument doc;
   doc.InsertEndChild(ElementColinRequest);
   //doc.Print();
   parameter_stream << doc;
#else
   EXCEPTION_MNGR(std::logic_error, "AnalysisCode::write_input_file(): "
                  "cannot write input file: compiled without "
                  "ACRO_USING_TINYXML");
#endif
   //
   // Explicit flush and close added to prevent problem of reading input
   // file before the write was completed.
   //
   parameter_stream.flush();
   parameter_stream.close();
}


} // namespace colin

#endif
