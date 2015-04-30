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
//
// ThreadObj.cpp
//

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <pebbl/ThreadObj.h>

using namespace std;

namespace pebbl {


int ThreadObj::num_threads=0;

#ifdef ACRO_HAVE_MPI
ThreadObj::ThreadObj(int Group_, double bias_, MPI_Comm /*comm_*/)
 : tag(AnyMsg)
#else
ThreadObj::ThreadObj(int Group_, double bias_)
#endif
{
  //  name = name_string;
  // This looks weird (JE): sprintf(name,"Thd%d",num_threads);
  name = NULL;
  id = num_threads++; 
  state_flag = ThreadReady;
  bias = bias_; 
  priority = prev_priority = 0.0; 
  Group = Group_;
  last_reset = 0.0; 
  priority_ctr = 0;
  nruns = 0;
  run_time = 0.0;
  defaultControlParam = 1.0;
}


#ifdef ACRO_HAVE_MPI
ThreadObj::ThreadObj(MPI_Comm /*comm_*/)
 : tag(AnyMsg)
#else
ThreadObj::ThreadObj()
#endif
{
  // name = name_string;
  // Weird-looking (JE) sprintf(name,"Thd%d",num_threads);
  name = NULL;
  id = num_threads++; 
  state_flag = ThreadReady;
  bias = 0; 
  priority = prev_priority = 0.0; 
  Group = 0;
  last_reset = 0.0; 
  priority_ctr = 0;
  nruns = 0;
  run_time = 0.0;
  defaultControlParam = 1.0;
}


void ThreadObj::dump(std::ostream& s)
{
  s << "Dump of ThreadObj:" << endl;
  s << ":   Name  = " << name << endl;
  s << ":   ID    = " << id << endl;
  s << ":   State = " << state_flag << endl;
#ifdef ACRO_HAVE_MPI
  s << ":   Tag   = " << tag << endl;
#endif 
}
  

} // namespace pebbl
