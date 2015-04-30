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
 * \file SysCallAnalysisCode.h
 *
 * Defines the colin::SysCallAnalysisCode class.
 */

#ifndef colin_SysCallAnalysisCode_h
#define colin_SysCallAnalysisCode_h

#include <acro_config.h>
#include <colin/AnalysisCode.h>
#include <colin/CommandShell.h>

namespace colin
{


/** \class SysCallAnalysisCode
  *
  * Derived class in the AnalysisCode class hierarchy which spawns
  * simulations using system calls.
  *
  * \c SysCallAnalysisCode creates separate simulation processes using
  * the C system() command.  It utilizes \c CommandShell to manage shell
  * syntax and asynchronous invocations.
  */
class SysCallAnalysisCode: public AnalysisCode
{
public:

   /// Constructor
   SysCallAnalysisCode() {}

   /// Destructor
   ~SysCallAnalysisCode() {}

   /** Spawn a complete function evaluation.
     * Put the SysCallAnalysisCode to the shell using either the default syntax
     * or specified commandUsage syntax.  This function is used when all portions
     * of the function evaluation (i.e., all analysis drivers) are executed on
     * the local processor.
     */
   void spawn_evaluation(bool block_flag);

   /// optional command usage string for supporting nonstandard
   /// command syntax (supported only by SysCall analysis codes)
   std::string commandUsage;

};


}

#endif
