/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// $Id: CommandLineHandler.cpp 6856 2010-07-19 16:55:12Z wjbohnh $
// S Manoharan. Advanced Computer Research Institute. Lyon. France

#include "CommandLineHandler.hpp"
// #ifdef DAKOTA_HAVE_MPI
// #include <mpi.h>
// #endif // DAKOTA_HAVE_MPI
#include <string>


namespace Dakota {

GetLongOpt::GetLongOpt(const char optmark)
{
   table = last = 0;
   ustring = "[valid options and arguments]";
   enroll_done = 0;
   optmarker = optmark;
}

GetLongOpt::~GetLongOpt()
{
   Cell *t = table;

   while ( t ) {
      Cell *tmp = t;
      t = t->next;
      delete tmp;
   }
}

// MSE, 10/8/99: "pname" changed to "p" to quiet compiler warning
char * GetLongOpt::basename(char * const p) const
{
   char *s;

   s = std::strrchr(p, '/');
   if ( s == 0 ) s = p;
   else ++s;

   return s;
}

int GetLongOpt::enroll(const char * const opt, const OptType t,
		       const char * const desc, const char * const val)
{
   if ( enroll_done ) return 0;

   Cell *c = new Cell;
   c->option = opt;
   c->type = t;
   c->description = desc ? desc : "no description available";
   c->value = val;
   c->next = 0;

   if ( last == 0 ) {
      table = last = c;
   }
   else {
      last->next = c;
      last = c;
   }

   return 1;
}

const char * GetLongOpt::retrieve(const char * const opt) const
{
   Cell *t;
   for ( t = table; t != 0; t = t->next ) {
      if ( std::strcmp(opt, t->option) == 0 )
	 return t->value;
   }
   Cerr << "GetLongOpt::retrieve - unenrolled option "
        << optmarker << opt << '\n';
   return 0;
}

int GetLongOpt::parse(int argc, char * const *argv)
{
   int optind = 1;

   pname = basename(*argv);
   enroll_done = 1;
   if ( argc-- <= 1 ) return optind;

   while ( argc >= 1 ) {
      char *token = *++argv; --argc;

      if ( token[0] != optmarker || !token[1] )	/* allow "-" for stdin */
	 break;	/* end of options */
      if ( token[1] == optmarker ) {
		/* "--" ==> end of options; omit it and break */
		if ( !token[2] ) {
			++optind;
			break;
			}
		/* treat --opname as -opname */
		++token;
		}

      ++optind;
      char *tmptoken = ++token;
      while ( *tmptoken && *tmptoken != '=' )
	 ++tmptoken;
      /* (tmptoken - token) is now equal to the command line option
	 length. */

      Cell *t;
      enum { NoMatch, ExactMatch, PartialMatch } matchStatus = NoMatch;
      Cell *pc = 0;	// pointer to the partially-matched cell
      for ( t = table; t != 0; t = t->next ) {
	 if ( std::strncmp(t->option, token, (tmptoken - token)) == 0 ) {
	    if ( std::strlen(t->option) == (tmptoken - token) ) {
	       /* an exact match found */
	       int stat = setcell(t, tmptoken, *(argv+1), pname);
	       if ( stat == -1 ) return -1;
	       else if ( stat == 1 ) {
		  ++argv; --argc; ++optind;
	       }
	       matchStatus = ExactMatch;
	       break;
	    }
	    else {
	       /* partial match found */
	       matchStatus = PartialMatch;
	       pc = t;
	    }
	 } /* end if */
      } /* end for */

      if ( matchStatus == PartialMatch ) {
	 int stat = setcell(pc, tmptoken, *(argv+1), pname);
	 if ( stat == -1 ) return -1;
	 else if ( stat == 1 ) {
	    ++argv; --argc; ++optind;
	 }
      }
      else if ( matchStatus == NoMatch ) {
	 Cerr << pname << ": unrecognized option "
	      << optmarker << std::strtok(token,"= ") << '\n';
	 return -1;		/* no match */
      }

   } /* end while */

   return optind;
}

int GetLongOpt::parse(char * const str, char * const p)
{
   enroll_done = 1;
   char *token = std::strtok(str, " \t");
   const char *name = p ? p : "GetLongOpt";

   while ( token ) {
      if ( token[0] != optmarker || token[1] == optmarker ) {
	 Cerr << name << ": nonoptions not allowed\n";
	 return -1;	/* end of options */
      }

      char *ladtoken = 0;	/* lookahead token */
      char *tmptoken = ++token;
      while ( *tmptoken && *tmptoken != '=' )
	 ++tmptoken;
      /* (tmptoken - token) is now equal to the command line option
	 length. */

      Cell *t;
      enum { NoMatch, ExactMatch, PartialMatch } matchStatus = NoMatch;
      Cell *pc =0;	// pointer to the partially-matched cell
      for ( t = table; t != 0; t = t->next ) {
	 if ( std::strncmp(t->option, token, (tmptoken - token)) == 0 ) {
	    if ( std::strlen(t->option) == (tmptoken - token) ) {
	       /* an exact match found */
	       ladtoken = std::strtok(0, " \t");
	       int stat = setcell(t, tmptoken, ladtoken, name);
	       if ( stat == -1 ) return -1;
	       else if ( stat == 1 ) {
		  ladtoken = 0;
	       }
	       matchStatus = ExactMatch;
	       break;
	    }
	    else {
	       /* partial match found */
	       matchStatus = PartialMatch;
	       pc = t;
	    }
	 } /* end if */
      } /* end for */

      if ( matchStatus == PartialMatch ) {
	 ladtoken = std::strtok(0, " \t");
	 int stat = setcell(pc, tmptoken, ladtoken, name);
	 if ( stat == -1 ) return -1;
	 else if ( stat == 1 ) {
	    ladtoken = 0;
	 }
      }
      else if ( matchStatus == NoMatch ) {
	 Cerr << name << ": unrecognized option "
	      << optmarker << std::strtok(token,"= ") << '\n';
	 return -1;		/* no match */
      }

      token = ladtoken ? ladtoken : std::strtok(0, " \t");
   } /* end while */

   return 1;
}

/* ----------------------------------------------------------------
GetLongOpt::setcell returns
   -1	if there was an error
    0	if the nexttoken was not consumed
    1	if the nexttoken was consumed
------------------------------------------------------------------- */

int GetLongOpt::setcell(Cell *c, char *valtoken, char *nexttoken,
			const char *name)
{
   if ( c == 0 ) return -1;

   switch ( c->type ) {
   case GetLongOpt::Valueless :
      if ( *valtoken == '=' ) {
	 Cerr << name << ": unsolicited value for flag "
	      << optmarker << c->option << '\n';
	 return -1;	/* unsolicited value specification */
      }
      c->value = (c->value) ? 0 : (char *) ~0;
      return 0;
   case GetLongOpt::OptionalValue :
      if ( *valtoken == '=' ) {
	 c->value = ++valtoken;
	 return 0;
      }
      else {
	 if ( nexttoken != 0 && nexttoken[0] != optmarker ) {
	    c->value = nexttoken;
	    return 1;
	 }
	 else {
	   // optional keyword with optional value [-kw[=value]]
	   // if optional switch specified, but no value, and no default,
	   // set empty
	   //   kw not specified:        value will be left default (poss NULL)
	   //   kw specified, no value:  set value = default is specified or 
	   //                            "" (empty string) if NULL
	   //   kw specified w/ value:   set value = kw (string)
	   if (c->value == NULL)
	     c->value = "";
	   return 0;
	 }
      }
   case GetLongOpt::MandatoryValue :
      if ( *valtoken == '=' ) {
	 c->value = ++valtoken;
	 return 0;
      }
      else {
	 if ( nexttoken != 0 && nexttoken[0] != optmarker ) {
	    c->value = nexttoken;
	    return 1;
	 }
	 else {
	    Cerr << name << ": mandatory value for "
	         << optmarker << c->option << " not specified\n";
	    return -1;	/* mandatory value not specified */
	 }
      }
   default :
      break;
   }
   return -1;
}

void GetLongOpt::usage(std::ostream &outfile) const
{
   Cell *t;
   std::string usage_msg("usage: ");

   usage_msg += pname;
   usage_msg += " ";
   usage_msg += ustring;
   usage_msg += '\n';
   for ( t = table; t != 0; t = t->next ) {
      usage_msg += "\t";
      usage_msg += optmarker;
      usage_msg += t->option;
      if ( t->type == GetLongOpt::MandatoryValue )
	 usage_msg += " <$val>";
      else if ( t->type == GetLongOpt::OptionalValue )
	 usage_msg += " [$val]";
      usage_msg += " (";
      usage_msg += t->description;
      usage_msg += ")\n";
   }

   outfile << usage_msg << std::endl;
}

void GetLongOpt::store(const char *name, const char *value)
{
	Cell *t;
	for(t = table; t; t = t->next)
		if (!std::strcmp(name, t->option)) {
			t->value = value;
			break;
			}
	}


void CommandLineHandler::initialize_options()
{
  // NOTE: MandatoryValue means that a value must be specified _if_ the command
  // line option is used.  This does not make the command line option itself
  // mandatory.  Required command line inputs are enforced in check_usage().

  GetLongOpt::usage("[options and <args>]");

  // information options
  enroll("help",    GetLongOpt::Valueless, "Print this summary", NULL);

  enroll("version", GetLongOpt::Valueless, "Print DAKOTA version number", NULL);

  // I/O options
  enroll("input",   GetLongOpt::MandatoryValue,
	 "REQUIRED DAKOTA input file $val", NULL);

  enroll("output",  GetLongOpt::MandatoryValue,
	 "Redirect DAKOTA standard output to file $val", NULL);

  enroll("error",   GetLongOpt::MandatoryValue,
         "Redirect DAKOTA standard error to file $val", NULL);

  enroll("parser",  GetLongOpt::MandatoryValue,
	 "Parsing technology: nidr[strict][:dumpfile]", NULL);

  enroll("no_input_echo", GetLongOpt::Valueless, 
	 "Do not echo DAKOTA input file", NULL);

  // run mode options
  enroll("check",   GetLongOpt::Valueless, "Perform input checks", NULL);

  enroll("pre_run",  GetLongOpt::OptionalValue, 
  	 "Perform pre-run (variables generation) phase", NULL);

  enroll("run",     GetLongOpt::OptionalValue, 
	 "Perform run (model evaluation) phase", NULL);

  enroll("post_run", GetLongOpt::OptionalValue, 
	 "Perform post-run (final results) phase", NULL);

  // The read restart filename string is optional.  
  // read_restart not invoked: retrieve returns NULL
  // read_restart no value:    retrieve returns empty string
  // read_restart with value:  retrieve returns value
  enroll("read_restart", GetLongOpt::OptionalValue,
         "Read an existing DAKOTA restart file $val", NULL);

  enroll("stop_restart", GetLongOpt::MandatoryValue,
         "Stop restart file processing at evaluation $val", NULL);


  // The write restart filename string is optional.  
  // write_restart not invoked: retrieve returns NULL
  // write_restart no value:    retrieve returns enpty string
  // write_restart with value:  retrieve returns value
  enroll("write_restart", GetLongOpt::OptionalValue,
         "Write a new DAKOTA restart file $val", NULL);

  //enroll("mpi", GetLongOpt::Valueless,
  //       "Turn on message passing within an executable built with MPI", 0);
}


void CommandLineHandler::check_usage(int argc, char** argv)
{
  const char *cs;
  int optind = parse(argc, argv);
  //Cout << "CommandLineHandler::check_usage: optind = " << optind << std::endl;

  if (optind < 1) {
//#ifdef USE_MPI
// Must comment out "return -1" for NoMatch case in parse
//    Cerr << "Warning: some unrecognized tokens in CommandLineHandler::"
//         << "check_usage.\n         Assuming tokens are MPI arguments." 
//         << std::endl;
//#else // if no MPI, then echo usage message and exit
    usage();
    abort_handler(-1);
//#endif // USE_MPI
  }

  if (retrieve("help") != NULL) {
    // usage may be version-specific, so include it
    // BMA TODO: decide how to handle; could have call outputmanager's function
    //    output_version();
    usage();
    return;  // no further processing for usage
  }

  if (retrieve("version") != NULL) {
    //output_version();
    return;  // no further processing for version
  }

  if (retrieve("input") == NULL) { // an input file is REQUIRED
    if (optind == argc - 1)
      GetLongOpt::store("input", argv[optind]);
    else {
      usage();
      output_helper("Missing input file command line argument.", Cerr);
      abort_handler(-1);
    }
  }

  // the combination pre/post without run is unsupported
  if (retrieve("pre_run") != NULL && retrieve("run") == NULL && 
      retrieve("post_run") != NULL) {
    usage();
    std::string err_msg("\nRun phase '-run' is required when specifying both ");
    err_msg += "'-pre_run' and '-post_run'.";
    output_helper(err_msg, Cerr);
    abort_handler(-1);
  }

  if (retrieve("read_restart") == NULL && retrieve("stop_restart") != NULL) {
    usage();
    output_helper("\nread_restart is REQUIRED for use with stop_restart.", Cerr);
    abort_handler(-1);
  }

  cs = retrieve("parser");
  if (cs /*&& std::strncmp(cs,"idr",3)*/ && std::strncmp(cs,"nidr",4)) {
    usage();
    output_helper("\n-parser must specify nidr....", Cerr);
    abort_handler(-1);
  }
  
  // always output version before run proceeds
  //  output_version();

}


// BMA TODO: avoid this weird overload of usage if we retain GetLongOpt
void CommandLineHandler::usage(std::ostream &outfile) const
{
  if (worldRank == 0)
    GetLongOpt::usage(outfile);
}


/** When there is a valid ParallelLibrary, output only on rank 0 */
void CommandLineHandler::
output_helper(const std::string& message, std::ostream &os) const
{
  if (worldRank == 0)
    os << message << std::endl;
}


} // namespace Dakota
