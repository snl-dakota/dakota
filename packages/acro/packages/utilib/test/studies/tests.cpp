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

#include <string.h>
#include <iostream>

   //int test_any(int argc, char** argv);
   int test_sharedval(int argc, char** argv);
   //int test_any_deprecated(int argc, char** argv);
   int test_array(int argc, char** argv);
   int test_bitarray(int argc, char** argv);
   int test_calloc(int argc, char** argv);
   //int test_class_options(int argc, char** argv);
   int test_commonio(int argc, char** argv);
   int test_commonio_debug(int argc, char** argv);
   int test_ereal(int argc, char** argv);
   int test_exception_test(int argc, char** argv);
   int test_factory(int argc, char** argv);
   int test_hash(int argc, char** argv);
   int test_hashedset(int argc, char** argv);
   int test_heap(int argc, char** argv);
   int test_iotest(int argc, char** argv);
   int test_iotest_debug(int argc, char** argv);
   int test_listtest(int argc, char** argv);
   int test_lphash(int argc, char** argv);
   int test_malloc(int argc, char** argv);
   int test_math(int argc, char** argv);
   int test_matrix(int argc, char** argv);
   int test_multibitarray(int argc, char** argv);
   int test_namespace(int argc, char** argv);
   int test_olist(int argc, char** argv);
   //int test_option_parser(int argc, char** argv);
   int test_packbuf(int argc, char** argv);
   int test_paramtest(int argc, char** argv);
   int test_pvector(int argc, char** argv);
   int test_qlist(int argc, char** argv);
   int test_rng(int argc, char** argv);
   int test_smartptr(int argc, char** argv);
   int test_sort(int argc, char** argv);
   int test_splay(int argc, char** argv);
   int test_sregistry(int argc, char** argv);
   int test_stl(int argc, char** argv);
   int test_string(int argc, char** argv);
   int test_timing(int argc, char** argv);
   int test_triang(int argc, char** argv);
   int test_tuple(int argc, char** argv);
   int test_typeManager(int argc, char** argv);
   int test_vector(int argc, char** argv);
   //int test_serialize(int argc, char** argv);
   //int test_sserialstream(int argc, char** argv);
   //int test_fserialstream(int argc, char** argv);
   //int test_fserialstream_ref(int argc, char** argv);
   //int test_fxmlserialstream(int argc, char** argv);

int main(int argc, char** argv)
{
char* tmp = 0;
//
// This ignores all arguments after the first.
//
if (argc > 1) {
   tmp = argv[1];
   argv++;
   argc--;
}

int status=-1;

try {

//if (!tmp || (strcmp(tmp,"any") == 0))
   //status = test_any(argc,argv);

//if (!tmp || (strcmp(tmp,"any-deprecated") == 0))
   //status = test_any_deprecated(argc,argv);

if (!tmp || (strcmp(tmp,"array") == 0))
   status = test_array(argc,argv);

if (!tmp || (strcmp(tmp,"bitarray") == 0))
   status = test_bitarray(argc,argv);

if (!tmp || (strcmp(tmp,"calloc") == 0))
   status = test_calloc(argc,argv);

#if 0
if (!tmp || (strcmp(tmp,"class_options") == 0))
   status = test_class_options(argc,argv);
#endif

if (!tmp || (strcmp(tmp,"commonio") == 0))
   status = test_commonio(argc,argv);

if (!tmp || (strcmp(tmp,"commonio_debug") == 0))
   status = test_commonio_debug(argc,argv);

if (!tmp || (strcmp(tmp,"ereal") == 0))
   status = test_ereal(argc,argv);

if (!tmp || (strcmp(tmp,"exceptions") == 0))
   status = test_exception_test(argc,argv);

if (!tmp || (strcmp(tmp,"factory") == 0))
   status = test_factory(argc,argv);

if (!tmp || (strcmp(tmp,"hash") == 0))
   status = test_hash(argc,argv);

#if 1
if (!tmp || (strcmp(tmp,"hashedset") == 0))
   status = test_hashedset(argc,argv);
#endif

if (!tmp || (strcmp(tmp,"heap") == 0))
   status = test_heap(argc,argv);

if (!tmp || (strcmp(tmp,"iotest") == 0))
   status = test_iotest(argc,argv);

if (!tmp || (strcmp(tmp,"iotest_debug") == 0))
   status = test_iotest_debug(argc,argv);

if (!tmp || (strcmp(tmp,"listtest") == 0))
   status = test_listtest(argc,argv);

if (!tmp || (strcmp(tmp,"lphash") == 0))
   status = test_lphash(argc,argv);

if (!tmp || (strcmp(tmp,"malloc") == 0))
   status = test_malloc(argc,argv);

if (!tmp || (strcmp(tmp,"math") == 0))
   status = test_math(argc,argv);

if (!tmp || (strcmp(tmp,"matrix") == 0))
   status = test_matrix(argc,argv);

if (!tmp || (strcmp(tmp,"multibitarray") == 0))
   status = test_multibitarray(argc,argv);

if (!tmp || (strcmp(tmp,"namespace") == 0))
   status = test_namespace(argc,argv);

if (!tmp || (strcmp(tmp,"olist") == 0))
   status = test_olist(argc,argv);

#if 0
if (!tmp || (strcmp(tmp,"option_parser") == 0))
   status = test_option_parser(argc,argv);
#endif

if (!tmp || (strcmp(tmp,"packbuf") == 0))
   status = test_packbuf(argc,argv);

if (!tmp || (strcmp(tmp,"paramTest") == 0))
   status = test_paramtest(argc,argv);

if (!tmp || (strcmp(tmp,"pvector") == 0))
   status = test_pvector(argc,argv);

if (!tmp || (strcmp(tmp,"qlist") == 0))
   status = test_qlist(argc,argv);

if (!tmp || (strcmp(tmp,"rng") == 0))
   status = test_rng(argc,argv);

#if 1
if (!tmp || (strcmp(tmp,"smartptr") == 0))
   status = test_smartptr(argc,argv);
#endif

if (!tmp || (strcmp(tmp,"sharedval") == 0))
   status = test_sharedval(argc,argv);

if (!tmp || (strcmp(tmp,"sort") == 0))
   status = test_sort(argc,argv);

if (!tmp || (strcmp(tmp,"splay") == 0))
   status = test_splay(argc,argv);

if (!tmp || (strcmp(tmp,"sregistry") == 0))
   status = test_sregistry(argc,argv);

if (!tmp || (strcmp(tmp,"stl") == 0))
   status = test_stl(argc,argv);

if (!tmp || (strcmp(tmp,"string") == 0))
   status = test_string(argc,argv);

if (!tmp || (strcmp(tmp,"timing") == 0))
   status = test_timing(argc,argv);

if (!tmp || (strcmp(tmp,"triang") == 0))
   status = test_triang(argc,argv);

if (!tmp || (strcmp(tmp,"tuple") == 0))
   status = test_tuple(argc,argv);

if (!tmp || (strcmp(tmp,"typeManager") == 0))
   status = test_typeManager(argc,argv);

if (!tmp || (strcmp(tmp,"vector") == 0))
   status = test_vector(argc,argv);

//if (!tmp || (strcmp(tmp,"serialize") == 0))
   //status = test_serialize(argc,argv);

//if (!tmp || (strcmp(tmp,"sserialstream") == 0))
   //status = test_sserialstream(argc,argv);

//if (!tmp || (strcmp(tmp,"fserialstream") == 0))
   //status = test_fserialstream(argc,argv);

//if (!tmp || (strcmp(tmp,"fserialstream-ref") == 0))
   //status = test_fserialstream_ref(argc,argv);

//if (!tmp || (strcmp(tmp,"fxmlserialstream") == 0))
   //status = test_fxmlserialstream(argc,argv);

if (status == -1)
   std::cout << "ERROR: unknown test \"" << tmp << "\"" << std::endl;

} // end of main try block...
catch ( std::exception &e )
   { std::cout << "Caught std::exception: " << e.what() << std::endl; }
catch ( ... )
   { std::cout << "Caught unknown exception." << std::endl; }   

return status;
}

