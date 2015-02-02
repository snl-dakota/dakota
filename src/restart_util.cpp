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

/// print a restart file
void print_restart(int argc, char** argv, String print_dest);
/// print a restart file (PDB format)
void print_restart_pdb(int argc, char** argv, String print_dest);
/// print a restart file (tabular format)
void print_restart_tabular(int argc, char** argv, String print_dest);
/// read a restart file (neutral file format)
void read_neutral(int argc, char** argv);
/// repair a restart file by removing corrupted evaluations
void repair_restart(int argc, char** argv, String identifier_type);
/// concatenate multiple restart files
void concatenate_restart(int argc, char** argv);

} // namespace Dakota


/// The main program for the DAKOTA restart utility.

/** Parse command line inputs and invoke the appropriate utility
    function (print_restart(), print_restart_tabular(),
    read_neutral(), repair_restart(), or concatenate_restart()). */

int main(int argc, char* argv[])
{
  if (argc<3) {
    Cerr << "Usage: \"dakota_restart_util print <restart_file>\"\n"
         << "       \"dakota_restart_util to_neutral <restart_file> "
	 << "<neutral_file>\"\n"
         << "       \"dakota_restart_util from_neutral <neutral_file> "
	 << "<restart_file>\"\n"
         << "       \"dakota_restart_util to_pdb <restart_file> "
	 << "<pdb_file>\"\n"
         << "       \"dakota_restart_util to_tabular <restart_file> "
	 << "<text_file>\"\n"
         << "       \"dakota_restart_util remove <double> <old_restart_file> "
	 << "<new_restart_file>\"\n"
         << "       \"dakota_restart_util remove_ids <int_1> ... <int_n> "
         << "<old_restart_file> <new_restart_file>\"\n"
         << "       \"dakota_restart_util cat <restart_file_1> ... "
	 << "<restart_file_n> <new_restart_file>\"" << endl;
    exit(-1);
  }

  String util_command(argv[1]);

  if (util_command == "print")
    print_restart(argc, argv, "stdout");
  else if (util_command == "to_neutral")
    print_restart(argc, argv, "neutral_file");
  else if (util_command == "from_neutral")
    read_neutral(argc, argv);
  else if (util_command == "to_pdb")
    print_restart_pdb(argc, argv, "pdb_file");
  else if (util_command == "to_tabular")
    print_restart_tabular(argc, argv, "text_file");
  else if (util_command == "remove")
    repair_restart(argc, argv, "by_value");
  else if (util_command == "remove_ids")
    repair_restart(argc, argv, "by_id");
  else if (util_command == "cat")
    concatenate_restart(argc, argv);
  else {
    Cerr << "Error: " << util_command << " not supported." << endl;
    exit(-1);
  }

  return 0;
}


namespace Dakota {

/** \b Usage: "dakota_restart_util print dakota.rst"\n
              "dakota_restart_util to_neutral dakota.rst dakota.neu"

    Prints all evals. in full precision to either stdout or a neutral
    file.  The former is useful for ensuring that duplicate detection
    is successful in a restarted run (e.g., starting a new method
    from the previous best), and the latter is used for translating
    binary files between platforms. */
void print_restart(int argc, char** argv, String print_dest)
{
  if (print_dest != "stdout" && print_dest != "neutral_file") {
    Cerr << "Error: bad print_dest in print_restart" << endl;
    exit(-1);
  }
  else if (print_dest == "stdout" && argc != 3) {
    Cerr << "Usage: \"dakota_restart_util print <restart_file>\"." << endl;
    exit(-1);
  }
  else if (print_dest == "neutral_file" && argc != 4) {
    Cerr << "Usage: \"dakota_restart_util to_neutral <restart_file> "
	 << "<neutral_file>\"." << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(argv[2], std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << argv[2] << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  std::ofstream neutral_file_stream;
  if (print_dest == "neutral_file") {
    cout << "Writing neutral file " << argv[3] << '\n';
    neutral_file_stream.open(argv[3]);
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
void print_restart_pdb(int argc, char** argv, String print_dest)
{
  if (argc != 4) {
    Cerr << "Usage: \"dakota_restart_util to_pdb <restart_file> <pdb_file>\"."
         << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(argv[2], std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << argv[2] << endl;
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
  cout << "Writing PDB file:" << argv[3] << '\n';
  PDBfile *fileID;
  if ((fileID = PD_open(argv[3], "w")) == NULL){
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
    cout << "Problem closing PDB file" << argv[3] << '\n';
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
void print_restart_tabular(int argc, char** argv, String print_dest)
{
  if (argc != 4) {
    Cerr << "Usage: \"dakota_restart_util to_tabular <restart_file> "
	 << "<text_file>\"." << endl;
    exit(-1);
  }

  std::ifstream restart_input_fs(argv[2], std::ios::binary);
  if (!restart_input_fs.good()) {
    Cerr << "Error: failed to open restart file " << argv[2] << endl;
    exit(-1);
  }
  boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

  size_t num_evals = 0;
  cout << "Writing tabular text file " << argv[3] << '\n';
  std::ofstream tabular_text(argv[3]);
  // to track changes in interface and/or labels
  String curr_interf;
  StringMultiArray curr_acv_labels;
  StringMultiArray curr_adiv_labels;
  StringMultiArray curr_adsv_labels;
  StringMultiArray curr_adrv_labels;
  StringArray curr_resp_labels;

  // Note: tabular not written in maximum precision; need command-line option).
  // Override default to output data in full precision (double = 16 digits).
  // Note: setprecision(write_precision) and std::ios::floatfield are embedded
  // in write_data_tabular() functions.

  //write_precision = 16;  // extern defined in dakota_global_defs.cpp

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
	current_pair.write_tabular_labels(tabular_text);
      }
    }
    current_pair.write_tabular(tabular_text);  // also writes IDs
    ++num_evals;

    // peek to force EOF if the last restart record was read
    restart_input_fs.peek();
  }

  cout << "Restart file processing completed: " << num_evals
       << " evaluations tabulated.\n";
}


/** \b Usage: "dakota_restart_util from_neutral dakota.neu dakota.rst"

    Reads evaluations from a neutral file.  This is used for translating
    binary files between platforms. */
void read_neutral(int argc, char** argv)
{
  if (argc != 4) {
    Cerr << "Usage: \"dakota_restart_util from_neutral <neutral_file> "
	 << "<restart_file>\"." << endl;
    exit(-1);
  }

  std::ifstream neutral_file_stream(argv[2]);
  if (neutral_file_stream)
    cout << "Reading neutral file " << argv[2] << '\n';
  else {
    Cerr << "Error: failed to open neutral file " << argv[2] << endl;
    exit(-1);
  }
  
  std::ofstream restart_output_fs(argv[3], std::ios::binary);
  boost::archive::binary_oarchive restart_output_archive(restart_output_fs);
  cout << "Writing new restart file " << argv[3] << '\n';

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
void repair_restart(int argc, char** argv, String identifier_type)
{
  double  remove_val;
  bool    by_value;
  IntList bad_ids;
  String  read_restart_filename, write_restart_filename;
  if (identifier_type == "by_value") {
    if (argc != 5) {
      Cerr << "Usage: \"dakota_restart_util remove <double> <old_restart_file> "
           << "<new_restart_file>\"." << endl;
      exit(-1);
    }
    by_value = true;
    remove_val = atof(argv[2]);
    read_restart_filename  = argv[3];
    write_restart_filename = argv[4];
  }
  else if (identifier_type == "by_id") {
    if (argc < 5) {
      Cerr << "Usage: \"dakota_restart_util remove_ids <int_1> ... <int_n> "
           << "<old_restart_file> <new_restart_file>\"." << endl;
      exit(-1);
    }
    by_value = false;
    for (int i=2; i<argc-2; ++i)
      bad_ids.push_back(atoi(argv[i]));
    read_restart_filename  = argv[argc-2];
    write_restart_filename = argv[argc-1];
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
void concatenate_restart(int argc, char** argv)
{
  if (argc < 5) {
    Cerr << "Usage: \"dakota_restart_util cat <restart_file_1> ... "
	 << "<restart_file_n> <new_restart_file>\"." << endl;
    exit(-1);
  }

  std::ofstream restart_output_fs(argv[argc-1], std::ios::binary);
  boost::archive::binary_oarchive restart_output_archive(restart_output_fs);

  cout << "Writing new restart file " << argv[argc-1] << '\n';

  for (int cat_cntr=2; cat_cntr<argc-1; cat_cntr++) {

    std::ifstream restart_input_fs(argv[cat_cntr], std::ios::binary);
    if (!restart_input_fs.good()) {
      Cerr << "Error: failed to open restart file " << argv[cat_cntr] << endl;
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

    cout << argv[cat_cntr] << " processing completed: " << cntr
         << " evaluations retrieved.\n";
  }
  restart_output_fs.close();
}

} // namespace Dakota
