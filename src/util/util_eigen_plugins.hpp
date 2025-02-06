/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_EIGEN_PLUGINS_HPP
#define DAKOTA_UTIL_EIGEN_PLUGINS_HPP

/// Allow serializers access to private class data
friend class boost::serialization::access;
template <class Archive>
/**
 * \brief Serialize an Eigen matrix.
 * \param[in] ar Archive of serialized data.
 * \param[in] version version number (unused).
 */
void serialize(Archive& ar, const unsigned int version) {
  Eigen::Index rows = derived().rows();
  Eigen::Index cols = derived().cols();
  ar& rows;
  ar& cols;
  if (rows != derived().rows() || cols != derived().cols())
    derived().resize(rows, cols);
  if (derived().size() != 0)
    ar& boost::serialization::make_array(derived().data(), rows * cols);
}

#endif
