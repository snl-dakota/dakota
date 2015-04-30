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
 * \file PackBuf.cpp
 *
 * \author William E. Hart
 */
 
#include <utilib/std_headers.h>

#define USING_STL
#include <utilib/PackBuf.h>
#include <utilib/CharString.h>
#include <utilib/comments.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif


namespace utilib {


PackBuffer::PackBuffer(UnPackBuffer& copyBuf)
{
  Size   = copyBuf.message_length();
  buffer = new char[Size];

  memcpy(buffer,copyBuf.buf(),Size);

  Index = Size;
}


void PackBuffer::resize(const size_type newsize)
{
if ((Index+newsize) >= Size) {
   Size = std::max(2*Size,Index+newsize);
   char* tmp = new char [Size];
   memcpy(tmp,buffer,Index);
   if (buffer)
      delete [] buffer;
   buffer = tmp;
   }
}


void PackBuffer::pack(char* data)
{
size_type len = strlen(data);
pack(len);
for (size_type i=0; i<len; i++)
  pack(data[i]);
}


void PackBuffer::pack(const char* data)
{
size_type len = strlen(data);
pack(len);
for (size_type i=0; i<len; i++)
  pack(data[i]);
}


void PackBuffer::writeBinary(ostream& ostr)
{
  ostr.write((char *) &Index,sizeof(size_type));
  ostr.write(buffer,Index);
  reset();
}


void UnPackBuffer::resize(const size_type newsize)
{
  if (newsize != Size) {
    if (buffer && own_flag)
      delete [] buffer;
    buffer = new char [newsize];
    Index  = 0;
    Size   = newsize;
    MessageLength = newsize;
    if (buffer)
      status_flag = true;
    own_flag = true;
  }
}



void UnPackBuffer::setup(char* buf_, size_type size_, bool flag)
{
  if (buffer && own_flag)
    delete [] buffer;
  Index         = 0;
  Size          = size_;
  MessageLength = size_;
  buffer        = buf_;
  own_flag      = flag;
  status_flag   = true;
}



void UnPackBuffer::unpack(char* data)
{
  int len;
  unpack(len);

  for (int i=0; i<len; i++)
    unpack(data[i]);

  data[len] = '\000';
}


void UnPackBuffer::readBinary(istream& istr)
{
  size_type sizeWanted;
  istr.read((char *) &sizeWanted,sizeof(size_type));
  if (sizeWanted > size())
    resize(sizeWanted);
  istr.read(buffer,sizeWanted);
  reset(sizeWanted);
}


UnPackBuffer& UnPackBuffer::operator=(const PackBuffer& pbuff)
{
if (pbuff.size() == 0)
   reset();
else {
   char* tmp = new char [pbuff.size()];
   memcpy(tmp, pbuff.buf(), pbuff.size()*sizeof(char));
   setup(tmp,pbuff.size(),1);
   }
return *this;
}


void UnPackBuffer::reset(size_type message_size)
{
  Index = 0;
  if (message_size > Size)
     EXCEPTION_MNGR(runtime_error, "UnPackBuffer has length " << Size << ", but message has size " << message_size);
  MessageLength = message_size;
  status_flag   = true;
};

} // namespace utilib

utilib::PackBuffer& operator<< (utilib::PackBuffer& buff, istream& istr)
{
if (!istr) return buff;

char c;
while (1) {
  istr.get(c);
  if (!istr) break;
  buff.pack(c);
  }
return buff;
}


utilib::UnPackBuffer& operator>> (utilib::UnPackBuffer& buff, ostream& str)
{
if (!buff || !str) return buff;

char c;
while (1) {
  buff.unpack(c);
  if (!buff) break;
  str << c;
  }
return buff;
}


