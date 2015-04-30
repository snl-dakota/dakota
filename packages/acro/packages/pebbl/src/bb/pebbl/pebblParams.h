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
 * \file pebblParams.h
 *
 * Defines the pebbl::pebblParams class.
 */

#ifndef pebbl_pebblParams_h
#define pebbl_pebblParams_h

#include <acro_config.h>
#include <utilib/ParameterSet.h>

namespace pebbl {


/// A base class for PEBBL parameters. 
class pebblParams : 
  virtual public utilib::ParameterSet, 
  virtual public utilib::CommonIO
{
public:

  ///
  pebblParams();

  ///
  int statusPrintCount;

  ///
  double statusPrintSeconds;

  ///
  bool depthFirst;

  ///
  bool breadthFirst;

  ///
  bool initialDive;

  ///
  bool integralityDive;

  ///
  bool lazyBounding;

  ///
  bool eagerBounding;

  ///
  double relTolerance;

  ///
  double absTolerance;

  ///
  double earlyOutputMinutes;

  ///
  double startIncumbent;

  ///
  bool validateLog;

  ///
  bool heurLog;

  ///
  double loadLogSeconds;

  ///
  double loadLogWriteSeconds;

  ///
  int maxSPBounds;

  ///
  double maxCPUMinutes;

  ///
  double maxWallMinutes;

  ///
  bool haltOnIncumbent;

  ///
  bool printAbortMessage;

  ///
  bool printIntMeasure;

  ///
  bool printDepth;

  ///
  int debugPrecision;

  ///
  bool suppressWarnings;

  ///
  int loadMeasureDegree;

  ///
  double enumRelTol;

  ///
  double enumAbsTol;

  ///
  double enumCutoff;

  ///
  int enumCount;

  /// 
  int enumHashSize;

  ///
  bool debug_solver_params;

  ///
  bool use_abort;

  ///
  bool version_flag;

  ///
  bool printFullSolution;

  ///
  std::string solFileName;

  ///
  int printSpTimes;

};

} // namespace pebbl

#endif
