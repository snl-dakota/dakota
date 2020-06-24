/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_EIGEN_PLUGINS_HPP
#define DAKOTA_UTIL_EIGEN_PLUGINS_HPP

friend class boost::serialization::access;
template<class Archive>
void serialize(Archive& ar, const unsigned int version) {

  Eigen::Index rows = derived().rows();
  Eigen::Index cols = derived().cols();
  ar & rows;
  ar & cols;
  if(rows != derived().rows() || cols != derived().cols())
    derived().resize(rows, cols);
  if(derived().size() !=0)
    ar &  boost::serialization::make_array(derived().data(), rows * cols);

}

#endif
