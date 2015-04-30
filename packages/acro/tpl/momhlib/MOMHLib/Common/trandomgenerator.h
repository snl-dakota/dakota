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

#ifndef __TRANDOMGENERATOR_H_
#define __TRANDOMGENERATOR_H_


/** Parameter for randomizer **/
const long				RAND_IM = 714025L;
/** Parameter for randomizer **/
const long				RAND_IA = 1366L;
/** Parameter for randomizer **/
const long				RAND_IC = 150889L;
/** Parameter for randomizer **/
const long				RAND_TABLE_LENGTH = 97;
/** Maximal value of number generated using this generator **/
const long				RAND_RANDMAX = RAND_IM - 1;

/** This class implements pseudo-random values generator. 
 *
 * This class replace system rand() function. Now it is possible to customize
 * random number generation.
 **/
class TRandomGenerator  
{
private:

    /** Last generated random number.
     **/
    long         m_lOut;

    /** Table used for storing last generated random values.
     * This table is used for generating next number on the base of history. 
    **/
    long		 m_lTable[RAND_TABLE_LENGTH];

    /** Internal value that contains information about generator start.
     **/
    bool		 m_bInit;

    /** Seed value used for initialising generator
     **/
    long		 m_lSeed;

protected:
    /** Get Random value from generator. 
     *
     * Get Random value from generator. Value is in range 0-RAND_RANDMAX
     **/
	long GetRandomValue();

public:

    /** Initialise generator number of given value
     *
     * This function restart generator with given seed value. 
     *
     * @param lSeed seed value (initialize generator)
     **/
	void SetSeed(long lSeed);

    /** Get long value from given range.
     *
     * This function retuns integer value from 0 to lMaxScope <0;MaxScope>
     * @param lMaxScope upper bound of scope
     * @return random value
     **/
	long GetLongRND(long lMaxScope);

    /** Get double value from given range.
     *
     * This function retuns real value from 0 to lMaxScope <0;MaxScope>
     *
     * @param dMaxScope upper bound of scope
     * @return random value
     **/
	double GetDoubleRND(double dMaxScope);

    /** Construct and initialize generator object with given seed value.
     *
     * @param lSeed initial generator value
     **/
	TRandomGenerator(long lSeed);

    /** Construct and initialize generator object with default seed value.
     **/
	TRandomGenerator();

    /** Destruct and release generator object
     **/
	virtual ~TRandomGenerator();
};

#endif
