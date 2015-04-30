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
#ifndef utilib_ParameterList_h
#define utilib_ParameterList_h

#include <string>
#include <map>

namespace utilib {

class ParameterList
{
public:

  void add(const std::string& name, const std::string& value)
    { data[name] = value; }

  std::map<std::string,std::string> data;
};

}

#endif
#endif
#if 1
/**
 * \file ParameterList.h
 * 
 * Defines the utilib::ParameterList class.
 */
#ifndef utilib_ParameterList_h
#define utilib_ParameterList_h

#include <utilib/std_headers.h>
#include <utilib/Parameter.h>
#include <utilib/ParameterSet.h>
#if defined(UTILIB_HAVE_MPI)
#include <utilib/mpiUtil.h>
#endif

namespace utilib {

class ParameterSet;


/**
 * Defines an auxiliary class that can be used to define a list of 
 * parameters that will be fed into a ParameterSet object.  This class
 * offers additional functionality beyond a simple list of pairs of
 * strings.  Specifically, it can process command-line arguments and
 * files of parameters.
 *
 * Parameter values can be initialized with command-line arguments using the 
 * ParameterList::process_parameters method.  This method supports
 * the use of the GNU style uses parameter keywords preceded by two hyphens
 * rather than keyword letters. This style is extensible to contexts in
 * which there are two many parameters to use single-letter parameters. This 
 * style is easier to read than the alphabet soup of older styles, and it can be
 * combined with single-letter parameters (for commonly used parameters). 
 * A parameter argument (if any) can be separated by either whitespace or a 
 * single .=. (equal sign) character:
 *
 *    program --param1 paramval --param2=paramval
 *
 */
class ParameterList
{
  #if !defined(DOXYGEN)
  friend class ParameterSet;
  #endif

public:

  /// Constructor
  ParameterList() {}

  /// Destructor
  ~ParameterList() {}

#if defined(UTILIB_HAVE_MPI) || defined(DOXYGEN)
  /// Synchronize the parameter list
  void synchronize(int root=0, MPI_Comm comm = MPI_COMM_WORLD);
#endif

  /// Read parameter list values
  void read_parameters(std::istream& is, const std::string& terminator);

  /// Read parameter list values
  void read_parameters(std::istream& is)
		{read_parameters(is,"");}

  /// Write parameter list values
  void write_parameters(std::ostream& os) const;

  /// Pack the parameter list
  void write_parameters(PackBuffer& os) const;

  /// Unpack the parameter list
  void read_parameters(UnPackBuffer& is);

  /// Process a parameter list to get parameter values
  void process_parameters(int& argc, char**& argv, 
			unsigned int min_num_required_args);

  /// Process parameters from a file
  void process_parameters(const std::string& filename);

  /// Returns the number of parameter values
  size_type size()
		{return parameters.size();}

  /// Add a parameter
  void add(const std::string& label, const std::string& value,
				bool process_param=true)
        {add_parameter(label,value,process_param);}

  /// Add a parameter
  void add_parameter(const std::string& label, const std::string& value,
				bool process_param=true);

  /// Add posix parameters
  void add_posix_parameters(const std::string& params, const std::string& value,
				bool process_param=true);

  /// Register a parameter
  void register_parameters(ParameterSet& pset);

  /// Register a parameter
  template <class Type>
  void register_parameters();

  /// Write the registered parameters
  void write_registered_parameters(std::ostream& os, bool verbose=true,
                                        const char* seperator="\n") const
		{registered_params.write_parameters(os,verbose,seperator);}

  /// Write unused parameters
  void write_unused_parameters(std::ostream& os);

  /// The number of unused parameters
  int unused();

  /// An internal class that incorporates data about a parameter list item.
  class ParameterListItem {
    public:
    /// Constructor
    ParameterListItem() : used(false) {}
    /// The name of the parameter
    std::string label;
    /// The value
    std::string value;
    /// If true, then this parameter is being used.
    bool   used;
    };

  /// The list of parameter values
  std::list<ParameterListItem> parameters;

  /// A temporary list item
  ParameterListItem tmp_param;

  /// The set of registered parameters
  ParameterSet registered_params;

};


template <class Type>
inline void ParameterList::register_parameters()
{
Type tmp;
register_parameters(tmp);
}

} // namespace utilib

#endif
#endif
