/* $Id: tclMatrix.c 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1994, 1995
    Maurice LeBrun			mjl@dino.ph.utexas.edu
    Institute for Fusion Studies	University of Texas at Austin

    Copyright (C) 2004  Joao Cardoso

    This file is part of PLplot.

    PLplot is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Library Public License as published
    by the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    PLplot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with PLplot; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    This file contains routines that implement Tcl matrices.
    These are operators that are used to store, return, and modify
    numeric data stored in binary array format.  The emphasis is
    on high performance and low overhead, something that Tcl lists
    or associative arrays aren't so good at.
*/

/*
#define DEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tclMatrix.h"

/* Cool math macros */

#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

/* For the truly desperate debugging task */

#ifdef DEBUG_ENTER
#define dbug_enter(a) \
    fprintf(stderr, "%s: Entered %s\n", __FILE__, a);

#else
#define dbug_enter(a)
#endif

/* Internal data */

static int matTable_initted = 0;/* Hash table initialization flag */
static Tcl_HashTable matTable;	/* Hash table for external access to data */

/* Function prototypes */

/* Handles matrix initialization lists */

static int
matrixInitialize(Tcl_Interp* interp, tclMatrix* m,
		 int dim, int offs, int nargs, char** args);

/* Invoked to process the "matrix" Tcl command. */

static  int
MatrixCmd(ClientData clientData, Tcl_Interp *interp, int argc, char **argv);

/* Causes matrix command to be deleted.  */

static char *
DeleteMatrixVar(ClientData clientData,
		Tcl_Interp *interp, char *name1, char *name2, int flags);

/* Releases all the resources allocated to the matrix command. */

static void
DeleteMatrixCmd(ClientData clientData);

/* These do the put/get operations for each supported type */

static void
MatrixPut_f(ClientData clientData, Tcl_Interp* interp, int index, char *string);

static void
MatrixGet_f(ClientData clientData, Tcl_Interp* interp, int index, char *string);

static void
MatrixPut_i(ClientData clientData, Tcl_Interp* interp, int index, char *string);

static void
MatrixGet_i(ClientData clientData, Tcl_Interp* interp, int index, char *string);

/*--------------------------------------------------------------------------*\
 *
 * Tcl_MatCmd --
 *
 *	Invoked to process the "matrix" Tcl command.  Creates a multiply
 *	dimensioned array (matrix) of floats or ints.  The number of
 *	arguments determines the dimensionality.
 *
 * Results:
 *	Returns the name of the new matrix.
 *
 * Side effects:
 *	A new matrix (operator) gets created.
 *
\*--------------------------------------------------------------------------*/

int
Tcl_MatrixCmd(ClientData clientData, Tcl_Interp *interp,
	      int argc, char **argv)
{
    register tclMatrix *matPtr;
    int i, j, length, new, index, persist = 0, initializer = 0;
    Tcl_HashEntry *hPtr;
    Tcl_CmdInfo infoPtr;
    char c;

    dbug_enter("Tcl_MatrixCmd");

    if (argc < 3) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
	    " ?-persist? var type dim1 ?dim2? ?dim3? ...\"", (char *) NULL);
	return TCL_ERROR;
    }

/* Create hash table on first call */

    if ( ! matTable_initted) {
	matTable_initted = 1;
	Tcl_InitHashTable(&matTable, TCL_STRING_KEYS);
    }

/* Check for -persist flag */

    for (i = 1; i < argc; i++) {
	c = argv[i][0];
	length = strlen(argv[i]);

    /* If found, set persist variable and compress argv-list */

	if ((c == '-') && (strncmp(argv[i], "-persist", length) == 0)) {
	    persist = 1;
	    argc --;
	    for (j = i; j < argc; j++)
		argv[j] = argv[j+1];
	    break;
	}
    }

/* Create matrix data structure */

    matPtr = (tclMatrix *) malloc(sizeof(tclMatrix));
    matPtr->fdata = NULL;
    matPtr->idata = NULL;
    matPtr->name = NULL;
    matPtr->dim = 0;
    matPtr->len = 1;
    matPtr->tracing = 0;
    for (i = 0; i < MAX_ARRAY_DIM; i++)
	matPtr->n[i] = 1;

/* Create name */
/* It should be unique */

    argc--; argv++;

    if (Tcl_GetCommandInfo(interp, argv[0], &infoPtr)) {
	Tcl_AppendResult(interp, "Matrix operator \"", argv[0],
	    "\" already in use", (char *) NULL);
	free((void *) matPtr);
	return TCL_ERROR;
    }

    if (Tcl_GetVar(interp, argv[0], 0) != NULL) {
	Tcl_AppendResult(interp, "Illegal name for Matrix operator \"",
	    argv[0], "\": local variable of same name is active",
	    (char *) NULL);
	free((void *) matPtr);
	return TCL_ERROR;
    }

    matPtr->name = (char *) malloc(strlen(argv[0])+1);
    strcpy(matPtr->name, argv[0]);

/* Initialize type */

    argc--; argv++;
    c = argv[0][0];
    length = strlen(argv[0]);

    if ((c == 'f') && (strncmp(argv[0], "float", length) == 0)) {
	matPtr->type = TYPE_FLOAT;
	matPtr->put = MatrixPut_f;
	matPtr->get = MatrixGet_f;
    }
    else if ((c == 'i') && (strncmp(argv[0], "int", length) == 0)) {
	matPtr->type = TYPE_INT;
	matPtr->put = MatrixPut_i;
	matPtr->get = MatrixGet_i;
    }
    else {
	Tcl_AppendResult(interp, "Matrix type \"", argv[0],
	    "\" not supported, should be \"float\" or \"int\"",
	    (char *) NULL);

	DeleteMatrixCmd((ClientData) matPtr);
	return TCL_ERROR;
    }

/* Initialize dimensions */

    argc--; argv++;
    for (; argc > 0; argc--, argv++) {

    /* Check for initializer */

	if (strcmp(argv[0], "=") == 0) {
	    argc--; argv++;
	    initializer = 1;
	    break;
	}

    /* Must be a dimensional parameter.  Increment number of dimensions. */

	matPtr->dim++;
	if (matPtr->dim > MAX_ARRAY_DIM) {
	    Tcl_AppendResult(interp,
		"too many dimensions specified for Matrix operator \"",
		matPtr->name, "\"", (char *) NULL);

	    DeleteMatrixCmd((ClientData) matPtr);
	    return TCL_ERROR;
	}

    /* Check to see if dimension is valid and store */

	index = matPtr->dim - 1;
	matPtr->n[index] = atoi(argv[0]);
	if (matPtr->n[index] < 1) {
	    Tcl_AppendResult(interp, "invalid matrix dimension \"", argv[0],
		"\" for Matrix operator \"", matPtr->name, "\"",
		(char *) NULL);

	    DeleteMatrixCmd((ClientData) matPtr);
	    return TCL_ERROR;
	}
	matPtr->len *= matPtr->n[index];
    }

    if (matPtr->dim < 1) {
	Tcl_AppendResult(interp,
	    "insufficient dimensions given for Matrix operator \"",
	     matPtr->name, "\"", (char *) NULL);
	DeleteMatrixCmd((ClientData) matPtr);
	return TCL_ERROR;
    }

/* Allocate space for data */

    switch (matPtr->type) {
    case TYPE_FLOAT:
	matPtr->fdata = (Mat_float *) malloc(matPtr->len * sizeof(Mat_float));
	for (i = 0; i < matPtr->len; i++)
	    matPtr->fdata[i] = 0.0;
	break;

    case TYPE_INT:
	matPtr->idata = (Mat_int *) malloc(matPtr->len * sizeof(Mat_int));
	for (i = 0; i < matPtr->len; i++)
	    matPtr->idata[i] = 0;
	break;
    }

/* Process the initializer, if present */

    if (initializer)
	matrixInitialize(interp, matPtr, 0, 0, 1, &argv[0]);

/* Delete matrix when it goes out of scope unless -persist specified */
/* Use local variable of same name as matrix and trace it for unsets */

    if ( ! persist) {
	if (Tcl_SetVar(interp, matPtr->name,
		       "old_bogus_syntax_please_upgrade", 0) == NULL) {
	    Tcl_AppendResult(interp, "unable to schedule Matrix operator \"",
		matPtr->name, "\" for automatic deletion", (char *) NULL);
	    DeleteMatrixCmd((ClientData) matPtr);
	    return TCL_ERROR;
	}
	matPtr->tracing = 1;
	Tcl_TraceVar(interp, matPtr->name, TCL_TRACE_UNSETS,
		     (Tcl_VarTraceProc*) DeleteMatrixVar, (ClientData) matPtr);
    }

/* Create matrix operator */

#ifdef DEBUG
    fprintf(stderr, "Creating Matrix operator of name %s\n", matPtr->name);
#endif
    Tcl_CreateCommand(interp, matPtr->name, (Tcl_CmdProc*)  MatrixCmd,
		      (ClientData) matPtr, (Tcl_CmdDeleteProc*) DeleteMatrixCmd);

/* Store pointer to interpreter to handle bizarre uses of multiple */
/* interpreters (e.g. as in [incr Tcl]) */

    matPtr->interp = interp;

/* Create hash table entry for this matrix operator's data */
/* This should never fail */

    hPtr = Tcl_CreateHashEntry(&matTable, matPtr->name, &new);
    if ( ! new) {
	Tcl_AppendResult(interp,
	    "Unable to create hash table entry for Matrix operator \"",
	    matPtr->name, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(hPtr, matPtr);

    Tcl_SetResult( interp, matPtr->name, TCL_VOLATILE );
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 *
 * Tcl_GetMatrixPtr --
 *
 *	Returns a pointer to the specified matrix operator's data.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
\*--------------------------------------------------------------------------*/

tclMatrix *
Tcl_GetMatrixPtr(Tcl_Interp *interp, char *matName)
{
    Tcl_HashEntry *hPtr;

    dbug_enter("Tcl_GetMatrixPtr");

    if (!matTable_initted) { return NULL; }

    hPtr = Tcl_FindHashEntry(&matTable, matName);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "No matrix operator named \"",
			 matName, "\"", (char *) NULL);
	return NULL;
    }
    return (tclMatrix *) Tcl_GetHashValue(hPtr);
}

/*--------------------------------------------------------------------------*\
 *
 *  Tcl_MatrixInstallXtnsn --
 *
 *	Install a tclMatrix extension subcommand.
 *
 * Results:
 *	Should be 1.  Have to think about error results.
 *
 * Side effects:
 *	Enables you to install special purpose compiled code to handle
 *	custom operations on a tclMatrix.
 *
\*--------------------------------------------------------------------------*/

static tclMatrixXtnsnDescr *head = (tclMatrixXtnsnDescr *) NULL;
static tclMatrixXtnsnDescr *tail = (tclMatrixXtnsnDescr *) NULL;

int
Tcl_MatrixInstallXtnsn( char *cmd, tclMatrixXtnsnProc proc )
{
/*
 * My goodness how I hate primitive/pathetic C.  With C++ this
 * could've been as easy as:
 *     List<TclMatrixXtnsnDescr> xtnlist;
 *     xtnlist.append( tclMatrixXtnsnDescr(cmd,proc) );
 * grrrrr.
 */

    tclMatrixXtnsnDescr *new =
	(tclMatrixXtnsnDescr *) malloc(sizeof(tclMatrixXtnsnDescr));

    dbug_enter("Tcl_MatrixInstallXtnsn");

#ifdef DEBUG
    fprintf(stderr, "Installing a tclMatrix extension -> %s\n", cmd );
#endif

    new->cmd = malloc( strlen(cmd)+1 );
    strcpy( new->cmd, cmd );
    new->cmdproc = proc;
    new->next = (tclMatrixXtnsnDescr *) NULL;

    if (!head) {
	tail = head = new;
	return 1;
    } else {
	tail = tail->next = new;
	return 1;
    }

}

/*--------------------------------------------------------------------------*\
 *
 * matrixInitialize --
 *
 *	Handles matrix initialization lists.
 *	Written by Martin L. Smith.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
\*--------------------------------------------------------------------------*/

static int matrixInitialize(Tcl_Interp* interp, tclMatrix* m,
			    int dim, int offs, int nargs, char** args)
{
    static int verbose = 0;

    char** newargs;
    int numnewargs;
    int newoffs;
    int i;

    if (verbose)
	fprintf(stderr, "level %d  offset %d  args %d\n", dim, offs, nargs);

    if (dim < m->dim) {
	for (i = 0; i < nargs; i++) {
	    if (Tcl_SplitList(interp, args[i], &numnewargs, (CONST char ***) &newargs)
		!= TCL_OK) {
		Tcl_AppendResult(interp, "bad matrix initializer list form: ",
				 args[i], (char *) NULL);
		return TCL_ERROR;
	    }
	    if (dim > 0)
		newoffs = offs * m->n[dim - 1] + i;
	    else
		newoffs = 0;

	    matrixInitialize(interp, m, dim + 1, newoffs, numnewargs, newargs);
	    /* Must use Tcl_Free since allocated by Tcl */
	    Tcl_Free((char *) newargs);
	}
	return TCL_OK;
    }

    for (i = 0; i < nargs; i++) {
	newoffs = offs * m->n[dim - 1] + i;
	(m->put)((ClientData) m, interp, newoffs, args[i]);
	if (verbose)
	    fprintf(stderr, "\ta[%d] = %s\n",  newoffs, args[i]);
    }
    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 *
 * MatrixCmd --
 *
 *	When a Tcl matrix command is invoked, this routine is called.
 *
 * Results:
 *	A standard Tcl result value, usually TCL_OK.
 *	On matrix get commands, one or a number of matrix elements are
 *	printed.
 *
 * Side effects:
 *	Depends on the matrix command.
 *
\*--------------------------------------------------------------------------*/

static int
MatrixCmd(ClientData clientData, Tcl_Interp *interp,
	  int argc, char **argv)
{
    register tclMatrix *matPtr = (tclMatrix *) clientData;
    int length, put = 0;
    char c, tmp[80];
    char *name = argv[0];
    int nmin[MAX_ARRAY_DIM], nmax[MAX_ARRAY_DIM];
    int i, j, k;

/* Initialize */

    if (argc < 2) {
	Tcl_AppendResult(interp, "wrong # args, type: \"",
		argv[0], " help\" for more info", (char *) NULL);
	return TCL_ERROR;
    }

    for (i = 0; i < MAX_ARRAY_DIM; i++) {
	nmin[i] = 0;
	nmax[i] = matPtr->n[i]-1;
    }

/* First check for a matrix command */

    argc--; argv++;
    c = argv[0][0];
    length = strlen(argv[0]);

/* dump -- send a nicely formatted listing of the array contents to stdout */
/* (very helpful for debugging) */

    if ((c == 'd') && (strncmp(argv[0], "dump", length) == 0)) {
	for (i = nmin[0]; i <= nmax[0]; i++) {
	    for (j = nmin[1]; j <= nmax[1]; j++) {
		for (k = nmin[2]; k <= nmax[2]; k++) {
		    (*matPtr->get)((ClientData) matPtr, interp, I3D(i,j,k), tmp);
		    printf("%s ", tmp);
		}
		if (matPtr->dim > 2)
		    printf("\n");
	    }
	    if (matPtr->dim > 1)
		printf("\n");
	}
	printf("\n");
	return TCL_OK;
    }

/* delete -- delete the array */

    else if ((c == 'd') && (strncmp(argv[0], "delete", length) == 0)) {
#ifdef DEBUG
	fprintf(stderr, "Deleting array %s\n", name);
#endif
	Tcl_DeleteCommand(interp, name);
	return TCL_OK;
    }

/* filter */
/* Only works on 1d matrices */

    else if ((c == 'f') && (strncmp(argv[0], "filter", length) == 0)) {
	Mat_float *tmp;
	int ifilt, nfilt;

	if (argc != 2 ) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     name, " ", argv[0], " num-passes\"",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	if (matPtr->dim != 1 || matPtr->type != TYPE_FLOAT) {
	    Tcl_AppendResult(interp, "can only filter a 1d float matrix",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	nfilt = atoi(argv[1]);
	tmp = (Mat_float *) malloc((matPtr->len+2) * sizeof(Mat_float));

	for (ifilt = 0; ifilt < nfilt; ifilt++) {

	/* Set up temporary filtering array.  Use even boundary conditions. */

	    j = 0; tmp[j] = matPtr->fdata[0];
	    for (i = 0; i < matPtr->len; i++) {
		j++; tmp[j] = matPtr->fdata[i];
	    }
	    j++; tmp[j] = matPtr->fdata[matPtr->len-1];

	/* Apply 3-point binomial filter */

	    for (i = 0; i < matPtr->len; i++) {
		j = i+1;
		matPtr->fdata[i] = 0.25*( tmp[j-1] + 2*tmp[j] + tmp[j+1] );
	    }
	}

	free ((void *) tmp);
	return TCL_OK;
    }

/* help */

    else if ((c == 'h') && (strncmp(argv[0], "help", length) == 0)) {
	Tcl_AppendResult(interp,
	    "So you really thought there'd be help, eh?  Sucker.",
	    (char *) NULL);
	return TCL_OK;
    }

/* info */

    else if ((c == 'i') && (strncmp(argv[0], "info", length) == 0)) {
	for (i = 0; i < matPtr->dim; i++) {
	    sprintf(tmp, "%d", matPtr->n[i]);
	/* Must avoid trailing space. */
	    if (i < matPtr->dim - 1)
		Tcl_AppendResult(interp, tmp, " ", (char *) NULL);
	    else
		Tcl_AppendResult(interp, tmp, (char *) NULL);
	}
	return TCL_OK;
    }

/* max */

    else if ((c == 'm') && (strncmp(argv[0], "max", length) == 0)) {
	int len;
	if (argc < 1 || argc > 2 ) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     name, " ", argv[0], " ?length?\"",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	if (argc == 2)
	    len = atoi(argv[1]);
	else
	    len = matPtr->len;

	switch (matPtr->type) {
	case TYPE_FLOAT:{
	    Mat_float max = matPtr->fdata[0];
	    for (i = 1; i < len; i++)
		max = MAX(max, matPtr->fdata[i]);
	    sprintf(tmp, "%g", max);
	    Tcl_AppendResult(interp, tmp, (char *) NULL);
	    break;
	}
	case TYPE_INT:{
	    Mat_int max = matPtr->idata[0];
	    for (i = 1; i < len; i++)
		max = MAX(max, matPtr->idata[i]);
	    sprintf(tmp, "%d", max);
	    Tcl_AppendResult(interp, tmp, (char *) NULL);
	    break;
	}}
	return TCL_OK;
    }

/* min */

    else if ((c == 'm') && (strncmp(argv[0], "min", length) == 0)) {
	int len;
	if (argc < 1 || argc > 2 ) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     name, " ", argv[0], " ?length?\"",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	if (argc == 2)
	    len = atoi(argv[1]);
	else
	    len = matPtr->len;

	switch (matPtr->type) {
	case TYPE_FLOAT:{
	    Mat_float min = matPtr->fdata[0];
	    for (i = 1; i < len; i++)
		min = MIN(min, matPtr->fdata[i]);
	    sprintf(tmp, "%g", min);
	    Tcl_AppendResult(interp, tmp, (char *) NULL);
	    break;
	}
	case TYPE_INT:{
	    Mat_int min = matPtr->idata[0];
	    for (i = 1; i < len; i++)
		min = MIN(min, matPtr->idata[i]);
	    sprintf(tmp, "%d", min);
	    Tcl_AppendResult(interp, tmp, (char *) NULL);
	    break;
	}}
	return TCL_OK;
    }

/* redim */
/* Only works on 1d matrices */

    else if ((c == 'r') && (strncmp(argv[0], "redim", length) == 0)) {
	int newlen;
	void *data;

	if (argc != 2 ) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     name, " ", argv[0], " length\"",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	if (matPtr->dim != 1) {
	    Tcl_AppendResult(interp, "can only redim a 1d matrix",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	newlen = atoi(argv[1]);
	switch (matPtr->type) {
	case TYPE_FLOAT:
	    data = realloc(matPtr->fdata, newlen * sizeof(Mat_float));
	    if (data == NULL) {
		Tcl_AppendResult(interp, "redim failed!",
				 (char *) NULL);
		return TCL_ERROR;
	    }
	    matPtr->fdata = (Mat_float *) data;
	    for (i = matPtr->len; i < newlen; i++)
		matPtr->fdata[i] = 0.0;
	    break;

	case TYPE_INT:
	    data = realloc(matPtr->idata, newlen * sizeof(Mat_int));
	    if (data == NULL) {
		Tcl_AppendResult(interp, "redim failed!",
				 (char *) NULL);
		return TCL_ERROR;
	    }
	    matPtr->idata = (Mat_int *) data;
	    for (i = matPtr->len; i < newlen; i++)
		matPtr->idata[i] = 0;
	    break;
	}
	matPtr->n[0] = matPtr->len = newlen;
	return TCL_OK;
    }

/* scale */
/* Only works on 1d matrices */

    else if ((c == 's') && (strncmp(argv[0], "scale", length) == 0)) {
	Mat_float scale;

	if (argc != 2 ) {
	    Tcl_AppendResult(interp, "wrong # args: should be \"",
			     name, " ", argv[0], " scale-factor\"",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	if (matPtr->dim != 1) {
	    Tcl_AppendResult(interp, "can only scale a 1d matrix",
			     (char *) NULL);
	    return TCL_ERROR;
	}

	scale = atof(argv[1]);
	switch (matPtr->type) {
	case TYPE_FLOAT:
	    for (i = 0; i < matPtr->len; i++)
		matPtr->fdata[i] *= scale;
	    break;

	case TYPE_INT:
	    for (i = 0; i < matPtr->len; i++)
		matPtr->idata[i] *= scale;
	    break;
	}
	return TCL_OK;
    }

/* Not a "standard" command, check the extension commands. */

    {
	tclMatrixXtnsnDescr *p = head;
	for( ; p; p=p->next ) {
	    if ((c == p->cmd[0]) && (strncmp(argv[0],p->cmd,length) == 0)) {
#ifdef DEBUG
		printf( "found a match, invoking %s\n", p->cmd );
#endif
		return (*(p->cmdproc))( matPtr, interp, --argc, ++argv );
	    }
	}
    }

/* Must be a put or get.  Get array indices.  */

    if (argc < matPtr->dim) {
	Tcl_AppendResult(interp, "not enough dimensions specified for \"",
			 name, (char *) NULL);
	return TCL_ERROR;
    }
    for (i = 0; i < matPtr->dim; i++) {
	if (strcmp(argv[0], "*") == 0) {
	    nmin[i] = 0;
	    nmax[i] = matPtr->n[i]-1;
	}
	else {
	    nmin[i] = atoi(argv[0]);
	    nmax[i] = nmin[i];
	}
	if (nmin[i] < 0 || nmax[i] > matPtr->n[i]-1) {
	    sprintf(tmp, "Array index %d out of bounds: %s; max: %d\n",
		    i, argv[0], matPtr->n[i]-1);
	    Tcl_AppendResult(interp, tmp, (char *) NULL);
	    return TCL_ERROR;
	}
	argc--; argv++;
    }

/* If there is an "=" after indicies, it's a put.  Do error checking. */

    if (argc > 0) {
	put = 1;
	if (strcmp(argv[0], "=") == 0) {
	    argc--; argv++;
	    if (argc == 0) {
		Tcl_AppendResult(interp, "no value specified",
				 (char *) NULL);
		return TCL_ERROR;
	    }
	    else if (argc > 1) {
		Tcl_AppendResult(interp, "extra characters after value: \"",
				 argv[1], "\"", (char *) NULL);
		return TCL_ERROR;
	    }
	}
	else {
	    Tcl_AppendResult(interp, "extra characters after indices: \"",
			     argv[0], "\"", (char *) NULL);
	    return TCL_ERROR;
	}
    }

/* Do the get/put. */
/* The loop over all elements takes care of the multi-element cases. */

    for (i = nmin[0]; i <= nmax[0]; i++) {
	for (j = nmin[1]; j <= nmax[1]; j++) {
	    for (k = nmin[2]; k <= nmax[2]; k++) {
		if (put)
		    (*matPtr->put)((ClientData) matPtr, interp, I3D(i,j,k), argv[0]);
		else {
		    (*matPtr->get)((ClientData) matPtr, interp, I3D(i,j,k), tmp);
		    if (i == nmax[0] && j == nmax[1] && k == nmax[2])
			Tcl_AppendResult(interp, tmp, (char *) NULL);
		    else
			Tcl_AppendResult(interp, tmp, " ", (char *) NULL);
		}
	    }
	}
    }

    return TCL_OK;
}

/*--------------------------------------------------------------------------*\
 *
 * Routines to handle Matrix get/put dependent on type:
 *
 * MatrixPut_f	MatrixGet_f
 * MatrixPut_i	MatrixGet_i
 *
 * A "put" converts from string format to the intrinsic type, storing into
 * the array.
 *
 * A "get" converts from the intrinsic type to string format, storing into
 * a string buffer.
 *
\*--------------------------------------------------------------------------*/

static void
MatrixPut_f(ClientData clientData, Tcl_Interp* interp, int index, char *string)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    matPtr->fdata[index] = atof(string);
}

static void
MatrixGet_f(ClientData clientData, Tcl_Interp* interp, int index, char *string)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;
    double value = matPtr->fdata[index];

    sprintf(string, "%.12g", value);
}

static void
MatrixPut_i(ClientData clientData, Tcl_Interp* interp, int index, char *string)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    matPtr->idata[index] = atoi(string);
}

static void
MatrixGet_i(ClientData clientData, Tcl_Interp* interp, int index, char *string)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;

    sprintf(string, "%d", matPtr->idata[index]);
}

/*--------------------------------------------------------------------------*\
 *
 * DeleteMatrixVar --
 *
 *	Causes matrix command to be deleted.  Invoked when variable
 *	associated with matrix command is unset.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	See DeleteMatrixCmd.
 *
\*--------------------------------------------------------------------------*/

static char *
DeleteMatrixVar(ClientData clientData,
		Tcl_Interp *interp, char *name1, char *name2, int flags)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;
    Tcl_CmdInfo infoPtr;
    char *name;

    dbug_enter("DeleteMatrixVar");

    if (matPtr->tracing != 0) {
	matPtr->tracing = 0;
	name = (char *) malloc(strlen(matPtr->name)+1);
	strcpy(name, matPtr->name);

#ifdef DEBUG
	if (Tcl_GetCommandInfo(matPtr->interp, matPtr->name, &infoPtr)) {
	    if (Tcl_DeleteCommand(matPtr->interp, matPtr->name) == TCL_OK)
		fprintf(stderr, "Deleted command %s\n", name);
	    else
		fprintf(stderr, "Unable to delete command %s\n", name);
	}
#else
	if (Tcl_GetCommandInfo(matPtr->interp, matPtr->name, &infoPtr))
	    Tcl_DeleteCommand(matPtr->interp, matPtr->name);
#endif
	free((void *) name);
    }
    return (char *) NULL;
}

/*--------------------------------------------------------------------------*\
 *
 * DeleteMatrixCmd --
 *
 *	Releases all the resources allocated to the matrix command.
 *	Invoked just before a matrix command is removed from an interpreter.
 *
 *	Note: If the matrix has tracing enabled, it means the user
 *	explicitly deleted a non-persistent matrix.  Not a good idea,
 *	because eventually the local variable that was being traced will
 *	become unset and the matrix data will be referenced in
 *	DeleteMatrixVar.  So I've massaged this so that at worst it only
 *	causes a minor memory leak instead of imminent program death.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	All memory associated with the matrix operator is freed (usually).
 *
\*--------------------------------------------------------------------------*/

static void
DeleteMatrixCmd(ClientData clientData)
{
    tclMatrix *matPtr = (tclMatrix *) clientData;
    Tcl_HashEntry *hPtr;

    dbug_enter("DeleteMatrixCmd");

#ifdef DEBUG
    fprintf(stderr, "Freeing space associated with matrix %s\n", matPtr->name);
#endif

/* Remove hash table entry */

    hPtr = Tcl_FindHashEntry(&matTable, matPtr->name);
    if (hPtr != NULL)
	Tcl_DeleteHashEntry(hPtr);

/* Free data */

    if (matPtr->fdata != NULL) {
	free((void *) matPtr->fdata);
	matPtr->fdata = NULL;
    }
    if (matPtr->idata != NULL) {
	free((void *) matPtr->idata);
	matPtr->idata = NULL;
    }

/* Attempt to turn off tracing if possible. */

    if (matPtr->tracing) {
	if (Tcl_VarTraceInfo(matPtr->interp, matPtr->name, TCL_TRACE_UNSETS,
			     (Tcl_VarTraceProc *) DeleteMatrixVar, NULL) != NULL) {
	    matPtr->tracing = 0;
	    Tcl_UntraceVar(matPtr->interp, matPtr->name, TCL_TRACE_UNSETS,
			   (Tcl_VarTraceProc *) DeleteMatrixVar, (ClientData) matPtr);
	    Tcl_UnsetVar(matPtr->interp, matPtr->name, 0);
	}
    }

/* Free name.  */

    if (matPtr->name != NULL) {
	free((void *) matPtr->name);
	matPtr->name = NULL;
    }

/* Free tclMatrix */

    if ( ! matPtr->tracing)
	free((void *) matPtr);
#ifdef DEBUG
    else
	fprintf(stderr, "OOPS!  You just lost %d bytes\n", sizeof(tclMatrix));
#endif
}
