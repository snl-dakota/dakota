/* cplplotcanvasmodule - C python wrapper for the plplotcanvas

  Copyright (C) 2004, 2005 Thomas J. Duck
  All rights reserved.

  Thomas J. Duck <tom.duck@dal.ca>
  Department of Physics and Atmospheric Science,
  Dalhousie University, Halifax, Nova Scotia, Canada, B3H 3J5


NOTICE

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA 
*/

#include <pygobject.h>

#define PY_ARRAY_UNIQUE_SYMBOL plplotcanvasapi
#include "Numeric/arrayobject.h"
 
void cplplotcanvas_register_classes (PyObject *d); 
extern PyMethodDef cplplotcanvas_functions[];
 
DL_EXPORT(void)
initcplplotcanvas(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("cplplotcanvas",cplplotcanvas_functions);
    d = PyModule_GetDict (m);
 
    cplplotcanvas_register_classes (d);

    import_array();
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module cplplotcanvas");
    }
}
