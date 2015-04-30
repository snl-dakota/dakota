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
 * \file XMLProcessor.h
 *
 *  Defines the colin::XML_Processor class and colin::XMLProcessor()
 *  singleton.
 */

#ifndef colin_XMLProcessor_h
#define colin_XMLProcessor_h

#include <utilib/std_headers.h>

class TiXmlElement;

namespace colin
{

class XML_Processor;


/// A convenience function that provides a system-wide 
/// XML_Processor singleton
XML_Processor& XMLProcessor();


///
/// A class that coordinates the processing of XML elements.  
/// ElementFunctor's are registered with this class, and then this
/// class applies them to sequentially process each element.
///
class XML_Processor
{
public:
   class ElementFunctor
   {
   public:
      virtual ~ElementFunctor() {}
      virtual void process(TiXmlElement* root, int version) = 0;
      virtual void describe(std::ostream &os, size_t indent = 0) const = 0;
   };

   static const std::string indent_string;

public:

   ///
   XML_Processor();

   ///
   ~XML_Processor();

   /// Register top-level XML element (XML_Processor will own & delete pointer)
   void register_element(const std::string name,
                         const size_t describe_order,
                         ElementFunctor* fcn);

   /// Process a COLIN XML doctree
   void process(TiXmlElement* root);

   /// Describe the valid parsable XML syntax (recursively)
   void describe(std::ostream &os, size_t indent = 0) const;

private:

   ///
   struct Data;

   ///
   Data *data;
};


} // namespace colin

#endif // colin_XMLProcessor_h
