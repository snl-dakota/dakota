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

#ifndef colin_PointSet_h
#define colin_PointSet_h

#include <colin/Cache.h>
#include <colin/cache/Factory.h>

#include <colin/application/Base.h>

#include <utilib/TypeManager.h>

#include <list>

namespace colin {

class PointSet
{
public:

   ///
   PointSet()
      : cache_()
   {}

   ///
   PointSet(CacheHandle cache__)
      : cache_(cache__)
   {}

   ///
   void add_point(ApplicationHandle problem, utilib::Any point)
   {
      colin::AppRequest req = problem->set_domain(point);
      (*this)->insert( problem->eval_mngr().perform_evaluation(req) );
   }

   ///
   void get_point(ApplicationHandle problem, utilib::AnyFixedRef point)
   {
      std::list<utilib::Any> points;
      get_points(problem, points, 1);
      if ( points.empty() )
         EXCEPTION_MNGR(std::runtime_error, "PointSet::get_point(): "
                        "empty PointSet.");
      utilib::TypeManager()->lexical_cast(points.front(), point);
   }

   ///
   utilib::Any get_point(ApplicationHandle problem)
   {
      std::list<utilib::Any> points;
      get_points(problem, points, 1);
      if ( points.empty() )
         EXCEPTION_MNGR(std::runtime_error, "PointSet::get_point(): "
                        "empty PointSet.");
      return points.front();
   }

   ///
   template <class PointT>
   void get_points( ApplicationHandle problem, std::list<PointT>& points, 
                    size_t num_points = 0 )
   {
      Application_Base* app = problem.object();
      Cache::iterator it = (*this)->begin(app);
      Cache::iterator itEnd = (*this)->end();
      for( ; it!= itEnd; ++it )
      {
         points.push_back(PointT());
         utilib::TypeManager()->lexical_cast
            (it->second.asResponse(app).get_domain(app), points.back());
      }
   }

   ///
   CacheHandle& cache()
   {
      if ( cache_.empty() )
      {
         cache_ = CacheFactory().create_view("Subset");
         // If the default caching system is "No Cache", we still need
         // to create a cache for the PointSet to work.  We will fall
         // back to using a Local Cache.
         if ( cache_.empty() )
            cache_ = CacheFactory().create("Local");
      }
      assert( ! cache_.empty() );
      return cache_;
   }

   ///
   Cache* operator->()
   {
      return cache().object();
   }


   ///
   size_t size(const Application_Base* app) const
   { return cache_.empty() ? 0 : cache_->size(app); }

private:

   /// The cache that we are wrapping
   CacheHandle cache_;
};

} // namespace colin

#endif // colin_PointSet_h
