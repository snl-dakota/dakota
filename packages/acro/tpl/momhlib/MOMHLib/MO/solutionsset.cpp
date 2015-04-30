/*
Multiple Objective MetaHeuristics Library in C++ MOMHLib++
Copyright (C) 2001 Andrzej Jaszkiewicz

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

#include "solutionsset.h"
#include "weights.h"

void TSolutionsSet::DeleteAll () 
{
	unsigned int i; for (i = 0; i < size (); i++)
    {
		if ((*this)[i] != NULL)
        {
            delete (*this)[i];
            (*this)[i] = NULL;
        }
    }
	clear ();
}

void TSolutionsSet::OutperformanceCompare (TSolutionsSet& Set2, bool &bWeak2By1, bool &bStrong2By1, bool &bComplete2By1, bool &bWeak1By2, bool &bStrong1By2, bool &bComplete1By2) {

	unsigned int iCoveredInThis = 0;
	unsigned int iCoveredInSet2 = 0;
	unsigned int iDominatedInThis = 0;
	unsigned int iDominatedInSet2 = 0;
	
	vector<bool> ScannedInSet2;
	
	ScannedInSet2.resize (Set2.size (), false);
	
	unsigned int i; for (i = 0; i < size (); i++) {        
        if ((*this)[i] == NULL) {
            continue;
        }

		bool bScanned = false;
		unsigned int j; for (j = 0; (j < Set2.size ()) && !bScanned; j++) {
            if ((*this)[j] == NULL) {
                continue;
            }

            if (!ScannedInSet2 [j]) {
				TCompare ComparisonResult = (*this)[i]->Compare (*Set2 [j]);
				
				if ((ComparisonResult == _Dominating) || (ComparisonResult == _Equal)) {
					iCoveredInSet2++;
					ScannedInSet2 [j] = true;
					if (ComparisonResult == _Dominating) {
						iDominatedInSet2++;
					}
				}

				if ((ComparisonResult == _Dominated) || (ComparisonResult == _Equal)) {
					iCoveredInThis++;
					bScanned = true;
					if (ComparisonResult == _Dominated) {
						iDominatedInThis++;
					}
				}
			}
		}
	}
	
	bWeak1By2 = (iCoveredInThis == size ()) && (iCoveredInSet2 < Set2.size ());
	bWeak2By1 = (iCoveredInThis < size ()) && (iCoveredInSet2 == Set2.size ());
	bStrong1By2 = bWeak1By2 && (iDominatedInThis > 0);
	bStrong2By1 = bWeak2By1 && (iDominatedInSet2 > 0);
	bComplete1By2 = (iCoveredInThis == size ()) && (iCoveredInSet2 == 0);
	bComplete2By1 = (iCoveredInThis == 0) && (iCoveredInSet2 == Set2.size ());
}

void TSolutionsSet::CalculateCoverage (TSolutionsSet& Set2, double &CoverageByThis, double &CoverageBySet2) {
	int iCoveredInThis = 0;
	int iCoveredInSet2 = 0;
	
	vector<bool> ScannedInSet2;
	
	ScannedInSet2.resize(Set2.size (), false);

	int ithisSize = 0;
	int iSet2Size = 0;
	
	unsigned int j; for (j = 0; j < Set2.size (); j++) {
		if (Set2 [j] == NULL) {
			continue;
		}
		iSet2Size++;
	}

	unsigned int i; for (i = 0; i < size (); i++) {
        if ((*this)[i] == NULL) {
            continue;
        }

		ithisSize++;
        
		bool bScanned = false;
		unsigned int j; for (j = 0; (j < Set2.size ()) && !bScanned; j++) {
            if (Set2[j] == NULL) {
                continue;
            }

			if (!ScannedInSet2 [j]) {
				TCompare ComparisonResult = (*this)[i]->Compare (*Set2 [j]);
				
				if ((ComparisonResult == _Dominating) || (ComparisonResult == _Equal)) {
					iCoveredInSet2++;
					ScannedInSet2 [j] = true;
				}
				
				if ((ComparisonResult == _Dominated) || (ComparisonResult == _Equal)) {
					iCoveredInThis++;
					bScanned = true;
				}
			}
		}
	}
	
	CoverageByThis = (double)iCoveredInSet2 / (double)iSet2Size;
	CoverageBySet2 = (double)iCoveredInThis / (double)ithisSize;
}

double TSolutionsSet::AverageScalarizingFunction (TPoint& ReferencePoint,
												  TScalarizingFunctionType ScalarizingFunctionType,
												  TWeightsSet WeightsSet) {
	double Sum = 0;
	unsigned int k;
	for (k = 0; k < WeightsSet.size (); k++) {
        int iBest = -1;
		double MinScalarizingFunctionValue = 1e30;
		unsigned int i; for (i = 0; i < size (); i++) {
			if ((*this)[i] == NULL) {
				continue;
			}
			double ScalarizingFunctionValue;

			switch (ScalarizingFunctionType) {
			case _Linear :
				ScalarizingFunctionValue = (*this)[i]->LinearScalarizingFunction (WeightsSet [k], ReferencePoint);
				break;
			case _Chebycheff :
				ScalarizingFunctionValue = (*this)[i]->ChebycheffScalarizingFunction (WeightsSet [k], ReferencePoint);
				break;
			}
			
			if (ScalarizingFunctionValue < MinScalarizingFunctionValue) {
				MinScalarizingFunctionValue = ScalarizingFunctionValue;
				iBest = i;
			}
		}
		Sum += MinScalarizingFunctionValue;

	}

	Sum /= (k);
	return Sum;
}

void TSolutionsSet::UpdateIdealNadir()
{
	// For all points
	unsigned int i; for (i = 0; i < size (); i++) {
        if ((*this)[i] == NULL) {
            continue;
        }

        // If first point
		if (i == 0) {
			int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				ApproximateIdealPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
				ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
			}
		}
		else {
			int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
				if (Objectives [iobj].bActive) {
					if (Objectives [iobj].ObjectiveType == _Max) {
						if (ApproximateIdealPoint.ObjectiveValues [iobj] < (*this)[i]->ObjectiveValues [iobj]) {
							ApproximateIdealPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
						}
						if (ApproximateNadirPoint.ObjectiveValues [iobj] > (*this)[i]->ObjectiveValues [iobj])
							ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
					}
					if (Objectives [iobj].ObjectiveType == _Min) {
						if (ApproximateIdealPoint.ObjectiveValues [iobj] > (*this)[i]->ObjectiveValues [iobj])
							ApproximateIdealPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
						if (ApproximateNadirPoint.ObjectiveValues [iobj] < (*this)[i]->ObjectiveValues [iobj])
							ApproximateNadirPoint.ObjectiveValues [iobj] = (*this)[i]->ObjectiveValues [iobj];
					}
				}
			}
		}
	}
}

void TSolutionsSet::UpdateIdealNadir (TSolution& Solution) 
{	
	int iobj; for (iobj = 0; iobj < NumberOfObjectives; iobj++) {
		if (Objectives [iobj].bActive) {
			if (Objectives [iobj].ObjectiveType == _Max) {
				if (ApproximateIdealPoint.ObjectiveValues [iobj] < Solution.ObjectiveValues [iobj])
					ApproximateIdealPoint.ObjectiveValues [iobj] = Solution.ObjectiveValues [iobj];
				if ((ApproximateNadirPoint.ObjectiveValues [iobj] > Solution.ObjectiveValues [iobj]) || (size () == 0))
					ApproximateNadirPoint.ObjectiveValues [iobj] = Solution.ObjectiveValues [iobj];
			}
			if (Objectives [iobj].ObjectiveType == _Min) {
				if (ApproximateIdealPoint.ObjectiveValues [iobj] > Solution.ObjectiveValues [iobj])
					ApproximateIdealPoint.ObjectiveValues [iobj] = Solution.ObjectiveValues [iobj];
				if ((ApproximateNadirPoint.ObjectiveValues [iobj] < Solution.ObjectiveValues [iobj]) || (size () == 0))
					ApproximateNadirPoint.ObjectiveValues [iobj] = Solution.ObjectiveValues [iobj];
			}
		}
	}
}

void TSolutionsSet::Save(char* FileName)
{
	fstream Stream (FileName, ios::out);

	unsigned int i; for (i = 0; i < size (); i++) {
        if ((*this)[i] == NULL) {
            continue;
        }

        (*this) [i]->Save (Stream);
		Stream << '\n';
	}

	Stream.close ();
}

void TSolutionsSet::Load(char* FileName)
{
	fstream Stream (FileName, ios::in);

	while (Stream.rdstate () == ios::goodbit) {
		TSolution* Solution = new TSolution;

		Solution->Load (Stream);

		if (Stream.rdstate () == ios::goodbit) {
			push_back (Solution);
			// Read the rest of the line
			char c;
			do {
				Stream.get (c);
			}
			while (c != '\n');
		}
		else
			delete Solution;

	}

	Stream.close ();
}
