/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_BOOST_MERSENNE_TWISTER_HPP
#define DAKOTA_BOOST_MERSENNE_TWISTER_HPP

/** \file Portability header for deprecated Boost integer_log2.hpp
    header used in Boost 1.69 random library. To be removed once we
    migrate to std library RNG. */

#include <boost/version.hpp>
#if (BOOST_VERSION < 107000) && !defined(BOOST_ALLOW_DEPRECATED_HEADERS)
//could alternately use: #define BOOST_PENDING_INTEGER_LOG2_HPP 1
#define BOOST_ALLOW_DEPRECATED_HEADERS 1
#include <boost/random/mersenne_twister.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS
#else
#include <boost/random/mersenne_twister.hpp>
#endif

#endif  // include guard
