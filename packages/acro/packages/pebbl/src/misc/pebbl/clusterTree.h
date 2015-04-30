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
 * \file clusterTree.h
 * \author Jonathan Eckstein
 *
 * Organize some clusters into a tree.  Used for PEBBL load balancing.
 */

#ifndef pebbl_clusterTree_h
#define pebbl_clusterTree_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/clustering.h>
#include <pebbl/treeTopology.h>
#include <pebbl/fundamentals.h>

namespace pebbl {


class nAryClusterTree : public nAryTreeRememberParent
{
private:
  
  clusterObj* cluster;

  int clusterNumberOf(int where)
    {
      int whereCluster = cluster->whichCluster(where);
#ifdef ACRO_VALIDATING
      if (where != cluster->leaderOfCluster(whereCluster))
	EXCEPTION_MNGR(std::runtime_error,"Cluster tree finding address of non-leader");
#endif
      return whereCluster;
    };
 
public:

  int currentChild()
    {
      return cluster->leaderOfCluster(nAryTree::currentChild());
    };

  int parent()
    {
      return cluster->leaderOfCluster(nAryTreeRememberParent::parent());
    };

  int advanceChild()
    {
      return cluster->leaderOfCluster(nAryTree::advanceChild());
    };

  int validChild(int where)
    {
      return nAryTree::validChild(clusterNumberOf(where));
    };

  int whichChild(int where)
    {
      return nAryTree::whichChild(clusterNumberOf(where));
    };

  nAryClusterTree(clusterObj& cluster_,
		  int radix_ = 2) :
  nAryTreeRememberParent(radix_,
			 0,
			 cluster_.clusterNumber,
			 cluster_.numClusters
			 ),
  cluster(&cluster_)
    { };

};

} // namespace pebbl

#endif

#endif
