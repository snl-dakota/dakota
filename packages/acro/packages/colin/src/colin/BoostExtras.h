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
 * \file BoostExtras.h
 *
 * Declares supplemental utility functions for use with Boost template
 * definitions.
 */

#ifndef colin_BoostExtras_h
#define colin_BoostExtras_h

#include <acro_config.h>

namespace colin
{

namespace boost_extras {

/** Special variant of boost::signal that calls slots in the reverse
 *  order from which they were registered (i.e. Last in, First out).
 */
template<typename Signature, // function type R (T1, T2, ..., TN)
         typename Combiner = boost::last_value
             <typename boost::function_traits<Signature>::result_type>,
         typename Group = int,
         typename GroupCompare = std::less<Group>,
         typename SlotFunction = boost::function<Signature>
>
class lifo_signal : public boost::signal<Signature, Combiner, Group, 
                                         GroupCompare, SlotFunction> 
{
   typedef boost::signal<Signature, Combiner, Group, 
                         GroupCompare, SlotFunction>  signal_t;

public:
   explicit lifo_signal(const Combiner& combiner = Combiner(),
                        const GroupCompare& group_compare = GroupCompare())
      : signal_t(combiner, group_compare)
   {}

   // Connect a slot to this signal
   boost::signals::connection
   connect(const typename signal_t::slot_type& fcn)
   {
      return signal_t::connect(fcn, boost::signals::at_front);
   }
      
   boost::signals::connection
   connect(const typename signal_t::group_type& group, 
           const typename signal_t::slot_type& fcn)
   {
      return signal_t::connect(group, fcn, boost::signals::at_front);
   }
};


/** A Combiner class that will sum the return values from all of the slots.
 */
template<typename T>
struct sum {
   typedef T result_type;

   template<typename InputIterator>
   T operator()(InputIterator first, InputIterator last) const
   {
      T value = T();
      while (first != last)
         value = value + *first++;
      return value;
   }
};

template<>
struct sum<void> {
   struct unusable {};

   typedef unusable result_type;

   template<typename InputIterator>
   result_type
   operator()(InputIterator first, InputIterator last) const
   {
      while (first != last)
         *first++;
      return result_type();
   }
};



/** A Combiner class that will call slots until each slot returns true.
 *  The return value is the number of slots that never returned true
 *  (after the first complete pass through the slots where no new slots
 *  returned true).
 */
struct call_until_pass {
   typedef size_t result_type;

   template<typename InputIterator>
   size_t operator()(InputIterator first, InputIterator last) const
   {
      if (first == last)
         return 0;

      // an absurdly large number just to prevent infinite loops
      size_t maxPasses = 100; 

      bool slotPassed = false;
      std::list<InputIterator> pending;
      for( ; first != last; ++first )
      {
         int ans = *first;
         if ( ans != 0 )
            slotPassed = true;
         if ( ans >= 0 )
            pending.push_back(first);
      }

      while ( slotPassed && ! pending.empty() && --maxPasses )
      {
         slotPassed = false;
         std::list<InputIterator> active;
         active.swap(pending);
         while ( ! active.empty() )
         {
            int ans = *active.front();
            if ( ans != 0 )
               slotPassed = true;
            if ( ans >= 0 )
               pending.splice(pending.end(), active, active.begin());
            else
               active.pop_front();
         }
      }

      return pending.size();
   }
};


} // namespace colin::boost_extras
} // namespace colin

#endif // defined colin_BoostExtras_h
