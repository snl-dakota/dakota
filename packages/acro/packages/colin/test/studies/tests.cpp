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

#include <string.h>
#include <iostream>
#include <utilib/TypeManager.h>

int test_test1a(int argc, char** argv);
int test_test1b(int argc, char** argv);
int test_test1c(int argc, char** argv);
int test_test1d(int argc, char** argv);
int test_test1e(int argc, char** argv);
int test_test1f(int argc, char** argv);

int test_test2(int argc, char** argv);
int test_test3(int argc, char** argv);
int test_test4(int argc, char** argv);

int test_test5a(int argc, char** argv);
int test_test5b(int argc, char** argv);
int test_test5c(int argc, char** argv);
int test_test5d(int argc, char** argv);

int test_test6a(int argc, char** argv);
int test_test6b(int argc, char** argv);
int test_test6c(int argc, char** argv);
int test_test6d(int argc, char** argv);

int test_test7a(int argc, char** argv);
int test_test7b(int argc, char** argv);
int test_test7c(int argc, char** argv);
int test_test7d(int argc, char** argv);

int test_test8a(int argc, char** argv);
int test_test8b(int argc, char** argv);
int test_test8c(int argc, char** argv);
int test_test8d(int argc, char** argv);

int test_test9a(int argc, char** argv);
int test_test9b(int argc, char** argv);
int test_test9c(int argc, char** argv);
int test_test9d(int argc, char** argv);
int test_test9e(int argc, char** argv);
int test_test9f(int argc, char** argv);

int test_test20a(int argc, char** argv);
int test_test20b(int argc, char** argv);
int test_test20c(int argc, char** argv);
int test_test20d(int argc, char** argv);
int test_test20e(int argc, char** argv);
int test_test20f(int argc, char** argv);

int test_test21a(int argc, char** argv);
int test_test21b(int argc, char** argv);
int test_test21c(int argc, char** argv);
int test_test21d(int argc, char** argv);

int main(int argc, char** argv)
{
   char* tmp = 0;
//
// This ignores all arguments after the first.
//
   if (argc > 1)
   {
      tmp = argv[1];
      argv++;
      argc--;
   }

   int status = -1;

   try
   {

      if (!tmp || (strcmp(tmp, "test1a") == 0))
         status = test_test1a(argc, argv);
      if (!tmp || (strcmp(tmp, "test1b") == 0))
         status = test_test1b(argc, argv);
      if (!tmp || (strcmp(tmp, "test1c") == 0))
         status = test_test1c(argc, argv);
      if (!tmp || (strcmp(tmp, "test1d") == 0))
         status = test_test1d(argc, argv);
      if (!tmp || (strcmp(tmp, "test1e") == 0))
         status = test_test1e(argc, argv);
      if (!tmp || (strcmp(tmp, "test1f") == 0))
         status = test_test1f(argc, argv);

      if (!tmp || (strcmp(tmp, "test2") == 0))
         status = test_test2(argc, argv);

      if (!tmp || (strcmp(tmp, "test3") == 0))
         status = test_test3(argc, argv);

      if (!tmp || (strcmp(tmp, "test4") == 0))
         status = test_test4(argc, argv);

      if (!tmp || (strcmp(tmp, "test5a") == 0))
         status = test_test5a(argc, argv);
      if (!tmp || (strcmp(tmp, "test5b") == 0))
         status = test_test5b(argc, argv);
      if (!tmp || (strcmp(tmp, "test5c") == 0))
         status = test_test5c(argc, argv);
      if (!tmp || (strcmp(tmp, "test5d") == 0))
         status = test_test5d(argc, argv);

      if (!tmp || (strcmp(tmp, "test6a") == 0))
         status = test_test6a(argc, argv);
      if (!tmp || (strcmp(tmp, "test6b") == 0))
         status = test_test6b(argc, argv);
      if (!tmp || (strcmp(tmp, "test6c") == 0))
         status = test_test6c(argc, argv);
      if (!tmp || (strcmp(tmp, "test6d") == 0))
         status = test_test6d(argc, argv);

      if (!tmp || (strcmp(tmp, "test7a") == 0))
         status = test_test7a(argc, argv);
      if (!tmp || (strcmp(tmp, "test7b") == 0))
         status = test_test7b(argc, argv);
      if (!tmp || (strcmp(tmp, "test7c") == 0))
         status = test_test7c(argc, argv);
      if (!tmp || (strcmp(tmp, "test7d") == 0))
         status = test_test7d(argc, argv);

      if (!tmp || (strcmp(tmp, "test8a") == 0))
         status = test_test8a(argc, argv);
      if (!tmp || (strcmp(tmp, "test8b") == 0))
         status = test_test8b(argc, argv);
      if (!tmp || (strcmp(tmp, "test8c") == 0))
         status = test_test8c(argc, argv);
      if (!tmp || (strcmp(tmp, "test8d") == 0))
         status = test_test8d(argc, argv);

      if (!tmp || (strcmp(tmp, "test9a") == 0))
         status = test_test9a(argc, argv);
      if (!tmp || (strcmp(tmp, "test9b") == 0))
         status = test_test9b(argc, argv);
      if (!tmp || (strcmp(tmp, "test9c") == 0))
         status = test_test9c(argc, argv);
      if (!tmp || (strcmp(tmp, "test9d") == 0))
         status = test_test9d(argc, argv);
      if (!tmp || (strcmp(tmp, "test9e") == 0))
         status = test_test9e(argc, argv);
      if (!tmp || (strcmp(tmp, "test9f") == 0))
         status = test_test9f(argc, argv);

      if (!tmp || (strcmp(tmp, "test20a") == 0))
         status = test_test20a(argc, argv);
      if (!tmp || (strcmp(tmp, "test20b") == 0))
         status = test_test20b(argc, argv);
      if (!tmp || (strcmp(tmp, "test20c") == 0))
         status = test_test20c(argc, argv);
      if (!tmp || (strcmp(tmp, "test20d") == 0))
         status = test_test20d(argc, argv);
      if (!tmp || (strcmp(tmp, "test20e") == 0))
         status = test_test20e(argc, argv);
      if (!tmp || (strcmp(tmp, "test20f") == 0))
         status = test_test20f(argc, argv);

      if (!tmp || (strcmp(tmp, "test21a") == 0))
         status = test_test21a(argc, argv);
      if (!tmp || (strcmp(tmp, "test21b") == 0))
         status = test_test21b(argc, argv);
      if (!tmp || (strcmp(tmp, "test21c") == 0))
         status = test_test21c(argc, argv);
      if (!tmp || (strcmp(tmp, "test21d") == 0))
         status = test_test21d(argc, argv);

      if (status == -1)
         std::cerr << "ERROR: unknown test \"" << tmp << "\"" << std::endl;
   }
#if 0
   catch (utilib::bad_context_cast& err)
   {
      std::cerr << "Caught exception utilib::bad_context_cast: " <<
      err.what() << std::endl;
   }
#endif
   catch (utilib::bad_lexical_cast& err)
   {
      std::cerr << "Caught exception utilib::bad_lexical_cast: " <<
      err.what() << std::endl;
   }
   catch (utilib::typeManager_error& err)
   {
      std::cerr << "Caught exception utilib::typeManager_error: " <<
      err.what() << std::endl;
   }
   STD_CATCH()

   return status;
}

