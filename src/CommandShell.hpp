/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        CommandShell
//- Description:
//-
//-
//- Owner:        Bill Bohnhoff
//- Version: $Id: CommandShell.hpp 7021 2010-10-12 22:19:01Z wjbohnh $

#ifndef COMMAND_SHELL_H
#define COMMAND_SHELL_H

#include "dakota_system_defs.hpp"
#include <string>


namespace Dakota {

/// Utility class which defines convenience operators for spawning
/// processes with system calls.

/** The CommandShell class wraps the C system() utility and defines
    convenience operators for building a command string and then
    passing it to the shell. */

class CommandShell
{
public:

  //
  //- Heading: Constructor and destructor
  //

  CommandShell();   ///< constructor
  ~CommandShell();  ///< destructor

  //
  //- Heading: Operator overloaded functions
  //

  /// appends cmd to sysCommand
  CommandShell& operator<<(const char* cmd);
  CommandShell& operator<<(const std::string& cmd);

  /// allows passing of the flush function to the shell using <<
  CommandShell& operator<<(CommandShell& (*f)(CommandShell&));

  /// "flushes" the shell; i.e. executes the sysCommand
  CommandShell& flush();

  void asynch_flag(const bool flag);          ///< set the asynchFlag
  bool asynch_flag() const;                   ///< get the asynchFlag

  void suppress_output_flag(const bool flag); ///< set the suppressOutputFlag
  bool suppress_output_flag() const;          ///< get the suppressOutputFlag

private:

  //
  //- Heading: Data members
  //

  /// The command string that is constructed through one or more <<
  /// insertions and then executed by flush
  std::string sysCommand;

  /// flags nonblocking operation (background system calls)
  bool asynchFlag;

  /// flags suppression of shell output (no command echo)
  bool suppressOutputFlag;
};


/// constructor
inline CommandShell::CommandShell() :
  asynchFlag(false), suppressOutputFlag(false)
{ }

/// destructor
inline CommandShell::~CommandShell()
{ }

/// convenience operator:  allows passing of the flush func to the shell via <<
inline CommandShell& CommandShell::operator<<(CommandShell& (*f)(CommandShell&))
{ return (*f)(*this); }

/// convenient operator:  appends string to the commandString to be executed
inline CommandShell& CommandShell::operator<<(const char* cmd)
{
  sysCommand += cmd;
  return *this;
}

/// convenient operator:  appends string to the commandString to be executed
inline CommandShell& CommandShell::operator<<(const std::string& cmd)
{
  sysCommand += cmd;
  return *this;
}

/// convenient shell manipulator function to "flush" the shell
CommandShell& flush(CommandShell& shell);

inline void CommandShell::asynch_flag(const bool flag)
{ asynchFlag = flag; }
 
inline bool CommandShell::asynch_flag() const
{ return asynchFlag; }

inline void CommandShell::suppress_output_flag(const bool flag)
{ suppressOutputFlag = flag; }
 
inline bool CommandShell::suppress_output_flag() const
{ return suppressOutputFlag; }

} // namespace Dakota

#endif
