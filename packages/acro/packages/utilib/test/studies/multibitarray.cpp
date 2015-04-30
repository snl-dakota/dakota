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
#include <utilib/TwoBitArray.h>
#include <utilib/EnumBitArray.h>
#include <utilib/CharString.h>
#include <utilib/OptionParser.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
#endif

using namespace utilib;

// Define some weird types to check how enum-arrays work.

namespace utilib {

enum wxyz { wval=0, xval=1, yval=2, zval=3 };
typedef EnumBitArray<1,wxyz> wxyzArray;
template<>
int  wxyzArray::enum_count    = 0;
template<>
const char* wxyzArray::enum_labels = "";
template<>
wxyz *wxyzArray::enum_vals   = NULL;

enum thirteen { zero, one, two, three, four, five, six,
		seven, eight, nine, ten, eleven, twelve };
typedef EnumBitArray<2,thirteen> fourBitter;
template<>
int      fourBitter::enum_count    = 0;
template<>
const char*     fourBitter::enum_labels = "";
template<>
thirteen *fourBitter::enum_vals   = NULL;



int printlevel = 0;
bool mpi=false;
bool b2=true;
bool e2=true;
bool e4=true;


class funkyPackBuffer : public PackBuffer
{
public:
  char* data() { return buffer; };
};



template<class Aclass>
void check(const char* descrip, 
	   Aclass&     array,
	   const char* pattern, 
	   int         trailing_zeros=0)
{
  if (printlevel > 0)
    cout << descrip << ": ";
  const char* pointer = pattern;
  size_type i=0;
  for(; i<array.size()-trailing_zeros; i++)
    {
      if (array[i] != array.translate_from_char(*pointer))
	{
	  if (printlevel == 0)
	    cout << descrip << ": ";
	  cout << "Mismatch in element " << i 
	       << ": got " << array[i] << ", expected " 
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
	cout << "Trailing zero mismatch in element " << i << endl;
	cout << array << endl;
	exit(1);
      }
  if (printlevel > 0)
    cout << "OK" << endl;
  if (printlevel > 1)
    cout << array << endl;
}



template<class Aclass>
void claim(const char* descrip, 
	   int         assertion,
	   Aclass*     pa1 = 0,
	   Aclass*     pa2 = 0)
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



template<class Aclass, class T>
void test_class(const char* name,
		unsigned int length,
		int element_bits,
		unsigned int numValues)
{
  Aclass a(length);
  unsigned int i;

  if (printlevel)
    cout << "***** ";
  else 
    cout << '(';
  cout << name;
  if (printlevel)
    cout << " *****\n" << a.bytes_used() << " bytes, " 
	 << a.allocation() << " 'words'\n";
  else
    cout << ") ";

  char onechar[2];
  onechar[0] = a.translate_to_char(0);
  onechar[1] = '\0';

  check("Standard initialization",a,onechar,0);
  claim("Element size",a.element_size()==element_bits,&a);

  CharString sequence(numValues);
  for (i=0; i<numValues; i++)
    sequence[i] = a.translate_to_char(i % numValues);

  for (i=0; i<length; i++)
    a.put(i,(T) (i % numValues));
  check("put(i,*)",a,sequence.data());

  for(i=0; i<length; i++)
    if (a(i) != a[i])
      {
	cout << "Operator() and operator[] don't match!\n\n";
	exit(1);
      }
  if (printlevel)
    cout << "operator(): OK" << endl;

  Aclass b;
  claim("Empty constructor",b.size() == 0,&b);

  Aclass c(a);
  a << (T) 0;
  check("Copy constructor",c,sequence.data());
  check("Set = 0",a,onechar);
  for (i=1; i<numValues; i++)
    {
      a << (T) i;
      onechar[0] = a.translate_to_char(i);
      check("Set to constant",a,onechar);
    }

  a << c;
  check("operator=",a,sequence.data());

  CharString sequence123(3);
  for (i=1; i<=3; i++)
    sequence123[i-1] = a.translate_to_char(i);

  for(i=0; i<length; i++)
    c.put(i,(T) ((i % 3) + 1));
  check("put(i,*) (ii)",c,sequence123.data());

  a << c;
  check("operator<<",a,sequence123.data());

  b = c;
  check("operator=(ii)",b,sequence123.data());

  b &= a;
  check("operator&=(i)",b,sequence123.data());
  a << (T) 3;
  onechar[0] = a.translate_to_char(3);
  check("operator&=(ii)",b,onechar);

  CharString buffer;
  buffer += length;
  buffer += " : ";
  size_type j = 0;
  for(i=0; i<length; i++)
    {
      buffer += a.translate_to_char(i % numValues);
    }
  istringstream ins(buffer.data());
  a.read(ins);
  check("read from stream",a,sequence.data());

  onechar[0] = a.translate_to_char(1);
  a << (T) 1;
  check("Resize setup (a=1) ",a,onechar);
  a.resize(length+1);
  check("Resize by +1(i)    ",a,onechar,1);
  a.put(length,(T) 1);
  check("Resize by +1(ii)   ",a,onechar);
  a.resize(length+5);
  check("Resize by +4(i)    ",a,onechar,4);
  a << (T) 1;
  check("Resize by +4(ii)   ",a,onechar);
  a.resize(length/2);
  check("Resize by -50%(i)  ",a,onechar);
  claim("Resize by -50%(ii) ",a.size()==length/2,&a);
  a.resize(length);
  check("Resize by +100%(i) ",a,onechar,length - (length/2));
  a << (T) 1;
  check("Resize by +100%(ii)",a,onechar);
  a.resize(length+32);
  check("Resize by +32(i)   ",a,onechar,32);
  a << (T) 1;
  check("Resize by +32(ii)  ",a,onechar);
  a.resize(length);
  claim("Resize by -32      ",a.size()==length,&a);

  for(i=0; i<length; i++)
    a.put(i,(T) (i % numValues));
  b = c;
  b << a;

  claim("compare ==",a.compare(b)==0,&a,&b);
  claim("operator==",a == b,&a,&b);
  claim("operator>=",a >= b,&a,&b);
  claim("operator<=",a <= b,&a,&b);
  claim("operator< ",!(a < b),&a,&b);
  claim("operator> ",!(a > b),&a,&b);

  for(j=length; j>=1; )
  {
     --j;
     if (((int)a[j]) != (int)(numValues-1)) break;
  };

      a.put(j,(T) (a[j] + 1));
      claim("compare > ",a.compare(b)>0,&a,&b);
      claim("compare < ",b.compare(a)<0,&b,&a);
      claim("operator==",!(a == b),&a,&b);
      claim("operator>=",a >= b,&a,&b);
      claim("operator<=",!(a <= b),&a,&b);
      claim("operator< ",!(a < b),&a,&b);
      claim("operator> ",a > b,&a,&b);
      a.put(j,(T) (j % numValues));

  for(j=0; j<length; j++)
    if (((int)b[j]) != (int)(numValues-1)) break;

  if (j < length)
    {
      b.put(j,(T) (b[j]+1));
      claim("compare < ",a.compare(b)<0,&b,&a);
      claim("compare > ",b.compare(a)>0,&a,&b);
      claim("operator==",!(a == b),&a,&b);
      claim("operator>=",!(a >= b),&a,&b);
      claim("operator<=",a <= b,&a,&b);
      claim("operator< ",a < b,&a,&b);
      claim("operator> ",!(a > b),&a,&b);
      b.put(j,(T) (j % numValues));
    }

#ifdef UTILIB_HAVE_MPI
  if (uMPI::running())
    {
      funkyPackBuffer pbuf;
      pbuf << a;
      UnPackBuffer upbuf(pbuf.data(),pbuf.size());
      c << (T) 0;
      upbuf >> c;
      check("Pack/unpack",a,sequence.data());
    }
#endif

  if (printlevel)
    cout << endl;
  
}



void test(int length)
{
  if (printlevel)
      cout << endl << "***** Testing length=";
  cout << length;
  if (printlevel)
    cout << " *****" << endl;
  else
    cout << ": ";

  if (b2)
    test_class<TwoBitArray,int>((const char*)"TwoBitArray", (unsigned int)length, 2, (unsigned int)4);

  if (e2)
    test_class<wxyzArray,wxyz>((const char*)"2-bit enum array",
	       length, 2, 4);

  if (e4)
    test_class<fourBitter,thirteen>((const char*)"4-bit enum array",
	       length, 4, 13);

  cout << "OK\n";
}

}

int test_multibitarray(int argc, char** argv)
{
  OptionParser params;
  params.add("printlevel",printlevel);
  params.validate("printlevel",printlevel,ParameterBounds<int>(0,2));
  params.add("mpi",mpi);
  params.add("b2",b2);
  params.add("e2",e2);
  params.add("e4",e4);

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

  // Initialize statis data for the utilib classes  
char *wxyzLabelArray = new char[4];
  wxyzLabelArray[0] = 'w';
  wxyzLabelArray[1] = 'x';
  wxyzLabelArray[2] = 'y';
  wxyzLabelArray[3] = 'z';
  wxyz *wxyzEnumValues = new wxyz[4];
  wxyzEnumValues[0] = wval;
  wxyzEnumValues[1] = xval;
  wxyzEnumValues[2] = yval;
  wxyzEnumValues[3] = zval;

  wxyzArray wxyzInitializer;

  wxyzInitializer.setEnumInformation(4,wxyzLabelArray, wxyzEnumValues);

char *FBLabelArray = new char[13];
  FBLabelArray[0] = '0';
  FBLabelArray[1] = '1';
  FBLabelArray[2] = '2';
  FBLabelArray[3] = '3';
  FBLabelArray[4] = '4';
  FBLabelArray[5] = '5';
  FBLabelArray[6] = '6';
  FBLabelArray[7] = '7';
  FBLabelArray[8] = '8';
  FBLabelArray[9] = '9';
  FBLabelArray[10] = 't';
  FBLabelArray[11] = 'l';
  FBLabelArray[12] = 'w';
  thirteen *FBEnumValues = new thirteen[13];
  FBEnumValues[0] = zero;
  FBEnumValues[1] = one;
  FBEnumValues[2] = two;
  FBEnumValues[3] = three;
  FBEnumValues[4] = four;
  FBEnumValues[5] = five;
  FBEnumValues[6] = six;
  FBEnumValues[7] = seven;
  FBEnumValues[8] = eight;
  FBEnumValues[9] = nine;
  FBEnumValues[10] = ten;
  FBEnumValues[11] = eleven;
  FBEnumValues[12] = twelve;

  fourBitter FBInitializer;

  FBInitializer.setEnumInformation(13,FBLabelArray, FBEnumValues);

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
      for(int i=1; i<=(1 << 7); i *= 2)
	for(int j=-32; j<=32; j++)
	    if (i + j > lastsize)
	      {
		lastsize = i + j;
		test(lastsize);
	      }
    }

  cout << endl << "Test completed successfully\n\n";

#ifdef UTILIB_HAVE_MPI
  if (mpi)
    uMPI::done();
#endif

  return 0;
}

