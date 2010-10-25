#ifndef __CtelRegExp_h__
#define __CtelRegExp_h__
/////////////////////////////////////////////////////////////////////////////
// $Header$
//
// Copyright (c) 1998 ConnectTel, Inc. All Rights Reserved.
//  
// MODULE DESCRIPTION: 
// This module define a C++ class that wraps the Regular Expression
// engine released by Henry Spencer of the University of Toronto.
//
// This class is not Thread Safe. If it is used in a multi-threaded application,
// proper synchronization must be provided.
//
// MODIFICATION/HISTORY:
// $Log$
//
// Revision 1.2  1998/11/12 15:36:54  rds
// *** empty log message ***
//
// Revision 1.1  1998/11/12 15:02:14  rds
// Initial revision
//
/////////////////////////////////////////////////////////////////////////////

// NOTE: this will not compile in non-ANSI mode as it uses std::string
#include "system_defs.h"
#include <string>
#include <vector>

struct regexp;

/////////////////////////////////////////////////////////////////////////////
/**
 DESCRIPTION: Wrapper for the Regular Expression engine( regexp ) released by
	      Henry Spencer of the University of Toronto.

*/
/////////////////////////////////////////////////////////////////////////////
class CtelRegexp
{
public:
   /// Error codes reported by the engine - Most of these codes never really 
   /// occurs with this implementation.
   enum RStatus 
   {
  	 GOOD = 0,	   ///< Success - no errors
	 EXP_TOO_BIG,	   ///< Regular expression is too big to be compiled
	 OUT_OF_MEM,       ///< out of space( memory )   			
	 TOO_MANY_PAR,     ///< too many () parenteses    			
	 UNMATCH_PAR,      ///< unmatched () parenteses   			
	 STARPLUS_EMPTY,   ///< *+ operand could be empty
	 STARPLUS_NESTED,  ///< nested *?          		
	 INDEX_RANGE,      ///< invalid [] range    		
	 INDEX_MATCH,      ///< unmatched []        		
	 STARPLUS_NOTHING, ///< ?+* follows nothing 		
	 TRAILING,         ///< trailing \\"              
	 INT_ERROR,        ///< junk on end, "internal urp", "internal disaster"
	 BAD_PARAM,        ///< NULL parameter       
	 BAD_OPCODE        ///< corrupted opcode       
   };

   /// Constructor - compile a regular expression
   CtelRegexp( const std::string & pattern );
   /// Destructor
   ~CtelRegexp();

   /// Compile a new regular expression
   bool compile( const std::string & pattern );
   /// matches a particular string; this method returns a string
   /// that is a sub-string matching with the regular expression
   std::string match( const std::string & str );
   /// another form of matching; returns the indexes of the maching
   bool match( const std::string & str, size_t *start, size_t *size );

   // Error codes
   /// Get status
   RStatus getStatus() { return status; }
   /// Get status message
   const std::string & getStatusMsg() { return statusMsg; }
   /// Clear all errors
   void clearErrors(); 

   /// Return regular expression pattern
   const std::string & getRe() { return strPattern; }

   /// Split 
   bool split( const std::string & str,
	       std::vector< std::string > & all_matches );

private:

   // Copy and assignment not allowed at this point
   /// Private copy constructor
   CtelRegexp( const CtelRegexp & );
   /// Private assignment operator 
   CtelRegexp & operator=( const CtelRegexp & );

   /// STL string to hold pattern
   std::string strPattern;
   /// Pointer to regexp
   regexp *r;
   /// Return status, enumerated type
   RStatus status;
   /// STL string to hold status message
   std::string statusMsg;
};

#endif  //  __CtelRegExp_h__
