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

#include <utilib/std_headers.h>
#include <utilib/SparseMatrix.h>
#include <utilib/PackBuf.h>

using namespace utilib;

int test_matrix(int,char**)
{
int itmp;
double dtmp;

{
CMSparseMatrix<double> mat;
itmp=0;
dtmp=1.0;
mat.adjoinColumn(1,&itmp,&dtmp);
itmp=1;
dtmp=2.0;
mat.adjoinColumn(1,&itmp,&dtmp);
itmp=2;
dtmp=3.0;
mat.adjoinColumn(1,&itmp,&dtmp);
std::cout << mat << std::endl;
}

{
CMSparseMatrix<double> mat;
itmp=2;
dtmp=1.0;
mat.adjoinColumn(1,&itmp,&dtmp);
itmp=0;
dtmp=2.0;
mat.adjoinColumn(1,&itmp,&dtmp);
itmp=1;
dtmp=3.0;
mat.adjoinColumn(1,&itmp,&dtmp);
std::cout << mat << std::endl;

CMSparseMatrix<double> newmat;
PackBuffer pack;
pack << mat;

UnPackBuffer unpack(pack);
unpack >> newmat;
std::cout << newmat << std::endl;
}

{
RMSparseMatrix<double> mat;
itmp=0;
dtmp=1.0;
mat.adjoinRow(1,&itmp,&dtmp);
itmp=1;
dtmp=2.0;
mat.adjoinRow(1,&itmp,&dtmp);
itmp=2;
dtmp=3.0;
mat.adjoinRow(1,&itmp,&dtmp);
std::cout << mat << std::endl;
}

{
RMSparseMatrix<double> mat;
itmp=2;
dtmp=1.0;
mat.adjoinRow(1,&itmp,&dtmp);
itmp=0;
dtmp=2.0;
mat.adjoinRow(1,&itmp,&dtmp);
itmp=1;
dtmp=3.0;
mat.adjoinRow(1,&itmp,&dtmp);
std::cout << mat << std::endl;

RMSparseMatrix<double> newmat;
PackBuffer pack;
pack << mat;

UnPackBuffer unpack(pack);
unpack >> newmat;
std::cout << newmat << std::endl;
}

return 0;
}
