/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class: GetLongOpt
//- Description: GetLongOpt manages the definition and parsing of 
//- long options. Command line options can be abbreviated as long as there 
//- is no ambiguity. If an option requires a value, the value should
//- be separated from the option either by whitespace or an "=".
//- 
//- Other features: $
//- o	GetLongOpt can be used to parse options given through environments.$
//- o	GetLongOpt provides a usage function to print usage.$
//- o	Flags & options with optional or mandatory values are supported.$
//- o	The option marker ('-' in Unix) can be customized.$
//- o	Parsing of command line returns optind (see getopt(3)).$
//- o	Descriptive error messages.$
//-
//- Author: S Manoharan. Advanced Computer Research Institute. Lyon. France
//- Owner: Bill Bohnhoff
//- Checked By:
//- Version $Id: CommandLineHandler.hpp 6710 2010-04-01 22:20:54Z briadam $

#ifndef GETLONGOPT_H
#define GETLONGOPT_H

#include "dakota_system_defs.hpp"
#include "dakota_global_defs.hpp"


namespace Dakota {

/// GetLongOpt is a general command line utility from S. Manoharan
/// (Advanced Computer Research Institute, Lyon, France).

/** GetLongOpt manages the definition and parsing of "long options."
    Command line options can be abbreviated as long as there is no
    ambiguity. If an option requires a value, the value should be
    separated from the option either by whitespace or an "=". */

class GetLongOpt
{
public:
  /// enum for different types of values associated with command line options.
  enum OptType { 
    Valueless,      ///< option that may never have a value
    OptionalValue,  ///< option with optional value 
    MandatoryValue  ///< option with required value
   };

  /// Constructor
  /** Constructor for GetLongOpt takes an optional argument: the option
      marker. If unspecified, this defaults to '-', the standard (?)
      Unix option marker. */
  GetLongOpt(const char optmark = '-');

  ~GetLongOpt(); ///< Destructor
  
  /// parse the command line args (argc, argv).
  /** A return value < 1 represents a parse error. Appropriate error
      messages are printed when errors are seen. parse returns the the
      optind (see getopt(3)) if parsing is successful. */
  int parse(int argc, char * const *argv);
  /// parse a string of options (typically given from the environment).
  /** A return value < 1 represents a parse error. Appropriate error
      messages are printed when errors are seen. parse takes two
      strings: the first one is the string to be parsed and the second
      one is a string to be prefixed to the parse errors. */
  int parse(char * const str, char * const p);

  /// Add an option to the list of valid command options.
  /** enroll adds option specifications to its internal database. The
      first argument is the option sting. The second is an enum saying
      if the option is a flag (Valueless), if it requires a mandatory
      value (MandatoryValue) or if it takes an optional value
      (OptionalValue).  The third argument is a string giving a brief
      description of the option. This description will be used by
      GetLongOpt::usage.  GetLongOpt, for usage-printing, uses {$val}
      to represent values needed by the options. {\<$val\>} is a
      mandatory value and {[$val]} is an optional value. The final
      argument to enroll is the default string to be returned if the
      option is not specified. For flags (options with Valueless), use
      "" (empty string, or in fact any arbitrary string) for
      specifying TRUE and 0 (null pointer) to specify FALSE. */
  int enroll(const char * const opt, const OptType t,
	     const char * const desc, const char * const val);
  
  /// Retrieve value of option
  /** The values of the options that are enrolled in the database can
      be retrieved using retrieve. This returns a string and this
      string should be converted to whatever type you want.  See atoi,
      atof, atol, etc.  If a "parse" is not done before retrieving all
      you will get are the default values you gave while enrolling!
      Ambiguities while retrieving (may happen when options are
      abbreviated) are resolved by taking the matching option that was
      enrolled last. For example, -{v} will expand to {-verify}.  If
      you try to retrieve something you didn't enroll, you will get a
      warning message. */
  const char *retrieve(const char * const opt) const;

  /// Print usage information to outfile
  void usage(std::ostream &outfile = Cout) const;

  /// Change header of usage output to str
  /** GetLongOpt::usage is overloaded. If passed a string "str", it sets the
      internal usage string to "str". Otherwise it simply prints the
      command usage. */
  void usage(const char *str)		{ ustring = str; }

  /// Store a specified option value
  void store(const char *name, const char *value);

private:
  struct Cell {
     const char *option;	///< option name
     OptType type;		///< option type
     const char *description;	///< a description of option
     const char *value;	        ///< value of option (string)
     Cell *next;		///< pointer to the next cell
     
     Cell() { option = description = value = 0; next = 0; }
   };

  Cell *table;				///< option table
  const char *ustring;			///< usage message
  char *pname;				///< program basename
  char optmarker;			///< option marker
  
  int enroll_done;			///< finished enrolling
  Cell *last;				///< last entry in option table 

  /// extract the base name from a string as delimited by '/'
  char *basename(char * const p) const;
  /// internal convenience function for setting Cell::value
  int setcell(Cell *c, char *valtoken, char *nexttoken, const char *p);
};


//- Class:       CommandLineHandler
//- Description: CommandLineHandler provides additional functionality that
//-              is specific to Dakota's needs beyond that of the base class,
//-              GetLongOpt.
//- Owner:       Bill Bohnhoff


/// Utility class for managing command line inputs to DAKOTA.

/** CommandLineHandler provides additional functionality that is
    specific to DAKOTA's needs for the definition and parsing of
    command line options.  Inheritance is used to allow the class to
    have all the functionality of the base class, GetLongOpt. */

class CommandLineHandler: public GetLongOpt
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor, requires check_usage() call for parsing
  CommandLineHandler();
  /// constructor with parsing
  CommandLineHandler(int argc, char** argv, int world_rank);
  /// destructor
  ~CommandLineHandler();

  //
  //- Heading: Methods
  //   

  /// Verifies that DAKOTA is called with the correct command usage.
  /// Prints a descriptive message and exits the program if incorrect.
  void check_usage(int argc, char** argv);

  /// Returns the number of evaluations to be read from the restart
  /// file (as specified on the DAKOTA command line) as an integer
  /// instead of a const char*.
  int read_restart_evals() const;

  /// Print usage information to outfile, conditionally on rank
  void usage(std::ostream &outfile = Cout) const;

private:

  //
  //- Heading: Methods
  //

  /// enrolls the supported command line inputs.
  void initialize_options();
  
  /// output only on Dakota worldRank 0 if possible
  void output_helper(const std::string& message, std::ostream &os) const;
  
  /// Rank of this process within Dakota's allocation; manages conditional output
  int worldRank;

};


inline CommandLineHandler::CommandLineHandler():
  worldRank(0)
{ initialize_options(); }


inline CommandLineHandler::
CommandLineHandler(int argc, char** argv, int world_rank):
  worldRank(world_rank)
{ initialize_options(); check_usage(argc, argv); }


inline CommandLineHandler::~CommandLineHandler() { }


inline int CommandLineHandler::read_restart_evals() const
{
  const char* stop_restart_str = retrieve("stop_restart");
  // stop_restart_str is a NULL pointer if no command line input
  return (stop_restart_str) ? std::atoi(stop_restart_str) : 0;
}


} // namespace Dakota

#endif // GETLONGOPT_H
