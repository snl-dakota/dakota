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

//
// ParameterSet.cpp
//

#include <utilib/_math.h>
#include <utilib/comments.h>
#include <utilib/ParameterSet.h>
#include <utilib/ParameterList.h>

using namespace std;

namespace utilib {

//
// Two parameters are created by default
//
size_type ParameterSet::num_default_parameters=2;


//============================================================================
//
//
ParameterSet::ParameterSet()
  : help_parameter(false)
{
param_file_callback.params = this;
create_parameter("help",help_parameter,"<bool>","false",
	"Used to determine if parameter information has been requested.");
create_parameter("param-file",param_file,"<string>","<null>",
	"Defines an auxillary parameter file that is processed immediately.");
alias_parameter("param-file","param_file");
set_parameter_callback("param-file", param_file_callback);
}


//============================================================================
//
//
ParameterSet::~ParameterSet()
{
//
// Delete the parameters
//
size_type ndx=0;
vector<Parameter*>::const_iterator tcurr = param_info.begin();
vector<Parameter*>::const_iterator tlast = param_info.end();
while (tcurr != tlast) {
  if (param_ownership(ndx++))
     delete *tcurr;
  tcurr++;
  }
//
// Delete the index maps used for categorized parameters
//
map<string, map<string,size_type>* >::const_iterator curr = 
						categorized_parameters.begin();
map<string, map<string,size_type>* >::const_iterator end  = 
						categorized_parameters.end();
while (curr != end) {
  delete (*curr).second;
  curr++;
  }
}


//============================================================================
//
//
void ParameterSet::set_parameters(ParameterList& params, bool delete_used)
{
std::list<ParameterList::ParameterListItem >::iterator curr = params.parameters.begin();
std::list<ParameterList::ParameterListItem >::iterator end = params.parameters.end();
while (curr != end) {
  if (parameter_exists( (*curr).label )) {
     set_parameter_with_string( (*curr).label, (*curr).value );
     if (!delete_used) {
        (*curr).used = true;
        curr++;
        }
     else
        curr = params.parameters.erase(curr);
     }
  else
     curr++;
  }
}


//============================================================================
//
//
void ParameterSet::write_parameters(ostream& os, bool verbose,
                                        const char* seperator) const
{
if (!verbose) {

   map<string,size_type>::const_iterator curr = parameters.begin();
   map<string,size_type>::const_iterator last = parameters.end();
   
   while (curr != last) {
     if (param_info[(*curr).second]->name != (*curr).first) {
        curr++;
        continue;
        }

     const Parameter& param = *(param_info[(*curr).second]);
     if ( !(param.disabled) ) {
        os << "  " << param.name;
        os << seperator;
        }
     curr++;
     }
   }

else {
   map<string, map<string,size_type>* >::const_iterator curr = 
						categorized_parameters.begin();
   map<string, map<string,size_type>* >::const_iterator end  = 
						categorized_parameters.end();
   while (curr != end) {
     os << endl;
     os << "-----------------------------------------------------------------------------" << endl;
     os << "  Category: " << (*curr).first << endl;
     os << "-----------------------------------------------------------------------------" << endl;
     map<string, size_type>::const_iterator lcurr = (*curr).second->begin();
     map<string, size_type>::const_iterator lend  = (*curr).second->end();

     while (lcurr != lend) {
       if (param_info[(*lcurr).second]->name != (*lcurr).first) {
          lcurr++;
          continue;
          }

       const Parameter& param = *(param_info[(*lcurr).second]);
       if ( param.disabled ) {
          lcurr++;
          continue;
          }
  
       os << "    " << param.name << " " <<
                param.syntax << " (default: " <<
                param.default_value << " )\n";
       if (param.validator)
          os << "\t[ " << *(param.validator) << " ]\n";
       os << "\t" <<  param.description << "\n";
       if (param.aliases.size() > 0) {
          os << "\tAliases:";
          set<string>::const_iterator alias_curr = param.aliases.begin();
          set<string>::const_iterator alias_last = param.aliases.end();
          while (alias_curr != alias_last) {
	    os << " " << (*alias_curr);
	    alias_curr++;
            }
          os << endl;
          }
       lcurr++;
       }
    curr++;
    }
  }
}


ostream& xml_escape(ostream& os, const std::string& str)
{
std::string::const_iterator curr = str.begin();
std::string::const_iterator end  = str.end();
while (curr != end) {
  if (*curr == '&')
     os << "&amp;";
  else if (*curr == '>')
     os << "&gt;";
  else if (*curr == '<')
     os << "&lt;";
  else
     os << *curr;
  curr++;
  }
return os;
}

//============================================================================
//
//
void ParameterSet::write_parameters_xml(ostream& os) const
{
   map<string, map<string,size_type>* >::const_iterator curr = categorized_parameters.begin();
   map<string, map<string,size_type>* >::const_iterator end  = categorized_parameters.end();
   os << "<Parameters>" << endl;
   while (curr != end) {
     os << "  <Category name=\"" << (*curr).first << "\">" << endl;
     map<string, size_type>::const_iterator lcurr = (*curr).second->begin();
     map<string, size_type>::const_iterator lend  = (*curr).second->end();

     while (lcurr != lend) {
       const Parameter& param = *(param_info[(*lcurr).second]);
       //
       // Ignore aliases (I think)
       //
       if (param.name != (*lcurr).first) {
          lcurr++;
          continue;
          }
       //
       // Ignore disabled parameters
       //
       if ( param.disabled ) {
          lcurr++;
          continue;
          }
       //
       // Write parameter info
       //
       os << "    <Parameter name=\"" << param.name << "\">" << endl;
       os << "      <Syntax>";
       xml_escape(os,param.syntax) << "</Syntax>" << endl;
       os << "      <Default>";
       xml_escape(os,param.default_value) << "</Default>" << endl;
       os << "      <Description>";
       xml_escape(os,param.description) << "</Description>" << endl;
       if (param.aliases.size() > 0) {
          set<string>::const_iterator alias_curr = param.aliases.begin();
          set<string>::const_iterator alias_last = param.aliases.end();
          while (alias_curr != alias_last) {
            os << "      <Alias>" <<  (*alias_curr) << "</Alias>" << endl;
	    alias_curr++;
            }
          }
       os << "    </Parameter>" << endl;
       lcurr++;
       }
     os << "  </Category>" << endl;
    curr++;
    }
  os << "</Parameters>" << endl;
}


//============================================================================
//
//
void ParameterSet::write_parameters_html(ostream& os) const
{
   if (categorized_parameters.size() == 0) return;
   os << "<table border=1 cellspacing=0 cellpadding=5>" << endl;
   map<string, map<string,size_type>* >::const_iterator curr = 
						categorized_parameters.begin();
   map<string, map<string,size_type>* >::const_iterator end  = 
						categorized_parameters.end();
   while (curr != end) {
     os << "<tr><td valign=top rowspan=";

     /// Count Num Rows in this Category
     
     int ctr=0;
     map<string, size_type>::const_iterator lcurr = (*curr).second->begin();
     map<string, size_type>::const_iterator lend  = (*curr).second->end();
     while (lcurr != lend) {
       if ((param_info[(*lcurr).second]->name == (*lcurr).first) &&
           !((param_info[(*lcurr).second])->disabled))
          ctr++;
       lcurr++;
       }

     os << ctr << "><b>" << (*curr).first << "</b></td>"<< endl;

     /// Iterate through all rows

     bool firstflag=true;
     lcurr = (*curr).second->begin();
     lend  = (*curr).second->end();
     while (lcurr != lend) {
       if (param_info[(*lcurr).second]->name != (*lcurr).first) {
          lcurr++;
          continue;
          }

       const Parameter& param = *(param_info[(*lcurr).second]);
       if ( param.disabled ) {
          lcurr++;
          continue;
          }
  
       if (!firstflag) {
          os << "<tr>" << endl;
          }
       firstflag=false;

       os << "<td><b>" << param.name << "</b><br> " <<
                param.syntax << " (default: " <<
                param.default_value << " )<br> ";
       if (param.validator)
          os << "\t[ " << *(param.validator) << " ]<br> ";
       os << "\t" <<  param.description << "<br> ";
       if (param.aliases.size() > 0) {
          os << "\tAliases:";
          set<string>::const_iterator alias_curr = param.aliases.begin();
          set<string>::const_iterator alias_last = param.aliases.end();
          while (alias_curr != alias_last) {
	    os << " " << (*alias_curr);
	    alias_curr++;
            }
          }
       os << "</td>" << endl;
       lcurr++;
       os << "</tr>" << endl;
       }
    curr++;
    }
    os << "</table>" << endl;
}


//============================================================================
//
//
void ParameterSet::write_parameter_values(ostream& os, const char* opt_label) const
{
if (strcmp(opt_label,"")!=0)
   os << "# ---- Options for " << opt_label << " ----" << endl;

size_type len=0;
vector<Parameter*>::const_iterator tcurr = param_info.begin();
vector<Parameter*>::const_iterator tlast = param_info.end();
while (tcurr != tlast) {
  if (len < (*tcurr)->name.size()) len = (*tcurr)->name.size();
  tcurr++;
  }
char tformat[32];
#ifdef _MSC_VER
sprintf_s(tformat,32,"%%-%lds",(long int)len);
#else
sprintf(tformat,"%%-%lds",(long int)len);
#endif

map<string,size_type>::const_iterator curr = parameters.begin();
map<string,size_type>::const_iterator last = parameters.end();

char tmp[128];
while (curr != last) {
  if (param_info[(*curr).second]->name != (*curr).first) {
     curr++;
     continue;
     }

  const Parameter& param = *(param_info[(*curr).second]);
  if ( !(param.disabled) ) {
#ifdef _MSC_VER
     sprintf_s(tmp,128,tformat,param.name.data());
#else
     sprintf(tmp,tformat,param.name.data());
#endif
     if (!param.initialized)
        os << tmp << " " << param.info << "\t# default\n";
     else
        os << tmp << " " << param.info << "\n";
     }
  curr++;
  }
}


//============================================================================
//
//
void ParameterSet::read_parameter_values(istream& is, const string& terminator)
{
bool eof_flag = (terminator == "");
char buf[256];

if (!is) return;

string label;

is >> comment_lines;
is >> label;
while (is) {
  try {
    if (!eof_flag && (label == terminator))
       return;

    Parameter& param = find_parameter(label);
    is.getline(buf,256);
    if (!is)
       EXCEPTION_MNGR(runtime_error, "ParameterSet::read_parameter_values - problem reading value of the " << label << " param");
  
    stringstream str(buf);
    str >> param.info;
    param.initialized = true;
    }
  catch (invalid_argument& ) {
     EXCEPTION_MNGR(runtime_error,
                    "ParameterSet::read_parameter_values - missing parameter \"" <<
                    label.data() << "\".");
    }
  //
  // Get next parameter
  //
  is >> comment_lines;
  is >> label;
  }
}


//============================================================================
//
//
void ParameterSet::write_parameter_values(PackBuffer& os) const
{
vector<Parameter*>::const_iterator curr = param_info.begin();
vector<Parameter*>::const_iterator last = param_info.end();
while (curr != last) {
  if ( !((*curr)->disabled) ) {
     os << (*curr)->name << *(*curr);
     }
  curr++;
  }
}


//============================================================================
//
//
void ParameterSet::read_parameter_values(UnPackBuffer& is)
{
if (!is) return;
if (is.curr() == is.size()) return;

string label;

is >> label;
while (is) {
  //cerr << "LABEL: " << label << endl;
  try {
    Parameter& param = find_parameter(label);
    is >> param;
    param.initialized = true;
    if (!is)
       EXCEPTION_MNGR(runtime_error, "ParameterSet::read_parameter_values - problem reading value of the " << label << " parameter");
    }
  catch (invalid_argument& ) {
     EXCEPTION_MNGR(runtime_error,
                    "ParameterSet::read_parameter_values - missing parameter \"" <<
                    label.data() << "\".");
    }
  //
  // Get next parameter
  //
  if (is.curr() == is.size()) break;
  is >> label;
  }
}


//============================================================================
//
//
bool ParameterSet::process_parameters(int& argc, char**& argv,
				unsigned int min_num_required_params)
{
ParameterList plist;
plist.process_parameters(argc,argv,min_num_required_params);
set_parameters(plist);
#if 0

//
// Go through the argument list
//
int i=1;
while ((i < argc) && ((argv[i][0] == '-') && (argv[i][1] == '-'))) {
  //
  // Process an argument
  //
  char* tmp = strchr(argv[i],'=');
  if (tmp) tmp++;
  if (tmp && (*tmp != '\000')) {
     *(tmp-1) = '\000';
     set_parameter_with_string( &(argv[i][2]), tmp);
     i += 1;
     }
  else if (tmp && (*tmp == '\000')) {
     *(tmp-1) = '\000';
     set_parameter( &(argv[i][2]), true);
     i += 1;
     }
  else if ( ((i+1) == argc) || 
            ((argv[i+1][0] == '-') && (argv[i+1][1] == '-'))) {
     //
     // Implicitly true flag
     // Example: -flush -help
     //
     set_parameter( &(argv[i][2]), true);
     i++;
     }
  else {
     set_parameter_with_string( &(argv[i][2]), &(argv[i+1][0]) );
     i += 2;
     }
  }

//
// Move the argv list to 'eliminate' the processed arguments
// JE -- modified this to match similar code in ParameterList
// It would be a lot better if all this code was not duplicated!
//
if (i == 1)
   return true;

 int j=1;
 while (i < argc) 
   argv[j++] = argv[i++];
 argc = j;
 
 argv[argc] = (char*)0;

#endif
 return (argc == 1) || (argv[1][0] != '-');
}


//============================================================================
//
//
void ParameterSet::set_parameter_with_string(const string& name, const string& value)
{
try {
  Parameter& param = find_parameter(name);
  std::stringstream sbuf;
  sbuf << value;
  param.set_value_with_string(sbuf.str());
  //sbuf >> param.info;
  param.initialized=true;
  }
catch (invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::set_parameter - missing parameter \"" <<
                  name.data() << "\".");
  }
catch (const bad_any_cast&) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::set_parameter - bad parameter type for \"" <<
                  name << "\".");
  }
}


#if 0
//============================================================================
//
//
void ParameterSet::add_parameter(const Parameter& param, const string& category)
{
Parameter* tparam = new Parameter(param);
add_parameter(tparam,category);
}
#endif


//============================================================================
//
//
void ParameterSet::add_parameter(Parameter* param, 
						const string& category,
						bool ownership)
{
size_type tmp = param_info.size();
//cerr << param->name << " " << tmp << endl << flush;

string tmp_category;
if (category == "")
   tmp_category = "General";
else
   tmp_category = category;
map<string, map<string,size_type>* >::iterator curr = categorized_parameters.find(tmp_category);
if (curr == categorized_parameters.end())
   categorized_parameters[tmp_category] = new map<string,size_type>;
(*categorized_parameters[tmp_category])[param->name] = tmp;

parameters[param->name] = tmp;

//cerr << param->name << " " << tmp << endl << flush;
param_info.push_back(param);
param_ownership.resize(param_ownership.size()+1);
param_ownership.put(param_ownership.size()-1,ownership);

if (param->aliases.size() > 0) {
   size_type index = param_info.size()-1;
   std::set<string>::iterator curr = param->aliases.begin();
   std::set<string>::iterator end  = param->aliases.end();
   while (curr != end) {
     #if !defined(COUGAR)
     parameters[(*curr)] = index;
     #else
     parameters.push_back(pair<string,size_type>((*curr),index));
     #endif
     curr++;
     }
   }
}


//============================================================================
//
//
void ParameterSet::alias_parameter(const string& name, const string& alias)
{
try {
  size_type index = parameter_index(name);
  param_info[index]->aliases.insert(alias);
  parameters[alias] = index;
  }
catch (invalid_argument& ) {
   EXCEPTION_MNGR(runtime_error,
                  "ParameterSet::alias_parameter - missing parameter \"" <<
                  name.data() << "\".");
  }
}


//============================================================================
//
//
void ParameterSet::augment_parameters(ParameterSet& pset, bool overwrite)
{
map<string, map<string,size_type>* >::iterator curr = pset.categorized_parameters.begin();
map<string, map<string,size_type>* >::iterator end  = pset.categorized_parameters.end();
while (curr != end) {
  map<string, size_type>::iterator lcurr = (*curr).second->begin();
  map<string, size_type>::iterator lend  = (*curr).second->end();
  while (lcurr != lend) {
    if (overwrite || !parameter_exists((*lcurr).first)) {
       Parameter* foo = pset.param_info[(*lcurr).second];
       //registered_params.add_parameter(*foo, (*curr).first);
       add_parameter(*foo,(*curr).first);
       }
    lcurr++;
    }
  curr++;
  }
}

} // namespace utilib
