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
 * \file BasicArray.h
 *
 * Defines the utilib::BasicArray class
 */

#ifndef utilib_BasicArray_h
#define utilib_BasicArray_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/ArrayBase.h>
#include <utilib/Ereal.h>


namespace utilib {

template <class T>
class BasicArray;


#ifdef UTILIB_HAVE_NAMESPACES
using std::min;
#endif


 /// An iterator base class that defines data types used to 
 /// simplify an iterator's implementation
 template<typename _Category, typename _Tp, typename _Distance = ptrdiff_t,
          typename _Pointer = _Tp*, typename _Reference = _Tp&>
   struct iterator
   {
     #if !defined(DOXYGEN)
     typedef _Category iterator_category;
     typedef _Tp value_type;
     typedef _Distance difference_type;
     typedef _Pointer pointer;
     typedef _Reference reference;
     #endif
   };







 /// Iterator traits.
 template<typename _Iterator>
   struct iterator_traits
   {
     #if !defined(DOXYGEN)
     typedef typename _Iterator::iterator_category iterator_category;
     typedef typename _Iterator::value_type value_type;
     typedef typename _Iterator::difference_type difference_type;
     typedef typename _Iterator::pointer pointer;
     typedef typename _Iterator::reference reference;
     #endif
   };

 /// A specialization of iterator traits for pointer data
 template<typename _Tp>
   struct iterator_traits<_Tp*>
   {
     #if !defined(DOXYGEN)
     typedef std::random_access_iterator_tag iterator_category;
     typedef _Tp value_type;
     typedef ptrdiff_t difference_type;
     typedef _Tp* pointer;
     typedef _Tp& reference;
     #endif
   };

 /// A specialization of iterator traits for const pointer data
 template<typename _Tp>
   struct iterator_traits<const _Tp*>
   {
     #if !defined(DOXYGEN)
     typedef std::random_access_iterator_tag iterator_category;
     typedef _Tp value_type;
     typedef ptrdiff_t difference_type;
     typedef const _Tp* pointer;
     typedef const _Tp& reference;
     #endif
   };

/**
* template class for __normal_iterator  That a BasicArray can used so it has an 
* iterator for the array
*/
template<typename _Iterator, typename _Container>
class __normal_iterator
      : public iterator<typename iterator_traits<_Iterator>::iterator_category,
                        typename iterator_traits<_Iterator>::value_type,
                        typename iterator_traits<_Iterator>::difference_type,
                        typename iterator_traits<_Iterator>::pointer,
                        typename iterator_traits<_Iterator>::reference>
{
public:

  ///makes Difference_type an int type
  typedef typename iterator_traits<_Iterator>::difference_type difference_type;

  ///Makes reference be a & of the datatype in the BasicArray
  typedef typename iterator_traits<_Iterator>::reference reference;

  ///Makes reference be a & of the datatype in the BasicArray
  // WEH - I'm not sure why the SGI compiler
		//       complains when this is included in this
		//	 declaration.
  typedef 
  #if !defined(UTILIB_SGI_CC) && !defined(UTILIB_OSF_CC) && !defined(__PGI) && !defined(_MSC_VER)
  const
  #endif
   typename iterator_traits<_Iterator>::reference const_reference;

  ///Makes reference be a * of the datatype in the BasicArray
  typedef typename iterator_traits<_Iterator>::pointer pointer;

  /**
   * Constructor for __normal_iterator used to pass values to _M_current,
   * _M_begin, _M_end and _M_array. So the Iterator knows where the array 
   * stops and begins
   */
  explicit
  __normal_iterator(const _Iterator& __i,const _Iterator& __begin,
			const _Iterator& __end, _Container* _array)
			: _M_current(__i), _M_begin(__begin), _M_end(__end),
			  _M_array(_array) { }

  /**
   * Constructor for __normal_iterator used to pass values to _M_current,
   * _M_begin, * _M_end and _M_array. So the Iterator knows where the array 
   * stops and begins
   */
  template<typename _Iter>
  inline __normal_iterator(const __normal_iterator<_Iter, _Container>& __i)
        : _M_current(__i.current), _M_begin(__i.begin), _M_end(__i.end), 
	  _M_array(__i._M_array) { }


  /**
   * Return the reference value. Enforce correctness here, checking to see 
   * if the array has changed and to see if the iterator is still in the 
   * array. If so you get an error.
   */
  reference operator*() const
	{
 	if (_M_array->Data != _M_begin)
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - bad iterator index");
 	if ((_M_current < _M_begin) || (_M_current >= _M_end))
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - invalid iterator");
	return *_M_current ;
        }

  /**
   * Return the pointer. Enforce correctness here, checking to see 
   * if the array has changed and to see if the iterator is still in the 
   * array. If so you get an error.
   */
  pointer operator->() const 
 	{
 	if (_M_array->Data != _M_begin)
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - bad iterator index");
 	if ((_M_current < _M_begin) || (_M_current >= _M_end))
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - invalid iterator");
	return _M_current;
	}

  /**
   * Return the pointer. 
   */
  pointer ptr() const 
 	{
	return _M_current;
	}


  /// Returns true if the iteratore value equals the current value
  bool operator==(const __normal_iterator& __x) const
        { return this->_M_current == __x._M_current; }

  /// Returns true if the iteratore value is not equal to the current value
  bool operator!=(const __normal_iterator& __x) const
        { return this->_M_current != __x._M_current; }

  /// Moves the iterator forward one. Prefixed form
  __normal_iterator& operator++()
	{
	++_M_current;
	return *this;
	}

  /// Moves the iterator forward one. Postfixed form
  __normal_iterator operator++(int)
	{ return __normal_iterator(_M_current++,_M_begin,_M_end,_M_array); }

  /// Moves the iterator backward one. Prefixed form
  __normal_iterator& operator--()
	{
	--_M_current;
	return *this;
	}

  ///Moves the iterator backward one space. Postfixed form
  __normal_iterator operator--(int)
	{ return __normal_iterator(_M_current--,_M_begin,_M_end,_M_array); } 

  /**
   * Return the reference value of the specific index value. Enforce 
   * correctness here, checking to see if the array has changed and to see 
   * if the iterator is still in the array. If so you get an error.
   */
  reference operator[](const difference_type& __n) const
       	{
 	if (_M_array->Data != _M_begin)
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - bad iterator index");
 	if ((_M_current < _M_begin) || (_M_current >= _M_end))
	   EXCEPTION_MNGR(runtime_error, "BasicArray iterator - invalid iterator");

  	return _M_current[__n];
	}

  /// Moves the iterator forward \a __n
  __normal_iterator& operator+=(const difference_type& __n)
  	{
	_M_current += __n;
	return *this;
	}

  /// Create an iterator at the index plus \a __n
  __normal_iterator operator+(const difference_type& __n) const
  	{ return __normal_iterator(_M_current + __n,_M_begin,_M_end,_M_array); }

  /// Moves the iterator back \a __n 
  __normal_iterator& operator-=(const difference_type& __n)
  	{
	_M_current -= __n;
	return *this;
	}

  /// Create an iterator at the index minus \a __n
  __normal_iterator operator-(const difference_type& __n) const
 	{ return __normal_iterator(_M_current - __n,_M_begin,_M_end,_M_array); }

  /// Return the current value
  const _Iterator& base() const
   	{ return _M_current; }

protected:

  /// knows where the iterator is in the BasicArray
  _Iterator _M_current;

  ///knows where the BasicArray starts so _M_begin <= _M_current
  _Iterator _M_begin;

  ///Knows where the BasicArray ends so _M_current < _M_end
  _Iterator _M_end;

  /// The array into which we are pointing
  _Container* _M_array;

};



/// A class used by BasicArray to make the assessment of 
/// bounds checking for arrays more portable and robust.  Specifically,
/// this class is used to check if array indeces are non-negative, which
/// is an unnecessary test for unsigned data types.
template <class T>
class BasicArrayBounds
{
protected:

  /// Returns true if the array index is negative
  bool is_negative(const T data) const {return (data < 0);}
};


/// Specialization for size_t
template <>
class BasicArrayBounds<size_type>
{
protected:

  /// Returns true if the array index is negative
  bool is_negative(const size_type) const {return false;}
};

// Forward declaration of structure for registering conversions/serializations
template<typename T> struct BasicArray_registration;


/**
 * A "basic" array of any kind of object, implemented on top of ArrayBase
 */
template <class T>
class BasicArray : public ArrayBase< T,BasicArray<T> >, protected BasicArrayBounds<size_type>
{
public:

  #if !defined(DOXYGEN)
  typedef ArrayBase<T,BasicArray<T> > base_t;
  typedef BasicArray<T> vector_type;
  typedef const BasicArray<T> const_vector_type;
  //#if !defined(UTILIB_SOLARIS_CC)
  using base_t::Data;
  using base_t::Len;
  //#endif

  typedef ::size_type                                   size_type;
  typedef T                                             value_type;
  typedef value_type*                                   pointer;
  typedef const value_type*                             const_pointer;

  typedef typename __normal_iterator<pointer, vector_type>::reference reference;
  typedef typename __normal_iterator<const_pointer, vector_type>::const_reference const_reference;
  typedef __normal_iterator<pointer, vector_type>       iterator;
  typedef __normal_iterator<const_pointer, const_vector_type> const_iterator;
  friend class __normal_iterator<pointer, vector_type>;
  friend class __normal_iterator<const_pointer, vector_type>;
  #endif

  /// Empty constructor.
   BasicArray()
   { 
      this->construct(0,(T*)0,DataOwned); 
   }


  /**
   * Constructor that initializes the length and possibly data.
   * The \a len parameter specifies the length of the array.
   * If the \a d parameter is not null, then this array is
   * assumed to have length \a len.  The initialization of the
   * array then depends upon the value of the \a o parameter. If
   * \a o is \b DataNotOwned (the default), then the data is 
   * copied from \a d.  Otherwise, the internal point is set to
   * \a d and a special list termination pointer is used to 
   * mark the data as shared.
   */
  explicit 
    BasicArray(const size_type mylen, T *d=((T*)0), 
				const EnumDataOwned o=DataNotOwned)
		{ this->construct(mylen,d,o); }

  /// Copy constructor.
  BasicArray(const BasicArray<T>& array)
     : ArrayBase<T,BasicArray<T> >(array), 
       BasicArrayBounds<size_type>(array)
  {
     (void)registrations_complete;
     this->construct(array.size(), array.data(), DataOwned); 
  }

  /// Destructor.
  virtual ~BasicArray()
		{ }

  /// Begin iterator
  iterator begin()
                {return iterator(Data, Data, Data+Len, this);}
 
  /// Begin const_iterator
  const_iterator begin() const
		{return const_iterator(Data, Data, Data+Len, 
					const_cast<BasicArray<T>*>(this));}

  /// end iterator
  iterator end()
		{return iterator(Data+Len, Data, Data+Len, this);}

  /// end const_iterator
  const_iterator end() const
                {return const_iterator(Data+Len, Data, Data+Len, this);}

  /// The back of the array
  reference back()
		{return *(end()-1);}

  /// The back of the array
  const_reference back() const
		{return *(end()-1);}

  /// Resizes the vector to have no elements
  void clear()
		{ base_t::resize(0); }

 /**
   * Accesses the i-th element of the array.
   * This method gives BasicArray's the same look-and-feel as C
   * arrays.  Note that the index for these arrays begins at zero.
   * This method returns a modifiable data element.
   */
  T& operator[](size_type i);

  /// Accesss the i-th element of the array and returns a const.
  const T& operator[](size_type i) const;

  /// Copy operator
  BasicArray<T>& operator=(const BasicArray<T>& val)
		{base_t::operator=(val); return *this;}

  /** Set the Data and len of the array to \a data and \a len,
    * respectively, and set the ownership mode ot \a o.  This
    * operation also affects all arrays that are sharing memory
    * with this one.
    * This method is basically inherited from ArrayBase, 
    * except it returns a reference.  
    */
  BasicArray<T>& set_data(const size_type len, 
			  T* data,
			  const EnumDataOwned o=DataNotOwned)
		{
      		base_t::set_data(len,data,o);
      		return *this;
    		} 

  /** 
   * Method to explicitly set the internal data array to share the data 
   * in a \b BasicArray.
   * The \a start parameter specifies the initial point in the 
   * BasicArray, and \a newlen specifies the length of the subvector
   * that the current array object sees.
   * Applies to all members of a share group.
   * Note: the \a vec object still owns the memory.
   */
  BasicArray<T>& set_data(const BasicArray<T>& vec, 
			  const size_type start,
			  const size_type newlen);

  /// Calls \b set_data with a default value for \a newlen
  BasicArray<T>& set_data(const BasicArray<T>& vec, const size_type start=0)
		{ return set_data(vec,start,vec.size()-start); }

  /// Copy a subvector of a \b BasicArray into the current array.
  BasicArray<T>& set_subvec(const size_type start, 
			    const size_type len,
			    const BasicArray<T>& datavec, 
			    const size_type offset=0);

  /// This works like the \b set_data method
  BasicArray<T>& at(const size_type start, const size_type newlen,
		    BasicArray<T>* array = (BasicArray<T>*)0) const;

  /// This works like the \b set_data method
  BasicArray<T>& at(const size_type start=0) const
		{ return at(start, base_t::size()-start); }


  /// Resize and append an element onto the array
  void append(const T& element)
		{
      		size_type tmp=Len;
      		base_t::resize(tmp+1);
      		(*this)[tmp] = element;
    		}

  /**
   * Append an element to the end of an array.
   * Note: this forces a resize of the array, which is not amortized!
   */
  void push_back(const T& element)
	  	{ append(element); }

  /// Resize and append an array onto the array
  void append(BasicArray<T>& vec)
    		{
      		size_type tmp=Len;
      		resize(tmp+vec.size());
      		for (size_type i=tmp; i<Len; i++)
		  (*this)[i] = vec[i-tmp];
    		}


  /// Checks to see if the entire array equals \b val.
  bool operator==(const T& val) const;

  /// Checks to see if the entire array does not equal \b val.
  bool operator!=(const T& val) const;

  /// Checks to see if the current array equals \b array.
  bool operator==(const BasicArray<T>& array) const;

  /// Checks to see if the current array does not equal \b array.
  bool operator!=(const BasicArray<T>& array) const;

  /**
  * Compares the current array with \b array.
  * Returns -1 if the current array is lexicographically less than 
  * \b array, 0 if they are equal, and 1 otherwise.
  */
  int compare(const BasicArray<T>& array) const;

  /// Checks to see if the current array is lexicographically
  /// less than \b array.
  inline bool operator<(const BasicArray<T>& array) const
        { return compare(array) < 0; }

  /// Checks to see if the current array is lexicographically
  /// greater than \b array.
  inline bool operator>(const BasicArray<T>& array) const
        { return compare(array) > 0; }

  /// Checks to see if the current array is lexicographically
  /// less than or equal to \b array.
  inline bool operator<=(const BasicArray<T>& array) const
        { return compare(array) <= 0; }

  /// Checks to see if the current array is lexicographically
  /// greater than or equal to \b array.
  inline bool operator>=(const BasicArray<T>& array) const
        { return compare(array) >= 0; }


protected:
   static int serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize );
private:
   friend struct BasicArray_registration<T>;

   static const volatile bool registrations_complete;

   template<typename FROM, typename TO>
   static int stream_cast(const Any& from, Any& to)
   {
      to.set<TO>() << from.template expose<FROM>();
      return OK;
   }
};

/// Structure containing method to register type conversions and serialization.
/** Indirection structure containing method to register type conversions
 *  and serialization.  This allows sepcialization / partial
 *  specialization to disable the registration for specific template
 *  instances.
 */
template<typename T>
struct BasicArray_registration {
   static bool registrar()
   {
      Serializer().template register_serializer<BasicArray<T> >
         ( std::string("utilib::BasicArray;") + mangledName(typeid(T)),
           BasicArray<T>::serializer );
         
      TypeManager()->register_lexical_cast
         (typeid(BasicArray<T>), typeid(std::vector<T>), 
          BasicArray<T>::template stream_cast<BasicArray<T>, std::vector<T> >);
      TypeManager()->register_lexical_cast
         (typeid(std::vector<T>), typeid(BasicArray<T>), 
          BasicArray<T>::template stream_cast<std::vector<T>, BasicArray<T> >);
         
      return true;
   }
};

//============================================================================

template<typename T>
const volatile bool BasicArray<T>::registrations_complete = 
   BasicArray_registration<T>::registrar();


//============================================================================
//
//
template <class T>
T & BasicArray<T>::operator[](size_type idx)
{
#if (ArraySanityChecking==1)
if (is_negative(idx) || (static_cast<size_type>(idx) >= Len))
   EXCEPTION_MNGR(runtime_error, "BasicArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << Len);
#endif
 
return Data[idx];
}
 
 
//============================================================================
//
//
template <class T>
const T & BasicArray<T>::operator[](size_type idx) const
{
#if (ArraySanityChecking==1)
if (is_negative(idx) || (static_cast<size_type>(idx) >= Len))
   EXCEPTION_MNGR(runtime_error, "BasicArray<T>::operator[] : iterator out of range. idx=" << idx << " len=" << Len);
#endif
 
return Data[idx];
}


//============================================================================
//
//
template <class T>
BasicArray<T>& BasicArray<T>::set_data(const BasicArray<T>& array, 
				       const size_type start,
				       const size_type newlen)
{
  if (array.Data == Data)     // Return unmodified array when
    return *this;             // you are setting yourself.
 
  if (array.Data)
    base_t::set_data(newlen, &(array.Data[start]), DataNotOwned);
  else
    base_t::resize(0);

  return *this;
}


//============================================================================
//
//
template <class T>
BasicArray<T>& BasicArray<T>::set_subvec(const size_type start, 
					 const size_type len,
					 const BasicArray<T>& array, 
					 const size_type offset)
{
  T* begin   = Data + start;
  T* newdata = array.Data + offset;
  for (size_type i=0; i<len; i++, begin++, newdata++)
    *begin = *newdata;
 
  return *this;
}


//============================================================================
//
//
template <class T>
BasicArray<T>& BasicArray<T>::at(const size_type start, 
				 const size_type newlen,
				 BasicArray<T>* array ) const
{
  BasicArray<T> *tmp;
 
  if (array == NULL) {
    tmp = new BasicArray<T>();
    if (tmp == 0)
       EXCEPTION_MNGR(runtime_error, "BasicArray::at - new BasicArray failed.");
  }
  else
    tmp = array;
  tmp->set_data((*this), start, newlen);
  return *tmp;
}



//============================================================================
//
//
template <class T>
bool BasicArray<T>::operator==(const T& val) const
{
T* tmp = Data;
size_type mylen = base_t::size();
for (size_type i=0; i<mylen; i++)
  if (tmp[i] != val)
     return false;
 
return true;
}


//============================================================================
//
//
template <class T>
bool BasicArray<T>::operator!=(const T& val) const
{
T* tmp = Data;
size_type mylen = base_t::size();
for (size_type i=0; i<mylen; i++)
  if (tmp[i] != val)
     return true;
 
return false;
}
 

//============================================================================
//
//
template <class T>
bool BasicArray<T>::operator==(const BasicArray<T>& array) const
{
if (array.Data == Data)
   return true;
if (array.size() != base_t::size())
   return false;
 
T* tmp = Data;
T* arraytmp = array.Data;
size_type mylen = base_t::size();
for (size_type i=0; i<mylen; i++)
  if (tmp[i] != arraytmp[i])
     return false;
 
return true;
}


//============================================================================
//
//
template <class T>
int BasicArray<T>::compare(const BasicArray<T>& array) const
{
if (array.Data == Data)
   return 0;

size_type mylen = base_t::size();
size_type arraylen = array.size();
 
T* tmp = Data;
T* arraytmp = array.Data;
for (size_type i=0; i< std::min(mylen,arraylen); i++) {
  if (tmp[i] != arraytmp[i]) {
     if (tmp[i] < arraytmp[i]) return -1;
     else return 1;
     }
  }

if (arraylen == mylen)
   return 0;
else if (arraylen > mylen)	
   return -1;
else
   return 1;
}

//============================================================================

/// Serialize a BasicArray object
template <class T>
int BasicArray<T>::serializer( SerialObject::elementList_t& serial, 
                               Any& object, bool serialize )
{
   // The following is a trick to cast the incoming Any into the base
   // class (needed until Anys support polymorphism). 
   Any tmp;
   tmp.set<ArrayBase<T,BasicArray<T> > >
      (object.template expose<BasicArray<T> >(), true);
   // NB: directly calling the base class serializer ONLY works
   // because this derived class does not add any additional data.
   // That said, doing this saves a level of indirection in the
   // actual serial stream.
   return ArrayBase<T,BasicArray<T> >::serializer(serial, tmp, serialize); 
}

template<class T>
class Any::Comparator<BasicArray<T> > 
   : public STL_Any_AuxFcns::SequenceComparator<BasicArray<T> > 
{};

template<class T>
class Any::Printer<BasicArray<T> > 
   : public STL_Any_AuxFcns::SequencePrinter<BasicArray<T> > 
{};


//============================================================================
//
//
template <class T>
bool BasicArray<T>::operator!=(const BasicArray<T>& array) const
{
if (array.Data == Data) return false;

size_type mylen = base_t::size();
if (array.size() != mylen)
   return true;
 
T* tmp = Data;
T* arraytmp = array.Data;
for (size_type i=0; i<mylen; i++)
  if (tmp[i] != arraytmp[i])
     return true;
 
return false;
}


/// Comparison operator for BasicArray objects
template <class T>
int compare(const BasicArray<T>& first, const BasicArray<T>& second)
{ return first.compare(second);}


} // namespace utilib


//============================================================================
///
/// Operator to copy one array into another
///
template <class T, class V>
utilib::BasicArray<T>& operator<<(utilib::BasicArray<T>& x, const utilib::BasicArray<V>& y)
{
  if (((void*)(x.data())) == ((void*)(y.data())))
    return x;
  size_type len = x.size();
  if (len != y.size()) {
    x.resize(y.size());
    len = y.size();
    }
 
  for (size_type i=0; i<len; i++)
    x[i] = y[i];

  return x;
}


//============================================================================
///
/// Operator to fill an array with a value
///
template <class T>
utilib::BasicArray<T>& operator<<(utilib::BasicArray<T>& x, const T& y)
{
for (size_type i=0; i<x.size(); i++)
  x[i] = y;

return x;
}


//============================================================================
///
/// Operator to fill an array of Ereal objects with a numerical value
///
template <class T>
utilib::BasicArray<utilib::Ereal<T> >& operator<<(utilib::BasicArray<utilib::Ereal<T> >& x, const T& y)
{
for (size_type i=0; i<x.size(); i++)
  x[i] = y;

return x;
}


//============================================================================
///
/// Operator to copy a BasicArray into a STL vector
///
template <class TYPE, class BTYPE>
std::vector<TYPE>& operator<< ( std::vector<TYPE>& vec, const utilib::BasicArray<BTYPE>& old)
{
vec.resize(old.size());
typename std::vector<TYPE>::iterator curr = vec.begin();
typename std::vector<TYPE>::iterator last = vec.end();

int i=0;
while (curr != last) {
  *curr = old[i];
  ++curr;
  i++;
  }
return vec;
}


//============================================================================
///
/// Operator to copy a STL vector into a BasicArray
///
template <class TYPE, class BTYPE>
utilib::BasicArray<BTYPE>& operator<< ( utilib::BasicArray<BTYPE>& vec, const std::vector<TYPE>& old)
{
vec.resize(old.size());
typename std::vector<TYPE>::const_iterator curr = old.begin();
typename std::vector<TYPE>::const_iterator last = old.end();

int i=0;
while (curr != last) {
  vec[i] = *curr;
  ++curr;
  i++;
  }
return vec;
}

/// Out-stream operator for writing the contents of a BasicArray<T>
template <class T>
inline std::ostream& operator<<(std::ostream& os, const utilib::BasicArray<T>& obj)
{
os << obj.size() << " : ";
T* tmp = obj.data();
for (size_type i=0; i<obj.size(); i++, tmp++)
  os << *tmp << " ";
return os;
}

/// Out-stream operator for writing the contents of a BasicArray<T>
template <class T>
inline utilib::PackBuffer& operator<<(utilib::PackBuffer& os,
                                        const utilib::BasicArray<T>& obj)
{
os << obj.size();
T* tmp = obj.data();
for (size_type i=0; i<obj.size(); i++, tmp++)
  os << *tmp;
return os;
}

/// In-stream operator for reading the contents of a BasicArray<T>
template <class T>
inline std::istream& operator>>(std::istream& is, utilib::BasicArray<T>& obj)
{
size_type len;
is >> len;
obj.resize(len);

char c;
is >> c;

T* tmp = obj.data();
for (size_type i=0; i<len; i++, tmp++)
  is >> *tmp;
return is;
}

/// In-stream operator for reading the contents of a BasicArray<T>
template <class T>
inline utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& is,
                                        utilib::BasicArray<T>& obj)
{
size_type len=0;
is >> len;
obj.resize(len);

T* tmp = obj.data();
for (size_type i=0; i<len; i++, tmp++)
  is >> *tmp;
return is;
}


#ifdef CXXTEST_RUNNING
#include <cxxtest/ValueTraits.h>
namespace CxxTest {

/// Printing utility for use in CxxTest unit tests
template<class T>
class ValueTraits< utilib::BasicArray<T> >
{
public:
   ValueTraits(const utilib::BasicArray<T>& t)
   {
      std::ostringstream ss;
      ss << t;
      str = ss.str().c_str();
   }

   const char *asString() const
   { return str.c_str(); }

private:
   std::string str;
};

} // namespace CxxTest
#endif // CXXTEST_RUNNING


#endif // utilib_BasicArray_h
