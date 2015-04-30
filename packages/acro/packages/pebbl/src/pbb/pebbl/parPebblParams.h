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
 * \file parPebblParams.h
 * \author William Hart
 *
 * Defines the pebbl::parallelPebblParams class.
 */

#ifndef pebbl_parPebblParams_h
#define pebbl_parPebblParams_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/ParameterSet.h>

namespace pebbl {


class parallelPebblParams : virtual public utilib::ParameterSet
{
public:

  ///
  parallelPebblParams();

  // Controls how long we do the synchronous ramp-up phase.  This may have
  // no effect if the continueRampUp() method has been overridden.

  int rampUpPoolLimit;
  double rampUpPoolLimitFac;
  int minRampUpSubprobsCreated;

  // Control parameters for clustering.

  int clusterSize;
  int numClusters;
  int hubsDontWorkSize;

  // Default thread control, priorities, and timeslices.

  bool   useIncumbentThread;
  double incSearchMaxControl;

  double maxWorkerControl;
  double workerThreadBias;

  double incThreadMaxBias;
  double incThreadMinBias;
  double noIncumbentMinBias;
  double incThreadBiasFactor;
  double incThreadBiasPower;

  double timeSlice;

  double incThreadGapSlices;

  // Miscellaneous printout control parameters.

  bool workersPrintStatus;
  bool hubsPrintStatus;
  bool printSolutionSynch;
  bool combineTimingsMax;
  bool trackIncumbent;

  // Safety checking control.

  bool checkTokensMatch;

  // Hub operation and hub-worker interacation.

  int maxSPAcks;
  int maxTokensInHubMsg;
  int localScatterQuantum;
  int localLoadInformRate;
  int globalLoadInformRate;
  int workerLoadReportRate;
  int dispatchLoadInformRate;

  double hubLoadFac;
  double lowLoadFac;
  double highLoadFac;

  double initialHubSeconds;
  double initialHubBusyFraction;
  double hubBusyInflate;
  double hubBusyAdjust;

  double starveSeconds;
  double workerLoadReportFac;
  double workerBoundReportFac;
  static int maxTokenQueuing;
  int maxDispatchPacking;
  int maxSPPacking;
  int spReceiveBuf;
  double minScatterProb;
  double scatterFac;
  double targetScatterProb;
  double maxScatterProb;
  int initForceReleases;

  // Incumbent broadcast and related

  int incumbTreeRadix;
  int refSolArrayQuantum;

  // Worker-hub "rebalancing" 

  bool   rebalancing;
  int    workerKeepCount;
  double rebalLoadFac;
  double rebalLoadDiff;

  // Quality-of-work balancing.

  bool qualityBalance;

  // Multiple hubs and load balancing

  double clusterLowLoadRatio;
  double clusterHighLoadRatio;

  double minNonLocalScatterProb;
  double targetNonLocalScatterProb;
  double maxNonLocalScatterProb;

  int loadBalTreeRadix;
  int maxLoadBalSize;
  double loadBalDonorFac;
  double loadBalReceiverFac;
  int loadBalMinSourceCount;

  bool   maxLoadBalRate;
  double loadBalSeconds;
  double loadBalIdleIncrease;

  int loadBalEarlyRounds;

  // Enumeration

  int rampUpSolQuantum;
  int reposTreeRadix;
  double reposMergeSeconds;
  double reposMergeSkew;
  bool enumFlowControl;

  // Checkpoint management

  double checkpointMinutes;
  double checkpointMinInterval;

  bool restart;
  bool reconfigure;

  std::string checkpointDir;

  int abortCheckpoint;
  int abortCheckpointCount;

  // For debugging and testing purposes

  bool rampUpOnly;

  int schedulerDebug;
  int hubDebug;
  int workerDebug;
  int loadBalDebug;
  int termDebug;
  int workerAuxDebug;
  int reposDebug;

  int abortDebug;

  //  int outputCacheSize;
  double boundWasteTime;

  int cpDebugCount;

  int debugSeqDigits;

  bool stallForDebug;
  bool forceParallel;

  bool loadLogSMP;
  int  loadLogClockSyncs;
  bool logTransitions;

};


} // namespace pebbl

#endif

#endif
