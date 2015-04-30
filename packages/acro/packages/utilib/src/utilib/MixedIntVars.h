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
 * \file MixedIntVars.h
 *
 * Defines the utilib::MixedIntVarsRep and utilib::MixedIntVars classes
 */

#ifndef utilib_MixedIntVars_h
#define utilib_MixedIntVars_h

#include <utilib/std_headers.h>
#include <utilib/BitArray.h>
#include <utilib/NumArray.h>
#include <utilib/Any.h>

namespace utilib {

/**
 * The internal class that is used to manage the data for MixedIntVars 
 * objects.
 */
class MixedIntVarsRep
{
 
  #if !defined(DOXYGEN)
  friend class MixedIntVars;
  #endif

protected:
 
  /// Constructor, which can be used to specify the number of different groups
  /// of variables.
  explicit MixedIntVarsRep(size_type nBinVars=0, size_type nIntVars=0, size_type nDblVars=0) 
     : ref(1), 
       //own(AssumeOwnership),
       bitVars(nBinVars), gintVars(nIntVars), doubleVars(nDblVars)
   {}

  /// Number of MixedIntVars objects that are referencing this instance.
  int ref;

  /// The ownership category. -- not used.
  //EnumDataOwned own;

  /// The array used to store the binary variables.
  BitArray bitVars;

  /// The array used to store the general integer variables.
  NumArray<int> gintVars;

  /// The array used to store the real variables.
  NumArray<double> doubleVars;

};



/**
 * A class that defines a set of parameters that includes binary, integer and
 * real variables.  This class manages these variables in arrays of bits, 
 * integers and doubles.
 *
 * \par
 * This class uses set() and get() methods to manage the values of the
 * variables.  This is more cumbersum than using an operator[] method, but
 * the operator[] method does not work well for binary variables.
 *
 * \note Note actively used.
 */
class MixedIntVars : public PackObject
{
public:

  //typedef BitArray::iterator binary_iterator;
  //typedef BitArray::const_iterator const_binary_iterator;
  typedef NumArray<int>::iterator integer_iterator;
  typedef NumArray<int>::const_iterator const_integer_iterator;
  typedef NumArray<double>::iterator real_iterator;
  typedef NumArray<double>::const_iterator const_real_iterator;

  /**
   * Constructor, which can be used to specify the number of different groups 
   * of variables.
   */
  MixedIntVars(const int numBits, const int numGenInts, const int numDoubles); 
  /**
   * Empty constructor.
   */
  MixedIntVars();

  /**
   * Copy constructor.
   */
  MixedIntVars(const MixedIntVars& obj)
     : PackObject(obj), 
       a(0)
   { (void)registrations_complete; *this = obj; }

  /// Destructor.
  virtual ~MixedIntVars()
		{free();}

  /// Resizes the number of binary, integer and real variables.
  void resize(const size_type numBits, const size_type numGenInts, const size_type numDoubles);

  /// Returns the number of references to the MixedIntVarsRep class.
  int nrefs() const
	{return (a ? a->ref : 0);}

  /// Returns the total number of integer variables.
  size_type numIntegerVars() const
		{ return (a ? a->gintVars.size() + a->bitVars.size() : 0); }

  /// Returns the number of binary variables.
  size_type numBinaryVars() const
		{ return (a ? a->bitVars.size() : 0); }

  /// Returns the number of general integer variables.
  size_type numGeneralIntVars() const
		{ return (a ? a->gintVars.size() : 0); }

  /// Returns the number of real variables.
  size_type numDoubleVars() const
		{ return (a ? a->doubleVars.size() : 0); }

  /// Returns the number of all variables.
  size_type numAllVars() const
		{ return (a ?  a->bitVars.size() + a->gintVars.size() + a->doubleVars.size() : 0); }


  /// Returns the binary variables
  BitArray& Binary() { return a->bitVars; }

  /// Returns the binary variables
  const BitArray& Binary() const { return a->bitVars; }

  /// Returns the integer variables
  NumArray<int>& Integer() { return a->gintVars; }

  /// Returns the integer variables
  const NumArray<int>& Integer() const { return a->gintVars; }

  /// Returns the real variables
  NumArray<double>& Real() { return a->doubleVars; }

  /// Returns the real variables
  const NumArray<double>& Real() const { return a->doubleVars; }

  /**
   * Gets the value of the \a ndx-th variable.
   * This assumes that binary variables precede general integer
   * variables, which precede real variables.
   */
  //JDS: this method is not implementable (how do you get a double
  //   reference into a int or bit array???)
  //double& get(const size_type ndx);

  /**
   * Gets the value of the \a ndx-th variable.
   * This assumes that binary variables precede general integer
   * variables, which precede real variables.
   */
  double get(const size_type ndx) const;

  /// Copies the \b array object by constructing a new \b MixedIntVarsRep.
  MixedIntVars& operator=(const MixedIntVars& array);

  /// Copies the pointer from the \b array object.
  MixedIntVars& operator&=(const MixedIntVars& array);

  /// Set all binary variables to \b val.
  MixedIntVars& operator=(const char val);

  /// Set all general integer variables to \b val.
  MixedIntVars& operator=(const int val);

  /// Set all real variables to \b val.
  MixedIntVars& operator=(const double val);

  /// Checks to see if the current array equals \b array.
  bool operator==(const MixedIntVars& array) const;
  /// Checks if the current array is "less than" \b array (for STL support)
  bool operator<(const MixedIntVars& array) const;

  /**
   * Compares \a array with the current variables.
   * Compares the binary, integer and then real variable arrays.
   * If all are equal, then returns 0.  Otherwise, when one is
   * non-zero, return that comparison.
   */
  int compare(const MixedIntVars& array) const;

  /// Write the array to an output stream.
  void write(std::ostream& os) const;

  /// Read the array from an input stream.
  void read(std::istream& is);

  /// Pack the array into a PackBuffer class.
  void write(PackBuffer& os) const;

  /// Unpack the array from a UnPackBuffer class.
  void read(UnPackBuffer& is);

protected:

  /// The pointer to the internal array representation.
  MixedIntVarsRep* a;

  /// Method used by constructors to setup the MixedIntVars class.
  void construct(const size_type nbits, const size_type nints, const size_type ndlbs);

  /// Method used to delete the internal array classes.
  void free();

private:
   static const volatile bool registrations_complete;

   /// The MixedIntVars serializer / deserializer
   static int serializer( SerialObject::elementList_t& serial, 
                          Any& data, bool serialize );

   static bool register_aux_functions();
};

} // namespace utilib


/// Copy doubles from a MixedIntVars object into a STL vector
inline std::vector<double>& operator<< (std::vector<double>& buff, 
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Real().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Real()[i];
return buff;
}

/// Copy a STL vector of doubles into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const std::vector<double>& buff)
{
vars.Real().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Real()[i] = buff[i];
return vars;
}

/// Copy doubles from a MixedIntVars object into a NumArray
inline utilib::NumArray<double>& operator<< (utilib::NumArray<double>& buff, 
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Real().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Real()[i];
return buff;
}

/// Copy a NumArray of doubles into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::NumArray<double>& buff)
{
vars.Real().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Real()[i] = buff[i];
return vars;
}

/// Copy doubles from a MixedIntVars object into a BasicArray
inline utilib::BasicArray<double>& operator<< (utilib::BasicArray<double>& buff,
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Real().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Real()[i];
return buff;
}

/// Copy a BasicArray of doubles into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::BasicArray<double>& buff)
{
vars.Real().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Real()[i] = buff[i];
return vars;
}


/// Copy integers from a MixedIntVars object into a vector
inline std::vector<int>& operator<< (std::vector<int>& buff, 
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Integer().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Integer()[i];
return buff;
}

/// Copy a vector of integers into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const std::vector<int>& buff)
{
vars.Integer().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Integer()[i] = buff[i];
return vars;
}

/// Copy integers from a MixedIntVars object into a NumArray
inline utilib::NumArray<int>& operator<< (utilib::NumArray<int>& buff, 
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Integer().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Integer()[i];
return buff;
}

/// Copy a NumArray of integers into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::NumArray<int>& buff)
{
vars.Integer().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Integer()[i] = buff[i];
return vars;
}

/// Copy integers from a MixedIntVars object into a BasicArray
inline utilib::BasicArray<int>& operator<< (utilib::BasicArray<int>& buff, 
					const utilib::MixedIntVars& vars)
{
buff.resize(vars.Integer().size());
for (size_type i=0; i<buff.size(); i++)
  buff[i] = vars.Integer()[i];
return buff;
}

/// Copy a BasicArray of integers into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::BasicArray<int>& buff)
{
vars.Integer().resize(buff.size());
for (size_type i=0; i<buff.size(); i++)
  vars.Integer()[i] = buff[i];
return vars;
}

/// Copy booleans from a MixedIntVars object into a BitArray
inline utilib::BitArray& operator<< (utilib::BitArray& buff, 
					const utilib::MixedIntVars& vars)
{
buff << vars.Binary();
return buff;
}

/// Copy a BitArray into a MixedIntVars object
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::BitArray& buff)
{
vars.Binary() << buff;
return vars;
}

/// Copy one MixedIntVars into another
inline utilib::MixedIntVars& operator<< (utilib::MixedIntVars& vars,
					const utilib::MixedIntVars& buff)
{
vars.resize(buff.numBinaryVars(), buff.numGeneralIntVars(), buff.numDoubleVars());

vars << buff.Binary();
vars << buff.Integer();
vars << buff.Real();

return vars;
}

namespace utilib {
DEFINE_FULL_ANY_EXTENSIONS(utilib::MixedIntVars);
} // namespace utilib

#endif
