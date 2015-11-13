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


//KRD modified this starting from shubert.cpp
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
    std::cerr << "\nError: Herbie doesn't support constraints but you said #constraints=" << num_fns-1 << std::endl;
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
  // f(\underline{x})=-\prod_{i=1}^num_vars w(x_i) //latex notation
  // w(x_i) = \exp(-(x_i-1)^2)+\exp(-0.8*(x_i+1)^2)-0.05*\sin(8*(x_i+0.1)) //w for herbie
  std::vector<double> w(num_vars);  
  if (ASV[0] >= 1) 
    for (i=0; i<num_vars; i++) {
      double dtemp1=x[i]-1.0;
      double dtemp2=x[i]+1.0;
      w[i]=
	std::exp(-dtemp1*dtemp1)+
	std::exp(-0.8*dtemp2*dtemp2)
	-0.05*std::sin(8.0*(x[i]+0.1));
    }

  if(ASV[0] & 1) {
    double val = 1.0;
    for (i=0; i<num_vars; ++i) 
      val*=w[i];
    fout << "                     " << -val << " f\n";
  }

  // **** df/dx:
  // df/dx_i=-(\prod_{j=1}^{i-1} w(x_j)) * dw(x_i)/dx_i * (\prod_{j=i+1}^{num_vars} w(x_j))
  // dw/dx_i= -2*(x_i-1)*\exp(-(x_i-1)^2)-0.8*2*(x_i+1)*\exp(-0.8*(x_i+1)^2) -0.05*8*cos(8*(x_i+0.1))//for herbie
  std::vector<double> d1w(num_vars);
  if (ASV[0] >= 2) {
    for (i=0; i<num_deriv_vars; ++i) {
      int ii=DVV[i]-1;
      double dtemp1=x[ii]-1.0;
      double dtemp2=x[ii]+1.0;
      d1w[ii]=
	-2.0*dtemp1*std::exp(-dtemp1*dtemp1)
	-1.6*dtemp2*std::exp(-0.8*dtemp2*dtemp2)
	-0.4*std::cos(8.0*(x[ii]+0.1));
    }
  }
  double d1;
  if (ASV[0] & 2) {
    fout << "[ ";
    for (i=0; i<num_deriv_vars; ++i) {
      int ii=DVV[i]-1;
      d1=d1w[ii];
      for (j=0; j<ii; ++j)
	d1*=w[j];
      for (j=ii+1; j<num_vars; ++j)
	d1*=w[j];  
      fout << -d1 << ' ';
    }
    fout << "]\n";
  }

  // **** d^2f/dx^2:
  // if i<k   d^2f/(dx_i*dx_k)=-(\prod_{j=1)^{i-1} w(x_j))   * dw(x_i)/dx_i ...
  //                           *(\prod_{j=i+1)^{k-1} w(x_j)) * dw(x_k)/dx_k ...
  //                           *(\prod_{j=k+1)^{num_vars} w(x_j))  
  // if i==k   d^2f/dx_i^2 =(\prod_{j=1)^{i-1} w(x_j)) * d^2w(x_i)/dx_i^2 * (\prod_{j=i+1)^{num_vars} w(x_j))   
  // d^2w(x_i)/dx_i^2 = -2*\exp(-(x_i-1)^2)+4*(x_i-1)^2*\exp(-(x_i-1)^2) ...
  //                    -1.6*\exp(-0.8*(x_i+1)^2)+2.56*(x_i+1)^2*\exp(-0.8*(x_i+1)^2) 
  //                    +3.2*\sin(8*(x_i+0.1)) //for  herbie
  if (ASV[0] & 4) {
    fout << "[[ ";
    double d2;
    for (i=0; i<num_deriv_vars; ++i) {
      for (k=0; k<num_deriv_vars; ++k) {
	int ii=DVV[i]-1;
	int kk=DVV[k]-1;
	if (ii==kk) {
	  double dtemp1=x[ii]-1.0; dtemp1*=dtemp1;
	  double dtemp2=x[ii]+1.0; dtemp2*=dtemp2;
	  d2=(-2.0+4.0*dtemp1)*std::exp(-dtemp1)
	    +(-1.6+2.56*dtemp2)*std::exp(-0.8*dtemp2)
	    +3.2*std::sin(8.0*(x[ii]+0.1));
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
	//for (j=0; j<num_vars; ++j)
	//if((j!=ii)&&(j!=kk))
	//d2*=w[j];
	fout << -d2 << ' ';
      }    
    }
    fout << "]]\n";
  }
  
  fout.flush();
  fout.close();
  return 0;
}
