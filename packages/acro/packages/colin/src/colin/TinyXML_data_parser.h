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
 * \file TinyXml_data_parser.h
 *
 * Defines fundamental helper function for parsing a TinyXML element
 * into an instantiated object.
 */

#ifndef colin_TinyXml_data_parser_h
#define colin_TinyXml_data_parser_h

#include <utilib/Any.h>

class TiXmlElement;

namespace colin {

utilib::Any parse_data(const std::string& data, std::string datatype = "");

utilib::Any parse_xml_data(TiXmlElement* elt, std::string datatype = "");

} // namespace colin

#endif // colin_TinyXml_data_parser_h
