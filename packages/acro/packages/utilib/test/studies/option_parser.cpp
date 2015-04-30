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

#if 0
//
// ParameterSet test
//

#include <utilib/std_headers.h>
#include <utilib/stl_auxiliary.h>
#include <utilib/OptionParser.h>
#include <utilib/mpiUtil.h>

#ifdef UTILIB_HAVE_NAMESPACES
using namespace std;
using namespace utilib;
#endif

void test1()
{
bool a=false;
int b=-1;
double c=-1.0;
unsigned int d=1;

OptionParser options;
options.add_usage("command options arguments");
options.description="Description here.";
options.version("beta");
options.add("A",a,"A option");
options.add("B",b,"B option");
options.add("C",c,"C option");
options.add("D",d,"D option");
options.add_argument("arg1","This is an arg description.");

options.write(std::cout);

std::cout << a << " " << b << " " << c << " " << d << std::endl;

int argc = 8;
char* argv[] = { "command", "--A", "--B","0", "--C","2.0", "--D", "2", 0};
options.parse_args(argc, argv);

std::cout << a << " " << b << " " << c << " " << d << std::endl;
}



void test2()
{
bool a=false;
int b=-1;
double c=-1.0;
unsigned int d=1;

OptionParser options;
options.add_usage("command options arguments");
options.description="Description here.";
options.version("beta");
options.add('A',a,"A option");
options.add('B',b,"B option");
options.add('C',c,"C option");
options.add('D',d,"D option");
options.add_argument("arg1","This is an arg description.");

options.write(std::cout);

std::cout << a << " " << b << " " << c << " " << d << std::endl;

int argc = 8;
char* argv[] = { "command", "-A", "-B","0", "-C","2.0", "-D", "2", 0};
options.parse_args(argc, argv);

std::cout << a << " " << b << " " << c << " " << d << std::endl;
}


void test3()
{
bool a=false;
int b=-1;
double c=-1.0;
unsigned int d=1;

OptionParser options;
options.add_usage("command options arguments");
options.description="Description here.";
options.version("beta");
options.add('A',a,"A option");
options.add('B',"my-b",b,"B option");
options.add("my-c",c,"C option");
options.add('D',d,"D option");
options.add_argument("arg1","This is an arg description.");

options.write(std::cout);

std::cout << a << " " << b << " " << c << " " << d << std::endl;

int argc = 8;
char* argv[] = { "command", "-A", "--my-b","0", "--my-c","2.0", "-D", "2", 0};
options.parse_args(argc, argv);

std::cout << a << " " << b << " " << c << " " << d << std::endl;
}


void test4()
{
bool a=false;
int b=-1;
double c=-1.0;
unsigned int d=1;

OptionParser options;
options.add_usage("command options arguments");
options.description="Description here.";
options.add('A',a,"A option");
options.add('B',"my-b",b,"B option");
options.add("my-ccccccccccccccccccc",c,"C option");
options.add("my-dddddddddddddddddddd",c,"D option");
options.add("my-eeeeeeeeeeeeeeeeee",c,"E option");
options.add("my-feeeeeeeeeeeeeeeee",c,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
options.add("my-geeeeeeeeeeeeeeeee",c,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae            ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
options.add("my-heeeeeeeeeeeeeeeee",c,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \nab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
options.add("my-ieeeeeeeeeeeeeeeee",c,"Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae             \n - ab illo\n - inventore \n - veritatis et quasi architecto \n - beatae vitae dicta\n sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?");
options.add('D',d,"D option");
options.add_argument("arg1","This is an arg description.");

options.write(std::cout);
}


int test_option_parser(int argc, char** argv)
{
#ifdef UTILIB_HAVE_MPI
	uMPI::init(&argc,&argv);
#endif

utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);

test1();
test2();
test3();
test4();

utilib::exception_mngr::set_mode(utilib::exception_mngr::Standard);
return 0;
}
#endif
