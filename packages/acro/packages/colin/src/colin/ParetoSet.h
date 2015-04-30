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
 * \file ParetoSet.h
 *
 * Defines the colin::ParetoSet class.
 */

#ifndef colin_ParetoSet_h
#define colin_ParetoSet_h

#include <acro_config.h>

#include <utilib/Uniform.h>

namespace colin
{

/** A comparison function for assessing whether one point dominates another. */
template <class ArrayT>
class ArrayDomination
{
public:

   /// Returns true if x dominates y
   bool operator()(const ArrayT& x, const ArrayT& y)
   {
      bool eq_flag = true;
      for (unsigned int i = 0; i < x.size(); i++)
      {
         if (y[i] < x[i])
            return false;
         if (y[i] > x[i]) eq_flag = false;
      }
      if (eq_flag == true)
         return false;
      return true;
   }
};


/** A set for pareto-optimal solutions.  */
template <class DomainT, class ValueT, class DominationFuncT>
class ParetoSet
{
public:

   ///
   typedef std::pair<DomainT, ValueT> point;

   ///
   ParetoSet() {}

   ///
   unsigned int size() const {return points.size();}

   ///
   void clear()
   { points.clear(); }

   ///
   bool insert(const DomainT& x, const ValueT& values)
   {
      typename std::list<point>::iterator curr = points.begin();
      typename std::list<point>::iterator end  = points.end();
      while (curr != end)
      {
         //
         // Remove points in the cache that this point dominates
         //
         while ((curr != end) && (compare(values, curr->second) == true))
            curr = points.erase(curr);
         //
         // If the cache point dominates this point, then return
         //
         //ucout << "COMPARE: " << curr->second << " / " << values << " " << compare(curr->second,values) << std::endl;
         if (curr == end) continue;

         if (compare(curr->second, values) == true)
            return false;
         curr++;
      }
      points.push_back(std::pair<DomainT, ValueT>(x, values));
      return true;
   }

   ///
   DomainT& get_random(utilib::Uniform& urnd)
   {
      if (points.size() == 0)
         EXCEPTION_MNGR(std::runtime_error, "ERROR - cannot call get_random with no points in the cache.");
         unsigned int i = utilib::Discretize<unsigned int>(urnd(), 0, points.size() - 1);
      unsigned int j = 0;
      typename std::list<point>::iterator curr = points.begin();
      while (j < i)
      {
         curr++;
         j++;
      }
      return curr->first;
   }

   ///
   void write(std::ostream& os) const
   {
      typename std::list<point>::const_iterator curr = points.begin();
      typename std::list<point>::const_iterator end  = points.end();
      while (curr != end)
      {
         os << curr->first << " / " << curr->second << std::endl;
         curr++;
      }
   }

protected:

   ///
   DominationFuncT compare;

   ///
   std::list<point> points;

};


}

template <class DomainT, class ValueT, class DominationFuncT>
std::ostream& operator<<(std::ostream& os,
                         const colin::ParetoSet<DomainT, ValueT, DominationFuncT> & obj)
{ obj.write(os); return os; }

#endif
