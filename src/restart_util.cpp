/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A utility for printing, repairing, and merging restart files.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: restart_util.cpp 6818 2010-06-05 00:46:38Z dmgay $

/** \file restart_util.cpp
    \brief file containing the DAKOTA restart utility main program */

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/program_options.hpp>
#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#ifdef HAVE_PDB_H
#include <pdb.h>
#endif

using namespace std;
using namespace Dakota;


namespace Dakota {

using std::endl;

/// print restart utility help message
void print_usage(std::ostream& s);

/// print a restart file
void print_restart(StringArray pos_args, String print_dest);
/// print a restart file (PDB format)
void print_restart_pdb(StringArray pos_args, String print_dest);
/// print a restart file (tabular format)
void print_restart_tabular(StringArray pos_args, String print_dest, 
			   unsigned short tabular_format, int tabular_precision);
/// read a restart file (neutral file format)
void read_neutral(StringArray pos_args);
/// repair a restart file by removing corrupted evaluations
void repair_restart(StringArray pos_args, String identifier_type);
/// concatenate multiple restart files
void concatenate_restart(StringArray pos_args);

} // namespace Dakota


/// The main program for the DAKOTA restart utility.

/** Parse command line inputs and invoke the appropriate utility
    function (print_restart(), print_restart_tabular(),
    read_neutral(), repair_restart(), or concatenate_restart()). */

int main(int argc, char* argv[])
{
  std::string util_command;                 // restart utility mode
  std::vector<std::string> pos_args;        // all remaining positional args
  bool freeform = false;                    // whether freeform requested
  std::vector<std::string> tabular_opts;    // custom_annotated options
  int tabular_precision = write_precision;  // tabular write precision
  try {
    // setup command-line options
    namespace bpo = boost::program_options;
    // options in include in help message
    bpo::options_description general_opts("options");
    general_opts.add_options()
      ("help", "show dakota_restart_util help message")
      ("custom_annotated",
       bpo::value<std::vector<std::string> >(&tabular_opts)->multitoken(), 
       "tabular file options: header, eval_id, interface_id")
      ("freeform", "tabular file: freeform format")
      ("output_precision", 
       bpo::value<int>(&tabular_precision)->default_value(write_precision),
       "set tabular output precision")
      ;
    // positional arguments to hide
    bpo::options_description hidden_opts("positional options");
    hidden_opts.add_options()
      ("util_command", bpo::value<std::string>(&util_command), 
       "restart utility command/mode")
      ("pos_args", bpo::value<std::vector<std::string> >(&pos_args), 
       "positional opts")
      ;
    // concatenated options
    bpo::options_description all_opts("all options");
    all_opts.add(general_opts).add(hidden_opts);
    // positional options are the restart_util command and any options to it
    bpo::positional_options_description positional_opts;
    positional_opts.add("util_command", 1).add("pos_args", -1);

    // parse the command line
    bpo::variables_map vm;
    bpo::store(bpo::command_line_parser(argc, argv).
	       options(all_opts).positional(positional_opts).run(), vm);
    bpo::notify(vm);    

    if (vm.count("help")) {
      print_usage(cout);
      cout << general_opts << std::endl;
      return 0;
    }
    if (util_command.empty() || pos_args.empty()) {
      print_usage(Cerr);
      Cerr << general_opts << std::endl;
      return -1;
    }
    if (vm.count("freeform"))
      freeform = true;
    if (vm.count("freeform") && vm.count("custom_annotated")) {
      Cerr << "\nError: options --freeform and --custom_annotated are mutually "
	   << "exclusive.\n";
      return -1;
    }
    if (tabular_precision < 1) {
      Cerr << "\nError: output_precision must be a positive integer.\n";
      return -1;
    }
  }
  catch (const std::exception& e) {
    Cerr << "\nError parsing command-line options: " << e.what() << std::endl;
    return -1;
  }

  unsigned short tabular_format = TABULAR_ANNOTATED;  // default
  if (freeform)
    tabular_format = TABULAR_NONE;
  else if (!tabular_opts.empty()) {
    bool found_error = false;
    tabular_format = TABULAR_NONE;
    BOOST_FOREACH(const String& tab_opt, tabular_opts) {
      if (tab_opt == "header")
	tabular_format |= TABULAR_HEADER;
      else if (tab_opt == "eval_id")
	tabular_format |= TABULAR_EVAL_ID;
      else if (tab_opt == "interface_id")
	tabular_format |= TABULAR_IFACE_ID;
      else {
	Cerr << "Error: unknown custom_annotated option '" << tab_opt << "'\n";
	found_error= true;
      }
    }
    if (found_error)
      return -1;
  }
  
  if (util_command == "print")
    print_restart(pos_args, "stdout");
  else if (util_command == "to_neutral")
    print_restart(pos_args, "neutral_file");
  else if (util_command == "from_neutral")
    read_neutral(pos_args);
  else if (util_command == "to_pdb")
    print_restart_pdb(pos_args, "pdb_file");
  else if (util_command == "to_tabular")
    print_restart_tabular(pos_args, "text_file", tabular_format, 
			  tabular_precision);
  else if (util_command == "remove")
    repair_restart(pos_args, "by_value");
  else if (util_command == "remove_ids")
    repair_restart(pos_args, "by_id");
  else if (util_command == "cat")
    concatenate_restart(pos_args);
  else {
    Cerr << "Error: command '" << util_command << "' not supported." << endl;
    print_usage(Cerr);
    return -1;
  }

  return 0;
}


namespace Dakota {

void print_usage(std::ostream& s)
{
  s << "Usage:\n  dakota_restart_util command <arg1> [<arg2> <arg3> ...] --options\n"
    << "    dakota_restart_util print <restart_file>\n"
    << "    dakota_restart_util to_neutral <restart_file> <neutral_file>\n"
    << "    dakota_restart_util from_neutral <neutral_file> <restart_file>\n"
#ifdef HAVE_PDB_H
    << "    dakota_restart_util to_pdb <restart_file> <pdb_file>\n"
#endif
    << "    dakota_restart_util to_tabular <restart_file> <text_file> [--custom_annotated [header] [eval_id] [interface_id]] [--output_precision <int>]\n"
    << "    dakota_restart_util remove <double> <old_restart_file> <new_restart_file>\n"
    << "    dakota_restart_util remove_ids <int_1> ... <int_n> <old_restart_file> <new_restart_file>\n"
    << "    dakota_restart_util cat <restart_file_1> ... <restart_file_n> <new_restart_file>" 
    << endl;
}


/** \b Usage: "dakota_restart_util print dakota.rst"\n
              "dakota_restart_util to_neutral dakota.rst dakota.neu"

    Prints all evals. in full precision to either stdout or a neutral
    file.  The former is useful for ensuring that duplicate detection
    is successful in a restarted run (e.g., starting a new method
    from the previous best), and the latter is used for translating
    binary files between platforms. */
void print_restart(StringArray pos_args, String print_dest)
{
  if (print_dest != "stdout" && print_dest != "neutral_file") {
    Cerr << "Error: bad print_dest in print_restart" << endl;
    exit(-1);
  }
  else if (print_dest == "stdout" && pos_args.size() != 1) {
    Cerr << "Usage: dakota_restart_util print <restart_file>." << endl;
    exit(-1);
  }
  else if (print_dest == "neutral_file" && pos_args.size() != 2) {
    Cerr << "Usage: dakota_restart_util to_neutral <restart_file> "
	 << "<neutral_file>." << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(pos_args[0].c_str(), std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << pos_args[0] << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  std::ofstream neutral_file_stream;
  if (print_dest == "neutral_file") {
    cout << "Writing neutral file " << pos_args[1] << '\n';
    neutral_file_stream.open(pos_args[1].c_str());
  }

  // override default to output data in full precision (double = 16 digits)
  write_precision = 16;

  int cntr = 0;
  restart_input_fs.peek();  // peek to force EOF if no records in restart file
  while (restart_input_fs.good() && !restart_input_fs.eof()) {

    ParamResponsePair current_pair;
    try { 
      restart_input_archive & current_pair; 
    }
    catch(const boost::archive::archive_exception& e) {
      Cerr << "\nError reading restart file (boost::archive exception):\n" 
	   << e.what() << std::endl;
      abort_handler(-1);
    }
    // serialization functions no longer throw strings

    cntr++;
    if (print_dest == "stdout")
      cout << "------------------------------------------\nRestart record "
	   << setw(4) << cntr << "  (evaluation id " << setw(4)
	   << current_pair.eval_id()
	   << "):\n------------------------------------------\n"
	   << current_pair;
    else if (print_dest == "neutral_file")
      current_pair.write_annotated(neutral_file_stream);

    // peek to force EOF if the last restart record was read
    restart_input_fs.peek();
  }
  if (print_dest == "neutral_file")
    neutral_file_stream.close();
  cout << "Restart file processing completed: " << cntr
       << " evaluations retrieved.\n";
}


/** \b Usage: "dakota_restart_util to_pdb dakota.rst dakota.pdb"

    Unrolls all data associated with a particular tag for all
    evaluations and then writes this data in a tabular format
    (e.g., to a PDB database or MATLAB/TECPLOT data file). */
void print_restart_pdb(StringArray pos_args, String print_dest)
{
  if (pos_args.size() != 2) {
    Cerr << "Usage: dakota_restart_util to_pdb <restart_file> <pdb_file>."
         << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(pos_args[0].c_str(), std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << pos_args[0] << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  size_t i, j, num_evals = 0;
  PRPCache read_pairs;
  restart_input_fs.peek();  // peek to force EOF if no records in restart file
  while (restart_input_fs.good() && !restart_input_fs.eof()) {

    ParamResponsePair current_pair;
    try { 
      restart_input_archive & current_pair; 
    }
    catch(const boost::archive::archive_exception& e) {
      Cerr << "\nError reading restart file (boost::archive exception):\n" 
	   << e.what() << std::endl;
      abort_handler(-1);
    }
    // serialization functions no longer throw strings

    read_pairs.insert(current_pair);
    ++num_evals;

    // peek to force EOF if the last restart record was read
    restart_input_fs.peek();
  }

  PRPCacheCIter prp_iter = read_pairs.begin();
  StringMultiArrayConstView cv_labels
    = prp_iter->variables().continuous_variable_labels();
  StringMultiArrayConstView div_labels
    = prp_iter->variables().discrete_int_variable_labels();
  StringMultiArrayConstView drv_labels
    = prp_iter->variables().discrete_real_variable_labels();
  const StringArray& fn_labels
    = prp_iter->response().function_labels();
  size_t num_cv = cv_labels.size(), num_div = div_labels.size(),
    num_drv = drv_labels.size(), num_fns = fn_labels.size();

  // pack up tabular data matrices (organized such that use of a single index 
  // returns the complete history for an attribute).
  RealVector      uninitialized_rv(num_evals, false);
  IntVector       uninitialized_iv(num_evals, false);
  RealVectorArray tabular_rdata(num_cv+num_drv+num_fns, uninitialized_rv);
  std::vector<IntVector> tabular_idata(num_div, uninitialized_iv);

  for (i=0; i<num_evals; ++i, ++prp_iter) {
    // Extract variables related data
    const Variables&  local_vars    = prp_iter->variables();
    const RealVector& local_c_vars  = local_vars.continuous_variables();
    const IntVector&  local_di_vars = local_vars.discrete_int_variables();
    const RealVector& local_dr_vars = local_vars.discrete_real_variables();
    for (j=0; j<num_cv; ++j)
      tabular_rdata[j][i] = local_c_vars[j];
    for (j=0; j<num_div; ++j)
      tabular_idata[j][i] = local_di_vars[j];
    for (j=0; j<num_drv; ++j)
      tabular_rdata[j+num_cv][i] = local_dr_vars[j];

    // Extract response related data
    const Response&   local_response = prp_iter->response();
    const RealVector& local_fns      = local_response.function_values();
    //const ShortArray& local_asv =local_response.active_set_request_vector();
    //const RealMatrix& local_grads = local_response.function_gradients();
    //const RealMatrixArray& local_hessians
    //  = local_response.function_hessians();
    // NOTE: if any fns are inactive, they will appear as 0's in tabular_data
    for (j=0; j<num_fns; ++j)
      tabular_rdata[j+num_cv+num_drv][i] = local_fns[j];
  }

#ifdef HAVE_PDB_H
  // open the PDB file
  cout << "Writing PDB file:" << pos_args[1] << '\n';
  PDBfile *fileID;
  if ((fileID = PD_open(pos_args[1].c_str(), "w")) == NULL){
    cout << "Problem opening PDB file";
    return;
  }

  // Note: tabular_rdata[j] returns the jth row vector from the matrix and
  //       values() returns the double* pointer to the data of this row
  //       vector.  const char* and const double* are passed to PDB_Write.

  char *tag, cdim[6];
  std::sprintf(cdim, "(%d)", num_evals);

  String tag_name, snum_evals(cdim);

  // write continuous variables
  for (j=0; j<num_cv; ++j){
    tag_name = cv_labels[j] + snum_evals;
    tag = const_cast<char *>(tag_name.data());
    if(!PD_write(fileID, tag, "double", (void *)tabular_rdata[j].values()))
      cout << "PD_write error=" << PD_err << '\n';
  }

  // write discrete integer variables
  for (j=0; j<num_div; ++j){
    tag_name = div_labels[j] + snum_evals;
    tag = const_cast<char *>(tag_name.data());
    if(!PD_write(fileID, tag, "integer", (void *)tabular_idata[j].values()))
      cout << "PD_write error=" << PD_err << '\n';
  }

  // write discrete real variables
  for (j=0; j<num_drv; ++j){
    tag_name = drv_labels[j] + snum_evals;
    tag = const_cast<char *>(tag_name.data());
    if(!PD_write(fileID, tag, "double",
		 (void *)tabular_rdata[j+num_cv].values()))
      cout << "PD_write error=" << PD_err << '\n';
  }

  // write corresponding function values
  for (j=0; j<num_fns; ++j){
    tag_name = fn_labels[j] + snum_evals;
    tag = const_cast<char *>(tag_name.data());
    if(!PD_write(fileID, tag, "double",
		 (void *)tabular_rdata[j+num_cv+num_drv].values()))
      cout << "PD_write error=" << PD_err << '\n';
  }
    
  if(!PD_close(fileID))
    cout << "Problem closing PDB file" << pos_args[1] << '\n';
#else
  cout << "PDB utilities not available" << endl;
  exit(0);
#endif

  cout << "Restart file processing completed: " << num_evals
       << " evaluations tabulated.\n";
}


/** \b Usage: "dakota_restart_util to_tabular dakota.rst dakota.txt"

    Unrolls all data associated with a particular tag for all
    evaluations and then writes this data in a tabular format
    (e.g., to a PDB database or MATLAB/TECPLOT data file). */
void print_restart_tabular(StringArray pos_args, String print_dest,
			   unsigned short tabular_format, int tabular_precision)
{
  if (pos_args.size() != 2) {
    Cerr << "Usage: dakota_restart_util to_tabular <restart_file> "
	 << "<text_file>." << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(pos_args[0].c_str(), std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << pos_args[0] << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  size_t num_evals = 0;
  cout << "Writing tabular text file " << pos_args[1] << '\n';
  std::ofstream tabular_text(pos_args[1].c_str());
  // to track changes in interface and/or labels
  String curr_interf;
  StringMultiArray curr_acv_labels;
  StringMultiArray curr_adiv_labels;
  StringMultiArray curr_adsv_labels;
  StringMultiArray curr_adrv_labels;
  StringArray curr_resp_labels;

  // Note: tabular defaults to write_precision from global defs
  // Note: setprecision(write_precision) and std::ios::floatfield are embedded
  //       in write_data_tabular() functions.

  int wp_save = write_precision;  // later restore since this is global data
  write_precision = tabular_precision;

  restart_input_fs.peek();  // peek to force EOF if no records in restart file
  while (restart_input_fs.good() && !restart_input_fs.eof()) {

    ParamResponsePair current_pair;
    try { 
      restart_input_archive & current_pair; 
    }
    catch(const boost::archive::archive_exception& e) {
      Cerr << "\nError reading restart file (boost::archive exception):\n" 
	   << e.what() << std::endl;
      abort_handler(-1);
    }
    // serialization functions no longer throw strings

    // The number of variables or responses may differ across
    // different interfaces.  Output the header when needed due to
    // label or length changes.
    const String& new_interf = current_pair.interface_id();
    if (num_evals == 0  || new_interf != curr_interf) {
      curr_interf = new_interf;
      const Variables& curr_vars = current_pair.variables();
      if (curr_vars.all_continuous_variable_labels() != curr_acv_labels ||
	  curr_vars.all_discrete_int_variable_labels() != curr_adiv_labels ||
	  curr_vars.all_discrete_string_variable_labels() != curr_adsv_labels ||
	  curr_vars.all_discrete_real_variable_labels() != curr_adrv_labels ||
	  current_pair.response().function_labels() != curr_resp_labels) {
	// update the current copy of the labels, sizing first
	curr_acv_labels.resize(boost::extents[curr_vars.acv()]);
	curr_acv_labels = curr_vars.all_continuous_variable_labels();
	curr_adiv_labels.resize(boost::extents[curr_vars.adiv()]);
	curr_adiv_labels = curr_vars.all_discrete_int_variable_labels();
	curr_adsv_labels.resize(boost::extents[curr_vars.adsv()]);
	curr_adsv_labels = curr_vars.all_discrete_string_variable_labels();
	curr_adrv_labels.resize(boost::extents[curr_vars.adrv()]);
	curr_adrv_labels = curr_vars.all_discrete_real_variable_labels();
	curr_resp_labels = current_pair.response().function_labels();
	// write the new header
	current_pair.write_tabular_labels(tabular_text, tabular_format);
      }
    }
    current_pair.write_tabular(tabular_text, tabular_format);  // also writes IDs
    ++num_evals;

    // peek to force EOF if the last restart record was read
    restart_input_fs.peek();
  }

  cout << "Restart file processing completed: " << num_evals
       << " evaluations tabulated.\n";

  write_precision = wp_save;  // restore since this is global data
}


/** \b Usage: "dakota_restart_util from_neutral dakota.neu dakota.rst"

    Reads evaluations from a neutral file.  This is used for translating
    binary files between platforms. */
void read_neutral(StringArray pos_args)
{
  if (pos_args.size() != 2) {
    Cerr << "Usage: dakota_restart_util from_neutral <neutral_file> "
	 << "<restart_file>." << endl;
    exit(-1);
  }

  std::ifstream neutral_file_stream(pos_args[0].c_str());
  if (neutral_file_stream)
    cout << "Reading neutral file " << pos_args[0] << '\n';
  else {
    Cerr << "Error: failed to open neutral file " << pos_args[0] << endl;
    exit(-1);
  }
  
  std::ofstream restart_output_fs(pos_args[1].c_str(), std::ios::binary);
  boost::archive::binary_oarchive restart_output_archive(restart_output_fs);
  cout << "Writing new restart file " << pos_args[1] << '\n';

  int cntr = 0;
  neutral_file_stream >> std::ws;
  while (neutral_file_stream.good() && !neutral_file_stream.eof()) {
    ParamResponsePair current_pair;
    try { 
      current_pair.read_annotated(neutral_file_stream); 
    }
    // shouldn't need to allow failed partial reads throwing string anymore
    catch(const FileReadException& fr_except) {
      Cerr << "\nError reading neutral file:\n  " << fr_except.what() 
	   << std::endl;
      abort_handler(-1);
    }
    restart_output_archive & current_pair;
    cntr++;
    neutral_file_stream >> std::ws;
  }
  cout << "Neutral file processing completed: " << cntr
       << " evaluations retrieved.\n";
  restart_output_fs.close();
}


/** \b Usage: "dakota_restart_util remove 0.0 dakota_old.rst dakota_new.rst"\n
              "dakota_restart_util remove_ids 2 7 13 dakota_old.rst
                 dakota_new.rst"

    Repairs a restart file by removing corrupted evaluations.  The
    identifier for evaluation removal can be either a double precision
    number (all evaluations having a matching response function value
    are removed) or a list of integers (all evaluations with matching
    evaluation ids are removed). */
void repair_restart(StringArray pos_args, String identifier_type)
{
  double  remove_val;
  bool    by_value;
  IntList bad_ids;
  String  read_restart_filename, write_restart_filename;
  if (identifier_type == "by_value") {
    if (pos_args.size() != 3) {
      Cerr << "Usage: dakota_restart_util remove <double> <old_restart_file> "
           << "<new_restart_file>." << endl;
      exit(-1);
    }
    by_value = true;
    try {
      remove_val = boost::lexical_cast<double>(pos_args[0]);
    }
    catch (const boost::bad_lexical_cast& blc_except) {
      Cerr << "\nError invalid floating point response " << pos_args[0] 
	   << " to remove." << std::endl;
      exit(-1);
    }
    read_restart_filename  = pos_args[1];
    write_restart_filename = pos_args[2];
  }
  else if (identifier_type == "by_id") {
    if (pos_args.size() < 3) {
      Cerr << "Usage: dakota_restart_util remove_ids <int_1> ... <int_n> "
           << "<old_restart_file> <new_restart_file>." << endl;
      exit(-1);
    }
    by_value = false;
    write_restart_filename = pos_args.back(); pos_args.pop_back();
    read_restart_filename  = pos_args.back(); pos_args.pop_back();
    try {
      BOOST_FOREACH(const String& pa, pos_args) {
	bad_ids.push_back(boost::lexical_cast<int>(pa));
      }
    }
    catch (const boost::bad_lexical_cast& blc_except) {
      Cerr << "\nError: invalid integer IDs " << pos_args 
	   << " for command remove_ids" << std::endl;
      exit(-1);
    }
  }
  else {
    Cerr << "Error: bad identifier_type in repair_restart." << endl;
    exit(-1);
  }

  if (read_restart_filename == write_restart_filename) {
    Cerr << "Error: old and new restart filenames must differ." << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(read_restart_filename.c_str(), 
				 std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file "
         << read_restart_filename << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  std::ofstream restart_output_fs(write_restart_filename.c_str(), 
				  std::ios::binary);
  boost::archive::binary_oarchive restart_output_archive(restart_output_fs);

  cout << "Writing new restart file " << write_restart_filename << '\n';

  int cntr = 0, good_cntr = 0;
  restart_input_fs.peek();  // peek to force EOF if no records in restart file
  while (restart_input_fs.good() && !restart_input_fs.eof()) {

    ParamResponsePair current_pair;
    try { 
      restart_input_archive & current_pair; 
    }
    catch(const boost::archive::archive_exception& e) {
      Cerr << "\nError reading restart file (boost::archive exception):\n" 
	   << e.what() << std::endl;
      abort_handler(-1);
    }
    // serialization functions no longer throw strings

    cntr++;

    // detect if current_pair is to be removed
    bool bad_flag = false;
    if (by_value) {
      const Response& resp      = current_pair.response();
      const RealVector& fn_vals = resp.function_values();
      const ShortArray& asv     = resp.active_set_request_vector();
      for (size_t j=0; j<fn_vals.length(); ++j) {
        if ((asv[j] & 1) && fn_vals[j] == remove_val) {
          bad_flag = true;
          break;
        }
      }
    }
    else if (contains(bad_ids, current_pair.eval_id()))
      bad_flag = true;

    // if current_pair is bad, omit it from the new restart file
    if (!bad_flag) {
      restart_output_archive & current_pair;
      good_cntr++;
    }

    // peek to force EOF if the last restart record was read
    restart_input_fs.peek();
  }
  cout << "Restart repair completed: " << cntr << " evaluations retrieved"
       << ", " << cntr-good_cntr << " removed, " << good_cntr << " saved.\n";
  restart_output_fs.close();
}


/** \b Usage: "dakota_restart_util cat dakota_1.rst ... dakota_n.rst
                 dakota_new.rst"

    Combines multiple restart files into a single restart database. */
void concatenate_restart(StringArray pos_args)
{
  if (pos_args.size() < 3) {
    Cerr << "Usage: dakota_restart_util cat <restart_file_1> ... "
	 << "<restart_file_n> <new_restart_file>." << endl;
    exit(-1);
  }

  String write_restart_filename = pos_args.back(); pos_args.pop_back();
  std::ofstream restart_output_fs(write_restart_filename.c_str(),
				  std::ios::binary);
  boost::archive::binary_oarchive restart_output_archive(restart_output_fs);

  cout << "Writing new restart file " << write_restart_filename << '\n';

  BOOST_FOREACH(const String& rst_file, pos_args) {

    std::ifstream restart_input_fs(rst_file.c_str(), std::ios::binary);
    if (!restart_input_fs.good()) {
      Cerr << "Error: failed to open restart file " << rst_file << endl;
      exit(-1);
    }
    boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

    int cntr = 0;
    restart_input_fs.peek();  // peek to force EOF if no records in restart file
    while (restart_input_fs.good() && !restart_input_fs.eof()) {

      ParamResponsePair current_pair;
      try { 
	restart_input_archive & current_pair; 
      }
      catch(const boost::archive::archive_exception& e) {
	Cerr << "\nError reading restart file (boost::archive exception):\n" 
	     << e.what() << std::endl;
	abort_handler(-1);
      }
      // serialization functions no longer throw strings
      restart_output_archive & current_pair;
      cntr++;
 
      // peek to force EOF if the last restart record was read
      restart_input_fs.peek();
    }

    cout << rst_file << " processing completed: " << cntr
         << " evaluations retrieved.\n";
  }
  restart_output_fs.close();
}

} // namespace Dakota
