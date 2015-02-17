/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    2opyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "BinaryIO_Helper.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <iostream>

using namespace boost::assign; // WJB: OK here in tester, but remove in Hlpr.hpp
using namespace Dakota;

//#define RANK 2

namespace Dakota {
  short H5VariableString::numVStrUses = 0;
  //hid_t H5VariableString::varStringType = -1;
  hid_t H5VariableString::varStringType = H5VariableString::datatype();
}

template <typename T>
void test_write_read_native_val(const std::string& data_label, T val_out,
                                const std::string& file_name="")
{
  herr_t status;

  if ( !file_name.empty() )
  {
  // WJB: begin -- just a test, BUT SHOULD have proper indentation

  // scope within which file write takes place
  {
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = true;

    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    status = binary_file.store_data_scalar(data_label, val_out);
    //status = 1;
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = false;
    T val_in = 0;

    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    // WJB: see hack (hdr file) to make a single val look like a vec of len==1
    status = binary_file.read_data(data_label, val_in);
    assert(status >= 0);
    assert(val_in == val_out);
  }
  // WJB: end -- just a test, BUT SHOULD have proper indentation
  }

  else {
    // scope within which DB write takes place
    {
      // open HDF5 DB - FIRST test of DEFAULT constructor with default args
      HDF5BinaryStream incore_db;

      status = incore_db.store_data_scalar(data_label, val_out);
      assert(status >= 0);

      // binary stream goes out of scope... (file close)
    }
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
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = true;

    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    status = binary_file.store_data(data_label, vec_out);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    bool db_is_incore = false;
    bool file_exist = true;
    bool write_file = false;
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

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
  bool db_is_incore = false;
  bool file_exist = true;
  bool write_file = true;
  herr_t status;

#if 0
    std::string ja_str_out("AnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTestAnotherStringToGiveTheNewTypeMoreOfAstressTest_PlusGiveTheNewTypeMoreOfAstressTest");
#endif
  std::string tst_str("StringsForDakotaResultsShouldBeVariableLen");

  // scope within which file write takes place
  {
    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    status = binary_file.store_data("/StdString", tst_str);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    write_file = false;
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    std::string tst_str_in;
    status = binary_file.read_data("/StdString", tst_str_in);
    assert(status >= 0);

    //std::cout << "WJB-verify string data: " << tst_str_in.c_str() <<std::endl;
    assert(tst_str_in == tst_str);

    // binary stream goes out of scope... (file close)
  }
}


template <typename T>
void test_write_read_vlvec_array(const std::string& data_label, T seed_val,
                                 const std::string& file_name="")
{
  using std::vector;
  bool db_is_incore = false;
  bool file_exist = true;
  bool write_file = true;
  herr_t status;

  vector< vector<T> > vlvec_array_out;
  vector<T> wdat0(4);
  vector<T> wdat1, wdat3;
  vector<T> wdat2(3);

  for (size_t i=0; i<wdat0.size(); ++i)
    wdat0[i] = wdat0.size() - i; // 4 3 2 1

  for (size_t i=0; i<wdat2.size(); ++i)
    wdat2[i] = wdat2.size() - i; // 3 2 1

  wdat1 += seed_val,seed_val;
  for (size_t i=2; i<6; ++i) {
    wdat1.push_back(wdat1[i-1] + wdat1[i-2]); // 1 1 2 3 5 8 etc.
  }

  wdat3 += seed_val,seed_val;
  for (size_t i=2; i<8; ++i) {
    wdat3.push_back(wdat3[i-1] + wdat3[i-2]); // 1 1 2 3 5 8 13 21 etc.
  }

  vlvec_array_out.push_back(wdat0); vlvec_array_out.push_back(wdat1); 
  vlvec_array_out.push_back(wdat2); vlvec_array_out.push_back(wdat3); 

  // scope within which file write takes place
  {
    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    //status = binary_file.store_vl_data(data_label, vlvec_array_out); // WJB: old approach, no hyperslabs

    // WJB - come back ASAP -- need to test array_OF_arrays to confirm proper use of HYPERSLABS status = binary_file.store_data_array(data_label, vlvec_array_out);
    binary_file.store_data_array(data_label, wdat3);
    assert(status >= 0);

    /* WJB: TEUCHOS types are the PRIORITY!! Test append to existing 2D dataset
    status = binary_file.append_data_slab(data_label, wdat2);
    assert(status >= 0);

    status = binary_file.append_data_slab(data_label, wdat1);
    assert(status >= 0);

    status = binary_file.append_data_slab(data_label, wdat0);
    assert(status >= 0);
    */
    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    /* open/read file
    file_exist = true;
    write_file = false;
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    vector< vector<T> > vlvec_array_in;
    //binary_file.read_data("/RaggedArrayData", vlvec_array_in);
    
    // WJB: consider Teuchos vector ( hopefully overloaded == )
    assert(vlvec_array_in.size() == vlvec_array_out.size());
    // for (size_t i=0; i<str_array_out.size(); ++i)
    //   assert(str_array_in[i] = str_array_out[i]);

    // binary stream goes out of scope... (file close) */
  }

}

void test_write_read_string_array(const std::string& file_name)
{
  bool db_is_incore = false;
  bool file_exist = true;
  bool write_file = true;
  herr_t status;

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

  // scope within which file write takes place
  {
    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    status = binary_file.store_data("/StringArrayData", str_array_out);

    assert(status >= 0);
    // binary stream goes out of scope... (file close)
  }

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    write_file = false;
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    StringArray str_array_in;
    binary_file.read_data("/StringArrayData", str_array_in);
    
    assert(str_array_in.size() == str_array_out.size());
    // for (size_t i=0; i<str_array_out.size(); ++i)
    //   assert(str_array_in[i] = str_array_out[i]);

    // binary stream goes out of scope... (file close)
  }

}


void test_write_read_array_of_teuchos_matrices(const std::string& file_name)
{
  bool db_is_incore = false;
  bool file_exist = true;
  bool write_file = true;
  herr_t status;

  RealMatrix::scalarType rval_out = 3.14159;

  RealMatrix rmatrix_out(2, 2);
  rmatrix_out(0, 0) = rval_out;
  rmatrix_out(0, 1) = 0.23;
  rmatrix_out(1, 0) = 0.23;
  rmatrix_out(1, 1) = rval_out;

  RealMatrixArray array_of_matrices_out;
  assert(array_of_matrices_out.empty() == true);

  array_of_matrices_out.push_back(rmatrix_out);

  // scope within which file write takes place
  {
    // open file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

    status = binary_file.store_data("/ArrayOfTeuchosMatrices",
               array_of_matrices_out);
    assert(status >= 0);

    // Test append to existing 3D dataset ("tweak" existing matrix a bit 1st)

    for(int i=0; i<rmatrix_out.numRows(); ++i) {
      RealMatrix::scalarType tweakVal = 2*i;
      for(int j=0; j<rmatrix_out.numCols(); ++j) {
        RealMatrix::scalarType tmp = rmatrix_out(i, j);
        rmatrix_out(i, j) = tmp + tweakVal;
      }
    }

#if 0 // WJB: temporarily disable to get jenkins builds with HDF5 going
    // rather than append, need to test insert at index
    status = binary_file.append_data_slab("/ArrayOfTeuchosMatrices",
               rmatrix_out);
    assert(status >= 0);
#endif

    // binary stream goes out of scope... (file close)
  }

}

herr_t testHDF5fileDB(const std::string& file_name)
{
  double rval_out = 3.14159;

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

  bool db_is_incore = false;
  bool file_exist = false;
  bool write_file = true;
  herr_t status;

  // WJB - ToDo: CONTINUE to split-out into functions

  // scope within which file write takes place
  {
    // open/create file
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);
    
    // RealMatrix -- WJB: come back ASAP -- try RealVectorArray instead
    status = binary_file.store_data("/RealMatrixData", rmatrix_out);
    assert(status >= 0);

    // binary stream goes out of scope... (file close)
  }

  test_write_read_array_of_teuchos_matrices(file_name);

  test_write_read_native_val("/DakPi", 3.14159, file_name);
  // WJB: get ALL numerial vector and matrix types working priot to getting fancy with template specialization and SCALAR, and vectors of primitive types -- test_write_read_std_vec(file_name, "/SomeIntVectorData", 314159);

  test_write_read_vlvec_array("/RaggedIntArrayData",   1, file_name);
  test_write_read_vlvec_array("/RaggedRealArrayData", 1., file_name);

  test_write_read_string(file_name);
  test_write_read_string_array(file_name);


  // WJB - ToDo: split out into functions

  // scope within which file read takes place
  {
    // open/read file
    file_exist = true;
    write_file = false;
    HDF5BinaryStream binary_file(file_name, db_is_incore, file_exist,
                                 write_file);

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


#if 0
// WJB: ASAP -- more granular functions for code sharing (incore vs file_stream)
herr_t testHDF5legacyRMatrixDBstorage(bool db_is_incore)
{
  bool file_exist = false;
  bool write_file = false;
  herr_t status;

  return status;
}
#endif

void testHDF5incoreDB()
{
  // Need:  more granular functions for code sharing (testHDF5legacy... )
  /* bool db_is_incore = true;
  bool file_exist = false;
  bool write_file = true;
  herr_t status; */

  test_write_read_native_val("/DakPi", 3.14159);
  //return status;
}


herr_t testHDF5_DB(const std::string& file_name="")
{
  herr_t status;

  if ( !file_name.empty() ) {
    // Legacy, fileStream test of HDF5 IO
    status = testHDF5fileDB(file_name);
  }
  else {
    // No arguments to test function implies in-core test coverage
    //status = testHDF5incoreDB();
    testHDF5incoreDB();
  }

  return status;
}


int main()
{
  herr_t status;

  status = testHDF5_DB("binary_io_test.h5");

  // No arguments to the test function implies default, i.e. test in-core HDF5
  // NOTE (1/8/14):  New default parameters for IO helper constructor means
  // that the HDF5 DB will NOT be persisted to a file when the program ends.
  // 
  // In order to get the in-core DB persisted to a file, there is now a need
  // to override the default parameters to the HDF5BinaryStream constructor.
  // 
  //status = testHDF5_DB();
  testHDF5_DB();

  // ToDo:  test vector of DBs

  return status;
}

