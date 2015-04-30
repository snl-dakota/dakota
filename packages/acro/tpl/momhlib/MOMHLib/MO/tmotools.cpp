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

#include "global.h"
#include "tmotools.h"
#include "solutionsset.h"
#include "momhconstrainedsolution.h"


int TMOTools::GetRandomSolutionUsingFitnessMax(vector <TSolution *> &oSolutions, vector <double> &dFitness, double dSumOfFitness)
{
double		dRandomSumOfFitnesses = 0.0;
double		dTempSum = 0.0;
unsigned int			i = 0;

	assert(oSolutions.size() > 0);

	dRandomSumOfFitnesses = ((double)rand () / (double)RAND_MAX) * dSumOfFitness;

	i = 0;
	while((i < (oSolutions.size() - 1)) && 
		 (dTempSum < dRandomSumOfFitnesses)) 
	{
		dTempSum += dFitness[i];
		i += 1;
	}

	return i;
}


int TMOTools::GetSolutionUsingBinaryTournamentMin(vector <TSolution *> &oSolutions, vector <double> &dFitness)
{
int		iSolution1 = 0;
int		iSolution2 = 0;
int		iWinner = 0;

	assert(oSolutions.size() > 0);

	if (oSolutions.size() == 1) 
	{
		return 0;
	}

	// select solution 1
	iSolution1 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());

	do 
	{
		iSolution2 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());
	} 
	while(iSolution1 == iSolution2);

	if (dFitness[iSolution1] <= dFitness[iSolution2])
	{
		iWinner = iSolution1;
	} 
	else
	{
		iWinner = iSolution2;
	}

	return iWinner;
}

int TMOTools::GetSolutionUsingBinaryTournamentMax(vector <TSolution *> &oSolutions, vector <double> &dFitness)
{
int		iSolution1 = 0;
int		iSolution2 = 0;
int		iWinner = 0;

	assert(oSolutions.size() > 0);

	if (oSolutions.size() == 1) 
	{
		return 0;
	}

	// select solution 1
	iSolution1 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());

	do 
	{
		iSolution2 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());
	} 
	while(iSolution1 == iSolution2);

	if (dFitness[iSolution1] >= dFitness[iSolution2])
	{
		iWinner = iSolution1;
	} 
	else
	{
		iWinner = iSolution2;
	}

	return iWinner;
}

int TMOTools::GetSolutionUsingBinaryTournamentNischedMin(vector <TSolution *> &oSolutions, vector <double> &dFitness, vector <double> &dDistance)
{
int		iSolution1 = 0;
int		iSolution2 = 0;
int		iWinner = 0;

	assert(oSolutions.size() > 0);

	if (oSolutions.size() == 1) 
	{
		return 0;
	}

	// select solution 1
	iSolution1 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());

	do 
	{
		iSolution2 = (int)(((double)rand () / ((double)RAND_MAX + 1.0)) * oSolutions.size());
	} 
	while(iSolution1 == iSolution2);

	if (dFitness[iSolution1] < dFitness[iSolution2])
	{
		iWinner = iSolution1;
	} 
	else if (dFitness[iSolution1] > dFitness[iSolution2])
	{
		iWinner = iSolution2;
	}
	else if (dDistance[iSolution1] == -1)
	{
		iWinner = iSolution1;
	}
	else if (dDistance[iSolution2] == -1)
	{
		iWinner = iSolution2;
	}
	else if (dDistance[iSolution1] > dDistance[iSolution2])
	{
		iWinner = iSolution1;
	}
	else
	{
		iWinner = iSolution2;
	}

	return iWinner;
}

int TMOTools::GetRandomIndex(TSolutionsSet &oSolutionSet, vector <int> &iIndices)
{
int			 iCounter = 0;
int			 iIndex = -1;
bool		 bNoSolutions = false;
unsigned int i = 0;

	if (iIndices.size() == 0)
	{
		return -1;
	}

	iCounter = (int)floor(((double)rand() / (double)RAND_MAX) * iIndices.size() * 2.0);

	do
	{
		bNoSolutions = true;
		
		for(i = 0; i < iIndices.size(); i++)
		{
			if (oSolutionSet[iIndices[i]] != NULL)
			{
				bNoSolutions = false;
				iCounter = iCounter - 1;
				iIndex = i;
			}
		}
		
		if (bNoSolutions == true)
		{
			return -1;
		}
	} 
	while(iCounter > 0);

	return iIndices[iIndex];
}


bool TMOTools::DoMutate(double dProbability)
{
double		dRandomValue = 0.0;

	dRandomValue = (double)(rand() % 1000);
	dRandomValue /= 1000;

	if (dRandomValue < dProbability)
	{
		return true;
	} 
	else 
	{
		return false;
	}
}



bool TMOTools::FastNondominatedSort(TSolutionsSet &oSolutions, vector <IntVector> &iF)
{
int										i = 0;
int										j = 0;
unsigned int							k = 0;
int										iParentSize = 0;
int										iSolutionP = 0;
int										iSolutionQ = 0;
int										iPopulationSize = 0;
vector <IntVector>						iS;
vector <int>							iN;
vector <bool>							bA;
TSolution								*pP = NULL;
TSolution								*pQ = NULL;
TCompare								eCompareResult;

	if (oSolutions.size() == 0) 
	{
		return false;
	}

	iPopulationSize = oSolutions.size();

	iS.resize(iPopulationSize);
	iN.resize(iPopulationSize);
	bA.resize(iPopulationSize);

	// Clear rank
	for(i = 0; i < iPopulationSize; i++) 
	{
		iN[i] = 0;
		iS[i].resize(0);
		bA[i] = false;
	}

	iF.resize(1);
	iF[0].resize(0);

	// Set fronts
	for(i = 0; i < iPopulationSize; i++) 
	{
		for(j = 0; j < iPopulationSize; j++) 
		{
			pP = (TSolution *)oSolutions[i];
			pQ = (TSolution *)oSolutions[j];

			eCompareResult = (*pP).Compare(*pQ);
			
			if (eCompareResult == _Dominating)
			{
				iS[i].push_back(j);
			}

			if (eCompareResult == _Dominated)
			{
				iN[i] += 1;
			}
		}

		// fill the first nondominated front
		if (iN[i] == 0) 
		{
			iF[0].push_back(i); 
			bA[i] = true;
		}
	}

	// Generate next fronts
	i = 1;
	while(iF[i - 1].size() > 0) 
	{
		// prepare next front
		iF.resize(i + 1);
		iF[i].resize(0);

		// scan front
		iParentSize = iF[i - 1].size();
		for(j = 0; j < iParentSize; j++)
		{
			iSolutionP = iF[i - 1][j];
			// check dominated solutions by solution from current front
			for(k = 0; k < iS[iSolutionP].size(); k++)
			{
				iSolutionQ = iS[iSolutionP][k];		
				iN[iSolutionQ] -= 1;

				if ((iN[iSolutionQ] == 0) && (bA[iSolutionQ] == false))
				{
					iF[i].push_back(iSolutionQ);
					bA[iSolutionQ] = true;
				}
			}
		}

		i += 1;
	}

	// debug
	for(i = 0; i < iPopulationSize; i++) 
	{
		assert(bA[iSolutionQ] == true);
	}

	return true;
}

bool TMOTools::ConstrainedFastNondominatedSort(TSolutionsSet &oSolutions, vector <IntVector> &iF)
{
int										i = 0;
int										j = 0;
unsigned int							k = 0;
int										iParentSize = 0;
int										iSolutionP = 0;
int										iSolutionQ = 0;
int										iPopulationSize = 0;
vector <IntVector>						iS;
vector <int>							iN;
vector <bool>							bA;
TDPAMConstrainedSolution				*pP = NULL;
TDPAMConstrainedSolution				*pQ = NULL;
TCompare								eCompareResult;

	if (oSolutions.size() == 0) 
	{
		return false;
	}

	iPopulationSize = oSolutions.size();

	iS.resize(iPopulationSize);
	iN.resize(iPopulationSize);
	bA.resize(iPopulationSize);

	// Clear rank
	for(i = 0; i < iPopulationSize; i++) 
	{
		iN[i] = 0;
		iS[i].resize(0);
		bA[i] = false;
	}

	iF.resize(1);
	iF[0].resize(0);

	// Set fronts
	for(i = 0; i < iPopulationSize; i++) 
	{
		for(j = 0; j < iPopulationSize; j++) 
		{
			pP = (TDPAMConstrainedSolution *)oSolutions[i];
			pQ = (TDPAMConstrainedSolution *)oSolutions[j];

			eCompareResult = (*pP).ConstrainedCompare(*pQ);
			
			if (eCompareResult == _Dominating)
			{
				iS[i].push_back(j);
			}

			if (eCompareResult == _Dominated)
			{
				iN[i] += 1;
			}
		}

		// fill the first nondominated front
		if (iN[i] == 0) 
		{
			iF[0].push_back(i); 
			bA[i] = true;
		}
	}

	// Generate next fronts
	i = 1;
	while(iF[i - 1].size() > 0) 
	{
		// prepare next front
		iF.resize(i + 1);
		iF[i].resize(0);

		// scan front
		iParentSize = iF[i - 1].size();
		for(j = 0; j < iParentSize; j++)
		{
			iSolutionP = iF[i - 1][j];
			// check dominated solutions by solution from current front
			for(k = 0; k < iS[iSolutionP].size(); k++)
			{
				iSolutionQ = iS[iSolutionP][k];		
				iN[iSolutionQ] -= 1;

				if ((iN[iSolutionQ] == 0) && (bA[iSolutionQ] == false))
				{
					iF[i].push_back(iSolutionQ);
					bA[iSolutionQ] = true;
				}
			}
		}

		i += 1;
	}

	// debug
	for(i = 0; i < iPopulationSize; i++) 
	{
		assert(bA[iSolutionQ] == true);
	}

	return true;
}

void TMOTools::RecursiveQuickSortSolutionsMax(int iStartPos, int iEndPos, TSolutionsSet &oSolutionSet, vector <double> &dFitness)
{
int				iCurrentStartPos = 0;
int				iCurrentEndPos = 0; 
double			d = 0;
double			dSplitValue = 0.0;
TSolution		*pTmp = NULL;

	iCurrentStartPos = iStartPos;
	iCurrentEndPos = iEndPos;
	
	dSplitValue = dFitness[(iStartPos + iEndPos) / 2];

	do {
		while(dFitness[iCurrentStartPos] > dSplitValue) 
		{
			iCurrentStartPos += 1;
		}

		while(dFitness[iCurrentEndPos] < dSplitValue) 
		{
			iCurrentEndPos -= 1;
		}
		
		if (iCurrentStartPos <= iCurrentEndPos)
		{
			pTmp = oSolutionSet[iCurrentStartPos]; 
			oSolutionSet[iCurrentStartPos] = oSolutionSet[iCurrentEndPos]; 
			oSolutionSet[iCurrentEndPos] = pTmp;

			d = dFitness[iCurrentStartPos]; 
			dFitness[iCurrentStartPos] = dFitness[iCurrentEndPos]; 
			dFitness[iCurrentEndPos] = d;

			iCurrentStartPos += 1;
			iCurrentEndPos -= 1;
		}
	} 
	while(iCurrentStartPos <= iCurrentEndPos);

	if (iStartPos < iCurrentEndPos)  
	{
		RecursiveQuickSortSolutionsMax(iStartPos, iCurrentEndPos, oSolutionSet, dFitness);
	}

	if (iEndPos > iCurrentStartPos)  
	{
		RecursiveQuickSortSolutionsMax(iCurrentStartPos, iEndPos, oSolutionSet, dFitness);
	}
}

void TMOTools::QuickSortSolutionUsingFitnessMax(TSolutionsSet &oSolutionSet, vector <double> &dFitness)
{
	// debug	
	assert(oSolutionSet.size() == dFitness.size());

	RecursiveQuickSortSolutionsMax(0, oSolutionSet.size() - 1, oSolutionSet, dFitness);
}


void  TMOTools::RecursiveQuickSortObjectiveMax(int iObjective, int iStartPos,int iEndPos, TSolutionsSet &oSolutions, IntVector &iIndices)
{
int			iCurrentStartPos = 0;
int			iCurrentEndPos = 0; 
int			i = 0;
double		dSplitValue = 0.0;

	if (Objectives[iObjective].bActive == false) 
	{
		return;
	}

	iCurrentStartPos = iStartPos; 
	iCurrentEndPos = iEndPos;
	
	dSplitValue = (oSolutions[iIndices[(iStartPos + iEndPos) / 2]]) -> ObjectiveValues[iObjective];

	do {
		if (Objectives[iObjective].ObjectiveType == _Max) {
			while(oSolutions[iIndices[iCurrentStartPos]] -> ObjectiveValues[iObjective] < dSplitValue) {
				iCurrentStartPos += 1;
			}

			while(oSolutions[iIndices[iCurrentEndPos]] -> ObjectiveValues[iObjective] > dSplitValue) {
				iCurrentEndPos -= 1;
			}
		} else {
			while(oSolutions[iIndices[iCurrentStartPos]] -> ObjectiveValues[iObjective] > dSplitValue) {
				iCurrentStartPos += 1;
			}

			while(oSolutions[iIndices[iCurrentEndPos]] -> ObjectiveValues[iObjective] < dSplitValue) {
				iCurrentEndPos -= 1;
			}		
		}

		if (iCurrentStartPos <= iCurrentEndPos)
		{
			i = iIndices[iCurrentStartPos]; 
			iIndices[iCurrentStartPos] = iIndices[iCurrentEndPos]; 
			iIndices[iCurrentEndPos] = i;

			iCurrentStartPos += 1;
			iCurrentEndPos -= 1;
		}
	} 
	while(iCurrentStartPos <= iCurrentEndPos);

	if (iStartPos < iCurrentEndPos)  
	{
		RecursiveQuickSortObjectiveMax(iObjective, iStartPos, iCurrentEndPos, oSolutions, iIndices);
	}

	if (iEndPos > iCurrentStartPos)  
	{
		RecursiveQuickSortObjectiveMax(iObjective, iCurrentStartPos, iEndPos, oSolutions, iIndices);
	}
}

bool TMOTools::QuickSortByObjectivesMax(TSolutionsSet &oSolutions, IntVector &iSelectedIndices, vector <IntVector> &iSortTable)
{
int				i = 0;
unsigned int	j = 0;
int				iSetSize = 0;

	// prepare sort table
	iSortTable.resize(NumberOfObjectives);
	
	// prepare table
	if (iSelectedIndices.size() == 0)
	{
		// get all indices	
		for(i = 0; i < NumberOfObjectives; i++)
		{
			iSortTable[i].resize(0);
		}

		for(j = 0; j < oSolutions.size(); j++)
		{
			if (oSolutions[i] != NULL) 
			{ 
				for(i = 0; i < NumberOfObjectives; i++)
				{
					iSortTable[i].push_back(j);
				}
			}
		}

		iSetSize = oSolutions.size() - 1;
	} 
	else
	{
		// sort only selected
		for(i = 0; i < NumberOfObjectives; i++)
		{
			iSortTable[i].resize(iSelectedIndices.size());
		}

		for(i = 0; i < NumberOfObjectives; i++)
		{
			for(j = 0; j < iSelectedIndices.size(); j++)
			{
				iSortTable[i][j] = iSelectedIndices[j];
			}
		}

		iSetSize = iSelectedIndices.size() - 1;
	}
	
	// sort
	for(i = 0; i < NumberOfObjectives; i++)
	{
		RecursiveQuickSortObjectiveMax(i, 0, iSetSize, oSolutions, iSortTable[i]);
	}

	return true;	
}


void  TMOTools::RecursiveQuickSortByValueMax(int iStartPos, int iEndPos, TSolutionsSet &oSolutionSet, vector <double> &dValue, vector <int> &iIndices)
{
int				iCurrentStartPos = 0;
int				iCurrentEndPos = 0; 
int				i = 0;
double			dSplitValue = 0.0;
TSolution		*pTmp = NULL;

	iCurrentStartPos = iStartPos;
	iCurrentEndPos = iEndPos;
	
	dSplitValue = dValue[iIndices[(iStartPos + iEndPos) / 2]];

	do 
	{
		while(dValue[iIndices[iCurrentStartPos]] > dSplitValue) 
		{
			iCurrentStartPos += 1;
		}

		while(dValue[iIndices[iCurrentEndPos]] < dSplitValue) 
		{
			iCurrentEndPos -= 1;
		}
		
		// swap element
		if (iCurrentStartPos <= iCurrentEndPos)
		{
			i = iIndices[iCurrentStartPos]; 
			iIndices[iCurrentStartPos] = iIndices[iCurrentEndPos]; 
			iIndices[iCurrentEndPos] = i;

			iCurrentStartPos += 1;
			iCurrentEndPos -= 1;
		}
	} 
	while(iCurrentStartPos <= iCurrentEndPos);

	if (iStartPos < iCurrentEndPos)
	{
		RecursiveQuickSortByValueMax(iStartPos, iCurrentEndPos, oSolutionSet, dValue, iIndices);
	}

	if (iEndPos > iCurrentStartPos)  
	{
		RecursiveQuickSortByValueMax(iCurrentStartPos, iEndPos, oSolutionSet, dValue, iIndices);
	}
}

void TMOTools::QuickSortByValueMax(TSolutionsSet &oSolutionSet, vector <double> &dValue, vector <int> &iIndices)
{
IntVector		iTmpIndices;
unsigned int	i = 0;

	if (iIndices.size() == 0)
	{
		// whole population
		iTmpIndices.resize(0);
		for(i = 0; i < oSolutionSet.size(); i++)
		{
			if (oSolutionSet[i] != NULL) 
			{
				iTmpIndices.push_back(i);
			}
		}
	}
	else
	{
		// selected population
		iTmpIndices.resize(iIndices.size());
		for(i = 0; i < iIndices.size(); i++)
		{
			iTmpIndices[i] = iIndices[i];	
		}
	}

	RecursiveQuickSortByValueMax(0, iTmpIndices.size() - 1, oSolutionSet, dValue, iTmpIndices);
	
	iIndices.resize(iTmpIndices.size());
	for(i = 0; i < iTmpIndices.size(); i++)
	{
		iIndices[i] = iTmpIndices[i];	
	}

	return;
}
