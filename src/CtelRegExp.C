/////////////////////////////////////////////////////////////////////////////
// $Header$
//
// Copyright (c) 1998 ConnectTel, Inc. All Rights Reserved.
//  
// MODULE DESCRIPTION:

/** This module implements a C++ wrapper for Regular Expressions based on the
    public domain engine for regular expressions released by:
	  Copyright (c) 1986 by University of Toronto.
	  Written by Henry Spencer.  Not derived from licensed software.
*/

/////////////////////////////////////////////////////////////////////////////

#include "CtelRegExp.H"		// C++ class wrapper.
#include "regexp.h"		// original C engine

// **********************************************************************
// The following code this a kludge to catch the error code raised by the 
// C engine. Unfortunately, just a string is sent in regerror(), and we need 
// to keep a mapping of them to translate to a number in order to be able 
// to convert the error code to a more readable message later.
/// Data structure to hold errors
struct ErrorTable 
{
   /// Enumerated type to hold status codes
   CtelRegexp::RStatus rc;
   /// Holds character string error message
   const char *msg;
};

// Mapping of the error codes and the message raised by the C engine
static ErrorTable errorMapping[] =  
{
    { CtelRegexp::RE_GOOD,         "Ok"             			},
    { CtelRegexp::EXP_TOO_BIG,     "regexp too big" 			},
    { CtelRegexp::OUT_OF_MEM,      "out of space"   			},
    { CtelRegexp::TOO_MANY_PAR,    "too many ()"    			},
    { CtelRegexp::UNMATCH_PAR,     "unmatched ()"   			},
    { CtelRegexp::STARPLUS_EMPTY,  "*+ operand could be empty"  },
    { CtelRegexp::STARPLUS_NESTED, "nested *?+"          		},
    { CtelRegexp::INDEX_RANGE,     "invalid [] range"    		},
    { CtelRegexp::INDEX_MATCH,     "unmatched []"        		},
    { CtelRegexp::STARPLUS_NOTHING,"?+* follows nothing" 		},
    { CtelRegexp::TRAILING,        "trailing \\"                },
    { CtelRegexp::INT_ERROR,       "junk on end"                },
    { CtelRegexp::INT_ERROR,       "internal urp"               },
    { CtelRegexp::INT_ERROR,       "internal disaster"          },
    { CtelRegexp::BAD_PARAM,       "NULL parameter"             },
    { CtelRegexp::BAD_OPCODE,      "corrupted opcode"           },
};

// Global Data to hold the error codes raised by the C engine
static int errorCode 		    = 0;
static const char *errorMsg = "Ok";

// This table has the messages returned to the user.
static const char* errorMsgsTable[] =
{
   "Success",                                                     // 0
   "Regular Expression is too big to compile",                    // 1
   "Out of memory: could not allocate memory to perform command", // 2
   "Regular Expression Syntax: too many parenteses ()",           // 3
   "Regular Expression Syntax: unmatched parenteses ()",          // 4
   "Regular Expression Syntax: *+ operand could be empty",        // 5
   "Regular Expression Syntax: nested *?+",                       // 6
   "Regular Expression Syntax: invalid [] range",                 // 7
   "Regular Expression Syntax: unmatched []",                     // 8
   "Regular Expression Syntax: ?+* follows nothing",              // 9
   "Regular Expression Syntax: trailing \\",                      // 10
   "Internal error when processing command",                      // 11
   "Invalid parameter",                                           // 12
   "Regular Expression internal code is corrupted"                // 13
};

/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: 
// This method is called by the Regexp engine when one error is detected.
//
// RETURNS: nothing.
//
/////////////////////////////////////////////////////////////////////////////
extern "C" void regerror( char *s )
{
   size_t arrSize = sizeof( errorMapping )/sizeof( errorMapping[ 0 ] ); 
   for( size_t i = 0; i < arrSize; i++ )
   {
      if( !std::strcmp( s, errorMapping[ i ].msg ) )
      {
         errorCode = errorMapping[ i ].rc;
         errorMsg = errorMsgsTable[ errorCode ];
         return;
      }
   }

   // Just show internal error if the code gets here.
   errorCode = CtelRegexp::INT_ERROR;
   errorMsg = errorMsgsTable[ errorCode ];
}
// **********************************************************************




/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Ctor - compiles the regular expression to an internal 
//                     representation. The caller should check the return code
//                     to make sure that everything has succeed.
//
// RETURNS: nothing.
//
/////////////////////////////////////////////////////////////////////////////
CtelRegexp::CtelRegexp( const std::string & pattern )
: r( 0 ), status( CtelRegexp::RE_GOOD )
{
  compile( pattern );
}

/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Dtor - Delete any resources allocate by the instance.
//
// RETURNS: nothing.
//
/////////////////////////////////////////////////////////////////////////////
CtelRegexp::~CtelRegexp()
{
  if( r ) 
  {
    std::free( r );
  }
}



/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Clear any previous error code & messages
//
// RETURNS: nothing.
//
/////////////////////////////////////////////////////////////////////////////
void CtelRegexp::clearErrors()
{
   status = CtelRegexp::RE_GOOD;
   statusMsg = "Ok";
}

/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Compile a new Regular Expression
//
// RETURNS: true  - success;
//			false - error while compling regexp.
//
/////////////////////////////////////////////////////////////////////////////
bool CtelRegexp::compile( const std::string & pattern )
{
   // Save a copy of the pattern
   strPattern = pattern;
   if( r )
   {
      std::free( r );
   }
   // Compile it to make sure that everything is fine.
   r = regcomp( (char *)strPattern.c_str() );
   status = (RStatus)::errorCode;
   statusMsg = ::errorMsg;
   return ( r ) ? true : false;
}

/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Matches a string against the current regular expression.
//
// RETURNS: sub-string representing the first match or an empty string if no
//          match is found.
//
/////////////////////////////////////////////////////////////////////////////
std::string CtelRegexp::match( const std::string & str )
{
   size_t start;
   size_t size;
   std::string strMatch;
   if( match( str, &start, &size ) )
   {
      strMatch = std::string( (const char *)r->startp[ 0 ], size ); 
   }
   status = (RStatus)::errorCode;
   statusMsg = ::errorMsg;
   return strMatch;
}

/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Get the indexes of the string matching with the regular 
//              expression. If no match is found, zeros are set to the
//              indexes and the function returns false.
//
// RETURNS: true  - success matching 
//          false - no matches 
//
/////////////////////////////////////////////////////////////////////////////
bool CtelRegexp::match( const std::string & str, size_t *start, size_t *size )
{
   // Try to match
   if( r && regexec( r, (char *)str.c_str() ) ) 
   {     
     status = (RStatus)::errorCode;
     statusMsg = ::errorMsg;
     // Make a sanity check of the internal pointers.
     if( r->endp[ 0 ] && r->startp[ 0 ] )
     {
        // Get the size of the matching 
        *size = r->endp[ 0 ] - r->startp[ 0 ];
        *start = r->startp[ 0 ] - str.c_str();
        return true;
     }
   }
   status = (RStatus)::errorCode;
   statusMsg = ::errorMsg;
   // Well, something did not work; return an error
   *start = 0; 
   *size = 0; 
   return false;
}



/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Scan the whole string to find all possible sub-strings that
//				matches with the regular expression.
//
// RETURNS: true  - success matching 
//			false - no matches 
//
/////////////////////////////////////////////////////////////////////////////
bool CtelRegexp::split( const std::string & str,
			std::vector< std::string > & all_matches )
{
   // Clear up anything in the array
   all_matches.clear(); 
   const char *p = str.c_str(); 
   size_t start;
   size_t size;
   bool rc = false;  // assume no matches
   // match any sub-string
   while( match( std::string( p ), &start, &size ) )
   {
       rc = true;
       std::string s( p+start, size );
	   all_matches.push_back( s );
	   p = p + ( start + size );
	   // Check if we have reached the end of the string.
	   if( (*p == 0 ) || (*( p+1 ) == 0 ) )
	   {
  	       break;
	   }
   }
   return rc;
}


#if 0
/////////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION: Test driver of this class
//
// RETURNS: nothing.
//
/////////////////////////////////////////////////////////////////////////////
void main()
{
   char *pat = "sdfdfsgfdg013alocalAB5163k000la";
   CtelRegexp reg( "l[a-z]*" );
   printf( "Trying to match:\n" );
   std::string str = reg.match( pat );
   printf( "Success1: [ %s ]\n", str.c_str() );  // matches to: local
   size_t size = 0;
   size_t start = 0;
   if( reg.match( pat, &start, &size ) )
   {
      printf( "Start=%d, Size=%d\n", start, size );
   }
   else
   {
      printf( "No Matches - RC=%d, RCMSG=%s\n", reg.getStatus(),
	      reg.getStatusMsg().c_str() );
   }
   // match to: local, la
   std::vector< std::string > v;
   reg.split( pat, v );
   std::vector< std::string >::iterator iter;
   for( iter = v.begin(); iter != v.end(); ++iter )
   {
	  printf( "String: %s   ", (*iter).c_str() );
   }

}

#endif
