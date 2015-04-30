/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */


#include <acro_config.h>
#include <colin/OptPointDB.h>

using namespace std;

void exit_fn() {}

int main()
{
   utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
   utilib::exception_mngr::set_exit_function(&exit_fn);

   colin::db::OptPointDB<utilib::BasicArray<double> > foo;

   utilib::BasicArray<double> tmp(3);
   for (size_type i = 0; i < tmp.size(); i++)
      tmp[i] = i + 1.0;

   for (unsigned int k = 0; k < 40; k++)
   {
      for (size_type i = 0; i < tmp.size(); i++)
         tmp[i] /= 10.0;
      colin::real tval = k;
      foo.add(tmp, tval, 1);
   }

   foo.write(std::cout);

   return 0;
}
