#ifndef PARSER_H
#define PARSER_H

//==============================================================================
//
//           Parser - the parser class in the gpx library
//
//               Copyright (C) 2013  Dick van Oudheusden
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free
// Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//==============================================================================
//
//  $Date$ $Revision$
//
//==============================================================================

#include <iostream>

#include "expat.h"

#include "GPX.h"

namespace gpx
{
  ///
  /// @class Parser
  ///
  /// @brief The xml parser class.
  ///
  
  class DLL_API Parser
  {
  public:

    ///
    /// Constructor
    ///
    /// @param  report  the report stream for warnings and errors (or 0 for not used)
    ///
    Parser(Report *report);

    ///
    /// Deconstructor
    ///
    virtual ~Parser();

    // Properties

    ///
    /// Get the root GPX object after parsing
    ///
    /// @return the root GPX node (or 0 if not present)
    ///
    GPX *root() const { return _root; }

    ///
    /// Get the last error text
    ///
    /// @return the last error text
    ///
    const std::string &errorText() const { return _errorText; }

    ///
    /// Get the last error line number
    ///
    /// @return the last error line number
    ///
    int errorLineNumber() const { return _errorLineNumber; }

    ///
    /// Get the last error column number
    ///
    /// @return the last error column number
    ///
    int errorColumnNumber() const { return _errorColumnNumber; }

  private:

    // Building methods

    ///
    /// Build an element node
    ///
    /// @param  name    the name of the element
    ///
    virtual void makeElement(const char *name);

    ///
    /// Build an attribute node
    ///
    /// @param  name    the name of the attribute
    ///
    virtual void makeAttribute(const char *name);

    ///
    /// Done building the node
    ///
    virtual void made();

    ///
    /// Set the value
    ///
    /// @param value   the value of the attribute or element
    ///
    virtual void value(std::string value);

  public:

    // Parsing methods

    ///
    /// Parse data
    ///
    /// @param  data    the data to be parsed
    /// @param  length  the length of the data
    /// @param  isFinal is this the last data ?
    ///
    /// @return success
    ///
    bool parse(const char *data, size_t length, bool isFinal);

    ///
    /// Parse text
    ///
    /// @param  text    the zero terminated text to be parsed
    /// @param  isFinal is this the last data ?
    ///
    /// @return success
    ///
    bool parse(const char *text, bool isFinal);

    ///
    /// Parse string
    ///
    /// @param  data    the data to be parsed
    /// @param  isFinal is this the last data ?
    ///
    /// @return success
    ///
    bool parse(const std::string &data, bool isFinal);

    ///
    /// Parse an input stream
    ///
    /// @param  stream        the stream to be parsed
    ///
    /// @return the root GPX node (or 0 after an error)
    ///
    GPX *parse(std::istream &stream);

  private:

    // Start using expat
    void startExpat();

    // Stop using expat
    void stopExpat();

    // Statics for expat
    static void xmlDeclHandler              (void *userData, const XML_Char *version, const XML_Char *encoding, int standalone);
    static void startElementHandler         (void *userData, const XML_Char *name,    const XML_Char **atts);
    static void endElementHandler           (void *userData, const XML_Char *name);
    static void characterDataHandler        (void *userData, const XML_Char *s, int len);
    static void commentHandler              (void *userData, const XML_Char *data);
    static void processingInstructionHandler(void *userData, const XML_Char *target, const XML_Char *data);
    static void startCdataSectionHandler    (void *userData);
    static void endCdataSectionHandler      (void *userData);
    static void defaultHandler              (void *userData, const XML_Char *s, int len);
    static void startNamespaceDeclHandler   (void *userData, const XML_Char *prefix, const XML_Char *uri);
    static void endNamespaceDeclHandler     (void *userData, const XML_Char *prefix);

    // Members
    XML_Parser      _parser;
    std::string     _errorText;
    int             _errorLineNumber;
    int             _errorColumnNumber;
    Node           *_current;
    GPX            *_root;
    Report         *_report;

    // Disable copy constructors
    Parser(const Parser &);
    Parser& operator=(const Parser &);  
  };
}


#endif

