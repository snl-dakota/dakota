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

#if !defined(AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_)
#define AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_

#include "nondominatedset.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

const int	MO_NDS_LIST = 1;
const int	MO_NDS_QUAD_TREE = 2;

/**	Abstract class - generalization of all multiple objective
*	methods
*
*	It is assumed that the goal of the method is to generate
*	a set of potentially nondominated (Pareto-optimal) solutions */
template <class TProblemSolution> class TMOMethod
{
private:
	/** Used by OnlineOfflineEvaluation */
	TWeightsSet EvaluationWeightsSet;
	/** Used by OnlineOfflineEvaluation */
	double AverageOnlineScalarizingFunctionValue;
	/** Used by OnlineOfflineEvaluation */
	double AverageOnlineTime;
	/** Used by OnlineOfflineEvaluation */
	time_t OfflineTime;
	/** Used by OnlineOfflineEvaluation */
	time_t PreviousTime;
	/** Used by OnlineOfflineEvaluation */
	double TimeUnit;
	/** Used by OnlineOfflineEvaluation */
	TPoint ReferencePoint;
	/** Used by OnlineOfflineEvaluation */
	double MaximumEffectivenessIndex;
	/** Used by OnlineOfflineEvaluation */
	TScalarizingFunctionType EvaluationScalarizingFunctionType;

protected:

	/** Used by OnlineOfflineEvaluation 
	*
	*	If true Run method should continue optimization
	*	until bStop field is set to true.
	*	Other stopping criteria should not be used.
	*/
	bool bOnlineOfflineEvaluating;

	/** It is assumed that by setting this field to true 
	*	the method can be stopped */
	bool bStop;

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TMOMetod 
	*
	*	Specialization of TMOMethod class 
	*	should call this method at very begining of 
	*	the run of the optimization */
	virtual void Start () {};

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TMOMetod 
	*
	*	Specialization of TMOMethod class 
	*	should call this method at the end of 
	*	the run of the optimization */
	virtual void End () {};

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TMOMetod 
	*
	*	Specialization of TMOMethod class 
	*	should call this method whenever a new solution
	*	is generated */
	virtual void NewSolutionGenerated (TProblemSolution& NewSolution);

	/** Virtual callback function to be (optionally) defined in 
	*	a specialization of TMOMetod 
	*
	*	Specialization of TMOMethod class 
	*	should call this method whenever a new solution
	*	is added to the nondominated set */
	virtual void NewNondominatedSolutionFound () {};
public:
    /** Default constructor */
	TMOMethod () 
    {
		bOnlineOfflineEvaluating = false;
		bStop = false;
        pNondominatedSet = NULL;
        InitializeNondominatedSet(MO_NDS_LIST);
	}

    /** Destroy object - release nondominated set **/
    ~TMOMethod()
    {
        if (pNondominatedSet != NULL)
        {
			pNondominatedSet->DeleteAll ();
            delete pNondominatedSet;
            pNondominatedSet = NULL;
        }
    }

    /** Initialize type of nondominated set (must be set on list or quad tree) 
    *
    *  List or QuadTree can be choosed. Default choose is list.
    *
    *  @param iType if MO_NDS_LIST list will be choosen, if MO_NDS_QUAD_TREE the quad tree 
    *  @return true - ok, false - error
    **/
    bool InitializeNondominatedSet(int iType);

	/** Abstract method. Runs the multiple objective method 
	*
	*	The method has to be defined in specialization of
	*	TMOMethod.
	*	Note that the method is parameter free. 
	*	Parameters of particular multiple objective methods
	*	should be set in another way. For example specialization
	*	of TMOMethod may have a sperate method to set its
	*	parameters or the parameters may be set in constructor of 
	*	the specialization */
	virtual void Run () = 0;

	/** Runs a MOSA-like algorithm
	*
	*	Call single objective optimization for the predefined set of weight
	*	vectors */
	virtual void RunMOSALike (TScalarizingFunctionType ScalarizingFunctionType,
		int WeightSpaceSamplingParameter);

	/**	Runs single objective optimization of scalarizing function
	*
	*	The scalarizing function is defined by ReferencePoint
	*	ScalarizingFunctionType and WeightVector.
	*	This method is optional. It may be useful while testing
	*	your implementation and while evaluation the multiple
	*	objective method. 
	*	Note that this method will return object of TProblemSolution
	*	only if it has a = operator defined. By default it will 
	*	return object of TPoint class. */
	virtual TProblemSolution SingleObjectiveOptimization (TPoint ReferencePoint, 
		TScalarizingFunctionType ScalarizingFunctionType, 
		TWeightVector& WeightVector,
		bool bUpdateNondominatedSet) 
	{
		TProblemSolution Solution;
		return Solution;
	};

	/** Set of potentially nondominated solutions - the outcome
	*	of the method. Default choose is a list.  
    */
	TNondominatedSet *pNondominatedSet;

    /** Type of nondominated set MO_NDS_LIST or MO_NDS_QUAD_TREE **/
    int iTypeOfNondominatedSet;

	/** Method used to evaluate computation effectiveness of a multiple objective
	*	method in comparison to its single objective correspondent.	
	*
	*	Calls virtual method TMOMethod::SingleObjectiveOptimization (...)
	*	NumberOfWeightVectors with randomly generated weight vectors.
	*	Then calls TMOMethod::Run (). TMOMethod::Run () is stopped when
	*	the average quality of solutions generated by the multiple
	*	objective method being the best on the set of the scalarizing 
	*	functions is not worse than the quality of solutions generated by the
	*	single objective method. TMOMethod::Run () is also stopped when
	*	the ration 
	*	Running_time_of_the_multiple_objective_method / 
	*	Average_running_time_of_the_single_objective_method
	*	exceedes value of MaximumEffectivenessIndex field 
	*	The method measures their running time using physical CPU time.
	*	Returns effectiveness index. If MaximumEffectivenessIndex stopping
	*	fired, then returns value lower than 0.
	*/
	double OnlineOfflineEvaluation (int NumberOfWeightVectors, 
		TScalarizingFunctionType ScalarizingFunctionType,
		double MaximumEffectivenessIndex);
};

#include "momethod.cpp"

#endif // !defined(AFX_MOMETHOD_H__46C81620_9636_11D4_829A_000000000000__INCLUDED_)
