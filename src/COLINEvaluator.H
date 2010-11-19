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
 * \file SerialEvaluator.h
 *
 * Defines the \c SerialEvaluator class.
 *
 **/

#ifndef COLINEvaluator_h
#define COLINEvaluator_h

#include <colin/eval/ConcurrentEvaluator.h>

namespace Dakota
{

class COLINEvaluator : public colin::ConcurrentEvaluator
{
public:
   COLINEvaluator(evalMngrID_t id);

   virtual ~COLINEvaluator()
   {}

   static EvaluationManager_Base* create(size_t mngr_id)
   { return new COLINEvaluator(mngr_id); }

protected: // methods

private:  // data

};


} // namespace colin

#endif // defined COLINEvaluator_h
