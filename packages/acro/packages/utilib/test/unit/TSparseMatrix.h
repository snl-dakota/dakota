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

#include <utilib/SparseMatrix.h>
#include <utilib/BasicArray.h>
#include <utilib/TypeManager.h>

#include <cxxtest/TestSuite.h>

#include "CommonTestUtils.h"

namespace utilib {

namespace unittest { class Test_RMSparseMatrix; }
class utilib::unittest::Test_RMSparseMatrix : public CxxTest::TestSuite
{
public:

    ///
    void test_constructor1()
    {
    utilib::RMSparseMatrix<double> a;
    TS_ASSERT_EQUALS(a.get_nrows(),0);
    TS_ASSERT_EQUALS(a.get_ncols(),0);
    TS_ASSERT_EQUALS(a.get_nnzero(),0);
    }

    ///
    void test_initialize()
    {
    utilib::RMSparseMatrix<double> a;
    a.initialize(3,3,9);
    TS_ASSERT_EQUALS(a.get_nrows(),3);
    TS_ASSERT_EQUALS(a.get_ncols(),3);
    TS_ASSERT_EQUALS(a.get_nnzero(),9);
    }

    ///
    void test_lexical_cast1()
    {
    utilib::RMSparseMatrix<double> a;
    utilib::BasicArray< utilib::BasicArray<double> > mat;
    mat.resize(4);
    for (size_t i=0; i<4; i++) {
        mat[i].resize(5);
        for (size_t j=0; j<5; j++)
            mat[i][j] = (i+j) % 3;
    }
    //std::cout << "BBd " << mat << std::endl;
    utilib::TypeManager()->lexical_cast(mat,a);
    //std::cerr << "RM " << a << std::endl;

    utilib::BasicArray< utilib::BasicArray<double> > res;
    utilib::TypeManager()->lexical_cast(a,res);

    //std::cout << "RMd " << res << std::endl;
    TS_ASSERT_EQUALS(mat, res);
    }


   void test_row_delete()
   {
      std::vector< std::vector<double> > src;
      std::vector< std::vector<double> > ref;
      std::vector< std::vector<double> > ans;
      src.resize(4);
      for (size_t i=0; i<4; i++) 
      {
         src[i].resize(5);
         for (size_t j=0; j<5; j++)
            src[i][j] = (i*10+j);
      }
      
      // single row deletion
      utilib::RMSparseMatrix<double> a;
      utilib::TypeManager()->lexical_cast(src,a);
      //std::cerr << "RM " << a << std::endl;
      a.delete_row(1);
      //std::cerr << "RM " << a << std::endl;
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      ref.erase(ref.begin()+1);
      TS_ASSERT_EQUALS(ref, ans);

      // multiple internal row deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_rows(1, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      ref.erase(ref.begin()+1);
      ref.erase(ref.begin()+1);
      TS_ASSERT_EQUALS(ref, ans);
      
      // multiple leading row deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_rows(0, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      ref.erase(ref.begin());
      ref.erase(ref.begin());
      TS_ASSERT_EQUALS(ref, ans);

      // multiple trailing row deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_rows(2, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      ref.erase(ref.begin()+2);
      ref.erase(ref.begin()+2);
      TS_ASSERT_EQUALS(ref, ans);

      // Invalid range
      utilib::TypeManager()->lexical_cast(src,a);
      TS_ASSERT_THROWS_ASSERT
         ( a.delete_rows(2, 3), std::runtime_error &e,
           TEST_WHAT( e, "RMSparseMatrix<T>::delete_rows(): "
                      "row range (2 to 5) extends past end of matrix (4)" ) );
   } 

};


namespace unittest { class Test_CMSparseMatrix; }
class utilib::unittest::Test_CMSparseMatrix : public CxxTest::TestSuite
{
public:

    ///
    void test_constructor1()
    {
    utilib::CMSparseMatrix<double> a;
    TS_ASSERT_EQUALS(a.get_nrows(),0);
    TS_ASSERT_EQUALS(a.get_ncols(),0);
    TS_ASSERT_EQUALS(a.get_nnzero(),0);
    }

    ///
    void test_initialize()
    {
    utilib::CMSparseMatrix<double> a;
    a.initialize(3,3,9);
    TS_ASSERT_EQUALS(a.get_nrows(),3);
    TS_ASSERT_EQUALS(a.get_ncols(),3);
    TS_ASSERT_EQUALS(a.get_nnzero(),9);
    }

    ///
    void test_lexical_cast1()
    {
    utilib::CMSparseMatrix<double> a;
    utilib::BasicArray< utilib::BasicArray<double> > mat;
    mat.resize(4);
    for (size_t i=0; i<4; i++) {
        mat[i].resize(5);
        for (size_t j=0; j<5; j++)
            mat[i][j] = (i+j) % 3;
    }
    //std::cout << "BBd " << mat << std::endl;
    utilib::TypeManager()->lexical_cast(mat,a);
    //std::cout << "CM " << a << std::endl;

    utilib::BasicArray< utilib::BasicArray<double> > res;
    utilib::TypeManager()->lexical_cast(a,res);

    //std::cout << "CMd " << res << std::endl;
    TS_ASSERT_EQUALS(mat, res);
    }

   void test_row_delete()
   {
      std::vector< std::vector<double> > src;
      std::vector< std::vector<double> > ref;
      std::vector< std::vector<double> > ans;
      src.resize(4);
      for (size_t i=0; i<4; i++) 
      {
         src[i].resize(5);
         for (size_t j=0; j<5; j++)
            src[i][j] = (i*10+j);
      }
      
      // single col deletion
      utilib::CMSparseMatrix<double> a;
      utilib::TypeManager()->lexical_cast(src,a);
      //std::cerr << "CM " << a << std::endl;
      a.delete_col(1);
      //std::cerr << "CM " << a << std::endl;
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      delete_ref_col(ref, 1);
      TS_ASSERT_EQUALS(ref, ans);

      // multiple internal col deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_cols(1, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      delete_ref_col(ref, 1);
      delete_ref_col(ref, 1);
      TS_ASSERT_EQUALS(ref, ans);
      
      // multiple leading col deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_cols(0, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      delete_ref_col(ref, 0);
      delete_ref_col(ref, 0);
      TS_ASSERT_EQUALS(ref, ans);

      // multiple trailing col deletion
      utilib::TypeManager()->lexical_cast(src,a);
      a.delete_cols(3, 2);
      utilib::TypeManager()->lexical_cast(a,ans);

      ref = src;
      delete_ref_col(ref, 3);
      delete_ref_col(ref, 3);
      TS_ASSERT_EQUALS(ref, ans);

      // Invalid range
      utilib::TypeManager()->lexical_cast(src,a);
      TS_ASSERT_THROWS_ASSERT
         ( a.delete_cols(2, 4), std::runtime_error &e,
           TEST_WHAT( e, "CMSparseMatrix<T>::delete_cols(): "
                      "column range extends past end of matrix" ) );
   } 

private:
   void delete_ref_col(std::vector<std::vector<double> > &ref, size_t col)
   {
      for( size_t i = 0; i < ref.size(); ++i )
         ref[i].erase(ref[i].begin() + col);
   }
};

} // namespace utilib
