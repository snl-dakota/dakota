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

#include "trandomgenerator.h"

TRandomGenerator::TRandomGenerator()
{
	m_bInit = false;
	m_lSeed = 797L;
	m_lOut = 0;
}

TRandomGenerator::~TRandomGenerator()
{
	
}

TRandomGenerator::TRandomGenerator(long lSeed)
{
	m_bInit = false;
	m_lSeed = lSeed;
	m_lOut = 0;
}

long TRandomGenerator::GetRandomValue()
{
int       i;

	if (m_bInit == false) {
		// initialise table of the generator
		for(i = 0; i < RAND_TABLE_LENGTH; i++) {
			m_lSeed = (RAND_IA * m_lSeed + RAND_IC) % RAND_IM;
			m_lTable[i] = m_lSeed;
		}
		m_lSeed = (RAND_IA * m_lSeed + RAND_IC) % RAND_IM;
		m_lOut = m_lSeed;

		m_bInit = true;
	}

	// get value from table and update it
	i = ((RAND_TABLE_LENGTH * m_lOut) / RAND_IM) % RAND_TABLE_LENGTH;
	m_lOut = m_lTable[i];
	m_lSeed = (RAND_IA * m_lSeed + RAND_IC) % RAND_IM;
	m_lTable[i] = m_lSeed;
	
	return(m_lOut);
}

double TRandomGenerator::GetDoubleRND(double dMaxScope)
{
double dValue;
	
	dValue = GetRandomValue();
	// scale to range <0;1> 
	dValue = dValue / RAND_RANDMAX;
	
	return(dValue*dMaxScope);
}

long TRandomGenerator::GetLongRND(long lMaxScope)
{
double dValue;
	
	dValue = GetRandomValue();
	// scale to range <0;1> 
	dValue = dValue / ((double)(RAND_RANDMAX) + 1.0);
	
	return((long)(dValue*(lMaxScope + 1)));
}

void TRandomGenerator::SetSeed(long lSeed)
{
	m_bInit = false;
	m_lSeed = lSeed % (RAND_RANDMAX - 1);
	m_lOut = 0;
}
