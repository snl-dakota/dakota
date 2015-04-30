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

/**
 * \file EvaluationID.h
 *
 * Defines the colin::EvaluationID class.  
 */

#ifndef colin_EvaluationID_h
#define colin_EvaluationID_h

#include <acro_config.h>
#include <colin/QueueManager.h>

// forward declaration of stream output function
std::ostream& operator<<(std::ostream& os, const colin::EvaluationID& id);

namespace colin
{

class EvaluationID
{
   friend std::ostream& ::operator<<
      (std::ostream& os, const colin::EvaluationID& id);

public:
   EvaluationID()
      : m_rank(0), m_eMngr(0), m_solver(0), m_queue(0), m_id(0)
   {}

   EvaluationID( const QueueManager::evalMngrID_t eID, 
                 const QueueManager::solverID_t sID,
                 const QueueManager::queueID_t qID );


   bool operator==(const EvaluationID& rhs) const;

   bool operator<(const EvaluationID& rhs) const;

   bool empty() const
   { return m_id == 0; }

   QueueManager::evalMngrID_t  eManager() const
   { return m_eMngr; }

   QueueManager::solverID_t    solver() const
   { return m_solver; }

   QueueManager::queueID_t     queue()  const
   { return m_queue; }

   int rank() const
   { return m_rank; }

private:
   static size_t getNewFEvalID();

private:
   int                         m_rank;
   QueueManager::evalMngrID_t  m_eMngr;
   QueueManager::solverID_t    m_solver;
   QueueManager::queueID_t     m_queue;
   size_t                      m_id;
};

} // namespace colin


#endif // colin_EvaluationID_h
