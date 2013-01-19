/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: A utility for printing, repairing, and merging restart files.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: restart_util.cpp 6818 2010-06-05 00:46:38Z dmgay $

/** \file restart_util.cpp
    \brief file containing the DAKOTA restart utility main program */

#include "system_defs.hpp"
#include "data_types.hpp"
#include "DakotaBinStream.hpp"
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
/// print a restart file (tabular format)
void print_restart_tabular(int argc, char** argv, String print_dest);
/// read a restart file (neutral file format)
void read_neutral(int argc, char** argv);
/// repair a restart file by removing corrupted evaluations
void repair_restart(int argc, char** argv, String identifier_type);
/// concatenate multiple restart files
void concatenate_restart(int argc, char** argv);

} // namespace Dakota

extern "C" int nidr_save_exedir(const char*, int);

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

  nidr_save_exedir(argv[0], 3);	// 3 ==> add both the directory containing this binary
				// and . to the end of $PATH if not already on $PATH.
  String util_command(argv[1]);

  if (util_command == "print")
    print_restart(argc, argv, "stdout");
  else if (util_command == "to_neutral")
    print_restart(argc, argv, "neutral_file");
  else if (util_command == "from_neutral")
    read_neutral(argc, argv);
  else if (util_command == "to_pdb")
    print_restart_tabular(argc, argv, "pdb_file");
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

  BiStream read_restart(argv[2]);
  if (!read_restart) {
    Cerr << "Error: failed to open restart file " << argv[2] << endl;
    exit(-1);
  }

  std::ofstream neutral_file_stream;
  if (print_dest == "neutral_file") {
    cout << "Writing neutral file " << argv[3] << '\n';
    neutral_file_stream.open(argv[3]);
  }

  // override default to output data in full precision (double = 16 digits)
  write_precision = 16;

  int cntr = 0;
  while (!read_restart.eof()) {

    ParamResponsePair current_pair;
    try { read_restart >> current_pair; }

    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << endl;
      break; // out of while loop
    }

    cntr++;
    if (print_dest == "stdout")
      cout << "------------------------------------------\nRestart record "
	   << setw(4) << cntr << "  (evaluation id " << setw(4)
	   << current_pair.eval_id()
	   << "):\n------------------------------------------\n"
	   << current_pair;
    else if (print_dest == "neutral_file")
      current_pair.write_annotated(neutral_file_stream);
  }
  if (print_dest == "neutral_file")
    neutral_file_stream.close();
  cout << "Restart file processing completed: " << cntr
       << " evaluations retrieved.\n";
}


/** \b Usage: "dakota_restart_util to_pdb dakota.rst dakota.pdb"\n
              "dakota_restart_util to_tabular dakota.rst dakota.txt"

    Unrolls all data associated with a particular tag for all
    evaluations and then writes this data in a tabular format
    (e.g., to a PDB database or MATLAB/TECPLOT data file). */
void print_restart_tabular(int argc, char** argv, String print_dest)
{
  if (print_dest != "pdb_file" && print_dest != "text_file") {
    Cerr << "Error: bad print_dest in print_restart_tabular" << endl;
    exit(-1);
  }
  else if (print_dest == "pdb_file" && argc != 4) {
    Cerr << "Usage: \"dakota_restart_util to_pdb <restart_file> <pdb_file>\"."
         << endl;
    exit(-1);
  }
  else if (print_dest == "text_file" && argc != 4) {
    Cerr << "Usage: \"dakota_restart_util to_tabular <restart_file> "
	 << "<text_file>\"." << endl;
    exit(-1);
  }

  BiStream read_restart(argv[2]);
  if (!read_restart) {
    Cerr << "Error: failed to open restart file " << argv[2] << endl;
    exit(-1);
  }

  extern PRPCache data_pairs;
  size_t num_evals = 0;
  while (!read_restart.eof()) {

    ParamResponsePair current_pair;
    try { read_restart >> current_pair; }

    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << endl;
      break; // out of while loop
    }

    data_pairs.insert(current_pair);

    num_evals++; // more efficient than evaluating data_pairs.entries()
  }

  size_t i, j;
  if (print_dest == "pdb_file") {
    PRPCacheCIter prp_iter = data_pairs.begin();
    StringMultiArrayConstView cv_labels
      = prp_iter->prp_parameters().continuous_variable_labels();
    StringMultiArrayConstView div_labels
      = prp_iter->prp_parameters().discrete_int_variable_labels();
    StringMultiArrayConstView drv_labels
      = prp_iter->prp_parameters().discrete_real_variable_labels();
    const StringArray& fn_labels
      = prp_iter->prp_response().function_labels();
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
      const Variables&  local_vars    = prp_iter->prp_parameters();
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
      const Response&   local_response = prp_iter->prp_response();
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
  }
  else if (print_dest == "text_file") {
    cout << "Writing tabular text file " << argv[3] << '\n';
    std::ofstream tabular_text(argv[3]);

    String curr_interf;
    PRPCacheCIter prp_iter = data_pairs.begin();

    for (i=0; i<num_evals; ++i, ++prp_iter) {
      const String& new_interf = prp_iter->interface_id();
      if (i == 0 || new_interf != curr_interf) {
        curr_interf = new_interf;
        // Header (note: use matlab comment syntax "%"):
        StringMultiArrayConstView acv_labels
	  = prp_iter->prp_parameters().all_continuous_variable_labels();
        StringMultiArrayConstView adiv_labels
	  = prp_iter->prp_parameters().all_discrete_int_variable_labels();
        StringMultiArrayConstView adrv_labels
	  = prp_iter->prp_parameters().all_discrete_real_variable_labels();
        const StringArray& fn_labels
	  = prp_iter->prp_response().function_labels();
        size_t num_acv = acv_labels.size(), num_adiv = adiv_labels.size(),
	  num_adrv = adrv_labels.size(), num_fns = fn_labels.size();
	if (!curr_interf.empty())
	  tabular_text << "%Interface = " << curr_interf << '\n';
        tabular_text << "%eval_id ";
        for (j=0; j<num_acv; ++j)
          tabular_text << setw(14) << acv_labels[j].data() << ' ';
        for (j=0; j<num_adiv; ++j)
          tabular_text << setw(14) << adiv_labels[j].data() << ' ';
        for (j=0; j<num_adrv; ++j)
          tabular_text << setw(14) << adrv_labels[j].data() << ' ';
        for (j=0; j<num_fns; ++j)
          tabular_text << setw(14) << fn_labels[j].data() << ' ';
        tabular_text << '\n';
      }
      // Data: (note: not maximum precision)
      prp_iter->write_tabular(tabular_text);
    }
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

  extern BoStream write_restart;
  write_restart.open(argv[3]);
  cout << "Writing new restart file " << argv[3] << '\n';

  int cntr = 0;
  while (!neutral_file_stream.eof()) {
    ParamResponsePair current_pair;
    try { current_pair.read_annotated(neutral_file_stream); }
    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << endl;
      break; // out of while loop
    }
    write_restart << current_pair;
    cntr++;
  }
  cout << "Neutral file processing completed: " << cntr
       << " evaluations retrieved.\n";
  write_restart.close();
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

  BiStream read_restart(read_restart_filename);
  if (!read_restart) {
    Cerr << "Error: failed to open restart file "
         << read_restart_filename << endl;
    exit(-1);
  }

  extern BoStream write_restart;
  write_restart.open(write_restart_filename);

  cout << "Writing new restart file " << write_restart_filename << '\n';

  int cntr = 0, good_cntr = 0;
  while (!read_restart.eof()) {

    ParamResponsePair current_pair;
    try { read_restart >> current_pair; }

    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << endl;
      break; // out of while loop
    }

    cntr++;

    // detect if current_pair is to be removed
    bool bad_flag = false;
    if (by_value) {
      const Response& resp      = current_pair.prp_response();
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
      write_restart << current_pair;
      good_cntr++;
    }
  }
  cout << "Restart repair completed: " << cntr << " evaluations retrieved"
       << ", " << cntr-good_cntr << " removed, " << good_cntr << " saved.\n";
  write_restart.close();
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

  extern BoStream write_restart;
  write_restart.open(argv[argc-1]);
  cout << "Writing new restart file " << argv[argc-1] << '\n';

  for (int cat_cntr=2; cat_cntr<argc-1; cat_cntr++) {

    BiStream read_restart(argv[cat_cntr]);
    if (!read_restart) {
      Cerr << "Error: failed to open restart file " << argv[cat_cntr] << endl;
      exit(-1);
    }

    int cntr = 0;
    while (!read_restart.eof()) {

      ParamResponsePair current_pair;
      try { read_restart >> current_pair; }

      catch(String& err_msg) {
        //Cerr << "Warning: " << err_msg << endl;
        break; // out of while loop
      }
      write_restart << current_pair;
      cntr++;
    }

    cout << argv[cat_cntr] << " processing completed: " << cntr
         << " evaluations retrieved.\n";
  }
  write_restart.close();
}

} // namespace Dakota
