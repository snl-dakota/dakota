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
 * \file AnyRNG.h
 *
 * Defines the utilib::AnyRNG class.
 */

#ifndef utilib_AnyRNG_h
#define utilib_AnyRNG_h

#include <utilib/std_headers.h>

namespace utilib {

#ifndef UTILIB_SEED_TYPE
#define UTILIB_SEED_TYPE
typedef unsigned int seed_t;
#endif


/**
 * Templated container that contains a pointer to any RNG-compatible random
 * number generator, which does <i>not</i> need to be a subclass of RNG.
 */
class AnyRNG
{
public:

  /// Constructor
  AnyRNG()
	: content(0)
	{}

  /**
   * Templated constructor that takes a pointer to a RNG-compatible 
   * random number generator
   */
  template <typename RNGT>
  AnyRNG(RNGT* rng)
	: content(rng ? new holder<RNGT>(rng) : 0)
	{}

  /// Constructor that takes a pointer to a AnyRNG object
  explicit AnyRNG(AnyRNG* rng)
	: content(rng && rng->content ? rng->content->clone() : 0)
	{}

  /// Copy constructor
  AnyRNG(const AnyRNG& other)
	: content(other.content ? other.content->clone() : 0)
	{}

  /// Destructor
  ~AnyRNG()
	{ if (content) delete content; }

  /// Equal operator
  template<typename RNGT>
  AnyRNG& operator=(RNGT* rhs)
	{
	if (content) delete content;
	content = (rhs ? new holder<RNGT>(rhs) : 0);
	return *this;
	}

  /// Equal operator
  AnyRNG& operator=(const AnyRNG& rhs)
	{
	if (content) delete content;
	content = (rhs.content ? rhs.content->clone() : 0);
	return *this;
	}

  /// Equals operator
  template<typename RNGT>
  bool operator==(RNGT* rhs) const
	{
	AnyRNG rng(rhs);
	return operator==(rng);
	}

  /// Equals operator
  bool operator==(const AnyRNG& rhs) const
	{
	if (content) {
	   if (rhs.content)
	      return (content->voidptr() == rhs.content->voidptr());
	   else
	      return false;
           }
	return !(rhs.content);
	}

  /// Not-equals operator
  template<typename RNGT>
  bool operator!=(RNGT* rhs) const
	{
	AnyRNG rng(rhs);
	return operator!=(rng);
	}

  /// Not-equals operator
  bool operator!=(const AnyRNG& rhs) const
	{
	if (content) {
	   if (rhs.content)
	      return (content->voidptr() != rhs.content->voidptr());
	   else
	      return true;
           }
	return (rhs.content != 0);
	}

  /// Returns true if the object is not pointing to an object
  bool empty() const
	{
	return !content;
	}

  /// Returns true if the object is pointing to an object
  operator bool () const
	{
	return content != 0;
	}

  /// Return float in [0,1[
  float asFloat()
	{
	return content ? content->asFloat() : static_cast<float>(0.0);
	}

  /// return double in [0,1[
  double asDouble()
	{
	return content ? content->asDouble() : 0.0;
        }

  /// Return long in 0...LONG_MAX
  unsigned long asLong()
	{
	return content ? content->asLong() : 0;
	}

  /// A generic API for setting seeds for the RNG
  void set_seed(seed_t seed=0)
	{ content->set_seed(seed); }

  /// A generic API for getting seeds for the RNG
  seed_t get_seed() const
	{ return content->get_seed(); }

  /// Reset the state of the random number generator
  void reset()
	{
	if (content) content->reset();
	}

  /// Write the random number generator's state to a stream
  void write(std::ostream& output) const
	{
	if (content) content->write(output);
	}

  /// Read the random number generator's state from a stream
  void read(std::istream& input)
	{
	if (content) content->read(input);
	}

#ifndef NO_MEMBER_TEMPLATE_FRIENDS
    private: // types
#else
    public: // types (public so any_cast can be non-friend)
#endif

	/// Private class that defines the base class of the
	/// objects to which an AnyRNG points
        class placeholder
        {
        public: // structors

	    /// Destructor
            virtual ~placeholder() { }

        public: // queries

	    /// Clone this object
            virtual placeholder * clone() const = 0;

	    /// Generate a float with this RNG
            virtual float asFloat() = 0;

	    /// Generate a double with this RNG
            virtual double asDouble() = 0;

	    /// Generate a long integer with this RNG
            virtual long asLong() = 0;

	    /// Set the RNG seed
	    virtual void set_seed(seed_t) = 0;

	    /// Get the RNG seed
	    virtual seed_t get_seed() = 0;

	    /// Reset this object
            virtual void reset() = 0;

	    /// Write this object
            virtual void write(std::ostream& ) const = 0;

	    /// Read this object
            virtual void read(std::istream& ) = 0;

            /// Return the value as a void pointer
	    virtual const void* voidptr() const = 0;

        };

	/// Private templated class to which an AnyRNG points
        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

	    /// Constructor
            holder(ValueType* value)
              : held(value) { }

        public: // queries

	    /// Clone this object
            placeholder * clone() const
              { return new holder(held); }

	    /// Generate a float with this RNG
            float asFloat()
              { return held->asFloat(); }

	    /// Generate a double with this RNG
            double asDouble()
              { return held->asDouble(); }

	    /// Generate a long integer with this RNG
            long asLong()
              { return held->asLong(); }

	    /// Set the RNG seed
	    void set_seed(seed_t val)
		{ return held->set_seed(val); }

	    /// Get the RNG seed
	    seed_t get_seed()
		{ return held->get_seed(); }

	    /// Reset this object
            void reset()
              { held->reset(); }

	    /// Write this object
            void write(std::ostream& ostr) const
              { ostr << *held; }

	    /// Read this object
            void read(std::istream& istr)
              { istr >> *held; }

            /// Return the value as a void pointer
	    const void* voidptr() const
	      { return held; }

        public: // representation

	    /// The value of this object
            ValueType* held;

        };

protected: // representation

  /// The random number generator to which the object points
  placeholder * content;

};
   
} // namespace utilib


/// Out-stream operator for AnyRNG objects
inline std::ostream& operator << (std::ostream& output, const utilib::AnyRNG& rng)
{
rng.write(output);
return output;
}


/// In-stream operator for AnyRNG objects
inline std::istream& operator >> (std::istream& input, utilib::AnyRNG& rng)
{
rng.read(input);
return input;
}

#endif
