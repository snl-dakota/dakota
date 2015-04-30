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

#include <acro_config.h>
#include <colin/AnalysisCode.h>
#include <colin/CommandShell.h>
#if !(defined(_MSC_VER) || defined(__MINGW32__))
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <process.h>
#endif
#include <utilib/string_ops.h>

using utilib::seed_t;

namespace colin
{


//============================================================================
//
//
AnalysisCode::AnalysisCode()
   : method(syscall), quietFlag(false)
{
   verboseFlag = ColinGlobals::output_level == "debug" ||
                 ColinGlobals::output_level == "verbose";
   quietFlag = ColinGlobals::output_level == "quiet";
}


//============================================================================
//
//
void AnalysisCode::define_filenames(const int id, const int analysis_servers)
{
   // Different analysis servers _must_ share the same parameters and
   // results file root names.  If temporary file names are not used,
   // then each evalComm processor can define the same names without
   // need for interprocessor communication.  However, if temporary
   // file names are used, then evalCommRank 0 must define the names
   // and broadcast them to other evalComm processors.
   int eval_comm_rank   = ColinGlobals::processor_id();
   bool bcast_flag = (analysis_servers > 1 &&
                      ((inputFileName.size() == 0) ||
                       (outputFileName.size() == 0)));

   if (eval_comm_rank == 0 || !bcast_flag)
   {
      std::ostringstream ctr_tag;
      ctr_tag << "." << eval_comm_rank << "_" << id;

      modifiedInputFileName = inputFileName;
      if (fileTagFlag)
      { modifiedInputFileName += ctr_tag.str(); }
      modifiedOutputFileName = outputFileName;
      if (fileTagFlag)
      { modifiedOutputFileName += ctr_tag.str(); }
   }

#if 0
   ifdef ACRO_HAVE_MPI
   if (bcast_flag)
   {
      MPI_Comm eval_comm = parallelLib.evaluation_intra_communicator();
      if (eval_comm_rank == 0)
      {
         // pack the buffer with root file names for the evaluation
         PackBuffer send_buffer;
         send_buffer << modifiedInputFileName << modifiedOutputFileName;
         // broadcast buffer length so that other procs can allocate UnPackBuffer
         //int buffer_len = send_buffer.len();
         //parallelLib.bcast(buffer_len, eval_comm);
         // broadcast actual buffer
         parallelLib.bcast(send_buffer, eval_comm);
      }
      else
      {
         // receive length of incoming buffer and allocate space for UnPackBuffer
         //int buffer_len;
         //parallelLib.bcast(buffer_len, eval_comm);
         // receive incoming buffer
         //UnPackBuffer recv_buffer(buffer_len);
         UnPackBuffer recv_buffer(256); // 256 should be plenty for 2 filenames
         parallelLib.bcast(recv_buffer, eval_comm);
         recv_buffer >> modifiedInputFileName >> modifiedOutputFileName;
      }
   }
#endif
}


//============================================================================
//
//
void AnalysisCode::read_output_file(AppResponse::response_map_t& response,
                                    const int id,
                                    seed_t& seed)
{
// Retrieve parameters and results file names using fn. eval. id.  A list of
// filenames is used because the names of tmp files must be available here
// and asynch_recv operations can perform output filtering out of order
// (which rules out making the file names into attributes of AnalysisCode or
// rebuilding the file name from the root name plus the counter).
   std::string& parameters_filename = inputFName[id];
   std::string& results_filename = outputFName[id];

   std::ifstream recovery_stream(results_filename.data());
   if (!recovery_stream)
      EXCEPTION_MNGR(std::runtime_error,
                     "AnalysisCode::read_parameters_file -  cannot open results file \"" << results_filename.data() << "\".");

//
// Process output file
//
#ifdef ACRO_USING_TINYXML
   TiXmlDocument doc;
   recovery_stream >> doc;
   recovery_stream.close();
   if (doc.Error())
      EXCEPTION_MNGR(std::runtime_error, "Error in " << doc.Value() << " : " << doc.Error());
      TiXmlHandle docHandle(&doc);
   TiXmlElement* child = docHandle.FirstChild("ColinResponse").ToElement();
   if (child)
   {
      for (child = child->FirstChild()->ToElement(); child; child = child->NextSiblingElement())
      {
         if (strcmp(child->Value(), "Seed") == 0)
         {
            std::istringstream istr(child->GetText());
            istr >> seed;
            if (!istr)
            {
               EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::read_output_file - Problem parsing the seed returned from the application");
            }
         }
         else
         {
            response_info_t id;
            try
            {
               id = AppResponseXML(child->Value());
            }
            catch (std::runtime_error& err)
            {
               EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::read_output_file - Unknown application value: " << child->Value());
            }
            response.insert( AppResponse::response_pair_t
                             ( id, AppResponseXML(id)->process(child) ) );
         }
      }
   }
#endif

// Clean up files based on fileSaveFlag.
   if (!fileSaveFlag)
   {
      if (!quietFlag && verboseFlag)
      {
         ucout << "Removing " << parameters_filename << " and "
         << results_filename;
         ucout << '\n';
      }
      remove(parameters_filename.data());
      remove(results_filename.data());
   }

// Prevent overwriting of files with reused names for which a file_save
// request has been given.
   if (fileSaveFlag && !fileTagFlag &&
         (!inputFileName.size() || !outputFileName.size()))
   {
      //
      if (!quietFlag && verboseFlag)
         ucout << "Files with nonunique names will be tagged for file_save:\n";
      char ctr_tag[16];
      sprintf(ctr_tag, ".%d", id);
      if (!inputFileName.size())
      {
         std::string new_str = inputFileName + ctr_tag;
         if (!quietFlag && verboseFlag)
            ucout << "Moving " << inputFileName << " to " << new_str << '\n';
         rename(inputFileName.data(), new_str.data());
      }
      if (!outputFileName.size())
      {
         std::string old_str = outputFileName;
         std::string new_str = outputFileName + ctr_tag;
         //if (numPrograms == 1 || (oFilterName.size() > 0)) {
         if (!quietFlag && verboseFlag)
            ucout << "Moving " << old_str << " to " << new_str << '\n';
         rename(old_str.data(), new_str.data());
         //}
      }
   }
//
// Remove the evaluation which has been processed from the filename lists
//
   inputFName.erase(id);
   outputFName.erase(id);
}


//============================================================================
//
//
void AnalysisCode::setup_args()
{
utilib::CharString tmp = this->programName.data();
tmp += " "; 
tmp += this->modifiedInputFileName;
tmp += " ";
tmp += this->modifiedOutputFileName;

args_data = utilib::split(tmp,' ');
args.resize(args_data.size()+3);
int j=0;
for (size_t i=0; i<args_data.size(); i++) {
  if (args_data[i] != "") {
     args[j++] = args_data[i].data();
  }
}
args_in = j;
args_out = j+1;
args[j++]=0;
args[j++]=0;
args[j]=0;
}


//============================================================================
//
//
void AnalysisCode::spawn_evaluation(bool block_flag)
{
switch (method) {
#if defined(_MSC_VER) || defined(__MINGW32__)
  case spawn:
	//std::cerr << "SPAWN" << std::endl;
	windows_spawn(block_flag);
	break;
  case fork:
	EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::spawn_evaluation - fork evaluations not supported under windows.");
	break;
#else
  case spawn:
	EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::spawn_evaluation - spawn evaluations not supported under windows.");
	break;
  case fork:
	//std::cerr << "FORK" << std::endl;
	fork_spawn(block_flag);
	break;
#endif
  case syscall:
	//std::cerr << "SYSCALL" << std::endl;
	syscall_spawn(block_flag);
	break;
	
};
}


//============================================================================
//
//
void AnalysisCode::syscall_spawn(bool block_flag)
{
   CommandShell shell;

   #ifndef _WIN32
   shell << "(";
   #endif

   shell << this->programName << " " << this->modifiedInputFileName << " "
		<< this->modifiedOutputFileName;

   #ifndef _WIN32
   shell << " ; )" ;
   #endif

   // Process definition complete; now set the shell's asynchFlag/quietFlag from
   // the incoming block_flag & the program's quietFlag and spawn the process.
   shell.asynch_flag(!block_flag);
   shell.quiet_flag(!this->verboseFlag);
   shell << flush;
}


//============================================================================
//
//
#if !(defined(_MSC_VER) || defined(__MINGW32__))
void AnalysisCode::fork_spawn(bool block_flag)
{
   pid_t pid = vfork();
   if (pid == 0) // child
   {
      char*const args[] = {  
		        const_cast<char*>(this->programName.data()),
			const_cast<char*>(this->modifiedInputFileName.data()),
			const_cast<char*>(this->modifiedOutputFileName.data()),
			0};
      if (execvp(this->programName.data(), args) < 0) {
         utilib::CharString cmd;
         cmd += this->programName;
         cmd += " ";
         cmd += this->modifiedInputFileName;
         cmd += " ";
         cmd += this->modifiedOutputFileName;
         EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::fork_spawn - failed to launch command \"" << cmd << "\"");
      }
   }
   else // parent
   {
   // WEH - should the parent wait?
   if (!block_flag) 
	{ return; }
   int childExitStatus;
   pid_t ws = waitpid( pid, &childExitStatus, 0);
   if( !WIFEXITED(childExitStatus) )
   {
     EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::fork_spawn - waitpid() exited with an error: Status= " << WEXITSTATUS(childExitStatus));
   }
   else if( WIFSIGNALED(childExitStatus) )
   {
      EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::fork_spawn - waitpid() exited due to a signal: " << WTERMSIG(childExitStatus));
   }
   }
}
#endif


//============================================================================
//
//
#if defined(_MSC_VER) || defined(__MINGW32__)
void AnalysisCode::windows_spawn(bool block_flag)
{
intptr_t pid;
int tstat;

args[args_in] = const_cast<char*>(this->modifiedInputFileName.data());
args[args_out] = const_cast<char*>(this->modifiedOutputFileName.data());
#if 0
std::cerr << "SPAWN" << std::endl;
size_t i=0;
while (args[i] != 0) {
  std::cerr << args[i++] << std::endl;
}
#endif

pid = _spawnvp(_P_NOWAIT, args[0], &(args[0]));
if (pid < 0) {
   EXCEPTION_MNGR(std::runtime_error, "AnalysisCode::windows_spawn - spawn failed for command \"" << this->programName << " " << this->modifiedInputFileName << " " << modifiedOutputFileName << "\"");
}
if (block_flag) {
   _cwait(&tstat, pid, WAIT_CHILD);
}
}
#endif

}
