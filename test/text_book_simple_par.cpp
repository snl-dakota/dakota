/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Simpler version of text_book_par for use in DAKOTA/Application parallelism
// dump node run info
// allow n variables and just have proc 0 write fnVals (f, c1, c2)
//
// To build executable:
// gmake "CXX=mpicxx" "CXXFLAGS+= -DTB_USE_MPI" text_book_simple_par

#ifdef TB_USE_MPI
#include <mpi.h>
#endif // TB_USE_MPI

#include <cassert>
#include <cmath>
#include <ctime>
#include <iostream>
#include <fstream>
#include <limits.h>  // for _POSIX_HOST_NAME_MAX
#include <unistd.h>
#include <vector>
#include <string>


double feval(const std::vector<double>& x, size_t rank, size_t nprocs);
void   write_data(const std::string& filename, const std::vector<double>& fnvals);


/// Simplest possible, PARALLEL "text_book function" simulator
int main(int argc, char* argv[])
{
  int rank = 0, nprocs = 1;
  char proc_name[_POSIX_HOST_NAME_MAX + 1];

#ifdef TB_USE_MPI
  MPI::Init(argc, argv);
  rank = MPI::COMM_WORLD.Get_rank();
  nprocs = MPI::COMM_WORLD.Get_size();

  int proc_name_len = 0;
  MPI::Get_processor_name(proc_name, proc_name_len);
  assert(MPI_MAX_PROCESSOR_NAME == _POSIX_HOST_NAME_MAX + 1);
  assert(proc_name_len <= MPI_MAX_PROCESSOR_NAME);
#else
  gethostname(proc_name, _POSIX_HOST_NAME_MAX);
#endif // TB_USE_MPI

  time_t rawtime;
  time(&rawtime);
  struct tm* timeinfo = localtime ( &rawtime );
  std::cout << argv[0] << ": task " << rank+1 << " of " << nprocs << " starting on "
       << proc_name << " at " << asctime(timeinfo);
  /* cout << "with " << argc-1 << " arg(s):\n";
  // Allow testing without DAKOTA parameters/results files (for now)
  if (argc == 2 || argc > 3) {
    cerr << "Error: text_book_simple_par requires 0 or 2 arguments." << endl;
    status = 1;
  }
  else if (rank == 0)
    ;//cout << "INFO: Running the SIMPLE text_book parallel simulator." << endl;
  */


  std::ifstream fin(argv[1]);  // WJB - ToDo: add FILE OPEN OK assertion
  size_t num_vars = 0;
  std::string vars_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars);

  for (size_t i=0; i<num_vars; ++i) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Verify input is consistent with problem definition - ONLY 3 functions!
  size_t num_fns = 0;
  fin >> num_fns;
  assert(num_fns == 3 && "SIMPLE text_book: Number of funcs must be exactly 3");

  // Use a std::vector to store the response (1 objective, 2 constraints)
  std::vector<double> fnvals(num_fns); // f, c1, c2

  // **** c2:
  fnvals[2] = x[1]*x[1] - 0.5*x[0];

  // **** c1:
  fnvals[1] = x[0]*x[0] - 0.5*x[1];

  // **** f:
  fnvals[0] = feval(x, rank, nprocs);

  if (rank == 0) {
    write_data(argv[2], fnvals);
  }
  else {
    sleep(1);
  }

#ifdef TB_USE_MPI
  MPI::Finalize();
#endif // TB_USE_MPI

  exit(0);
}


/// Evaluate the "text_book" function
double feval(const std::vector<double>& x, size_t rank, size_t nprocs)
{
  double retval    = 0.;
  double local_val = 0.;

  for (size_t i=0; i<x.size(); ++i)
    local_val += pow(x[i]-1.0, 4.0);

  local_val /= nprocs;

  if (nprocs > 1) {
    double global_val = 0.;
#ifdef TB_USE_MPI
    MPI::COMM_WORLD.Reduce(&local_val, &global_val, 1, MPI_DOUBLE, MPI_SUM, 0);
#endif // TB_USE_MPI

    // only rank 0 has the correct sum.  This is OK (MPI_Allreduce not needed)
    // since only rank 0 writes the results.
    if (rank == 0)
      retval = global_val;
  }
  else
    retval = local_val;

  return retval;
}


/// Write response data to file specified by filename arg
void write_data(const std::string& filename, const std::vector<double>& fnvals)
{
  std::ofstream fout( filename.c_str() );

  // Computed results are output directly to file (the NO_FILTER option
  // is used).  Response tags are now optional; output them for ease of
  // results readability.
  if (!fout) {
    // WJB -- ToDo: rather than return status code -1, throw EXCEPTION
    std::cerr << "\nError: failure creating " << filename << std::endl;
  }
  else {
    fout.precision(15); // 16 total digits
    fout.setf(std::ios::scientific);
    fout.setf(std::ios::right);

    fout << "                     " << fnvals[0] << " f\n";
    fout << "                     " << fnvals[1] << " c1\n";
    fout << "                     " << fnvals[2] << " c2\n";

    fout.flush();
    fout.close();
  }
}

