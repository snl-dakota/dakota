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

/**
 * \file CommandShell.h
 *
 * Defines the colin::CommandShell class.
 */

#ifndef colin_CommandShell_h
#define colin_CommandShell_h

#include <acro_config.h>
#include <utilib/std_headers.h>
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <sys/wait.h>
#include <sys/types.h>
#endif
#include <errno.h>
#include <utilib/exception_mngr.h>
#include <utilib/CommonIO.h>

/// The environment variable used by execve.
extern char** environ;

namespace colin
{

/// A wrapper for the standard system call
inline int my_system(const char* command)
{
#if 1
   return system(command);
#elif defined(_MSC_VER) || defined(__MINGW32__)
   EXCEPTION_MNGR(std::runtime_error, "my_system - implementation not available for windows.");
#else
   int pid, status;

   if (command == 0)
      return 1;

   pid = fork();
   if (pid == -1)
      return -1;

   if (pid == 0)
   {
      char *const argv[4] = { "sh", "-c", (char*)command, 0 };
      execve("/bin/sh", argv, environ);
      exit(126);
   }

   do
   {
      if (waitpid(pid, &status, 0) == -1)
      {
         if (errno != EINTR)
            return -1;
      }
      else
      {
#ifdef WCOREDUMP
         if (WCOREDUMP(status))
         {
            EXCEPTION_MNGR(std::runtime_error, "my_system -- command terminated with a core dump!");
         }
#endif
         if (WIFEXITED(status) == 0)
         {
            EXCEPTION_MNGR(std::runtime_error, "my_system -- command exited abnormally!");
         }
         else if (WEXITSTATUS(status))
         {
            EXCEPTION_MNGR(std::runtime_error, "my_system -- command with nonzero exit status!");
         }
         else
            return 0;
      }
   }
   while (1);
#endif
}



/**
  * A class that encapsulates a command shell operation, using
  * a stream model.  Flushing corresponds to executing the shell.
  *
  * This class was originally developed by Bill Bohnhoff as part of the
  * DAKOTA project, and later updated to use STL classes by Bill Hart.
  */
class CommandShell
{
public:

   /// Constructor
   CommandShell() : asynchFlag(false), quietFlag(false) {}

   /// Stream operator that appends \c string to the unix command
   CommandShell& operator<<(const char* str)
   { unixCommand += str; return *this; }

   /// Stream operator that appends \c string to the unix command
   CommandShell& operator<<(const std::string& str)
   { unixCommand += str; return *this; }

   /// Execute the function \c f on the current command shell
   CommandShell& operator<<(CommandShell& (*f)(CommandShell&))
   { return(*f)(*this); }

   /// Flushes the shell; i.e. executes the unixCommand
   CommandShell& flush()
   {
      #ifndef _WIN32
      if (asynchFlag) unixCommand += " &";
      #endif
      if (!quietFlag)
         ucout << unixCommand << std::endl;
      int status = my_system(unixCommand.data());
      if (status == -1)
      {
         EXCEPTION_MNGR(std::runtime_error, "CommandShell -- fork failed in my_system() call");
      }
      else if (status == 1)
      {
         EXCEPTION_MNGR(std::runtime_error, "CommandShell -- null command for my_system(): \"" << unixCommand << "\"");
      }
      else if (status == 127)
      {
         EXCEPTION_MNGR(std::runtime_error, "CommandShell -- my_system() call to /bin/sh failed");
      }
      unixCommand.resize(0);
      return *this;
   }

   /// Set \c asynchFlag
   void asynch_flag(const bool flag)
   {asynchFlag = flag;}

   /// Get \c asynchFlag
   bool asynch_flag() const
   {return asynchFlag;}

   /// Set \c quietFlag
   void quiet_flag(const bool flag)
   {quietFlag = flag;}

   /// Return quietFlag
   bool quiet_flag()
   {return quietFlag;}

protected:

   /// The string that contains the unix command
   std::string unixCommand;

   /// If true, then execute the shell asynchronously
   bool asynchFlag;

   /// If true, then execute the shell without debugging I/O
   bool quietFlag;

};



/// A convenient shell manipulator function to "flush" the shell
inline CommandShell& flush(CommandShell& shell)
{
   shell.flush();
   return shell;
}

}

#endif
