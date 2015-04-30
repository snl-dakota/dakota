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

#ifndef __TQUADTREE_H_
#define __TQUADTREE_H_

#include "global.h"
#include "solution.h"
#include "nondominatedset.h"
#include "tbtree.h"
#include "tquadnode.h"

/** Implementation of nondominated set based on quad tree
 *
 * This nondominated set is based on vector of solution and multidimensional quad tree 
 * bulid on this vector. This solution significantly speedup updating of nondominated set. 
 * Quad Tree solution is much more sophisticated then simple list but also more difficult to implement.
 **/
template <class TProblemSolution> class TQuadTree : public TNondominatedSet
{
public:

    /** Removed solution will besorted or not **/
    bool m_bQueueSort;

   /** Save quad tree to stream.
     *  
     * This function allows to serialize quad tree structure.
     *
     * @param oStream output stream
     * @param iSpace number of spaces used in indentations generation
     * @return output stream
     **/
	ostream& Save(ostream& oStream, int iSpace = 0); 

   /** Update set using given solution
     * 
     * This function reduce nondominated set to given number of solutions.
     *
     * @param Solution possibly nondominated solution
     * @return if true solution is nondominated and set has been updated with this solution, false solution is dominated by solution in set
     **/
	virtual bool Update(TSolution& Solution);
    
    /** Merge two nondominated sets and give final nondominated set.
     * 
     * This function add two sets and eliminate dominated solutions from both sets.
     *
     * @param NondominatedSet possibly nondominated solution
     * @return if true there weren't dominated solution, if false some solutions have been eliminated 
     **/
	virtual bool Update(TNondominatedSet& NondominatedSet);

    /** This function choose random solution from set of solutions
     * 
     * Probability of choose for every solution should be equal. 
     *
     * @param pSolution reference to pointer where solution will be placed
     **/
	virtual void GetRandomSolution(TSolution* &pSolution);

    /** Delete all solutions from set.
     * 
     * Every solution in set is released and vector is reallocated to size 0. 
     **/
	virtual void DeleteAll();

   /** Reduce nondominated set to given size using clustering.
     * 
     * This function reduce nondominated set to given number of solutions.
     *
     * @param iPopulationSize target size of population
     * @param bScalarize if true the distances by objectives will be scalarized
     * @return if true ok, false - error
     **/
	virtual bool ReduceSetByClustering(int iPopulationSize, bool bScalarize);


    /** This method removes null entries from TQuadTree vector and rebuilds tree.
     * 
     * The method uses heuristic to build almost balanced quad tree in rebuild process. 
     *
     * @return if true ok, false - error
     **/
	bool CompactTree();

protected:

    /** Vector of indices used in computations **/
	typedef	vector<int>	IVector;

    /** Add node (solution) to QuadTree 
     *
     * This method add new solution or exist one. It attach new TQuadNode to given parent node
     * and appropriate place in tree structure.
     * 
     * @param TQuadNodePtr parent quad node where solution will be added
     * @param pSolution pointer to new solution or null if solution exist on TQuadTree vector
     * @param iListPosition position on TQuadTree vector (when exist solution will be added to tree)
     * @param bKSuccessor path to children position in tree structure (need for b-tree)
     * @return if true - ok, false - error
     **/
	bool AddNode(TQuadNodePtr pParentNode, 
				 TSolution *pSolution, 
				 int iListPosition, 
				 vector<bool> &bKSuccessor);

    /** Reinsert solutions placed in indices vector to quad tree 
     *
     * This methed calls AddExistSolution on every index in list.
     * 
     * @param iReinsertIndices vector of indicess to add
     * @return if true - ok, false - error
     **/
	bool ReinsertRemoved(vector<int> &iReinsertIndices);

    /** Method used to filter out of dominated solution from TQuadTree subnode flatten to vector of indices.
     *
     * This method check that solutions (pointed by indices) in vector are dominated
     * by new solution. Dominated solution are removed from TQuadTree vactor. 
     * 
     * @param iReinsertIndices vector of indicess to add
     * @return if true - ok, false - error
     **/
	bool VerifyRemoved(TSolution &oNewSolution, 
					   vector<int> &iReinsertIndices);
    
    /** Method delete quad tree node and stores all offsprings to vector.
     *
     * Method delete quad tree node and stores all offsprings to vector. Used in dominated 
     * removing quad node from tree.
     * 
     * @param pCurrentNode pointer to node that will be deleted
     * @param iReinsertIndices storage for offsprings (based on vector)
     * @return if true - ok, false - error
     **/
	bool DeleteNode(TQuadNodePtr &pCurrentNode,
					vector<int> &iReinsertIndices);

    /** Recursive browse tree and look for dominating solution.
     *
     * Quad tree is browsed for dominating and dominates elements
     * separately. The main increase of efficiency is achieved by 
     * browsing only selected subnodes.
     * 
     * @param oNewSolution new solution that will be add to set (or not if is dominated)
     * @param oQuadNode parent quad node
     * @param bIsDominated true if element is dominated by element that exist in tree
     * @return if true - ok, false - error
     **/
	bool FindRecursiveDominatingInTree(TSolution &oNewSolution,
									   TQuadNode &oQuadNode,
									   bool &bIsDominated);

    /** Browse tree and look for dominating solution.
     *
     * This method calls FindRecursiveDominatingInTree
     * 
     * @param oNewSolution new solution that will be add to set (or not if is dominated)
     * @param bIsDominated true if element is dominated by element that exist in tree
     * @return if true - ok, false - error
     **/
	bool FindDominatingInTree(TSolution &oNewSolution,
							  bool &bIsDominated);

    /** Recursive browse tree and look for dominated solutions.
     *
     * Quad tree is browsed for dominated elements if a such element 
     * will be found. These elements will be removed and offsprings will be placed in vector
     * iReinsertIndices. This vector will be verified by VerifyRemoved and offsprings are
     * filter out. If quad tree is balanced such solution is quite efficient but
     * in the worst case this solution has the same efficiency as list.
     * 
     * @param oNewSolution new solution that will be add to set (or not if is dominated)
     * @param pCurrentNode pointer to parent node (recursive browsing)
     * @param iReinsertIndices vector where offsprings of removed elements are stored 
     * @return if true - ok, false - error
     **/
	bool FindRecursiveAndRemoveDominatedInTree(TSolution &oNewSolution,
											   TQuadNodePtr &pCurrentNode,
											   vector<int> &iReinsertIndices);

    /** Recursive browse tree and look for dominated solutions.
     *
     * This method calls FindAndRemoveDominatedInTree with tree root.
     * 
     * @param oNewSolution new solution that will be add to set (or not if is dominated)
     * @param iReinsertIndices vector where offsprings of removed elements are stored 
     * @return if true - ok, false - error
     **/
	bool FindAndRemoveDominatedInTree(TSolution &oNewSolution,
									  vector<int> &iReinsertIndices);

    /** Recursively find place where AddNode can put new solution
     *
     * This function browse tree to find appropriate place for new point.
     * 
     * @param pCurrentNode pointer to parent node (recursive browsing)
     * @param pSolution solution that will be added
     * @return if true - ok, false - error
     **/
	bool AddRecursiveNewSolution(TQuadNodePtr &pCurrentNode,
								 TSolution *pSolution);

    /** Find place where AddNode can put new solution
     *
     * This function calls AddRecursiveNewSolution with tree root.
     * 
     * @param pSolution solution that will be added
     * @return if true - ok, false - error
     **/
	bool AddNewSolution(TSolution *pSolution);

    /** Recursively find place where AddNode can put existing solution
     *
     * This function browse tree to find appropriate place for new point.
     * 
     * @param pCurrentNode pointer to parent node (recursive browsing)
     * @param iListPosition position of existing element in TQuadTree vector
     * @return if true - ok, false - error
     **/
	bool AddRecursiveExistSolution(TQuadNodePtr &pCurrentNode,
								   int iListPosition);

    /** Recursively find place where AddNode can put existing solution
     *
     * This function calls AddExistSolution with tree root.
     * 
     * @param iListPosition position of existing element in TQuadTree vector
     * @return if true - ok, false - error
     **/
	bool AddExistSolution(int iListPosition);

    /** Detach pChildNode from pParentNode.
     *
     * This procedure detach child node from binary tree of parent node.
     * 
     * @param pParentNode pointer to parent node
     * @param pChildNode pointer to child node
     * @return if true - ok, false - error
     **/
	bool RemoveChild(TQuadNode *pParentNode, TQuadNode *pChildNode);

    /** Sort table of indices using heuristic. 
     *
     * This procedure prepare vactor of indices to add in such a way that
     * created sub-tree will be almost balanced. This procedure uses QueueSortObjective
     * to sort indices by every objective. Then using binary search method points are get from
     * sort tables (one table for every objective) starting on median elements. Then in every
     * half of tables are get next median elements (binary browsing) if they were not got previously.
     * Getting madian elements ensure that elements added to tree will split one of the dimension in space
     * on half. This function calls QueueSortObjective and QueueSortReinsert
     * 
     * @param iReinsertIndices vector of indices that will be sorted
     * @param pChildNode pointer to child node
     * @return if true - ok, false - error
     **/
	bool QueueSort(vector<int> &iReinsertIndices);

    /** Randomly mix table of indices. 
     *
     * Other heuristic used in preparing indices to reinsert. Much less efficient.
     * 
     * @param iReinsertIndices vector of indices that will be sorted
     * @param pChildNode pointer to child node
     * @return if true - ok, false - error
     **/
	bool QueueRandomly(vector<int> &iReinsertIndices);

    /** Sort elements in vector iIndices using map iPositions on given objective.  
     *
     * Quick sort procedure that sort iIndices. Method uses iPositions map to sort
     * elements.
     * 
     * @param iObjective index of objective used in sorting 
     * @param iStartPos start position used in moving elements by quick sort
     * @param iEndPos end position used in moving elements by quick sort
     * @param iIndices indices to sort on selected objective
     * @param iPositions map table that represent sorting order
     * @return if true - ok, false - error
     **/
	void QueueSortObjective(int iObjective, 
							int iStartPos,
							int iEndPos, 
							vector<int> &iIndices, 
							vector<int> &iPositions);

    /** Prepare iReinsertIndices to reinsert
     *
     * This method refill iReinsertIndices using median rule, oIndicesMatrix and oPositionMatrix
     * 
     * @param iStart start position used in moving elements by quick sort (usally 0)
     * @param iEnd end position used in moving elements by quick sort (usually size of solutions vector)  
     * @param oIndicesMatrix input value - vector of size equal to number of objectives, each element of vector is a vector containing elements to sort 
     * @param oPositionMatrix input value - map table to oIndicesMatrix (oIndicesMatrix is not changed but sorting is done by oPositionMatrix)
     * @param iReinsertIndices vector that contains references to solutions that should be sort.  
     * @param bInserted mask which values must be sort (used internally) 
     * @return if true - ok, false - error
     **/
	void QueueSortReinsert(int iStart, 
						   int iEnd, 
						   vector<IVector> &oIndicesMatrix, 
						   vector<IVector> &oPositionMatrix, 
						   vector<int> &iReinsertIndices, 
						   vector<bool> &bInserted);



    /** Pointer to root of quad tree **/
	TQuadNode	*m_pQuadRoot;

	/** Update nadir and ideal point values
     * 
     * Method updates nadir and ideal point approximation.
     * New point is taken into consideration.
     *
     * @param Solution new solution added to nondominated set
     **/
	void UpdateIdealAndNadir(TSolution& Solution);
	
    /** Update nadir point values
     * 
     * Method updates nadir point approximation. 
     * Nadir point is approximated on current nondominated set.  
     *
     * @param Solution solution to be removed
     **/
    void UpdateNadir(TSolution& Solution);

	// test variables;

public:

	/** Test variable used in efficiency measures **/
    double m_dUpdateTime;

    /** Test variable used in efficiency measures **/
	double m_dReinsertTime;

    /** Construct TQuadTree **/
	TQuadTree();

    /** Destruction of TQuadTree - tree is released here **/
	~TQuadTree() {
		if (m_pQuadRoot != NULL) {
			delete m_pQuadRoot;
			m_pQuadRoot = NULL;
		}
	};
};


#include "tquadtree.cpp"

#endif
