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


#if !defined(AFX_CONSTRAINEDNSGAIIC_H__292694EC_4936_4A67_AD07_277A0D9E6482__INCLUDED_)
#define AFX_CONSTRAINEDNSGAIIC_H__292694EC_4936_4A67_AD07_277A0D9E6482__INCLUDED_

#include "global.h"
#include "weights.h"
#include "momhsolution.h"
#include "nondominatedset.h"
#include "momethod.h"
#include "paretobase.h"
#include "nsgaiic.h"
#include "momhconstrainedsolution.h"

/** Implements Constrained version of Nondominated Sorting Genetic Algorithm II - controled approach 
*   as propsoed by Deb, Pratap, Agarwal, Meyarivan
**/
template <class TProblemSolution> class TConstrainedNSGAIIC : public TNSGAIIC <TProblemSolution>
{
public:
    /** Call to start computations
     **/
	void Run();
};

/** Visualize computation process
 **/
template <class TProblemSolution> class TDisplayConstrainedNSGAIIC : public TConstrainedNSGAIIC <TProblemSolution> 
{
protected:
	virtual void End ();
	virtual void NewNondominatedSolutionFound ();
	virtual void NewGenerationFound ();
};

#include "constrainednsgaiic.cpp"

#endif // !defined(AFX_CONSTRAINEDNSGAIIC_H__292694EC_4936_4A67_AD07_277A0D9E6482__INCLUDED_)
