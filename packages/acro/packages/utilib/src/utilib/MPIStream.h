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
 * \file MPIStream.h
 */
#ifndef utilib_MPIStream_h
#define utilib_MPIStream_h

#include <utilib_config.h>

// This whole class is specific to MPI builds
#ifdef UTILIB_HAVE_MPI

#include <utilib/BinarySerialStream.h>

#include <mpi.h>


namespace utilib {

class basic_mpibuf;

/**
 * The MPICommManager provides a mechanism for managing the parings of
 * input and output streams between MPI ranks within a MPI communicator.
 * The MPICommManager must exist before the first MPIStream is created
 * and must not be deleted until after all MPIStreams are deleted.  
 * 
 * <B>Note:</B> The MPICommManager constructor makes a private duplicate
 * of the passed Comm object.  This duplication is a collective
 * operation, and as such, must be done at the same time by all ranks in
 * the communicator.
 *
 * For convenience, constructing an MPIStream or MPIBroadcastStream
 * without specifying a MPICommManager will use the default
 * MPICommManager::Default() manager (based on MPI_COMM_WORLD).  The
 * Default() manager will be created implicitly during the first call to
 * Default(), OR may be explicitly created by calling
 * MPICommManager::InitDefault().
 *
 * The MPICommManager allocates sender-reciever pair-specific tags.
 * This allows two ranks to have multiple active streams between them
 * without "crossing the streams."  Tag=0 is reserved for reply messages
 * for MPIBroadcastStreams.
 *
 * <B>Note:</b> Streams from one rank to another must be allocated and
 * deallocated in the same order on both processes.  For example, doing
 * the following WILL create a situation where the streams will cross --
 * probably resulting in deadlock:
 *
 * <table>
 *   <tr><th>Rank 1</th>  <th>Rank 2</th></tr>
 *   <tr><td>
 *      oMPIStream *a = oMPIStream(2);<BR>
 *      oMPIStream *b = oMPIStream(2);<BR>
 *      oMPIStream *c = oMPIStream(2);<BR>
 *      delete b;
 *   </td><td>
 *      iMPIStream *a = iMPIStream(1);<BR>
 *      iMPIStream *b = iMPIStream(1);<BR>
 *      delete b;<BR>
 *      iMPIStream *c = iMPIStream(1);
 *   </td></tr>
 */
class MPICommManager
{
   struct MPIBuffer;
   friend class basic_mpibuf;

public:
   struct Config
   {
      static const size_t BufferSize; ///< Individual buffer size (bytes)
      /// Max buffers with pending MPI ISend.  Set to 0 for no limit
      static size_t MaxPendingPoolSize;
      /// Max unused buffers to keep around.  Set to 0 for no limit
      static size_t MaxFreePoolSize;
   };

   enum Mode { 
      BCast = 0, 
      Send  = 1, 
      Recv  = 2, 
      Bidirectional = 3
   };

#ifdef UTILIB_HAVE_MPI_CPP_API
   MPICommManager(MPI::Intracomm comm);
#else
   MPICommManager(MPI_Comm comm);
#endif

   ~MPICommManager();

   static void InitDefault();
   static void ReleaseDefault();
   static MPICommManager& Default()
   { 
      if ( sm_default == NULL )
         InitDefault();
      return *sm_default; 
   }

   int Get_rank() const;

private:
   typedef int  tag_t;
   typedef int  rank_t;

   tag_t get_stream_tag(Mode mode, rank_t rank);
   void free_stream_tag(tag_t tag, Mode mode, rank_t rank);

   
   /// Get a buffer (places it in the active pool)
   MPIBuffer* get_buffer();
   /// Return a buffer from the active pool to the free pool
   void free_buffer(MPIBuffer* buffer);
   /// Move a buffer from the active pool into the pending pool
   void mark_buffer_pending(MPIBuffer* buffer);

private:
   static MPICommManager*  sm_default;

   typedef std::map<rank_t, std::set<tag_t> >  tagSet_t;
   typedef std::list<MPIBuffer>  pool_t;

#ifdef UTILIB_HAVE_MPI_CPP_API
   MPI::Intracomm  m_comm;
#else
   MPI_Comm  m_comm;
#endif
   std::vector<tagSet_t>  m_tags;

   size_t  m_freePoolSize;
   size_t  m_pendingPoolSize;

   struct Pools;
   Pools *m_pools;
};


/** This class provides the basic MPI buffer management system and the
 *  interface to the actual raw MPI commands.
 */
class basic_mpibuf : public std::basic_streambuf<char> 
{
public:
   basic_mpibuf( MPICommManager& commMngr, 
                 int remoteRank, 
                 MPICommManager::Mode mode );
   virtual ~basic_mpibuf();

   bool dataAvailable();

protected:
   // MPI streams must explicitly disable buffer seeking
   virtual pos_type seekpos( pos_type,  std::ios_base::openmode m 
                             = std::ios_base::in | std::ios_base::out );
   // MPI streams must explicitly disable buffer seeking
   virtual pos_type seekoff( off_type,  std::ios_base::seekdir,
                             std::ios_base::openmode m
                             = std::ios_base::in | std::ios_base::out );

   virtual int_type sync();
   virtual int_type overflow(int_type c = traits_type::eof());
   virtual int_type underflow();

private: // methods
   void process_received_data(int dataSize);
   void send_active_buffer(bool isEOF);

private: // data
   static const char ContinuationCharacter;
   static const char TerminationCharacter;

   MPICommManager        &m_commMngr;
   int                    m_remoteRank;
   MPICommManager::Mode   m_mode;
   MPICommManager::tag_t  m_sendTag;
   MPICommManager::tag_t  m_recvTag;
   bool                   m_inputEOF;

   MPICommManager::MPIBuffer  *m_sendBuffer;
   MPICommManager::MPIBuffer  *m_recvBuffer;
   MPICommManager::MPIBuffer  *m_readBuffer;
};



/** This provides a basic output SerialStream for point-to-point
 *  communication between to ranks in an MPI application.
 */
class oMPIStream : public oBinarySerialStream
{
public:
   oMPIStream( int remoteRank, 
               MPICommManager &mngr = MPICommManager::Default() );

private:
   basic_mpibuf mpibuf;
};


/** This provides a basic input SerialStream for point-to-point
 *  communication between to ranks in an MPI application.
 */
class iMPIStream : public iBinarySerialStream
{
public:
   iMPIStream( int remoteRank, 
               MPICommManager &mngr = MPICommManager::Default() );

   bool dataAvailable() 
   { return mpibuf.dataAvailable(); }
   
private:
   basic_mpibuf mpibuf;
};


/** This provides a basic bidirectional SerialStream for point-to-point
 *  communication between to ranks in an MPI application.
 */
class MPIStream : public BinarySerialStream
{
public:
   MPIStream( int remoteRank, 
              MPICommManager &mngr = MPICommManager::Default() );

   bool dataAvailable() 
   { return mpibuf.dataAvailable(); }
   
private:
   basic_mpibuf mpibuf;
};


/** This provides a basic Broadcast Serial Stream for broadcast
 *  communication to all ranks in a specified communicator within a MPI
 *  application.  The Broadcast stream constructor takes the broadcast
 *  source rank as its only constructor.
 *
 *  As Broadcast is a blocking operation, you should never have more
 *  than one Broadcast operation active at a time.  While you might be
 *  able to get away with it (due to MPI's internal buffering), you
 *  really shouldn't try.  Creating two MPIBroadcastStream objects on a
 *  single rank that are either both broadcasters or both receivers from
 *  the same source WILL cause problems.
 *
 *  While MPIBroadcastStream looks like a bidirectional stream, it is
 *  not.  The source rank is write-only and all other ranks are
 *  read-only.  This convention was adopted for simplicity and to
 *  (somewhat) conform to MPI's BCast() API.
 */
class MPIBroadcastStream : public BinarySerialStream 
{
public:
   /// Constructor for all ranks
   MPIBroadcastStream( int sourceRank, 
                       MPICommManager &mngr = MPICommManager::Default() );

private:
   basic_mpibuf mpibuf;
};


} // namespace utilib

#endif // UTILIB_HAVE_MPI
#endif // utilib_MPIStream_h
