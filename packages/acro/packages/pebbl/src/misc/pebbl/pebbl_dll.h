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
 * \file pebbl_dll.h
 *
 * This information is used by MVC++ to create a DLL
 */

#ifndef pebbl_pebbl_dll_h
#define pebbl_pebbl_dll_h

#include <acro_config.h>
#ifdef _MSC_VER
#ifdef PEBBL_EXPORTS
#define PEBBL_API __declspec(dllexport)
#else
#define PEBBL_API __declspec(dllimport)
#endif

#else
#define PEBBL_API
#endif

#endif

