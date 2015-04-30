/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file MPIStream.cpp
 */

#include <utilib/MPIStream.h>
#include <utilib/exception_mngr.h>

// This whole class is specific to MPI builds
#ifdef UTILIB_HAVE_MPI

using std::cerr;
using std::cout;
using std::endl;

using std::map;
using std::set;
using std::list;

namespace utilib {

//=====================================================================
// MPICommManager methods
//=====================================================================

// Buffer management configuration
const size_t MPICommManager::Config::BufferSize = 4096;          //  4 KB
size_t       MPICommManager::Config::MaxPendingPoolSize = 1024;  //  4 MB
size_t       MPICommManager::Config::MaxFreePoolSize = 10;       // 40 KB

MPICommManager* MPICommManager::sm_default = NULL;

/** This is a private structure for managing the MPI transmission
 *  buffers.  As a result, we can do some peculiar things here.  Most
 *  notably, since we never really "copy" these buffers around (they are
 *  associated with a list iterator, and that iterator is moved between
 *  lists), we can make the buffer pointer behave like an AutoPointer.
 *  That is, assignment transfers ownership of the allocated data.  
 *
 *  This violated the "const-ness" of the RHS of the assignment
 *  operator. BUT, since it is only used within this file, and I AM
 *  WARNING YOU HERE, we should be OK.
 */
struct MPICommManager::MPIBuffer {
   MPIBuffer() 
      : buffer(new char[MPICommManager::Config::BufferSize]) 
   {}
   ~MPIBuffer() 
   { delete [] buffer; }

   MPIBuffer(const MPIBuffer& rhs)
      : buffer(NULL)
   { operator=(rhs); }

   MPIBuffer& operator=(const MPIBuffer& rhs)
   {
      if ( buffer == rhs.buffer )
         return *this;

      delete [] buffer;
      buffer = rhs.buffer;
      const_cast<MPIBuffer&>(rhs).buffer = NULL;
      request = rhs.request;
      iter = rhs.iter;

      return *this;
   }

   char*             buffer;
#ifdef UTILIB_HAVE_MPI_CPP_API
   MPI::Request      request;
#else
   MPI_Request       request;
#endif
   pool_t::iterator  iter;
};


struct MPICommManager::Pools {
   pool_t  freePool;
   pool_t  activePool;
   pool_t  pendingPool;
};


#ifdef UTILIB_HAVE_MPI_CPP_API
MPICommManager::MPICommManager(MPI::Intracomm comm)
#else
MPICommManager::MPICommManager(MPI_Comm comm)
#endif
   : m_comm(comm),
     m_tags(2),
     m_freePoolSize(0),
     m_pendingPoolSize(0),
     m_pools(new Pools)
{}


MPICommManager::~MPICommManager()
{
   if ( ! m_pools->activePool.empty() )
      cerr << "Warning: MPICommManager destroyed before all MPIStream "
         "clients (" << m_pools->activePool.size() << "buffers allocated)"
           << endl;

   // Explicitly empty the pool of free buffers
   m_pools->freePool.clear();
   // Wait for all pending sends to complete
   while ( ! m_pools->pendingPool.empty() )
   {
#ifdef UTILIB_HAVE_MPI_CPP_API
      m_pools->pendingPool.front().request.Wait();
#else
      MPI_Wait(&m_pools->pendingPool.front().request, MPI_STATUS_IGNORE);
#endif
      m_pools->pendingPool.pop_front();
   }
}


void MPICommManager::InitDefault()
{
   if ( sm_default != NULL )
      EXCEPTION_MNGR(std::runtime_error, "Error: duplicate initialization "
                     "of default MPICommManager.");
   sm_default = new MPICommManager(MPI_COMM_WORLD);
}


void MPICommManager::ReleaseDefault()
{
   if ( sm_default == NULL )
      EXCEPTION_MNGR(std::runtime_error, "Error: duplicate destruction "
                     "of default MPICommManager.");
   delete sm_default;
   sm_default = NULL;
}


int MPICommManager::Get_rank() const
{ 
#ifdef UTILIB_HAVE_MPI_CPP_API
   return m_comm.Get_rank();
#else
   int rank = 0;
   MPI_Comm_rank(m_comm, &rank);
   return rank;
#endif
}


MPICommManager::tag_t 
MPICommManager::get_stream_tag(Mode mode, rank_t rank)
{
   int index = -1;
   if ( mode == Send )
      index = 0;
   else if ( mode == Recv )
      index = 1;
   else
      EXCEPTION_MNGR(runtime_error, "MPICommManager::get_stream_tag(): "
                     "invalid mode.");
   
   set<tag_t> &tags = m_tags[index][rank];
   if ( tags.empty() )
      return *( tags.insert(tags.end(), 1) );

   set<tag_t>::iterator it = tags.end();
   --it;

   if ( *it < MPI_TAG_UB )
      return *( tags.insert(tags.end(), *it + 1) );

   // Somehow we have used all (32000+?!?) tags at least once.  We need
   // to find an unused one.
   it = tags.begin();

   // This is cute: if we have available tags at the beginning of the
   // list, return the largest one available so that the next time
   // around we won't have to walk the list.
   if ( *it > 1 )
      return *( tags.insert(it, *it - 1) );

   // OK...we gotta walk the list
   set<tag_t>::iterator tmp = it++;
   while ( *it - *tmp == 1 )
   {
      tmp = it++;
      if ( it == tags.end() )
         EXCEPTION_MNGR(runtime_error, "MPICommManager ran out of available "
                        "stream tags!");
   }
   return *( tags.insert(it, *it - 1) );
}


void 
MPICommManager::free_stream_tag(tag_t tag, Mode mode, rank_t rank)
{
   int index = -1;
   if ( mode == Send )
      index = 0;
   else if ( mode == Recv )
      index = 1;
   else
      EXCEPTION_MNGR(runtime_error, "MPICommManager::free_stream_tag(): "
                     "invalid mode.");

   set<tag_t> &tags = m_tags[index][rank];
   if ( tags.erase(tag) == 0 )
      EXCEPTION_MNGR(runtime_error, "MPICommManager: attempt to release "
                     "non-allocated stream tag.");
}


MPICommManager::MPIBuffer*
MPICommManager::get_buffer()
{
   // Check for any completed buffers from the Pending Pool
   if ( ! m_pools->pendingPool.empty() )
   {
      int i = 0;
      list<MPIBuffer>::iterator it = m_pools->pendingPool.begin();
#ifdef UTILIB_HAVE_MPI_CPP_API
      while (( it != m_pools->pendingPool.end()) && ( it->request.Test() ))
#else
      int test_ans = 0;
      MPI_Test(&it->request, &test_ans, MPI_STATUS_IGNORE);
      while (( it != m_pools->pendingPool.end()) && ( test_ans ))
#endif
      {
         ++it;
         ++i;
      }
      if ( i > 0 )
      {
         m_pools->freePool.splice( m_pools->freePool.end(), 
                                   m_pools->pendingPool,
                                   m_pools->pendingPool.begin(), 
                                   it );
         m_freePoolSize += i;
         m_pendingPoolSize -= i;
      }
   }

   // Determine the available buffer
   pool_t::iterator ans;
   if ( m_pools->freePool.empty() )
   {
      // Allocate a new buffer
      ans = m_pools->activePool.insert(m_pools->activePool.end(), MPIBuffer());
   }
   else
   {
      // re-use an existing free buffer
      ans = m_pools->freePool.begin();
      m_pools->activePool.splice( m_pools->activePool.end(), 
                                  m_pools->freePool, 
                                  m_pools->freePool.begin() );
      --m_freePoolSize;
   }

   // Since the buffer is now in the active pool, it needs to know it's
   // own iterator.
   ans->iter = ans;

   // If the free pool is too big, shrink it...
   while ( m_freePoolSize > Config::MaxFreePoolSize )
   {
      m_pools->freePool.pop_back();
      --m_freePoolSize;
   }

   // Return the "new" buffer.  While it would look to be a bit sketchy
   // to be tossing around pointers to data stored in a list, according
   // to te standard, this should be safe.  List iterators are only
   // invalidated when the item itself is deleted, so, since we always
   // keep the buffers in a list and we only use "splice" to move
   // buffers between lists, the pointer should remain valid.
   return &*ans;
}


void
MPICommManager::free_buffer(MPIBuffer* buffer)
{
   if (( buffer == NULL ) || ( buffer->iter == m_pools->activePool.end() ))
      EXCEPTION_MNGR(runtime_error, "MPICommManager::free_buffer(): "
                     "Attempt to free a buffer that is not in the "
                     "set of active buffers.");

   // return the buffer to the free pool
   m_pools->freePool.splice( m_pools->freePool.end(), 
                             m_pools->activePool, 
                             buffer->iter );
   buffer->iter = m_pools->activePool.end();
}


void
MPICommManager::mark_buffer_pending(MPIBuffer* buffer)
{
   if (( buffer == NULL ) || ( buffer->iter == m_pools->activePool.end() ))
      EXCEPTION_MNGR(runtime_error, "MPICommManager::mark_buffer_pending(): "
                     "Attempt to move a buffer that is not in the "
                     "set of active buffers.");

   // To prevent too many buffers from sitting around, we may have to
   // wait for one to complete.  While I would prefer to only block
   // until *any* buffer finishes, it is easier to just wait for the
   // oldest one to finalize.
   if (( m_pendingPoolSize >= Config::MaxPendingPoolSize ) && 
       ( Config::MaxPendingPoolSize > 0 ))
   {
      pool_t::iterator it = m_pools->pendingPool.begin();
#ifdef UTILIB_HAVE_MPI_CPP_API
      it->request.Wait();
#else
      MPI_Wait(&it->request, MPI_STATUS_IGNORE);
#endif
      m_pools->freePool.splice( m_pools->freePool.end(), 
                                m_pools->pendingPool, 
                                it );
      --m_pendingPoolSize;
      ++m_freePoolSize;
   }

   m_pools->pendingPool.splice( m_pools->pendingPool.end(), 
                                m_pools->activePool, 
                                buffer->iter );
   buffer->iter = m_pools->activePool.end();
   ++m_pendingPoolSize;
}

//=====================================================================
// basic_mpibuf methods
//=====================================================================

const char basic_mpibuf::ContinuationCharacter = 2;
const char basic_mpibuf::TerminationCharacter = 1;

basic_mpibuf::basic_mpibuf( MPICommManager& commMngr, 
                            int remoteRank, 
                            MPICommManager::Mode mode )
   : m_commMngr(commMngr),
     m_remoteRank(remoteRank),
     m_mode(mode),
     m_sendTag(0),
     m_recvTag(0),
     m_sendBuffer(NULL),
     m_recvBuffer(NULL),
     m_readBuffer(NULL)
{
   if ( (mode & MPICommManager::Recv) > 0 )
   {
      m_recvTag = m_commMngr.get_stream_tag(MPICommManager::Recv, remoteRank);
      m_readBuffer = m_commMngr.get_buffer();
      m_recvBuffer = m_commMngr.get_buffer();

#ifdef UTILIB_HAVE_MPI_CPP_API
      m_recvBuffer->request = m_commMngr.m_comm.Irecv
         ( m_recvBuffer->buffer, MPICommManager::Config::BufferSize, 
           MPI::BYTE, remoteRank, m_recvTag );
#else
      if ( MPI_Irecv(m_recvBuffer->buffer, MPICommManager::Config::BufferSize,
                     MPI_BYTE, remoteRank, m_recvTag, m_commMngr.m_comm,
                     &m_recvBuffer->request) )
      {
         EXCEPTION_MNGR(runtime_error, "basic_mpibuf::basic_mpibuf(): "
                        "Irecv returned error");
      }
#endif
   }
   if ( (mode & MPICommManager::Send) > 0 )
   {
      m_sendTag = m_commMngr.get_stream_tag(MPICommManager::Send, remoteRank);
      m_sendBuffer = m_commMngr.get_buffer();
   }
   if ( mode == MPICommManager::BCast )
   {
      if ( remoteRank == m_commMngr.Get_rank() )
         m_sendBuffer = m_commMngr.get_buffer();
      else
         m_recvBuffer = m_commMngr.get_buffer();
   }
   
   //cerr << commMngr.Get_rank() << ": tags and buffers created:" << endl;
   //cerr << "  tags = " << m_recvTag << ", " << m_sendTag << endl;
   //cerr << "  buffers = " << (void*)m_recvBuffer->buffer << ", " 
   //     << (void*)m_readBuffer->buffer << ", " 
   //     << (void*)m_sendBuffer->buffer << endl;

   // set up the input streambuf pointers to initially trigger an underflow
   setg(0,0,0);
   m_inputEOF = ( m_recvBuffer == NULL );

   // set up the output streambuf pointers to point at the first buffer
   if ( m_sendBuffer == NULL )
      setp(0,0);
   else
   {
      char* end = m_sendBuffer->buffer + MPICommManager::Config::BufferSize-1;
      if ( mode == MPICommManager::BCast )
         end -= sizeof(int);
      setp(m_sendBuffer->buffer, end);
           
   }
}


basic_mpibuf::~basic_mpibuf()
{
   // Close down my side of the communication (this moves my send buffer
   // into the Pending pool - and thus under the responsibility of the
   // MPICommManager).
   if ( m_sendBuffer != NULL )
      send_active_buffer(true);

   if ( m_recvBuffer != NULL )
   {
      // flush any un-read data from the stream
      while ( ! m_inputEOF ) 
         underflow();
      m_commMngr.free_buffer(m_recvBuffer);
   }
   if ( m_readBuffer != NULL )
      m_commMngr.free_buffer(m_readBuffer);

   // Release my tags
   if ( m_recvTag != 0 )
      m_commMngr.free_stream_tag(m_recvTag,MPICommManager::Recv,m_remoteRank);
   if ( m_sendTag != 0 )
      m_commMngr.free_stream_tag(m_sendTag,MPICommManager::Send,m_remoteRank);
}


bool
basic_mpibuf::dataAvailable()
{
   // Is there data still in the active read buffer?
   if ( gptr() < egptr() )
      return true;
   // Is this a readable stream (and NOT a broadcast stream)?
   if (( m_inputEOF ) || ( m_readBuffer == NULL ))
      return false;
   // Has the pending receive request processed?
#ifdef UTILIB_HAVE_MPI_CPP_API
   MPI::Status status;
   if ( ! m_recvBuffer->request.Test(status) )
#else
   MPI_Status status;
   int test = 0;
   MPI_Test(&m_recvBuffer->request, &test, &status);
   if ( ! test )
#endif
      return false;

   // Because the Test function is destructive, we MUST immediately
   // process the incoming data if the test returns true.
#ifdef UTILIB_HAVE_MPI_CPP_API
   int dataSize = status.Get_count(MPI::BYTE);
#else
   int dataSize = 0;
   MPI_Get_count(&status, MPI_BYTE, &dataSize);
#endif
   process_received_data(dataSize);
   return ( dataSize > 1);
}


basic_mpibuf::pos_type
basic_mpibuf::seekpos( pos_type,  std::ios_base::openmode )
{
   EXCEPTION_MNGR(runtime_error, "basic_mpibuf is not seekable");
   return pos_type(-1);
}


basic_mpibuf::pos_type
basic_mpibuf::seekoff( off_type,  std::ios_base::seekdir,
                       std::ios_base::openmode )
{
   EXCEPTION_MNGR(runtime_error, "basic_mpibuf is not seekable");
   return pos_type(-1);
}



basic_mpibuf::int_type
basic_mpibuf::sync()
{
   // NB: This only guarantees that ISend has been called -- NOT that
   // the MPI request has completed.
   if ( pptr() != pbase() )
      return overflow(traits_type::eof());

   return 0;  // this needs to be anything BUT traits_type::eof. (?)
}


basic_mpibuf::int_type 
basic_mpibuf::overflow(int_type c)
{
   assert(m_sendBuffer != NULL);

   //cerr << "send buffer " << (void*)m_sendBuffer->buffer << endl;
   //cerr << "sending " << (pptr() - pbase()) << " chars" << endl << "   ";
   //char* i = pbase();
   //while ( i < pptr() )
   //{
   //   if ( *i > 32 )
   //      cerr << *i << " ";
   //   else
   //      cerr << static_cast<int>(*i) << " ";
   //   ++i;
   //}
   //cerr << endl;


   // send the buffer
   send_active_buffer(false);

   // get a new buffer
   m_sendBuffer = m_commMngr.get_buffer();

   // set up the streambuf pointers
   char* end = m_sendBuffer->buffer + MPICommManager::Config::BufferSize - 1;
   if ( m_mode == MPICommManager::BCast )
      end -= sizeof(int);

   setp(m_sendBuffer->buffer, end);

   // if necessary, write out the pending character
   if (c != traits_type::eof())
   {
      *pptr() = static_cast<char>(c);
      pbump(1);
   }

   return 0; // this needs to be anything BUT traits_type::eof.
}


basic_mpibuf::int_type 
basic_mpibuf::underflow()
{
   if ( m_inputEOF )
      return traits_type::eof();

   assert(m_recvBuffer != NULL);

   // If this is a bi-directional link, then we may need to flush the
   // outgoing buffer before waiting for a reply from the remote
   // process.
   if ( m_sendBuffer != NULL )
      sync();

   // Wait for the next chunk of data
   int dataSize = 0;
   if ( m_mode == MPICommManager::BCast )
   {
#ifdef UTILIB_HAVE_MPI_CPP_API
      m_commMngr.m_comm.Bcast( m_recvBuffer->buffer, 
                               MPICommManager::Config::BufferSize, 
                               MPI::BYTE, m_remoteRank );
#else
      MPI_Bcast(m_recvBuffer->buffer, MPICommManager::Config::BufferSize, 
                MPI_BYTE, m_remoteRank, m_commMngr.m_comm);
#endif
      dataSize = * reinterpret_cast<int*>
         ( m_recvBuffer->buffer + MPICommManager::Config::BufferSize 
           - sizeof(int) );
   }
   else
   {
#ifdef UTILIB_HAVE_MPI_CPP_API
      MPI::Status status;
      m_recvBuffer->request.Wait(status);
      dataSize = status.Get_count(MPI::BYTE);
#else
      MPI_Status status;
      MPI_Wait(&m_recvBuffer->request, &status);
      dataSize = 0;
      MPI_Get_count(&status, MPI_BYTE, &dataSize);
#endif
   }

   process_received_data(dataSize);

   if ( dataSize > 1 )
      return 0; // Something that is NOT traits_type::eof()
   else if ( m_inputEOF )
      return traits_type::eof();
   else
      return 0; // Something that is NOT traits_type::eof()
}


void
basic_mpibuf::send_active_buffer(bool isEOF)
{
   std::streamsize dataSize = pptr() - pbase();
   assert(dataSize < static_cast<std::streamsize>(MPICommManager::Config::BufferSize));
   
   // add the "non-termination character" (safe since I lie to the
   // basic_streambuf about the actual size of the buffer)
   *pptr() = (isEOF ? TerminationCharacter : ContinuationCharacter);
   ++dataSize;

   if ( m_mode == MPICommManager::BCast )
   {
      assert(dataSize <= static_cast<std::streamsize>(MPICommManager::Config::BufferSize - sizeof(int)));

      // Because there is no "non-blocking broadcast", we can broadcast
      // the message and immediately return the buffer to the free pool
      * reinterpret_cast<int*>
         ( m_sendBuffer->buffer + MPICommManager::Config::BufferSize 
           - sizeof(int) ) = dataSize;
#ifdef UTILIB_HAVE_MPI_CPP_API
      m_commMngr.m_comm.Bcast( m_sendBuffer->buffer, 
                               MPICommManager::Config::BufferSize, 
                               MPI::BYTE, m_remoteRank );
#else
      MPI_Bcast( m_sendBuffer->buffer, MPICommManager::Config::BufferSize, 
                 MPI_BYTE, m_remoteRank, m_commMngr.m_comm );
#endif
      m_commMngr.free_buffer(m_sendBuffer);
   }
   else
   {
#ifdef UTILIB_HAVE_MPI_CPP_API
      m_sendBuffer->request = m_commMngr.m_comm.Isend
         (m_sendBuffer->buffer, dataSize, MPI::BYTE, m_remoteRank, m_sendTag);
#else
      MPI_Isend( m_sendBuffer->buffer, dataSize, MPI_BYTE, m_remoteRank, 
                 m_sendTag, m_commMngr.m_comm, &m_sendBuffer->request);
#endif
      m_commMngr.mark_buffer_pending(m_sendBuffer);
   }

   m_sendBuffer = NULL;
}


void 
basic_mpibuf::process_received_data(int dataSize)
{
   //cerr << "recv buffer == " << (void*)m_recvBuffer->buffer << endl;
   //cerr << "received " << (dataSize) << " chars" << endl << "   ";
   //char* i = m_recvBuffer->buffer;
   //int n = dataSize;
   //while ( n > 0 )
   //{
   //   if ( *i > 32 )
   //      cerr << *i << " ";
   //   else
   //      cerr << static_cast<int>(*i) << " ";
   //   ++i;
   //   --n;
   //}
   //cerr << endl;


   // the last char is always the continuation flag
   --dataSize; 

   // Test if this is the last chunk of data
   if ( m_recvBuffer->buffer[dataSize] == TerminationCharacter )
      m_inputEOF = true;

   if ( m_mode == MPICommManager::BCast )
   {
      setg( m_recvBuffer->buffer, m_recvBuffer->buffer,
            m_recvBuffer->buffer + dataSize );
   }
   else
   {
      // Swap the buffers
      MPICommManager::MPIBuffer *tmp = m_recvBuffer;
      m_recvBuffer = m_readBuffer;
      m_readBuffer = tmp;
      
      // initialize the basic_streambuf pointers
      setg( m_readBuffer->buffer, m_readBuffer->buffer,
            m_readBuffer->buffer + dataSize );
   
      // start getting the next chunk of data
      if ( ! m_inputEOF )
      {
#ifdef UTILIB_HAVE_MPI_CPP_API
         m_recvBuffer->request = m_commMngr.m_comm.Irecv
            ( m_recvBuffer->buffer, MPICommManager::Config::BufferSize, 
              MPI::BYTE, m_remoteRank, m_recvTag );
#else
         if ( MPI_Irecv(m_recvBuffer->buffer, 
                        MPICommManager::Config::BufferSize, MPI_BYTE, 
                        m_remoteRank, m_recvTag, m_commMngr.m_comm,
                        &m_recvBuffer->request) )
         {
            EXCEPTION_MNGR(runtime_error, 
                           "basic_mpibuf::process_received_data(): "
                           "Irecv returned error");
         }
#endif
      }
   }
}

//=====================================================================
// MPIStream methods
//=====================================================================

oMPIStream::oMPIStream(int remoteRank, MPICommManager &mngr)
   : mpibuf(mngr, remoteRank, MPICommManager::Send)
{
   init(&mpibuf); 
   init_buffer(std::ios_base::out);
   init_stream();
}


iMPIStream::iMPIStream(int remoteRank, MPICommManager &mngr)
   : iBinarySerialStream(true),
     mpibuf(mngr, remoteRank, MPICommManager::Recv)
{
   init(&mpibuf); 
   init_buffer(std::ios_base::in);
   init_stream();
}


MPIStream::MPIStream(int remoteRank, MPICommManager &mngr)
   : BinarySerialStream(true),
     mpibuf(mngr, remoteRank, MPICommManager::Bidirectional)
{
   init(&mpibuf); 
   init_buffer(std::ios_base::out | std::ios_base::in);
   init_stream();
}

//=====================================================================
// MPIBroadcastStream methods
//=====================================================================


MPIBroadcastStream::MPIBroadcastStream(int sourceRank, MPICommManager &mngr)
   : BinarySerialStream(true),
     mpibuf(mngr, sourceRank, MPICommManager::BCast)
{
   init(&mpibuf); 
   if ( mngr.Get_rank() == sourceRank )
   {
      init_buffer(std::ios_base::out);
      oBinarySerialStream::init_stream();
   }
   else
   {
      init_buffer(std::ios_base::in);
      iBinarySerialStream::init_stream();
   }
}


} // namespace utilib

#endif // UTILIB_HAVE_MPI
