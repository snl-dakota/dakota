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
 * \file AppResponseTypes.h
 *
 * Defines the colin::AppResponseTypes class.
 */

#ifndef colin_AppResponseTypes_h
#define colin_AppResponseTypes_h

#include <acro_config.h>
#include <utilib/pvector.h>
#include <colin/real.h>

namespace colin
{

/**
 * Defines typedefs for response objects.
 */
#ifdef ACRO_HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS
template <template <typename Type> class ArrayT = utilib::pvector,
class VectorT = ArrayT<real>,
class DArrayT = ArrayT<real>,
class IArrayT = ArrayT<int>,
class MatrixT = ArrayT<VectorT>,
class VArrayT = ArrayT<VectorT>,
class MArrayT = ArrayT<MatrixT> >
#else
template <class VectorT = utilib::BasicArray<real>,
class DArrayT = utilib::BasicArray<real>,
class IArrayT = utilib::BasicArray<int>,
class MatrixT = utilib::BasicArray<VectorT>,
class VArrayT = utilib::BasicArray<VectorT>,
class MArrayT = utilib::BasicArray<MatrixT> >
#endif
class AppResponseTypes
{
public:

   /// The vector type.
   typedef VectorT vector_t;
   /// The matrix type.
   typedef MatrixT matrix_t;
   /// The real array type.
   typedef DArrayT realarray_t;
   /// The int array type.
   typedef IArrayT intarray_t;
   /// The vector array type.
   typedef VArrayT vectorarray_t;
   /// The matrix array type.
   typedef MArrayT matrixarray_t;

};

}

#endif
