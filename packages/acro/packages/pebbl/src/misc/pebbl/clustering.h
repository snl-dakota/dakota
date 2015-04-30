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
 * \file clustering.h
 * \author Jonathan Eckstein
 *
 * Logic for organizing things (typically processors) into clusters.
 */

#ifndef pebbl_clustering_h
#define pebbl_clustering_h

#include <acro_config.h>

namespace pebbl {


class clusterObj
{
public:
  
  // Initialize things to avoid valgrind complaints when registering
  // parameters for parMIP.
  // Meaningful initialization is by reset(...) with real arguments.

  clusterObj() { reset(0,1,1,1,0); };

  void reset(int rank_,
	     int size_,
	     int sizeWanted,
	     int clustersWanted,
	     int forceSeparateSize);

  int rank;
  int size;

  bool separateFunctions;
  bool iAmLeader;
  bool iAmFollower;

  int clusterNumber;
  int typicalSize;
  int clusterSize;
  int lastClusterSize;
  int leader;
  int numPureLeaders;
  int numFollowers;
  int positionInCluster;
  int followerPosInCluster;
  int numClusters;
  int totalNumFollowers;

  bool typicallySeparated;
  bool lastSeparated;

  int typicalFollowers;
  int myGlobalFollowerRank;

  int follower(int whichFollower)
    {
      return leader + whichFollower + separateFunctions;
    };

  int whichFollower(int n)
    {
      return n - leader - separateFunctions;
    };

  int leaderOfCluster(int whichCluster)
    {
      return whichCluster*typicalSize;
    };

  int whichCluster(int n)
    {
      return n/typicalSize;
    };

  int hasPureLeader(int cNum)
    {
      if (cNum + 1 == numClusters)
	return lastSeparated;
      else
	return typicallySeparated;
    };

  // Translates an index into the total collection of followers
  // into an index into the total collection of things.

  int globalFollower(int n)
    {
      int targetCluster = n/typicalFollowers;
      int targetPos     = n - targetCluster*typicalFollowers;
      return (targetCluster*typicalSize + 
	      hasPureLeader(targetCluster) + targetPos);
    };

  // Translates an index into the total collection of things into
  // an index into the total collection of followers.

  int followerRank(int globalRank)
    {
      int targetCluster = whichCluster(globalRank);
      int relPos        = globalRank - targetCluster*typicalSize;
      return (targetCluster*typicalFollowers + 
	      relPos - hasPureLeader(targetCluster));
    };

  // Returns whether a given global index corresponds to a leader

  bool isLeader(int n)
  {
    return (n % typicalSize) == 0;
  }

  // Returns whether a given global index corresponds to a follower

  bool isFollower(int n)
  {
    if (!isLeader(n))
      return true;
    if (whichCluster(n) == numClusters - 1)
      return !lastSeparated;
    return !typicallySeparated;
  }

};
  
}

#endif
