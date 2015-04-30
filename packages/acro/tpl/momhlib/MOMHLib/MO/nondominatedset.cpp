/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz, Radoslaw Ziembinski (radekz@free.alpha.net.pl)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation (www.gnu.org); 
either version 2.1 of the License, or (at your option) any later 
version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#if !defined(_NONDOMINATEDSET_CPP)
#define _NONDOMINATEDSET_CPP

#include "global.h"
#include "momhsolution.h"
#include "solutionsset.h"
#include "nondominatedset.h"

double TNondominatedSet::ClusterDistance(IntVector iCluster1, IntVector iCluster2, int iSpaceType, bool bScalarize, vector<double> &dIdeal, vector<double> &dNadir)
{
double			dDistance = 0.0;
double			dAverageDistance = 0.0;
TMOMHSolution	*pSolution1 = NULL;
TMOMHSolution	*pSolution2 = NULL;
unsigned int	i = 0;
unsigned int	j = 0;

	// compute average distance between each par in the cluster
	dDistance = 0.0;
	for(i = 0; i < iCluster1.size(); i++) 
	{
		for(j = 0; j < iCluster2.size(); j++) 
		{
			pSolution1 = (TMOMHSolution *)(*this)[iCluster1[i]];
			pSolution2 = (TMOMHSolution *)(*this)[iCluster2[j]];

			assert(pSolution1 != NULL);
			assert(pSolution2 != NULL);

			// measure distance in parameter space or in objective space
			switch(iSpaceType)
			{
				default:
				case PARAMETER_SPACE:
					dDistance += abs(pSolution1->ParameterValuesDistance(*pSolution2));
					break;
				case OBJECTIVE_SPACE:
					dDistance += abs(pSolution1->ObjectiveValuesDistance(*pSolution2, bScalarize, dIdeal, dNadir));
					break;
			}
		}
	}

	dAverageDistance = dDistance / (iCluster1.size() * iCluster2.size());

	return dAverageDistance;
}

int TNondominatedSet::GetCentroid(IntVector iCluster, int iSpaceType, bool bScalarize, vector<double> &dIdeal, vector<double> &dNadir)
{
unsigned int		i = 0;
unsigned int		j = 0;	
int					iWinner = 0;
double				dMinDistance = 0.0;
TMOMHSolution		*pSolution1 = NULL;
TMOMHSolution		*pSolution2 = NULL;
double				dDistance = 0.0;
double				dAverageDistance = 0.0;

	assert(iCluster.size() > 0);

	if (iCluster.size() <= 2)
	{
		iWinner = iCluster[0];
		return iWinner;
	}

	dMinDistance = -1; // infinity

	for(i = 0; i < iCluster.size(); i++)
	{
		dDistance = 0.0;
		for(j = 0; j < iCluster.size(); j++)
		{
			if (i != j) {
				pSolution1 = (TMOMHSolution *)(*this)[iCluster[i]];
				pSolution2 = (TMOMHSolution *)(*this)[iCluster[j]];

				assert(pSolution1 != NULL);
				assert(pSolution2 != NULL);

				// measure distance in parameter space or in objective space
				switch(iSpaceType)
				{
					default:
					case PARAMETER_SPACE:
						dDistance += abs(pSolution1->ParameterValuesDistance(*pSolution2));
						break;
					case OBJECTIVE_SPACE:
						dDistance += abs(pSolution1->ObjectiveValuesDistance(*pSolution2, bScalarize, dIdeal, dNadir));
						break;
				}
				
			}
		}

		dAverageDistance = dDistance / (iCluster.size() - 1.0);

		if ((dMinDistance == -1) || (dMinDistance > dAverageDistance))
		{
			iWinner = iCluster[i];
			dMinDistance = dAverageDistance;
		}
	}

	return iWinner;
}

bool TNondominatedSet::ReduceIndividuals(unsigned int iPopulationSize, vector<int> &iReducedIndividuals, int iSpaceType, bool bScalarize)
{
vector<int>			iIndices;
vector<IntVector>		iClusterSet;
unsigned int			i = 0;
unsigned int			j = 0;
int						k = 0;
int						iSolution = 0;
int						iCluster1 = 0;
int						iCluster2 = 0;
int						iClusterToRemove = 0;
int						iClusterToCollect = 0;
double					dMinDistance = 0.0;	
double					dDistance = 0.0;
vector<double>			dIdeal;
vector<double>			dNadir;

	// store all solutions
	iIndices.resize(0);
	
	for(i = 0; i < size(); i++) 
	{
		if ((*this)[i] != NULL) 
		{	
			iIndices.push_back(i);
		}
	}

	// prepare ideal and nadir point for scalarization
	if (bScalarize == true)
	{
		UpdateIdealNadir();
		dIdeal.resize(NumberOfObjectives);
		dNadir.resize(NumberOfObjectives);

		for(i = 0; i < NumberOfObjectives; i++)
		{
			if (Objectives[i].bActive == true)
			{
				dIdeal[i] = ApproximateIdealPoint.ObjectiveValues[i];
				dNadir[i] = ApproximateNadirPoint.ObjectiveValues[i];
			}
			else 
			{
				dIdeal[i] = 0.0;
				dNadir[i] = 0.0;
			}
		}
	}

	// prepare clusters
	// each of element became single cluster 
	iClusterSet.resize(iIndices.size());
	iReducedIndividuals.resize(0);

	for(i = 0; i < iClusterSet.size(); i++)
	{
		iClusterSet[i].resize(0);
		iClusterSet[i].push_back(iIndices[i]);
	}

	// join clusters until population will smaller than demanded value
	while(iClusterSet.size() > iPopulationSize)
	{
		dMinDistance = -1.0; // infinity
		iCluster1 = -1;
		iCluster2 = -1;

		
		for(i = 0; i < iClusterSet.size(); i++) 
		{
			for(j = 0; j < iClusterSet.size(); j++)
			{
				if (i != j) 
				{
					dDistance = abs(ClusterDistance(iClusterSet[i], iClusterSet[j], iSpaceType, bScalarize, dIdeal, dNadir));

					if ((dMinDistance > dDistance) || (dMinDistance == -1.0))
					{
						dMinDistance = dDistance;
						iCluster1 = i;
						iCluster2 = j;
					} 
				}
			}
		}

		// join two clusters
		iClusterToRemove = (iCluster1 < iCluster2) ? (iCluster2) : (iCluster1);
		iClusterToCollect = (iCluster1 < iCluster2) ? (iCluster1) : (iCluster2);

		// join clusters 
		for(i = 0; i < iClusterSet[iClusterToRemove].size(); i++)
		{
			iClusterSet[iClusterToCollect].push_back(iClusterSet[iClusterToRemove][i]);
		}

		// remove uneeded cluster
		iClusterSet.erase(iClusterSet.begin() + iClusterToRemove);
	}

	// if number of clusters is less than assumed fill output list
	for(i = 0; i < iClusterSet.size(); i++) 
	{
		iSolution = GetCentroid(iClusterSet[i], iSpaceType, bScalarize, dIdeal, dNadir);
		iReducedIndividuals.push_back(iSolution);
	}

	return true;
}

#endif // !defined(_NONDOMINATEDSET_CPP)
