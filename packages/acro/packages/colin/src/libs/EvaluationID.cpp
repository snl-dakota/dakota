/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#include <colin/EvaluationID.h>
#include <colin/ColinGlobals.h>

#include <utilib/exception_mngr.h>


using std::map;
using std::cerr;
using std::endl;


///
std::ostream& operator<<(std::ostream& os, const colin::EvaluationID& id)
{
   os << "(" << id.m_rank << "-" << id.m_eMngr << ":" 
      << id.m_solver << "-" << id.m_queue 
      << ":" << id.m_id << ")";
   return os;
}


namespace colin
{

//======================================================================
// EvaluationID member functions
//======================================================================

EvaluationID::EvaluationID( const QueueManager::evalMngrID_t eID, 
                            const QueueManager::solverID_t sID,
                            const QueueManager::queueID_t qID )
   : m_rank(ColinGlobals::processor_id()),
     m_eMngr(eID),
     m_solver(sID),
     m_queue(qID),
     m_id(getNewFEvalID())
{}


size_t EvaluationID::getNewFEvalID()
{
   static size_t lastFEvalID = 0;
   return ++lastFEvalID;
}


bool EvaluationID::operator==(const EvaluationID& rhs) const
{
   return ( m_id == rhs.m_id ) && ( m_rank == rhs.m_rank );
}


bool EvaluationID::operator<(const EvaluationID& rhs) const
{
   if (m_solver < rhs.m_solver) return true;
   if (m_solver > rhs.m_solver) return false;
   if (m_queue < rhs.m_queue) return true;
   if (m_queue > rhs.m_queue) return false;
   if (m_id < rhs.m_id) return true;
   if (m_id > rhs.m_id) return false;
   if (m_rank < rhs.m_rank) return true;
   if (m_rank > rhs.m_rank) return false;
   if (m_eMngr < rhs.m_eMngr) return true;
   if (m_eMngr > rhs.m_eMngr) return false;
   return false;
}


} // namespace colin
