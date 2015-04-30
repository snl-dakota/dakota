/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file QueueList.h
 *
 * Defines the utilib::QueueList class
 */

#ifndef utilib_QueueList_h
#define utilib_QueueList_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/LinkedList.h>

namespace utilib {

/**
 * A data type that defines a queue using a doubly-linked
 * list data structure.  This class is a simple derivation of 
 * LinkedList that sets the the queue mode.
 */
template <class T>
class QueueList : public LinkedList<T>
{
public:

  /// Empty constructor
  QueueList() : LinkedList<T>() {this->mode = this->queueLL;}

};

} // namespace utilib

#endif
