/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "BinaryIO_Helper.hpp"

#include <boost/assign/std/vector.hpp>

#include <cassert>
#include <iostream>

using namespace boost::assign; // WJB: OK here in tester, but remove in Hlpr.hpp
using namespace Dakota;

//#define RANK 2

template <typename T>
void test_write_read_native_val(const std::string& file_name,
                                const std::string& data_label,
                                T val_out)
{
  herr_t status;

  // scope within which file write takes place
  {
    bool file_exist = true;
    bool read_only = false;

    // open file
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    status = binary_file.store_data(data_label, val_out);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    bool file_exist = true;
    bool read_only = true;
    T val_in = 0;

    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    // WJB: see hack (hdr file) to make a single val look like a vec of len==1
    status = binary_file.read_data(data_label, val_in);
    assert(status >= 0);
    assert(val_in == val_out);
  }
}


template <typename T>
void test_write_read_std_vec(const std::string& file_name,
                             const std::string& data_label,
                             T val_out)
{
  std::vector<T> vec_out;
  vec_out += val_out, 23, 333;

  herr_t status;

  assert(vec_out.size() == 3);
  assert(val_out == vec_out[0]);

  // scope within which file write takes place
  {
    bool file_exist = true;
    bool read_only = false;

    // open file
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    status = binary_file.store_data(data_label, vec_out);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    bool file_exist = true;
    bool read_only = true;
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

#if 0
    // WJB: enabling read of more sophisticated data types suggests a lack of
    //      code quality, although the h5dumps appear to be "sane"
    std::vector<int> ivec_in; // recall use of templates for built-in types
    status = binary_file.read_data<int, 1>(data_label, ivec_in);
    assert(status >= 0);

    assert( ivec_in[0]     == ivec_out[0] );
    assert( ivec_in.size() == ivec_out.size() );
    assert( ivec_in[ivec_out.size()-1] == ivec_out[ivec_out.size()-1] );
#endif
  }
}


void test_write_read_string(const std::string& file_name)
{
  bool file_exist = true;
  bool read_only = false;
  herr_t status;

  // scope within which file write takes place
  {
    // open file
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    // std::string
    // Currently limited to an array "type", derived from H5T_C_S1, max_len=128
#if 0
    std::string ja_str_out("AnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTestAnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTest");
#endif
    std::string tst_str("StringsUsedInDakotaResultsDataCanBeUpTo128");
    status = binary_file.store_data("/StdString_128chars", tst_str);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    read_only = true;
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    std::string tst_str_in;
    status = binary_file.read_data("/StdString_128chars", tst_str_in);
    assert(status >= 0);
    assert( tst_str_in.capacity() == DerivedStringType128::length() );
    assert( tst_str_in.size() <= DerivedStringType128::length() );

    //std::cout << "WJB-verify string data: " << tst_str_in.c_str() <<std::endl;
    std::string tst_str("S");
    assert( tst_str_in[0] == tst_str[0] );

    // binary stream goes out of scope... (file close)
  }
}


void test_write_read_string_array(const std::string& file_name)
{
  bool file_exist = true;
  bool read_only = false;
  herr_t status;

  // scope within which file write takes place
  {
    // open file
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    // std::string
    // Currently limited to an array "type", derived from H5T_C_S1, max_len=128
#if 0
    std::string ja_str_out("AnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTestAnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTest");
#endif
    std::string ja_str_out("Taking_Four_Score_to_WriteRead_HDF5");

    StringArray str_array_out;
    assert(str_array_out.empty() == true);

    str_array_out.push_back(ja_str_out);
    str_array_out.push_back(file_name);
    str_array_out.push_back(ja_str_out);
    str_array_out.push_back(file_name);

    status = binary_file.store_data("/StringArrayData", str_array_out);

    assert(status >= 0);
    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    read_only = true;
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);


    // binary stream goes out of scope... (file close)
  }

}


int main()
{
  double rval_out = 3.14159;
  std::string file_name("binary_io_test.h5");

  std::vector<double> rmatrix_row0;
  rmatrix_row0 += rval_out, .23;

  //RealMatrix rmatrix_out(rdims[0], rdims[1]);
  /* RealVectorArray rmatrix_out( rdims[0],
                               RealVector( Teuchos::View,
                                           rmatrix_row0.data(),
                                           rdims[1] ) ); */
  RealVectorArray rmatrix_out;
  rmatrix_out.push_back( RealVector( Teuchos::View, rmatrix_row0.data(), 2 ) );
  rmatrix_out.push_back( RealVector( Teuchos::View, rmatrix_row0.data(), 2 ) );
/*
  rmatrix_out(0, 0) = rval_out;
  rmatrix_out(0, 1) = 0.23;
  rmatrix_out(1, 0) = 0.23;
  rmatrix_out(1, 1) = rval_out;
*/
  rmatrix_out[1][0] = .23;
  rmatrix_out[1][1] = rval_out;

  //assert(rmatrix_out.numCols() == 2);
  assert(rmatrix_out[0].length() == 2); // numCols
  assert(rmatrix_out[1].length() == 2); // confirm square "matrix" in this test
  assert(rval_out == rmatrix_out[0](0));
  assert(rval_out == rmatrix_out[1](1));

  bool file_exist = false;
  bool read_only = false;
  herr_t status;

  // WJB - ToDo: CONTINUE to split-out into functions

  // scope within which file write takes place
  {
    // open/create file
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);
    
    // RealMatrix -- WJB: come back ASAP -- try RealVectorArray instead
    status = binary_file.store_data("/RealMatrixData", rmatrix_out);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  test_write_read_native_val(file_name, "/DakPi", 3.14159);
  test_write_read_std_vec(file_name, "/SomeIntVectorData", 314159);

  test_write_read_string(file_name);
  test_write_read_string_array(file_name);


  // WJB - ToDo: split out into functions

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    read_only = true;
    SimpleBinaryStream binary_file(file_name, file_exist, read_only);

    // read data 
  
#if 0
    // WJB: enabling read of more sophisticated data types suggests a lack of
    //      code quality, although the h5dumps appear to be "sane"
    //RealMatrix rmatrix_in;

    //herr_t status = binary_file.read_data<double, 2>("/RealMatrixData",
                      //rmatrix_in);
    //assert(status >= 0);
    //assert(rmatrix_in(0, 0) == rmatrix_out(0, 0));
#endif 
  }

  return status;
}

