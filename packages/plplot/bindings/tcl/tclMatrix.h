/* -*-C++-*-
 * $Id: tclMatrix.h 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1994, 1995
    Maurice LeBrun			mjl@dino.ph.utexas.edu
    Institute for Fusion Studies	University of Texas at Austin

    Copyright (C) 2004  Maurice LeBrun

    This file is part of PLplot.

    PLplot is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PLplot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PLplot; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    Contains declarations for Tcl "Matrix" command.
    C functions that need access to the matrix data will need
    to include this file.
*/

#ifndef __TCLMATRIX_H__
#define __TCLMATRIX_H__

#include "plplot.h"
#include <tcl.h>

typedef PLFLT Mat_float;

#if defined(MSDOS)
typedef long  Mat_int;
#else
typedef int   Mat_int;
#endif

enum { TYPE_FLOAT, TYPE_INT };

/* Arrays are column dominant (normal C ordering) */
/* Array elements are stored contiguously */
/* Require dimension <= 3 for simplicity */

#define MAX_ARRAY_DIM 3

/* Useful macros for index calculations */

#define I3D(i,j,k)	k + matPtr->n[2] * (I2D(i,j))
#define I2D(i,j)	j + matPtr->n[1] * (I1D(i))
#define I1D(i)		i

/* Matrix operator data */

typedef struct {
    int type;			/* Data type */
    int len;			/* Total length of array */
    int dim;			/* Number of dimensions */
    int n[MAX_ARRAY_DIM];	/* Holds array length in each dimension */
    int tracing;		/* Set if not persistent */

    char *name;			/* Matrix operator name, malloc'ed */

    Mat_float *fdata;		/* Floating point data, malloc'ed */
    Mat_int   *idata;		/* Integer data, malloc'ed */

    Tcl_Interp *interp;		/* Interpreter where command is installed */

/* These do the put/get operations for each supported type */

    void (*put) (ClientData clientData, Tcl_Interp* interp, int index, char *string);
    void (*get) (ClientData clientData, Tcl_Interp* interp, int index, char *string);

} tclMatrix;

/* Function prototypes */

#ifdef __cplusplus
/*---------------------------------------------------------------------------//
// Since C++ does not generally have a per-platform ABI the way C
// does, we stick to a totally inline class declaration and
// definition.  That way you don't have to keep a separate version of
// libplplot*.a for each compiler you'd like to use.

// Start by setting up some important macros.
*/

#include <iostream>

#ifdef throw
#define TCL_NO_UNDEF
#endif

#ifndef throw
#ifdef __hpux
#if defined(__GNUC__) || defined(__lucid) || defined(__CENTERLINE__) \
|| defined(CENTERLINE_CLPP)
#define NO_XCPT
#endif
#else
#define NO_XCPT
#endif

#ifdef NO_XCPT
#define try
#define throw(a) \
{ cerr << "THROW: " << #a << " from " << __FILE__ \
       << " line " << __LINE__ << endl << flush; abort(); }
#define catch(a) if (0)
#define Throw
#else
#define Throw throw
#endif
#endif

#define tMat_Assert(a,b) if (!(a)) \
{ using namespace std; \
  cerr << "Assertion " << #a << " failed in " << __FILE__ \
       << " at line " << __LINE__ << endl << flush; \
  throw(b); }

/*---------------------------------------------------------------------------//
// class TclMatFloat

// This class provides a convenient way to access the data of a
// tclMatrix from within compiled code.  Someone should make clones of
// this class for the other tclMatrix supported data types.
//---------------------------------------------------------------------------*/

class TclMatFloat {
    tclMatrix *matPtr;
  public:
    TclMatFloat( tclMatrix *ptm )
	: matPtr(ptm)
    {
	tMat_Assert( matPtr->type == TYPE_FLOAT, "Type mismatch" );
    }

    int Dimensions() const { return matPtr->dim; }

    int dim_size( int d ) const {
	tMat_Assert( d < matPtr->dim, "Range error." );
	return matPtr->n[d];
    }

    void redim( int nx )
    {
	free( matPtr->fdata );
	matPtr->dim = 1;
	matPtr->n[0] = nx;
	matPtr->len = nx;
	matPtr->fdata = (Mat_float *) malloc( matPtr->len *
					      sizeof(Mat_float) );
    }

    void redim( int nx, int ny )
    {
	free( matPtr->fdata );
	matPtr->dim = 2;
	matPtr->n[0] = nx;
	matPtr->n[1] = ny;
	matPtr->len = nx * ny;
	matPtr->fdata = (Mat_float *) malloc( matPtr->len *
					      sizeof(Mat_float) );
    }

    void redim( int nx, int ny, int nz )
    {
	free( matPtr->fdata );
	matPtr->dim = 3;
	matPtr->n[0] = nx;
	matPtr->n[1] = ny;
	matPtr->n[2] = nz;
	matPtr->len = nx * ny * nz;
	matPtr->fdata = (Mat_float *) malloc( matPtr->len *
					      sizeof(Mat_float) );
    }

    Mat_float& operator()( int i )
    {
	tMat_Assert( matPtr->dim == 1, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0],
		     "Out of bounds reference" );

	return matPtr->fdata[i];
    }

    Mat_float& operator()( int i, int j )
    {
	tMat_Assert( matPtr->dim == 2, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0] &&
		     j >= 0 && j < matPtr->n[1],
		     "Out of bounds reference" );

	return matPtr->fdata[I2D(i,j)];
    }

    Mat_float& operator()( int i, int j, int k )
    {
	tMat_Assert( matPtr->dim == 3, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0] &&
		     j >= 0 && j < matPtr->n[1] &&
		     k >= 0 && k < matPtr->n[2],
		     "Out of bounds reference" );

	return matPtr->fdata[I3D(i,j,k)];
    }
};

/*---------------------------------------------------------------------------//
// class TclMatInt

// This class provides a convenient way to access the data of a
// tclMatrix from within compiled code.  This is just like TclMatFloat above,
// but for ints.
//---------------------------------------------------------------------------*/

class TclMatInt {
    tclMatrix *matPtr;
  public:
    TclMatInt( tclMatrix *ptm )
	: matPtr(ptm)
    {
	tMat_Assert( matPtr->type == TYPE_INT, "Type mismatch" );
    }

    int Dimensions() const { return matPtr->dim; }

    int dim_size( int d ) const {
	tMat_Assert( d < matPtr->dim, "Range error." );
	return matPtr->n[d];
    }

    void redim( int nx )
    {
	free( matPtr->idata );
	matPtr->dim = 1;
	matPtr->n[0] = nx;
	matPtr->len = nx;
	matPtr->idata = (Mat_int *) malloc( matPtr->len * sizeof(Mat_int) );
    }

    void redim( int nx, int ny )
    {
	free( matPtr->idata );
	matPtr->dim = 2;
	matPtr->n[0] = nx;
	matPtr->n[1] = ny;
	matPtr->len = nx * ny;
	matPtr->idata = (Mat_int *) malloc( matPtr->len * sizeof(Mat_int) );
    }

    void redim( int nx, int ny, int nz )
    {
	free( matPtr->idata );
	matPtr->dim = 3;
	matPtr->n[0] = nx;
	matPtr->n[1] = ny;
	matPtr->n[2] = nz;
	matPtr->len = nx * ny * nz;
	matPtr->idata = (Mat_int *) malloc( matPtr->len * sizeof(Mat_int) );
    }

    Mat_int& operator()( int i )
    {
	tMat_Assert( matPtr->dim == 1, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0],
		     "Out of bounds reference" );

	return matPtr->idata[i];
    }

    Mat_int& operator()( int i, int j )
    {
	tMat_Assert( matPtr->dim == 2, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0] &&
		     j >= 0 && j < matPtr->n[1],
		     "Out of bounds reference" );

	return matPtr->idata[I2D(i,j)];
    }

    Mat_int& operator()( int i, int j, int k )
    {
	tMat_Assert( matPtr->dim == 3, "Wrong number of indicies." );
	tMat_Assert( i >= 0 && i < matPtr->n[0] &&
		     j >= 0 && j < matPtr->n[1] &&
		     k >= 0 && k < matPtr->n[2],
		     "Out of bounds reference" );

	return matPtr->idata[I3D(i,j,k)];
    }
};

#ifndef TCL_NO_UNDEF

#ifdef NO_XCPT
#undef NO_XCPT
#undef try
#undef throw
#undef Throw
#undef catch
#endif

#endif

#undef tMat_Assert

extern "C" {
/*---------------------------------------------------------------------------*/
#endif

/* Tcl package initialisation function */

int Matrix_Init	(Tcl_Interp*);

/* This procedure is invoked to process the "matrix" Tcl command. */

int
Tcl_MatrixCmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char **argv);

/* Returns a pointer to the specified matrix operator's data. */

tclMatrix *
Tcl_GetMatrixPtr(Tcl_Interp *interp, char *matName);

/* Some stuff for handling extension subcommands. */

typedef int (*tclMatrixXtnsnProc) ( tclMatrix *pm, Tcl_Interp *interp,
				    int argc, char *argv[] );

typedef struct tclMatrixXtnsnDescr {
    char *cmd;
    tclMatrixXtnsnProc cmdproc;
    struct tclMatrixXtnsnDescr *next;
} tclMatrixXtnsnDescr;

int Tcl_MatrixInstallXtnsn( char *cmd, tclMatrixXtnsnProc proc );

#ifdef __cplusplus
}
#endif

#endif	/* __TCLMATRIX_H__ */
