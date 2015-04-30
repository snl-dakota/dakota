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

#include <utilib/std_headers.h>
#include <utilib/mpiUtil.h>
#include <utilib/BitArray.h>
#include <utilib/CharString.h>
#include <utilib/OptionParser.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
using namespace utilib;
#endif


int printlevel=0;
bool mpi=false;


class funkyPackBuffer : public PackBuffer
{
public:
  char* data() { return buffer; };
};



void check(const char* descrip, 
	   BitArray&   array,
	   const char* pattern, 
	   unsigned int expected_count,
	   int         trailing_zeros=0)
{
  if (printlevel > 0)
    cout << descrip << ": ";
  const char* pointer = pattern;
  size_type i=0;
  for(; i<array.size()-trailing_zeros; i++)
    {
      //int tmp = array[i].value();
      if (array[i] != ((*pointer) - '0'))
	{
	  if (printlevel == 0)
	    cout << descrip << ": ";
	  cout << "Mismatch in bit " << i << ", expected " 
	       << *pointer << endl;
	  cout << array << endl;
	  exit(1);
	}
      if (*(++pointer) == 0)
	pointer = pattern;
    }
  for(; i<array.size(); i++)
    if (array[i] != 0)
      {
	if (printlevel == 0)
	  cout << descrip << ": ";
	cout << "Trailing zero mismatch in bit " << i << endl;
	cout << array << endl;
	exit(1);
      }
  if (array.nbits() != expected_count)
    {
      if (printlevel == 0)
	cout << descrip << ": ";
      cout << "Count mismatch -- got " 
	   << array.nbits() << ", expected " 
	   << expected_count << endl;
      cout << array << endl;
      exit(1);
    }
  if (printlevel > 0)
    cout << "OK" << endl;
  if (printlevel > 1)
    cout << array << endl;
}


void claim(const char* descrip, 
	   int         assertion,
	   BitArray*   pa1 = 0,
	   BitArray*   pa2 = 0)
{
  if (assertion)
    {
      if (printlevel)
	cout << descrip << ": OK\n";
      return;
    }
  cout << descrip << ": failure\n";
  if (pa1)
    cout << *pa1 << endl;
  if (pa2)
    cout << *pa2 << endl;
  exit(1);
}


void test(int length)
{
  if (printlevel)
    cout << endl << "***** Testing length=" << length << " *****" << endl;
  else
    cout << "Length=" << length << ": ";

  BitArray a(length);
  int i;

  if (printlevel)
    cout << a.bytes_used() << " bytes, " << a.allocation() << " 'words'\n";

  check("Standard initialization",a,"0",0);

  if (a.element_size() != 1)
    {
      cout << "Element size yields " << a.element_size() << endl;
      exit(1);
    }
  if (printlevel)
    cout << "element_size(): OK" << endl;

  for (i=0; i<length; i += 3)
    a.set(i);
  check("set(i)",a,"100",(length+2)/3);

  a.set();
  check("set()",a,"1",length);

  for (i=0; i<length; i += 4)
    a.reset(i);
  check("reset(i)",a,"0111",length - (length+3)/4);

  a.reset();
  check("reset()",a,"0",0);

  for(i=0; i<length; i += 2)
    a.put(i,1);
  check("put(i,1)",a,"10",(length+1)/2);

  for(i=0; i<length; i += 4)
    a.put(i,0);
  check("put(i,0)",a,"0010",(length+1)/4);
  
  for(i=0; i<length; i++)
    if (a(i) != a[i])
      {
	cout << "Operator() and operator[] don't match!\n\n";
	exit(1);
      }
  if (printlevel)
    cout << "operator(): OK" << endl;

  a.flip();
  check("flip()",a,"1101",length - (length+1)/4);

  for(i=0; i<length; i += 4)
    a.flip(i);
  check("flip(i)",a,"01",length/2);

  BitArray b;
  claim("Empty constructor",b.size() == 0,&b);

  BitArray c(a);
  a.reset();
  check("Copy constructor",c,"01",length/2);

  a << 1;
  check("Set = 1",a,"1",length);

  a << 0;
  check("Set = 0",a,"0",0);

  for (i=0; i<length; i += 3)
    a.set(i);

  b = a;
  a << 1;
  check("operator=(i) ",b,"100",(length+2)/3);
  check("operator=(ii)",a,"1",length);

  a << b;
  check("operator<<",a,"100",(length+2)/3);

  for(i=0; i<length; i++)
    a.put(i,1 - (i & 1));
  check("put(i,val)",a,"10",(length+1)/2);

  bitwise_or(a,b,c);
  check("bitwise_or",c,"101110",length - (length/6) - ((length+4)/6));

  bitwise_xor(a,b,c);
  check("bitwise_xor",c,"001110",(length+3)/6 + (length+2)/6 + (length+1)/6);

  bitwise_and(a,b,c);
  check("bitwise_and",c,"100000",(length+5)/6);

  c &= b;
  check("operator&=(i)  ",c,"100",(length+2)/3);
  b.flip();
  check("operator&=(ii) ",c,"011",length - (length+2)/3);
  b = a;
  check("operator=(iii) ",b,"10",(length+1)/2);
  check("operator&=(iii)",c,"011",length - (length+2)/3);

  a = c;
  a.flip();
  claim("shared_one (i)  ",!a.shared_one(c),&a,&c);
  if (c.nbits() > 0)
    {
      int j;
      for (j=length - 1; c[j]==0; j--);
      a.flip(j);
      claim("shared_one (ii) ",a.shared_one(c),&a,&c);
      a.flip(j);
      for(j=0; c[j]==0; j++);
      a.flip(j);
      claim("shared_one (iii)",a.shared_one(c),&a,&c);
    }

  CharString buffer;
  buffer += length;
  buffer += " : ";
  int j = 0;
  for(i=0; i<length; i++)
    {
      if (i % 2 == 0)
         buffer += "0";
      else
         buffer += "1";
    }
  std::istringstream ins(buffer.data());
  a.read(ins);
  check("read from stream",a,"01",length/2);

  a << 1;
  check("Resize setup (a=1) ",a,"1",length);
  a.resize(length+1);
  check("Resize by +1(i)    ",a,"1",length,1);
  a.set(length);
  check("Resize by +1(ii)   ",a,"1",length+1);
  a.resize(length+9);
  check("Resize by +8(i)    ",a,"1",length+1,8);
  a.set();
  check("Resize by +8(ii)   ",a,"1",length+9);
  a.resize(length+40);
  check("Resize by +31(i)   ",a,"1",length+9,31);
  a.set();
  check("Resize by +31(ii)  ",a,"1",length+40);
  a.resize(length);
  check("Resize by -31      ",a,"1",length);
  a.resize(length/2);
  check("Resize by -50%     ",a,"1",length/2);
  a.resize(length);
  check("Resize by +100%(i) ",a,"1",length/2,length - (length/2));
  a.set();
  check("Resize by +100%(ii)",a,"1",length);
  a.resize(length+32);
  check("Resize by +32(i)   ",a,"1",length,32);
  a.set();
  check("Resize by +32(ii)  ",a,"1",length+32);
  a.resize(length);
  check("Resize by -32      ",a,"1",length);

  for(i=0; i<length; i++)
    a.put(i,i & 1);
  b << a;
  claim("compare ==",a.compare(b)==0,&a,&b);
  claim("operator==",a == b,&a,&b);
  claim("operator>=",a >= b,&a,&b);
  claim("operator<=",a <= b,&a,&b);
  claim("operator< ",!(a < b),&a,&b);
  claim("operator> ",!(a > b),&a,&b);

  for(j=length-1; (a[j]==1) && (j>=0); j--);
  if (j >= 0)
    {
      a.set(j);
      claim("compare > ",a.compare(b)>0,&a,&b);
      claim("compare < ",b.compare(a)<0,&b,&a);
      claim("operator==",!(a == b),&a,&b);
      claim("operator>=",a >= b,&a,&b);
      claim("operator<=",!(a <= b),&a,&b);
      claim("operator< ",!(a < b),&a,&b);
      claim("operator> ",a > b,&a,&b);
      a.reset(j);
    }
  for(j=0; (a[j]==1) && (j<length); j++);
  if (j < length)
    {
      b.set(j);
      claim("compare < ",a.compare(b)<0,&b,&a);
      claim("compare > ",b.compare(a)>0,&a,&b);
      claim("operator==",!(a == b),&a,&b);
      claim("operator>=",!(a >= b),&a,&b);
      claim("operator<=",a <= b,&a,&b);
      claim("operator< ",a < b,&a,&b);
      claim("operator> ",!(a > b),&a,&b);
      b.reset(j);
    }

  b.resize(length + 6);
  claim("compare < ",a.compare(b)<0,&b,&a);
  claim("compare > ",b.compare(a)>0,&a,&b);
  claim("operator==",!(a == b),&a,&b);
  claim("operator>=",!(a >= b),&a,&b);
  claim("operator<=",a <= b,&a,&b);
  claim("operator< ",a < b,&a,&b);
  claim("operator> ",!(a > b),&a,&b);

  b.resize(length - 1);
  claim("compare > ",a.compare(b)>0,&a,&b);
  claim("compare < ",b.compare(a)<0,&b,&a);
  claim("operator==",!(a == b),&a,&b);
  claim("operator>=",a >= b,&a,&b);
  claim("operator<=",!(a <= b),&a,&b);
  claim("operator< ",!(a < b),&a,&b);
  claim("operator> ",a > b,&a,&b);

  b.resize(0);
  claim("compare > ",a.compare(b)>0,&a,&b);
  claim("compare < ",b.compare(a)<0,&b,&a);
  claim("operator==",!(a == b),&a,&b);
  claim("operator>=",a >= b,&a,&b);
  claim("operator<=",!(a <= b),&a,&b);
  claim("operator< ",!(a < b),&a,&b);
  claim("operator> ",a > b,&a,&b);

  a.resize(0);
  claim("compare ==",a.compare(b)==0,&a,&b);
  claim("operator==",a == b,&a,&b);
  claim("operator>=",a >= b,&a,&b);
  claim("operator<=",a <= b,&a,&b);
  claim("operator< ",!(a < b),&a,&b);
  claim("operator> ",!(a > b),&a,&b);

  a.resize(length);
  b.resize(length);
  for(i=0; i<length; i++)
    a.put(i,i & 1);
  b << a;

#ifdef UTILIB_HAVE_MPI
  if (uMPI::running())
    {
      funkyPackBuffer pbuf;
      pbuf << a;
      UnPackBuffer upbuf(pbuf.data(),pbuf.size());
      c.reset();
      upbuf >> c;
      check("Pack/unpack",a,"01",length/2);
    }
#endif

  cout << "OK\n";
  
}


int test_bitarray(int argc, char** argv)
{
#if 0
  BitArray tmp(3);
  tmp[0] = false;
  tmp[1] = true;
  tmp[2] = true;
  cout << tmp << endl;
#endif

  OptionParser params;
  params.add("printlevel", printlevel, "TODO");
  params.validate("printlevel", printlevel, ParameterBounds<int>(0,2));
  params.add("mpi",mpi,"TODO");

  params.parse_args(argc,argv);
     if (params.help_option())
   {
      params.write(std::cout);
      return 1;
   }

#ifdef UTILIB_HAVE_MPI
  if (mpi)
    uMPI::init(&argc,&argv);
#endif
  
  int argcount = 0;
  for(int arg = 1; arg<argc; arg++)
    if(*(argv[arg]) != '-')
      {
	int testsize = 0;
#ifdef _MSC_VER
	if (!sscanf_s(argv[arg],"%d",&testsize))
#else
	if (!sscanf(argv[arg],"%d",&testsize))
#endif
	  {
	    cout << "Could not parse argument '" << argv[arg] << "'\n";
	    exit(1);
	  }
	if (testsize < 1)
	  {
	    cout << testsize << " is too small a size.\n";
	    exit(1);
	  }
	argcount++;
	test(testsize);
      }
  if (argcount == 0)
    {
      cout << "\n***** Testing default size sequence *****\n";
      int lastsize = 0;
      for(int i=1; i<=(1 << 7); i = 2*i)
	for(int j=-32; j<=32; j++)
	    if (i + j > lastsize)
	      {
		lastsize = i + j;
		test(lastsize);
	      }
    }
  
  cout << endl << "Tests completed successfully.\n\n";

#ifdef UTILIB_HAVE_MPI
  if (mpi)
    uMPI::done();
#endif

  return 0;
}
