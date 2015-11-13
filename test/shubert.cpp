/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
//#include "unistd.h" // for sleep


//KRD modified this starting from text_book.cpp
int main(int argc, char** argv)
{

  std::ifstream fin(argv[1]);
  if (!fin) {
    std::cerr << "\nError: failure opening " << argv[1] << std::endl;
    exit(-1);
  }
  size_t i, j, k, num_vars, num_fns, num_deriv_vars;
  std::string vars_text, fns_text, dvv_text;

  // Get the parameter std::vector and ignore the labels
  fin >> num_vars >> vars_text;
  std::vector<double> x(num_vars);
  for (i=0; i<num_vars; i++) {
    fin >> x[i];
    fin.ignore(256, '\n');
  }

  // Get the ASV std::vector and ignore the labels
  fin >> num_fns >> fns_text;
  if(num_fns!=1) {
    std::cerr << "\nError: Shubert doesn't support constraints but you said #constraints=" << num_fns-1 << std::endl;
    exit(-1);
  }
  std::vector<int> ASV(num_fns);
  fin >> ASV[0];
  fin.ignore(256, '\n');

  // Get the DVV std::vector and ignore the labels
  fin >> num_deriv_vars >> dvv_text;
  std::vector<int> DVV(num_deriv_vars);
  for (i=0; i<num_deriv_vars; i++) {
    fin >> DVV[i];
    fin.ignore(256, '\n');
  }

  //srand ( (unsigned int) (time(NULL)/x[0]) );
  //sleep( (int)(3.0*((double)rand()/RAND_MAX)));

  //sleep(5);

  // Compute the results and output them directly to argv[2] (the NO_FILTER
  // option is used).  Response tags are now optional; output them for ease
  // of results readability.
  std::ofstream fout(argv[2]);
  if (!fout) {
    std::cerr << "\nError: failure creating " << argv[2] << std::endl;
    exit(-1);
  }
  fout.precision(15); // 16 total digits
  fout.setf(std::ios::scientific);
  fout.setf(std::ios::right);

  // **** f:
  // f(\underline{x})=\prod_{i=1}^num_vars w(x_i) //latex notation
  // w(x_i) = \sum_{j=1}^5 j*cos(x_i*(j+1)+j)  //w for shubert
  std::vector<double> w(num_vars);  
  double jdbl;
  if (ASV[0] >=1)
    for (i=0; i<num_vars; i++) {
      w[i]=0.0;
      for (j=1; j<=5; ++j) {
	jdbl=static_cast<double>(j);
	w[i]+=jdbl*std::cos(x[i]*(jdbl+1.0)+jdbl);
      }
    }
  
  if (ASV[0] & 1) {
    double val = 1.0;
    for (i=0; i<num_vars; i++) 
      val*=w[i];
    fout << "                     " << val << " f\n";
  }

  // **** df/dx:
  // df/dx_i=(\prod_{j=1}^{i-1} w(x_j)) * dw(x_i)/dx_i * (\prod_{j=i+1}^{num_vars} w(x_j))
  // dw/dx_i= \sum_{j=1}^5 j*(j+1)*-sin(x_i*(j+1)+j)  //for shubert
  std::vector<double> d1w(num_vars);
  if (ASV[0] >=2) 
    for (i=0; i<num_deriv_vars; i++) {
      int ii=DVV[i]-1;
      d1w[ii]=0.0;
      for (j=1; j<=5; ++j) {
	jdbl=static_cast<double>(j);
	d1w[ii]+=jdbl*(jdbl+1.0)*-std::sin(x[ii]*(jdbl+1.0)+jdbl);
      }
    }
  double d1;
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; i++) {
      int ii=DVV[i]-1;
      d1=d1w[ii];
      for (j=0; j<ii; ++j)
	d1*=w[j];
      for (j=ii+1; j<num_vars; ++j)
	d1*=w[j];      
      fout << d1 << ' ';
    }
    fout << "]\n";
  }

  // **** d^2f/dx^2:
  // if i<k    d^2f/(dx_i*dx_k)=(\prod_{j=1)^{i-1} w(x_j))   * dw(x_i)/dx_i ...
  //                           *(\prod_{j=i+1)^{k-1} w(x_j)) * dw(x_k)/dx_k ...
  //                           *(\prod_{j=k+1)^{num_vars} w(x_j))  
  // if i==k   d^2f/dx_i^2 =(\prod_{j=1)^{i-1} w(x_j)) * d^2w(x_i)/dx_i^2 * (\prod_{j=i+1)^{num_vars} w(x_j))   
  // d^2w(x_i)/dx_i^2 = \sum{j=1}^5 j*(j+1)*(j+1)*-cos(x_i*(j+1)+j) //for shubert
  if (ASV[0] & 4) {
    fout << "[[ ";
    double d2;
    for (i=0; i<num_deriv_vars; ++i) {
      for (k=0; k<num_deriv_vars; ++k) {
	int ii=DVV[i]-1;
	int kk=DVV[k]-1;
	if (ii==kk) {
	  d2=0.0;
	  for (j=1; j<=5; ++j) {
	    jdbl=static_cast<double>(j);	    
	    d2+=jdbl*(jdbl+1.0)*(jdbl+1.0)*-std::cos(x[ii]*(jdbl+1.0)+jdbl);
	  }
	  for (j=0; j<ii; ++j)
	    d2*=w[j];
	  for (j=ii+1; j<num_vars; ++j)
	    d2*=w[j];
	}
	else {
	  d2=d1w[ii]*d1w[kk];
	  if(kk<ii) {
	    j=ii;
	    ii=kk;
	    kk=j;
	  }
	  for (j=0; j<ii; ++j)
	    d2*=w[j];
	  for (j=ii+1; j<kk; ++j)
	    d2*=w[j];
	  for (j=kk+1; j<num_vars; ++j)
	    d2*=w[j];
	}
	//	for (j=0; j<num_vars; ++j)
	//	  if((j!=ii)&&(j!=kk))
	//	    d2*=w[j];
	fout << d2 << ' ';
      }  
    }
    fout << "]]\n";
  }
  
  fout.flush();
  fout.close();
  return 0;
}
