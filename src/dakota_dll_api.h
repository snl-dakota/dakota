/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: Declaration of the Dakota API for DLL interactions
//- Owner:       Brian Adams
//- Checked by:
//- Version: $Id$

/** \file dakota_dll_api.h
    \brief API for DLL interactions */

#ifndef DAKOTA_DLL_API_H
#define DAKOTA_DLL_API_H

#ifdef __MINGW32__
#ifdef BUILDING_DAKOTA_DLL
// used when building the API to specify its exports
#define DAKOTA_DLL_FN __declspec(dllexport)
#else
// used by clients importing this DLL
#define DAKOTA_DLL_FN __declspec(dllimport)
#endif
#else
#define DAKOTA_DLL_FN
#endif

/// create and configure a new DakotaRunner, adding it to list of instances
extern "C" void DAKOTA_DLL_FN dakota_create(int* dakota_ptr_int, 
					    const char* logname);

/// command DakotaRunner instance id to read from file dakotaInput
extern "C" int DAKOTA_DLL_FN dakota_readInput(int id, const char* dakotaInput);

/// command DakotaRunner instance id to start (plugin interface and run strategy)
extern "C" int DAKOTA_DLL_FN dakota_start(int id);

/// delete Dakota runner instance id and remove from active list
extern "C" void DAKOTA_DLL_FN dakota_destroy (int id);

/// command DakotaRunner instance id to stop execution
extern "C" void DAKOTA_DLL_FN dakota_stop(int* id);

/// return current results output as a string
extern "C" const char* DAKOTA_DLL_FN dakota_getStatus(int id);

/// get the DAKOTA pointer to ModelCenter
extern "C" int DAKOTA_DLL_FN get_mc_ptr_int();

/// set the DAKOTA pointer to ModelCenter
extern "C" void DAKOTA_DLL_FN set_mc_ptr_int(int ptr_int);

/// get the DAKOTA pointer to ModelCenter current design point
extern "C" int DAKOTA_DLL_FN get_dc_ptr_int();

/// set the DAKOTA pointer to ModelCenter current design point
extern "C" void DAKOTA_DLL_FN set_dc_ptr_int(int ptr_int);

/// return the variable and response names
extern "C" void DAKOTA_DLL_FN 
dakota_get_variable_info(int id,
			 char*** pVarNames, int* pNumVarNames, 
			 char*** pRespNames, int* pNumRespNames);

#endif // DAKOTA_DLL_API_H

