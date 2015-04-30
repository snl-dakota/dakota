#ifndef __TINDEXEDPOINT_H_
#define __TINDEXEDPOINT_H_

#include "solution.h"

/** Simple copy of solution used in NSGA.  
 *  
 * Because of many copy operations in NSGA these operation are performed on very simply
 * image of solution contains only objective values. It speedups whole process. 
 **/
class TIndexedSolutionPoint : public TSolution 
{
public:
	/** Used in NSGA ass feedback to list position */
	int m_iListPosition;

	/** Copy constructor 
     * 
     * @param oPoint souce point 
     **/
	TIndexedSolutionPoint(TIndexedSolutionPoint& oPoint);

    /** Convert oObject to this object 
     *
     * @param oPoint souce solution 
     **/
	TIndexedSolutionPoint(TSolution& oObject);

    /** Create empty object 
     **/
	TIndexedSolutionPoint();

    /** Destroy object 
     **/
	virtual ~TIndexedSolutionPoint();
};

#endif 
