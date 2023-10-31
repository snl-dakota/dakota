/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef MPI_PACK_BUFFER_H
#define MPI_PACK_BUFFER_H

#include "dakota_system_defs.hpp"

namespace Dakota {

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef long long long_long;


//---------------------------------------------------------------------
//
// MPIPackBuffer
//
//---------------------------------------------------------------------

/// Class for packing MPI message buffers.

/** A class that provides a facility for packing message buffers using
    the MPI_Pack facility. The \c MPIPackBuffer class dynamically
    resizes the internal buffer to contain enough memory to pack the
    entire object.  When deleted, the \c MPIPackBuffer object deletes
    this internal buffer.  This class is based on the Dakota_Version_3_0
    version of utilib::PackBuffer from utilib/src/io/PackBuf.[cpp,h].
    This snapshot preceded the introduction of templatization on data
    type, which was problematic at that time (would be more reliable now).
*/

class MPIPackBuffer {

public:
 
  /// Constructor, which allows the default buffer size to be set.
  MPIPackBuffer(int size_ = 1024)
    { Index = 0; Size = size_; Buffer = new char [size_]; }
  /// Desctructor.
  ~MPIPackBuffer() { if (Buffer) delete [] Buffer; }
 
  /// Returns a pointer to the internal buffer that has been packed.
  const char* buf() { return Buffer; }
  /// The number of bytes of packed data.
  int size() { return Index; }
  /// the allocated size of Buffer.
  int capacity() { return Size; }
  /// Resets the buffer index in order to reuse the internal buffer.
  void reset() { Index = 0; }

  /// Pack one or more \b int's
  void pack(const int* data, const int num = 1);
  /// Pack one or more \b unsigned \b int's
  void pack(const u_int* data, const int num = 1);
  /// Pack one or more \b long's
  void pack(const long* data, const int num = 1);
  /// Pack one or more \b unsigned \b long's
  void pack(const u_long* data, const int num = 1);
  /// Pack one or more long long's
  void pack(const long long* data, const int num = 1);
  /// Pack one or more unsigned long long's
  void pack(const unsigned long long* data, const int num = 1);
  /// Pack one or more \b short's
  void pack(const short* data, const int num = 1);
  /// Pack one or more \b unsigned \b short's
  void pack(const u_short* data, const int num = 1);
  /// Pack one or more \b char's
  void pack(const char* data, const int num = 1);
  /// Pack one or more \b unsigned \b char's
  void pack(const u_char* data, const int num = 1);
  /// Pack one or more \b double's
  void pack(const double* data, const int num = 1);
  /// Pack one or more \b float's
  void pack(const float* data, const int num = 1);
  /// Pack one or more \b bool's
  void pack(const bool* data, const int num = 1);

  /// Pack a \b int
  void pack(const int& data) 		{ pack(&data); }
  /// Pack a \b unsigned \b int
  void pack(const u_int& data)		{ pack(&data); }
  /// Pack a \b long
  void pack(const long& data) 		{ pack(&data); }
  /// Pack a \b unsigned \b long
  void pack(const u_long& data)		{ pack(&data); }
  /// Pack a long long
  void pack(const long long& data) { pack(&data); }
  /// Pack a unsigned long long
  void pack(const unsigned long long& data) { pack(&data); }
  /// Pack a \b short
  void pack(const short& data) 		{ pack(&data); }
  /// Pack a \b unsigned \b short
  void pack(const u_short& data)	{ pack(&data); }
  /// Pack a \b char
  void pack(const char& data) 		{ pack(&data); }
  /// Pack a \b unsigned \b char
  void pack(const u_char& data)		{ pack(&data); }
  /// Pack a \b double
  void pack(const double& data) 	{ pack(&data); }
  /// Pack a \b float
  void pack(const float& data) 		{ pack(&data); }
  /// Pack a \b bool
  void pack(const bool& data) 		{ pack(&data); }

protected:

  /// Resizes the internal buffer
  void resize(const int newsize);

  /// The internal buffer for packing
  char* Buffer;
  /// The index into the current buffer
  int Index;
  /// The total size that has been allocated for the buffer
  int Size;
};


/// insert an int
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const int& data)
{ buff.pack(data); return buff; }
/// insert a u_int
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const u_int& data)
{ buff.pack(data); return buff; }
/// insert a long
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const long& data)
{ buff.pack(data); return buff; }
/// insert a u_long
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const u_long& data)
{ buff.pack(data); return buff; }
/// insert a long long
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const long long& data)
{ buff.pack(data); return buff; }
/// insert a unsigned long long
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const unsigned long long& data)
{ buff.pack(data); return buff; }
/// insert a short
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const short& data)
{ buff.pack(data); return buff; }
/// insert a u_short
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const u_short& data)
{ buff.pack(data); return buff; }
/// insert a char
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const char& data)
{ buff.pack(data); return buff; }
/// insert a u_char
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const u_char& data)
{ buff.pack(data); return buff; }
/// insert a double
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const double& data)
{ buff.pack(data); return buff; }
/// insert a float
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const float& data)
{ buff.pack(data); return buff; }
/// insert a bool
inline MPIPackBuffer& operator<< (MPIPackBuffer& buff, const bool& data)
{ buff.pack(data); return buff; }


//---------------------------------------------------------------------
//
// MPIUnpackBuffer
//
//---------------------------------------------------------------------

/// Class for unpacking MPI message buffers.

/** A class that provides a facility for unpacking message buffers
    using the MPI_Unpack facility. This class is based on the
    Dakota_Version_3_0 version of utilib::UnPackBuffer from
    utilib/src/io/PackBuf.[cpp,h] */

class MPIUnpackBuffer {
 
public:

  /// Method that does the setup for the constructors
  void setup(char* buf_, int size_, bool flag_ = false);

  /// Default constructor.
  MPIUnpackBuffer() : Buffer(NULL), ownFlag(false)
    { setup(NULL, 0, false); }
  /// Constructor that specifies the size of the buffer
  MPIUnpackBuffer(int size_) : Buffer(NULL), ownFlag(false)
    { setup(new char [size_], size_, true); }
  /// Constructor that sets the internal buffer to the given array
  MPIUnpackBuffer(char* buf_, int size_, bool flag_ = false) :
    Buffer(NULL), ownFlag(false) { setup(buf_, size_, flag_); }
  /// Destructor.
  ~MPIUnpackBuffer() { if (Buffer && ownFlag) delete [] Buffer; }

  /// Resizes the internal buffer
  void resize(const int newsize);
  /// Returns a pointer to the internal buffer
  const char* buf() { return Buffer; }
  /// Returns the length of the buffer.
  int size() { return Size; }
  /// Returns the number of bytes that have been unpacked from the buffer.
  int curr() { return Index; }
  /// Resets the index of the internal buffer.
  void reset() { Index = 0; }

  /// Unpack one or more \b int's
  void unpack(int* data, const int num = 1);
  /// Unpack one or more \b unsigned \b int's
  void unpack(u_int* data, const int num = 1);
  /// Unpack one or more \b long's
  void unpack(long* data, const int num = 1);
  /// Unpack one or more \b unsigned \b long's
  void unpack(u_long* data, const int num = 1);
  /// Unpack one or more long long's
  void unpack(long long* data, const int num = 1);
  /// Unpack one or more unsigned long long's
  void unpack(unsigned long long* data, const int num = 1);
  /// Unpack one or more \b short's
  void unpack(short* data, const int num = 1);
  /// Unpack one or more \b unsigned \b short's
  void unpack(u_short* data, const int num = 1);
  /// Unpack one or more \b char's
  void unpack(char* data, const int num = 1);
  /// Unpack one or more \b unsigned \b char's
  void unpack(u_char* data, const int num = 1);
  /// Unpack one or more \b double's
  void unpack(double* data, const int num = 1);
  /// Unpack one or more \b float's
  void unpack(float* data, const int num = 1);
  /// Unpack one or more \b bool's
  void unpack(bool* data, const int num = 1);

  /// Unpack a \b int
  void unpack(int& data) 		{ unpack(&data); }
  /// Unpack a \b unsigned \b int
  void unpack(u_int& data)		{ unpack(&data); }
  /// Unpack a \b long
  void unpack(long& data) 		{ unpack(&data); }
  /// Unpack a \b unsigned \b long
  void unpack(u_long& data)		{ unpack(&data); }
  /// Unpack a long long
  void unpack(long long& data) { unpack(&data); }
  /// Unpack a unsigned long long
  void unpack(unsigned long long& data) { unpack(&data); }
  /// Unpack a \b short
  void unpack(short& data) 		{ unpack(&data); }
  /// Unpack a \b unsigned \b short
  void unpack(u_short& data)		{ unpack(&data); }
  /// Unpack a \b char
  void unpack(char& data) 		{ unpack(&data); }
  /// Unpack a \b unsigned \b char
  void unpack(u_char& data)		{ unpack(&data); }
  /// Unpack a \b double
  void unpack(double& data) 		{ unpack(&data); }
  /// Unpack a \b float
  void unpack(float& data) 		{ unpack(&data); }
  /// Unpack a \b bool
  void unpack(bool& data) 		{ unpack(&data); }

protected:
 
  /// The internal buffer for unpacking
  char* Buffer;
  /// The index into the current buffer
  int Index;
  /// The total size that has been allocated for the buffer
  int Size;
  /// If \c TRUE, then this class owns the internal buffer
  bool ownFlag;
};


/// extract an int
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, int& data)
{ buff.unpack(data); return buff; }
/// extract a u_int
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, u_int& data)
{ buff.unpack(data); return buff; }
/// extract a long
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, long& data)
{ buff.unpack(data); return buff; }
/// extract a u_long
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, u_long& data)
{ buff.unpack(data); return buff; }
/// extract a long long
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, long long& data)
{ buff.unpack(data); return buff; }
/// extract an unsigned long long
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, unsigned long long& data)
{ buff.unpack(data); return buff; }
/// extract a short
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, short& data)
{ buff.unpack(data); return buff; }
/// extract a u_short
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, u_short& data)
{ buff.unpack(data); return buff; }
/// extract a char
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, char& data)
{ buff.unpack(data); return buff; }
/// extract a u_char
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, u_char& data)
{ buff.unpack(data); return buff; }
/// extract a double
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, double& data)
{ buff.unpack(data); return buff; }
/// extract a float
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, float& data)
{ buff.unpack(data); return buff; }
/// extract a bool
inline MPIUnpackBuffer& operator>> (MPIUnpackBuffer& buff, bool& data)
{ buff.unpack(data); return buff; }


//---------------------------------------------------------------------
//
// MPIPackSize
//
//---------------------------------------------------------------------

/// return packed size of an int
int MPIPackSize(const int& data, const int num = 1);
/// return packed size of a u_int
int MPIPackSize(const u_int& data, const int num = 1);
/// return packed size of a long
int MPIPackSize(const long& data, const int num = 1);
/// return packed size of a u_long
int MPIPackSize(const u_long& data, const int num = 1);
/// return packed size of a long long
int MPIPackSize(const long long& data, const int num = 1);
/// return packed size of an unsigned long long
int MPIPackSize(const unsigned long long& data, const int num = 1);
/// return packed size of a short
int MPIPackSize(const short& data, const int num = 1);
/// return packed size of a u_short
int MPIPackSize(const u_short& data, const int num = 1);
/// return packed size of a char
int MPIPackSize(const char& data, const int num = 1);
/// return packed size of a u_char
int MPIPackSize(const u_char& data, const int num = 1);
/// return packed size of a double
int MPIPackSize(const double& data, const int num = 1);
/// return packed size of a float
int MPIPackSize(const float& data, const int num = 1);
/// return packed size of a bool
int MPIPackSize(const bool& data, const int num = 1);


} // namespace Dakota

#endif // MPI_PACK_BUFFER_H
