/*
Copyright (C) 2002  Gary Bishop
Copyright (C) 2002, 2004  Alan W. Irwin
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
A SWIG interface to PLplot for Java. This wrapper does the following:

   1) it strictly provides the C-API with the usual change of not
      requiring lengths for arrays,

   2) it attempts to provide the entire API *excluding* callbacks for
      plcont and plshade(s) (for now).

   3) it works both with the single and double-precision versions of the
      PLplot library.

This is known to work with swig-1.3.21.

*/
%module plplotjavac
%include typemaps.i

%{
#include "plplotP.h"
%}

#ifdef PL_DOUBLE
typedef double PLFLT;
#else
typedef float PLFLT;
#endif

typedef long PLINT;
typedef unsigned int PLUNICODE;

/***************************
	A trick for docstrings
****************************/

%define DOC(func, string)
%wrapper %{#define _doc_ ## func string %}
%enddef

/* Infrastructure for handling swig compatible plplot API definitions. */

#ifdef PL_DOUBLE
#define setup_array_1d_PLFLT setup_array_1d_d
#define setup_array_2d_PLFLT setup_array_2d_d
#define jPLFLTArray "jdoubleArray"
#define jPLFLTbracket "double[]"
#define jPLFLTbracket2 "double[][]"
#define GetPLFLTArrayElements GetDoubleArrayElements
#define ReleasePLFLTArrayElements ReleaseDoubleArrayElements
#define jPLFLT jdouble
#else
#define setup_array_1d_PLFLT setup_array_1d_f
#define setup_array_2d_PLFLT setup_array_2d_f
#define jPLFLTArray "jfloatArray"
#define jPLFLTbracket "float[]"
#define jPLFLTbracket2 "float[][]"
#define GetPLFLTArrayElements GetFloatArrayElements
#define ReleasePLFLTArrayElements ReleaseFloatArrayElements
#define jPLFLT jfloat
#endif

%{
/*---------------------------------------------------------------------------
 * Array allocation & copy helper routines.  Note because of swig limitations
 * it is necessary to release the java array memory right after calling these
 * routines.  Thus it is necessary to allocate and copy the arrays  even if
 * the java and plplot arrays are of the same type.  Note, because of this
 * change to Geoffrey's original versions, caller must always free memory
 * afterwards.  Thus, the must_free_buffers logic is gone as well.
 *---------------------------------------------------------------------------*/

/* 1d array of jints */

static void
setup_array_1d_i( PLINT **pa, jint *adat, int n )
{
   int i;
   *pa = (PLINT *) malloc( n * sizeof(PLINT) );
   for( i=0; i < n; i++ ) {
      (*pa)[i] = adat[i];
   }
}

/* 1d array of jfloats */

static void
setup_array_1d_f( PLFLT **pa, jfloat *adat, int n )
{
   int i;
   *pa = (PLFLT *) malloc( n * sizeof(PLFLT) );
   for( i=0; i < n; i++ ) {
      (*pa)[i] = adat[i];
   }
}

/* 1d array of jdoubles */

static void
setup_array_1d_d( PLFLT **pa, jdouble *adat, int n )
{
   int i;
   *pa = (PLFLT *) malloc( n * sizeof(PLFLT) );
   for( i=0; i < n; i++ ) {
      (*pa)[i] = adat[i];
   }
}

/* 2d array of floats */
/* Here caller must free(a[0]) and free(a) (in that order) afterward */

static void
setup_array_2d_f( PLFLT ***pa, jfloat **adat, int nx, int ny )
{
   int i, j;

   *pa = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
   (*pa)[0] = (PLFLT *) malloc( nx * ny * sizeof(PLFLT) );

   for( i=0; i < nx; i++ )
     {
	(*pa)[i] = (*pa)[0] + i*ny;
	for( j=0; j < ny; j++ )
	  (*pa)[i][j] = adat[i][j];
     }

}

/* 2d array of doubles */
/* Here caller must free(a[0]) and free(a) (in that order) afterward */

static void
setup_array_2d_d( PLFLT ***pa, jdouble **adat, int nx, int ny )
{
   int i, j;

   *pa = (PLFLT **) malloc( nx * sizeof(PLFLT *) );
   (*pa)[0] = (PLFLT *) malloc( nx * ny * sizeof(PLFLT) );

   for( i=0; i < nx; i++ )
     {
	(*pa)[i] = (*pa)[0] + i*ny;
	for( j=0; j < ny; j++ )
	  (*pa)[i][j] = adat[i][j];
     }

}
%}



/* I hate global variables but this is the best way I can think of to manage consistency
   checking among function arguments. */
%{
   static PLINT Alen = 0;
   static PLINT Xlen = 0, Ylen = 0;
   static PLFLT **xg;
   static PLFLT **yg;
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

/* with preceding count */
%typemap(in) (PLINT n, PLINT *Array) {
   jint *jxdata = (*jenv)->GetIntArrayElements( jenv, $input, 0 );
   $1 = (*jenv)->GetArrayLength( jenv, $input);
   Alen = $1;
   setup_array_1d_i( &$2, jxdata, Alen);
   /* Could find no easy way to do this as part of freearg so I modified
    * the previous function so it ALWAYS mallocs and copies so that
    * the java array can be released immediately. */
   (*jenv)->ReleaseIntArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) (PLINT n, PLINT *Array) {
   free($2);
}
%typemap(jni) (PLINT n, PLINT *Array) "jintArray"
%typemap(jtype) (PLINT n, PLINT *Array) "int[]"
%typemap(jstype) (PLINT n, PLINT *Array) "int[]"
%typemap(javain) (PLINT n, PLINT *Array) "$javainput"
%typemap(javaout) (PLINT n, PLINT *Array) {
   return $jnicall;
}

/* Trailing count and check consistency with previous*/
%typemap(in) (PLINT *ArrayCk, PLINT n) {
   jint *jydata = (*jenv)->GetIntArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) != Alen) {
      printf("Vectors must be same length.\n");
      return;
   }
   $2 = (*jenv)->GetArrayLength( jenv, $input );
   setup_array_1d_i( &$1, jydata, Alen);
   (*jenv)->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) (PLINT *ArrayCk, PLINT n) {
   free($1);
}
%typemap(jni) (PLINT *ArrayCk, PLINT n) "jintArray"
%typemap(jtype) (PLINT *ArrayCk, PLINT n) "int[]"
%typemap(jstype) (PLINT *ArrayCk, PLINT n) "int[]"
%typemap(javain) (PLINT *ArrayCk, PLINT n) "$javainput"
%typemap(javaout) (PLINT *ArrayCk, PLINT n) {
   return $jnicall;
}

/* no count but check consistency with previous */
%typemap(in) PLINT *ArrayCk {
   jint *jydata = (*jenv)->GetIntArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) != Alen) {
      printf("Vectors must be same length.\n");
      return;
   }
   setup_array_1d_i( &$1, jydata, Alen);
   (*jenv)->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLINT *ArrayCk {
   free($1);
}
%typemap(jni) PLINT *ArrayCk "jintArray"
%typemap(jtype) PLINT *ArrayCk "int[]"
%typemap(jstype) PLINT *ArrayCk "int[]"
%typemap(javain) PLINT *ArrayCk "$javainput"
%typemap(javaout) PLINT *ArrayCk {
   return $jnicall;
}

/* Weird case to allow argument to be one shorter than others */
%typemap(in) PLINT *ArrayCkMinus1 {
   jint *jydata = (*jenv)->GetIntArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) < Alen-1) {
      printf("Vector must be at least length of others minus 1.\n");
      return;
   }
   setup_array_1d_i( &$1, jydata, Alen);
   (*jenv)->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLINT *ArrayCkMinus1 {
   free($1);
}
%typemap(jni) PLINT *ArrayCkMinus1 "jintArray"
%typemap(jtype) PLINT *ArrayCkMinus1 "int[]"
%typemap(jstype) PLINT *ArrayCkMinus1 "int[]"
%typemap(javain) PLINT *ArrayCkMinus1 "$javainput"
%typemap(javaout) PLINT *ArrayCkMinus1 {
   return $jnicall;
}

/* No length but remember size to check others */
%typemap(in) PLINT *Array {
   jint *jydata = (*jenv)->GetIntArrayElements( jenv, $input, 0 );
   Alen = (*jenv)->GetArrayLength( jenv, $input);
   setup_array_1d_i( &$1, jydata, Alen);
   (*jenv)->ReleaseIntArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLINT *Array {
   free($1);
}
%typemap(jni) PLINT *Array "jintArray"
%typemap(jtype) PLINT *Array "int[]"
%typemap(jstype) PLINT *Array "int[]"
%typemap(javain) PLINT *Array "$javainput"
%typemap(javaout) PLINT *Array {
   return $jnicall;
}

/******************************************************************************
				 PLFLT Arrays
******************************************************************************/

//temporary
#if 0
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

/* temporary*/
#endif
/* with preceding count */
%typemap(in) (PLINT n, PLFLT *Array) {
   jPLFLT *jxdata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   $1 = (*jenv)->GetArrayLength( jenv, $input);
   Alen = $1;
   setup_array_1d_PLFLT( &$2, jxdata, Alen );
   /* Could find no easy way to do this as part of freearg so I modified
    * the previous function so it ALWAYS mallocs and copies so that
    * the java array can be released immediately. */
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) (PLINT n, PLFLT *Array) {
   free($2);
}
%typemap(jni) (PLINT n, PLFLT *Array) jPLFLTArray
%typemap(jtype) (PLINT n, PLFLT *Array) jPLFLTbracket
%typemap(jstype) (PLINT n, PLFLT *Array) jPLFLTbracket
%typemap(javain) (PLINT n, PLFLT *Array) "$javainput"
%typemap(javaout) (PLINT n, PLFLT *Array) {
   return $jnicall;
}

/* trailing count, and check consistency with previous */
%typemap(in) (PLFLT *ArrayCk, PLINT n) {
   jPLFLT *jydata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   $2 = (*jenv)->GetArrayLength( jenv, $input );
   if((*jenv)->GetArrayLength( jenv, $input ) != Alen) {
      printf("Vectors must be same length.\n");
      return;
   }
   setup_array_1d_PLFLT( &$1, jydata, Alen );
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) (PLFLT *ArrayCk, PLINT n) {
   free($1);
}
%typemap(jni) (PLFLT *ArrayCk, PLINT n) jPLFLTArray
%typemap(jtype) (PLFLT *ArrayCk, PLINT n) jPLFLTbracket
%typemap(jstype) (PLFLT *ArrayCk, PLINT n) jPLFLTbracket
%typemap(javain) (PLFLT *ArrayCk, PLINT n) "$javainput"
%typemap(javaout) (PLFLT *ArrayCk, PLINT n){
   return $jnicall;
}

/* no count, but check consistency with previous */
%typemap(in) PLFLT *ArrayCk {
   jPLFLT *jydata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) != Alen) {
      printf("Vectors must be same length.\n");
      return;
   }
   setup_array_1d_PLFLT( &$1, jydata, Alen );
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLFLT *ArrayCk {
   free($1);
}
%typemap(jni) PLFLT *ArrayCk jPLFLTArray
%typemap(jtype) PLFLT *ArrayCk jPLFLTbracket
%typemap(jstype) PLFLT *ArrayCk jPLFLTbracket
%typemap(javain) PLFLT *ArrayCk "$javainput"
%typemap(javaout) PLFLT *ArrayCk{
   return $jnicall;
}

/* set X length for later consistency checking */
%typemap(in) PLFLT *ArrayX {
   jPLFLT *jxdata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   Xlen = (*jenv)->GetArrayLength( jenv, $input);
   setup_array_1d_PLFLT( &$1, jxdata, Xlen);
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) PLFLT *ArrayX {
   free($1);
}
%typemap(jni) PLFLT *ArrayX jPLFLTArray
%typemap(jtype) PLFLT *ArrayX jPLFLTbracket
%typemap(jstype) PLFLT *ArrayX jPLFLTbracket
%typemap(javain) PLFLT *ArrayX "$javainput"
%typemap(javaout) PLFLT *ArrayX {
   return $jnicall;
}

/* set Y length for later consistency checking */
%typemap(in) PLFLT *ArrayY {
   jPLFLT *jydata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   Ylen = (*jenv)->GetArrayLength( jenv, $input);
   setup_array_1d_PLFLT( &$1, jydata, Ylen);
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLFLT *ArrayY {
   free($1);
}
%typemap(jni) PLFLT *ArrayY jPLFLTArray
%typemap(jtype) PLFLT *ArrayY jPLFLTbracket
%typemap(jstype) PLFLT *ArrayY jPLFLTbracket
%typemap(javain) PLFLT *ArrayY "$javainput"
%typemap(javaout) PLFLT *ArrayY {
   return $jnicall;
}

/* with trailing count */
%typemap(in) (PLFLT *Array, PLINT n) {
   jPLFLT *jxdata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   $2 = (*jenv)->GetArrayLength( jenv, $input );
   setup_array_1d_PLFLT( &$1, jxdata, $2);
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) (PLFLT *Array, PLINT n) {
   free($1);
}
%typemap(jni) (PLFLT *Array, PLINT n) jPLFLTArray
%typemap(jtype) (PLFLT *Array, PLINT n) jPLFLTbracket
%typemap(jstype) (PLFLT *Array, PLINT n) jPLFLTbracket
%typemap(javain) (PLFLT *Array, PLINT n) "$javainput"
%typemap(javaout) (PLFLT *Array, PLINT n) {
   return $jnicall;
}

/* with no trailing count */
%typemap(in) PLFLT *Array {
   jPLFLT *jxdata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   Alen = (*jenv)->GetArrayLength( jenv, $input );
   setup_array_1d_PLFLT( &$1, jxdata, Alen);
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) PLFLT *Array {
   free($1);
}
%typemap(jni) PLFLT *Array jPLFLTArray
%typemap(jtype) PLFLT *Array jPLFLTbracket
%typemap(jstype) PLFLT *Array jPLFLTbracket
%typemap(javain) PLFLT *Array "$javainput"
%typemap(javaout) PLFLT *Array {
   return $jnicall;
}

/* check consistency with X dimension of previous */
%typemap(in) PLFLT *ArrayCkX {
   jPLFLT *jxdata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) != Xlen) {
      printf("Vectors must be same length.\n");
      return;
   }
   setup_array_1d_PLFLT( &$1, jxdata, Xlen );
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jxdata, 0 );
}
%typemap(freearg) PLFLT *ArrayCkX {
   free($1);
}
%typemap(jni) PLFLT *ArrayCkX jPLFLTArray
%typemap(jtype) PLFLT *ArrayCkX jPLFLTbracket
%typemap(jstype) PLFLT *ArrayCkX jPLFLTbracket
%typemap(javain) PLFLT *ArrayCkX "$javainput"
%typemap(javaout) PLFLT *ArrayCkX{
   return $jnicall;
}

/* check consistency with Y dimension of previous */
%typemap(in) PLFLT *ArrayCkY {
   jPLFLT *jydata = (*jenv)->GetPLFLTArrayElements( jenv, $input, 0 );
   if((*jenv)->GetArrayLength( jenv, $input ) != Ylen) {
      printf("Vectors must be same length.\n");
      return;
   }
   setup_array_1d_PLFLT( &$1, jydata, Ylen );
   (*jenv)->ReleasePLFLTArrayElements( jenv, $input, jydata, 0 );
}
%typemap(freearg) PLFLT *ArrayCkY {
   free($1);
}
%typemap(jni) PLFLT *ArrayCkY jPLFLTArray
%typemap(jtype) PLFLT *ArrayCkY jPLFLTbracket
%typemap(jstype) PLFLT *ArrayCkY jPLFLTbracket
%typemap(javain) PLFLT *ArrayCkY "$javainput"
%typemap(javaout) PLFLT *ArrayCkY{
   return $jnicall;
}

/* 2D array with trailing dimensions, check consistency with previous */
%typemap(in) (PLFLT **MatrixCk, PLINT nx, PLINT ny) {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   if( nx != Xlen || ny != Ylen ) {
      printf( "Vectors must match matrix.\n" );
      for( i=0; i < nx; i++ )
	(*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      free(adat);
      free(ai);
      return;
   }
   setup_array_2d_PLFLT( &$1, adat, nx, ny );
   $2 = nx;
   $3 = ny;
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }
   

   free(adat);
   free(ai);

}
%typemap(freearg) (PLFLT **MatrixCk, PLINT nx, PLINT ny) {
   free($1[0]);
   free($1);
}
%typemap(jni) (PLFLT **MatrixCk, PLINT nx, PLINT ny) "jobjectArray"
%typemap(jtype) (PLFLT **MatrixCk, PLINT nx, PLINT ny) jPLFLTbracket2
%typemap(jstype) (PLFLT **MatrixCk, PLINT nx, PLINT ny) jPLFLTbracket2
%typemap(javain) (PLFLT **MatrixCk, PLINT nx, PLINT ny) "$javainput"
%typemap(javaout) (PLFLT **MatrixCk, PLINT nx, PLINT ny) {
   return $jnicall;
}

/* 2D array with trailing dimensions, set the X, Y size for later checking */
%typemap(in) (PLFLT **Matrix, PLINT nx, PLINT ny) {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   Xlen = nx;
   Ylen = ny;
   setup_array_2d_PLFLT( &$1, adat, nx, ny );
   $2 = nx;
   $3 = ny;
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }

   free(adat);
   free(ai);

}
%typemap(freearg) (PLFLT **Matrix, PLINT nx, PLINT ny) {
   free($1[0]);
   free($1);
}
%typemap(jni) (PLFLT **Matrix, PLINT nx, PLINT ny) "jobjectArray"
%typemap(jtype) (PLFLT **Matrix, PLINT nx, PLINT ny) jPLFLTbracket2
%typemap(jstype) (PLFLT **Matrix, PLINT nx, PLINT ny) jPLFLTbracket2
%typemap(javain) (PLFLT **Matrix, PLINT nx, PLINT ny) "$javainput"
%typemap(javaout) (PLFLT **Matrix, PLINT nx, PLINT ny) {
   return $jnicall;
}

/* 2D array with no trailing dimensions, set the X, Y size for later checking */
%typemap(in) PLFLT **Matrix {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   Xlen = nx;
   Ylen = ny;
   setup_array_2d_PLFLT( &$1, adat, nx, ny );
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }

   free(adat);
   free(ai);

}
%typemap(freearg) PLFLT **Matrix {
   free($1[0]);
   free($1);
}
%typemap(jni) PLFLT **Matrix "jobjectArray"
%typemap(jtype) PLFLT **Matrix jPLFLTbracket2
%typemap(jstype) PLFLT **Matrix jPLFLTbracket2
%typemap(javain) PLFLT **Matrix "$javainput"
%typemap(javaout) PLFLT **Matrix {
   return $jnicall;
}

/* 2D array, check for consistency */
%typemap(in) PLFLT **MatrixCk {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   if( nx != Xlen || ny != Ylen ) {
      printf( "Vectors must match matrix.\n" );
      for( i=0; i < nx; i++ )
	(*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      free(adat);
      free(ai);
      return;
   }
   setup_array_2d_PLFLT( &$1, adat, nx, ny );
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }

   free(adat);
   free(ai);

}
%typemap(freearg) PLFLT **MatrixCk {
   free($1[0]);
   free($1);
}
%typemap(jni) PLFLT **MatrixCk "jobjectArray"
%typemap(jtype) PLFLT **MatrixCk jPLFLTbracket2
%typemap(jstype) PLFLT **MatrixCk jPLFLTbracket2
%typemap(javain) PLFLT **MatrixCk "$javainput"
%typemap(javaout) PLFLT **MatrixCk {
   return $jnicall;
}

%{
   typedef PLINT (*defined_func)(PLFLT, PLFLT);
   typedef void (*fill_func)(PLINT, PLFLT*, PLFLT*);
   typedef void (*pltr_func)(PLFLT, PLFLT, PLFLT *, PLFLT*, PLPointer);
   typedef PLFLT (*f2eval_func)(PLINT, PLINT, PLPointer);
   %}

/* First of two object arrays, where we check X and Y with previous.
 * Note this is the simplified Tcl-like approach to handling the xg
 * and yg arrays.  Later we would like to move to true call-back functions
 * here instead like is done with the python interface. */
%typemap(in) pltr_func pltr {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   if( !((nx == Xlen && ny == Ylen) || (nx == Xlen && ny == 1))) {
      printf( "Xlen = %d, nx = %d, Ylen = %d, ny = %d\n", Xlen, nx, Ylen, ny );
      printf( "X vector or matrix must match matrix dimensions.\n" );
      for( i=0; i < nx; i++ )
	(*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      free(adat);
      free(ai);
      return;
   }
   /* Store whether second dimension is unity. */
   Alen = ny;
   setup_array_2d_PLFLT( &xg, adat, nx, ny );
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }

   free(adat);
   free(ai);
   $1 = pltr2;

}

%typemap(freearg) pltr_func pltr {
   free(xg[0]);
   free(xg);
}
%typemap(jni) pltr_func pltr "jobjectArray"
%typemap(jtype) pltr_func pltr jPLFLTbracket2
%typemap(jstype) pltr_func pltr jPLFLTbracket2
%typemap(javain) pltr_func pltr "$javainput"
%typemap(javaout) pltr_func pltr {
   return $jnicall;
}

/* Second of two object arrays, where we check X and Y with previous object. */
%typemap(in) PLPointer OBJECT_DATA {
   jPLFLT **adat;
   jobject *ai;
   int nx = (*jenv)->GetArrayLength( jenv, $input );
   int ny = -1;
   int i, j;
   PLcGrid2 cgrid;
   ai = (jobject *) malloc( nx * sizeof(jobject) );
   adat = (jPLFLT **) malloc( nx * sizeof(jPLFLT *) );

   for( i=0; i < nx; i++ )
     {
	ai[i] = (*jenv)->GetObjectArrayElement( jenv, $input, i );
	adat[i] = (*jenv)->GetPLFLTArrayElements( jenv, ai[i], 0 );

	if (ny == -1)
	  ny = (*jenv)->GetArrayLength( jenv, ai[i] );
	else if (ny != (*jenv)->GetArrayLength( jenv, ai[i] )) {
	   printf( "Misshapen a array.\n" );
	   for( j=0; j <= i; j++ )
	     (*jenv)->ReleasePLFLTArrayElements( jenv, ai[j], adat[j], 0 );
	   free(adat);
	   free(ai);
	   return;
	}
     }

   if( !((nx == Xlen && ny == Ylen) || (nx == Ylen && ny == 1 && ny == Alen))) {
      printf( "Xlen = %d, nx = %d, Ylen = %d, Alen = %d, ny = %d\n",
	      Xlen, nx, Ylen, Alen, ny );
      printf( "Y vector or matrix must match matrix dimensions.\n" );
      for( i=0; i < nx; i++ )
	(*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      free(adat);
      free(ai);
      return;
   }
   setup_array_2d_PLFLT( &yg, adat, nx, ny );
   for( i=0; i < nx; i++ ) {
      (*jenv)->ReleasePLFLTArrayElements( jenv, ai[i], adat[i], 0 );
      (*jenv)->DeleteLocalRef(jenv, ai[i]);
   }

   free(adat);
   free(ai);
   cgrid.xg = xg;
   cgrid.yg = yg;
   cgrid.nx = nx;
   cgrid.ny = ny;
   $1 = &cgrid;

}

%typemap(freearg) PLPointer OBJECT_DATA {
   free(yg[0]);
   free(yg);
}
%typemap(jni) PLPointer OBJECT_DATA "jobjectArray"
%typemap(jtype) PLPointer OBJECT_DATA jPLFLTbracket2
%typemap(jstype) PLPointer OBJECT_DATA jPLFLTbracket2
%typemap(javain) PLPointer OBJECT_DATA "$javainput"
%typemap(javaout) PLPointer OBJECT_DATA {
   return $jnicall;
}

// Do not specify defined function or fill function from java.  Instead
// specify NULL and plfill defaults in the interface C code.
%typemap(in, numinputs=0) defined_func df {
     $1 = NULL;
}
%typemap(in, numinputs=0) fill_func ff {
     $1 = plfill;
}

/***************************
	String returning functions
	Adopt method in SWIG-1.3.21/Examples/java/typemap/example.i
****************************/

/* Define the types to use in the generated JNI C code and Java code */
%typemap(jni) char *OUTPUT "jobject"
%typemap(jtype) char *OUTPUT "StringBuffer"
%typemap(jstype) char *OUTPUT "StringBuffer"

/* How to convert Java(JNI) type to requested C type */
%typemap(in) char *OUTPUT {

  $1 = NULL;
  if($input != NULL) {
    /* Get the String from the StringBuffer */
    jmethodID setLengthID;
    jclass sbufClass = (*jenv)->GetObjectClass(jenv, $input);
    /* Take a copy of the C string as the typemap is for a non const C string */
    jmethodID capacityID = (*jenv)->GetMethodID(jenv, sbufClass, "capacity", "()I");
    jint capacity = (*jenv)->CallIntMethod(jenv, $input, capacityID);
    $1 = (char *) malloc(capacity+1);

    /* Zero the original StringBuffer, so we can replace it with the result */
    setLengthID = (*jenv)->GetMethodID(jenv, sbufClass, "setLength", "(I)V");
    (*jenv)->CallVoidMethod(jenv, $input, setLengthID, (jint) 0);
  }
}


/* How to convert the C type to the Java(JNI) type */
%typemap(argout) char *OUTPUT {

   if($1 != NULL) {
      /* Append the result to the empty StringBuffer */
      jstring newString = (*jenv)->NewStringUTF(jenv, $1);
      jclass sbufClass = (*jenv)->GetObjectClass(jenv, $input);
      jmethodID appendStringID = (*jenv)->GetMethodID(jenv, sbufClass, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
      (*jenv)->CallObjectMethod(jenv, $input, appendStringID, newString);

      /* Clean up the string object, no longer needed */
      free($1);
      $1 = NULL;
   }
}
/* Prevent the default freearg typemap from being used */
%typemap(freearg) char *OUTPUT ""

/* Convert the jstype to jtype typemap type */
%typemap(javain) char *OUTPUT "$javainput"

/* Character arrays: */

%typemap(jni) (int *p_argc, char **argv) "jobjectArray"
%typemap(jtype) (int *p_argc, char **argv) "String[]"
%typemap(jstype) (int *p_argc, char **argv) "String[]"
%typemap(javain) (int *p_argc, char **argv) "$javainput"
%typemap(javaout) (int *p_argc, char **argv) {
   return $jnicall;
}
%typemap(in) (int *p_argc, char **argv) (jint size) {
   int i = 0;
   size = (*jenv)->GetArrayLength(jenv, $input);
   $1 = &size;
   $2 = (char **) malloc((size+1)*sizeof(char *));
   /* make a copy of each string */
   for (i = 0; i<size; i++) {
      jstring j_string = (jstring)(*jenv)->GetObjectArrayElement(jenv, $input, i);
      const char * c_string = (char *) (*jenv)->GetStringUTFChars(jenv, j_string, 0);
/* Commented out version straight from swig documentation, but I think
 * it is wrong.
 *    $2[i] = malloc(strlen((c_string)+1)*sizeof(const char *)); */
      $2[i] = malloc((strlen(c_string)+1)*sizeof(const char *));
      strcpy($2[i], c_string);
      (*jenv)->ReleaseStringUTFChars(jenv, j_string, c_string);
      (*jenv)->DeleteLocalRef(jenv, j_string);
   }
   $2[i] = 0;
}

/* This cleans up the memory we malloc'd before the function call */
%typemap(freearg) (int *p_argc, char **argv) {
   int i;
/* Commented out version straight from swig documentation, but I think
 * it is wrong.
 * for (i=0; i<size$argnum-1; i++) */
   for (i=0; i<size$argnum; i++)
     free($2[i]);
   free($2);
}

#if 0
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
#endif

/* swig compatible PLplot API definitions from here on. */
%include plplotcapi.i
