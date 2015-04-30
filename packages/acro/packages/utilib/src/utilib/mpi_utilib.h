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
 * \file mpi_utilib.h
 *
 * This header file provides misc wrapper information for MPI, including
 * dummy declarations of MPI data types for when MPI is not defined.
 *
 * \author William E. Hart
 */

#ifndef utilib_mpi_utilib_h
#define utilib_mpi_utilib_h

#include <utilib_config.h>

/*
 * TODO: When is DAKOTA_UTILIB defined? configure command line?
 */
#if ( !defined(UTILIB_HAVE_MPI) && !defined(DAKOTA_UTILIB) ) || defined(DOXYGEN)

/**
 * \typedef MPI_Request
 *
 * A dummy typedef for when MPI is not used.
 */
typedef void* MPI_Request;

/**
 * \typedef MPI_Status
 *
 * A dummy typedef for when MPI is not used.
 */
typedef void* MPI_Status;

#endif


#if defined(UTILIB_HAVE_MPI) || defined(DOXYGEN)


#include <mpi.h>

#if defined(LAM_MPI)
#define UTILIB_MPI_COMM_IS_POINTER
#endif

#if !defined(DOXYGEN)

inline MPI_Datatype mpi_datatype(char*)
{return MPI_CHAR;}

inline MPI_Datatype mpi_datatype(unsigned char*)
{return MPI_UNSIGNED_CHAR;}

/// Workaround for MPI distributions lacking MPI_SIGNED_CHAR.
/// Not sure this is safe, but should allow us to at least compile (JE)
#ifdef MPI_SIGNED_CHAR
inline MPI_Datatype mpi_datatype(signed char*)
{return MPI_SIGNED_CHAR;}
#else
inline MPI_Datatype mpi_datatype(signed char*)
{return MPI_BYTE;}
#endif

inline MPI_Datatype mpi_datatype(short*)
{return MPI_SHORT;}

inline MPI_Datatype mpi_datatype(unsigned short*)
{return MPI_UNSIGNED_SHORT;}

inline MPI_Datatype mpi_datatype(int*)
{return MPI_INT;}

inline MPI_Datatype mpi_datatype(unsigned int*)
{return MPI_UNSIGNED;}

inline MPI_Datatype mpi_datatype(long*)
{return MPI_LONG;}

inline MPI_Datatype mpi_datatype(unsigned long*)
{return MPI_UNSIGNED_LONG;}

inline MPI_Datatype mpi_datatype(float*)
{return MPI_FLOAT;}

inline MPI_Datatype mpi_datatype(double*)
{return MPI_DOUBLE;}

inline MPI_Datatype mpi_datatype(long double*)
{return MPI_LONG_DOUBLE;}

inline MPI_Datatype mpi_datatype(long long int*)
{return MPI_LONG_LONG_INT;}

#ifdef MPI_UNSIGNED_LONG_LONG
inline MPI_Datatype mpi_datatype(unsigned long long int*)
{return MPI_UNSIGNED_LONG_LONG;}
#else
inline MPI_Datatype mpi_datatype(unsigned long long int*)
{return MPI_UNSIGNED_LONG;}
#endif

template <class TYPE>
inline int PackSize(const TYPE& data, const int num=1)
{
int size;
MPI_Pack_size(num, mpi_datatype((TYPE*)&data), MPI_COMM_WORLD, &size);
return size;
}

#else


/// Return the MPI datatype for a templated datatype
template <class TYPE>
inline MPI_Datatype mpi_datatype()
                {return MPI_DATATYPE_NULL;}


/// Specialization for char
template <>
inline MPI_Datatype mpi_datatype<char>()
{return MPI_CHAR;}

/// Specialization for const char
template <>
inline MPI_Datatype mpi_datatype<char const>()
{return MPI_CHAR;}

/// Specialization for unsigned char
template <>
inline MPI_Datatype mpi_datatype<unsigned char>()
{return MPI_UNSIGNED_CHAR;}

/// Specialization for unsigned const char
template <>
inline MPI_Datatype mpi_datatype<unsigned char const>()
{return MPI_UNSIGNED_CHAR;}

/// Specialization for unsigned char
template <>
inline MPI_Datatype mpi_datatype<signed char>()
{return MPI_SIGNED_CHAR;}

/// Specialization for unsigned char
template <>
inline MPI_Datatype mpi_datatype<signed char const>()
{return MPI_SIGNED_CHAR;}

/// Specialization for short int
template <>
inline MPI_Datatype mpi_datatype<short>()
{return MPI_SHORT;}

/// Specialization for unsigned short int
template <>
inline MPI_Datatype mpi_datatype<unsigned short>()
{return MPI_UNSIGNED_SHORT;}

/// Specialization for short int
template <>
inline MPI_Datatype mpi_datatype<int>()
{return MPI_INT;}

/// Specialization for unsigned int
template <>
inline MPI_Datatype mpi_datatype<unsigned int>()
{return MPI_UNSIGNED;}

/// Specialization for long int
template <>
inline MPI_Datatype mpi_datatype<long>()
{return MPI_LONG;}

/// Specialization for unsigned long int
template <>
inline MPI_Datatype mpi_datatype<unsigned long>()
{return MPI_UNSIGNED_LONG;}

/// Specialization for float
template <>
inline MPI_Datatype mpi_datatype<float>()
{return MPI_FLOAT;}

/// Specialization for double
template <>
inline MPI_Datatype mpi_datatype<double>()
{return MPI_DOUBLE;}

/// Specialization for long double
template <>
inline MPI_Datatype mpi_datatype<long double>()
{return MPI_LONG_DOUBLE;}

/// Specialization for long long int
template <>
inline MPI_Datatype mpi_datatype<long long int>()
{return MPI_LONG_LONG_INT;}

#ifdef MPI_UNSIGNED_LONG_LONG
/// Specialization for long long unsigned int
template <>
inline MPI_Datatype mpi_datatype<unsigned long long int>()
{return MPI_UNSIGNED_LONG_LONG;}
#else
template <>
inline MPI_Datatype mpi_datatype<unsigned long long int>()
{return MPI_UNSIGNED_LONG;}
#endif


/// Return the MPI_Pack_size of a given data type
template <class TYPE>
inline int PackSize(const TYPE& data, const int num=1)
{
int size;
MPI_Pack_size(num, mpi_datatype<TYPE>(), MPI_COMM_WORLD, &size);
return size;
}
#endif

#endif
#endif
