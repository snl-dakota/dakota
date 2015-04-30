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
 * \file pvector.h
 *
 * Defines the utilib::pvector class
 */

#ifndef utilib_pvector_h
#define utilib_pvector_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/Serialize.h>
#include <utilib/TypeManager.h>

namespace utilib {

/**
 * A wrapper for the STL vector class with (a) protected memory
 * access and (b) additional methods for numerical computation and I/O.
 *
 * Note that the methods in the STL vector class are NOT virtual.  Thus this
 * class should be used with caution.  Consequently, the overloaded methods
 * in this derived class are _NOT_ used when a pvector is cast back to a
 * vector object.
 */
template <class _Tp>
class pvector : public std::vector<_Tp>
{
private:

  #if !defined(DOXYGEN)
  typedef std::vector<_Tp> base_t;
  #endif

public:
#if defined(UTILIB_SGI_CC)
   //typedef typename std::_Alloc_traits<_Tp, _PAlloc>::allocator_type allocator_type;
   typedef _Tp value_type;
   typedef value_type* pointer;
   typedef const value_type* const_pointer;
   typedef value_type* iterator;
   typedef const value_type* const_iterator;
   typedef value_type& reference;
   typedef const value_type& const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;

#elif defined(UTILIB_AIX_CC) || defined(_MSC_VER)
   //typedef typename _PAlloc::size_type size_type;
   //typedef typename _PAlloc::difference_type difference_type;
   typedef _Tp value_type;
   typedef _Tp  *pointer;
   typedef const _Tp  *const_pointer;
   typedef _Tp & reference;
   typedef const _Tp & const_reference;
   //typedef _PAlloc allocator_type;

#elif defined(UTILIB_OSF_CC) || defined(UTILIB_SOLARIS_CC)
   typedef typename base_t::size_type 		size_type;
   typedef typename base_t::reference 		reference;
   typedef typename base_t::const_reference	const_reference;
   typedef typename base_t::iterator		iterator;
   typedef typename base_t::const_iterator		const_iterator;
   
#else // (__GNUC__ >= 3) || REDSTORM || PGI || ...
   typedef size_t				      size_type;
   typedef typename base_t::allocator_type             allocator_type;
   typedef _Tp                                         value_type;
   typedef value_type*                                 pointer;
   typedef const value_type*                           const_pointer;
   typedef value_type&                                 reference;
   typedef const value_type&                           const_reference;
   typedef typename base_t::iterator                   iterator;
   typedef typename base_t::const_iterator             const_iterator;

#endif

public:

  /// Constructor
  explicit pvector()
	: std::vector<_Tp>() 
  { }

  /// Constructor
  explicit pvector(size_type __n, const _Tp& __val)
	: std::vector<_Tp>(__n, __val)
  { }

  /// Constructor
  explicit pvector(size_type __n)
	: std::vector<_Tp>(__n)
  { }

  /// Copy constructor
  pvector(const pvector<_Tp>& __x)
	: std::vector<_Tp>(__x) { (void)registrations_complete; }

  /// Constructor
#ifdef UTILIB_HAVE_MEMBER_TEMPLATES
  template <class _InputIterator>
  pvector(_InputIterator __first, _InputIterator __last)
	: std::vector<_Tp>(__first, __last) {}
#else
  pvector(const _Tp* __first, const _Tp* __last)
	: std::vector<_Tp>(__first, __last) {}
#endif

  /// Destructor
  ~pvector() {}

  /// Operator equal
  pvector<_Tp>& operator=(const std::vector<_Tp>& __x)
	{
	resize(__x.size());
	iterator curr = this->begin();
	iterator last = this->end();
	const_iterator xcurr = __x.begin();

	while (curr != last) {
  	  (*curr) = (*xcurr);
  	  curr++;
  	  xcurr++;
  	  }

	return *this;
	}

  /// Operator equal
  pvector<_Tp>& operator=(const _Tp& val)
	{
	if (base_t::size() > 0)
	   assign(base_t::size(), val);
	return *this;
	}

  /// Return a referenc value for the \a __n -th element
  reference operator[](size_type __n)
	{
	if ( __n >= base_t::size())
           EXCEPTION_MNGR(runtime_error, "__n=" << __n << " >= size()=" << base_t::size());
	return *(base_t::begin() + __n);
	}

  /// Return a const referenc value for the \a __n -th element
  const_reference operator[](size_type __n) const
	{
	if ( __n >= base_t::size())
           EXCEPTION_MNGR(runtime_error, "__n=" << __n << " >= size()=" << base_t::size());
	return *(base_t::begin() + __n);
	}

  /// Push a value to the back of the vector
  void push_back(const _Tp& val)
	{ std::vector<_Tp>::push_back(val); }

  /// Push a vector to the back of the vector
  void push_back(const std::vector<_Tp>& vec)
	{ insert(base_t::end(), vec.begin(), vec.end()); }

  /// Sets the array equal to the pair-wise value \a x + \a y.
  void plus  (const pvector<_Tp>& x, const pvector<_Tp>& y);

  /// Sets the array equal to the pair-wise value \a x - \a y.
  void minus (const pvector<_Tp>& x, const pvector<_Tp>& y);

  /// Sets the array equal to the pair-wise value \a x * \a y.
  void times (const pvector<_Tp>& x, const pvector<_Tp>& y);

  /// Sets the array equal to the pair-wise value \a x / \a y.
  void divide(const pvector<_Tp>& x, const pvector<_Tp>& y);

  /// Sets the array equal to the value \a x[i] + z.
  void plus  (const pvector<_Tp>& x, const _Tp& z);

  /// Sets the array equal to the value \a x[i] - z.
  void minus (const pvector<_Tp>& x, const _Tp& z);

  /// Sets the array equal to the value \a x[i] * z.
  void times (const pvector<_Tp>& x, const _Tp& z);

  /// Sets the array equal to the value \a x[i] / z.
  void divide(const pvector<_Tp>& x, const _Tp& z);

  /// Changes the sign of elements in the array
  pvector<_Tp> operator-    () const;

  /// Adds the values of \a x elementwise to the current array.
  pvector<_Tp>& operator+= (const pvector<_Tp>& x);

  /// Subtracts the values of \a x elementw to the current array.ise
  pvector<_Tp>& operator-= (const pvector<_Tp>& x);

  /// Multiplies the values of \a x elementwise to the current array.
  pvector<_Tp>& operator*= (const pvector<_Tp>& x);

  /// Divides the values of \a x elementwise to the current array.
  pvector<_Tp>& operator/= (const pvector<_Tp>& x);

  /// Adds \a z to the elements of the current array.
  pvector<_Tp>& operator+=  (const _Tp& z);

  /// Subtracts \a z to the elements of the current array.
  pvector<_Tp>& operator-=  (const _Tp& z);

  /// Multiplies \a z to the elements of the current array.
  pvector<_Tp>& operator*=  (const _Tp& z);

  /// Divides \a z to the elements of the current array.
  pvector<_Tp>& operator/=  (const _Tp& z);

private:
   static const volatile bool registrations_complete;

   template<typename FROM, typename TO>
   static int stream_cast(const Any& from, Any& to)
   {
      to.set<TO>() << from.template expose<FROM>();
      return OK;
   }

   static bool register_aux_functions()
   {
      Serializer().template register_serializer<pvector<_Tp> >
         ( std::string("utilib::pvector;") + mangledName(typeid(_Tp)),
           utilib::STL_Serializers::sequence_serializer<pvector<_Tp> > );
      
      TypeManager()->register_lexical_cast
         ( typeid(pvector<_Tp>), typeid(std::vector<_Tp>), 
           &stream_cast<pvector<_Tp>, std::vector<_Tp> > );
      TypeManager()->register_lexical_cast
         ( typeid(std::vector<_Tp>), typeid(pvector<_Tp>), 
           &stream_cast<std::vector<_Tp>, pvector<_Tp> > );
 
      return true;
    }
};

//============================================================================

template<typename T>
const volatile bool pvector<T>::registrations_complete = 
   pvector<T>::register_aux_functions();

//============================================================================

#if !defined(DOXYGEN)

#define BINARYOP(opname,pseudonym)\
template <class _Tp>\
inline pvector<_Tp> opname (const pvector<_Tp>& a1, const pvector<_Tp>& a2)\
{\
pvector<_Tp> res;\
res.pseudonym(a1,a2);\
return res;\
}\
\
template <class _Tp>\
inline pvector<_Tp> opname (const pvector<_Tp>& a1, const _Tp& val)\
{\
pvector<_Tp> res;\
res.pseudonym(a1,val);\
return res;\
}\
\
template <class _Tp>\
inline pvector<_Tp> opname (const _Tp& val, const pvector<_Tp>& a1)\
{\
pvector<_Tp> res;\
res.pseudonym(a1,val);\
return res;\
}

BINARYOP( operator+ , plus )
BINARYOP( operator- , minus )
BINARYOP( operator* , times )
BINARYOP( operator/ , divide )

#undef BINARYOP


#define BINARYOP(opname,op1,pseudonym, op)\
template <class _Tp>\
void pvector<_Tp>::pseudonym(const pvector<_Tp>& a1, const pvector<_Tp>& a2)\
{\
if (a1.size() != a2.size())\
   EXCEPTION_MNGR(runtime_error, "a1.size()=" << a1.size() << " != a2.size()=" << a2.size());\
resize(a1.size());\
typename pvector<_Tp>::const_iterator curr_a1 = a1.begin();\
typename pvector<_Tp>::const_iterator curr_a2 = a2.begin();\
typename pvector<_Tp>::iterator curr = this->begin();\
typename pvector<_Tp>::iterator last = this->end();\
while (curr != last) {\
  (*curr) = (*curr_a1) op (*curr_a2);;\
  curr++;\
  curr_a1++;\
  curr_a2++;\
  }\
}\
\
template <class _Tp>\
void pvector<_Tp>::pseudonym(const pvector<_Tp>& a1, const _Tp& val)\
{\
resize(a1.size());\
typename pvector<_Tp>::const_iterator curr_a1 = a1.begin();\
typename pvector<_Tp>::iterator curr = this->begin();\
typename pvector<_Tp>::iterator last = this->end();\
while (curr != last) {\
  (*curr) = (*curr_a1) op val;\
  curr++;\
  curr_a1++;\
  }\
}\
\
template <class _Tp>\
pvector<_Tp>& pvector<_Tp>::opname(const _Tp& val)\
{\
typename pvector<_Tp>::iterator curr = this->begin();\
typename pvector<_Tp>::iterator last = this->end();\
while (curr != last) {\
  (*curr) op1 val;\
  curr++;\
  }\
return(*this); \
}\
\
template <class _Tp>\
pvector<_Tp>& pvector<_Tp>::opname(const pvector<_Tp>& array)\
{\
typename pvector<_Tp>::const_iterator curr_a = array.begin();\
typename pvector<_Tp>::iterator curr = this->begin();\
typename pvector<_Tp>::iterator last = this->end();\
while (curr != last) {\
  (*curr) op1 (*curr_a);\
  curr++;\
  curr_a++;\
  }\
return(*this); \
}


BINARYOP(operator+=,+=,plus, + )
BINARYOP(operator-=,-=,minus, - )
BINARYOP(operator*=,*=,times, * )
BINARYOP(operator/=,/=,divide, / )

#undef BINARYOP

#endif

template <class _Tp>
pvector<_Tp> pvector<_Tp>::operator-    () const
{
pvector<_Tp> res(this->size());

typename pvector<_Tp>::const_iterator curr = this->begin();
typename pvector<_Tp>::const_iterator last = this->end();
typename pvector<_Tp>::iterator       res_curr = res.begin();

while (curr != last) {
  (*res_curr) = (*curr);
  res_curr++;
  curr++;
  }
return res;
}



} // namespace utilib

#endif
