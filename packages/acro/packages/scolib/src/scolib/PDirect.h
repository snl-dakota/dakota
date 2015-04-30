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
 * \file PDirect.h
 *
 * Defines the colin::PDIRECT class.
 */

#ifndef scolib_PDirect_h
#define scolib_PDirect_h

#include <acro_config.h>
#include <scolib/Direct.h>

namespace scolib {


class PDIRECT : public DIRECT
{
public:
    
  ///
  PDIRECT();
    	  
  int pid;
  int nproc;				// number of processors   
	
  void minimize();

  void reset();

private:

  ///
  std::string pmethod_str;

  ///Parallel method selection, 1 = Original DIRECT, 2 = Continuous DIRECT, 3 = Switch from 1 to 2 at iteration itter
  int pmethod;

  int switcher;

  /// true = can't divide parent again until all children are finished
  bool dd;

  ///
  std::string startcond_str;

  /// 1 = use DIRECT division, 2 = use even boxes
  int startcond;

  int start;				
  int wait2;				
  int nsort;				// number of completed boxes
  std::queue<std::vector<double> > pool;		// list of points to be analyzed
  std::list<int> pfin;				// list of processors to send pts to
  std::list<direct::vec> child;			// list of parent and child boxes waiting to be divided
  std::list<int> index;
  std::list<direct::mesg> request;

  FILE *fp5;
	   		
  virtual void Divider(std::vector<unsigned int>& optimal);
  void Distribute();
  void insert(int );   
  void initial(std::vector<int>& optimal); 
  void divide2(std::list<direct::vec>::iterator ); 
  void initial2(int ); 
};
  
} // namespace scolib

#endif
