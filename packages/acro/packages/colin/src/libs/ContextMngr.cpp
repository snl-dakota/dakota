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


#include <utilib/BasicArray.h>
#include <utilib/MixedIntVars.h>

#include <colin/ContextMngr.h>
#include <colin/ColinRegistrations.h>
#include <colin/real.h>


using std::vector;

using utilib::Any;
using utilib::BasicArray;
using utilib::MixedIntVars;

namespace colin {

// This call guarantees that all factory components are linked in when
// we build against static libraries.  This is here because the
// StaticInitializers register problem transformations with the
// ProblemMngr(), so if your application uses the ProblemMngr (or
// ContextMngr(), it will automatically link in all the
// statically-registered colin objects.
namespace StaticInitializers {
namespace {
const bool register_all = static_colin_registrations;
} // namespace colin::StaticInitializers::(local)
} // namespace colin::StaticInitializers


//----------------------------------------------------------------------------

namespace {

template<typename FROM, typename TO>
int lexicalCast_vectorVector(const Any& from_, Any& to_)
{
   const vector<vector<FROM> >& from = from_.expose<vector<vector<FROM> > >();
   vector<vector<TO> >& to = to_.set<vector<vector<TO> > >();

   to.resize(from.size());
   for(size_t i=0; i<from.size(); ++i)
   {
      size_t j=from[i].size();
      to[i].resize(j);
      while( j > 0 )
      {
         --j;
         to[i][j] = from[i][j];
      }
   }
   return OK;
}


template<typename T>
int lexicalCast_matrix2vector(const Any& from_, Any& to_)
{
   const vector<vector<T> >& from = from_.expose<vector<vector<T> > >();
   vector<T>& to = to_.set<vector<T> >();

   if ( from.empty() )
      return OK;
   
   to = from[0];
   return from.size() == 1 ? OK : utilib::Type_Manager::CastWarning::ValueLost;
}


/// MixedIntVars -> vector of reals
int lexicalCast_MixedIntVars_realVector(const Any& from_, Any& to_)
{
   const utilib::MixedIntVars& from = from_.expose<utilib::MixedIntVars>();
   std::vector<colin::real> &ans = to_.set<std::vector<colin::real> >();

   ans.reserve(from.numAllVars());
   size_t n = from.Binary().size();
   for(size_t i = 0; i < n; ++i)
      ans.push_back(from.Binary()[i]);

   n = from.Integer().size();
   for(size_t i = 0; i < n; ++i)
      ans.push_back(from.Integer()[i]);

   n = from.Real().size();
   for(size_t i = 0; i < n; ++i)
      ans.push_back(from.Real()[i]);

   return OK;
}


bool LocalRegistrations()
{
   utilib::Type_Manager *t_manager = utilib::TypeManager();
   //t_manager = new utilib::Type_Manager();

   t_manager->register_lexical_cast
      (typeid(MixedIntVars), typeid(vector<colin::real>), 
       &lexicalCast_MixedIntVars_realVector);
   
   t_manager->register_lexical_cast
      ( typeid(vector<vector<double> >), 
        typeid(vector<double>), 
        &lexicalCast_matrix2vector<double>,
        10 );
   
   t_manager->register_lexical_cast
      ( typeid(vector<vector<real> >), 
        typeid(vector<real>), 
        &lexicalCast_matrix2vector<real>,
        10 );
   
   t_manager->register_lexical_cast
      ( typeid(vector<vector<int> >), 
        typeid(vector<int>), 
        &lexicalCast_matrix2vector<int>,
        10 );
   
   t_manager->register_lexical_cast
      ( typeid(vector<vector<double> >), 
        typeid(vector<vector<real> >), 
        &lexicalCast_vectorVector<double, real> );
   
   t_manager->register_lexical_cast
      ( typeid(vector<vector<real> >), 
        typeid(vector<vector<double> >), 
        &lexicalCast_vectorVector<real, double> );

   return true;
}

const bool local_registrations = LocalRegistrations();

} // namespace colin::(local)

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

utilib::Type_Manager& ProblemMngr()
{
   static utilib::Type_Manager* t_manager = NULL;
   if ( t_manager == NULL )
   {
      t_manager = new utilib::Type_Manager();
      t_manager->clear_lexical_casts();
      t_manager->setDefaultForceExact(false);
   }
   return *t_manager;
}

} // namespace colin
