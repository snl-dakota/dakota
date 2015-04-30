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
 * \file Tuple.h
 *
 * Defines the tuple class objects: utilib::Tuple1, utilib::Tuple2, 
 * utilib::Tuple3, utilib::Tuple4, utilib::Tuple5, utilib::Tuple6 
 * and utilib::Tuple7.
 *
 * \note It might be possible to create a truly n-tuple object using 
 * template value parameters (e.g. Tuple<n>), but this does not appear to be
 * widely supported at the moment.
 */

#ifndef utilib_Tuple_h
#define utilib_Tuple_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/BasicArray.h>

namespace utilib {

template <class T1> class Tuple1;
template <class T1, class T2> class Tuple2;
template <class T1, class T2, class T3> class Tuple3;
template <class T1, class T2, class T3, class T4> class Tuple4;
template <class T1, class T2, class T3, class T4, class T5> class Tuple5;
template <class T1, class T2, class T3, class T4, class T5, class T6> class Tuple6;
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7> class Tuple7;



/**
 * Implements a 1-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1> class Tuple1 : public PackObject
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple1<T1> >;

  /// Constructor
  explicit Tuple1(T1 _val1) 
		: val1(_val1) {num_elements=1;}

  /// Destructor
  virtual ~Tuple1() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple1<T1>& tuple) const
		{return (tuple.val1 == val1);}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple1<T1>& tuple) const
		{return (val1 < tuple.val1);}

  /// The value for this tuple
  T1 val1;

  /// Write the tuple
  virtual void write(std::ostream& os) const
		{
		os << "( " << val1;
		if (num_elements == 1)
		   os << " )";
		}

  /// Pack the tuple
  virtual void write(PackBuffer& os) const
		{os << val1;}

  /// Read the tuple
  virtual void read(std::istream& /*is*/) 
		{;}

  /// Unpack the tuple
  virtual void read(UnPackBuffer& is)
		{is >> val1;}

protected:

  /// Constructor
  Tuple1() {num_elements=0;}

  /// The number of elements in the tuple
  unsigned int num_elements;
};


/**
 * Implements a 2-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2>
class Tuple2 : public Tuple1<T1>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple2<T1,T2> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple1<T1> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple2(T1 _val1, T2 _val2) 
		: Tuple1<T1>(_val1), val2(_val2) {num_elements=2;}

  /// Destructor
  virtual ~Tuple2() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple2<T1,T2>& tuple) const
		{return (Tuple1<T1>::operator==(tuple) && 
			(tuple.val2 == val2));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple2<T1,T2>& tuple) const
		{return (Tuple1<T1>::operator<(tuple) || 
			 (Tuple1<T1>::operator==(tuple) && 
			  (val2 < tuple.val2)));}

  /// Valued added by this tuple
  T2 val2;

  ///
  void write(std::ostream& os) const
		{
		Tuple1<T1>::write(os);
		os << ", " << val2;
		if (num_elements == 2)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple1<T1>::write(os);os << val2; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val2; Tuple1<T1>::read(is);}

protected:

  /// Constructor
  Tuple2() {num_elements=0;}
};


/**
 * Implements a 3-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2, class T3>
class Tuple3 : public Tuple2<T1,T2>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple3<T1,T2,T3> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple2<T1,T2> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple3(T1 _val1, T2 _val2, T3 _val3) 
		: Tuple2<T1,T2>(_val1,_val2), val3(_val3) {num_elements=3;}

  /// Destructor
  virtual ~Tuple3() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple3<T1,T2,T3>& tuple) const
		{return (Tuple2<T1,T2>::operator==(tuple) && 
			(tuple.val3 == val3));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple3<T1,T2,T3>& tuple) const
		{return (Tuple2<T1,T2>::operator<(tuple) || 
			 (Tuple2<T1,T2>::operator==(tuple) && 
		 	  (val3 < tuple.val3))
			);}

  /// Value added by this tuple
  T3 val3;

  ///
  void write(std::ostream& os) const
		{
		Tuple2<T1,T2>::write(os);
		os << ", " << val3;
		if (num_elements == 3)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple2<T1,T2>::write(os);os << val3; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val3; Tuple2<T1,T2>::read(is);}

protected:

  /// Constructor
  Tuple3() {num_elements=0;}
};


/**
 * Implements a 4-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2, class T3, class T4>
class Tuple4 : public Tuple3<T1,T2,T3>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple4<T1,T2,T3,T4> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple3<T1,T2,T3> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple4(T1 _val1, T2 _val2, T3 _val3, T4 _val4) 
		: Tuple3<T1,T2,T3>(_val1,_val2,_val3), val4(_val4) {num_elements=4;}

  /// Destructor
  virtual ~Tuple4() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple4<T1,T2,T3,T4>& tuple) const
		{return (Tuple3<T1,T2,T3>::operator==(tuple) && 
			(tuple.val4 == val4));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple4<T1,T2,T3,T4>& tuple) const
		{return (Tuple3<T1,T2,T3>::operator<(tuple) ||
		         (Tuple3<T1,T2,T3>::operator==(tuple) && 
			  (val4 < tuple.val4)));}

  /// Value added by this tuple
  T4 val4;

  ///
  void write(std::ostream& os) const
		{
		Tuple3<T1,T2,T3>::write(os);
		os << ", " << val4;
		if (num_elements == 4)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple3<T1,T2,T3>::write(os);os << val4; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val4; Tuple3<T1,T2,T3>::read(is);}

protected:

  /// Constructor
  Tuple4() {num_elements=0;}
};


/**
 * Implements a 5-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2, class T3, class T4, class T5>
class Tuple5 : public Tuple4<T1,T2,T3,T4>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple5<T1,T2,T3,T4,T5> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple4<T1,T2,T3,T4> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple5(T1 _val1, T2 _val2, T3 _val3, T4 _val4, T5 _val5) 
		: Tuple4<T1,T2,T3,T4>(_val1,_val2,_val3,_val4), val5(_val5) {num_elements=5;}

  /// Destructor
  virtual ~Tuple5() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple5<T1,T2,T3,T4,T5>& tuple) const
		{return (Tuple4<T1,T2,T3,T4>::operator==(tuple) && 
			(tuple.val5 == val5));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple5<T1,T2,T3,T4,T5>& tuple) const
		{return (Tuple4<T1,T2,T3,T4>::operator<(tuple) ||
		         (Tuple4<T1,T2,T3,T4>::operator==(tuple) &&
			  (val5 < tuple.val5)));}

  /// Value added by this tuple
  T5 val5;

  ///
  void write(std::ostream& os) const
		{
		Tuple4<T1,T2,T3,T4>::write(os);
		os << ", " << val5;
		if (num_elements == 5)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple4<T1,T2,T3,T4>::write(os);os << val5; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val5; Tuple4<T1,T2,T3,T4>::read(is);}

protected:

  /// Constructor
  Tuple5() {num_elements=0;}
};


/**
 * Implements a 6-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2, class T3, class T4, class T5, class T6>
class Tuple6 : public Tuple5<T1,T2,T3,T4,T5>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple6<T1,T2,T3,T4,T5,T6> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple5<T1,T2,T3,T4,T5> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple6(T1 _val1, T2 _val2, T3 _val3, T4 _val4, T5 _val5, T6 _val6) 
		: Tuple5<T1,T2,T3,T4,T5>(_val1,_val2,_val3,_val4,_val5),
		  val6(_val6) {num_elements=6;}

  /// Destructor
  virtual ~Tuple6() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple6<T1,T2,T3,T4,T5,T6>& tuple) const
		{return (Tuple5<T1,T2,T3,T4,T5>::operator==(tuple) && 
			(tuple.val6 == val6));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple6<T1,T2,T3,T4,T5,T6>& tuple) const
		{return (Tuple5<T1,T2,T3,T4,T5>::operator<(tuple) ||
			 (Tuple5<T1,T2,T3,T4,T5>::operator==(tuple) &&
			  (val6 < tuple.val6)));}

  /// Value added by this tuple
  T6 val6;

  ///
  void write(std::ostream& os) const
		{
		Tuple5<T1,T2,T3,T4,T5>::write(os);
		os << ", " << val6;
		if (num_elements == 6)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple5<T1,T2,T3,T4,T5>::write(os);os << val6; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val6; Tuple5<T1,T2,T3,T4,T5>::read(is);}

protected:

  /// Constructor
  Tuple6() {num_elements=0;}
};


/**
 * Implements a 7-tuple which provides a fixed length list with
 * syntactic differentiation of the elements.
 */
template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class Tuple7 : public Tuple6<T1,T2,T3,T4,T5,T6>
{
public:

  /// Making a BasicArray of tuples a friend so it can construct the array.
  friend class BasicArray<Tuple7<T1,T2,T3,T4,T5,T6,T7> >;
  #if !defined(UTILIB_SOLARIS_CC) && !defined(DOXYGEN)
  typedef Tuple6<T1,T2,T3,T4,T5,T6> base_t;
  using base_t::num_elements;
  #endif

  /// Constructor
  explicit Tuple7(T1 _val1, T2 _val2, T3 _val3, T4 _val4, T5 _val5, T6 _val6,
				T7 _val7) 
		: Tuple6<T1,T2,T3,T4,T5,T6>(_val1,_val2,_val3,_val4,_val5,_val6),
		  val7(_val7) {num_elements=7;}

  /// Destructor
  virtual ~Tuple7() {}

  /// Confirm if two tuples are equal
  bool operator==(const Tuple7<T1,T2,T3,T4,T5,T6,T7>& tuple) const
		{return (Tuple6<T1,T2,T3,T4,T5,T6>::operator==(tuple) && 
			(tuple.val7 == val7));}

  /// Check if the current tuple is lexicographically less than another
  bool operator<(const Tuple7<T1,T2,T3,T4,T5,T6,T7>& tuple) const
		{return (Tuple6<T1,T2,T3,T4,T5,T6>::operator<<(tuple) ||
			 (Tuple6<T1,T2,T3,T4,T5,T6>::operator==(tuple) && 
			  (val7 < tuple.val7)));}

  /// Value added by this tuple
  T7 val7;

  ///
  void write(std::ostream& os) const
		{
		Tuple6<T1,T2,T3,T4,T5,T6>::write(os);
		os << ", " << val7;
		if (num_elements == 7)
		   os << " )";
		}

  ///
  void write(PackBuffer& os) const
		{Tuple6<T1,T2,T3,T4,T5,T6>::write(os);os << val7; }

  ///
  void read(std::istream& /*is*/) 
		{;}

  ///
  void read(UnPackBuffer& is)
		{is >> val7; Tuple6<T1,T2,T3,T4,T5,T6>::read(is);}

protected:

  /// Constructor
  Tuple7() {num_elements=0;}
};

} // namespace utilib

#endif
