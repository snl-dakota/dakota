/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "MPIPackBuffer.hpp"
#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#endif // DAKOTA_HAVE_MPI

// TODO: needs to use correct MPI_Comm throughout
//       see Rationale on p.137 of MPI: The Complete Reference

namespace Dakota {

//---------------------------------------------------------------------
//
// MPIPackBuffer
//
//---------------------------------------------------------------------

void MPIPackBuffer::resize(const int newsize)
{
  if (Index + newsize >= Size) {
    Size *= 2;
    char* tmp = new char [Size];
    std::memcpy(tmp, Buffer, Index);
    if (Buffer)
      delete [] Buffer;
    Buffer = tmp;
  }
}


#ifdef DAKOTA_HAVE_MPI
#define PACKBUF(type, mpitype) \
void MPIPackBuffer::pack(const type* data, const int num) \
{ \
  resize(MPIPackSize(data[0], num)); \
  MPI_Pack((void*)data, num, mpitype, Buffer, Size, &Index, MPI_COMM_WORLD); \
}
#else
#define PACKBUF(type, mpitype) \
void MPIPackBuffer::pack(const type* /*data*/, const int /*num*/) \
{ }
#endif // DAKOTA_HAVE_MPI


PACKBUF(int,MPI_INT)
PACKBUF(u_int,MPI_UNSIGNED)
PACKBUF(long,MPI_LONG)
PACKBUF(u_long,MPI_UNSIGNED_LONG)
PACKBUF(short,MPI_SHORT)
PACKBUF(u_short,MPI_UNSIGNED_SHORT)
PACKBUF(char,MPI_CHAR)
PACKBUF(u_char,MPI_UNSIGNED_CHAR)
PACKBUF(double,MPI_DOUBLE)
PACKBUF(float,MPI_FLOAT)


void MPIPackBuffer::pack(const bool* data, const int num)
{
#ifdef DAKOTA_HAVE_MPI
  resize(num*MPIPackSize(data[0],1));
  for (int i=0; i<num; i++) {
    char c = (data[i]) ? 'T' : 'F';
    MPI_Pack((void*)(&c), 1, MPI_CHAR, Buffer, Size, &Index, MPI_COMM_WORLD);
  }
#endif // DAKOTA_HAVE_MPI
}


//---------------------------------------------------------------------
//
// MPIUnpackBuffer
//
//---------------------------------------------------------------------

void MPIUnpackBuffer::resize(const int newsize)
{
  if (newsize != Size) {
    Size = newsize;
    if (Buffer)
      delete [] Buffer;
    Buffer = new char [Size];
  }
}


void MPIUnpackBuffer::setup(char* buf_, int size_, bool flag_)
{
  if (Buffer && ownFlag)
    delete [] Buffer;
  Index   = 0;
  Size    = size_;
  Buffer  = buf_;
  ownFlag = flag_;
}


#ifdef DAKOTA_HAVE_MPI
#define UNPACKBUF(type, mpitype) \
void MPIUnpackBuffer::unpack(type* data, const int num) \
{ MPI_Unpack(Buffer, Size, &Index, (void*)data, num, mpitype, MPI_COMM_WORLD); }
#else
#define UNPACKBUF(type, mpitype) \
void MPIUnpackBuffer::unpack(type* /*data*/, const int /*num*/) \
{ }
#endif // DAKOTA_HAVE_MPI
 
 
UNPACKBUF(int,MPI_INT)
UNPACKBUF(u_int,MPI_UNSIGNED)
UNPACKBUF(long,MPI_LONG)
UNPACKBUF(u_long,MPI_UNSIGNED_LONG)
UNPACKBUF(short,MPI_SHORT)
UNPACKBUF(u_short,MPI_UNSIGNED_SHORT)
UNPACKBUF(char,MPI_CHAR)
UNPACKBUF(u_char,MPI_UNSIGNED_CHAR)
UNPACKBUF(double,MPI_DOUBLE)
UNPACKBUF(float,MPI_FLOAT)


void MPIUnpackBuffer::unpack(bool* data, const int num)
{
#ifdef DAKOTA_HAVE_MPI
  for (int i=0; i<num; i++) {
    char c;
    MPI_Unpack(Buffer, Size, &Index, (void*)(&c), 1, MPI_CHAR, MPI_COMM_WORLD);
    data[i] = (c == 'T') ? true : false;
  }
#endif // DAKOTA_HAVE_MPI
}


//---------------------------------------------------------------------
//
// MPIPackSize
//
//---------------------------------------------------------------------

#ifdef DAKOTA_HAVE_MPI
#define PACKSIZE(type, mpitype)	\
int MPIPackSize(const type& /*data*/, const int num) \
{ \
  int size; \
  MPI_Pack_size(num, mpitype, MPI_COMM_WORLD, &size); \
  return size; \
}
#else
#define PACKSIZE(type, mpitype)	\
int MPIPackSize(const type& /*data*/, const int /*num*/) \
{ return 0; }
#endif // DAKOTA_HAVE_MPI


PACKSIZE(int,MPI_INT)
PACKSIZE(u_int,MPI_UNSIGNED)
PACKSIZE(long,MPI_LONG)
PACKSIZE(u_long,MPI_UNSIGNED_LONG)
PACKSIZE(short,MPI_SHORT)
PACKSIZE(u_short,MPI_UNSIGNED_SHORT)
PACKSIZE(char,MPI_CHAR)
PACKSIZE(u_char,MPI_UNSIGNED_CHAR)
PACKSIZE(double,MPI_DOUBLE)
PACKSIZE(float,MPI_FLOAT)


int MPIPackSize(const bool& /*data*/, const int num)
{
#ifdef DAKOTA_HAVE_MPI
  int size; 
  MPI_Pack_size(num, MPI_CHAR, MPI_COMM_WORLD, &size);
  return size;
#else
  return 0;
#endif // DAKOTA_HAVE_MPI
}

} // namespace Dakota
