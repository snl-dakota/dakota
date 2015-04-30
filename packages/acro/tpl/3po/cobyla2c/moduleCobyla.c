/* Python COBYLA module */

/*
 * Copyright (c) 2004, Jean-Sebastien Roy (js@jeannot.org)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static char const rcsid[] =
  "@(#) $Jeannot: moduleCobyla.c,v 1.8 2004/04/18 14:58:18 js Exp $";

#include "Python.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cobyla.h"

typedef struct _pycobyla_state
{
  PyObject *py_function;
  int failed;
} pycobyla_state;

static cobyla_function function;
static PyObject *moduleCobyla_minimize(PyObject *self, PyObject *args);
static int PyObject_AsDouble(PyObject *py_obj, double *x);
static double *PyList_AsDoubleArray(PyObject *py_list, int *size);
static PyObject *PyDoubleArray_AsList(int size, double *x);
static int PyList_IntoDoubleArray(PyObject *py_list, double *x, int size);

int PyObject_AsDouble(PyObject *py_obj, double *x)
{
  PyObject *py_float;

  py_float = PyNumber_Float(py_obj);
  
  if (py_float == NULL) return -1;
  
  *x = PyFloat_AsDouble(py_float);

  Py_DECREF(py_float);
  return 0;
}

double *PyList_AsDoubleArray(PyObject *py_list, int *size)
{
  int i;
  double *x;
  
  if (!PyList_Check(py_list))
  {
    *size = -1;
    return NULL;
  }
  
  *size = PyList_Size(py_list);
  if (*size <= 0) return NULL;
  x = malloc((*size)*sizeof(*x));
  if (x == NULL) return NULL;
  
  for (i=0; i<(*size); i++)
  {
    PyObject *py_float = PyList_GetItem(py_list, i);
    if (py_float == NULL || PyObject_AsDouble(py_float, &(x[i])))
    {
      free(x);
      return NULL;
    }
  }
  
  return x;
}

int PyList_IntoDoubleArray(PyObject *py_list, double *x, int size)
{
  int i;
  
  if (py_list == NULL) return 1;
  
  if (!PyList_Check(py_list)) return 1;
  
  if (size != PyList_Size(py_list)) return 1;
  
  for (i=0; i<size; i++)
  {
    PyObject *py_float = PyList_GetItem(py_list, i);
    if (py_float == NULL || PyObject_AsDouble(py_float, &(x[i])))
      return 1;
  }
  
  return 0;
}

PyObject *PyDoubleArray_AsList(int size, double *x)
{
  int i;
  PyObject *py_list;
  
  py_list = PyList_New(size);
  if (py_list == NULL) return NULL;
  
  for (i=0; i<size; i++)
  {
    PyObject *py_float;
    py_float = PyFloat_FromDouble(x[i]);
    if (py_float == NULL || PyList_SetItem(py_list, i, py_float))
    {
      Py_DECREF(py_list);
      return NULL;
    }
  }
  
  return py_list;
}

int function(int n, int m, double *x, double *f, double *con, void *state)
{
  PyObject *py_list, *arglist, *py_con, *result = NULL;
  pycobyla_state *py_state = (pycobyla_state *)state;

  py_list = PyDoubleArray_AsList(n, x);
  if (py_list == NULL)
  {
    PyErr_SetString(PyExc_MemoryError, "cobyla: memory allocation failed.");
    goto failure;
  }

  arglist = Py_BuildValue("(N)", py_list);
  result = PyEval_CallObject(py_state->py_function, arglist);
  Py_DECREF(arglist);

  if (result == NULL)
    goto failure;

  if (result == Py_None)
  {
    Py_DECREF(result);
    return 1;
  }

  if (PyTuple_Check(result))
  {
    if (!PyArg_ParseTuple(result, "dO!", f, &PyList_Type, &py_con))
    {
      PyErr_SetString(PyExc_ValueError,
        "cobyla: invalid return value from minimized function.");
      goto failure;
    }

    if (PyList_IntoDoubleArray(py_con, con, m))
    {
      PyErr_SetString(PyExc_ValueError,
        "cobyla: invalid return value from minimized function.");
      goto failure;
    }
  }
  else
  {
    if (PyObject_AsDouble(result, f))
    {
      PyErr_SetString(PyExc_ValueError,
        "cobyla: invalid return value from minimized function.");
      goto failure;
    }
  }

  Py_DECREF(result);

  return 0;
    
failure:
  py_state->failed = 1;
  Py_XDECREF(result);
  return 1;
}

PyObject *moduleCobyla_minimize(PyObject *self, PyObject *args)
{
  PyObject *py_x0, *py_list;
  PyObject *py_function = NULL;
  pycobyla_state py_state;
  int n, m;

  int rc, iprint, maxfun;
  double *x;
  double rhobeg, rhoend;
    
  if (!PyArg_ParseTuple(args, "OO!iddii",
    &py_function,
    &PyList_Type, &py_x0,
    &m, &rhobeg, &rhoend, &iprint, &maxfun))
    return NULL;

  if (!PyCallable_Check(py_function))
  {
    PyErr_SetString(PyExc_TypeError, "cobyla: function must be callable.");
    return NULL;
  }
    
  x = PyList_AsDoubleArray(py_x0, &n);
  if (n != 0 && x == NULL)
  {
    if (x) free(x);
    
    PyErr_SetString(PyExc_ValueError, "cobyla: invalid initial vector.");
    return NULL;
  }
    
  py_state.py_function = py_function;
  py_state.failed = 0;

  Py_INCREF(py_function);
  
  rc = cobyla(n, m, x, rhobeg, rhoend, iprint, &maxfun, 0,function, &py_state);

  Py_DECREF(py_function);

  if (py_state.failed)
  {
    if (x) free(x);
    return NULL;
  }

  if (rc == COBYLA_ENOMEM)
  {
    PyErr_SetString(PyExc_MemoryError, "cobyla: memory allocation failed.");
    if (x) free(x);
    return NULL;
  }

  if (rc == COBYLA_EINVAL)
  {
    PyErr_SetString(PyExc_ValueError, "cobyla: M<0.");
    if (x) free(x);
    return NULL;
  }

  py_list = PyDoubleArray_AsList(n, x);
  if (x) free(x);
  if (py_list == NULL)
  {
    PyErr_SetString(PyExc_MemoryError, "cobyla: memory allocation failed.");
    return NULL;
  }

  return Py_BuildValue("(iiN)", rc, maxfun, py_list);
}

static PyMethodDef moduleCobyla_methods[] =
{
  {"minimize", moduleCobyla_minimize, METH_VARARGS},
  {NULL, NULL}
};

void initmoduleCobyla(void)
{
  (void) Py_InitModule("moduleCobyla", moduleCobyla_methods);
}
