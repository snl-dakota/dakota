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
 * \file   CommonIO.h
 *
 * Defines the utilib::CommonIO class.
 *
 * \author William E. Hart
 */

#ifndef utilib_CommonIO_h
#define utilib_CommonIO_h

#include <utilib_config.h>
#include <utilib/std_headers.h>

// NB: If compilers gripe about static_cast<void>(0), we can switch back
// to the "do { } while ( false )" form (which, while portable, may
// defeat some compiler's in-lining rules, see: 
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.5

/// Executes \p cmd if \c verbosity(dlevel) is true.
#define OUTPUTPR(dlevel,cmd)  \
  if ( true ) { \
    if ( this->verbosity(dlevel) )  \
      { cmd; utilib::CommonIO::flush(); }  \
  } else static_cast<void>(0)

/// Similar to \c OUTPUTPR but does not flush the output.
#define OUTPUTPRP(dlevel,cmd) \
  if ( true ) { \
    if ( this->verbosity(dlevel) )  \
      { cmd; }  \
  } else static_cast<void>(0)

/**
 * Similar to \c OUTPUTPR, but uses a pointer to another CommonIO-based
 * class. It can be used to debug output from a class not based
 * on CommonIO, provided the verbosity method is defined for that
 * class.
 */
#define OUTPUTPRX(dlevel,ptr,stuff) \
  if ( true ) { \
    if ( (ptr)->verbosity(dlevel) ) \
      { ucout << stuff; utilib::CommonIO::flush(); } \
  } else static_cast<void>(0)

/**
 * Similar to \c OUTPUTPRX, but does not flush the output.
 */
#define OUTPUTPRXP(dlevel,ptr,stuff) \
  if ( true ) { \
    if ( (ptr)->verbosity(dlevel) )  \
      { ucout << stuff; } \
  } else static_cast<void>(0)


#if defined(UTILIB_YES_DEBUGPR) || defined(DOXYGEN)
  #ifdef DEBUGPR
    #undef DEBUGPR
  #endif

  /// Executes \p cmd if \c cond is true.
  #define DEBUG_IF(cond,cmd)  \
    if ( true ) { \
      if ( cond ) \
        { cmd; utilib::CommonIO::flush(); }  \
    } else static_cast<void>(0)

  /// Executes \p cmd if \c verbosity(dlevel) is true.
  #define DEBUGPR(dlevel,cmd)  \
    if ( true ) { \
      if ( this->verbosity(dlevel) ) \
        { cmd; utilib::CommonIO::flush(); }  \
    } else static_cast<void>(0)

  /// Similar to \c DEBUGPR but does not flush the output.
  #define DEBUGPRP(dlevel,cmd)  \
    if ( true ) { \
      if ( this->verbosity(dlevel) )  \
        { cmd; }  \
    } else static_cast<void>(0)

  /**
   * Similar to \c DEBUGPR, but uses a pointer to another CommonIO-based
   * class. It can be used to debug output from a class not based
   * on CommonIO, provided the verbosity method is defined for that
   * class.
   */
  #define DEBUGPRX(dlevel,ptr,stuff)  \
    if ( true ) { \
     if ( (ptr)->verbosity(dlevel) )  \
       { ucout << stuff; utilib::CommonIO::flush(); } \
    } else static_cast<void>(0)

  /**
   * Similar to \c DEBUGPRX, but does not flush the output.
   */
  #define DEBUGPRXP(dlevel,ptr,stuff) \
    if ( true ) { \
     if ( (ptr)->verbosity(dlevel) )  \
       { ucout << stuff; }  \
    } else static_cast<void>(0)

  /**
   * Declares that the debug level of the class should be taken from
   * the object pointed to by \p pointer.
   */
  #define REFER_DEBUG(pointer) \
    virtual int verbosity(int level) const  \
      { return pointer->verbosity(level); };

#else
  // Do nothing....
  #define DEBUG_IF(cond,cmd )          static_cast<void>(0)
  #define DEBUGPR(dlevel,cmd)          static_cast<void>(0)
  #define DEBUGPRP(dlevel,cmd)         static_cast<void>(0)
  #define DEBUGPRX(ptr,dlevel,stuff)   static_cast<void>(0)
  #define DEBUGPRXP(ptr,dlevel,stuff)  static_cast<void>(0)
  #define REFER_DEBUG(pointer) \
    virtual int verbosity(int level) const { return 0; }
#endif



#include <utilib/_generic.h>

// [15 Sep 11] JDS: removing these until someone can prove they are necessary
//#if !defined(UTILIB_HAVE_NAMESPACES)
//using std::ofstream;
//using std::ios;
//using std::ostream;
//using std::istream;
//#endif


namespace utilib {

class CommonIO;

/**
 * The globally defined CommonIO object that is used for functions.
 * This is currently not defined because of problems with static
 * initialization with streams.
 */
utilib::CommonIO& global_CommonIO();

} // namespace utilib

//----------------------------------------------------------------------------
#if defined(DOXYGEN) || ! defined(UTILIB_DISABLE_COMMONIO)

// [15 Sept 11] JDS: Can someone document why this is necessary?
#ifndef SWIG

/**
 * Definition for \c ucout masks the standard definition of the cout stream,
 * redirecting I/O through the CommonIO method \c outs.
 */
#define ucout	(*((utilib::CommonIO::io_mapping && (utilib::CommonIO::begin_end_counter > 0)) ? (utilib::CommonIO::MapCout) : &std::cout))
//#define ucout	(CommonIO::io_mapping && (CommonIO::begin_end_counter > 0) ? *(*(CommonIO::MapCout)) : std::cout)
/**
 * Definition for \c ucerr masks the standard definition of the cerr stream,
 * redirecting I/O through the CommonIO method \c errs.
 */
#define ucerr	(*((utilib::CommonIO::io_mapping && (utilib::CommonIO::begin_end_counter > 0)) ? utilib::CommonIO::MapCerr : &std::cerr))
//#define ucerr	(typename CommonIO::io_mapping && (typename CommonIO::begin_end_counter > 0) ? *(typename CommonIO::MapCerr) : typename std::cerr)
/**
 * Definition for \c ucin masks the standard definition of the cin stream,
 * redirecting I/O through the CommonIO method \c ins.
 */
#define ucin	(*((utilib::CommonIO::io_mapping && (utilib::CommonIO::begin_end_counter > 0)) ? utilib::CommonIO::MapCin : &std::cin))
/**
 * A macro that is always maps to the standard cout stream.
 */
#define stdcout (std::cout)
/**
 * A macro that is always maps to the standard cerr stream.
 */
#define stdcerr (std::cerr)
/**
 * A macro that is always maps to the standard cin stream.
 */
#define stdcin  (std::cin)

#endif // SWIG

//----------------------------------------------------------------------------
#else // ! [ defined(DOXYGEN) || ! defined(UTILIB_DISABLE_COMMONIO) ]

#define ucout (std::cout)
#define ucerr (std::cerr)
#define ucin  (std::cin)
#define stdcout (std::cout)
#define stdcerr (std::cerr)
#define stdcin  (std::cin)

#endif // defined(DOXYGEN) || ! defined(UTILIB_DISABLE_COMMONIO)
//----------------------------------------------------------------------------



extern "C" {
/**
 * Generic function for printing a stream to cout.  Useful for writing
 * C code that uses C++ streams without tieing stdio and C++ streams
 * directly.
 */
void cout_print(const char* str);
/**
 * Generic function for printing a stream to cerr.  Useful for writing
 * C code that uses C++ streams without tieing stdio and C++ streams
 * directly.
 */
void cerr_print(const char* str);
}

namespace utilib {

///
/// A class for coordinating I/O to enable tagging of IO and to control 
/// the printing of output.
///
class CommonIO {

  #if !defined(DOXYGEN)
  friend void cppMessage_abort();
  friend std::ostream& Flush(std::ostream& outstr);
  #endif

public:

  /// Constructor
  CommonIO();

  /// Destructor - flushes the I/O streams
  virtual ~CommonIO();

  /// Begin using CommonIO to redirect IO
  static void begin();

  /// Begin using CommonIO to redirect IO using buffered IO
  static void begin_buffered();

  /// End the use of CommonIO to redirect IO
  static void end();

  /// Turn off CommonIO mapping (by default it is on)
  static void map_off()
		{flush(); io_mapping = false; reset_map();}
  /// Turn on the CommonIO mapping

  static void map_on()
		{ io_mapping = true; reset_map();}

  /// Resets the I/O streams to the C++ standard streams
  static void reset();

  /// Specifies the debugging level (0 means no debugging)
  int debug;

  /// The maximum number of digits needed to print the I/O counter
  static int numDigits;

  /// Returns true if I/O is allowed from this process
  virtual int  verbosity(const int level) const
    		{
      		if ((IO_Rank != -1) && (IO_Rank != Rank))
		   return 0;
      		else
		   return (debug >= level);
    		}

  /// Set the debugging field
  void  setDebug(int level)
		{ debug = level; }

  /// Set the rank of the processor that is allowed to do I/O 
  static void  setIORank(int Rank_)
		{ IO_Rank = Rank_; }

  /// Set the flag to indicate whether I/O is explicitly flushed
  static void setIOFlush(int flush_)
		{ flush_flag = flush_; };

  /// Rank of the current process
  static int rank()
		{ return Rank; };

  /// Rank of the current process
  static int io_rank()
		{ return IO_Rank; };

  /// Number of processes
  static int numProcs()
		{ return Size; };

  /// The function for reseting the streams.  Null streams are unchanged.
  static void set_streams(std::ostream* cout_, std::ostream* cerr_, std::istream* cin_)
		{
		if (cout_) { common_cout = cout_; }
		if (cerr_) { common_cerr = cerr_; }
		if (cin_)  { common_cin  = cin_; }
		reset_map();
		}

  /// The function for reseting the cout stream.
  static void set_cout(std::ostream* cout_)
		{ if (cout_) {common_cout = cout_; reset_map();} }

  /// The function for reseting the cerr stream.
  static void set_cerr(std::ostream* cerr_)
		{ if (cerr_) {common_cerr = cerr_; reset_map();} }

  /// The function for reseting the cin stream.
  static void set_cin(std::istream* cin_)
		{ if (cin_)  {common_cin  = cin_; reset_map();} }

  /// Redirect ucout/ucerr I/O to seperate output streams for each processor
  static void set_ofile(const char* ofile);

  /// Begin tagging of IO
  static void begin_tagging()
		{IOflush(); tagging=true;}

  /// Begin tagging with a specified number of digits for each tag
  static void begin_tagging(const int numDigits_)
		{IOflush(); numDigits = numDigits_; tagging=true;}

  /// Stop tagging of IO
  static void end_tagging()
		{IOflush(); tagging=false;}

  /// Indicates whether tagging is currently enabled
  static int tagging_active() { return tagging; };

  /// Flush the IO buffers
  static void flush()
		{
		IOflush(); 
		if (common_cout) common_cout->flush(); 
		if (common_cerr) common_cerr->flush();
		}

  /// Synchronized the C++ streams with the STDIO streams
  static void sync_stdio()
		{
#ifdef UTILIB_HAVE_NAMESPACES
		std::ios::sync_with_stdio();
#endif
		}

  /// The C++ stream object used to process ucout stream information
  static std::ostream* MapCout;
  /// The C++ stream object used to process ucerr stream information
  static std::ostream* MapCerr;
  /// The C++ stream object used to process ucin stream information
  static std::istream* MapCin;

  /// Pointer to the standard cout stream
  static std::ostream* common_cout;
  /// Pointer to the standard cerr stream
  static std::ostream* common_cerr;
  /// Pointer to the standard cin stream
  static std::istream* common_cin;
  /** This is a masking flag.  If false, then CommonIO mapping is disabled. 
    * Otherwise, it CommonIO IO management proceeds normally.  By default, this
    * is true.
    */
  static bool io_mapping;
  /** The number of nested CommonIO environments that have been initiated.
    * If none have been initiated, then CommonIO does not map IO, and 
    * the std:: streams are used.
    */
  static int begin_end_counter;

  /// End the use of CommonIO to redirect IO, completely reseting 
  static void end_all();

  /// If true, then flush before exiting.
  static bool atexit_flag;
#ifndef UTILIB_HAVE_SSTREAM
  /// Pointer to the C++ stream object used to process cout stream info
  static strstream* pStrCout;
  /// Pointer to the C++ stream object used to process cerr stream info
  static strstream* pStrCerr;
#else
  /// Pointer to the C++ stream object used to process cout stream info
  static std::stringstream* pStrCout;
  /// Pointer to the C++ stream object used to process cerr stream info
  static std::stringstream* pStrCerr;
#endif

private:

  /// The number of nest CommonIO blocks that are buffered
  static int io_buffering;

  /// The rank of the processor that is allowed to do debugging I/O
  static int IO_Rank;
  /// The tagging flag
  static bool tagging;
  /// The number of times that CommonIO::begin has been called
  static int nref;
  /// Flag that indicates whetner 
  static int flush_flag;
  /// The number of I/O lines that have been printed
  static int seqNum;

  /// A flag that indicates whether or not MPI has been called to setup
  /// the rank and size information.
  static bool mpi_initialized;
  /// The rank of the current process
  static int Rank;
  /// The number of parallel processes
  static int Size;

  /// Sets up internal variables and returns an output stream
  static std::ostream& o_stream(const int flag);
  /// Output stream flag: 0 = common_cout, 1 = common_cerr
  static int stream_flag;
  /// If true, then try to print the rank header before next character
  static bool header_flag;
  /// Performs the flush the internal IO buffers
  static void IOflush();
  /// Performs the flush of a specific IO buffer
#ifndef UTILIB_HAVE_SSTREAM
  static void flush_stream(strstream& stream, int val);
#else
  static void flush_stream(std::stringstream& stream, int val);
#endif
  /// the common ofstream 
  static std::ofstream* common_ofstr;

  /// Resets the values of the Map# streams based on the current state
  static void reset_map();

};


/**
 * Flushes the \a CommonIO internal buffer
 */
inline std::ostream& Flush(std::ostream& outstr)
{
   if (CommonIO::io_mapping)
      CommonIO::flush();
   else
      outstr.flush();
   return outstr;
}

} // namespace utilib

#endif
