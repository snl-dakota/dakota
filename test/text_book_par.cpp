/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

#ifdef USE_MPI
#include <mpi.h>
#endif // USE_MPI

#define POW_VAL 1.0 // text_book: 1.0 is nominal, 1.4 used for B&B testing


int main(int argc, char** argv)
{

  int rank = 0, size = 1;
#ifdef USE_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif // USE_MPI

  size_t i, j, k, num_vars, num_fns, num_deriv_vars;
  std::ofstream fout;
  double* x;
  int* ASV;
  int* DVV;
  if (rank == 0) {
    std::ifstream fin(argv[1]);
    std::string vars_text, fns_text, dvv_text;

    // Get the parameter vector and ignore the labels
    fin >> num_vars >> vars_text;
    x = new double [num_vars];
    for (i=0; i<num_vars; i++) {
      fin >> x[i];
      fin.ignore(256, '\n');
    }

    // Get the ASV vector and ignore the labels
    fin >> num_fns >> fns_text;
    ASV = new int [num_fns];
    for (i=0; i<num_fns; i++) {
      fin >> ASV[i];
      fin.ignore(256, '\n');
    }

    // Get the DVV vector and ignore the labels
    fin >> num_deriv_vars >> dvv_text;
    DVV = new int [num_deriv_vars];
    for (i=0; i<num_deriv_vars; i++) {
      fin >> DVV[i];
      fin.ignore(256, '\n');
    }

#ifdef USE_MPI
    // Broadcast input data to other processors (no MPIPackBuffer)
    MPI_Bcast(&num_vars, 1, MPI_INT,    0, MPI_COMM_WORLD);
    MPI_Bcast(x, num_vars,  MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_fns, 1,  MPI_INT,    0, MPI_COMM_WORLD);
    MPI_Bcast(ASV, num_fns, MPI_INT,    0, MPI_COMM_WORLD);
    MPI_Bcast(&num_deriv_vars, 1,  MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(DVV, num_deriv_vars, MPI_INT, 0, MPI_COMM_WORLD);
#endif // USE_MPI

    // Compute the results and output them directly to argv[2] (the NO_FILTER
    // option is used).  Response tags are now optional; output them for ease
    // of results readability.
    fout.open(argv[2]);
    fout.precision(15); // 16 total digits
    fout.setf(std::ios::scientific);
    fout.setf(std::ios::right);
  }
#ifdef USE_MPI
  else {
    // Receive input data from rank 0 (no MPIUnpackBuffer)
    MPI_Bcast(&num_vars, 1, MPI_INT, 0, MPI_COMM_WORLD);
    x = new double [num_vars];
    MPI_Bcast(x, num_vars,  MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_fns, 1,  MPI_INT, 0, MPI_COMM_WORLD);
    ASV = new int [num_fns];
    MPI_Bcast(ASV, num_fns, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_deriv_vars, 1,  MPI_INT, 0, MPI_COMM_WORLD);
    DVV = new int [num_deriv_vars];
    MPI_Bcast(DVV, num_deriv_vars, MPI_INT, 0, MPI_COMM_WORLD);
  }
#endif // USE_MPI

  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  if (ASV[0] & 1) {
    double local_val = 0.0;
    for (i=rank; i<num_vars; i+=size) {
      double x_i = x[i];
      local_val += pow(x_i-POW_VAL, 4);
#ifdef TB_EXPENSIVE
      for (j=1; j<=5000; j++)
        local_val += 1./(pow(x_i-POW_VAL,4)+j/100.)/(pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double global_val = 0.0;
#ifdef USE_MPI
      MPI_Reduce(&local_val, &global_val, 1, MPI_DOUBLE, MPI_SUM, 0,
		 MPI_COMM_WORLD);
#endif // USE_MPI
      // only rank 0 has the correct sum.  This is OK (MPI_Allreduce not needed)
      // since only rank 0 returns the results to the output file.
      if (rank == 0)
	fout << "                     " << global_val << " f\n";
    }
    else
      fout << "                     " << local_val << " f\n";
  }

  // **********************************
  // **** c1: x[0]*x[0] - 0.5*x[1] ****
  // **********************************
  if (num_fns > 1 && (ASV[1] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=rank; i<num_vars; i+=size) {
      double x_i = x[i];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val += x_i*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val -= 0.5*x_i;
#ifdef TB_EXPENSIVE
      for (j=1; j<=5000; j++)
        local_val += 1./(pow(x_i-POW_VAL,4)+j/100.)/(pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double global_val = 0.0;
#ifdef USE_MPI
      MPI_Reduce(&local_val, &global_val, 1, MPI_DOUBLE, MPI_SUM, 0,
		 MPI_COMM_WORLD);
#endif // USE_MPI
      // only rank 0 has the correct sum.  This is OK (MPI_Allreduce not needed)
      // since only rank 0 returns the results to the output file.
      if (rank == 0)
	fout << "                     " << global_val << " c1\n";
    }
    else
      fout << "                     " << local_val << " c1\n";
  }

  // **********************************
  // **** c2: x[1]*x[1] - 0.5*x[0] ****
  // **********************************
  if (num_fns > 2 && (ASV[2] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=rank; i<num_vars; i+=size) {
      double x_i = x[i];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val -= 0.5*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val += x_i*x_i;
#ifdef TB_EXPENSIVE
      for (j=1; j<=5000; j++)
        local_val += 1./(pow(x_i-POW_VAL,4)+j/100.)/(pow(x_i-POW_VAL,4)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double global_val = 0.0;
#ifdef USE_MPI
      MPI_Reduce(&local_val, &global_val, 1, MPI_DOUBLE, MPI_SUM, 0,
		 MPI_COMM_WORLD);
#endif // USE_MPI
      // only rank 0 has the correct sum.  This is OK (MPI_Allreduce not needed)
      // since only rank 0 returns the results to the output file.
      if (rank == 0)
	fout << "                     " << global_val << " c2\n";
    }
    else
      fout << "                     " << local_val << " c2\n";
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (ASV[0] & 2) {
    double* local_grad = new double [num_deriv_vars];
    //for (i=0; i<num_deriv_vars; i++)
    //  local_grad[i] = 4.*pow(xC[i]-POW_VAL,3);
    for (i=0; i<num_deriv_vars; i++)
      local_grad[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      double x_i = x[DVV[i]-1]; // assumes no discrete vars
      local_grad[i] = 4.*pow(x_i-POW_VAL,3);
#ifdef TB_EXPENSIVE
      for (j=1; j<=5000; j++)
        local_grad[i] += 1./(pow(x_i-POW_VAL,3)+j/100.)
                           /(pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_grad = (rank) ? NULL : new double [num_deriv_vars];
#ifdef USE_MPI
      MPI_Reduce(local_grad, global_grad, num_deriv_vars, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[ ";
	for (i=0; i<num_deriv_vars; i++)
	  fout << global_grad[i] << ' ';
	fout << "]\n";
	delete [] global_grad;
      }
    }
    else {
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	fout << local_grad[i] << ' ';
      fout << "]\n";
    }
    delete [] local_grad;
  }

  // *****************
  // **** dc1/dx: ****
  // *****************
  if (num_fns > 1 && (ASV[1] & 2)) {
    double* local_grad = new double [num_deriv_vars];
    //local_grad[0] = 2.*x[0];
    //local_grad[1] = -0.5;
    for (i=0; i<num_deriv_vars; i++)
      local_grad[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      int var_index = DVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        local_grad[i] = 2.*x[0];
      else if (var_index == 1)
        local_grad[i] = -0.5;
#ifdef TB_EXPENSIVE
      double x_i = x[var_index];
      for (j=1; j<=5000; j++)
        local_grad[i] += 1./(pow(x_i-POW_VAL,3)+j/100.)
                           /(pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_grad = (rank) ? NULL : new double [num_deriv_vars];
#ifdef USE_MPI
      MPI_Reduce(local_grad, global_grad, num_deriv_vars, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[ ";
	for (i=0; i<num_deriv_vars; i++)
	  fout << global_grad[i] << ' ';
	fout << "]\n";
	delete [] global_grad;
      }
    }
    else {
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	fout << local_grad[i] << ' ';
      fout << "]\n";
    }
    delete [] local_grad;
  }

  // *****************
  // **** dc2/dx: ****
  // *****************
  if (num_fns > 2 && (ASV[2] & 2)) {
    double* local_grad = new double [num_deriv_vars];
    //local_grad[0] = -0.5;
    //local_grad[1] = 2.*x[1];
    for (i=0; i<num_deriv_vars; i++)
      local_grad[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      int var_index = DVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        local_grad[i] = -0.5;
      else if (var_index == 1)
        local_grad[i] = 2.*x[1];
#ifdef TB_EXPENSIVE
      double x_i = x[var_index];
      for (j=1; j<=5000; j++)
        local_grad[i] += 1./(pow(x_i-POW_VAL,3)+j/100.)
                           /(pow(x_i-POW_VAL,3)+j/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_grad = (rank) ? NULL : new double [num_deriv_vars];
#ifdef USE_MPI
      MPI_Reduce(local_grad, global_grad, num_deriv_vars, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[ ";
	for (i=0; i<num_deriv_vars; i++)
	  fout << global_grad[i] << ' ';
	fout << "]\n";
	delete [] global_grad;
      }
    }
    else {
      fout << "[ ";
      for (i=0; i<num_deriv_vars; i++)
	fout << local_grad[i] << ' ';
      fout << "]\n";
    }
    delete [] local_grad;
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (ASV[0] & 4) {
    int num_doubles = num_deriv_vars * num_deriv_vars;
    double* local_hess = new double [num_doubles];
    //for (i=0; i<num_deriv_vars; i++)
    //  local_hess[i][i] = 12.*pow(x[i]-POW_VAL,2);
    for (i=0; i<num_doubles; i++)
      local_hess[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      double x_i = x[DVV[i]-1]; // assumes no discrete vars
      local_hess[i*num_deriv_vars+i] = 12.*pow(x_i-POW_VAL,2);
#ifdef TB_EXPENSIVE
      for (j=0; j<num_deriv_vars; j++)
	for (k=1; k<=5000; k++)
	  local_hess[i*num_deriv_vars+j] += 1./(pow(x_i-POW_VAL,2)+k/100.)
                                              /(pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_hess = (rank) ? NULL : new double [num_doubles];
#ifdef USE_MPI
      MPI_Reduce(local_hess, global_hess, num_doubles, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[[ ";
	for (i=0; i<num_doubles; i++)
	  fout << global_hess[i] << ' ';
	fout << "]]\n";
	delete [] global_hess;
      }
    }
    else {
      fout << "[[ ";
      for (i=0; i<num_doubles; i++)
	fout << local_hess[i] << ' ';
      fout << "]]\n";
    }
    delete [] local_hess;
  }

  // *********************
  // **** d^2c1/dx^2: ****
  // *********************
  if (num_fns > 1 && (ASV[1] & 4)) {
    int num_doubles = num_deriv_vars * num_deriv_vars;
    double* local_hess = new double [num_doubles];
    //local_hess[0][0] = 2.;
    for (i=0; i<num_doubles; i++)
      local_hess[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      int var_index = DVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
	local_hess[i*num_deriv_vars+i] = 2.;
#ifdef TB_EXPENSIVE
      double x_i = x[var_index];
      for (j=0; j<num_deriv_vars; j++)
	for (k=1; k<=5000; k++)
	  local_hess[i*num_deriv_vars+j] += 1./(pow(x_i-POW_VAL,2)+k/100.)
                                              /(pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_hess = (rank) ? NULL : new double [num_doubles];
#ifdef USE_MPI
      MPI_Reduce(local_hess, global_hess, num_doubles, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[[ ";
	for (i=0; i<num_doubles; i++)
	  fout << global_hess[i] << ' ';
	fout << "]]\n";
	delete [] global_hess;
      }
    }
    else {
      fout << "[[ ";
      for (i=0; i<num_doubles; i++)
	fout << local_hess[i] << ' ';
      fout << "]]\n";
    }
    delete [] local_hess;
  }

  // *********************
  // **** d^2c2/dx^2: ****
  // *********************
  if (num_fns > 2 && (ASV[2] & 4)) {
    int num_doubles = num_deriv_vars * num_deriv_vars;
    double* local_hess = new double [num_doubles];
    //local_hess[1][1] = 2.;
    for (i=0; i<num_doubles; i++)
      local_hess[i] = 0.;
    for (i=rank; i<num_deriv_vars; i+=size) {
      int var_index = DVV[i] - 1; // assumes no discrete vars
      if (var_index == 1)
	local_hess[i*num_deriv_vars+i] = 2.;
#ifdef TB_EXPENSIVE
      double x_i = x[var_index];
      for (j=0; j<num_deriv_vars; j++)
	for (k=1; k<=5000; k++)
	  local_hess[i*num_deriv_vars+j] += 1./(pow(x_i-POW_VAL,2)+k/100.)
                                              /(pow(x_i-POW_VAL,2)+k/100.);
#endif // TB_EXPENSIVE
    }

    if (size > 1) {
      double* global_hess = (rank) ? NULL : new double [num_doubles];
#ifdef USE_MPI
      MPI_Reduce(local_hess, global_hess, num_doubles, MPI_DOUBLE, MPI_SUM,
		 0, MPI_COMM_WORLD);
#endif // USE_MPI
      if (rank == 0) {
	fout << "[[ ";
	for (i=0; i<num_doubles; i++)
	  fout << global_hess[i] << ' ';
	fout << "]]\n";
	delete [] global_hess;
      }
    }
    else {
      fout << "[[ ";
      for (i=0; i<num_doubles; i++)
	fout << local_hess[i] << ' ';
      fout << "]]\n";
    }
    delete [] local_hess;
  }

  if (rank == 0) {
    fout.flush();
    fout.close();
  }

  delete [] x;
  delete [] ASV;
  delete [] DVV;

#ifdef USE_MPI
  MPI_Finalize();
#endif // USE_MPI

  return 0;
}
