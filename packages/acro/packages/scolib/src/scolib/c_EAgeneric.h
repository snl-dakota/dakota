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

/**
 * \file c_EAgeneric.h
 *
 * Defines an API for defining generic evolutionary algorithms.
 */

#ifndef scolib_c_EAgeneric_h
#define scolib_c_EAgeneric_h

#include <acro_config.h>

typedef void* EAgenericUserType;
typedef void* EAgenericObjectType;


/* Routines used to setup and execute EAgeneric with the C interface. */
extern EAgenericObjectType c_EAgeneric_allocate();
extern void c_EAgeneric_set_parameter(EAgenericObjectType obj,
			char* name, char* value);
extern void c_EAgeneric_deallocate(EAgenericObjectType& obj);
extern void c_EAgeneric_reset(EAgenericObjectType obj);
extern void c_EAgeneric_minimize(EAgenericObjectType obj);
extern void c_EAgeneric_get_best_point(EAgenericObjectType obj, 
			EAgenericUserType pt);
extern void c_EAgeneric_get_value_of_best_point(EAgenericObjectType obj,
			double* value);

/* Routines used to perform search for a EAgeneric with the C interface. */
extern void c_EAgeneric_initialize_point(EAgenericUserType& pt);
extern void c_EAgeneric_randomize_point(EAgenericUserType& pt);
extern void c_EAgeneric_init_operators();
extern void c_EAgeneric_copy_point(EAgenericUserType, EAgenericUserType);
extern void c_EAgeneric_write_point(EAgenericUserType);
extern void c_EAgeneric_mutation(EAgenericUserType, int*, int*);
extern int  c_EAgeneric_point_equal(EAgenericUserType, EAgenericUserType);
extern int  c_EAgeneric_point_less(EAgenericUserType, EAgenericUserType);
extern int  c_EAgeneric_xover(EAgenericUserType, EAgenericUserType, 
			EAgenericUserType);
extern void c_EAgeneric_init_constraints(double* clower, double* cupper);
extern void c_EAgeneric_init_problem(int& num_constraints);
extern void c_EAgeneric_evaluate_point(const EAgenericUserType& pt_, double& value,
                        double* cvalues);

#endif
