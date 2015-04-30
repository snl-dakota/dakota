#include "tindexedpoint.h"

TIndexedSolutionPoint::TIndexedSolutionPoint()
{
	m_iListPosition = 0;
}

TIndexedSolutionPoint::~TIndexedSolutionPoint()
{

}

TIndexedSolutionPoint::TIndexedSolutionPoint(TIndexedSolutionPoint& oObject)
{
	ObjectiveValues.resize(NumberOfObjectives);
	for(int i = 0; i < NumberOfObjectives; i++) 
	{
		ObjectiveValues [i] = oObject.ObjectiveValues [i];
	}

	m_iListPosition = oObject.m_iListPosition;
}

TIndexedSolutionPoint::TIndexedSolutionPoint(TSolution& oObject)
{
	ObjectiveValues.resize(NumberOfObjectives);
	for(int i = 0; i < NumberOfObjectives; i++) 
	{
		ObjectiveValues [i] = oObject.ObjectiveValues [i];
	}
}
