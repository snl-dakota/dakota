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
 * \file panmictic_selection.h
 *
 * Defines the scolib::panmictic_selection class.
 */

#ifndef scolib_panmictic_selection_h
#define scolib_panmictic_selection_h

#include <acro_config.h>
#include <scolib/selection.h>

namespace scolib {


/** A class that performs panmictic selection in which each solution 
competes with every other solution. */
template <class ObjectT, class AccessorT, class CompareT>
class panmictic_selection : public selection<ObjectT,AccessorT,CompareT> {

public:

  /// Constructor.
  panmictic_selection(utilib::PropertyDict& properties)
	 : selection<ObjectT,AccessorT,CompareT>(properties) {set_window(10);}

  /// Reset this object.
  void reset()
	{
	selection<ObjectT,AccessorT,CompareT>::reset();
	set_window(this->window_size);
	}

  /// Scale the value of the worst fitness value and initialize
  void initialize(BasicArray<ObjectT>& Evals, AccessorT accessor,
							CompareT compare)
	{
	scale_fitness(Evals, accessor, compare);
	selection<ObjectT,AccessorT,CompareT>::initialize(Evals, accessor, compare);
	}

  ///
  void write(std::ostream&) const;

  ///
  void set_window(const int size);

protected:

  ///
  BasicArray<real > window;

  ///
  BasicArray<int> selection_map;

  ///
  void scale_fitness(BasicArray<ObjectT>& Eval, AccessorT& accessor,
						CompareT& compare);

};



template <class ObjectT, class AccessorT, class CompareT>
void panmictic_selection<ObjectT,AccessorT,CompareT>
	::set_window(const int size)
{
this->ctr=0;
this->window_size = size;
if (size > 0) {
  window.resize(size);
  this->first_time=true;
  }
}


template <class ObjectT, class AccessorT, class CompareT>
void panmictic_selection<ObjectT,AccessorT,CompareT>
	::scale_fitness(BasicArray<ObjectT>& pop_eval, AccessorT& accessor,
						CompareT& compare)
{
if (this->window_size == -2)			// static worst
   return;

   //
   // sigma scaling: add the sigma_factor*sigma term because
   // we're minimizing
   // 
else if (this->window_size == -1) {		// sigma scaling
   double sigma = std::sqrt(utilib::var(pop_eval, accessor));
   double avg = utilib::mean(pop_eval, accessor);
   this->worst_const = this->New_worst(avg + this->sigma_factor*sigma);
   }

else if (this->window_size > 0) {		// window scaling
                        		// avg of worst in last (window_size) gen's
   this->ctr++;
   if (this->first_time == true) {
      this->first_time = false;
      window << this->New_worst(accessor(utilib::max(pop_eval, compare)));
      this->worst_const = window[0];
      }
   else {
      int w = this->ctr % this->window_size;
      window[w] = this->New_worst(accessor(utilib::max(pop_eval, compare)));
      this->worst_const = utilib::mean(window);
      }
   OUTPUTPR(2, ucout << "Worst Window: " << window << "\n");
   }

else {					// infinite window scaling
                        		// worst individual ever seen
   if (this->first_time == true) {
      this->first_time = false;
      this->worst_const = this->New_worst(accessor(utilib::max(pop_eval, compare)));
      }
   else {
      real tmp = this->New_worst(accessor(utilib::max(pop_eval, compare)));
      this->worst_const = (this->worst_const < tmp? tmp : this->worst_const);
      }
   }
}

} // namespace scolib

#endif
