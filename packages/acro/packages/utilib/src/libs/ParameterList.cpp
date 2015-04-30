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

#if 1
#include <utilib/ParameterSet.h>
#include <utilib/ParameterList.h>
#include <utilib/comments.h>

using namespace std;

namespace utilib {

  void ParameterList::add_posix_parameters(const std::string& label, 
				    const std::string& value,
				    bool process_param)
  {
  std::string::const_iterator curr = label.begin();
  std::string::const_iterator end  = label.end();
  while (curr != end) {
    std::string tmp;
    tmp = *curr;
    add_parameter(tmp,value,process_param);
    curr++;
    }
  }

  void ParameterList::add_parameter(const std::string& label, 
				    const std::string& value,
				    bool process_param)
  {
    if (process_param) {
       if ((label == "param-file") || (label == "param_file")) {
         process_parameters(value);
         return;
       }
    }
   
    if ((registered_params.size() > ParameterSet::num_default_parameters) && 
	!registered_params.parameter_exists(label))
       EXCEPTION_MNGR(runtime_error,
                      "ParameterList::add_parameter - unknown parameter \"" 
                      << label << "\"");

    tmp_param.label = label;
    tmp_param.value = value;
    parameters.push_back(tmp_param);
  }



void ParameterList::read_parameters(istream& is, const string& terminator)
{
bool eof_flag = (terminator == "");

if (!is) return;

string label,value;

is >> comment_lines;
is >> label;
while (is) {
  if (!eof_flag && (label == terminator))
     return;

  is >> value;
  if (!is)
     EXCEPTION_MNGR(runtime_error, "ParameterList::read_parameter - problem reading value of the " << label << " param");
  add_parameter(label,value);

  //
  // Get next parameter
  //
  is >> comment_lines;
  is >> label;
  }
}


#ifdef UTILIB_HAVE_MPI
  void ParameterList::synchronize(int root, MPI_Comm )
  {
    int size = -1;
    if (uMPI::rank == root)
      {
	PackBuffer pack;
	write_parameters(pack);
	size = pack.size();
	uMPI::broadcast(&size,1,MPI_INT,root);
	uMPI::broadcast((void*) pack.buf(),size,MPI_PACKED,root);
      }
    else
      {
	uMPI::broadcast(&size,1,MPI_INT,root);
	UnPackBuffer unpack(size);
	unpack.reset();
	uMPI::broadcast((void *) unpack.buf(),size,MPI_PACKED,root);
	read_parameters(unpack);
      }
  }
#endif


void ParameterList::write_parameters(ostream& os) const
{
std::list<ParameterListItem>::const_iterator curr = parameters.begin();
std::list<ParameterListItem>::const_iterator end  = parameters.end();
while (curr != end) {
  os << (*curr).label << " " << (*curr).value << endl;
  curr++;
  }
}


void ParameterList::write_parameters(PackBuffer& os) const
{
os << parameters.size();
std::list<ParameterListItem>::const_iterator curr = parameters.begin();
std::list<ParameterListItem>::const_iterator end  = parameters.end();
while (curr != end) {
  os << (*curr).label;
  os << (*curr).value;
  curr++;
  }
}


void ParameterList::read_parameters(UnPackBuffer& is)
{
size_type size;
is >> size;
string label,value;

for (size_type i=0; i<size; i++) {
  is >> label >> value;
  add_parameter( label, value );
  }
}


void ParameterList::process_parameters(const string& fname)
{
std::ifstream ifstr;
ifstr.open(fname.c_str());
if (ifstr) {
   read_parameters(ifstr);
   ifstr.close();
   }
else
   EXCEPTION_MNGR(runtime_error, "ParameterList::process_parameters - unable to open file \"" << fname.c_str() << "\"");
}


void ParameterList::process_parameters(int& argc, char**& argv, 
				       unsigned int min_num_required_args)
{
  //
  // Go through the argument list
  //
  int i=1;
  while ((i < (argc-(int)min_num_required_args)) && (argv[i][0] == '-')) {
    //
    // Set tmp to the value of the argument
    //
    char* tmp = strchr(argv[i],'=');
    if (tmp) tmp++;
    if (tmp && (*tmp != '\000')) {
      *(tmp-1) = '\000';
      if (argv[i][1] == '-')
         add_parameter( &(argv[i][2]), tmp);
      else
         add_posix_parameters( &(argv[i][1]), tmp);
      i += 1;
    }
    else if (tmp && (*tmp == '\000')) {
      *(tmp-1) = '\000';
      if (argv[i][1] == '-')
         add_parameter( &(argv[i][2]), "true");
      else
         add_posix_parameters( &(argv[i][1]), "true");
      i += 1;
    }
    else if ( ((i+1) == (argc-(int)min_num_required_args)) ||
	      (argv[i+1][0] == '-')) {
      //
      // Implicitly true flag
      // Example: --flush --help
      //
      if (argv[i][1] == '-')
         add_parameter( &(argv[i][2]), "true");
      else
         add_posix_parameters( &(argv[i][1]), "true");
      i++;
    }
    else {
      if (argv[i][1] == '-')
         add_parameter( &(argv[i][2]), &(argv[i+1][0]) );
      else
         add_posix_parameters( &(argv[i][1]), &(argv[i+1][0]) );
      i += 2;
    }
  }
  //
  // Ignore min_num_required_args for "--help"
  //
  if ((i < argc) && (strlen(argv[i]) == 6) && 
                    (strncmp(argv[i],"--help",6) == 0)) {
     i += 1;
     add_parameter("help","true");
     }
  //
  // Check to ensure that first 'required arg' does not look like a parameter
  //
  if ((i < argc) && (argv[i][0] == '-'))
     EXCEPTION_MNGR(runtime_error,
                    "ParameterList::process_parameters "
                    "- first required argument looks "
                    "like a parameter flag: " << argv[i]);

  //
  // Move the argv list to 'eliminate' the processed arguments
  // (Greatly simplified by JE)
  //
  if (i == 1)                // If no parameters, just bail out
    return;                  // (not really needed, but speeds things up)
  int j=1;                   // Otherwise, compact things
  while (i < argc)
    argv[j++] = argv[i++];
  argc = j;                  // This calculation looked weird before
  argv[argc] = (char*)0;     // End the list with a null char string
}


void ParameterList::write_unused_parameters(ostream& os)
{
std::list<ParameterList::ParameterListItem >::iterator curr = parameters.begin();
std::list<ParameterList::ParameterListItem >::iterator end = parameters.end();
while (curr != end) {
  if ((*curr).used == false)
     os << (*curr).label << endl;
  curr++;
  }
}


int ParameterList::unused()
{
int num=0;

std::list<ParameterList::ParameterListItem >::iterator curr = parameters.begin();
std::list<ParameterList::ParameterListItem >::iterator end = parameters.end();
while (curr != end) {
  if ((*curr).used == false)
     num++;
  curr++;
  }
return num;
}


void ParameterList::register_parameters(ParameterSet& tmp)
{
map<string, map<string,size_type>* >::iterator curr = tmp.categorized_parameters.begin();
map<string, map<string,size_type>* >::iterator end  = tmp.categorized_parameters.end();
while (curr != end) {
  map<string, size_type>::const_iterator lcurr = (*curr).second->begin();
  map<string, size_type>::const_iterator lend  = (*curr).second->end();
  while (lcurr != lend) {
    if (!registered_params.parameter_exists( (*lcurr).first )) {
       Parameter* foo = tmp.param_info[(*lcurr).second];
       registered_params.add_parameter(*foo, (*curr).first);
       }
    lcurr++;
    }
  curr++;
  }
}


} // namespace utilib
#endif
