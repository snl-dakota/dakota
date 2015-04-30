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

#include <colin/XMLProcessor.h>

#include <utilib/exception_mngr.h>

#include <tinyxml/tinyxml.h>

using std::endl;

using std::string;
using std::ostringstream;

using std::make_pair;
using std::map;
using std::multimap;


namespace colin {

//---------------------------------------------------------------------
// XMLProcessor() singleton
//---------------------------------------------------------------------

XML_Processor& XMLProcessor()
{
   static XML_Processor processor;
   return processor;
}


//---------------------------------------------------------------------
// XML_Processor::Data definition
//---------------------------------------------------------------------

struct XML_Processor::Data {
   /// The registered top-level element handlers (by element name)
   map<string, ElementFunctor*> elements;

   /// The registered top-level element handlers (in description order)
   multimap<size_t, ElementFunctor*> description_order;
};


const std::string XML_Processor::indent_string = "   ";

//---------------------------------------------------------------------
// XML_Processor public methods
//---------------------------------------------------------------------


XML_Processor::XML_Processor()
   : data(new Data)
{}


XML_Processor::~XML_Processor()
{
   map<string, ElementFunctor*>::iterator it = data->elements.begin();
   map<string, ElementFunctor*>::iterator itEnd = data->elements.end();
   for( ; it != itEnd; ++it )
      delete it->second;

   delete data;
}


void
XML_Processor::register_element( const std::string name, 
                                 const size_t describe_order, 
                                 ElementFunctor* fcn )
{
   if ( ! data->elements.insert(make_pair(name, fcn)).second )
      EXCEPTION_MNGR(std::runtime_error, "XML_Processor::register_element(): "
                     "duplicate top-level element handler, " << name);

   data->description_order.insert(make_pair(describe_order, fcn));
}


void
XML_Processor::process(TiXmlElement* root)
{
   // Should we check that the root node is named correctly???
   //  ... naa
   
   // Get the file's self-proclaimed version
   int version = 0;
   int ans = root->QueryIntAttribute("version", &version);
   if ( ans == TIXML_WRONG_TYPE )
      EXCEPTION_MNGR(std::runtime_error, "XML_Processor::process(): "
                     "XML version attribute not parsable as an int");

   // Process all the top-level elements
   std::map<string, ElementFunctor*> &handlers = data->elements;
   std::map<string, ElementFunctor*>::iterator it;
   std::map<string, ElementFunctor*>::iterator itEnd = handlers.end();

   TiXmlElement* node = root->FirstChildElement();
   while ( node != NULL )
   {
      it = handlers.find(node->ValueStr());
      if ( it == itEnd )
         EXCEPTION_MNGR(std::runtime_error, "XML_Processor::process(): "
                        "No handler registered for element '" << 
                        node->ValueStr() << "'");
      it->second->process(node, version);
      node = node->NextSiblingElement();
   }
}


void
XML_Processor::describe(std::ostream &os, size_t indent) const
{
   ostringstream indention;
   for(size_t i=0; i<indent; ++i)
      indention << indent_string;

   os << indention.str() << "<ColinInput>" << endl;

   ++indent;
   std::multimap<size_t, ElementFunctor*>::iterator it 
      = data->description_order.begin();
   std::multimap<size_t, ElementFunctor*>::iterator itEnd 
      = data->description_order.end();
   for( ; it != itEnd; ++it )
      it->second->describe(os, indent);

   os << indention.str() << "</ColinInput>" << endl;
}


} // namespace colin
