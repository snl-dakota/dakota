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
 * \file CommonIO.cpp
 *
 * \author William E. Hart
 */

#include <utilib/std_headers.h>
#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif
#include <utilib/CommonIO.h>
#include <utilib/exception_mngr.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

namespace utilib {

#undef cout
#undef cerr
#undef cin

//
// Here is the logic for the control variables for CommonIO
//
// io_mapping=false
//	CommonIO streams pass through to std:: streams.
//
// io_mapping=true
//	CommonIO streams are used
//
// begin_end_counter = 0
//
//	CommonIO streams pass through to std:: streams.
// begin_end_counter > 0
//	
//

//
// Note: WEH - I had some problems when I defined this macro as returning a
// *(CommonIO::common_cout) object, which I don't entirely understand.
//
#define STREAM(val)\
	(val==0 ? (CommonIO::common_cout ? CommonIO::common_cout : &(std::cout)) : (CommonIO::common_cerr ? CommonIO::common_cerr : &(std::cerr)))

#if defined(__GNUC__) && (__GNUC__ < 3)
#define IOS_IN ios::in
#define IOS_OUT ios::out
#else
#define IOS_IN ios_base::in
#define IOS_OUT ios_base::out
#endif

extern "C" void cout_print(const char* str)
{ cout << str << Flush; }

extern "C" void cerr_print(const char* str)
{ cerr << str << Flush; }


CommonIO* global_CommonIOptr=0;

CommonIO& global_CommonIO()
{
if (!global_CommonIOptr)
   global_CommonIOptr = new CommonIO;
return *global_CommonIOptr;
}


#ifdef UTILIB_HAVE_NAMESPACES
namespace {
#endif

extern "C" void CommonIO_fini()
{
if (!CommonIO::io_mapping) {

   #ifndef UTILIB_HAVE_SSTREAM
   if (CommonIO::pStrCout && (CommonIO::pStrCout->pcount() > 0))
      EXCEPTION_MNGR(runtime_error,"CommonIO_fini - io_mapping disabled but cout buffer is not empty!");
   if (CommonIO::pStrCerr && (CommonIO::pStrCerr->pcount() > 0))
      EXCEPTION_MNGR(runtime_error,"CommonIO_fini - io_mapping disabled but cerr buffer is not empty!");
   #else
   if (CommonIO::pStrCout && (CommonIO::pStrCout->str().size() > 0))
      EXCEPTION_MNGR(runtime_error,"CommonIO_fini - io_mapping disabled but cout buffer is not empty!");
   if (CommonIO::pStrCerr && (CommonIO::pStrCerr->str().size() > 0))
      EXCEPTION_MNGR(runtime_error,"CommonIO_fini - io_mapping disabled but cerr buffer is not empty!");
   #endif
   return;
   }
//CommonIO::atexit_flag=true;
//CommonIO::end();
delete global_CommonIOptr;
}

int CommonIO_init()
{
if (!global_CommonIOptr)
   global_CommonIOptr = new CommonIO;
//CommonIO::begin();
return atexit(&CommonIO_fini);
}

int atexit_val = CommonIO_init();


#ifdef UTILIB_HAVE_NAMESPACES
}
#endif



//
// Define static variables
//
int CommonIO::begin_end_counter = 0;

bool CommonIO::mpi_initialized=false;
int CommonIO::Rank = 0;
int CommonIO::IO_Rank = 0;
int CommonIO::Size = 1;

bool CommonIO::atexit_flag = false;
bool CommonIO::io_mapping = true;
bool CommonIO::header_flag = true;

int CommonIO::nref = 0;
bool CommonIO::tagging = false;
int CommonIO::io_buffering = 0;
int CommonIO::seqNum = 1;
int CommonIO::numDigits = 0;
int CommonIO::flush_flag = 1;
int CommonIO::stream_flag = -1;

//
// The strstreams used for mapping IO.
//
#ifndef UTILIB_HAVE_SSTREAM
strstream*	CommonIO::pStrCout = 0;
strstream*  	CommonIO::pStrCerr = 0;
#else
stringstream*	CommonIO::pStrCout = 0;
stringstream*  	CommonIO::pStrCerr = 0;
#endif
static string iobuf;
//
// The iostreams used for user-defined IO
//
ostream*	CommonIO::common_cout = 0;
ostream*	CommonIO::common_cerr = 0;
istream*	CommonIO::common_cin = 0;

//
// The Map# streams are what is actually passed back to the user.
// These can point to either the pStrCout streams or the common_# streams.
//
ostream* CommonIO::MapCout = 0;
ostream* CommonIO::MapCerr = 0;
istream* CommonIO::MapCin  = 0;

ofstream* CommonIO::common_ofstr=0;

//
// By default, IO_Rank is setup to the current rank, which means that every
// processor can do IO.
//
CommonIO::CommonIO()
{
nref++;
debug = 0;
IO_Rank = -1;

#ifdef UTILIB_HAVE_MPI
int running;
MPI_Initialized(&running);
if (running && (mpi_initialized==false)) {
   MPI_Comm_rank(MPI_COMM_WORLD,&Rank);
   MPI_Comm_size(MPI_COMM_WORLD,&Size);
   mpi_initialized=true;
   }
#else
Rank = 0;
#endif
}


CommonIO::~CommonIO()
{
  nref--;
  flush_flag=1; 
  if ((nref == 0) && (begin_end_counter > 0))
    cout << '[' << rank() 
	 << "] ***** Warning: last CommonIO object destructed "
	 << "while expecting " << begin_end_counter
	 << " more calls to CommonIO:end()" << endl;
}


void CommonIO::reset_map()
{
if ((begin_end_counter > 0) && (io_mapping || (io_buffering > 0))) {
   CommonIO::MapCout = pStrCout;
   CommonIO::MapCerr = pStrCerr;
   }
else {
   CommonIO::MapCout = CommonIO::common_cout;
   CommonIO::MapCerr = CommonIO::common_cerr;
   }
}



void CommonIO::begin()
{
//
// If io_mapping is false, then CommonIO has been disabled.
//
if (!io_mapping) return;

if (begin_end_counter==0) {
   #ifndef UTILIB_HAVE_SSTREAM
   CommonIO::pStrCout = new strstream;
   CommonIO::pStrCerr = new strstream;
   #else
   CommonIO::pStrCout = new stringstream;
   CommonIO::pStrCerr = new stringstream;
   #endif
   }
begin_end_counter++;
//if (io_buffering > 0) io_buffering++;
io_buffering++;

#ifdef UTILIB_HAVE_MPI
int running;
MPI_Initialized(&running);
if (running && (mpi_initialized==false)) {
   MPI_Comm_rank(MPI_COMM_WORLD,&Rank);
   MPI_Comm_size(MPI_COMM_WORLD,&Size);
   mpi_initialized=true;
   }
#endif
reset_map();
}


void CommonIO::begin_buffered()
{
//
// If io_mapping is false, then CommonIO has been disabled.
//
if (!io_mapping) return;
CommonIO::begin();
if (io_buffering == 0)		// begin() will increment if io_buffered>0
   io_buffering++;
}


void CommonIO::end_all()
{ flush(); reset_map(); begin_end_counter=0; }


void CommonIO::end()
{
//
// If io_mapping is false, then CommonIO has been disabled.
//
if (!io_mapping) return;
if (io_buffering > 0) {
   flush();
   io_buffering--;
   }
if (atexit_flag && (!(CommonIO::common_cout || CommonIO::common_cerr)))
   flush();
if (begin_end_counter > 0) begin_end_counter--;
if (begin_end_counter==0) {
   flush();
   delete CommonIO::pStrCout;
   delete CommonIO::pStrCerr;
   if (common_ofstr)
      delete common_ofstr;
   }
reset_map();
}


void CommonIO::reset()
{
IOflush();
common_cout = 0;
common_cin = 0;
common_cerr = 0;
}


//
// If we're running under MPI, or if the number of numDigits has been
// setup, then return a stringstream, and add the rank/count formatting
// later.  Otherwise, simply return the global streams;  StrCout is
// only needed to provide formating of the output.
//
//Note: disabled the 'pass-through' I/O feature, since you can have situations
//where use get the ostream and then later setup tagging.  We could put 
//constraints on the "set tagging" operationg, but this seems artificial.
//
//
ostream& CommonIO::o_stream(const int flag)
{
if (flag == 0)
   return *MapCout;
else
   return *MapCerr;
}


void CommonIO::IOflush()
{
//
// If io_mapping is false, then CommonIO has been disabled.
//
if (!io_mapping) return;

//
// Perform flushing if:
//   begin_end_counter > 0  
// or
//   io_buffering > 0
//
if (!( (io_buffering > 0) || (begin_end_counter > 0))) return;

if (pStrCout) {
   if (!(pStrCout->good()))
      pStrCout->clear();
#ifndef UTILIB_HAVE_SSTREAM
   if (pStrCout->pcount() > 0)
#else
   if (pStrCout->str().size() > 0)
#endif
      flush_stream(*pStrCout,0);
   }
if (pStrCerr) {
   if (!(pStrCerr->good()))
      pStrCerr->clear();
#ifndef UTILIB_HAVE_SSTREAM
   if (pStrCerr->pcount() > 0)
#else
   if (pStrCerr->str().size() > 0)
#endif
      flush_stream(*pStrCerr,1);
   }
}



#ifndef UTILIB_HAVE_SSTREAM
void CommonIO::flush_stream(strstream& stream, int val)
#else
void CommonIO::flush_stream(stringstream& stream, int val)
#endif
{
#if 0
if (CommonIO::common_cout)
   cerr << "CommonIO::common_cout " << CommonIO::common_cout << " " << CommonIO::common_cout->good() << endl;
cerr << "BUG " << STREAM(val) << " " << STREAM(val)->good() << " " << (*STREAM(val)).good() << endl;
#endif

if (!(STREAM(val)->good())) {
   STREAM(val)->clear();
   if (!(STREAM(val)->good())) {
      reset_map();
      EXCEPTION_MNGR(runtime_error,"CommonIO::flush_stream - problem with output stream: val=" << val << " common_cout=" << common_cout << " common_cerr=" << common_cerr);
      }
   }

//stream << '\000';
if (tagging) {
   char digbuf[32];
#ifdef _MSC_VER
   sprintf_s(digbuf,32,"[%%d]-%%%d.%dd ",numDigits,numDigits);
#else
   sprintf(digbuf,"[%%d]-%%%d.%dd ",numDigits,numDigits);
#endif
   stream.rdbuf()->pubseekpos(0,IOS_IN);
   stream.rdbuf()->pubseekpos(0,IOS_OUT);
   while (stream.good()) {
     char tmpIobuf[1024];
     if (numDigits == 0) {
#ifdef _MSC_VER
        sprintf_s(tmpIobuf,1024,"[%d] ",rank());
#else
        sprintf(tmpIobuf,"[%d] ",rank());
#endif
     } else {
#ifdef _MSC_VER
        sprintf_s(tmpIobuf,1024,digbuf,rank(),seqNum++);
#else
        sprintf(tmpIobuf,digbuf,rank(),seqNum++);
#endif
     }
     iobuf = tmpIobuf;
     int numchars=0;
     char c;
     stream >> c;
     while (stream.good()) {
       if (c == '\n') break;
       iobuf += c;	/* .push_back(c);   COUGAR doesn't like */
       stream.get(c);
       numchars++;
       }
     //int tmp = stream.rdstate();
     if ((!stream.good()) && (numchars==0)) {
        if (numDigits != 0) seqNum--;
        break;
        }
     if (c == '\n') {
        iobuf += '\n';    /* .push_back('\n');  COUGAR doesn't like  */
     }
     //iobuf += '\000';     /* .push_back('\000'); COUGAR doesn't like */
     
     *STREAM(val) << iobuf;
     if (flush_flag)
        STREAM(val)->flush();
     }
   }
else {
   //
   // Send all IO in stream to the stream
   //
#ifndef UTILIB_HAVE_SSTREAM
   *STREAM(val) << stream.rdbuf();
#else
   {
   char tmp[1024];
   tmp[1023]='\000';
   stream.getline(tmp,1023);
   while (stream.good() || stream.eof()) {
     if (stream.eof() && (tmp[0] == '\000')) break;
     *STREAM(val) << tmp << '\n';
     if (flush_flag)
        STREAM(val)->flush();
     if (!stream.good())
        break;
     stream.getline(tmp,1023);
     }
   }
#endif
   }
//
// Reset stream to prepare it for use again.
// If the position values are not reset, the buffer keeps growing...
//
#ifdef UTILIB_HAVE_SSTREAM
string dummy("");
stream.rdbuf()->pubseekpos(0,IOS_IN);
stream.rdbuf()->pubseekpos(0,IOS_OUT);
stream.str(dummy);
stream.rdbuf()->pubseekpos(0,IOS_IN);
stream.rdbuf()->pubseekpos(0,IOS_OUT);
stream.clear();
#else
stream.rdbuf()->pubseekpos(0,IOS_IN);
stream.rdbuf()->pubseekpos(0,IOS_OUT);
stream.clear();
#endif
//
// Flush 'tmp'
//
if (flush_flag)
   STREAM(val)->flush();
}


void CommonIO::set_ofile(const char* str)
{
string fname;
fname += str;
fname += ".";
fname += rank();

common_ofstr = new ofstream(fname.data());
set_cout(common_ofstr);
set_cerr(common_ofstr);
}

} // namespace utilib

