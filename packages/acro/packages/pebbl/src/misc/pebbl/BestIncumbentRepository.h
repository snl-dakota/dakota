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
 * \file BestIncumbentRepository.h
 *
 * Defines the pebbl::PointList and pebbl::BestIncumbentRepository classes.
 */

#ifndef pebbl_BestIncumbentRepository_h
#define pebbl_BestIncumbentRepository_h

#include <acro_config.h>
#include <utilib/SimpleSplayTree.h>
#include <utilib/LinkedList.h>

namespace pebbl {

using utilib::LinkedList;
using utilib::SimpleSplayTree;
using utilib::SimpleSplayTreeItem;
using utilib::Ereal;


template <class PointT>
class PointList
{
public:

  PointList() {}

  double val;

  LinkedList<PointT> points;

  bool operator<(const PointList<PointT>& pt) const
	{ return ((pt.val - val)*sense > tolerance); }

  bool operator>(const PointList<PointT>& pt) const
	{ return ((val - pt.val)*sense > tolerance); }

  bool operator==(const PointList<PointT>& pt) const
	{ return !((pt.val - val)*sense > tolerance) && !((val - pt.val)*sense > tolerance); }

  int compare(const PointList<PointT>& pt) const
	{
        int status = 0;
	if ((pt.val - val)*sense > tolerance) status = -1;
	else if ((val - pt.val)*sense > tolerance) status = 1;
	return status;
	}

  bool add(PointT& pt) {
	bool flag=false;
	typename LinkedList<PointT>::iterator curr = points.begin();
	typename LinkedList<PointT>::iterator end = points.end();
	while (curr != end) {
	  if ( (*curr) == pt ) return false;
	  if ( (*curr) > pt ) {
	     points.insert(curr,pt);
	     flag=true;
	     break;
	     }
          curr++;
          }
	if (!flag)
	   points.push_back(pt);
	return true;
	}

  static double tolerance;
  static int sense;
};

template <class PointT>
double PointList<PointT>::tolerance = 1e-16;

template <class PointT>
int PointList<PointT>::sense = 1;

/**
 * This class assumes minimization
 */
template <class PointT>
class BestIncumbentRepository
{
public:

  BestIncumbentRepository() 
	: EnumLimit(0), 
	  EnumRelTolerance(Ereal<double>::positive_infinity), 
	  EnumAbsTolerance(Ereal<double>::positive_infinity),
	  value_tolerance(1e-7), sense(1),
	  num(0), worst_num(0),
	  best_val(Ereal<double>::positive_infinity),
	  worst_val(Ereal<double>::positive_infinity)
	  { }

  virtual ~BestIncumbentRepository()
	{ }

  void insert(double val, PointT& pt);

  size_type EnumLimit;

  Ereal<double> EnumRelTolerance;

  Ereal<double> EnumAbsTolerance;
 
  virtual void write(std::ostream& os) const;

  size_type size() {return num;}

  const Ereal<double>& worst_value() const {return worst_val;}

  double value_tolerance;

  int sense;

protected:

  size_type num;
 
  size_type worst_num;

  SimpleSplayTree<PointList<PointT> > tree;

  Ereal<double> best_val;

  Ereal<double> worst_val;

  PointList<PointT> tmp;
};


template <class PointT>
void BestIncumbentRepository<PointT>::insert(double val, PointT& pt)
{
//cerr << "HERE1 " << val << " " << pt << endl;
//
// Return if we already have EnumLimit points in the Cache and this
// isn't an improving point
//
if ((EnumLimit > 0) && ((val - worst_val)*sense >= 0.0) && (num >= EnumLimit)) return;
//
// Return if we're applying the EnumRelTolerance and this point is too large
//
if ((EnumRelTolerance != Ereal<double>::positive_infinity) && 
    ((val - best_val*EnumRelTolerance)*sense > 0.0)) return;
//
// Return if we're applying the EnumAbsTolerance and this point is too large
//
if ((EnumAbsTolerance != Ereal<double>::positive_infinity) && 
    ((val - best_val+EnumAbsTolerance)*sense > 0.0)) return;
//cerr << "HERE3 " << best_val << " " << EnumAbsTolerance << endl;
//
//
// Find this value in the tree
//
//
PointList<PointT>::tolerance = value_tolerance;
PointList<PointT>::sense = sense;
tmp.val = val;
tree.splay(tmp);
if (!tree || (std::fabs(tree.top()->key().val -val) > value_tolerance)) {
   PointList<PointT> tlist;
   tlist.val = val;
   tree.add(tlist);
   //if (&(tree.top()->key()) != tlist)
      //exit_fn();
   }
//
// Augment the tree
//
PointList<PointT>& curr = tree.top()->key();
//cerr << "HERE2 " << curr.val << " " << val << " " << pt << endl;
if (curr.add(pt))
   num++;
if ((val - best_val)*sense < 0.0)
   best_val = val;
if ((num != 1) && (val == worst_val))
   worst_num++;
else if ((num == 1) || ((val - worst_val)*sense > 0.0)) {
   worst_val = val;
   worst_num = 1;
   }
//
// Possibly prune the tree
//
if ((EnumLimit > 0) && ((val - worst_val)*sense < 0.0) && (num >= (worst_num+EnumLimit))) {
   tmp.val = worst_val;
   bool status;
   tree.splay(tmp);
   PointList<PointT>* tkey = &(tree.top()->key());
   num -= tkey->points.size();
   tree.remove(tmp,status);
   if (!status)
      EXCEPTION_MNGR(std::runtime_error,"Problem removing an element from a splay tree.");
   worst_val = tree.top()->key().val;
   worst_num = tree.top()->key().points.size();
   //if (worst_num == 0) exit_fn();
   }
}


template <class PointT>
void BestIncumbentRepository<PointT>::write(std::ostream& os) const
{
os << "Size " << num << " (Categories: " << tree.size() << ")" << std::endl;
const SimpleSplayTreeItem<PointList<PointT> >* node = tree.find_rank(0);

int count=0;
int i=1;

while (node) {
  typename LinkedList<PointT>::const_iterator curr = node->key().points.begin();
  typename LinkedList<PointT>::const_iterator end = node->key().points.end();
  while (curr != end) {
    count++;
    os << "Value: " << node->key().val << "   Solution: " << *curr << std::endl;
    curr++;
    }
  node = tree.find_rank(i++);
  }
}

}

template <class PointT>
inline std::ostream& operator<<(std::ostream& output, 
				pebbl::BestIncumbentRepository<PointT>& info)
{ info.write(output); return(output); }

#endif
