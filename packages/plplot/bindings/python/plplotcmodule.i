%module plplotc

/* 
Copyright (C) 2002  Gary Bishop
Copyright (C) 2004  Alan W. Irwin
Copyright (C) 2004  Andrew Ross

This file is part of PLplot.

PLplot is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; version 2 of the License.

PLplot is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with the file PLplot; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
*/

/* 
A SWIG interface to PLplot for Python. This wrapper is different from
the one supplied in plmodule.c (now stored in the "old" subdirectory of
the top-level build tree for reference purposes) in that:

   1) it strictly provides the C-API with the usual change of not
      requiring lengths for arrays,

   2) it attempts to provide the entire API including callbacks for
      plcont and plshade.
      
   3) it works both with the single and double-precision versions of the
      PLplot library.

This is known to work with swig-1.3.17 (versions prior to 1.3.14 *known* not
to work, 1.3.14-1.3.16 might work but you are on your own).  The resulting
interface works for both python-1.5.2 (RedHat 7.3) and python-2.1.3 (Debian
woody).

*/

%include typemaps.i

%{
/* Change this to the recommended
#include <Numeric/arrayobject.h> 
 once we no longer support python1.5 */
#include <arrayobject.h>        
#include "plplot.h"      
#include "plplotP.h"

#ifdef PL_DOUBLE
#define  PyArray_PLFLT PyArray_DOUBLE
#else
#define  PyArray_PLFLT PyArray_FLOAT
#endif

#define  PyArray_PLINT PyArray_LONG
/* python-1.5 compatibility mode? */
#define PySequence_Fast_GET_ITEM PySequence_GetItem
#define PySequence_Size PySequence_Length
%}

#ifdef PL_DOUBLE
typedef double PLFLT;
#else
typedef float PLFLT;
#endif

typedef long PLINT;
typedef unsigned int PLUNICODE;

/* We have to get import_array called in our extension before we can use Numeric */
%init %{
  import_array();
  %}

/* I hate global variables but this is the best way I can think of to manage consistency
   checking among function arguments. */
%{
  static PLINT Alen = 0;
  static PLINT Xlen = 0, Ylen = 0;
  %}

/* The following typemaps take care of marshaling values into and out of PLplot functions. The
Array rules are trickly because of the need for length checking. These rules manage
some global variables (above) to handle consistency checking amoung parameters. 

Naming rules:
	Array 		(sets Alen to dim[0])
	ArrayCk 	(tests that dim[0] == Alen)
	ArrayX 		(sets Xlen to dim[0]
	ArrayCkX 	(tests dim[0] == Xlen)
	ArrayY 		(sets Ylen to dim[1])
	ArrayCkY 	(tests dim[1] == Ylen)
	Matrix 		(sets Xlen to dim[0], Ylen to dim[1])
	MatrixCk 	(test Xlen == dim[0] && Ylen == dim[1])
*/

/**********************************************************************************
			 PLINT arrays
**********************************************************************************/

/* With preceding count */
%typemap(in) (PLINT n, PLINT *Array) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)PyArray_ContiguousFromObject($input, PyArray_PLINT, 1, 1);
  if(tmp == NULL) return NULL;
  $1 = Alen = tmp->dimensions[0];
  $2 = (PLINT*)tmp->data;
}
%typemap(freearg) (PLINT n, PLINT *Array) {Py_DECREF(tmp$argnum);}

/* Trailing count and check consistency with previous */
%typemap(in) (PLINT *ArrayCk, PLINT n) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)PyArray_ContiguousFromObject($input, PyArray_PLINT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Alen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $2 = tmp->dimensions[0];
  $1 = (PLINT*)tmp->data;
}
%typemap(freearg) (PLINT *ArrayCk, PLINT n) {Py_DECREF(tmp$argnum); }

/* No count but check consistency with previous */
%typemap(in) PLINT *ArrayCk (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)PyArray_ContiguousFromObject($input, PyArray_PLINT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Alen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $1 = (PLINT*)tmp->data;
}
%typemap(freearg) PLINT *ArrayCk { Py_DECREF(tmp$argnum);}

/* Weird case to allow argument to be one shorter than others */
%typemap(in) PLINT *ArrayCkMinus1 (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)PyArray_ContiguousFromObject($input, PyArray_PLINT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] < Alen-1) {
    PyErr_SetString(PyExc_ValueError, "Vector must be at least length of others minus 1.");
    return NULL;
  }
  $1 = (PLINT*)tmp->data;
}
%typemap(freearg) PLINT *ArrayCkMinus1 { Py_DECREF(tmp$argnum);}

/* No length but remember size to check others */
%typemap(in) PLINT *Array (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)PyArray_ContiguousFromObject($input, PyArray_PLINT, 1, 1);
  if(tmp == NULL) return NULL;
  Alen = tmp->dimensions[0];
  $1 = (PLINT*)tmp->data;
}
%typemap(freearg) (PLINT *Array) {Py_DECREF(tmp$argnum);}

/******************************************************************************
				 PLFLT Arrays 
******************************************************************************/

#ifndef PL_DOUBLE
%wrapper %{
/* some really twisted stuff to allow calling a single precision library from python */
PyArrayObject* myArray_ContiguousFromObject(PyObject* in, int type, int mindims, int maxdims)
{
  PyArrayObject* tmp = (PyArrayObject*)PyArray_ContiguousFromObject(in, PyArray_FLOAT,
								    mindims, maxdims);
  if (!tmp) {
    /* could be an incoming double array which can't be "safely" converted, do it anyway */
    if(PyArray_Check(in)) {
      PyErr_Clear();
      tmp = (PyArrayObject*)PyArray_Cast((PyArrayObject*)in, PyArray_FLOAT);
    }
  }
  return tmp;
}
 %}
#else
%wrapper %{
#define myArray_ContiguousFromObject PyArray_ContiguousFromObject
  %}
#endif

/* with preceding count */
%typemap(in) (PLINT n, PLFLT *Array) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  $1 = Alen = tmp->dimensions[0];
  $2 = (PLFLT*)tmp->data;
}
%typemap(freearg) (PLINT n, PLFLT *Array) { Py_DECREF(tmp$argnum);}

/* trailing count and check consistency with previous */
%typemap(in) (PLFLT *ArrayCk, PLINT n) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Alen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $1 = (PLFLT*)tmp->data;
  $2 = tmp->dimensions[0];
}
%typemap(freearg) (PLFLT *ArrayCk, PLINT n) { Py_DECREF(tmp$argnum);}

/* no count, but check consistency with previous */
%typemap(in) PLFLT *ArrayCk (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Alen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) PLFLT *ArrayCk { Py_DECREF(tmp$argnum);}

/* check consistency with X dimension of previous */
%typemap(in) PLFLT *ArrayCkX (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Xlen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) PLFLT *ArrayCkX { Py_DECREF(tmp$argnum);}

/* check consistency with Y dimension of previous */
%typemap(in) PLFLT *ArrayCkY (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Ylen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must be same length.");
    return NULL;
  }
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) PLFLT *ArrayCkY { Py_DECREF(tmp$argnum);}

/* set X length for later consistency checking */
%typemap(in) PLFLT *ArrayX (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  Xlen = tmp->dimensions[0];
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) PLFLT *ArrayX {Py_DECREF(tmp$argnum);}

/* set Y length for later consistency checking */
%typemap(in) PLFLT *ArrayY (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  Ylen = tmp->dimensions[0];
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) (PLFLT *ArrayY) {Py_DECREF(tmp$argnum);}

/* with trailing count */
%typemap(in) (PLFLT *Array, PLINT n) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  $2 = tmp->dimensions[0];
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) (PLFLT *Array, PLINT n) {Py_DECREF(tmp$argnum); }

/* with no count */
%typemap(in) PLFLT *Array (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 1, 1);
  if(tmp == NULL) return NULL;
  Alen = tmp->dimensions[0];
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) PLFLT *Array { Py_DECREF(tmp$argnum);}

/* 2D array with trailing dimensions, check consistency with previous */
%typemap(in) (PLFLT **MatrixCk, PLINT nx, PLINT ny) (PyArrayObject* tmp) {
  int i, size;
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 2, 2);
  if(tmp == NULL) return NULL;
  if(Xlen != tmp->dimensions[0] || Ylen != tmp->dimensions[1]) {
    PyErr_SetString(PyExc_ValueError, "Vectors must match matrix.");
    return NULL;
  }
  $2 = tmp->dimensions[0];
  $3 = tmp->dimensions[1];
  size = sizeof(PLFLT)*$3;
  $1 = (PLFLT**)malloc(sizeof(PLFLT*)*$2);
  for(i=0; i<$2; i++)
    $1[i] = (PLFLT*)(tmp->data + i*size);
}
%typemap(freearg) (PLFLT **MatrixCk, PLINT nx, PLINT ny) {
  Py_DECREF(tmp$argnum);
  free($1);
}

/* 2D array with trailing dimensions, set the X, Y size for later checking */
%typemap(in) (PLFLT **Matrix, PLINT nx, PLINT ny) (PyArrayObject* tmp) {
  int i, size;
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 2, 2);
  if(tmp == NULL) return NULL;
  Xlen = $2 = tmp->dimensions[0];
  Ylen = $3 = tmp->dimensions[1];
  size = sizeof(PLFLT)*$3;
  $1 = (PLFLT**)malloc(sizeof(PLFLT*)*$2);
  for(i=0; i<$2; i++)
    $1[i] = (PLFLT*)(tmp->data + i*size);
}
%typemap(freearg) (PLFLT **Matrix, PLINT nx, PLINT ny) {
  Py_DECREF(tmp$argnum);
  free($1);
}

/* 2D array with no dimensions, set the X, Y size for later checking */
%typemap(in) PLFLT **Matrix (PyArrayObject* tmp) {
  int i, size;
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 2, 2);
  if(tmp == NULL) return NULL;
  Xlen = tmp->dimensions[0];
  Ylen = tmp->dimensions[1];
  size = sizeof(PLFLT)*Ylen;
  $1 = (PLFLT**)malloc(sizeof(PLFLT*)*Xlen);
  for(i=0; i<Xlen; i++)
    $1[i] = (PLFLT*)(tmp->data + i*size);
}
%typemap(freearg) PLFLT **Matrix {
  Py_DECREF(tmp$argnum);
  free($1);
}

/* for plshade1, note the difference in the type for the first arg */
%typemap(in) (PLFLT *Matrix, PLINT nx, PLINT ny) (PyArrayObject* tmp) {
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 2, 2);
  if(tmp == NULL) return NULL;
  Xlen = $2 = tmp->dimensions[0];
  Ylen = $3 = tmp->dimensions[1];
  $1 = (PLFLT*)tmp->data;
}
%typemap(freearg) (PLFLT *Matrix, PLINT nx, PLINT ny) {
  Py_DECREF(tmp$argnum);
}

/* 2D array, check for consistency */
%typemap(in) PLFLT **MatrixCk (PyArrayObject* tmp) {
  int i, size;
  tmp = (PyArrayObject *)myArray_ContiguousFromObject($input, PyArray_PLFLT, 2, 2);
  if(tmp == NULL) return NULL;
  if(tmp->dimensions[0] != Xlen || tmp->dimensions[1] != Ylen) {
    PyErr_SetString(PyExc_ValueError, "Vectors must match matrix.");
    return NULL;
  }
  size = sizeof(PLFLT)*Ylen;
  $1 = (PLFLT**)malloc(sizeof(PLFLT*)*Xlen);
  for(i=0; i<Xlen; i++)
    $1[i] = (PLFLT*)(tmp->data + i*size);
}
%typemap(freearg) PLFLT **MatrixCk {
  Py_DECREF(tmp$argnum);
  free($1);
}

/***************************
	String returning functions
****************************/

/* This currently just used for plgdev, plgfnam, and plgver which apparently
 * have a limit of 80 bytes.  But to (hopefully) be safe for any future use
 * have a 1000 byte limit here. */
%typemap(in, numinputs=0) char *OUTPUT ( char buff[1000] ) {
  $1 = buff;
}
%typemap(argout,fragment="t_output_helper") char *OUTPUT {
   PyObject *o = PyString_FromString($1);
   $result = t_output_helper($result,o);
}

/***************************
	A trick for docstrings
****************************/

%define DOC(func, string) 
%wrapper %{#define _doc_ ## func string %}
%enddef

/* All code associated with special call-back functions. */
/* Identity transformation. */

%typemap(in, numinputs=0) PLPointer IGNORE {
  $1 = NULL;
}
void
pltr0(PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLPointer IGNORE);

/* This typemap takes care of converting a Python 2-tuple of 1D Arrays to the
   PLcGrid structure that pltr1 expects */
%wrapper %{
  PyArrayObject *pltr_xg, *pltr_yg;
  static PLcGrid tmpGrid1;
  static PLcGrid2 tmpGrid2;

  PLcGrid* marshal_PLcGrid1(PyObject* input) {
    /* fprintf(stderr, "marshal PLcGrid1\n"); */
    if(!PySequence_Check(input) || PySequence_Size(input) != 2) {
      PyErr_SetString(PyExc_ValueError, "Expected a sequence of two arrays.");
      return NULL;
    }
    pltr_xg = (PyArrayObject*)myArray_ContiguousFromObject(PySequence_Fast_GET_ITEM(input, 0),
							   PyArray_PLFLT, 1, 1);
    pltr_yg = (PyArrayObject*)myArray_ContiguousFromObject(PySequence_Fast_GET_ITEM(input, 1), 
							   PyArray_PLFLT, 1, 1);
    if(pltr_xg == 0 || pltr_yg == 0) {
      PyErr_SetString(PyExc_ValueError, "Expected a sequence to two 1D arrays.");
      return NULL;
    }
    tmpGrid1.nx = pltr_xg->dimensions[0];
    tmpGrid1.ny = pltr_yg->dimensions[0];
    if(Xlen != tmpGrid1.nx || Ylen != tmpGrid1.ny) {
      PyErr_SetString(PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg.");
      return NULL;
    }
    tmpGrid1.xg = (PLFLT*)pltr_xg->data;
    tmpGrid1.yg = (PLFLT*)pltr_yg->data;
    return &tmpGrid1;
  }

  void cleanup_PLcGrid1(void) {
    /* fprintf(stderr, "cleanup PLcGrid1\n"); */
    Py_DECREF(pltr_xg);
    Py_DECREF(pltr_yg);
  }

  PLcGrid2* marshal_PLcGrid2(PyObject* input) {
    int i, size;
    /* fprintf(stderr, "marshal PLcGrid2\n"); */
    if(!PySequence_Check(input) || PySequence_Size(input) != 2) {
      PyErr_SetString(PyExc_ValueError, "Expected a sequence of two arrays.");
      return NULL;
    }
    pltr_xg = (PyArrayObject*)myArray_ContiguousFromObject(PySequence_Fast_GET_ITEM(input, 0),
							   PyArray_PLFLT, 2, 2);
    pltr_yg = (PyArrayObject*)myArray_ContiguousFromObject(PySequence_Fast_GET_ITEM(input, 1), 
							   PyArray_PLFLT, 2, 2);
    if(pltr_xg == 0 || pltr_yg == 0) {
      PyErr_SetString(PyExc_ValueError, "Expected a sequence of two 2D arrays.");
      return NULL;
    }
    if(pltr_xg->dimensions[0] != pltr_yg->dimensions[0] ||
       pltr_xg->dimensions[1] != pltr_yg->dimensions[1]) {
      PyErr_SetString(PyExc_ValueError, "Arrays must be same size.");
      return NULL;
    }
    tmpGrid2.nx = pltr_xg->dimensions[0];
    tmpGrid2.ny = pltr_xg->dimensions[1];
    if(Xlen != tmpGrid2.nx || Ylen != tmpGrid2.ny) {
      PyErr_SetString(PyExc_ValueError, "pltr arguments must have X and Y dimensions of first arg.");
      return NULL;
    }
    size = sizeof(PLFLT)*tmpGrid2.ny;
    tmpGrid2.xg = (PLFLT**)malloc(sizeof(PLFLT*)*tmpGrid2.nx);
    for(i=0; i<tmpGrid2.nx; i++)
      tmpGrid2.xg[i] = (PLFLT*)(pltr_xg->data + i*size);
    tmpGrid2.yg = (PLFLT**)malloc(sizeof(PLFLT*)*tmpGrid2.nx);
    for(i=0; i<tmpGrid2.nx; i++)
      tmpGrid2.yg[i] = (PLFLT*)(pltr_yg->data + i*size);
    return &tmpGrid2;
  }

  void cleanup_PLcGrid2(void) {
    /* fprintf(stderr, "cleanup PLcGrid2\n"); */
    free(tmpGrid2.xg);
    free(tmpGrid2.yg);
    Py_DECREF(pltr_xg);
    Py_DECREF(pltr_yg);
  }
  %}

%typemap(in) PLcGrid* cgrid {
  $1 = marshal_PLcGrid1($input);
  if(!$1)
    return NULL;
}
%typemap(freearg) PLcGrid* cgrid {
  cleanup_PLcGrid1();
}

/* Does linear interpolation from singly dimensioned coord arrays. */
void
pltr1(PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLcGrid* cgrid);

/* This typemap marshals a Python 2-tuple of 2D arrays into the PLcGrid2
   structure that pltr2 expects */

%typemap(in) PLcGrid2* cgrid {
  $1 = marshal_PLcGrid2($input);
  if(!$1)
    return NULL;
}
%typemap(freearg) PLcGrid2* cgrid {
  cleanup_PLcGrid2();
}

/* Does linear interpolation from doubly dimensioned coord arrays */
/* (column dominant, as per normal C 2d arrays). */
void
pltr2(PLFLT x, PLFLT y, PLFLT *OUTPUT, PLFLT *OUTPUT, PLcGrid2* cgrid);

typedef PLINT (*defined_func)(PLFLT, PLFLT);
typedef void (*fill_func)(PLINT, PLFLT*, PLFLT*);
typedef void (*pltr_func)(PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer);
typedef PLFLT (*f2eval_func)(PLINT, PLINT, PLPointer);

%{
typedef PLINT (*defined_func)(PLFLT, PLFLT);
typedef void (*fill_func)(PLINT, PLFLT*, PLFLT*);
typedef void (*pltr_func)(PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer);
typedef PLFLT (*f2eval_func)(PLINT, PLINT, PLPointer);
  %}

#if 0
/* We need to get this PyThreadState structure initialized to use it for controlling
   threads on the callback. Probably not *really* necessary since I'm not allowing
   threads through the call */

%init %{
  save_interp = PyThreadState_Get()->interp;
  %}

#endif

%wrapper %{

  /* helper code for handling the callback */
#if 0
 static PyInterpreterState *save_interp = NULL;
#endif
 enum callback_type { CB_0, CB_1, CB_2, CB_Python } pltr_type;
 PyObject* python_pltr = NULL;
 PyObject* python_f2eval = NULL;

#if 0
#define MY_BLOCK_THREADS { \
	PyThreadState *prev_state, *new_state; \
	/* need to have started a thread at some stage */ \
	/* for the following to work */ \
	PyEval_AcquireLock(); \
	new_state = PyThreadState_New(save_interp); \
	prev_state = PyThreadState_Swap(new_state);
#define MY_UNBLOCK_THREADS \
	new_state = PyThreadState_Swap(prev_state); \
	PyThreadState_Clear(new_state); \
	PyEval_ReleaseLock(); \
	PyThreadState_Delete(new_state); \
}
#else
#define MY_BLOCK_THREADS
#define MY_UNBLOCK_THREADS
#endif

/* This is the callback that gets handed to the C code. It, in turn, calls the Python callback */

  void do_pltr_callback(PLFLT x, PLFLT y, PLFLT *tx, PLFLT *ty, PLPointer data)
    {
      PyObject *pdata, *arglist, *result;
      PyArrayObject *tmp;

      /* the data argument is acutally a pointer to a python object */
      pdata = (PyObject*)data;
      if(data == NULL) {
	pdata = Py_None;
      }
      if(python_pltr) { /* if not something is terribly wrong */
	/* hold a reference to the data object */
	Py_XINCREF(pdata);
	/* grab the Global Interpreter Lock to be sure threads don't mess us up */
	MY_BLOCK_THREADS
	/* build the argument list */
#ifdef PL_DOUBLE
	arglist = Py_BuildValue("(ddO)", x, y, pdata);
#else
	arglist = Py_BuildValue("(ffO)", x, y, pdata);
#endif
	if(arglist == NULL) {
	  fprintf(stderr, "Py_BuildValue failed to make argument list.\n");
	  *tx = *ty = 0;
	  return;
	}
	/* call the python function */
	result = PyEval_CallObject(python_pltr, arglist);
	/* release the argument list */
	Py_XDECREF(arglist);
	/* check and unpack the result */
	if(result == NULL) {
	  fprintf(stderr, "call to python pltr function with 3 arguments failed\n");
	  PyErr_SetString(PyExc_RuntimeError, "pltr callback must take 3 argments.");
	  *tx = *ty = 0;
	} else {
	  tmp = (PyArrayObject *)myArray_ContiguousFromObject(result, PyArray_PLFLT, 1, 1);
	  if(tmp == 0 || tmp->dimensions[0] != 2) {
	    fprintf(stderr, "pltr callback must return a 2 element array or sequence\n");
	    PyErr_SetString(PyExc_RuntimeError, "pltr callback must return a 2-sequence.");
	    *tx = *ty = 0;
	  } else {
	    PLFLT* t = (PLFLT*)tmp->data;
	    *tx = t[0];
	    *ty = t[1];
	    Py_XDECREF(tmp);
	  }
	}
	/* release the result */
	Py_XDECREF(result);
	/* release the global interpreter lock */
	MY_UNBLOCK_THREADS
      }	
    }

  PLFLT do_f2eval_callback(PLINT x, PLINT y, PLPointer data)
    {
      PyObject *pdata, *arglist, *result;
      PLFLT fresult = 0.0;

      /* the data argument is acutally a pointer to a python object */
      pdata = (PyObject*)data;
      if(python_f2eval) { /* if not something is terribly wrong */
	/* hold a reference to the data object */
	Py_XINCREF(pdata);
	/* grab the Global Interpreter Lock to be sure threads don't mess us up */
	MY_BLOCK_THREADS
	/* build the argument list */
	arglist = Py_BuildValue("(iiO)", x, y, pdata);
	/* call the python function */
	result = PyEval_CallObject(python_pltr, arglist);
	/* release the argument list */
	Py_DECREF(arglist);
	/* check and unpack the result */
	if(!PyFloat_Check(result)) {
	  fprintf(stderr, "f2eval callback must return a float\n");
	  PyErr_SetString(PyExc_RuntimeError, "f2eval callback must return a float.");
	} else {
	  /* should I test the type here? */
	  fresult = (PLFLT)PyFloat_AsDouble(result);
	}
	/* release the result */
	Py_XDECREF(result);
	/* release the global interpreter lock */
	MY_UNBLOCK_THREADS
      }	
      return fresult;
    }

/* marshal the pltr function pointer argument */
  pltr_func marshal_pltr(PyObject* input) {
    pltr_func result = do_pltr_callback;
    PyObject* rep = PyObject_Repr(input);
    if(rep) {
      char* str = PyString_AsString(rep);
      if(strcmp(str, "<built-in function pltr0>") == 0) {
	result = pltr0;
	pltr_type = CB_0;
        python_pltr = NULL;
      } else if(strcmp(str, "<built-in function pltr1>") == 0) {
	result = pltr1;
	pltr_type = CB_1;
        python_pltr = NULL;
      } else if(strcmp(str, "<built-in function pltr2>") == 0) {
	result = pltr2;
	pltr_type = CB_2;
        python_pltr = NULL;
      } else {
	python_pltr = input;
	pltr_type = CB_Python;
	Py_XINCREF(input);
      }
      Py_DECREF(rep);
    } else {
      python_pltr = input;
      pltr_type = CB_Python;
      Py_XINCREF(input);
    }
    return result;
  }

  void cleanup_pltr(void) {
    Py_XDECREF(python_pltr);
    python_pltr = 0;
  }

  PLPointer marshal_PLPointer(PyObject* input) {
    PLPointer result = NULL;
    switch(pltr_type) {
    case CB_0:
      break;
    case CB_1:
      if(input != Py_None)
	result = marshal_PLcGrid1(input);
      break;
    case CB_2:
      if(input != Py_None)
	result = marshal_PLcGrid2(input);
      break;
    case CB_Python:
      Py_XINCREF(input);
      result = (PLPointer*)input;
      break;
    default:
      fprintf(stderr, "pltr_type is invalid\n");
    }
    return result;
  }

  void cleanup_PLPointer(void) {
    switch(pltr_type) {
    case CB_0:
      break;
    case CB_1:
      cleanup_PLcGrid1();
      break;
    case CB_2:
      cleanup_PLcGrid2();
      break;
    case CB_Python:
      Py_XDECREF(python_pltr);
      break;
    default:
      fprintf(stderr, "pltr_type is invalid\n");
    }
    python_pltr = 0;
    pltr_type = CB_0;
  }
      
      
  %}
     
%typemap(in) pltr_func pltr {
  /* it must be a callable */
  if(!PyCallable_Check((PyObject*)$input)) {
    PyErr_SetString(PyExc_ValueError, "pltr argument must be callable");
    return NULL;
  }
  $1 = marshal_pltr($input);
}
%typemap(freearg) pltr_func pltr {
  cleanup_pltr();
}

/* you can omit the pltr func */
%typemap(default) pltr_func pltr {
  python_pltr = 0;
  $1 = NULL;
}

/* convert an arbitrary Python object into the void* pointer they want */
%typemap(in) PLPointer PYOBJECT_DATA {
  if($input == Py_None)
    $1 = NULL;
  else {
    $1 = marshal_PLPointer($input);
  }
}
%typemap(freearg) PLPointer PYOBJECT_DATA {
  cleanup_PLPointer();
}

/* you can omit the data too */
%typemap(default) PLPointer PYOBJECT_DATA {
  $1 = NULL;
}

/* marshall the f2eval function pointer argument */
%typemap(in) f2eval_func f2eval {
  /* it must be a callable */
  if(!PyCallable_Check((PyObject*)$input)) {
    PyErr_SetString(PyExc_ValueError, "pltr argument must be callable");
    return NULL;
  }
  /* hold a reference to it */
  Py_XINCREF((PyObject*)$input);
  python_f2eval = (PyObject*)$input;
  /* this function handles calling the python function */
  $1 = do_f2eval_callback;
}
%typemap(freearg) f2eval_func f2eval {
  Py_XDECREF(python_f2eval);
  python_f2eval = 0;
}
%typemap(in, numinputs=0) defined_func df {
  $1 = NULL;
}
%typemap(in, numinputs=0) fill_func ff {
  $1 = plfill;
}
/* this typemap takes a sequence of strings and converts them for plstripc 
   also checks that previous Arrays were of length 4 
*/
%typemap(in) char *legline[4] {
  int i;
  if(!PySequence_Check($input) || PySequence_Size($input) != 4) {
    PyErr_SetString(PyExc_ValueError, "Requires a sequence of 4 strings.");
    return NULL;
  }
  if(Alen != 4) {
    PyErr_SetString(PyExc_ValueError, "colline and styline args must be length 4.");
    return NULL;
  }
  $1 = malloc(sizeof(char*)*4);
  for(i=0; i<4; i++) {
    $1[i] = PyString_AsString(PySequence_Fast_GET_ITEM($input, i));
    if($1[i] == NULL) {
      free($1);
      return NULL;
    }
  }
}
%typemap(freearg) char *legline[4] {
  free($1);
}

/* End of all code associated with special call-back functions.*/

/* Process options list using current options info. */
%typemap(in) (int *p_argc, char **argv) (int tmp) {
  int i;
  if (!PyList_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expecting a list");
    return NULL;
  }
  tmp = PyList_Size($input);
  $1 = &tmp;
  $2 = (char **) malloc((tmp+1)*sizeof(char *));
  for (i = 0; i < tmp; i++) {
    PyObject *s = PyList_GetItem($input,i);
    if (!PyString_Check(s)) {
        free($2);
        PyErr_SetString(PyExc_ValueError, "List items must be strings");
        return NULL;
    }
    $2[i] = PyString_AsString(s);
  }
  $2[i] = 0;
}

%typemap(freearg) (int *p_argc, char **argv) {
   if ($2) free($2);
}


%typemap(in) PLGraphicsIn *gin (PLGraphicsIn tmp) {
  if(!PySequence_Check($input) || PySequence_Size($input) != 2) {
    PyErr_SetString(PyExc_ValueError, "Expecting a sequence of 2 numbers.");
    return NULL;
  }
  $1 = &tmp;
  $1->dX = PyFloat_AsDouble(PySequence_Fast_GET_ITEM($input, 0));
  $1->dY = PyFloat_AsDouble(PySequence_Fast_GET_ITEM($input, 1));
}
%typemap(argout) PLGraphicsIn *gin {
  PyObject *o;
  o = PyFloat_FromDouble($1->wX);
  resultobj = t_output_helper(resultobj, o);
  o = PyFloat_FromDouble($1->wY);
  resultobj = t_output_helper(resultobj, o);
}
/* swig compatible PLplot API definitions from here on. */
%include plplotcapi.i
