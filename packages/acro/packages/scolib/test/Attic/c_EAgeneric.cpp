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
#if !defined(TFLOPS)
#include <utilib/std_headers.h>
#include <scolib/c_EAgeneric.h>

using namespace std;

#define DIMENSION 3


//
// Create a vector<double> and cast this to the pointer type used
// by c_EAgeneric
//
void c_EAgeneric_initialize_point(EAgenericUserType& pt_)
{
vector<double>* pt = new vector<double>();
pt->resize(DIMENSION);
pt_ = static_cast<EAgenericUserType>(pt);
}


//
// Randomize the point
//
void c_EAgeneric_randomize_point(EAgenericUserType& pt_)
{
vector<double>* point = static_cast<vector<double>*>(pt_);
for (int i=0; i<DIMENSION; i++)
  #if defined(__MINGW32__)
  (*point)[i] = rand() % 10;
  #else
  (*point)[i] = random() % 10;
  #endif
}


//
// Initialize problem-specific information
//
void c_EAgeneric_init_problem(int& num_constraints)
{
num_constraints=3;
}


void c_EAgeneric_init_constraints(double* clower, double* cupper)
{
for (int i=0; i<3; i++) {
  clower[i] = -10.0;
  cupper[i] = 0.0;
  }
}


//
// Evaluate a point.  First cast the pointer type used by c_EAgeneric
// into a vector<double>, and then compute the objective (a simple
// quadratic in this case
//
void c_EAgeneric_evaluate_point(const EAgenericUserType& pt_, double& value,
				double* cvalues)
{
vector<double>* point = static_cast<vector<double>*>(pt_);

value=0.0;
for (int i=0; i<DIMENSION; i++) value += (*point)[i] * (*point)[i];

cvalues[0] = 100.0;
}


//
// Initialize the search operators
//
void c_EAgeneric_init_operators()
{}


//
// Copy a point.
//
void c_EAgeneric_copy_point(EAgenericUserType from_, EAgenericUserType to_)
{
int i;
vector<double>* from = static_cast<vector<double>*>(from_);
vector<double>* to   = static_cast<vector<double>*>(to_);

for (i=0; i<DIMENSION; i++) (*to)[i] = (*from)[i];
}


//
// Write a point.
//
void c_EAgeneric_write_point(EAgenericUserType pt_)
{
int i;
vector<double>* pt = static_cast<vector<double>*>(pt_);

for (i=0; i<DIMENSION; i++) printf("%f ",(*pt)[i]);
}


//
// Assess whether two points are equal
//
int c_EAgeneric_point_equal(EAgenericUserType pt1_, EAgenericUserType pt2_)
{
int i;
vector<double>* pt1 = static_cast<vector<double>*>(pt1_);
vector<double>* pt2 = static_cast<vector<double>*>(pt2_);

for (i=0; i<DIMENSION; i++) if ((*pt1)[i] != (*pt2)[i]) return 0;
return 1;
}


//
// Perform crossover (simple mid-point xover)
//
int c_EAgeneric_xover(EAgenericUserType p1_, EAgenericUserType p2_,
				EAgenericUserType c1_)
{
int i;
vector<double>* p1 = static_cast<vector<double>*>(p1_);
vector<double>* p2 = static_cast<vector<double>*>(p2_);
vector<double>* c1 = static_cast<vector<double>*>(c1_);

for (i=0; i<DIMENSION; i++) (*c1)[i] = ((*p1)[i] + (*p2)[i])/2.0;
return 3;
}


//
// Perform mutation (a fixed offset)
//
void c_EAgeneric_mutation(EAgenericUserType pt_, int* dummy, int* flag)
{
int i;
vector<double>* pt = static_cast<vector<double>*>(pt_);
*flag = 1;   /* Evaluate this point */
for (i=0; i<DIMENSION; i++) {
  #ifdef __MINGW32__
  int foo = rand();
  #else
  int foo = random();
  #endif
  if (foo % 2 == 0)
     (*pt)[i] += 0.1;
  else
     (*pt)[i] -= 0.1;
  }
}


int main(int argc, char** argv)
{
//
// Setup solver
//
EAgenericObjectType obj = c_EAgeneric_allocate();
c_EAgeneric_set_parameter(obj,"seed", "1");
c_EAgeneric_set_parameter(obj,"max_neval", "100");
c_EAgeneric_set_parameter(obj,"debug", "1000000");
c_EAgeneric_set_parameter(obj,"population_size", "10");
c_EAgeneric_set_parameter(obj,"precision", "15");
c_EAgeneric_reset(obj);
//
// Perform minimization
//
c_EAgeneric_minimize(obj);
//
// Get the best solution
//
vector<double> best_point;
double best_val;
c_EAgeneric_get_best_point(obj, (EAgenericUserType)&best_point );
c_EAgeneric_get_value_of_best_point(obj, &best_val );
//
// Cleanup
//
c_EAgeneric_deallocate(obj);

return 0;
}
#endif
