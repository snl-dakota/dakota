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
 * \file ResponseFunctors.h
 *
 * Defines the Response Functor classes
 */

#ifndef colin_ResponseFunctors_h
#define colin_ResponseFunctors_h

#include <acro_config.h>

#include <colin/real.h>

#include <utilib/Any.h>
#include <utilib/sort.h>

#include <functional>

namespace colin
{

class ResponseFunctor : public std::unary_function<const std::vector<utilib::Any>, utilib::Any >
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~ResponseFunctor() {}

   ///
   virtual utilib::Any initialize(size_t num_values) = 0;

   ///
   virtual utilib::Any operator()(utilib::Any state, utilib::Any value) = 0;

   ///
   virtual utilib::Any result(utilib::Any state) = 0;
};


///
class FirstResponseFunctor : public ResponseFunctor
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~FirstResponseFunctor() {}

   utilib::Any initialize(size_t)
   { return utilib::Any(); }

   utilib::Any operator()(utilib::Any state, utilib::Any value)
   { return state.empty() ? value : state; }

   utilib::Any result(utilib::Any state)
   { return state; }
};


///
class MeanDoubleFunctor : public ResponseFunctor
{
   typedef std::pair<double,double>  state_t;

public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~MeanDoubleFunctor() {}

   utilib::Any initialize(size_t num_values)
   { return state_t(num_values, 0.0); }

   utilib::Any operator()(utilib::Any state, utilib::Any value)
   { 
      double d;
      utilib::TypeManager()->lexical_cast(value, d);
      state.expose<state_t>().second += d;
      return state;
   }

   utilib::Any result(utilib::Any state)
   { 
      const state_t& s = state.expose<state_t>();
      return s.second / s.first;
   }
};

///
class StddevDoubleFunctor : public ResponseFunctor
{
   typedef std::vector<double>  state_t;

public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~StddevDoubleFunctor() {}

   utilib::Any initialize(size_t num_values)
   { 
      state_t state(3, 0);
      state[0] = num_values;
      return state; 
   }

   utilib::Any operator()(utilib::Any state, utilib::Any value)
   { 
      double d;
      utilib::TypeManager()->lexical_cast(value, d);
      state.expose<state_t>()[1] += d;
      state.expose<state_t>()[2] += d*d;
      return state;
   }

   utilib::Any result(utilib::Any state)
   { 
      const state_t& s = state.expose<state_t>();
      return sqrt((s[0]*s[2] - s[1]*s[1]) / (s[0]*(s[0]-1)));
   }
};



#if 0
///
class TCEDoubleFunctor : public ResponseFunctor
{
   typedef std::pair<double,double>  state_t;

public:

   ///
   TCEDoubleFunctor(double alpha_) : alpha(alpha_) {}

   /// Empty virtual destructor to silence compiler warnings
   virtual ~TCEDoubleFunctor() {}

   ///
   double alpha;

   ///
   utilib::Any operator()(const std::vector<utilib::Any>& x)
   {
      std::vector<double> vals(x.size());
      std::vector<double>::iterator      vals_it = vals.begin();
      std::vector<utilib::Any>::const_iterator it      = x.begin();
      std::vector<utilib::Any>::const_iterator itEnd   = x.end();
      for (; it < itEnd; it++, vals_it++)
      {
         double tmp;
         utilib::TypeManager()->lexical_cast(*it, tmp);
         *vals_it = tmp;
      }
      utilib::sort(vals, std::greater<double>());
      size_t num = ( alpha > 0 ? (size_t)std::ceil(x.size()*alpha) : 0 );
      if (num > x.size()) 
         num=x.size();
      double ans=0.0;
      vals_it = vals.begin();
      std::vector<double>::iterator vals_end = vals.end();
      for(size_t i=0; i<num; i++, vals_it++)
	{
	ans += *vals_it;
	}
      return ans/num;
   }
};

///
class GenericDoubleVectorFunctor : public ResponseFunctor
{
public:

   ///
   GenericDoubleVectorFunctor(std::vector<ResponseFunctor*>& functors_)
	{ functors = functors_; }

   /// Empty virtual destructor to silence compiler warnings
   virtual ~GenericDoubleVectorFunctor() {}

   ///
   FixedDoubleFunctor default_functor;

   ///
   std::vector<ResponseFunctor*> functors;

   ///
   std::vector<std::vector<utilib::Any> > values;

   ///
   utilib::Any operator()(const std::vector<utilib::Any>& x)
   {
      //
      // Return an empty array of no responses have been computed
      //
      if (x.size() == 0) {
         return std::vector<real>();
      }
      //
      // Setup data structures
      //
      std::vector<real> tmp;
      std::vector<utilib::Any>::const_iterator it = x.begin();
      utilib::TypeManager()->lexical_cast(*it, tmp);
      values.resize(tmp.size());
      std::vector<real> total(tmp.size());
      if (tmp.size() != functors.size()) {
         EXCEPTION_MNGR(std::runtime_error, "GenericDoubleFunctor - Functor list has " << functors.size() << " functors, but there are " << tmp.size() << " response values to summarize.");
      }
      std::vector<std::vector<utilib::Any> >::iterator v_it = values.begin();
      std::vector<std::vector<utilib::Any> >::iterator v_end = values.end();
      while (v_it != v_end) {
        v_it->resize(x.size());
        v_it++;
        }
      //
      // Fill the 'values' data structure
      //
      std::vector<utilib::Any>::const_iterator itEnd = x.end();
      for (size_t i=0; it < itEnd; it++, i++)
      {
         std::vector<real> tmp;
         utilib::TypeManager()->lexical_cast(*it, tmp);
         std::vector<real>::iterator tmp_it  = tmp.begin();
         std::vector<real>::iterator tmp_end = tmp.end();
         for (size_t j=0; tmp_it != tmp_end; j++, tmp_it++)
         {
            values[j][i] = *tmp_it;
         }
      }
      //
      // Apply each functor in turn
      //
      std::vector<real>::iterator t_it = total.begin();
      std::vector<real>::iterator t_end = total.end();
      std::vector<ResponseFunctor*>::iterator f_it = functors.begin();
      v_it = values.begin();
      for (; t_it != t_end; t_it++, f_it++, v_it++)
      {
         utilib::Any tmp;
         if (*f_it == 0)
            tmp = default_functor(*v_it);
         else
            tmp = (**f_it)(*v_it);
         utilib::TypeManager()->lexical_cast(tmp, *t_it);
      }
      return total;
   }
};

///
class MeanDoubleVectorFunctor : public ResponseFunctor
{
public:
   /// Empty virtual destructor to silence compiler warnings
   virtual ~MeanDoubleVectorFunctor() {}

   ///
   utilib::Any operator()(const std::vector<utilib::Any>& x) const
   {
      std::vector<real> total;
      std::vector<utilib::Any>::const_iterator it    = x.begin();
      std::vector<utilib::Any>::const_iterator itEnd = x.end();
      for (; it < itEnd; it++)
      {
         std::vector<real> tmp;
         utilib::TypeManager()->lexical_cast(*it, tmp);
         if (total.size() == 0) total.resize(tmp.size());
         std::vector<real>::iterator t_it = total.begin();
         std::vector<real>::iterator t_end = total.end();
         std::vector<real>::iterator tmp_it = tmp.begin();
         for (; t_it != t_end; t_it++, tmp_it++)
         {
            *t_it += *tmp_it;
         }
      }
      std::vector<real>::iterator t_it = total.begin();
      std::vector<real>::iterator t_end = total.end();
      for (; t_it != t_end; t_it++)
      {
         *t_it /= x.size();
      }
      return total;
   }
};
#endif


}

#endif // defined colin_ResponseFunctors_h
