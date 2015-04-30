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
 * \file OptPointDB.h
 *
 * Defines the colin::OptPointDB class.
 */

#ifndef colin_OptPointDB_h
#define colin_OptPointDB_h

#include <acro_config.h>
#include <utilib/hash_fn.h>
#include <utilib/SimpleHashTable.h>
#include <utilib/LPHashTable.h>
#include <utilib/stl_auxiliary.h>
#include <colin/real.h>

namespace colin
{


namespace db
{

using std::list;
using std::string;
using utilib::SimpleHashTable;
using utilib::LPHashTable;

//============================================================================
//============================================================================
// Class OptPoint
//============================================================================
//============================================================================
/**
 * Cointainer class for the information associated with an optimization
 * point.
 */
template <class DomainT>
class OptPointInfo
{
public:

   /// Constructor
   OptPointInfo() {}

   /// Destructor
   ~OptPointInfo() {}

   /// The point
   DomainT point;

   /// The point's ID
   unsigned long int id;

   /// The value of the point
   colin::real value;

   /// Write this object
   void write(std::ostream& os) const
      {os << id << " " << value << " " << point;}

};


//============================================================================
//============================================================================
// Class OptPointDB
//============================================================================
//============================================================================
/**
 * This class is used to cache points that are evaluated by an optimizer.
 */
template <class DomainT>
class OptPointDB
{
public:

   /// Constructor
   OptPointDB() {}

   /// Destructor
   ~OptPointDB() {}

   /// Add a point to the database
   bool add(const DomainT& point, const colin::real& value, long int id)
   {return add(point, value, id, dummy_list);}

   /// Add a point to the database with a list of string tags
   bool add(const DomainT& point, const colin::real& value, long int id,
               const list<string>& taglist);

   /// Return 'true' if this point exists in the database
   bool exists(const DomainT& point);

   /// Get tag info
   void get_taginfo(const string& tag, list<OptPointInfo<DomainT>* > *& points);

   /// Get point info
   void get_pointinfo(const DomainT& point, OptPointInfo<DomainT>*& value);

   /// Get a tagged point
   bool tag_point(const DomainT& point, const string& tag);

   /// Number of docimal places
   unsigned int round_precision;

   /// Write the DB
   void write(std::ostream& os) const
   {
      typename list<OptPointInfo<DomainT> >::const_iterator curr = point_list.begin();
      typename list<OptPointInfo<DomainT> >::const_iterator end  = point_list.end();
      while (curr != end)
      {
         os << *curr << std::endl;
         curr++;
      }
   }

protected:

   /// The DB type.
   typedef LPHashTable<DomainT, OptPointInfo<DomainT>* > point_table_type;

   /// The table of points
   point_table_type point_table;

   /// The Tag Table Type.
   typedef SimpleHashTable<string, list<OptPointInfo<DomainT>* > > tag_table_type;

   /// The tag table
   tag_table_type tag_table;

   /// The list of points
   list<OptPointInfo<DomainT> > point_list;

   /// TODO
   list<string> dummy_list;

};


template <class DomainT>
bool OptPointDB<DomainT>::add(const DomainT& point, const colin::real& value,
                                 long int id,
                                 const list<string>& taglist)
{
//
// Return if point exists
//
   if (point_table.exists(point))
      return false;
//
// Add point
//
   point_list.push_front(OptPointInfo<DomainT>());
   OptPointInfo<DomainT>* pt = &(point_list.front());
   pt->point = point;
   pt->value = value;
   pt->id    = id;
//
// Update point_table
//
   point_table.add(point, pt, true);
//
// Update tag_table
//
   list<string>::const_iterator curr = taglist.begin();
   list<string>::const_iterator end  = taglist.end();
   while (curr != end)
   {
      typename tag_table_type::item_type* item;
      if (!(item = tag_table.find(*curr)))
      {
         list<OptPointInfo<DomainT>* > tmp;
         tag_table.add(*curr, tmp, item, true);
      }
      item->info().push_back(pt);
   }

   return true;
}


template <class DomainT>
bool OptPointDB<DomainT>::exists(const DomainT& point)
{ return point_table.exists(point); }


template <class DomainT>
void OptPointDB<DomainT>::get_taginfo(const string& tag,
                                      list<OptPointInfo<DomainT>* > *& points)
{
   typename tag_table_type::item_type* item;
   if (item = tag_table.find(tag))
      points = &(item->info());
   else
      points = 0;
}


template <class DomainT>
void OptPointDB<DomainT>::get_pointinfo(const DomainT& point,
                                        OptPointInfo<DomainT>*& info)
{
   typename point_table_type::item_type* item;
   if (!(item = point_table.find(point)))
      info = 0;
   else
      info = &(item->key());
}


template <class DomainT>
bool OptPointDB<DomainT>::tag_point(const DomainT& point, const string& tag)
{
   typename point_table_type::item_type* pitem;
   if (!(pitem = point_table.find(point)))
      return false;

   typename tag_table_type::item_type* item;
   if (!(item = tag_table.find(tag)))
   {
      list<OptPointInfo<DomainT>* > tmp;
      //set_table.add(tag,tmp,true);
   }
   item->info().pushback(&(pitem->info()));

   return true;
}


}

}

/// Write a colin::OptPointInfo object.
template <class DomainT>
inline std::ostream& operator<<(std::ostream& output, const colin::db::OptPointInfo<DomainT>& data)
{ data.write(output); return output; }

/// Write a colin::OptPointDB object.
template <class DomainT>
inline std::ostream& operator<<(std::ostream& output, const colin::db::OptPointDB<DomainT>& data)
{ data.write(output); return output; }

#endif
