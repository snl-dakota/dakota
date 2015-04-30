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
#include <scolib/c_EAgeneric.h>

//
// Routines used to setup and execute EAgeneric with the C interface.
//

EAgenericObjectType c_EAgeneric_allocate()
{
int* foo=0;
return foo;
}

void c_EAgeneric_set_parameter(EAgenericObjectType obj,
			char* name, char* value)
{}

void c_EAgeneric_deallocate(EAgenericObjectType& obj)
{}

void c_EAgeneric_minimize(EAgenericObjectType obj)
{}

void c_EAgeneric_get_best_point(EAgenericObjectType obj, EAgenericUserType pt)
{}

void c_EAgeneric_get_value_of_best_point(EAgenericObjectType obj, double* value)
{}

void c_EAgeneric_init_problem(int& num_constraints)
{}

void c_EAgeneric_init_constraints(double* clower, double* cupper)
{}

void c_EAgeneric_evaluate_point(const EAgenericUserType& pt_, double& value,
                                double* cvalues)
{}

//
// Routines used to perform search for a EAgeneric with the C interface.
//

void c_EAgeneric_randomize_point(EAgenericUserType& pt)
{}

void c_EAgeneric_initialize_point(EAgenericUserType& pt)
{}

void c_EAgeneric_init_operators()
{}

void c_EAgeneric_copy_point(EAgenericUserType, EAgenericUserType)
{}

void c_EAgeneric_write_point(EAgenericUserType)
{}

void c_EAgeneric_mutation(EAgenericUserType, int*, int*)
{}

int c_EAgeneric_point_equal(EAgenericUserType, EAgenericUserType)
{ return 0; }

int c_EAgeneric_point_less(EAgenericUserType, EAgenericUserType)
{ return 0; }

int c_EAgeneric_xover(EAgenericUserType, EAgenericUserType, EAgenericUserType)
{ return 0; }
