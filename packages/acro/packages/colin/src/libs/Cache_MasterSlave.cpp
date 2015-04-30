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
 * \file Cache_MasterSlave.cpp
 *
 * Implements the colin::cache::MasterSlave class.
 */

#include <colin/cache/MasterSlave.h>
#include <colin/AppRequest.h>

#include <colin/application/Base.h>
#include <colin/ApplicationMngr.h>
#include <colin/cache/Factory.h>
#include <colin/cache/Local.h>
#include <colin/ExecuteMngr.h>

#include <utilib/SharedPtr.h>
#include <utilib/SerialStream.h>
#include <utilib/TinyXML_helper.h>

using std::cerr;
using std::endl;

using std::list;
using std::set;
using std::map;
using std::pair;
using std::string;
using std::make_pair;

using utilib::Any;
using utilib::seed_t;
using utilib::SharedPtr;
using utilib::ioSerialStream;

namespace colin {

namespace StaticInitializers {
namespace {

/// Function to create a new MasterSlave
CacheHandle create_masterSlave_cache()
{
   return CacheHandle::create<cache::MasterSlave>().first;
}

bool RegisterMasterSlaveCache()
{
   CacheFactory().declare_cache_type("MasterSlave", create_masterSlave_cache);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool master_slave_cache = RegisterMasterSlaveCache();

} // namespace colin::StaticInitializers


namespace {

// "Command" values
const char SYNC       = 1;
const char INSERT     = 2;
const char ERASE      = 3;
const char CLEAR      = 4;
const char ANNOTATE   = 5;
const char UNANNOTATE = 6;

} // namespace colin::(local)


namespace cache {

//================================================================
// MasterSlave::Data Member Definitions
//================================================================

struct MasterSlave::Data {

   Data()
      : masterRank(0)
   {}

   class CommandFunctor;

   void command(const char* cmd, const Application_Base *context);

   void synchronize_master(const Application_Base *context, int rank);

   void synchronize_slave(const Application_Base *context);

   void clear(const Application_Base *context);

   size_t erase(const Application_Base *context, Key key);

   std::pair<iterator, bool> 
   insert( const Application_Base *context, 
           const Key& key, const CoreResponseInfo &value );

   void annotate( const Application_Base *context, Key key, 
                  string attribute, Any value );

   size_t erase_annotation( const Application_Base *context, Key key,
                            string attribute );


   struct Event {
      Event(Key key_, char command_)
         : key(key_), response(), attrib(""), value(), rank_refs(), 
           command(command_)
      {}

      Key               key;
      CoreResponseInfo  response;
      string            attrib;
      Any               value;
      set<int>          rank_refs;
      char              command;
   };

   /// My local copy of the cache data
   Local cache;

   /// The process rank for the Master Cache
   int masterRank;

   /// remove me!!!
   string cache_name;

   typedef const Application_Base*  app_t;

   typedef list<Event>  event_list_t;

   typedef map<int, event_list_t::iterator>  rank_map_t;

   ///
   map<app_t, event_list_t> event_lists;

   map<app_t, rank_map_t>  rank_state;
   
};

//================================================================


class MasterSlave::Data::CommandFunctor 
   : public ExecuteManager::ExecuteFunctor
{
public:
   CommandFunctor(MasterSlave::Data *cache_)
      : cache(cache_)
   {}

   virtual void execute(TiXmlElement* parameters)
   {
      Application_Base *context = NULL;
      string appName;
      utilib::get_string_attribute(parameters, "context", appName, "");
      if ( ! appName.empty() )
      {
         ApplicationHandle handle = ApplicationMngr().get_application(appName);
         if ( handle.empty() )
            EXCEPTION_MNGR(std::runtime_error, "MasterSlave:"
                           "command functor: cannot cache an application that "
                           "is not registered with the ApplicationManager");
         context = handle.object();
      }

      int rank;
      utilib::get_num_attribute(parameters, "rank", rank);

      // NB: test the commands in decreasing frequency
      const string &command = parameters->ValueStr();
      if ( command.compare("sync") == 0 )
         cache->synchronize_master(context, rank);
      else if ( command.compare("insert") == 0 )
      {
         Key key;
         CoreResponseInfo response;

         SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(rank);
         *ss >> key >> response;
         *ss << cache->insert( context, key, response );
      }
      else if ( command.compare("annotate") == 0 )
      {
         Key key;
         string attribute;
         utilib::Any value;

         SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(rank);
         *ss >> key >> attribute >> value;
         cache->annotate(context, key, attribute, value);
      }
      else if ( command.compare("unannotate") == 0 )
      {
         Key key;
         string attribute;

         SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(rank);
         *ss >> key >> attribute;
         cache->erase_annotation(context, key, attribute);
      }
      else if ( command.compare("erase") == 0 )
      {
         Key key;

         SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(rank);
         *ss >> key;
         *ss << cache->erase(context, key);
      }
      else if ( command.compare("clear") == 0 )
      {
         cache->clear(context);
      }
      else
         EXCEPTION_MNGR(std::runtime_error, "MasterSlave:"
                        "command functor: unknown command, '"
                        << command << "'");
   }

private:
   MasterSlave::Data *cache;
};



void
MasterSlave::Data::
synchronize_master(const Application_Base *context, int rank)
{
   event_list_t &el = event_lists[context];
   event_list_t::iterator it;
   event_list_t::iterator itEnd = el.end();
   rank_map_t::iterator rank_it = rank_state[context].find(rank);
   if ( rank_it == rank_state[context].end() )
      it = el.begin();
   else
   {
      it = rank_it->second;
      ++it; // don't repeat the last event
   }

   // send the data
   SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(rank);
   for ( ; it != itEnd; ++it )
   {
      *ss << it->command;
      switch ( it->command )
      {
      case INSERT:
         *ss << it->key;
         *ss << it->response;
         break;
      case ANNOTATE:
         *ss << it->key;
         *ss << it->attrib;
         *ss << it->value;
         break;
      case ERASE:
         *ss << it->key;
         break;
      case CLEAR:
         break;
      default:
         EXCEPTION_MNGR(std::runtime_error, "MasterSlave::"
                        "Data::synchronize_master(): unknown command: " 
                        << it->command);
      }
   }

   if ( el.empty() )
      return;

   // update the state reference
   --it;
   if ( rank_it != rank_state[context].end() )
      rank_it->second->rank_refs.erase(rank);
   rank_state[context][rank] = it;
   it->rank_refs.insert(rank);

   // clear out any events no longer needed
   size_t num_ranks = ExecuteMngr().num_ranks();
   if ( rank_state[context].size() < num_ranks - 1 )
      return;
   while ( el.front().rank_refs.empty() )
      el.pop_front();
}


void
MasterSlave::Data::
synchronize_slave(const Application_Base *context)
{
   if ( ExecuteMngr().rank() == masterRank )
      return;

   if ( context == NULL )
      EXCEPTION_MNGR(std::runtime_error, "NOT IMPLEMENTED for context==NULL");

   Key  key;
   string  attrib;
   Any  value;
   CoreResponseInfo  response;

   command("sync", context);
   SharedPtr<ioSerialStream> ss = ExecuteMngr().serialstream(masterRank);
   char cmd;
   while ( *ss >> cmd )
   {
      switch ( cmd ) 
      {
      case INSERT:
         *ss >> key;
         *ss >> response;
         cache.insert(context, key, response);
         break;
      case ANNOTATE:
         *ss >> key;
         *ss >> attrib;
         *ss >> value;
         cache.annotate(cache.find(context, key), attrib, value);
         break;
      case UNANNOTATE:
         *ss >> key;
         *ss >> attrib;
         cache.erase_annotation(cache.find(context, key), attrib);
         break;
      case ERASE:
         *ss >> key;
         cache.erase(context, key);
         break;
      case CLEAR:
         cache.clear(context);
         break;
      default:
         EXCEPTION_MNGR(std::runtime_error, "MasterSlave::"
                        "Data::synchronize_slave(): unknown command: " << cmd);
      }
   }
}


void
MasterSlave::Data::
clear(const Application_Base *context)
{
   cache.clear(context);

   event_list_t &el = event_lists[context];
   el.clear();
   el.push_back(Event(Key(), CLEAR));
   // clearing the state map is OK since a missing entry is interpreted
   // as "transmit the entire history" (starting with the "CLEAR" event)
   rank_state[context].clear();
}


size_t
MasterSlave::Data::
erase( const Application_Base *context, Key key )
{
   event_lists[context].push_back(Event(key, ERASE));

   return cache.erase(context, key);
}


std::pair<Cache::iterator, bool> 
MasterSlave::Data::
insert( const Application_Base *context, 
        const Key& key, const CoreResponseInfo &values )
{ 
   event_list_t &el = event_lists[context];
   el.push_back(Event(key, INSERT));
   el.back().response = values;

   return cache.insert(context, key, values);
}


void
MasterSlave::Data::
annotate( const Application_Base *context, Key key,
          string attribute, Any value )
{
   Cache::iterator pos= cache.find(context, key);
   if ( pos == cache.end() )
      return;

   cache.annotate(pos, attribute, value);

   event_list_t &el = event_lists[pos->first.context];
   el.push_back(Event(pos->first.key, ANNOTATE));
   el.back().attrib = attribute;
   el.back().value  = value;
}

size_t
MasterSlave::Data::
erase_annotation( const Application_Base *context, Key key, string attribute )
{
   Cache::iterator pos= cache.find(context, key);
   if ( pos == cache.end() )
      return 0;

   size_t ans = cache.erase_annotation(pos, attribute);

   if ( ans > 0 ) 
   {
      event_list_t &el = event_lists[pos->first.context];
      el.push_back(Event(pos->first.key, UNANNOTATE));
      el.back().attrib = attribute;
   }
   return ans;
}


//================================================================

void
MasterSlave::Data::
command(const char* cmd, const Application_Base *context)
{
   TiXmlElement elt(cmd);
   elt.SetAttribute("rank", ExecuteMngr().rank());
   if ( context != NULL )
   {
      string name = ApplicationMngr().get_application_name(context);
      if ( name.empty() )
         EXCEPTION_MNGR(std::runtime_error, "MasterSlave::"
                        "Data::command(): cannot cache an application that "
                        "is not registered with the ApplicationManager");
      elt.SetAttribute("context", name);
   }
   ExecuteMngr().run_command(cache_name, masterRank, &elt);
}


//================================================================
// MasterSlave Public Member Definitions
//================================================================

MasterSlave::MasterSlave()
   : data(new Data())
{
   // Since the embedded Local cache will live as long as this cache, we
   // can be lazy here about scoping the lifetime of the connections
   data->cache.onClear.connect
      ( boost::bind(boost::ref(onClear), _1) );
   data->cache.onInsert.connect
      ( boost::bind(boost::ref(onInsert), _1) );
   data->cache.onUpdate.connect
      ( boost::bind(boost::ref(onUpdate), _1, _2) );
   data->cache.onErase.connect
      ( boost::bind(boost::ref(onErase), _1) );
   data->cache.onAnnotate.connect
      ( boost::bind(boost::ref(onAnnotate), _1, _2, _3) );
   data->cache.onEraseAnnotation.connect
      ( boost::bind(boost::ref(onEraseAnnotation), _1, _2) );
}


MasterSlave::~MasterSlave()
{
   delete data;
}


void
MasterSlave::
set_key_generator(utilib::SharedPtr<Cache::KeyGenerator> new_keygen)
{
   ConcreteCache::set_key_generator(new_keygen);
   data->cache.set_key_generator(new_keygen);
}


size_t MasterSlave::size( const Application_Base *context ) const
{
   const Application_Base *app = get_core_application(context);
   data->synchronize_slave(app);
   return data->cache.size(app);
}


void MasterSlave::clear( const Application_Base *context )
{
   const Application_Base *app = get_core_application(context);
   if ( ExecuteMngr().rank() == data->masterRank )
      data->clear(app);
   else
      data->command("clear", app);
}


Cache::iterator
MasterSlave::begin(const Application_Base *context) const
{
   data->synchronize_slave(context);
   return data->cache.begin(context);
}


Cache::iterator
MasterSlave::end() const
{
   return data->cache.end();
}


void 
MasterSlave::annotate( Cache::iterator pos,
                                     std::string attribute, 
                                     utilib::Any value )
{
   if ( ExecuteMngr().rank() == data->masterRank )
      data->annotate(pos->first.context, pos->first.key, attribute, value);
   else
   {
      data->command("annotate", pos->first.context);

      SharedPtr<ioSerialStream> ss 
         = ExecuteMngr().serialstream(data->masterRank);
      *ss << pos->first.key << attribute << value;
   }
}


size_t
MasterSlave::erase_annotation( Cache::iterator pos,
                                             std::string attribute )
{
   if ( ExecuteMngr().rank() == data->masterRank )
      return data->erase_annotation( pos->first.context, pos->first.key, 
                                     attribute );
   else
   {
      data->command("unannotate", pos->first.context);

      SharedPtr<ioSerialStream> ss 
         = ExecuteMngr().serialstream(data->masterRank);
      *ss << pos->first.key << attribute;
      ss->flush();

      size_t ans;
      *ss >> ans;
      return ans;
   }
}



std::pair<Cache::iterator, bool> 
MasterSlave::
insert_impl( const Application_Base *context,
             const Key& key,
             const CoreResponseInfo& response )
{
   // The consumers of the raw insert method should know what they are
   // doing and guarantee that the context is already pointing to the
   // core application.
   if ( context != get_core_application(context) )
      EXCEPTION_MNGR(std::runtime_error,
                     "MasterSlave::insert_impl(): "
                     "INTERNAL ERROR: cannot insert a non-core application "
                     "context");

   if ( ExecuteMngr().rank() == data->masterRank )
      return data->insert(context, key, response);
   else
   {
      data->command("insert", context);
      SharedPtr<ioSerialStream> ss 
         = ExecuteMngr().serialstream(data->masterRank);
      *ss << key << response;
      ss->flush();

      std::pair<Cache::iterator, bool> ans;
      *ss >> ans;
      return ans;
   }
}


size_t
MasterSlave::erase_item( CachedKey key )
{
   if ( ExecuteMngr().rank() == data->masterRank )
      return data->erase(key.context, key.key);
   else
   {
      data->command("erase", key.context);
      SharedPtr<ioSerialStream> ss 
         = ExecuteMngr().serialstream(data->masterRank);
      *ss << key.key;
      ss->flush();

      size_t ans;
      *ss >> ans;
      return ans;
   }
}

void 
MasterSlave::erase_iterator( Cache::iterator pos )
{
   // This relies on te fact that *cacheable* keys must be unique
   erase_item(pos->first);
}


Cache::iterator
MasterSlave::find_impl( CachedKey cached_key ) const
{
   data->synchronize_slave( cached_key.context );
   return data->cache.find( cached_key );
}


Cache::iterator
MasterSlave::lower_impl( CachedKey cached_key ) const
{
   data->synchronize_slave( cached_key.context );
   return data->cache.lower_bound( cached_key );
}


Cache::iterator
MasterSlave::upper_impl( CachedKey cached_key ) const
{
   data->synchronize_slave( cached_key.context );
   return data->cache.upper_bound( cached_key );
}


void
MasterSlave::
increment_iterator(iterator::_base_ptr &it, utilib::Any &data) const
{
   EXCEPTION_MNGR(std::logic_error, "This code should be unreachable.  "
                  "MasterSlave caches use the underlying local cache "
                  "to manage iterators.");
   static_cast<void>(it);
   static_cast<void>(data);
}

void
MasterSlave::
decrement_iterator(iterator::_base_ptr &it, utilib::Any &data) const
{
   EXCEPTION_MNGR(std::logic_error, "This code should be unreachable.  "
                  "MasterSlave caches use the underlying local cache "
                  "to manage iterators.");
   static_cast<void>(it);
   static_cast<void>(data);
}


} // namespace colin::cache
} // namespace colin
