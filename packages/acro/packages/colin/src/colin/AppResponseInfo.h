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
 * \file AppResponseInfo.h
 *
 * Function wrapper for managing AppResponse info types.
 */

#ifndef colin_AppResponseInfo_h
#define colin_AppResponseInfo_h

#include <acro_config.h>

#include <utilib/StringRegistry.h>
#include <utilib/Any.h>

namespace colin
{

/// Central registry for response_info_t declarations
utilib::StringRegistry& AppResponseInfo();

/// Defines a "type" for application response component management
/** JDS: Should we change this from a typedef (which doesn't actually
 *  define a type) into a simple class (which does)?
 */
typedef utilib::StringRegistry::registry_id_type   response_info_t;

/// The fundamental type for storing AppRequest / AppResponse ids
typedef long app_request_id_t;


// Standard responses that applications can generate
extern const response_info_t f_info;
extern const response_info_t mf_info;

extern const response_info_t cf_info;
extern const response_info_t eqcf_info;
extern const response_info_t ineqcf_info;

extern const response_info_t lcf_info;
extern const response_info_t leqcf_info;
extern const response_info_t lineqcf_info;

extern const response_info_t nlcf_info;
extern const response_info_t nleqcf_info;
extern const response_info_t nlineqcf_info;

extern const response_info_t ndcf_info;
extern const response_info_t ndeqcf_info;
extern const response_info_t ndineqcf_info;

extern const response_info_t cvf_info;
extern const response_info_t eqcvf_info;
extern const response_info_t ineqcvf_info;

extern const response_info_t lcvf_info;
extern const response_info_t leqcvf_info;
extern const response_info_t lineqcvf_info;

extern const response_info_t nlcvf_info;
extern const response_info_t nleqcvf_info;
extern const response_info_t nlineqcvf_info;

extern const response_info_t ndcvf_info;
extern const response_info_t ndeqcvf_info;
extern const response_info_t ndineqcvf_info;

extern const response_info_t g_info;

extern const response_info_t cg_info;
extern const response_info_t eqcg_info;
extern const response_info_t ineqcg_info;

extern const response_info_t lcg_info;
extern const response_info_t leqcg_info;
extern const response_info_t lineqcg_info;

extern const response_info_t nlcg_info;
extern const response_info_t nleqcg_info;
extern const response_info_t nlineqcg_info;

extern const response_info_t ndcg_info;
extern const response_info_t ndeqcg_info;
extern const response_info_t ndineqcg_info;

extern const response_info_t h_info;

}

#endif
